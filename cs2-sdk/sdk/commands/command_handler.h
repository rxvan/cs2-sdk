#pragma once
#include <type_traits>
#include <functional>
#include <map>

namespace sdk {
	namespace command_handler {
		namespace impl {
			std::map< std::string, std::function< void( const std::vector< std::string >& ) > > commands;

			template< typename T >
			struct function_traits : function_traits< decltype( &T::operator() ) > { };
			template< typename ClassType, typename ReturnType, typename... Args >
			struct function_traits< ReturnType( ClassType::* )( Args... ) const > {
				static const size_t arity = sizeof...( Args );
				using result_type = ReturnType;
				template< size_t i >
				struct arg {
					using type = typename std::tuple_element< i, std::tuple< Args... > >::type;
				};
			};
			template< typename T >
			struct is_callable {
				template< typename U >
				static auto test( U* p ) -> decltype( ( *p )( ), std::true_type( ) );
				template< typename U >
				static auto test( ... ) -> std::false_type;
				static const bool value = decltype( test< T >( nullptr ) )::value;
			};

			template< typename T >
			struct is_function : std::integral_constant< bool, std::is_function< typename std::remove_pointer< T >::type >::value > { };


		}

		ALWAYS_INLINE void init( );

		ALWAYS_INLINE bool run_command( const std::string& command );
	}
}

#include "impl/command_handler.inl"
