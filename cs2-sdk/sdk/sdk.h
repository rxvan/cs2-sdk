#pragma once
#include <phnt_windows.h>
#include <phnt_ntdef.h>
#include <phnt.h>

#include <type_traits>
#include <vector>
#include <format>
#include <array>
#include <map>
#include <string>
#include <string_view>

#pragma region Standard Definitions
#define PTR(x) std::add_pointer_t<x>
#define REF(x) std::add_lvalue_reference_t<x>
#define RREF(x) std::add_rvalue_reference_t<x>
#define PAD(x) std::array<std::byte, x>
#define LEN(x) std::extent_v<x>
#define SIZE(x) sizeof(x)
#define CSIZE(x) constexpr SIZE(x)
#define ALIGN(x) alignas(x)
#define ALIGNED(x) alignas(x) x
#define ALWAYS_INLINE __forceinline
#define NO_INLINE __declspec(noinline)
#define NO_DISCARD [[nodiscard]]
#define NO_EXCEPT noexcept
#define FUNC_SIG __FUNCSIG__
#define FUNC_NAME __FUNCTION__
#define FUNC_LINE __LINE__
#define FUNC_FILE __FILE__
#define FUNC_DATE __DATE__
#define FUNC_TIME __TIME__
#define FUNC_DATE_TIME __TIMESTAMP__
#pragma endregion

namespace sdk {
	template < typename _type >
	inline constexpr auto is_char_v = std::is_same_v< _type, char > || std::is_same_v< _type, char16_t >
		|| std::is_same_v< _type, char32_t > || std::is_same_v< _type, wchar_t >;

	template < typename ... _args_t >
	ALWAYS_INLINE constexpr std::string format( const char* fmt, _args_t&& ... args ) {
		std::string buffer( 1024, '\0' );
		while ( true ) {
			const auto size = std::snprintf( buffer.data( ), buffer.size( ), fmt, std::forward<_args_t>( args ) ... );
			if ( size < 0 )
				return { };
			if ( static_cast< std::size_t >( size ) < buffer.size( ) )
				return buffer.substr( 0, size );
			buffer.resize( size + 1 );
		}
	};

	typedef std::size_t hash_t;

	template <typename _string_t>
		requires is_char_v< typename _string_t::value_type >
	ALWAYS_INLINE constexpr hash_t hash( const _string_t* const val, const size_t len ) {
#if defined( _M_IX86 ) || defined( __i386__ ) || defined( _M_ARM ) || defined( __arm__ )
		constexpr auto k_basis = 0x811c9dc5u;
		constexpr auto k_prime = 0x1000193u;
#else
		constexpr auto k_basis = 0xcbf29ce484222325u;
		constexpr auto k_prime = 0x100000001b3u;
#endif
		hash_t hash{ k_basis };
		for ( hash_t i{ }; i < len; ++i )
			hash ^= val[ i ], hash *= k_prime;
		return hash;
	}

	template <typename _string_t>
		requires is_char_v< typename _string_t::value_type >
	ALWAYS_INLINE constexpr hash_t hash( const _string_t* const val ) {
		std::size_t len{ };
		for ( ; val[ len ]; ++len );
		return hash( val, len );
	}

	// wide string to multibyte string
	ALWAYS_INLINE std::string w2m( const wchar_t* const val ) {
		std::vector< char > buffer( 1024, '\0' );
		// convert the wide string to a multibyte string
		std::size_t size = WideCharToMultiByte( CP_UTF8, 0, val, -1, buffer.data( ), buffer.size( ), nullptr, nullptr );
		if ( size == 0 )
			return { };

		// if the buffer is too small, resize it and try again
		if ( size > buffer.size( ) ) {
			buffer.resize( size );
			size = WideCharToMultiByte( CP_UTF8, 0, val, -1, buffer.data( ), buffer.size( ), nullptr, nullptr );
			if ( size == 0 )
				return { };
		}

		return buffer.data( );
	}
}

#include "memory/memory.h"
#include "error_handler/error_handler.h"
#include "string_converter/string_converter.h"