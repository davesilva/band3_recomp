#pragma once

#include <type_traits>
#include <rex/hook.h>
#include <rex/ppc/stack.h>
#include <rex/ppc/function.h>
#include <rex/system/kernel_state.h>

#define BAND3_MEMORY_BASE() (REX_KERNEL_MEMORY()->virtual_membase())

/**
 * Wraps a big-endian guest address and automatically translates it to a host pointer.
 * This is intended for use in structs which reside in guest memory. For function arguments,
 * prefer ReXGlue's MappedPtr.
 *
 * MappedPtr provides the same guest<->host translation but it contains both the guest address
 * and the host pointer, which means it can't be used in place of a raw be_u32 guest address.
 *
 * ReXGlue also provides TypedGuestPointer for use in structs, but it's just a wrapper around
 * be_u32 with no automatic host pointer translation. This just combines the features of
 * those two types: 4 bytes wide with automatic pointer translation.
 *
 * NOTE: Be careful with null pointer checks. Due to address translation, `GuestAddr(0) != 0`.
 *       Always compare against nullptr, not 0.
 */
template <typename T>
class GuestAddr {
  be_u32 guest_addr_;

 public:
  GuestAddr() : guest_addr_(0) {}
  GuestAddr(be_u32 guest_addr) : guest_addr_(guest_addr) {}
  GuestAddr(std::nullptr_t) : guest_addr_(0) {}
  GuestAddr(T* host_ptr) : guest_addr_(static_cast<be_u32>(reinterpret_cast<const u8*>(host_ptr) - BAND3_MEMORY_BASE())) {}

  rex::MappedPtr<T> mapped_ptr() const {
    auto host_ptr = rex::memory::GuestPtr<T*>(BAND3_MEMORY_BASE(), guest_addr_);
    return rex::MappedPtr<T>(host_ptr, guest_addr_);
  }
  u32 guest_address() const { return static_cast<u32>(guest_addr_); }
  T* host_address() const { return mapped_ptr().host_address(); }

  operator rex::MappedPtr<T>() const { return mapped_ptr(); }
  operator T*() const { return host_address(); }

  T* operator->() const { return host_address(); }
  T& operator*() const { return *host_address(); }

  auto value() const {
    return mapped_ptr().value();
  }

  explicit operator bool() const { return guest_addr_ != 0; }
  explicit operator u32() const { return guest_address(); }

  GuestAddr operator+(std::ptrdiff_t offset) const {
    return GuestAddr(guest_addr_ + static_cast<be_u32>(offset * sizeof(T)));
  }
  GuestAddr operator-(std::ptrdiff_t offset) const {
    return GuestAddr(guest_addr_ - static_cast<be_u32>(offset * sizeof(T)));
  }

  friend bool operator==(const GuestAddr& lhs, std::nullptr_t) { return lhs.guest_addr_ == 0; }
  friend bool operator!=(const GuestAddr& lhs, std::nullptr_t) { return lhs.guest_addr_ != 0; }

  template <typename U>
  U as() const {
    return reinterpret_cast<U>(host_address());
  }

  template <typename U>
  rex::be<U>* as_array() const {
    return reinterpret_cast<rex::be<U>*>(host_address());
  }
};

static_assert_size(GuestAddr<char>, 4);


template <typename S>
struct ImportFunctionWrapper;

/**
 * Wrapper around ReXGlue's ImportFunction for calling recompiled guest functions
 * with correct argument and return types.
 *
 * Works around two limitations of ImportFunction:
 * 1. ImportFunction doesn't support pointer return types
 * 2. When called without an explicit context, ImportFunction looks up the context
 * from ThreadState, but then creates a new PPCContext for the call. This fails if
 * there's another ImportFunction call somewhere later down the call stack, because
 * ThreadState still points to the original context, not the new one. This wrapper
 * works around this by passing the existing context, rather than creating a new one.
 */
template <typename R, typename... Args>
struct ImportFunctionWrapper<R(Args...)> {
  PPCFunc& fn;

  R operator()(PPCContext& ctx, uint8_t* base, Args... args) const {
    if constexpr (std::is_pointer_v<R>) {
      auto imported_fn = rex::ppc::ImportFunction<u32(Args...)>{fn};
      u32 guest_addr = imported_fn(ctx, base, args...);
      return guest_addr ? reinterpret_cast<R>(base + guest_addr) : nullptr;
    } else {
      auto imported_fn = rex::ppc::ImportFunction<R(Args...)>{fn};
      return imported_fn(ctx, base, args...);
    }
  }

