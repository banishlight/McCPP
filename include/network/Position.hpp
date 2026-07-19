#pragma once
#include <Standards.hpp>

struct DecodedPosition {
    Int32 x;
    Int32 y;
    Int32 z;
};

Int64 EncodePosition(const Int32 x, const Int32 y, const Int32 z);
DecodedPosition DecodePosition(const Int64 compressedPosition);