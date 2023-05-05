#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

class WaitFreeCounter {
public:
    WaitFreeCounter(int num_threads) : counters(num_threads, 0) {}

    void increment(int thread_id) {
        counters[thread_id].fetch_add(1, std::memory_order_relaxed);
    }

    int get_total() const {
        int sum = 0;
        for (const auto& counter : counters) {
            sum += counter.load(std::memory_order_relaxed);
        }
        return sum;
    }

private:
    std::vector<std::atomic<int>> counters;
};

void worker(WaitFreeCounter& counter, int thread_id) {
    for (int i = 0; i < 1000; ++i) {
        counter.increment(thread_id);
    }
}

int main() {
    const int num_threads = 4;
    WaitFreeCounter counter(num_threads);
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, std::ref(counter), i);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Total count: " << counter.get_total() << std::endl;

    return 0;
}
