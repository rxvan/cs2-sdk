#include "../../sdk.h"
#include "../../../core/ctx.h"

#include "../command_handler.h"

namespace sdk {
	constexpr std::vector<std::string> command_handler::impl::split_arguments( const std::string& str ) {
		// find the first space (" ")
		auto first_space = str.find( ' ' );
		if ( first_space == std::string::npos )
			return { };

		// find the last space (" ")
		auto last_space = str.rfind( ' ' );
		if ( last_space == std::string::npos )
			return { };

		int nargs = 0;
		if ( first_space == last_space )
			nargs = 1;
		else {
			// count the number of spaces in the string
			for ( auto i = first_space; i <= last_space; ++i )
				if ( str[ i ] == ' ' )
					++nargs;
		}

		std::vector< std::string > args( nargs );

		// copy each argument into the vector
		auto start = first_space + 1;
		for ( auto i = 0; i < nargs; ++i ) {
			auto end = str.find( ' ', start );
			if ( end == std::string::npos )
				end = str.length( );

			args[ i ] = str.substr( start, end - start );
			start = end + 1;
		}

		return args;
	}

	void command_handler::init( ) {
		command_handler::impl::commands =
		{
			{
				"exit",
				impl::_exit
			},
			{
				"quit",
				impl::_quit
			}
		};
	}

	bool command_handler::run_command( const std::string& command ) {
		ASSERT( !command.empty( ) );

		std::string cmd;
		if ( auto command_name_index = command.find_first_of( ' ' ); command_name_index != std::string::npos ) {
			cmd = command.substr( 0, command_name_index );
		}
		else {
			cmd = command;
		}

		if ( auto command_it = impl::commands.find( cmd ); command_it != impl::commands.end( ) ) {
			command_it->second( impl::split_arguments( command ) );
			return true;
		}

		std::printf( "%s not found!\r\n", cmd.data( ) );
		return false;
	}
	
	bool command_handler::register_command( const std::string& command, std::function<void( const std::vector<std::string>& )> callback ) {
		impl::commands.insert( { command, callback } );
		return impl::commands.find( command ) != impl::commands.end( );
	}
}
