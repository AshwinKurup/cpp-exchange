#ifndef ORDER_H
#define ORDER_H

#include <iostream>
#include <csignal>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h> 

using namespace std;

enum Style { Maker, Taker };
enum Side { Buy, Sell };
enum QuoteSide { Bid, Ask };

template <>
struct fmt::formatter<Side> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(Side s, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", (s == Side::Buy) ? "Buy" : "Sell");
    }
};

class Order { 
public: 
    Style style;
    Side side;
    float price;
    int amount; 
    string id;

    Order() : style(Style::Maker), side(Side::Buy), price(0.0), amount(0), id("-1") {}
    Order(Style style, Side side, float price, int amount, string id) : style(style), side(side), price(price), amount(amount), id(id) {}
    Order(const Order& other) = default;

    friend std::ostream& operator<<(std::ostream& os, const Order& order) {
        os << "Order(style: " << (order.style == Style::Maker ? "Maker" : "Taker")
           << ", side: " << (order.side == Side::Buy ? "Buy" : "Sell")
           << ", price: " << order.price 
           << ", amount: " << order.amount
           << ", id: " << order.id << ")";
        return os;
    }
};

template <>
struct fmt::formatter<Order> {
  constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const Order& order, FormatContext& ctx) {
    return fmt::format_to(ctx.out(), "Order(side: {}, price: {}, amount: {}, id: {})", order.side, order.price, order.amount, order.id);
  }
};

#endif // ORDER_H
