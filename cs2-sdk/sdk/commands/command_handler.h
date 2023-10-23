#pragma once
#include <type_traits>
#include <functional>
#include <map>
#include <string>

#include "../../core/ctx.h"

#define MAX_ARGS 4

namespace sdk {
	namespace command_handler {
		namespace impl {
			struct cmd_result_t {
				static enum e_return_state : int {
					e_return_success = 0,
					e_return_unknown_command,
					e_return_invalid_arg,
					e_return_missing_arg
				};

				e_return_state state;
				const char* msg;

				cmd_result_t( e_return_state _state = e_return_success ) :
					state( _state )
				{ }
			};

			inline std::map< std::string, std::function< cmd_result_t( const std::vector< std::string >& ) > > commands{};

			constexpr std::vector< std::string > split_arguments( const std::string& str );

			static const auto _exit = [ ]( const std::vector< std::string >& args ) -> cmd_result_t {
				cmd_result_t result{ };
				g_ctx->get_stop_source( ).request_stop( );
				return result;
			};

			static const auto _quit = [ ]( const std::vector< std::string >& args ) -> cmd_result_t {
				cmd_result_t result{ };

				if ( args.empty( ) ) {
					exit( 0 );
				}
				else {
					// get the exit code
					try {
						const auto exit_code = std::stoi( args[ 0 ] );
						exit( exit_code );
					}
					catch ( const std::exception& ) {
						result.state = cmd_result_t::e_return_invalid_arg;
						result.msg = sdk::util::format( "arg 0 invalid! expected int, received %s.\r\n", args[ 0 ] ).data( );
					}
				}
			};
		}

		void init( );

		std::optional< const char* > run_command( const std::string& command );
		bool register_command( const std::string& command, std::function< impl::cmd_result_t( const std::vector< std::string >& ) > callback );
	}
}
