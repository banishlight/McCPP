#pragma once
#include <vector>
#include <Standards.hpp>
class Position
{
public:
	Position(Int32 posX, Int32 posY, Int32 posZ);
	std::vector<Int32> DecodePosition(Int64 compressedPosition);
	~Position() = default;
private:
	Int32 posX = 0;
	Int32 posY = 0;
	Int32 posZ = 0;
};

Int64 EncodePosition(const Int32 x, const Int32 y, const Int32 z);
