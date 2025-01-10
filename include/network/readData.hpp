#pragma once
#include <Standards.hpp>

int readData(void*& data, Int8& dest, int size = 0);
int readData(void*& data, Int16& dest, int size = 0);
int readData(void*& data, Int32& dest, int size = 0);
int readData(void*& data, Int64& dest, int size = 0);
int readData(void*& data, UInt8& dest, int size = 0);
int readData(void*& data, UInt16& dest, int size = 0);
int readData(void*& data, UInt32& dest, int size = 0);
int readData(void*& data, UInt64& dest, int size = 0);
int readData(void*& data, string& dest, int size = 0);
