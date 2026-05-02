#pragma once

#include <rex/types.h>
#include "DataNode.h"
#include "Symbol.h"

namespace band3 {

struct DataArray {
    // nodes in array
    rex::be<u32> mNodes;

    // the file this array belongs to
    rex::be<u32> mFile;

    // number of nodes in this array
    rex::be<i16> mSize;

    // number of references
    rex::be<i16> mRefs;

    // the line of the file this array is in
    rex::be<i16> mLine;

    // unused in retail build
    rex::be<i16> mDeprecated;

    i16 Size() const { return mSize; }
};

}
