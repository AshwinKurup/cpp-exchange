#include "exchange.h"
#include <optional>
#include "signal_handler.h"
#include <thread>
#include <chrono>
#include <spdlog/spdlog.h>
#include <stdexcept>

using namespace std;

Exchange::Exchange(OrderQueue& queue, OrderQueue& oeq) 
    : q(queue), oeq(oeq) {}

void Exchange::on_maker_order(Order order) {
    bool facts = order.side == Side::Buy;
    std::map<float, vector<Order>>& quote_lvls = order.side == Side::Buy ? bids : asks;
    spdlog::info("within order.side: {} for on on_maker_order and our truth is that: {}", order, facts);
    quote_lvls[order.price].push_back(order);
}

void Exchange::on_taker_order(Order order) {
    std::map<float, vector<Order>>& quote_lvls = order.side == Side::Buy ? asks : bids;
    float last_matched_px;
    bool break_cond = false;
    while(!quote_lvls.empty() && !break_cond) {
      
      std::map<float, vector<Order>>::iterator next_book_pair = order.side == Side::Buy ? quote_lvls.begin() : std::prev(quote_lvls.end());
      float book_px = next_book_pair->first;
      spdlog::info("Iteration price: {}, Order side: {}, Quote levels size: {}, Order: {}",
                   book_px, order.side, quote_lvls.size(), order);
      last_matched_px = book_px;
      break_cond = order.side == Side::Buy ? (order.price < book_px) : (order.price > book_px);
      bool stillcan = order.amount > 0;
      spdlog::info("Order amount: {}, Can still trade: {}", order.amount, stillcan);
      break_cond = break_cond || (order.amount == 0);
      if (break_cond) {
        break;
      }
      
      vector<Order>& book_order_queue = next_book_pair->second;
    
      bool should_break = false; 
      while (!book_order_queue.empty() && !should_break) {

        print_order_queue(book_order_queue);
        Order& book_order = book_order_queue[0];
        spdlog::info("Book order: {}", book_order);
        int _amt_traded = std::min(book_order.amount, order.amount);
        int amt_traded = std::max(_amt_traded, 0);
        spdlog::info("Book order queue size: {}, _amt_traded: {}, amt_traded: {}, Taker order: {}, Should break: {}",
                     book_order_queue.size(), _amt_traded, amt_traded, order, should_break);
        
        book_order.amount -= amt_traded;
        order.amount -= amt_traded;
        notify_order(amt_traded, book_order, order); 
        notify_orderbook(amt_traded, book_order);

        if (order.amount == 0) {
            spdlog::info("Setting should_break to true because order amount is zero: {}", order.amount);
            should_break = true;
        } else if (order.amount < 0) {
            spdlog::error("Negative taker order amount detected! This should never happen.");
            throw std::runtime_error("Negative taker order amounts are not allowed");
        }   
        
        if (book_order.amount == 0) {
            should_break = true;
            book_order_queue.erase(book_order_queue.begin()); // this is O(n), so later implement as dequeue
        } else if (book_order.amount < 0) {
            spdlog::error("Negative book order amount detected! This should never happen.");
            throw std::runtime_error("Negative book order amounts are not allowed");
        }

        if (should_break) {
          break;
        } 
      };

      if (book_order_queue.empty()) {
        spdlog::info("Consumed price level: {}", book_px);
        quote_lvls.erase(book_px);
      }
    };
}

void Exchange::notify_order(float amt_traded, Order& book_order, Order& order) {
  spdlog::info("Notifying order, book order: {}, taker order: {}", book_order, order);
}

void Exchange::notify_orderbook(float amt_traded, Order& book_order) {
  spdlog::info("Notifying order book update: {} units traded for book order: {}", amt_traded, book_order);
}

void Exchange::print_full_book() {
    spdlog::info("=============");
    print_levels(asks, QuoteSide::Ask);
    spdlog::info("===== A =====");
    spdlog::info("===== B =====");
    print_levels(bids, QuoteSide::Bid);
    spdlog::info("=============");
}

void Exchange::print_levels(const std::map<float, vector<Order>>& ob, QuoteSide quote_side) {
    for (auto it = ob.rbegin(); it != ob.rend(); it++) {
      float px = it->first;
      const std::vector<Order>& orders = it->second;
      print_order_queue(orders);
    }
}

void Exchange::print_order_queue(const vector<Order>& orderbook_queue) {
    for (const Order& order : orderbook_queue) {
      spdlog::info("Order in queue: {}", order);
    }
}

bool Exchange::is_taker(Order& order) {
  bool taker = (!asks.empty() && order.side == Side::Buy && order.price >= asks.begin()->first) ||
               (!bids.empty() && order.side == Side::Sell && order.price <= bids.rbegin()->first) || 
               (order.style == Style::Taker);
  spdlog::info("Order: {}, Is taker: {}", order, taker);
  return taker;
}

void Exchange::run() {
    Order order;
    while (trading::running.load()) {
        if (q.try_dequeue(order)) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            spdlog::info("\n\n Exchange Processing: {}", order);
            spdlog::info("Decided is taker order: {}", is_taker(order)); 
            if (is_taker(order)) {
                on_taker_order(order);
            } else {
                on_maker_order(order);
            }
            print_full_book();
        } else {
            std::this_thread::yield();
        }
    }
}
