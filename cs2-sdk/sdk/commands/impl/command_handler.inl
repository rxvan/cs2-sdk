#include <assert.h>

#include "../../sdk.h"

#include "../command_handler.h"



namespace sdk::command_handler {
	ALWAYS_INLINE void init( ) {

	}

	ALWAYS_INLINE bool run_command( const std::string& command ) {
		ASSERT( !command.empty( ) );

		if ( command.find( ' ' ) != std::string::npos ) {
			const std::string& cmd = command.substr( 0, command.find_first_of( ' ' ) );
		}
	}
}