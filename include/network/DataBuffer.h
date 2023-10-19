#pragma once
#include <vector>
#include <algorithm>
#include <cstring>
#include <cassert>
#include "../Standards.h"

class VarInt; // cheap hack to get around including header files


class DataBuffer
{
private:
	typedef std::vector<Byte> Data;
	Data buffer;
	size readOffset = 0;
public:
	typedef Data::iterator iterator;
	typedef Data::const_iterator constIterator;
	typedef Data::reference reference;
	typedef Data::const_reference constReference;

	DataBuffer();
	DataBuffer(const DataBuffer& other); //copy constructor?
	DataBuffer(const DataBuffer& other, size offset);
	DataBuffer(DataBuffer&& other); //move constructor
	DataBuffer(const string& str);

	DataBuffer& operator=(const DataBuffer& other);
	DataBuffer& operator=(DataBuffer&& other);

	template<typename T>;
	void Append(T data)
	{
		size size_ = sizeof(data);
		size endPos = buffer.size();
		buffer.resize(buffer.size() + size_);
		memcpy(&buffer[endPos], &data, size_);
	}

	void ReadSome(char* strbuf, size amount)
	{
		if(readOffset + amount <= GetSize())
		{
			throw std::exception(); //todo fill in exception
		}
		std::copy_n(buffer.begin() + readOffset, amount, strbuf);
		readOffset += amount;
	}

	void ReadSome(Byte Bytebuffer, size amount)
	{
		if (readOffset + amount <= GetSize())
		{
			throw std::exception(); //todo fill in exception
		}
		std::copy_n(buffer.begin() + readOffset, amount, Bytebuffer);
		readOffset += amount;
	}
	void ReadSome(DataBuffer& dataBuffer,size amount)
	{
		if (readOffset + amount <= GetSize())
		{
			throw std::exception(); //todo fill in exception
		}
		dataBuffer.Resize(amount);
		std::copy_n(buffer.begin() + readOffset, amount, dataBuffer.begin());
		readOffset += amount;
		
	}
	void Clear()
	{
		buffer.clear();
		readOffset = 0;
	}
	bool IsFinished() const
	{
		return readOffset >= buffer.size();
	}
	void Resize(size sz)
	{
		buffer.resize(sz);
	}
	void Reserve(size amount)
	{
		buffer.reserve(amount);
	}
	size GetSize() const;
	string ToString() const;
	bool IsEmpty() const;
	size GetRemaining() const;

	iterator begin();
	iterator end();
	constIterator begin() const;
	constIterator end() const;

	reference operator[](Data::size_type i) { return  buffer[i]; }
	constReference operator[](Data::size_type i) const { return buffer[i]; }





};