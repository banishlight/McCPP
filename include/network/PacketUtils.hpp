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

double deserializeDouble(std::vector<Byte>& data);
float deserializeFloat(std::vector<Byte>& data);

int deserializeVarInt(std::vector<Byte>& data);
Int64 deserializeLong(std::vector<Byte>& data);
Int16 deserializeShort(std::vector<Byte>& data);
Byte deserializeByte(std::vector<Byte>& data);
bool deserializeBool(std::vector<Byte>& data);

std::vector<Byte> deserializePrefixedArray(std::vector<Byte>& data);
std::vector<Byte> serializePrefixedArray(const std::vector<Byte>& data);

std::vector<Byte> generateVerifyToken();

std::vector<long> deserializeUUID(std::vector<Byte>& data);
std::vector<Byte> serializeUUID(const std::vector<long>& data);
// Dashless 32-hex-char lowercase form, e.g. for keying OpsList entries -- the
// inverse of Login.cpp's local uuidFromMojangHex.
string uuidToHexString(const std::vector<long>& uuid);
// Standard dashed (8-4-4-4-12) form, e.g. for playerdata filenames and the
// Status Response's player sample "id" field -- vanilla-matching filename/
// display convention, derived from uuidToHexString rather than reimplementing
// hex conversion.
string uuidToDashedHexString(const std::vector<long>& uuid);

// Wire "Angle" type: a full rotation (0-360 degrees) packed into a single
// byte (256 steps/turn). Used by entity rotation fields (Update Entity
// Rotation, Set Head Rotation, Teleport Entity, Spawn Entity's pitch/yaw).
Byte angleSerialize(float degrees);

std::vector<Byte> assemblePacket(int id, int threshold, const std::vector<Byte>& data);