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

long long ByteBuffer::get_index(long long p)
{
    return p % max_capacity;
}

bool ByteBuffer::has_wrap_around()
{
    long long quotient_w = write_p/max_capacity;
    long long quotient_r = read_p/max_capacity;
    return quotient_w>quotient_r && read_p > (quotient_r*max_capacity);
    
}


long long ByteBuffer::get_total_capacity()
{
    return max_capacity- get_total_remaining();
}

long long ByteBuffer::get_total_remaining()
{
    return write_p-read_p;
}


void ByteBuffer::increment_read_pointer(long long size)
{
    if(size>get_total_capacity())
    {
        throw std::runtime_error("ByteBuffer::increment_read_pointer error: More than capacity");

    }
    write_p+=size;
}

void ByteBuffer::read(std::byte* dest, long long size)
{

}

void ByteBuffer::write(std::byte* srcdest, long long)
{

}

void ByteBuffer::increment_read_pointer(long long size )
{

}

IOChunk ByteBuffer::get_write_chunk()
{

}

IOChunk ByteBuffer::get_read_chunk()
{
    
}