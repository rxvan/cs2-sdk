#pragma once
#include <phnt_windows.h>
#include <phnt_ntdef.h>
#include <phnt.h>
#include <stringapiset.h>
#include <variant>
#include <type_traits>
#include <optional>
#include <string>

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
	static constexpr hash_t hash( const _string_t* const val, const size_t len ) {
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
	ALWAYS_INLINE constexpr hash_t hash( const _string_t* const val ) {
		std::size_t len{ };
		for ( ; val[ len ]; ++len ); // quickest way to get string length, cleaner than a do-while loop too
		return hash( val, len );
	}

	// wide string to multibyte string
	ALWAYS_INLINE const std::string w2m( const wchar_t* const val ) {
		std::vector< char > buffer( 1024, '\0' );
		while ( true ) {
			const auto size = WideCharToMultiByte( CP_UTF8, 0, val, -1, buffer.data( ), buffer.size( ), nullptr, nullptr );
			if ( size < 0 )
				return { };
			if ( static_cast< std::size_t >( size ) < buffer.size( ) )
				return buffer.data( );
			buffer.resize( size + 1 );
		}

		return { };
	}

	static constexpr auto _char_to_integer = [ ]( const char& ch ) {
		if ( ch >= '0' && ch <= '9' )
			return static_cast< std::uint8_t >( ch - '0' );
		if ( ch >= 'A' && ch <= 'F' )
			return static_cast< std::uint8_t >( ch - 'A' + 10 );
		if ( ch >= 'a' && ch <= 'f' )
			return static_cast< std::uint8_t >( ch - 'a' + 10 );
		return static_cast< std::uint8_t >( 0 );
	};

	ALWAYS_INLINE constexpr std::vector< std::uint8_t > string_to_bytes( const char* _pattern ) {
		std::size_t len{ };
		for ( ; _pattern[ len ]; ++len ); // quickest way to get string length, cleaner than a do-while loop too

		std::vector<std::uint8_t> bytes;
		bytes.reserve( len ); // Reserve the size of the string (could be less, but this is enough)

		// Patterns are IDA-style, so we need to convert them to bytes. The only wildcard should be '?'
		for ( std::size_t m_string_index = 0; m_string_index < len; ++m_string_index ) {
			char ch = _pattern[ m_string_index ];
			// Check if the character is a wildcard
			if ( ch == '?' ) {
				bytes.push_back( 0xCC );
			}
			else if ( ( ch >= '0' && ch <= '9' ) || ( ch >= 'A' && ch <= 'F' ) || ( ch >= 'a' && ch <= 'f' ) ) {
				// Check if the character is a valid hex character
				if ( m_string_index + 1 < len ) {
					char second = _pattern[ m_string_index + 1 ];
					if ( ( second >= '0' && second <= '9' ) || ( second >= 'A' && second <= 'F' ) || ( second >= 'a' && second <= 'f' ) ) {
						// Convert the characters to integers
						std::uint8_t first_integer = _char_to_integer( ch );
						std::uint8_t second_integer = _char_to_integer( second );
						// Convert the integers to a byte
						std::uint8_t byte = ( first_integer << 4 ) | second_integer;
						// Add the byte to the vector
						bytes.push_back( byte );
						m_string_index++; // Move to the next character
					}
				}
			}
			else if ( ch != ' ' ) {
				// If we got here, the character is invalid
				bytes.clear( ); // Clear the vector because the pattern is invalid
				break;
			}
		}

		return bytes;
	}

	// reverse above (bytes to string)
	template < typename _container_t >
	ALWAYS_INLINE constexpr std::string bytes_to_string( const _container_t& bytes ) {
		static constexpr auto _integer_to_char = [ ]( const std::uint8_t& integer ) {
			if ( integer >= 0 && integer <= 9 )
				return static_cast< char >( integer + '0' );
			if ( integer >= 10 && integer <= 15 )
				return static_cast< char >( integer + 'A' - 10 );
			return static_cast< char >( 0 );
		};

		std::string string;
		for ( auto & byte : bytes ) {
			string.push_back( _integer_to_char( byte >> 4 ) );
			string.push_back( _integer_to_char( byte & 0xF ) );
		}

		return string;
	}

	template <typename _base_type_t>
	using pointer_t = std::add_pointer_t<std::remove_pointer_t<_base_type_t>>;
	template <typename _base_type_t>
	using reference_t = std::add_lvalue_reference_t<std::remove_pointer_t<_base_type_t>>;
	template <typename _base_type_t>
	using value_t = std::remove_pointer_t<_base_type_t>;
	template <typename _base_type_t>
	using optional_t = std::optional<std::remove_pointer_t<_base_type_t>>;
	template <typename ... _types_t>
	using variant_t = std::variant<std::remove_pointer_t<_types_t>...>;
}

constexpr sdk::hash_t operator "" _hash( const char* const str, const std::size_t len ) {
	return sdk::hash( str, len );
}

#include "memory/memory.h"
#include "error_handler/error_handler.h"
#include "commands/command_handler.h"
