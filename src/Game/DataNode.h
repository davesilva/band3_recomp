#pragma once

#include <rex/types.h>
#include <cstdint>
#include "DataArray.h"
#include "src/hook_helpers.h"

namespace band3 {

class DataNode;
class DataArray;
class DataArrayPtr;
namespace Hmx {
    class Object;
}

/** A function which can be called by a script/command. */
typedef DataNode DataFunc(DataArray *);

/** The possible DataNode types. */
enum DataType : u32 {
    kDataInt = 0,
    kDataFloat = 1,
    kDataVar = 2,
    kDataFunc = 3,
    kDataObject = 4,
    kDataSymbol = 5,
    kDataUnhandled = 6,
    kDataIfdef = 7,
    kDataElse = 8,
    kDataEndif = 9,
    kDataArray = 16,
    kDataCommand = 17,
    kDataString = 18,
    kDataProperty = 19,
    kDataGlob = 20,
    kDataDefine = 32,
    kDataInclude = 33,
    kDataMerge = 34,
    kDataIfndef = 35,
    kDataAutorun = 36,
    kDataUndef = 37,
};

/** A value which can be configured and accessed by scripts. */
class DataNode {
public:
    /** The possible data types that a DataNode can have. */
    union {
        GuestAddr<const char> symbol;
        be_i32 integer;
        be_f32 real;
        GuestAddr<DataArray> array;
        GuestAddr<DataNode> var;
        GuestAddr<DataFunc> func;
        GuestAddr<Hmx::Object> object;
    } mValue;
    rex::be<DataType> mType;

    DataType Type() const { return mType; }

    /** Is this node's type compatible with the supplied DataType?
     * @param [in] t The DataType in question
     * @returns True if the two types are compatible, false if not.
     */
    // bool CompatibleType(DataType t) const;

    /** Evaluate the contents of this DataNode and return the result.
     * @returns A DataNode with the results of the evaluation.
     */
    const DataNode &Evaluate() const;

    /** Evalute this DataNode, and return the resulting int.
     * @returns The resulting int from this DataNode.
     */
    i32 Int() const;

    /** Return the int directly inside of this DataNode.
     * @returns The aforementioned int.
     */
    // i32 LiteralInt() const;

    /** Evalute this DataNode, and return the resulting Symbol.
     * @returns The resulting Symbol from this DataNode.
     */
    const char* Sym() const;  // should be Symbol Sym() const;

    /** Return the Symbol directly inside of this DataNode.
     * @returns The aforementioned Symbol.
     */
    // Symbol LiteralSym() const;
    const char* ForceSym() const;  // should be Symbol ForceSym() const;

    /** Evalute this DataNode, and return the resulting string.
     * @returns The resulting string from this DataNode.
     */
    const char *Str() const;

    /** Return the string directly inside of this DataNode.
     * @returns The aforementioned string.
     */
    const char *LiteralStr() const;

    /** Evalute this DataNode, and return the resulting float.
     * @returns The resulting float from this DataNode.
     */
    f32 Float() const;

    /** Return the float directly inside of this DataNode.
      @returns The aforementioned float.
     */
    // f32 LiteralFloat() const;

    /** Return the DataFunc directly inside of this DataNode.
     * @returns The aforementioned DataFunc.
     */
    // DataFunc *Func() const;

    /** Evalute this DataNode, and return the resulting Hmx::Object.
     * @returns The resulting Hmx::Object from this DataNode.
     */
    Hmx::Object *GetObj() const;

    /** Evalute this DataNode, and return the resulting DataArray.
     * @returns The resulting DataArray from this DataNode.
     */
    // DataArray *Array() const;

    /** Return the DataArray directly inside of this DataNode.
     * @returns The aforementioned DataArray.
     */
    // DataArray *LiteralArray() const;

    /** Return the command DataArray directly inside of this DataNode.
     * @returns The aforementioned DataArray.
     */
    // DataArray *Command() const;

    /** Return the var DataNode directly inside of this DataNode.
     * @returns The aforementioned DataNode.
     */
    DataNode *Var() const;

    /** Get the Hmx::Object derivative resulting from this DataNode.
     * @returns The aforementioned Hmx::Object derivative.
     */
    // template <class T>
    // T *Obj() const {
    //     return dynamic_cast<T *>(GetObj());
    // }

    // bool operator==(const DataNode &n) const;
    // bool operator!=(const DataNode &n) const;
    bool NotNull() const;
    // bool operator!() const { return !NotNull(); }
    // DataNode &operator=(const DataNode &n);

    /** Print the DataNode's contents to the TextStream.
     * @param [in] s The TextStream to print to.
     * @param [in] compact If true, print any strings in a compact manner.
     */
    // void Print(TextStream &s, bool compact) const;
    /** Saves this DataNode into a BinStream. */
    // void Save(BinStream &d) const;
    /** Loads this DataNode from a BinStream. */
    // void Load(BinStream &d);
};

static_assert_size(DataNode, 8);
static_assert(offsetof(DataNode, mValue) == 0);
static_assert(offsetof(DataNode, mType) == 4);

}
