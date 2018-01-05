#include "ringbuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RingBuffer::RingBuffer()
{
    memset(m_buffer , 0 , sizeof(m_buffer));
    m_index = 0;
}

RingBuffer::~RingBuffer()
{

}

void RingBuffer::PushDataToBuffer(int data)
{
    m_buffer[m_index] = data;

    m_index = (m_index + 1) % BUFFER_SIZE;
}

bool RingBuffer::IsBufferDataEqual()
{
    for(int i = 0 ; i < BUFFER_SIZE - 1 ; i++)
    {
        if(m_buffer[i] != m_buffer[i+1])
            return false;
    }

    return true;
}

int RingBuffer::GetBufferAverageData()
{
    return m_buffer[m_index];
}

void RingBuffer::Clear()
{
    memset(m_buffer , 0 , sizeof(m_buffer));
    m_index = 0;
}
