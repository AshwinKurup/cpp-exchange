#ifndef EXCHANGE_H
#define EXCHANGE_H
#include "exc_benchmark.h" 
#include "signal_handler.h"
#include "order.h"
#include "blockingconcurrentqueue.h"
#include <map>
#include <iostream>
#include <vector>
#include <thread>

using namespace std;

class Exchange {
private:
    std::map<float, vector<Order>> bids;
    std::map<float, vector<Order>> asks;
    OrderQueue& q;
    OrderQueue& oeq;
    ExchangeBenchmark& benchmark;

    void on_maker_order(Order order);
    void on_taker_order(Order order);
    void notify_order(float amt_traded, Order& book_order, Order& order);
    void notify_orderbook(float amt_traded, Order& book_order);
    void print_full_book();
    void print_levels(const std::map<float, vector<Order>>& ob, QuoteSide quote_side);
    void print_order_queue(const std::vector<Order>& orderbook_queue);
    bool is_taker(Order& order);

public:
    Exchange(OrderQueue& queue, OrderQueue& oeq, ExchangeBenchmark& benchmark);
    void run();
};

#endif  // EXCHANGE_H

