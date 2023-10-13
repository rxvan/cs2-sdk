#include <memory>
#include <thread>

#include "ctx.h"

#include "../sdk/sdk.h"

c_ctx::c_ctx( ) :
	m_file( nullptr )
{
	std::jthread( &c_ctx::init, this ).detach( );
}

// pre-defines so we don't need to include consoleapi.h
#ifdef _DEBUG
extern "C" __declspec( dllimport ) int __stdcall AllocConsole( );
extern "C" __declspec( dllimport ) int __stdcall FreeConsole( );
extern "C" __declspec( dllimport ) HWND __stdcall GetConsoleWindow( );
extern "C" __declspec( dllimport ) int __stdcall AttachConsole( unsigned long );
#endif

int __stdcall DllMain( void* module, unsigned long reason, void* reserved ) {
	if ( reason != 1 )
		return 0;

	try {
		g_ctx = std::make_unique< c_ctx >( );
	}
	catch ( ... ) {
		return 0;
	}

	return 1;
}

void c_ctx::init( ) {
#ifdef _DEBUG
	if ( !AllocConsole( ) ) {
		if ( GetConsoleWindow( ) != NULL )
			if ( !AttachConsole( ATTACH_PARENT_PROCESS ) )
				return;
	}

	// redirect stdout to console
	if ( freopen_s( &m_file, "CONOUT$", "w", stdout ) != 0 )
		return;
#endif

	try {
		sdk::error_handler::init( );

	} catch ( std::exception& ex ) {
		// print error in red text using printf [ERROR]
#ifdef _DEBUG
		std::printf( "[ERROR] %s\r\n", ex.what( ) );
#else
		// create a dump at some point in the future.
#endif
		m_ready.store( false );
		ExitThread( EXIT_FAILURE );
	}

	m_ready.store( true );
}
