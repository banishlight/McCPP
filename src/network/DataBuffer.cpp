#include "../../include/Standards.h"
#include "../../include/network/DataBuffer.h"
#include <iomanip>

DataBuffer::DataBuffer() = default;
DataBuffer::DataBuffer(const DataBuffer & other) = default;
DataBuffer::DataBuffer(DataBuffer && other) : buffer(std::move(other.buffer)), readOffset((other.readOffset)) { }
DataBuffer::DataBuffer(const std::string & str) : buffer(str.begin(), str.end()) { }
DataBuffer::DataBuffer(const DataBuffer & other, std::size_t offset) {
    buffer.reserve(other.GetSize() - offset);
    std::copy(other.buffer.begin() + offset, other.buffer.end(), std::back_inserter(buffer));
}

DataBuffer& DataBuffer::operator=(const DataBuffer& other) = default;
DataBuffer& DataBuffer::operator=(DataBuffer&& other) {
    buffer = std::move(other.buffer);
    readOffset = (other.readOffset);
    return *this;
}
string DataBuffer::ToString() const {
    return string(buffer.begin(),buffer.end());
}

size DataBuffer::GetSize() const { return buffer.size(); }
bool DataBuffer::IsEmpty() const { return buffer.empty(); }
size DataBuffer::GetRemaining() const {
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