#pragma once
#include <Standards.hpp>

string readString(void* packet, int max);

template <typename T>
T readData(void*& data);