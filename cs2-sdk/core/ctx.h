#pragma once
#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>
#include <thread>

#include "../sdk/memory/memory.h"

class c_ctx {
public:
	c_ctx( void* hmodule );

	struct offsets_t {
		struct {
			sdk::memory::address_t	m_engine{},
				m_main_menu_panel{},
				m_run_script{};
		} m_panorama{};
	};
protected:
	std::stop_source m_stop_source;

	void* m_module;

	std::jthread m_threads[ 2 ];

	void init( );

	std::FILE* m_file[ 2 ]; // 0 = in, 1 = out
public:
	// thread safe initialized check.
	std::atomic< bool > m_ready{ false };

	std::atomic< struct lua_State* > m_lua_state{ nullptr };

	sdk::reference_t< std::stop_source > get_stop_source( ) {
		return m_stop_source;
	}

	sdk::atomic_reference_t< offsets_t > get_offsets( ) {
		static std::atomic< offsets_t > offsets{ };
		return offsets;
	}
};

inline std::unique_ptr< c_ctx > g_ctx;