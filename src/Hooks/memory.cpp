#include <rex/ppc/context.h>
#include <rex/ppc/function.h>
#include <rex/ppc/memory.h>
#include <cstdint>
#include <cstring>

// functions to avoid using recompiled stdlib memory stuff

static inline void ppc_memcpy_impl(uint8_t* base, uint32_t dst, uint32_t src, uint32_t n) {
    uint8_t* d = PPC_RAW_ADDR(dst);
    const uint8_t* s = PPC_RAW_ADDR(src);
    std::memmove(d, s, n);
}

// void* memcpy(void* dst, const void* src, size_t n)
extern "C" PPC_FUNC(_memcpy)
{
    uint32_t dst = ctx.r3.u32;
    uint32_t src = ctx.r4.u32;
    uint32_t n = ctx.r5.u32;

    if (n > 0) {
        ppc_memcpy_impl(base, dst, src, n);
    }
    ctx.r3.u64 = dst;
}

// void* memmove(void* dst, const void* src, size_t n)
extern "C" PPC_FUNC(_memmove)
{
    uint32_t dst = ctx.r3.u32;
    uint32_t src = ctx.r4.u32;
    uint32_t n = ctx.r5.u32;

    if (n > 0) {
        ppc_memcpy_impl(base, dst, src, n);
    }
    ctx.r3.u64 = dst;
}

// void* memset(void* dst, int c, size_t n)
extern "C" PPC_FUNC(_memset)
{
    uint32_t dst = ctx.r3.u32;
    int c = ctx.r4.s32;
    uint32_t n = ctx.r5.u32;

    if (n > 0) {
        std::memset(PPC_RAW_ADDR(dst), c, n);
    }
    ctx.r3.u64 = dst;
}
