#pragma once

#include <vector>
#include <mutex>
#include <type_traits>

template<typename T>
class ThreadSafeVector
{
public:

    void pushBack(const T& elem)
    {
        std::scoped_lock lock(m_vecMutex);
        m_vec.push_back(elem);
    }

    // after scope {} returned reference is not protected by std::scoped_lock !!!
    T& operator[](int i)
    {
        std::scoped_lock lock(m_vecMutex);
        return m_vec[i];
    }

    void setElement(int i, T& elem)
    {
        std::scoped_lock lock(m_vecMutex);
        m_vec[i] = elem;
    }

    bool empty()
    {
        std::scoped_lock lock(m_vecMutex);
        return m_vec.empty();
    }

    size_t size()
    {
        std::scoped_lock lock(m_vecMutex);
        return m_vec.size();
    }

    void clear()
    {
        std::scoped_lock lock(m_vecMutex);
        m_vec.clear();
    }

    void reserve(int i)
    {
        std::scoped_lock lock(m_vecMutex);
        m_vec.reserve(i);
    }

private:
    std::vector<T> m_vec;
    std::mutex m_vecMutex;
};