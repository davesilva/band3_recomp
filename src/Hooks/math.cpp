#include <rex/ppc/context.h>
#include <rex/ppc/function.h>
#include <cmath>

// native replacements for recompiled PPC math functions

extern "C" PPC_FUNC(_cos) {
    ctx.f1.f64 = std::cos(ctx.f1.f64);
}

extern "C" PPC_FUNC(_tan) {
    ctx.f1.f64 = std::tan(ctx.f1.f64);
}

extern "C" PPC_FUNC(_floor) {
    ctx.f1.f64 = std::floor(ctx.f1.f64);
}

extern "C" PPC_FUNC(_fmod) {
    ctx.f1.f64 = std::fmod(ctx.f1.f64, ctx.f2.f64);
}

extern "C" PPC_FUNC(_asin) {
    ctx.f1.f64 = std::asin(ctx.f1.f64);
}

extern "C" PPC_FUNC(_acos) {
    ctx.f1.f64 = std::acos(ctx.f1.f64);
}

extern "C" PPC_FUNC(_atan) {
    ctx.f1.f64 = std::atan(ctx.f1.f64);
}

extern "C" PPC_FUNC(_pow) {
    ctx.f1.f64 = std::pow(ctx.f1.f64, ctx.f2.f64);
}

extern "C" PPC_FUNC(_atan2) {
    ctx.f1.f64 = std::atan2(ctx.f1.f64, ctx.f2.f64);
}
