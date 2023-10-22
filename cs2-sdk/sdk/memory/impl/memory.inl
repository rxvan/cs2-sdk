#include <phnt.h>

#include <functional>

#include "../../sdk.h"

#include "../memory.h"

namespace sdk::memory {
	namespace impl {
		ALWAYS_INLINE PEB* get_peb( ) {
			static PEB* peb = reinterpret_cast< PEB* >( __readgsqword( 0x60 ) );
			return peb;
		}

		ALWAYS_INLINE void for_each_module( const std::function< void( const module_t&, bool& is_done ) >& fn ) {
			PEB* peb = impl::get_peb( );

			LIST_ENTRY* list_entry = nullptr;
			bool is_done = false;
			do {
				if ( is_done )
					break;

				list_entry = list_entry == nullptr ? peb->Ldr->InLoadOrderModuleList.Flink : list_entry->Flink;
				if ( list_entry == nullptr )
					break;

				LDR_DATA_TABLE_ENTRY* entry = CONTAINING_RECORD( list_entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks );
				if ( entry == nullptr )
					break;

				if ( entry->BaseDllName.Buffer != nullptr ) {
					// get the headers
					IMAGE_DOS_HEADER* dos_header = reinterpret_cast< IMAGE_DOS_HEADER* >( entry->DllBase );
					IMAGE_NT_HEADERS* nt_headers = reinterpret_cast< IMAGE_NT_HEADERS* >( reinterpret_cast< uintptr_t >( entry->DllBase ) + dos_header->e_lfanew );

					// call the function
					fn( module_t( sdk::w2m( entry->BaseDllName.Buffer ).data( ), nt_headers, dos_header ), is_done );
				}
			} while ( list_entry != peb->Ldr->InLoadOrderModuleList.Blink );
		}
	}

	ALWAYS_INLINE sdk::optional_t< module_t > const get_module_information( const char* name ) {
		std::optional< module_t > ret = std::nullopt;

		impl::for_each_module( [ &name, &ret ]( const module_t& mod, bool& is_done ) {
			const auto value = mod.get< e_get_module_name >( );
			if ( std::holds_alternative< const std::nullopt_t >( value ) )
				return;

			if ( std::holds_alternative< const std::string >( value ) ) {
				const auto& str = std::get< const std::string >( value );
				if ( str.compare( name ) == 0 ) {
					*&ret = mod;
					is_done = true;
				}
			}
		} );

		return ret;
	}

	ALWAYS_INLINE sdk::optional_t<module_t> const memory::get_module_information( void* base_address ) {
		std::optional< module_t > ret = std::nullopt;

		impl::for_each_module( [ &base_address, &ret ]( const module_t& mod, bool& is_done ) {
			const auto value = mod.get< e_get_module_base >( );
			if ( std::holds_alternative< const std::nullopt_t >( value ) )
				return;

			if ( std::holds_alternative< const address_t >( value ) ) {
				const address_t& addr = std::get< const address_t >( value );
				if ( addr == reinterpret_cast< std::uintptr_t >( base_address ) ) {
					ret = mod;
					is_done = true;
				}
			}
		} );

		return ret;
	}

}