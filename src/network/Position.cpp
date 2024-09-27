#include <Standards.hpp>
#include <network/Position.hpp>
#include <vector>

Position::Position(Int32 posX =0 , Int32 posY = 0, Int32 posZ = 0)
{
	this->posX = posX;
	this->posY = posY;
	this->posZ = posZ;
}

std::vector<Int32> Position::DecodePosition(Int64 compressedPosition)
{
	std::vector<Int32> returnVec;
	returnVec.push_back(posX);
	returnVec.push_back(posY);
	returnVec.push_back(posZ);
	return returnVec;
}
Int64 EncodePosition(const Int32 x, const Int32 y, const Int32 z)
{
	//ripped from the wiki
	return ((static_cast<Int64>(x & 0x3FFFFFF) << static_cast<Int64>(38)) | ((static_cast<Int64>(z & 0x3FFFFFF)) << 12) | (y & 0xFFF));
}


