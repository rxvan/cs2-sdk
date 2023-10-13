#include "../string_converter.h"
#include "../../string_converter/string_converter.h"

namespace sdk {

}

namespace sdk::string_converter {
	namespace impl {
		template <typename _string_type_t>
		ALWAYS_INLINE constexpr std::vector<std::uint8_t> _string_byte_converter_t<_string_type_t>::_to_bytes( ) {
			assert( !m_string.empty( ) && "You need to initialize the string first! (Did you call the constructor?)" );

			static constexpr auto _char_to_integer = [ ]( const char& ch ) {
				if ( ch >= '0' && ch <= '9' )
					return static_cast< std::uint8_t >( ch - '0' );
				if ( ch >= 'A' && ch <= 'F' )
					return static_cast< std::uint8_t >( ch - 'A' + 10 );
				if ( ch >= 'a' && ch <= 'f' )
					return static_cast< std::uint8_t >( ch - 'a' + 10 );
				return static_cast< std::uint8_t >( 0 );
				};

			std::vector<std::uint8_t> bytes;
			bytes.reserve( m_string.size( ) ); // Reserve the size of the string (could be less, but this is enough)

			// Patterns are IDA-style, so we need to convert them to bytes. The only wildcard should be '?'
			for ( std::size_t m_string_index = 0; m_string_index < m_string.size( ); ++m_string_index ) {
				char ch = m_string[ m_string_index ];
				// Check if the character is a wildcard
				if ( ch == '?' ) {
					bytes.push_back( 0xCC );
				}
				else if ( ( ch >= '0' && ch <= '9' ) || ( ch >= 'A' && ch <= 'F' ) || ( ch >= 'a' && ch <= 'f' ) ) {
					// Check if the character is a valid hex character
					if ( m_string_index + 1 < m_string.size( ) ) {
						char second = m_string[ m_string_index + 1 ];
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
					throw std::runtime_error( "Invalid character '" + std::string( 1, ch ) + "' in pattern!" );
				}
			}

			return bytes;
		}

		template <typename _string_type_t>
		ALWAYS_INLINE constexpr std::string _string_byte_converter_t<_string_type_t>::_to_string( ) {
			assert( !m_bytes.empty( ) && "You need to initialize the byte vector first! (Did you call the constructor?)" );

			static constexpr auto _integer_to_char = [ ]( const int n ) {
				// Check if the integer is a valid hex character
				if ( n >= 0 && n <= 9 )
					return static_cast< char >( '0' + n );
				else if ( n >= 10 && n <= 15 )
					return static_cast< char >( 'A' + n - 10 );
				return '\0'; // Return a null character for invalid input
				};

			std::string string;
			string.reserve( m_bytes.size( ) * 2 ); // Reserve space for the hex string

			for ( std::uint8_t byte : m_bytes ) {
				// Check if the byte is a wildcard
				if ( byte == 0xCC ) {
					string.push_back( '?' );
				}
				else {
					// Convert the byte to a string
					string.push_back( _integer_to_char( byte >> 4 ) );
					string.push_back( _integer_to_char( byte & 0x0F ) );
				}
			}

			return string;
		}
	}

	template<typename _return_type_t>
	ALWAYS_INLINE _return_type_t string_to( const std::string& string, std::function< void( const char& ch ) > fn ) {
		for ( const char& ch : string ) {
			// Call the function with the character
			fn( ch );
		}
	}

	template<typename _return_type_t>
	ALWAYS_INLINE static std::uint8_t bytes_to( const std::vector< std::uint8_t >& bytes, std::function< void( const char& ch ) > fn ) {
		for ( const std::uint8_t& byte : bytes ) {
			// Call the function with the byte
			fn( byte );
		}
	}
}