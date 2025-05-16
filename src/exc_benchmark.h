#ifndef EXC_BENCHMARK_H
#define EXC_BENCHMARK_H

#include <chrono>
#include <vector>
#include <string>

class ExchangeBenchmark {
private:
    std::vector<std::chrono::nanoseconds> order_latencies;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_checkpoint;
    int total_orders_processed = 0;
    
    struct PerformanceMetrics {
        double mean_order_latency_ns = 0;
        double median_order_latency_ns = 0;
        double p99_order_latency_ns = 0;
        double max_order_latency_ns = 0;
        double throughput_orders_per_sec = 0;
    } metrics;

public:
    ExchangeBenchmark();
    void reset();
    void start_order_timer();
    void end_order_timer();
    void record_matching_latency(std::chrono::nanoseconds latency);
    void calculate_metrics(std::chrono::seconds runtime);
    std::string generate_report(std::chrono::seconds runtime);
};

#endif
