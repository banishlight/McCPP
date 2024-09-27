#pragma once
#include <vector>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <iomanip>
#include <ios>

#include <Standards.hpp>
    class VarInt; // cheap hack to get around including header files


    class MCString;

    class DataBuffer {
    private:
        typedef std::vector<Byte> Data;
        Data buffer;
        std::size_t readOffset = 0;

    public:
        typedef Data::iterator iterator;
        typedef Data::const_iterator constIterator;
        typedef Data::reference reference;
        typedef Data::const_reference constReference;

        DataBuffer();
        DataBuffer(const DataBuffer& other);
        DataBuffer(const DataBuffer& other, std::size_t offset);
        DataBuffer(DataBuffer&& other);
        DataBuffer(const std::string& str);
        std::size_t GetReadOffset() { return readOffset; }
        DataBuffer& operator=(const DataBuffer& other);
        DataBuffer& operator=(DataBuffer&& other);

        template <typename T>
        void Append(T data) {
            std::size_t size = sizeof(data);
            std::size_t end_pos = buffer.size();
            buffer.resize(buffer.size() + size);
            memcpy(&buffer[end_pos], &data, size);
        }

        template <typename T>
        DataBuffer& operator<<(T data) {
            // Switch to big endian
            std::reverse((Byte*)&data, (Byte*)&data + sizeof(T));
            Append(data);
            return *this;
        }

        DataBuffer& operator<<(std::string data) {
            buffer.insert(buffer.end(), data.begin(), data.end());
            return *this;
        }

        DataBuffer& operator<<(DataBuffer& data) {
            buffer.insert(buffer.end(), data.begin(), data.end());
            return *this;
        }

        DataBuffer& operator<<(const DataBuffer& data) {
            buffer.insert(buffer.end(), data.begin(), data.end());
            return *this;
        }

        template <typename T>
        DataBuffer& operator>>(T& data) {
            if (readOffset + sizeof(T) <= GetSize()) {
                data = *(T*)&buffer[readOffset];
                std::reverse((Byte*)&data, (Byte*)&data + sizeof(T));
                readOffset += sizeof(T);
                return *this;
            }

        }

        DataBuffer& operator>>(DataBuffer& data) {
            data.Resize(GetSize() - readOffset);
            std::copy(buffer.begin() + readOffset, buffer.end(), data.begin());
            readOffset = buffer.size();
            return *this;
        }

        DataBuffer& operator>>(std::string& data) {
            data.resize(GetSize() - readOffset);
            std::copy(buffer.begin() + readOffset, buffer.end(), data.begin());
            readOffset = buffer.size();
            return *this;
        }

        void ReadSome(char* strbuffer, std::size_t amount) {
            if (readOffset + amount <= GetSize()) {
                std::copy_n(buffer.begin() + readOffset, amount, strbuffer);
                readOffset += amount;
            }
        }

        void ReadSome(Byte* byteBuffer, std::size_t amount) {
            if (readOffset + amount <= GetSize()) {
                std::copy_n(buffer.begin() + readOffset, amount, byteBuffer);
                readOffset += amount;
            }
        }

        void ReadSome(DataBuffer& buffer, std::size_t amount) {
            if (readOffset + amount <= GetSize()) {
                buffer.Resize(amount);
                std::copy_n(buffer.begin() + readOffset, amount, buffer.begin());
                readOffset += amount;
            }
        }

        void ReadSome(std::string& buffer, std::size_t amount) {
            if (readOffset + amount <= GetSize()) {
                buffer.resize(amount);
                std::copy_n(buffer.begin() + readOffset, amount, buffer.begin());
                readOffset += amount;
            }
        }

        void Resize(std::size_t size) {
            buffer.resize(size);
        }

        void Reserve(std::size_t amount) {
            buffer.reserve(amount);
        }

        void erase(iterator it) {
            buffer.erase(it);
        }

        void Clear() {
            buffer.clear();
            readOffset = 0;
        }

        bool IsFinished() const {
            return readOffset >= buffer.size();
        }

        std::size_t GetReadOffset() const { return readOffset; }
        void SetReadOffset(std::size_t pos);

        std::string ToString() const;
        std::size_t GetSize() const;
        bool IsEmpty() const;
        std::size_t GetRemaining() const;

        iterator begin();
        iterator end();
        constIterator begin() const;
        constIterator end() const;

        reference operator[](Data::size_type i) { return buffer[i]; }
        constReference operator[](Data::size_type i) const { return buffer[i]; }


    };


