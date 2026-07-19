#include <Standards.hpp>
#include <network/Position.hpp>

Int64 EncodePosition(const Int32 x, const Int32 y, const Int32 z)
{
	//ripped from the wiki
	return ((static_cast<Int64>(x & 0x3FFFFFF) << static_cast<Int64>(38)) | ((static_cast<Int64>(z & 0x3FFFFFF)) << 12) | (y & 0xFFF));
}

// Inverse of EncodePosition (docs/network-protocol.md, "Position"). Relies on
// right shift of a signed Int64 being an arithmetic (sign-extending) shift --
// true for GCC/x86-64, the project's only target.
DecodedPosition DecodePosition(const Int64 compressedPosition)
{
	DecodedPosition pos;
	pos.x = static_cast<Int32>(compressedPosition >> 38);
	pos.y = static_cast<Int32>((compressedPosition << 52) >> 52);
	pos.z = static_cast<Int32>((compressedPosition << 26) >> 38);
	return pos;
}