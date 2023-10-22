#pragma once
#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

class c_ctx {
public:
	c_ctx( void* hmodule );
protected:
	std::stop_source m_stop_source;

	void* m_module;

	std::jthread m_threads[ 2 ];

	void init( );

	std::FILE* m_file[ 2 ]; // 0 = in, 1 = out

	struct entity_offsets_t {

	};
public:
	// thread safe initialized check.
	std::atomic< bool > m_ready{ false };

	// thread safe global entity offsets
	std::atomic< entity_offsets_t* > m_entity_offsets{ nullptr };

	std::atomic< struct lua_State* > m_lua_state{ nullptr };
};

inline std::unique_ptr< c_ctx > g_ctx;