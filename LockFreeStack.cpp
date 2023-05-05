#include <atomic>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

template <typename T>
class LockFreeStack {
public:
    // Push a value onto the stack
    void push(const T& value) {
        Node* new_node = new Node(value); // Create a new node
        // Try to set the new node's next pointer to the current head
        do {
            new_node->next = head.load(std::memory_order_relaxed);
        // Attempt to update the head pointer to the new node
        } while (!head.compare_exchange_weak(new_node->next, new_node, std::memory_order_release, std::memory_order_relaxed));
    }

    // Pop a value from the stack
    bool pop(T& value) {
        Node* current_head;
        // Try to load the current head
        do {
            current_head = head.load(std::memory_order_relaxed);
            if (!current_head) {
                return false; // Stack is empty
            }
        // Attempt to update the head pointer to the next node
        } while (!head.compare_exchange_weak(current_head, current_head->next, std::memory_order_acquire, std::memory_order_relaxed));
        value = current_head->value; // Retrieve the value from the current head
        delete current_head; // Delete the popped node
        return true;
    }

private:
    // Node structure to store the value and the next node pointer
    struct Node {
        T value;
        Node* next;

        Node(const T& value) : value(value), next(nullptr) {}
    };

    std::atomic<Node*> head{nullptr}; // Atomic head pointer for the stack
};

// Worker function that pushes values onto the stack
void worker(LockFreeStack<int>& stack) {
    for (int i = 0; i < 1000; ++i) {
        stack.push(i);
    }
}

int main() {
    LockFreeStack<int> stack;
    std::vector<std::thread> threads;

    // Create multiple worker threads
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker, std::ref(stack));
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    int value;
    int count = 0;
    // Pop all values from the stack and count them
    while (stack.pop(value)) {
        ++count;
    }

    std::cout << "Total count: " << count << std::endl;

    return 0;
}
