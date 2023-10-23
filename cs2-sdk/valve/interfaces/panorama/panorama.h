#pragma once
#include <vector>

#include "../../../core/ctx.h"

namespace valve::interfaces::panorama {
	namespace impl {
		struct _panorama_panel_handle_t {
			int m_panel_index;					// 0x00
			unsigned int m_serial_number;		// 0x04

			ALWAYS_INLINE bool is_valid( ) const {
				return m_panel_index != 0 && m_serial_number != 0;
			}
		};

		struct _client_panel_t;

		struct _ui_panel_t {
			const void* vmt;					// 0x00
			_client_panel_t* m_client_panel;	// 0x04
		};

		struct _client_panel_t {
			const void* m_vmt;					// 0x00
			_ui_panel_t* m_panel;				// 0x04
		};
	}

	struct panorama_ui_engine_t {
		static void run_script( panorama_ui_engine_t* _this, impl::_ui_panel_t* panel, const char* src, const char* file = "", std::uint64_t line = 0x0 ) {
			static auto fn = g_ctx->get_offsets( ).load( ).m_panorama.m_run_script;
			if ( fn.is_valid( ) )
				fn.as< void( * )( panorama_ui_engine_t*, impl::_ui_panel_t*, const char*, const char*, std::uint64_t ) >( )( _this, panel, src, file, line );
		}

		static impl::_ui_panel_t* get_main_menu_panel( panorama_ui_engine_t* _this ) {
			static auto _offset = g_ctx->get_offsets( ).load( ).m_panorama.m_main_menu_panel;
			if ( _offset.is_valid( ) )
				return _offset.as< impl::_client_panel_t* >( )->m_panel;

			return nullptr; // bruh.
		}
	};

	void run_script( const char* src, const char* file = "", std::uint64_t line = 0x0 ) {
		static auto _offset = g_ctx->get_offsets( ).load( ).m_panorama.m_engine;
		if ( panorama_ui_engine_t** engine = _offset.as< panorama_ui_engine_t** >( ); engine != nullptr )
			return panorama_ui_engine_t::run_script( *engine, panorama_ui_engine_t::get_main_menu_panel( *engine ), src, file, line );
	}
}