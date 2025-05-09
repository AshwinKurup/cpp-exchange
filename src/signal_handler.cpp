#include "signal_handler.h"

namespace trading {
    std::atomic<bool> running(true);

    static void signalHandler(int signal) {
        if (signal == SIGINT) {
            running.store(false, std::memory_order_relaxed);
        }
    }

    void setupSignalHandler() {
        std::signal(SIGINT, signalHandler);
    }
    
    void kill() {
      running.store(false, std::memory_order_relaxed);
    }
} 
