#include "ringbuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QDebug>

RingBuffer::RingBuffer(int filterValue) : m_filterValue(filterValue)
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
    int min = m_buffer[0];
    int max = m_buffer[0];

    /////////////////////////////////////////////////////
    for(int i = 1 ; i < BUFFER_SIZE ; i++)
    {
        if(m_buffer[i] < min)
            min = m_buffer[i];

        if(m_buffer[i] > max)
            max = m_buffer[i];
    }

    if(m_filterValue == 2)
        qDebug("TTTTTTTTTTTTTTT max - min , %d - %d = %d" , max , min , max-min);

    ///////////////////////////////////////////////////
    if((max - min) < m_filterValue)
        return true;
    else
        return false;
}

int RingBuffer::GetBufferAverageData()
{
    return m_buffer[m_index];

    int av = 0;
    for(int i = 0 ; i < BUFFER_SIZE ; i++)
        av += m_buffer[i];

    av /= BUFFER_SIZE;

    return av;
}

void RingBuffer::Clear()
{
    memset(m_buffer , 0 , sizeof(m_buffer));
    m_index = 0;
}

void RingBuffer::SetBufferFilterValue(int filterValue)
{
    m_filterValue = filterValue;
}
