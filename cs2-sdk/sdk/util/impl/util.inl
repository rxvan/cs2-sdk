#include "../../sdk.h"

#include "../util.h"

// disable warning C4267 because there is no loss of data (larger type - smaller type, but we never go over INT_MAX anyway..)
#pragma warning(push)
#pragma warning(disable:4267)

namespace sdk::util{
	template<typename ..._args_t>
	constexpr ALWAYS_INLINE std::string format( const char* fmt, _args_t && ...args ) {
		std::string buffer( 1024, '\0' );
		while ( true ) {
			int size = std::snprintf( buffer.data( ), buffer.size( ), fmt, std::forward<_args_t>( args ) ... );
			if ( size < 0 )
				return { };
			if ( static_cast< std::size_t >( size ) < buffer.size( ) )
				return buffer.substr( 0, size );
			buffer.resize( size + 1 );
		}
	};

	template < typename _string_t >
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

	ALWAYS_INLINE const std::string w2m( const wchar_t* const val )
	{
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
}
#pragma warning(pop) // we still need the warning elsewhere, just as a heads-up...
