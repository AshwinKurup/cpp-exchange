#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H
#include "order.h"
#include <atomic>
#include <csignal>
// #include "blockingconcurrentqueue.h"
#include "concurrentqueue.h"
// using OrderQueue = moodycamel::BlockingConcurrentQueue<Order, moodycamel::ConcurrentQueueDefaultTraits>;

using OrderQueue = moodycamel::ConcurrentQueue<Order>;
namespace trading {
    extern std::atomic<bool> running;
    void setupSignalHandler();
}

#endif // SIGNAL_HANDLER_H 