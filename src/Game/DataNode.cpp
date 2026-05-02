#include <cstring>
#include <functional>
#include "DataNode.h"
#include "Symbol.h"
#include "generated/band3_init.h"
#include "src/Game/DataNode.h"
#include "src/Game/DataArray.h"

using namespace band3;

constexpr uint32_t gEvalIndex_addr  = 0x82E05220;
constexpr uint32_t gEvalNode_addr   = 0x82E05240;
constexpr uint32_t gDataDir_addr    = 0x82E05DB0;
const GuestAddr<be_u32> gEvalIndex  = GuestAddr<be_u32>(gEvalIndex_addr);
const GuestAddr<DataNode> gEvalNode = GuestAddr<DataNode>(gEvalNode_addr);
const GuestAddr<be_u32> gDataDir    = GuestAddr<be_u32>(gDataDir_addr);

FUNC_IMPORT(__imp__DataNode__Evaluate, _DataNode__Evaluate_original, DataNode* (const DataNode*))
//FUNC_IMPORT(__imp__Symbol__Symbol, _Symbol__Symbol_original, GuestAddr<const char>* (GuestAddr<const char>*, const char*))
FUNC_IMPORT_RVO(__imp__Symbol__Symbol, _Symbol__Symbol_original, GuestAddr<const char> (const char*))
FUNC_IMPORT(__imp__ObjectDir__FindObject, _ObjectDir__FindObject_original, Hmx::Object* (u32, const char*, bool))
FUNC_IMPORT(__imp__DataNode__UseQueue, _DataNode__UseQueue_original, const DataNode* (const DataNode*))


const DataNode& DataNode::Evaluate() const {
    if (mType == kDataVar) {
        return *mValue.var;
    } else if (mType == kDataCommand || mType == kDataProperty) {
        return *_DataNode__Evaluate_original(this);
    } else {
        return *this;
    }
}
METHOD_HOOK(DataNode__Evaluate, &DataNode::Evaluate)


i32 DataNode::Int() const {
    const DataNode &n = Evaluate();
    return n.mValue.integer;
}
METHOD_HOOK(DataNode___value, &DataNode::Int)


const char* DataNode::Sym() const {
    const DataNode &n = Evaluate();
    return n.mValue.symbol;
}
/*
REX_HOOK(DataNode__Sym, +[](rex::MappedPtr<GuestAddr<const char>> out, rex::MappedPtr<DataNode> self) -> u32 {
    *out = self->Sym();
    return out.guest_address();
});
*/
METHOD_HOOK_RVO(DataNode__Sym, &DataNode::Sym)


const char* DataNode::ForceSym() const {
    const DataNode &n = Evaluate();
    if (n.mType == kDataSymbol) {
        return n.mValue.symbol;
    } else {
        return n.mValue.var->mValue.symbol;
    }
}
REX_HOOK(DataNode__ForceSym, +[](rex::MappedPtr<GuestAddr<const char>> out, rex::MappedPtr<DataNode> self) -> u32 {
    const DataNode &n = self->Evaluate();
    *out = self->ForceSym();
    if (n.mType != kDataSymbol) {
        u32 addr = _Symbol__Symbol_original(out, *out)->guest_address();
        u32 addr2 = out.guest_address();
        if (addr != addr2) {
            REXLOG_ERROR("out addresses don't match");
        }
    }
    return out.guest_address();
});


const char* DataNode::Str() const {
    const DataNode &n = Evaluate();
    if (n.mType == kDataSymbol) {
        return n.mValue.symbol;
    } else {
        return n.mValue.var->mValue.symbol;
    }
}
METHOD_HOOK(DataNode__Str, &DataNode::Str)


const char* DataNode::LiteralStr() const {
    if (mType == kDataSymbol) {
        return mValue.symbol;
    } else {
        return mValue.var->mValue.symbol;
    }
}
METHOD_HOOK(DataNode__LiteralStr, &DataNode::LiteralStr)


f32 DataNode::Float() const {
    const DataNode &n = Evaluate();
    if (n.mType == kDataInt) {
        return n.mValue.integer;
    } else {
        return n.mValue.real;
    }
}
METHOD_HOOK(DataNode__Float, &DataNode::Float)


DataNode* DataNode::Var() const {
    return mValue.var;
}
METHOD_HOOK(DataNode__Var, &DataNode::Var)


Hmx::Object *DataNode::GetObj() const {
    const DataNode &n = Evaluate();
    if (n.mType == kDataObject) {
        return n.mValue.object;
    } else {
        const char *str = n.LiteralStr();
        if (*str != '\0') {
            return _ObjectDir__FindObject_original(*gDataDir, str, true);
        }
        return nullptr;
    }
}
METHOD_HOOK(DataNode__GetObj, &DataNode::GetObj)


bool DataNode::NotNull() const {
    const DataNode &n = Evaluate();
    DataType t = n.mType;
    if (t == kDataSymbol) {
        return n.mValue.symbol[0] != 0;
    } else if (t == kDataString) {
        return n.mValue.array->mSize < -1;
    } else if (t == kDataGlob) {
        return (u16)n.mValue.array->mSize != 0;
    } else {
        return n.mValue.array != nullptr;
    }
}
METHOD_HOOK(DataNode__NotNull, &DataNode::NotNull)


const DataNode *UseQueue(rex::MappedPtr<DataNode> node) {
    i32 i = *gEvalIndex;

    if ((node->mType | gEvalNode[i].mType) & kDataArray) {
        return _DataNode__UseQueue_original(&*node);
    }

    gEvalNode[i] = *node;
    *gEvalIndex = (i + 1) & 7;
    return &gEvalNode[i];
}
REX_HOOK(DataNode__UseQueue, UseQueue)
