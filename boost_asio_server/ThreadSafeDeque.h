#pragma once

#include <string>
#include <mutex>
#include <deque>
#include <condition_variable>

class ThreadSafeDeque
{
public:
    ~ThreadSafeDeque()
    {
        std::unique_lock<std::mutex> uniqLock(mutexBlocking);
        condVar.notify_all(); // release all waiting threads
    }

    std::string popFront()
    {
        std::scoped_lock lock(mutexDeque);
        std::string s = std::move(messages.front());
        messages.pop_front();
        return s;
    }

    void pushBack(std::string& str)
    {
        std::scoped_lock lock(mutexDeque);
        messages.emplace_back(std::move(str));

        // If any threads waiting on same conditional variable notify_one() unblocks one of the waiting threads
        std::unique_lock<std::mutex> uniqLock(mutexBlocking);
        condVar.notify_one(); // unblock other thread which called condVar.wait(uniqLock);
    }

    bool empty()
    {
        std::scoped_lock lock(mutexDeque);
        return messages.empty();
    }

    size_t size()
    {
        std::scoped_lock lock(mutexDeque);
        return messages.size();
    }

    void clear()
    {
        std::scoped_lock lock(mutexDeque);
        messages.clear();
    }

    // block thread if empty()
    void waitForMessage()
    {
        while (empty())
        {
            // block thread which call this code, until other thread calls condVar.notify_one();
            // thread which call this code cannot unblock itself
            std::unique_lock<std::mutex> uniqLock(mutexBlocking);
            condVar.wait(uniqLock);
            // condVar.wait(uniqLock) atomically releases the attached mutex, blocks the current thread,
            // and adds it to the list of threads waiting on the current condition variable object
        }
    }

private:
    std::deque<std::string> messages;
    std::mutex mutexDeque;
    std::mutex mutexBlocking;
    std::condition_variable condVar;
};