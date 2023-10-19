#include "../../include/Standards.h"
#include "../../include/network/PackData.h"

void PackBytesIntoData(UInt64 dataByte, UInt8** data, UInt32 *dataptr)
{
	for (int64_t i = BytesUsed(dataByte) - 1; i >= 0; i--) {

		*(*data + *dataptr) = (dataByte >> (i * 8)) & (0xFF); //Selects the right byte to put in data
		(*dataptr)++;
	}

}

void PackStringIntoData(const char** ip, UInt32 serverAddrLength, UInt8** data, UInt32 *dataptr)
{
	for (uint32_t i = 0; i < serverAddrLength; i++) {
		*(*data + *dataptr) = *(*ip + i);
		(*dataptr)++;
	}
}

void PackVarIntIntoData(VarInt vi, UInt8** data, UInt32* dataptr)
{
	int j = 0;
	for (int i = vi.varIntSize -1; i >= 0; i--)
	{
		*(*data + (*dataptr)) = (vi.varIntBuf[j] >> (i * 8)) & (0xFF); //Somehow chooses the right byte to insert the data into
		(*dataptr)++;
		j++;
	}
}

void PackVarLongIntoData(VarLong vl, UInt8** data, UInt32* dataptr)
{
	int j = 0;
	for (int i = vl.varIntSize - 1; i >= 0; i--)
	{
		*(*data + (*dataptr)) = (vl.varIntBuf[j] >> (i * 8)) & (0xFF); //Somehow chooses the right byte to insert the data into
		(*dataptr)++;
		j++;
	}
}


UInt8 BytesUsed(Int64 numb)
{
	if (numb == 0)
	{
		return 1;
	}
	for (UInt8 i = 4; i > 0; i--)
	{
		if(numb & 0xFF000000)
		{
			return i;
		}
		else
		{
			numb <<= 8;
		}
	}
}

