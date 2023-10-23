#pragma once

#include "../../sdk.h"

namespace sdk {
	enum e_get_module_type : int {
		e_get_module_base,
		e_get_module_size,
		e_get_module_name
	};
}

namespace sdk::memory {
	namespace detail {
		template <typename _base_type_t = uintptr_t>
		class base_address_t {
		protected:
			_base_type_t m_address{};
		public:
			ALWAYS_INLINE constexpr base_address_t( ) = default;
			ALWAYS_INLINE constexpr base_address_t( _base_type_t _val ) :
				m_address( _val ) {
				assert( is_valid( ) && "invalid address!" ); }

			ALWAYS_INLINE constexpr operator _base_type_t( ) {
				return m_address;
			}

			ALWAYS_INLINE sdk::reference_t< base_address_t< _base_type_t > > self_offset( const ptrdiff_t offset ) {
				m_address += offset;
				return *this;
			}

			ALWAYS_INLINE sdk::reference_t< base_address_t< _base_type_t > > self_deref( ptrdiff_t count = 1ull ) {
				for ( ; count > 0; --count )
					m_address = *reinterpret_cast< _base_type_t* >( m_address );
				return *this;
			}

			ALWAYS_INLINE sdk::reference_t< base_address_t< _base_type_t > > self_relative( const ptrdiff_t offset = 1ull, const bool is_long = true ) {
				m_address += is_long ? sizeof( int ) + *reinterpret_cast< int* >( m_address ) : sizeof( char ) + *reinterpret_cast< char* >( m_address );
				return *this;
			}

			ALWAYS_INLINE sdk::reference_t< base_address_t< _base_type_t > > offset( const ptrdiff_t offset ) {
				auto copy = *this;
				return copy.self_offset( offset );
			}

			ALWAYS_INLINE sdk::reference_t< base_address_t< _base_type_t > > deref( const ptrdiff_t count = 1ull ) {
				auto copy = *this;
				return copy.self_deref( count );
			}

			ALWAYS_INLINE sdk::reference_t< base_address_t< _base_type_t > > relative( const ptrdiff_t offset = 1ull, const bool is_long = true ) {
				auto copy = *this;
				return copy.self_relative( offset, is_long );
			}

			_base_type_t get( const ptrdiff_t offset = 0ull ) {
				return m_address + offset;
			}

			template < typename _cast_type_t >
			ALWAYS_INLINE _cast_type_t as( const ptrdiff_t offset = 0ull ) {
				return reinterpret_cast< _cast_type_t >( m_address + offset );
			}

			template < typename _cast_type_t >
			ALWAYS_INLINE _cast_type_t static_as( const ptrdiff_t offset = 0ull ) {
				

				return static_cast< _cast_type_t >( m_address + offset );
			}

			ALWAYS_INLINE constexpr bool is_valid( ) {
				return m_address > 0x0 && m_address != 0xCC; // wildcard for uninitialized memory
			}
		};
	}

	struct module_t {
	private:
		detail::base_address_t< > m_base{};
		std::size_t m_size{};
		const char* m_name{};

		IMAGE_NT_HEADERS64* m_nt_headers{};
		IMAGE_DOS_HEADER* m_dos_header{};
	public:
		std::map< const char*, detail::base_address_t< > > m_exports{};
		std::map< const char*, std::pair< detail::base_address_t< >, std::size_t > > m_sections{};

		module_t( ) = default;
		module_t( const char* name, IMAGE_NT_HEADERS64* nt_headers, IMAGE_DOS_HEADER* dos_header ) :
			m_name( name ),
			m_nt_headers( nt_headers ),
			m_dos_header( dos_header )
		{
			ASSERT( nt_headers && dos_header );
			m_base = reinterpret_cast< std::uintptr_t >( dos_header );
			m_size = nt_headers->OptionalHeader.SizeOfImage;

			IMAGE_EXPORT_DIRECTORY* export_directory = reinterpret_cast< IMAGE_EXPORT_DIRECTORY* >(
				m_base.get( nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress )
				);

			if ( export_directory != nullptr ) {
				auto names = reinterpret_cast< std::uint32_t* >( m_base.get( export_directory->AddressOfNames ) );
				auto functions = reinterpret_cast< std::uint32_t* >( m_base.get( export_directory->AddressOfFunctions ) );
				auto ordinals = reinterpret_cast< std::uint16_t* >( m_base.get( export_directory->AddressOfNameOrdinals ) );

				for ( std::size_t i = 0; i < export_directory->NumberOfNames; ++i ) {
					const char* name = reinterpret_cast< const char* >( m_base.get( names[ i ] ) );
					detail::base_address_t< > address = m_base.get( functions[ ordinals[ i ] ] );

					m_exports.insert( { name, address } );
				}
			}
		}
	public:
		template < int get_type >
		ALWAYS_INLINE constexpr sdk::variant_t< const detail::base_address_t< >, const std::size_t, const std::string, const std::nullopt_t > const get( ) const {
			assert( ( int )get_type >= 0 && ( int )get_type < 3 );

			switch ( get_type )
			{
			case e_get_module_base:
				return m_base;
			case e_get_module_size:
				return m_size;
			case e_get_module_name:
				return m_name;
			}

			return std::nullopt;
		}

		ALWAYS_INLINE std::pair< detail::base_address_t< >, std::size_t > const get_section( const char* name ) {
			for ( auto i = 0; i < m_nt_headers->FileHeader.NumberOfSections; ++i ) {
				IMAGE_SECTION_HEADER* section = reinterpret_cast< IMAGE_SECTION_HEADER* >( m_base.get( m_dos_header->e_lfanew + sizeof( IMAGE_NT_HEADERS64 ) + ( sizeof( IMAGE_SECTION_HEADER ) * i ) ) );
				// the name is an array of 8 bytes, so we need to copy it into a string
				std::string section_name = std::string( reinterpret_cast< char* >( section->Name ) );
				// check if the name is valid
				if ( section_name.find( '\0' ) != std::string::npos )
					section_name = section_name.substr( 0, section_name.find( '\0' ) );

				if ( section_name.compare( name ) == 0 )
					return { m_base.get( section->VirtualAddress ), section->Misc.VirtualSize };

			}

			return { }; // return empty pair, because we didn't find the section...!
		}

		template < size_t _len >
		ALWAYS_INLINE constexpr sdk::optional_t< detail::base_address_t< > > find_pattern( const std::array< int, _len >& bytes ) {
			auto code_section = get_section( ".text" );
			const std::uintptr_t code_base = m_base.get( );
			const std::size_t code_size = code_section.second,
				pattern_size = bytes.size( );

			/* ^^lazy above ignore pls ty */

			for ( std::uintptr_t current = code_base; current < code_base + ( code_size - _len ); ++current ) {
				bool found = true;

				for ( std::size_t i = 0; i < bytes.size( ); ++i ) {
					if ( bytes[ i ] != -1 && bytes[ i ] != reinterpret_cast< std::uint8_t* >( current )[ i ] ) {
						found = false;
						break;
					}
				}

				if ( found )
					return current;
			}

			return std::nullopt;
		}
	};
}
