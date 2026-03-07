#include <rex/ppc/context.h>
#include <rex/ppc/function.h>
#include <rex/ppc/memory.h>
#include <cstdint>
#include <cstring>

// functions to avoid using recompiled stdlib string stuff

// int strncmp(const char* s1, const char* s2, size_t n)
extern "C" PPC_FUNC(_strncmp)
{
    uint32_t s1 = ctx.r3.u32;
    uint32_t s2 = ctx.r4.u32;
    uint32_t n = ctx.r5.u32;

    int result = std::strncmp(
        reinterpret_cast<const char*>(PPC_RAW_ADDR(s1)),
        reinterpret_cast<const char*>(PPC_RAW_ADDR(s2)),
        n);
    ctx.r3.s64 = result;
}

// char* strchr(const char* s, int c)
extern "C" PPC_FUNC(_strchr)
{
    uint32_t s = ctx.r3.u32;
    int c = ctx.r4.s32;

    const char* result = std::strchr(
        reinterpret_cast<const char*>(PPC_RAW_ADDR(s)), c);

    ctx.r3.u64 = result ? static_cast<uint32_t>(result - reinterpret_cast<const char*>(base) - PPC_PHYS_HOST_OFFSET(s)) : 0;
}

// char* strrchr(const char* s, int c)
extern "C" PPC_FUNC(_strrchr)
{
    uint32_t s = ctx.r3.u32;
    int c = ctx.r4.s32;

    const char* result = std::strrchr(
        reinterpret_cast<const char*>(PPC_RAW_ADDR(s)), c);

    ctx.r3.u64 = result ? static_cast<uint32_t>(result - reinterpret_cast<const char*>(base) - PPC_PHYS_HOST_OFFSET(s)) : 0;
}

// char* strncpy(char* dst, const char* src, size_t n)
extern "C" PPC_FUNC(_strncpy)
{
    uint32_t dst = ctx.r3.u32;
    uint32_t src = ctx.r4.u32;
    uint32_t n = ctx.r5.u32;

    std::strncpy(
        reinterpret_cast<char*>(PPC_RAW_ADDR(dst)),
        reinterpret_cast<const char*>(PPC_RAW_ADDR(src)),
        n);
    ctx.r3.u64 = dst;
}

// char* strstr(const char* haystack, const char* needle)
extern "C" PPC_FUNC(_strstr)
{
    uint32_t haystack = ctx.r3.u32;
    uint32_t needle = ctx.r4.u32;

    const char* h = reinterpret_cast<const char*>(PPC_RAW_ADDR(haystack));
    const char* result = std::strstr(h,
        reinterpret_cast<const char*>(PPC_RAW_ADDR(needle)));

    ctx.r3.u64 = result ? haystack + static_cast<uint32_t>(result - h) : 0;
}

// char* strtok(char* s, const char* delim)
extern "C" PPC_FUNC(_strtok)
{
    uint32_t s = ctx.r3.u32;
    uint32_t delim_addr = ctx.r4.u32;

    static thread_local uint32_t saved_guest = 0;
    static thread_local char* saved_host = nullptr;

    char* str = nullptr;
    if (s != 0) {
        str = reinterpret_cast<char*>(PPC_RAW_ADDR(s));
        saved_guest = s;
        saved_host = str;
    }

    char* result = std::strtok(str,
        reinterpret_cast<const char*>(PPC_RAW_ADDR(delim_addr)));

    ctx.r3.u64 = result ? saved_guest + static_cast<uint32_t>(result - saved_host) : 0;
}
