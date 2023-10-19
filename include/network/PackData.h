#pragma once
#include <cstring>
#include "VarIntLong.h"
#include "../../include/Standards.h"

void PackBytesIntoData(UInt64 dataByte, UInt8** data, UInt32* dataptr);
void PackStringIntoData(const char** ip, UInt32 serverAddrLength, UInt8** data, UInt32* dataptr);
void PackVarIntIntoData(VarInt vi, UInt8** data, UInt32* dataptr);
void PackVarLongIntoData(VarLong vl, UInt8** data, UInt32* dataptr);
UInt8 BytesUsed(Int64 numb);

