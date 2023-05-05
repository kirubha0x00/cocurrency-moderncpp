#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <semaphore>

std::mutex mtx;
std::counting_semaphore<1> sem; // binary semaphore

void mutex_acquire(int id) {
    mtx.lock();
    std::cout << "Thread " << id << " locked the mutex." << std::endl;
}

void mutex_release(int id) {
    std::cout << "Thread " << id << " attempting to unlock the mutex." << std::endl;
    mtx.unlock();
}

void semaphore_acquire(int id) {
    sem.acquire();
    std::cout << "Thread " << id << " acquired the semaphore." << std::endl;
}

void semaphore_release(int id) {
    std::cout << "Thread " << id << " released the semaphore." << std::endl;
    sem.release();
}

int main() {
    std::thread t1(mutex_acquire, 1);
    t1.join();
    std::thread t2(mutex_release, 2); // This will cause undefined behavior or a runtime error
    t2.join();

    std::thread t3(semaphore_acquire, 1);
    t3.join();
    std::thread t4(semaphore_release, 2); // This will work with a semaphore
    t4.join();

    return 0;
}
