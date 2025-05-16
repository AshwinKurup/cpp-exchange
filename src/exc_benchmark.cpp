#include "exc_benchmark.h" 
#include <chrono>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <spdlog/spdlog.h>

ExchangeBenchmark::ExchangeBenchmark() {}

void ExchangeBenchmark::start_order_timer() {
  last_checkpoint = std::chrono::high_resolution_clock::now();
}

void ExchangeBenchmark::end_order_timer() {
    auto now = std::chrono::high_resolution_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_checkpoint);
    order_latencies.push_back(latency);
    total_orders_processed++;
}

void ExchangeBenchmark::calculate_metrics(std::chrono::seconds runtime) {
    if (order_latencies.empty()) {
        spdlog::warn("No orders processed during benchmark");
        return;
    }
    std::sort(order_latencies.begin(), order_latencies.end());
    auto total_ns = std::accumulate(order_latencies.begin(), order_latencies.end(), 
                                   std::chrono::nanoseconds(0));
    metrics.mean_order_latency_ns = total_ns.count() / static_cast<double>(order_latencies.size());
    metrics.median_order_latency_ns = order_latencies[order_latencies.size() / 2].count();
    size_t p99_index = static_cast<size_t>(order_latencies.size() * 0.99);
    metrics.p99_order_latency_ns = order_latencies[p99_index].count();
    metrics.max_order_latency_ns = order_latencies.back().count();
    metrics.throughput_orders_per_sec = total_orders_processed / runtime.count();
}

std::string ExchangeBenchmark::generate_report() {
    std::stringstream ss;
    ss << "\n==== EXCHANGE PERFORMANCE BENCHMARK RESULTS ====\n";
    ss << "total orders processed: " << total_orders_processed << "\n";
    
    ss << "order Latency metrics:\n";
    ss << "  mean latency: " << std::fixed << std::setprecision(2) << metrics.mean_order_latency_ns << " ns\n";
    ss << "  median latency: " << std::fixed << std::setprecision(2) << metrics.median_order_latency_ns << " ns\n";
    ss << "  99th percentile: " << std::fixed << std::setprecision(2) << metrics.p99_order_latency_ns << " ns\n";
    ss << "  max latency: " << std::fixed << std::setprecision(2) << metrics.max_order_latency_ns << " ns\n\n";
    
    ss << "throughput metrics:\n";
    ss << "  orders per second: " << std::fixed << std::setprecision(2) << metrics.throughput_orders_per_sec << "\n";
    ss << "===============================================\n";
    return ss.str();
}
