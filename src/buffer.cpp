#include "../include/buffer.h"

#include <cstring> 
#include <algorithm>
#include <stdexcept>

ByteBuffer::ByteBuffer(long long max_capacity)
{
    this->data = new std::byte[max_capacity];
    this->read_p = 0;
    this->write_p = 0;
    this->max_capacity = max_capacity;

}

ByteBuffer::~ByteBuffer()
{
    delete[] this->data;
}

long long ByteBuffer::get_total_capacity()
{
    return max_capacity- get_total_remaining();
}

long long ByteBuffer::get_total_remaining()
{
    return write_p-read_p;
}