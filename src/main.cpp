#include "order.h"
#include "exchange.h"
#include "concurrentqueue.h"
#include "exc_benchmark.h"

#include <boost/lockfree/queue.hpp>
#include <iostream>
#include <bits/stdc++.h>
#include <vector>
#include <bits/stdc++.h>
#include <map>
#include <random>
#include "signal_handler.h"

using namespace std;

class FastRandomString {
  public:
      FastRandomString() : gen(std::random_device{}()), dis(0, chars.size() - 1) {}
  
      std::string generate(size_t length) {
          std::string result;
          result.reserve(length);
          for (size_t i = 0; i < length; ++i) {
              result += chars[dis(gen)];
          }
          return result;
      }
  
  private:
      static constexpr std::string_view chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
      std::mt19937 gen;
      std::uniform_int_distribution<size_t> dis;
  };

class Trader {
private: 
  OrderQueue& queue;
  float last_traded_price;
  FastRandomString rng;
public:

  Trader(OrderQueue& q, float initial_order_price)
    : queue(q), last_traded_price(initial_order_price) {}

  void trader_loop() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> amount_dis(1, 100);
    std::uniform_real_distribution<float> price_dis(1, 100);

    int buy_count;
    int sell_count;

    while (trading::running.load()) {
      cout << "\n\n Trader making a decision..." << endl;
      int rand_seed = amount_dis(gen);
      int order_amt = amount_dis(gen);
      float price = price_dis(gen);

      Side side = (order_amt % 2 == 0) ? Side::Buy : Side::Sell;
      Style style = (rand_seed % 3 ==0) ? Style::Taker : Style::Maker;

      if (side == Side::Buy) {
        buy_count += 1;
      } else {
        sell_count += 1;
      }

      cout << "buy count is " << buy_count << " sell count is " << sell_count << endl;
        
      Order order1(style, side, price, order_amt, rng.generate(12));
      if (!trading::running.load()) {
        cout << "Breaking the Trader" << endl;
        break;
      }
      cout << "Trader Pushing Order to Queue" << order1 << endl;

      this->queue.enqueue(order1);
      std::this_thread::yield();
    };
  }
};

class MarketMaker {
private:
  Side last_traded_side; 
}; 

int main() {
    OrderQueue q;
    OrderQueue oeq;
    Exchange exc = Exchange(q, oeq);    
    Trader t1 = Trader(q, 0.0);  
    
    trading::setupSignalHandler();
    
    std::thread exc_thread(std::bind(&Exchange::run, &exc));
    std::thread trader1_thread(std::bind(&Trader::trader_loop, &t1));
    exc_thread.join();
    trader1_thread.join();

    ExchangeBenchmark benchmark;
    std::string report = benchmark.generate_report();
    spdlog::info("Benchmark Report: {}", report);

    return 0;
}
