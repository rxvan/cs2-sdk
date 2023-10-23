#pragma once
#include <functional>

#include "detail/base.h"

namespace sdk {
	namespace memory {
		using address_t = detail::base_address_t<>;

		namespace impl {
			ALWAYS_INLINE static PEB* get_peb( );

			ALWAYS_INLINE static void for_each_module( const std::function< void( const module_t& ) >& fn );
		}

		ALWAYS_INLINE sdk::optional_t< module_t > get_module_information( const char* module_name ); // i want to use a hash here but api-accessability is more important imo (hashing inside the fnuction is easier than lazy people complaining because they cant call util.hash(...) even though this is in theory more resource intensive :)))
		ALWAYS_INLINE sdk::optional_t< module_t > get_module_information( void* base_address ); // used in hooks, for example, as a quick way of identifying where it was called from

		template < typename _interface_class = void >
		ALWAYS_INLINE sdk::optional_t< _interface_class* > const get_interface( const module_t& mod, const char* interface_name );
	}
}

#include "impl/memory.inl"
