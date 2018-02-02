#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#define BUFFER_SIZE 20

class RingBuffer
{
public:
    explicit RingBuffer(int filterValue = 10);
    virtual ~RingBuffer();

    void PushDataToBuffer(int data);
    bool IsBufferDataEqual();
    int GetBufferAverageData();
    void Clear();
    void SetBufferFilterValue(int filterValue);

private:
    int m_buffer[BUFFER_SIZE];
    int m_index;

    int m_filterValue;

//    int m_firstBuffer[BUFFER_SIZE];
//    int m_firstIndex;

//    int m_secendBuffer[BUFFER_SIZE];
//    int m_secendBuffer;
};

#endif // RINGBUFFER_H
