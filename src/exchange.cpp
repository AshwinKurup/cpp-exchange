#include "exchange.h"
#include "exc_benchmark.h" 
#include <optional>
#include "signal_handler.h"
#include <thread>
#include <chrono>
#include <spdlog/spdlog.h>
#include <stdexcept>

using namespace std;

Exchange::Exchange(OrderQueue& queue, OrderQueue& oeq, ExchangeBenchmark& benchmark) 
    : q(queue), oeq(oeq), benchmark(benchmark) {}

void Exchange::on_maker_order(Order order) {
    bool facts = order.side == Side::Buy;
    std::map<float, vector<Order>>& quote_lvls = order.side == Side::Buy ? bids : asks;
    quote_lvls[order.price].push_back(order);
}

void Exchange::on_taker_order(Order order) {
    benchmark.start_order_timer();
    std::map<float, vector<Order>>& quote_lvls = order.side == Side::Buy ? asks : bids;
    float last_matched_px;
    int naive_iter_count = 0;
    bool price_exceeded = false;
    bool amount_depleted = false;

    // iterate through levels 
    while(!quote_lvls.empty()) {
      if (naive_iter_count > 1000) {
        spdlog::info("Early breaking for safety, very unlikely that any order queue is allowed to have >1000 orders");
        break;
      }
      std::map<float, vector<Order>>::iterator next_book_pair = order.side == Side::Buy ? quote_lvls.begin() : std::prev(quote_lvls.end());
      float book_px = next_book_pair->first;
      last_matched_px = book_px;
      price_exceeded = order.side == Side::Buy ? (order.price < book_px) : (order.price > book_px);
      
      if (price_exceeded) {
        break;
      };
      
      vector<Order>& book_order_queue = next_book_pair->second;
    
      // iterate through indiv queue at one level
      while (!book_order_queue.empty()) {
        naive_iter_count++;
        if (naive_iter_count > 1000) {
          spdlog::info("Early breaking for safety, very unlikely that any order queue is allowed to have >1000 orders");
          break;
        }
        Order& book_order = book_order_queue[0];
        int amt_traded = std::min(book_order.amount, order.amount);
        print_order_queue(book_order_queue);

        book_order.amount -= amt_traded;
        order.amount -= amt_traded;
        notify_order(amt_traded, book_order, order); 
        notify_orderbook(amt_traded, book_order);

        if (order.amount == 0) {
            amount_depleted = true;
        } else if (order.amount < 0) {
            spdlog::error("Negative taker order amount detected! This should never happen.");
            throw std::runtime_error("Negative taker order amounts are not allowed");
        }   
        
        if (book_order.amount == 0) {
            book_order_queue.erase(book_order_queue.begin());
        } else if (book_order.amount < 0) {
            spdlog::error("Negative book order amount detected! This should never happen.");
            throw std::runtime_error("Negative book order amounts are not allowed");
        }
        if (amount_depleted) {
          break;
        } 
      };

      if (book_order_queue.empty()) {
        quote_lvls.erase(book_px);
      }
    };
    benchmark.end_order_timer();
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
  return taker;
}

void Exchange::run() {
    Order order;
    while (trading::running.load()) {
        if (std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() % 5 == 0) {
            spdlog::info("Periodic report: {}", benchmark.generate_report(std::chrono::seconds(5)));
        }
        if (q.try_dequeue(order)) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            spdlog::info("\n\n Exchange Processing: {}", order);
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
    spdlog::info("Finished Exchange");
}
