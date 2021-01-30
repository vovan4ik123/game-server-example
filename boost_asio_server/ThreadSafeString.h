#pragma once

#include <string>
#include <mutex>

class ThreadSafeString
{
public:

    void setMove(std::string&& str)
    {
        std::scoped_lock lock(mutexString);
        buffer = std::move(str);
    }

    void setCopy(const std::string& str)
    {
        std::scoped_lock lock(mutexString);
        buffer = str;
    }

    std::string getMove()
    {
        std::scoped_lock lock(mutexString);
        std::string s = std::move(buffer);
        return s;
    }

    std::string getCopy()
    {
        std::scoped_lock lock(mutexString);
        return buffer;
    }

    bool empty()
    {
        std::scoped_lock lock(mutexString);
        return buffer.empty();
    }

    size_t size()
    {
        std::scoped_lock lock(mutexString);
        return buffer.size();
    }

    size_t capacity()
    {
        std::scoped_lock lock(mutexString);
        return buffer.capacity();
    }

    void clear()
    {
        std::scoped_lock lock(mutexString);
        buffer.clear();
    }

    void reserve(const size_t size)
    {
        std::scoped_lock lock(mutexString);
        buffer.reserve(size);
    }

    void resize(const size_t size)
    {
        std::scoped_lock lock(mutexString);
        buffer.resize(size);
    }

private:
    std::string buffer;
    std::mutex mutexString;
};