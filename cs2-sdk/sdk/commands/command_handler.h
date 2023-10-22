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
			inline std::map< std::string, std::function< void( const std::vector< std::string >& ) > > commands{};

			constexpr std::vector< std::string > split_arguments( const std::string& str );

			static const auto _exit = [ ]( const std::vector< std::string >& args ) -> void {
				g_ctx->get_stop_source( ).request_stop( );
			};

			static const auto _quit = [ ]( const std::vector< std::string >& args ) -> void {
				if ( args.empty( ) )
					return exit( 0 );
				else {
					// get the exit code
					const auto exit_code = std::stoi( args[ 0 ] );
					return exit( exit_code );
				}
			};
		}

		void init( );

		bool run_command( const std::string& command );
		bool register_command( const std::string& command, std::function< void( const std::vector< std::string >& ) > callback );
	}
}

#include "impl/command_handler.inl"
