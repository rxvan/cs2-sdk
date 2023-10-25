#include <memory>
#include <thread>
#include <mutex>

#include "../sdk/sdk.h"
#include "../sdk/commands/command_handler.h"
#include "../sdk/error_handler/error_handler.h"
#include "../sdk/memory/memory.h"
#include "../sdk/util/util.h"
#include "../valve/valve.h"

#include <stoba.h>

#define STB(x) stoba::make<x>()

#include "ctx.h"

// pre-defines so we don't need to include consoleapi.h
#ifdef _DEBUG
extern "C" __declspec( dllimport ) int __stdcall AllocConsole( );
extern "C" __declspec( dllimport ) int __stdcall FreeConsole( );
extern "C" __declspec( dllimport ) int __stdcall AttachConsole( unsigned long );
#endif

c_ctx::c_ctx( void* hmodule ) :
	m_file( ),
	m_module( hmodule ),
	m_stop_source( )
{
	( m_threads[ 0 ] = std::jthread( &c_ctx::init, this ) ).detach( );
	( m_threads[ 1 ] = std::jthread( [ & ]( ) {
		while ( !m_stop_source.get_token( ).stop_requested( ) ) {
			if ( !m_ready.load( ) ) {
				if ( !AllocConsole( ) ) {
					if ( GetConsoleWindow( ) != NULL )
						if ( !AttachConsole( -1 ) )
							return; // failed to attach to parent process...
				}

				if ( freopen_s( &m_file[ 0 ], "CONIN$", "r", stdin ) != 0 ||
					freopen_s( &m_file[ 1 ], "CONOUT$", "w", stdout ) != 0 )
					return; // failed to redirect stdout/stdin

				std::cin.clear( ); //
				std::cin.sync( ); // 

				std::once_flag flag;
				std::call_once( flag, [ & ]( ) {
					std::printf( "[INFO ] Initialising... Please wait!\r\n" );
				} );
			}
			else {
				std::once_flag flag;
				std::call_once( flag, [ & ]( ) {
					std::printf( "[INFO ] Initialised!\r\n" );
				} );

				// assign a var we can cin to
				std::string in{ };
				// read the input
				std::getline( std::cin, in );

				if ( const auto& res = sdk::command_handler::run_command( in ); res.has_value( ) )
					std::printf( "Failed to run command: %s", res.value( ) );
			}

			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		}

		std::printf( "[INFO ] Shutting down...\r\n" );

		// stop requested, we can now free ourselves.
		if ( m_file[ 0 ] != nullptr )
			std::fclose( m_file[ 0 ] );

		if ( m_file[ 1 ] != nullptr )
			std::fclose( m_file[ 1 ] );

		FreeConsole( );
		// close the console window
		if ( const HWND hwnd = GetConsoleWindow( ); hwnd != NULL )
			PostMessageA( hwnd, WM_CLOSE, 0, 0 );

		FreeLibraryAndExitThread( static_cast< HMODULE >( m_module ), 0ul );
		}
	) ).detach( );
}

int __stdcall DllMain( void* module, unsigned long reason, void* reserved ) {
	if ( reason != 1 )
		return 0;

	try {
		g_ctx = std::make_unique< c_ctx >( module );
	}
	catch ( ... ) {
		return 0;
	}

	return 1;
}

void c_ctx::init( ) {
	try {
		sdk::error_handler::init( );
		sdk::command_handler::init( );

		sdk::optional_t< sdk::memory::module_t >
			client = sdk::memory::get_module_information( "client.dll" ),
			panorama = sdk::memory::get_module_information( "panorama.dll" );

		assert( client.has_value( ) && panorama.has_value( ) );

		// print the base of client.dll
		if ( sdk::memory::address_t client_base = std::get< const sdk::memory::address_t >( client->get< sdk::e_get_module_base >( ) ); client_base.is_valid( ) )
			std::printf( "[INFO ] client.dll base: 0x%llX\r\n", client_base.get( ) );
		
		offsets_t offsets = get_offsets( ).load( );
		constexpr auto main_menu_pattern  = STB( "48 89 ? ? ? ? ? 4C 89 ? ? ? ? ? 48 89 ? ? ? ? ? 48 8B" );
		constexpr auto ui_engine_pattern  = STB( "74 25 48 8B 0D ? ? ? ? 4C 8B C2" );
		constexpr auto run_script_pattern = STB( "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 81 EC ? ? ? ? 4C 8B F1" );

		offsets.m_panorama.m_main_menu_panel = client->find_pattern(
			main_menu_pattern
		).value_or( 0xCC ).self_offset( 0x11 ).self_relative( ).deref( );
		offsets.m_panorama.m_engine = client->find_pattern(
			ui_engine_pattern
		).value_or( 0xCC ).self_offset( 0x5 ).self_relative( );
		offsets.m_panorama.m_run_script = panorama->find_pattern(
			run_script_pattern
		).value_or( 0xCC );
		get_offsets( ).store( offsets );


	}
	catch ( const std::exception& ex ) {
#ifdef _DEBUG
		std::printf( "[ERROR] %s\r\n", ex.what( ) );
#else
		// create a dump at some point in the future.
#endif
		m_ready.store( false );

		if ( m_stop_source.stop_possible( ) ) {
			m_stop_source.request_stop( );
		}
		return;
	}

	m_ready.store( true ); // everything is done bruh
}
