#include <memory>
#include <thread>

#include "ctx.h"

#include "../sdk/sdk.h"

#include <processthreadsapi.h>
#include <mutex>

c_ctx::c_ctx( void* hmodule ) :
	m_file( ),
	m_module( hmodule ),
	m_stop_source( )
{
	( m_threads[ 0 ] = std::jthread( &c_ctx::init, this ) ).detach( );
	( m_threads[ 1 ] = std::jthread( [ & ]( ) {
		while ( !m_stop_source.get_token( ).stop_requested( ) ) {
			if ( !m_ready.load( ) ) {
				std::once_flag flag;
				std::call_once( flag, [ & ]( ) {
					std::printf( "[INFO ] Initialising... Please wait!\r\n" );
				} );
			}
			else {
				std::printf( "[INFO ] Initialisation complete!\r\n" );
				break;
			}

			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		}

		// assign a var we can cin to
		std::string in{ };
		std::getline( std::cin, in );

		// stop requested, we can now free ourselves.
		if ( m_file[ 0 ] != nullptr )
			std::fclose( m_file[ 0 ] );

		if ( m_file[ 1 ] != nullptr )
			std::fclose( m_file[ 1 ] );

		if ( m_threads[ 0 ].joinable( ) )
			m_threads[ 0 ].join( );

		std::printf( "[INFO ] Shutting down...\r\n" );
		FreeLibraryAndExitThread( static_cast< HMODULE >( m_module ), 0ul );
	}
	) ).detach( );
}

// pre-defines so we don't need to include consoleapi.h
#ifdef _DEBUG
extern "C" __declspec( dllimport ) int __stdcall AllocConsole( );
extern "C" __declspec( dllimport ) int __stdcall FreeConsole( );
extern "C" __declspec( dllimport ) int __stdcall AttachConsole( unsigned long );
#endif

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
	if ( !AllocConsole( ) ) {
		if ( GetConsoleWindow( ) != NULL )
			if ( !AttachConsole( -1 ) )
				return; // failed to attach to parent process...
	}

	if ( freopen_s( &m_file[ 0 ], "CONOUT$", "r", stdout ) != 0 ||
		 freopen_s( &m_file[ 1 ], "CONOUT$", "w", stdout ) != 0 )
		return; // failed to redirect stdout/stdin

	try {
		sdk::error_handler::init( );

		const auto& client = sdk::memory::get_module_information( "client.dll" );

		// print the base of client.dll
		std::printf( "[INFO] client.dll base: 0x%llX\r\n", std::get< const sdk::memory::address_t >( client->get< sdk::e_get_module_base >( ) ).get( ) );

		const std::optional< sdk::memory::address_t > CreateInterface = client->find_pattern( sdk::string_to_bytes( "4C 8B 0D ? ? ? ? 4C 8B D2 4C" ) );
		if ( CreateInterface.has_value( ) )
			std::printf( "[INFO] CreateInterface: 0x%llX\r\n", CreateInterface->get( ) );

	} catch ( const std::exception& ex ) {
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
	m_threads[ 0 ].request_stop( );
}
