#pragma once

#include <cstdint>
#include "src/hook_helpers.h"

#define STR_TO_SYM(str) *reinterpret_cast<Symbol *>(const_cast<char **>(&str))

struct PPCContext;

namespace band3 {

struct Symbol {
    uint32_t guest_addr_ = 0;

    Symbol() = default;
    Symbol(const char* name);
    Symbol(PPCContext& ctx, uint8_t* base, const char* name);

    uint32_t value(uint8_t* base) const;

    uint32_t guest_addr() const { return guest_addr_; }

    operator const char*() const {
        return reinterpret_cast<const char*>(BAND3_MEMORY_BASE() + guest_addr_);
    }
};

static_assert(sizeof(Symbol) == 4);

}