  R operator()(rex::CallFrame& frame, uint8_t* base, Args... args) const {
    if constexpr (std::is_pointer_v<R>) {
      auto imported_fn = rex::ppc::ImportFunction<u32(Args...)>{fn};
      u32 guest_addr = imported_fn(frame, base, args...);
      return guest_addr ? reinterpret_cast<R>(base + guest_addr) : nullptr;
    } else {
      auto imported_fn = rex::ppc::ImportFunction<R(Args...)>{fn};
      return imported_fn(frame, base, args...);
    }
  }

  R operator()(Args... args) const {
    auto* ts = rex::runtime::ThreadState::Get();
    PPCContext& ctx = *ts->context();
    uint8_t* base = BAND3_MEMORY_BASE();
    rex::ppc::stack_guard guard(ctx);

    ctx.r1.u32 -= 0x70;

    if constexpr (std::is_pointer_v<R>) {
      auto imported_fn = rex::ppc::ImportFunction<u32(Args...)>{fn};
      u32 guest_addr = imported_fn(ctx, base, args...);
      return guest_addr ? reinterpret_cast<R>(base + guest_addr) : nullptr;
    } else {
      auto imported_fn = rex::ppc::ImportFunction<R(Args...)>{fn};
      return imported_fn(ctx, base, args...);
    }
  }
};

/** Typed callable import of a recompiled function (wrapper around REX_IMPORT)
 *   @param symbol   the exact linker symbol to reference
 *   @param callable the name of the typed callable variable
 *   @param sig      the function signature (e.g. u32(u32, u32))
 */
#define FUNC_IMPORT(symbol, callable, sig)     \
  REX_EXTERN(symbol);                          \
  inline ImportFunctionWrapper<sig> callable { \
    symbol                                     \
  };


template <typename S>
struct ImportFunctionWrapperWithRVO;

template <typename R, typename... Args>
struct ImportFunctionWrapperWithRVO<R(Args...)> {
  PPCFunc& fn;

  R operator()(PPCContext& ctx, uint8_t* base, Args... args) const {
    if constexpr (std::is_pointer_v<R>) {
      auto imported_fn = rex::ppc::ImportFunction<u32(Args...)>{fn};
      u32 guest_addr = imported_fn(ctx, base, args...);
      return guest_addr ? reinterpret_cast<R>(base + guest_addr) : nullptr;
    } else {
      auto imported_fn = rex::ppc::ImportFunction<R(Args...)>{fn};
      return imported_fn(ctx, base, args...);
    }
  }

  R operator()(rex::CallFrame& frame, uint8_t* base, Args... args) const {
    if constexpr (std::is_pointer_v<R>) {
      auto imported_fn = rex::ppc::ImportFunction<u32(Args...)>{fn};
      u32 guest_addr = imported_fn(frame, base, args...);
      return guest_addr ? reinterpret_cast<R>(base + guest_addr) : nullptr;
    } else {
      auto imported_fn = rex::ppc::ImportFunction<R(Args...)>{fn};
      return imported_fn(frame, base, args...);
    }
  }

  R operator()(Args... args) const {
    auto* ts = rex::runtime::ThreadState::Get();
    PPCContext& ctx = *ts->context();
    uint8_t* base = BAND3_MEMORY_BASE();
    rex::ppc::stack_guard guard(ctx);

    ctx.r1.u32 -= 0x70;

    auto imported_fn = rex::ppc::ImportFunctionWrapper<R*(R*, Args...)>{fn};
    // TODO
  }
};

/** Typed callable import of a recompiled function with return value optimization
 *   @param symbol   the exact linker symbol to reference
 *   @param callable the name of the typed callable variable
 *   @param sig      the function signature (e.g. u32(u32, u32))
 */
#define FUNC_IMPORT_RVO(symbol, callable, sig)        \
  REX_EXTERN(symbol);                                 \
  inline ImportFunctionWrapperWithRVO<sig> callable { \
    symbol                                            \
  };



template <typename T>
struct member_traits;

template <typename Return, typename Class, typename... Args>
struct member_traits<Return (Class::*)(Args...)> {
  using class_type = Class;
  using return_type = Return;
  using fn_ptr_type = Return (*)(Class*, Args...);
};

template <typename Return, typename Class, typename... Args>
struct member_traits<Return (Class::*)(Args...) const> {
  using class_type = Class;
  using return_type = Return;
  using fn_ptr_type = Return (*)(Class*, Args...);
};


