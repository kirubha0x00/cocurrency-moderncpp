#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>

template<typename T>
class LockFreeQueue {
public:
    struct Node {
        T data;
        std::atomic<Node*> next;
    };

    LockFreeQueue() : head(new Node), tail(head.load()) {}
    ~LockFreeQueue() {
        while (Node* old_head = head.load()) {
            head.store(old_head->next);
            delete old_head;
        }
    }

    void enqueue(T value) {
        Node* new_node = new Node{value, nullptr};
        Node* old_tail = tail.load();
        Node* null_node = nullptr;

        while (!old_tail->next.compare_exchange_weak(null_node, new_node)) {
            old_tail = tail.load();
            null_node = nullptr;
        }

        tail.compare_exchange_weak(old_tail, new_node);
    }

    bool dequeue(T& value) {
        Node* old_head = head.load();
        Node* next_node = old_head->next.load();

        while (next_node && !head.compare_exchange_weak(old_head, next_node)) {
            next_node = old_head->next.load();
        }

        if (next_node) {
            value = next_node->data;
            delete old_head;
            return true;
        }

        return false;
    }

private:
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
};

class ThreadPool {
public:
    ThreadPool(size_t num_threads) : done(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this]() {
                while (!done.load()) {
                    std::function<void()> task;
                    if (task_queue.dequeue(task)) {
                        task();
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        done.store(true);
        for (auto& worker : workers) {
            worker.join();
        }
    }

    void submit(std::function<void()> task) {
        task_queue.enqueue(task);
    }

private:
    std::vector<std::thread> workers;
    LockFreeQueue<std::function<void()>> task_queue;
    std::atomic<bool> done;
};

int main() {
    ThreadPool pool(4);

    for (int i = 0; i < 10; ++i) {
        pool.submit([i]() {
            std::cout << "Task " << i << " running on thread " << std::this_thread::get_id() << std::endl;
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
