#include "../../include/network/VarIntLong.h"

#include <unistd.h>

#include "../../include/Standards.h"
UInt32 VarInt::EncodedLength(UInt32 index){
	UInt32 result = 0;
	while (this->varIntBuf[index + result] > CONTINUE_BIT) {
		result++;
	}
	result++;
	if (index + result > 5) {
		throw std::exception();
	}
	return result;
}

Int32 VarInt::ReadVarInt(Int32* sock, Int32* readVal) {
	//Unpacks a varInt from the datastream into a 32bit integer

		Int32 unpackedVarint = 0;
		Byte tmp = 0x80;
		Byte i = 0;

		while (tmp & 0x80) {
			*readVal = read(*sock, &tmp, 1);
			unpackedVarint |= (tmp & 0x7F) << (7 * i);
			i++;
		}
		return unpackedVarint;
	
}
void VarInt::WriteVarInt(Int32 value, Int32*sock, Int32* writeVal) {
	//writes a 32 bit integer to the datastream
	int pos = 0;
	while ((value & -128) !=0)
	{
		this->varIntBuf[pos++] = ((value & 127) | 128);
		value = (UInt32)value >>  7; //Why the fuck does this work?
		this->varIntSize++;
	}
	this->varIntBuf[pos++] = (Byte)value;
	this->varIntSize++;
	for(int i=0;i<varIntSize;i++)
	{

	}
 }



Int64 VarLong::ReadVarLong() {
	//Takes a VarLongBuffer that is already loaded and return a signed long
	int64_t result = 0;
	int32_t count = 0;
	unsigned char byte;
	do
	{
		if (count >= 10) return 0;

		byte = this->varIntBuf[count];

		int64_t value = (byte & SEGMENT_BITS);
		result |= (value << (7 * count));
		count++;
	

	} while ((byte & CONTINUE_BIT) != 0);
	return result;
	
}
void VarLong::WriteVarLong(Int64 value)
{
	

	Int32 count = 0;
	Int64 i = value;

	do
	{
		Byte temp = (i & SEGMENT_BITS);
		i = (UInt64)i  >> 7;
		if (i != 0) temp |= CONTINUE_BIT;
		this->varIntBuf[count] = temp;
		count++;
		this->varIntSize++;
	} while (i != 0);

}