template <auto Method>
__attribute__((noinline)) void HostToGuestMethod(PPCContext& ctx, uint8_t* base) {
  using traits = member_traits<decltype(Method)>;
  using class_type = typename traits::class_type;
  using return_type = typename traits::return_type;
  using fn_ptr_type = typename traits::fn_ptr_type;

  // nullptr is ok because only the type matters here
  auto args = rex::ppc::function_args(static_cast<fn_ptr_type>(nullptr));
  rex::ppc::_translate_args_to_host<static_cast<fn_ptr_type>(nullptr)>(ctx, base, args);

  if constexpr (std::is_same_v<return_type, void>) {
    std::apply(std::mem_fn(Method), args);
  } else if constexpr (std::is_lvalue_reference_v<return_type>) {
    auto v = &std::apply(std::mem_fn(Method), args);

    // Memory barrier to ensure compiler doesn't reorder
    asm volatile("" ::: "memory");

    if (v != nullptr) {
      ctx.r3.u64 =
          static_cast<uint32_t>(reinterpret_cast<size_t>(v) - reinterpret_cast<size_t>(base));
    } else {
      ctx.r3.u64 = 0;
    }
  } else {
    auto v = std::apply(std::mem_fn(Method), args);

    // Memory barrier to ensure compiler doesn't reorder
    asm volatile("" ::: "memory");

    if constexpr (std::is_pointer<return_type>()) {
      if (v != nullptr) {
        ctx.r3.u64 =
            static_cast<uint32_t>(reinterpret_cast<size_t>(v) - reinterpret_cast<size_t>(base));
      } else {
        ctx.r3.u64 = 0;
      }
    } else if constexpr (rex::ppc::is_precise_v<return_type>) {
      ctx.f1.f64 = v;
    } else {
      ctx.r3.u64 = static_cast<uint64_t>(v);
    }
  }
}

#define METHOD_HOOK(subroutine, method)   \
  extern "C" REX_FUNC(subroutine) {       \
    HostToGuestMethod<method>(ctx, base); \
  }

template <typename Tuple, std::size_t... Is>
auto tuple_tail_impl(Tuple&& t, std::index_sequence<Is...>) {
    return std::make_tuple(std::get<Is + 1>(std::forward<Tuple>(t))...);
}

template <typename... Args>
auto tuple_tail(const std::tuple<Args...>& t) {
    static_assert(sizeof...(Args) > 0, "Cannot take tail of an empty tuple.");
    return tuple_tail_impl(t, std::make_index_sequence<sizeof...(Args) - 1>{});
}

template <typename T>
struct member_traits_rvo;

template <typename Return, typename Class, typename... Args>
struct member_traits_rvo<Return (Class::*)(Args...)> {
  using class_type = Class;
  using return_type = Return;
  using fn_ptr_type = Return (*)(Return*, Class*, Args...);
};

template <typename Return, typename Class, typename... Args>
struct member_traits_rvo<Return (Class::*)(Args...) const> {
  using class_type = Class;
  using return_type = Return;
  using fn_ptr_type = Return (*)(Return*, Class*, Args...);
};

template <auto Method>
__attribute__((noinline)) void HostToGuestMethodWithRVO(PPCContext& ctx, uint8_t* base) {
  using traits = member_traits_rvo<decltype(Method)>;
  using class_type = typename traits::class_type;
  using return_type = typename traits::return_type;
  using fn_ptr_type = typename traits::fn_ptr_type;
  using out_type = std::conditional_t<rex::ppc::is_precise_v<return_type>, be_f32, be_u32>;

  // nullptr is ok because only the type matters here
  auto args = rex::ppc::function_args(static_cast<fn_ptr_type>(nullptr));
  rex::ppc::_translate_args_to_host<static_cast<fn_ptr_type>(nullptr)>(ctx, base, args);

  mapped_u32 out = rex::MappedPtr<out_type>(rex::memory::GuestPtr<out_type*>(base, ctx.r3.u32), ctx.r3.u32);

  auto v = std::apply(std::mem_fn(Method), tuple_tail(args));

  // Memory barrier to ensure compiler doesn't reorder
  asm volatile("" ::: "memory");

  ctx.r3.u32 = out.guest_address();

  if constexpr (std::is_pointer<return_type>()) {
    if (v != nullptr) {
      *out = static_cast<uint32_t>(reinterpret_cast<size_t>(v) - reinterpret_cast<size_t>(base));
    } else {
      *out = 0;
    }
  } else if constexpr (rex::ppc::is_precise_v<return_type>) {
    *out = v;
  } else {
    *out = static_cast<uint32_t>(v);
  }
}

#define METHOD_HOOK_RVO(subroutine, method)   \
  extern "C" REX_FUNC(subroutine) {       \
    HostToGuestMethodWithRVO<method>(ctx, base); \
  }
