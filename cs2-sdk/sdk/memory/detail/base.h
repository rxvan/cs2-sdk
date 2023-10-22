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
			constexpr base_address_t( ) noexcept = default;
			constexpr base_address_t( _base_type_t _val ) :
				m_address( _val )
			{ }

			constexpr operator _base_type_t( ) const noexcept {
				return m_address;
			}

			sdk::reference_t< base_address_t< _base_type_t > > const self_offset( const ptrdiff_t offset ) const noexcept {
				m_address += offset;
				return *this;
			}

			sdk::reference_t< base_address_t< _base_type_t > > const self_deref( ptrdiff_t count = 1ull ) const noexcept {
				for ( ; count > 0; --count )
					m_address = *reinterpret_cast< _base_type_t* >( m_address );
				return *this;
			}

			sdk::reference_t< base_address_t< _base_type_t > > const self_relative( const ptrdiff_t offset = 1ull, const bool is_long = true ) const noexcept {
				m_address += is_long ? sizeof( int ) + *reinterpret_cast< int* >( m_address ) : sizeof( char ) + *reinterpret_cast< char* >( m_address );
				return *this;
			}

			sdk::reference_t< base_address_t< _base_type_t > > const offset( const ptrdiff_t offset ) const noexcept {
				auto copy = *this;
				return copy.self_offset( offset );
			}

			sdk::reference_t< base_address_t< _base_type_t > > const deref( const ptrdiff_t count = 1ull ) const noexcept {
				auto copy = *this;
				return copy.self_deref( count );
			}

			sdk::reference_t< base_address_t< _base_type_t > > const relative( const ptrdiff_t offset = 1ull, const bool is_long = true ) const noexcept {
				auto copy = *this;
				return copy.self_relative( offset, is_long );
			}

			sdk::value_t< _base_type_t > const get( const ptrdiff_t offset = 0ull ) const noexcept {
				return m_address + offset;
			}

			template < typename _cast_type_t >
			_cast_type_t as( const ptrdiff_t offset = 0ull ) const noexcept {
				return reinterpret_cast< _cast_type_t >( m_address + offset );
			}

			template < typename _cast_type_t >
			_cast_type_t static_as( const ptrdiff_t offset = 0ull ) const noexcept {
				return static_cast< _cast_type_t >( m_address + offset );
			}
		};
	}

	using address_t = detail::base_address_t<>;

	struct module_t {
	private:
		address_t m_base{};
		std::size_t m_size{};
		const char* m_name{};

		IMAGE_NT_HEADERS64* m_nt_headers{};
		IMAGE_DOS_HEADER* m_dos_header{};
	public:
		std::map< const char*, address_t > m_exports{};
		std::map< const char*, std::pair< address_t, std::size_t > > m_sections{};

		module_t( ) noexcept = default;
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
					address_t address = m_base.get( functions[ ordinals[ i ] ] );

					m_exports.insert( { name, address } );
				}
			}
		}
	public:
		template < int get_type >
		ALWAYS_INLINE constexpr sdk::variant_t< const address_t, const std::size_t, const std::string, const std::nullopt_t > const get( ) const noexcept {
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

		ALWAYS_INLINE std::pair< address_t, std::size_t > const get_section( const char* name ) const noexcept {
			for ( auto i = 0; i < m_nt_headers->FileHeader.NumberOfSections; ++i ) {
				IMAGE_SECTION_HEADER* section = reinterpret_cast< IMAGE_SECTION_HEADER* >( m_base.get( m_dos_header->e_lfanew + sizeof( IMAGE_NT_HEADERS64 ) + ( sizeof( IMAGE_SECTION_HEADER ) * i ) ) );
				// the name is an array of 8 bytes, so we need to copy it into a string
				std::string section_name = std::string( reinterpret_cast< char* >( section->Name ) );
				// check if the name is valid
				if ( section_name.find( '\0' ) != std::string::npos )
					section_name = section_name.substr( 0, section_name.find( '\0' ) );

				if ( section_name.compare( name ) == 0 )
					return { m_base.get( section->VirtualAddress ), section->Misc.VirtualSize };

				return { }; // return empty pair, because we didn't find the section...!
			}
		}

		template <typename _container_t = std::vector< std::uint8_t > >
		ALWAYS_INLINE sdk::optional_t< address_t > find_pattern( const _container_t& bytes ) const {
			const auto code_section = get_section( ".text" );
			const std::uint8_t* code_base = reinterpret_cast< std::uint8_t* >( code_section.first.get( ) );
			const std::size_t code_size = code_section.second,
				pattern_size = bytes.size( );

			/* ^^lazy above ignore pls ty */

			for ( std::uint8_t* current = const_cast< std::uint8_t* >( code_base ); current < code_base + code_size; ++current ) {
				bool found = true;

				for ( std::size_t i = 0; i < bytes.size( ); ++i ) {
					if ( bytes[ i ] != 0xCC && bytes[ i ] != current[ i ] ) {
						found = false;
						break;
					}
				}

				if ( found )
					return reinterpret_cast< std::uintptr_t >( current );
			}
		}
	}; 
}
