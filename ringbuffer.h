#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#define BUFFER_SIZE 10

class RingBuffer
{
public:
    explicit RingBuffer();
    virtual ~RingBuffer();

    void PushDataToBuffer(int data);
    bool IsBufferDataEqual();
    int GetBufferAverageData();
    void Clear();

private:
    int m_buffer[BUFFER_SIZE];
    int m_index;
};

#endif // RINGBUFFER_H
