#include "../memory.h"
#include "../../sdk.h"

#include <assert.h>

namespace sdk {
	__forceinline std::add_pointer_t< PEB > _peb( ) {
		static PEB* peb = NtCurrentTeb( )->ProcessEnvironmentBlock;
		return peb;
	}

	namespace memory {
#pragma region _base_address_t
		template < typename _base_type_t >
		ALWAYS_INLINE _base_address_t< _base_type_t >& _base_address_t< _base_type_t >::self_deref( ptrdiff_t count ) {
			for ( ; count && m_address; --count ) {
				m_address = *reinterpret_cast< _base_type_t* >( m_address );
			}

			return *this;
		}

		template<typename _base_type_t>
		ALWAYS_INLINE constexpr _base_address_t<_base_type_t>& _base_address_t<_base_type_t>::self_offset( ptrdiff_t offset ) {
			m_address += offset;
			return *this;
		}
#pragma endregion

#pragma region address_64_t
		template<typename _base_type_t>
		ALWAYS_INLINE address_64_t< _base_type_t > address_64_t< _base_type_t >::deref( ptrdiff_t count ) {
			address_64_t result = *this;
			return result.self_deref( count );
		}

		template<typename _base_type_t>
		ALWAYS_INLINE address_64_t< _base_type_t > address_64_t< _base_type_t >::offset( const ptrdiff_t offset ) {
			address_64_t result = *this;
			return result.self_offset( offset );
		}
#pragma endregion

#pragma region _base_module_t
		ALWAYS_INLINE bool _base_module_t::load( bool throw_on_fail ) {
			PEB* peb = sdk::_peb( );
			LIST_ENTRY* list_entry = peb->Ldr->InLoadOrderModuleList.Flink;
			while ( list_entry != &peb->Ldr->InLoadOrderModuleList ) {
				LDR_DATA_TABLE_ENTRY* entry = CONTAINING_RECORD( list_entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks );
				if ( entry->BaseDllName.Buffer ) {
					if ( sdk::w2m( entry->BaseDllName.Buffer ).compare( m_name.data( ) ) == 0 ) {
						m_base = address_t< >( reinterpret_cast< std::uintptr_t >( entry->DllBase ) ); // client.dll: 000000000161A690
						m_size = entry->SizeOfImage;
						break;
					}
				}
				list_entry = list_entry->Flink;
			}

			if ( !m_base && throw_on_fail ) {
				// rebase the wide string to a normal string
				throw std::runtime_error( sdk::format( "Failed to find %s in memory!", m_name.data( ) ) );
			}

			// get the headers
			m_dos_header = m_base.as< PIMAGE_DOS_HEADER >( );
			m_nt_headers = m_base.offset( m_dos_header->e_lfanew ).as< PIMAGE_NT_HEADERS >( );

			return m_base;
		}

		inline ALWAYS_INLINE void _base_module_t::for_each_export( const std::function<void( const char*, address_t< std::uintptr_t > )>& fn ) {
			// get the export directory
			IMAGE_EXPORT_DIRECTORY* export_directory = m_base.offset(
				m_nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress
			).as< PIMAGE_EXPORT_DIRECTORY >( );

			// get the names
			std::vector< std::uint32_t > names( export_directory->NumberOfNames );
			if ( std::uint32_t* address_of_names = m_base.offset( export_directory->NumberOfNames ).as< std::uint32_t* >( ); address_of_names != nullptr ) {
				std::copy( address_of_names, address_of_names + export_directory->NumberOfNames, names.begin( ) );
			}

			// get the functions
			std::vector< std::uint32_t > functions( export_directory->NumberOfFunctions );
			if ( std::uint32_t* address_of_functions = m_base.offset( export_directory->AddressOfFunctions ).as< std::uint32_t* >( ); address_of_functions != nullptr ) {
				std::copy( address_of_functions, address_of_functions + export_directory->NumberOfFunctions, functions.begin( ) );
			};

			// get the ordinals
			std::vector< std::uint16_t > ordinals( export_directory->NumberOfNames );
			if ( std::uint16_t* address_of_ordinals = m_base.offset( export_directory->AddressOfNameOrdinals ).as< std::uint16_t* >( ); address_of_ordinals != nullptr ) {
				std::copy( address_of_ordinals, address_of_ordinals + export_directory->NumberOfNames, ordinals.begin( ) );
			};

			// iterate over the exports
			for ( std::size_t i = 0; i < export_directory->NumberOfNames; ++i ) {
				// get the name
				const char* name = m_base.offset( names[ i ] ).as< const char* >( );

				// get the function
				address_t< std::uintptr_t > function = m_base.offset( functions[ ordinals[ i ] ] );

				// call the function
				fn( name, function );
			}
		}

		inline ALWAYS_INLINE address_t< > _base_module_t::get_export( const char* name ) {
			address_t< > address = 0x0;
			for_each_export( [ & ]( const char* export_name, address_t< std::uintptr_t > export_address ) {
				if ( !strcmp( export_name, name ) ) {
					address = export_address;
				}
				} );
			return address;
		}
#pragma endregion

#pragma region module_t
		ALWAYS_INLINE module_t::module_t( bool load_on_construct, const char* name ) :
			_base_module_t( name )
		{
			if ( load_on_construct )
				load( );
		}

		ALWAYS_INLINE address_t< > _base_module_t::find_pattern( std::vector<std::uint8_t> pattern_bytes ) {
			assert( !pattern_bytes.empty( ) && "Pattern bytes missing!" );

			const std::uint8_t* module_memory = reinterpret_cast< const std::uint8_t* >( m_base( ) );
			const std::uint8_t* end = module_memory + m_size - pattern_bytes.size( ) + 1;

			for ( const std::uint8_t* current = module_memory; current < end; ++current ) {
				bool found = true;

				for ( std::size_t i = 0; i < pattern_bytes.size( ); ++i ) {
					if ( pattern_bytes[ i ] != 0xCC && pattern_bytes[ i ] != current[ i ] ) {
						found = false;
						break;
					}
				}

				if ( found )
					return reinterpret_cast< std::uintptr_t >( current );
			}

			throw std::runtime_error{ sdk::format( "failed to find a pattern in %s", m_name ) };
			return unsigned int ( -1 );
		}
#pragma endregion

	}
}