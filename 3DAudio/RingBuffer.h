#pragma once
#include <vector>
#include <atomic>
#include <algorithm>



template <typename T>
class RingBuffer {
private:
    
    size_t m_capacity;
    std::vector<T> m_buffer;
    
    std::atomic<size_t> m_head{ 0 }; 
    uint8_t pad[56]{};
    std::atomic<size_t> m_tail{ 0 }; 
    
public:
    explicit RingBuffer(size_t capacity)
        : m_capacity(capacity), m_buffer(capacity + 1) {}

    size_t AvailableToRead() const {
        size_t head = m_head.load(std::memory_order_acquire);
        size_t tail = m_tail.load(std::memory_order_relaxed);
        if (head >= tail) {
            return head - tail;
        }
        return m_capacity + 1 - (tail - head);
    }

    size_t AvailableToWrite() const {
        return m_capacity - AvailableToRead();
    }

    size_t Write(const T* data, size_t count) {
        size_t head = m_head.load(std::memory_order_relaxed);
        size_t tail = m_tail.load(std::memory_order_acquire);

        size_t freeSpace = (head >= tail) ? (m_capacity + 1 - (head - tail) - 1)
                                          : (tail - head - 1);
        
        size_t toWrite = (std::min)(count, freeSpace);
        if (toWrite == 0) return 0;

        size_t firstChunk = (std::min)(toWrite, m_capacity + 1 - head);
        std::copy(data, data + firstChunk, m_buffer.data() + head);

        size_t secondChunk = toWrite - firstChunk;
        if (secondChunk > 0) {
            std::copy(data + firstChunk, data + toWrite, m_buffer.data());
        }

        m_head.store((head + toWrite) % (m_capacity + 1), std::memory_order_release);
        return toWrite;
    }

    size_t Read(T* destination, size_t count) {
        size_t head = m_head.load(std::memory_order_acquire);
        size_t tail = m_tail.load(std::memory_order_relaxed);

        size_t available = (head >= tail) ? (head - tail) 
                                          : (m_capacity + 1 - (tail - head));

        size_t toRead = (std::min)(count, available);
        if (toRead == 0) return 0;

        size_t firstChunk = (std::min)(toRead, m_capacity + 1 - tail);
        std::copy(m_buffer.data() + tail, m_buffer.data() + tail + firstChunk, destination);

        size_t secondChunk = toRead - firstChunk;
        if (secondChunk > 0) {
            std::copy(m_buffer.data(), m_buffer.data() + secondChunk, destination + firstChunk);
        }

        m_tail.store((tail + toRead) % (m_capacity + 1), std::memory_order_release);
        return toRead;
    }
};