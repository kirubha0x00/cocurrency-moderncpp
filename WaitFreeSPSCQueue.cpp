#include <atomic>
#include <iostream>
#include <thread>

template <typename T, size_t N>
class WaitFreeSPSCQueue {
public:
    // Constructor initializes the head and tail pointers
    WaitFreeSPSCQueue() : head(0), tail(0) {}

    // Push method for adding an element to the queue
    bool push(const T& value) {
        // Load the current head pointer
        size_t current_head = head.load(std::memory_order_relaxed);
        // Calculate the next head pointer
        size_t next_head = increment(current_head);
        // Check if the queue is full
        if (next_head == tail.load(std::memory_order_acquire)) {
            return false;
        }
        // Store the value at the current head position
        buffer[current_head] = value;
        // Update the head pointer
        head.store(next_head, std::memory_order_release);
        return true;
    }

    // Pop method for removing an element from the queue
    bool pop(T& value) {
        // Load the current tail pointer
        size_t current_tail = tail.load(std::memory_order_relaxed);
        // Check if the queue is empty
        if (current_tail == head.load(std::memory_order_acquire)) {
            return false;
        }
        // Retrieve the value at the current tail position
        value = buffer[current_tail];
        // Update the tail pointer
        tail.store(increment(current_tail), std::memory_order_release);
        return true;
    }

private:
    // Helper function to increment an index, wrapping around when necessary
    size_t increment(size_t index) const {
        return (index + 1) % N;
    }

    T buffer[N]; // Fixed-size buffer for the queue elements
    std::atomic<size_t> head; // Atomic head pointer
    std::atomic<size_t> tail; // Atomic tail pointer
};

// Main function demonstrating the usage of the wait-free SPSC queue
int main() {
    WaitFreeSPSCQueue<int, 1024> queue;
    std::thread producer([&queue]() {
        for (int i = 0; i < 100; ++i) {
            while (!queue.push(i));
        }
    });

    std::thread consumer([&queue]() {
        int value;
        for (int i = 0; i < 100; ++i) {
            while (!queue.pop(value));
            std::cout << "Popped: " << value << std::endl;
        }
    });

    producer.join();
    consumer.join();

    return 0;
}
