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
    return quotient_w>quotient_r ;//&& read_p > (quotient_r*max_capacity);
    
}

IOChunk ByteBuffer::get_write_chunk()
{
    long long write_index = get_index(write_p);
    if(has_wrap_around())
    {
        return {data+write_index , get_index(read_p)-write_index};
    }
    return {data+write_index, max_capacity-write_index};
}

IOChunk ByteBuffer::get_read_chunk()
{
    long long read_index = get_index(read_p);
    if(!has_wrap_around())
    {
        return {data+read_index, write_p-read_p};
    }
    return {data+read_index, max_capacity-read_index};
}

void ByteBuffer::increment_read_pointer(long long size )
{
    if(size>get_total_remaining())
    {
        throw std::runtime_error("ByteBuffer::increment_read_pointer error: More than remaining");
    }
    read_p+=size;
    if(read_p>max_capacity && write_p>max_capacity)
    {
        write_p-=max_capacity;
        read_p-=max_capacity;

    }
}


void ByteBuffer::increment_write_pointer(long long size)
{
    if(size>get_total_capacity())
    {
        throw std::runtime_error("ByteBuffer::increment_write_pointer error: More than capacity");

    }
    write_p+=size;
}

void ByteBuffer::read(std::byte* dest, long long size)
{
    if(size>get_total_remaining())
    {
        throw std::runtime_error("ByteBuffer::read error: Read size more than remaining");

    }
    
    while (size>0)
    {
        IOChunk chunk = get_read_chunk();
        long long read_size = std::min(size, chunk.size);
        std::memcpy(dest,chunk.data, read_size);
        size-=read_size;
        dest+=read_size;
        increment_read_pointer(read_size);

    }
    
}

void ByteBuffer::write(const std::byte* src, long long size)
{
    if(size>get_total_capacity())
    {
        throw std::runtime_error("ByteBuffer::write error: Write size more than capacity");
    }
    while(size>0)
    {
        IOChunk chunk = get_write_chunk();
        long long write_size = std::min(size, chunk.size);
        std::memcpy(chunk.data,src,write_size);
        size-=write_size;
        src+=write_size;
    increment_write_pointer(write_size);
    }
}


long long ByteBuffer::get_total_capacity()
{
    return max_capacity- get_total_remaining();
}

long long ByteBuffer::get_total_remaining()
{
    return write_p-read_p;
}
