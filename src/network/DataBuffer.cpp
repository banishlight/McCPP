#include "../../include/Standards.h"
#include "../../include/network/DataBuffer.h"
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstring>
#include <memory>
#include <iostream>
#include <cassert>

    DataBuffer::DataBuffer() { }
    DataBuffer::DataBuffer(const DataBuffer& other) : buffer(other.buffer), readOffset(other.readOffset) { }
    DataBuffer::DataBuffer(DataBuffer&& other) : buffer(std::move(other.buffer)), readOffset(std::move(other.readOffset)) { }
    DataBuffer::DataBuffer(const std::string& str) : buffer(str.begin(), str.end()) { }
    DataBuffer::DataBuffer(const DataBuffer& other, std::size_t offset) {
        buffer.reserve(other.GetSize() - offset);
        std::copy(other.buffer.begin() + offset, other.buffer.end(), std::back_inserter(buffer));
    }

    DataBuffer& DataBuffer::operator=(const DataBuffer& other) {
        buffer = other.buffer;
        readOffset = other.readOffset;
        return *this;
    }

    DataBuffer& DataBuffer::operator=(DataBuffer&& other) {
        buffer = std::move(other.buffer);
        readOffset = std::move(other.readOffset);
        return *this;
    }

    void DataBuffer::SetReadOffset(std::size_t pos) {
        if (pos <= GetSize()) {
            readOffset = pos;
        }
    }

    std::string DataBuffer::ToString() const {
        return std::string(buffer.begin(), buffer.end());
    }

    std::size_t DataBuffer::GetSize() const { return buffer.size(); }
    bool DataBuffer::IsEmpty() const { return buffer.empty(); }
    std::size_t DataBuffer::GetRemaining() const {
        return buffer.size() - readOffset;
    }

    DataBuffer::iterator DataBuffer::begin() {
        return buffer.begin();
    }
    DataBuffer::iterator DataBuffer::end() {
        return buffer.end();
    }
    DataBuffer::constIterator DataBuffer::begin() const {
        return buffer.begin();
    }
    DataBuffer::constIterator DataBuffer::end() const {
        return buffer.end();
    }
 
    std::ostream& operator<<(std::ostream& os, const DataBuffer& buffer) {
        for (unsigned char u : buffer)
            os << std::hex << std::setfill('0') << std::setw(2) << (int)u << " ";
        os << std::dec;
        return os;
    }
;

