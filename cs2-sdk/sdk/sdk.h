#pragma once
#include <phnt_windows.h>
#include <phnt_ntdef.h>
#include <phnt.h>
#include <stringapiset.h>
#include <variant>
#include <type_traits>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <assert.h>
#include <atomic>

#pragma region Standard Definitions
#define STR(x) #x											// stringify
#define PTR(x) std::add_pointer_t<x>						// pointer
#define REF(x) std::add_lvalue_reference_t<x>				// reference
#define RREF(x) std::add_rvalue_reference_t<x>				// reference
#define PAD(x) std::array<std::byte, x>						// class padding
#define LEN(x) std::extent_v<x>								// array length
#define SIZE(x) sizeof(x)									// size of type
#define CSIZE(x) constexpr SIZE(x)							// constexpr size of type
#define ALIGN(x) alignas(x)									// align type
#define ALIGNED(x) alignas(x) x								// align variable (use instead of padding, where possible, as its more readable)
#define ALWAYS_INLINE __forceinline							// force inline
#define NO_INLINE __declspec(noinline)						// no inline
#define NO_DISCARD [[nodiscard]]							// don't discard return value
#define NO_EXCEPT noexcept									// no exceptions
#define FUNC_SIG __FUNCSIG__								// function signature
#define FUNC_NAME __FUNCTION__ 								// function name
#define FUNC_LINE __LINE__									// current line
#define FUNC_FILE __FILE__									// current file
#define FUNC_DATE __DATE__									// current date
#define FUNC_TIME __TIME__									// current time
#define FUNC_DATE_TIME __TIMESTAMP__						// current date and time
#define GREATER_EQUAL(x, y) std::isgreaterequal( x, y )		// "safe" version of >= operator for floating point numbers

#ifdef _DEBUG
#define ASSERT(x) assert(x)									// assert
#else
#define ASSERT(x)											// assert
#endif
#pragma endregion

namespace sdk {
	template <typename _base_type_t>
	using pointer_t = std::add_pointer_t<std::remove_pointer_t<_base_type_t>>;
	template <typename _base_type_t>
	using reference_t = std::add_lvalue_reference_t<_base_type_t>;
	template <typename _base_type_t>
	using value_t = std::add_rvalue_reference_t<_base_type_t>;
	template <typename _base_type_t>
	using optional_t = std::optional<std::remove_pointer_t<_base_type_t>>;
	template <typename ... _types_t>
	using variant_t = std::variant<std::remove_pointer_t<_types_t>...>;
	template <typename _base_type_t>
	using atomic_reference_t = reference_t< std::atomic< _base_type_t > >;
}
