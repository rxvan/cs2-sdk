#pragma once

namespace sdk {
	typedef std::size_t hash_t;

	namespace util{
		namespace impl {
		}
		template < typename _type >
		inline constexpr auto is_char_v = std::is_same_v< _type, char > || std::is_same_v< _type, char16_t >
			|| std::is_same_v< _type, char32_t > || std::is_same_v< _type, wchar_t >;

		template < typename ... _args_t >
		ALWAYS_INLINE constexpr std::string format( const char* fmt, _args_t&& ... args );

		template <typename _string_t>
		ALWAYS_INLINE constexpr hash_t hash( const _string_t* const val, const size_t len );

		template <typename _string_t>
		ALWAYS_INLINE constexpr hash_t hash( const _string_t* const val ) {
			constexpr std::size_t len{ };
			for ( ; val[ len ]; ++len ); // quickest way to get string length, cleaner than a do-while loop too
			return hash( val, len );
		}

		// wide string to multibyte string
		ALWAYS_INLINE const std::string w2m( const wchar_t* const val );
	}
}

#include "impl/util.inl"

constexpr sdk::hash_t operator ""_hash( const char* const str, const std::size_t len ) {
	return sdk::util::hash( str, len );
}
