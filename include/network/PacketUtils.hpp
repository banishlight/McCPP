#pragma once
#include <Standards.hpp>
#include <vector>

std::vector<Byte> varIntSerialize(int num);
int varIntDeserialize(std::vector<Byte> buff);

std::vector<Byte> varLongSerialize(long num);
long varLongDeserialize(std::vector<Byte> buff);

int getVarIntSize(int num);
int getVarLongSize(long num);

// All of the deserialize methods below modify the given byte array of data to 
// automate iteration through the array
string deserializeString(std::vector<Byte>& data);
std::vector<Byte> serializeString(const string& str);

std::vector<Byte> serializePrefixedArray(const std::vector<Byte>& data);

std::vector<Byte> generateVerifyToken();

std::vector<long> deserializeUUID(std::vector<Byte>& data);
std::vector<Byte> serializeUUID(const std::vector<long>& data);

std::vector<Byte> compressedPacket(int id, const std::vector<Byte>& data);
std::vector<Byte> assemblePacket(int id, int threshold, const std::vector<Byte>& data);