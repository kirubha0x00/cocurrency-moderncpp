#include <atomic>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

template <typename T>
class LockFreeStack {
public:
    void push(const T& value) {
        Node* new_node = new Node(value);
        do {
            new_node->next = head.load(std::memory_order_relaxed);
        } while (!head.compare_exchange_weak(new_node->next, new_node, std::memory_order_release, std::memory_order_relaxed));
    }

    bool pop(T& value) {
        Node* current_head;
        do {
            current_head = head.load(std::memory_order_relaxed);
            if (!current_head) {
                return false; // Stack is empty
            }
        } while (!head.compare_exchange_weak(current_head, current_head->next, std::memory_order_acquire, std::memory_order_relaxed));
        value = current_head->value;
        delete current_head;
        return true;
    }

private:
    struct Node {
        T value;
        Node* next;

        Node(const T& value) : value(value), next(nullptr) {}
    };

    std::atomic<Node*> head{nullptr};
};

void worker(LockFreeStack<int>& stack) {
    for (int i = 0; i < 1000; ++i) {
        stack.push(i);
    }
}

int main() {
    LockFreeStack<int> stack;
    std::vector<std::thread> threads;

    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker, std::ref(stack));
    }

    for (auto& t : threads) {
        t.join();
    }

    int value;
    int count = 0;
    while (stack.pop(value)) {
        ++count;
    }

    std::cout << "Total count:
