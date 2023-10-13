#pragma once
#include <string>

namespace sdk {

}

namespace sdk::string_converter {
	namespace impl {
		template < typename _string_type_t = std::string >
		struct _string_byte_converter_t {
			_string_type_t m_string;
			std::vector< std::uint8_t > m_bytes;

			// @brief - rxvan: converts the given string to a vector of bytes.
			// @note - this constructor is used for the pattern scanner.
			constexpr _string_byte_converter_t( const char* string ) :
				m_string( string ),
				m_bytes( std::vector< std::uint8_t >( ) )
			{ }

			// @brief - rxvan: converts the given vector of bytes to a string.
			// @note - this constructor is used for debugging the pattern scanner.
			constexpr _string_byte_converter_t( const std::vector< std::uint8_t >& bytes ) :
				m_string( std::string( ) ),
				m_bytes( bytes )
			{ }

			// implementation functions.
			ALWAYS_INLINE constexpr std::vector< std::uint8_t > _to_bytes( );

			ALWAYS_INLINE constexpr std::string _to_string( );
		};


	}

	struct string_byte_converter_t : public impl::_string_byte_converter_t< std::string > {
		using impl::_string_byte_converter_t<>::_string_byte_converter_t;
	};

	ALWAYS_INLINE static constexpr std::vector< std::uint8_t > to_bytes( const char* string ) {
		return string_byte_converter_t( string )._to_bytes( );
	}

	ALWAYS_INLINE static constexpr std::string to_string( const std::vector< std::uint8_t >& bytes ) {
		return string_byte_converter_t( bytes )._to_string( );
	}

	template < typename _return_type_t >
	ALWAYS_INLINE static _return_type_t string_to( const std::string& string, std::function< void( const char& ch ) > fn );

	template < typename _return_type_t >
	ALWAYS_INLINE static std::uint8_t bytes_to( const std::vector< std::uint8_t >& bytes, std::function< void( const char& ch ) > );
}

#include "impl/string_converter.inl"
