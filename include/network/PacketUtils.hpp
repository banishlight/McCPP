#pragma once
#include <Standards.hpp>
#include <vector>

std::vector<Byte> varIntSerialize(int num);
int varIntDeserialize(std::vector<Byte> buff);

std::vector<Byte> varLongSerialize(long num);
long varLongDeserialize(std::vector<Byte> buff);

int getVarIntSize(int num);
int getVarLongSize(long num);