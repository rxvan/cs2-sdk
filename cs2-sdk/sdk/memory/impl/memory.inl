#include <phnt.h>

#include <functional>

#include "../../sdk.h"
#include "../../util/util.h"

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
					fn( module_t( util::w2m( entry->BaseDllName.Buffer ).data( ), nt_headers, dos_header ), is_done );
				}
			} while ( list_entry != peb->Ldr->InLoadOrderModuleList.Blink );
		}
	}

	ALWAYS_INLINE sdk::optional_t< module_t > get_module_information( const char* name ) {
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

	ALWAYS_INLINE sdk::optional_t<module_t> get_module_information( void* base_address ) {
		std::optional< module_t > ret = std::nullopt;

		impl::for_each_module( [ &base_address, &ret ]( const module_t& mod, bool& is_done ) {
			const auto value = mod.get< e_get_module_base >( );
			if ( std::holds_alternative< const std::nullopt_t >( value ) )
				return;

			if ( std::holds_alternative< const address_t >( value ) ) {
				address_t addr = std::get< const address_t >( value );
				if ( addr == reinterpret_cast< std::uintptr_t >( base_address ) ) {
					ret = mod;
					is_done = true;
				}
			}
		} );

		return ret;
	}

	template < typename _interface_class >
	ALWAYS_INLINE sdk::optional_t<_interface_class*> const get_interface( const module_t& mod, const char* interface_name ) {
		if ( mod.m_exports.find( "CreateInterface" ) == mod.m_exports.end( ) )
			return std::nullopt; // bruh

		address_t CreateInterface = mod.m_exports.at( "CreateInterface" );
		if ( !CreateInterface )
			return std::nullopt; // bruh

		CreateInterface.self_relative( 0x3 ).self_deref( );

		const auto get_interface = [ &CreateInterface ]( const char* interface_name ) -> void* {
			struct interface_entry_t {
				typedef void* ( *create_t )( );

				create_t            m_create_fn{};
				const char* m_name{};
				interface_entry_t* m_next{};
			};

			for ( interface_entry_t* cur = reinterpret_cast< interface_entry_t* >( CreateInterface.get( ) ); cur != nullptr; cur = cur->m_next ) {
				if ( cur->m_name == nullptr )
					continue;

				if ( sdk::util::hash( interface_name ) == sdk::util::hash( cur->m_name ) )
					return cur->m_create_fn( );
			}
		};

		if ( void* interface_ptr = get_interface( interface_name ) )
			return reinterpret_cast< _interface_class* >( interface_ptr );

		return std::nullopt; // bruh.
	}

}