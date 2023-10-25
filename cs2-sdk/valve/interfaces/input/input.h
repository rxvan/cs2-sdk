#pragma once
#include <array>

#include "../../../core/ctx.h"
#include "../../../sdk/sdk.h"

#include "../../util/pb.h"

namespace valve::interfaces::input {
	struct in_button_state_pb_t {
		PAD( 0x18 );						// 0x00
		unsigned __int32 m_buttons;			// 0x18
		PAD( 0xC );							// 0x1C
	};

	struct cmd_qangle_t {
		PAD( 0x18 );						// 0x00
		float m_pitch;						// 0x18
		float m_yaw;						// 0x1C
		float m_roll;						// 0x20
		PAD( 0x41C );						// 0x24
	}; // Size: 0x440

	struct base_user_cmd_t{
		PAD( 0x40 );						// 0x00
		in_button_state_pb_t* m_buttons;	// 0x40
		cmd_qangle_t* m_view_angles;		// 0x44
		float m_unk[ 2 ];					// 0x48
		float m_forward_move;				// 0x50
		float m_side_move;					// 0x54
		float m_unk2[ 5 ];					// 0x58
	}; // Size: 0x6C

	struct subtick_cmd_t {
		PAD( 0x18 );						// 0x00
		cmd_qangle_t* m_view_angles;		// 0x18
	}; // Size: 0x1C

	struct subtick_container_t {
		unsigned __int32 m_size;			// 0x00
		PAD( 0x4 );							// 0x04
		unsigned __int64 m_storage;			// 0x08

		subtick_cmd_t* cmd( int idx ) {
			assert( idx < m_size && "subtick cmd idx out of range!" );
			subtick_cmd_t** storage = reinterpret_cast< subtick_cmd_t** >( m_storage + 0x8 );
			return storage[ idx ];
		}
	};

	struct user_cmd_t {
		PAD( 0x30 );						// 0x00
		base_user_cmd_t* m_base_cmd;		// 0x30
		PAD( 0xC );							// 0x34
		unsigned __int32 m_buttons;			// 0x40
		PAD( 0x3EC );						// 0x44

		constexpr subtick_container_t* subtick_container( ) {
			return reinterpret_cast< subtick_container_t* >( reinterpret_cast< std::uintptr_t >( this ) + 0x20 );
		}

		void set_viewangles( const float viewangles[ 3 ] ) {
			cmd_qangle_t* cmd_viewangles = m_base_cmd->m_view_angles;
			cmd_viewangles->m_pitch = viewangles[ 0 ];
			cmd_viewangles->m_yaw = viewangles[ 1 ];
			cmd_viewangles->m_roll = viewangles[ 2 ];

			subtick_container_t& subtick = *subtick_container( );
			for ( int i = 0; i < subtick.m_size; i++ ) {
				subtick_cmd_t* cmd = subtick.cmd( i );
				cmd->m_view_angles->m_pitch = viewangles[ 0 ];
				cmd->m_view_angles->m_yaw = viewangles[ 1 ];
				cmd->m_view_angles->m_roll = viewangles[ 2 ];
			}
		}
	}; // Size: 0x430

	class csgo_input {
	private:
		PAD( 0x41BD );						// 0x0
	public:
		bool m_camera_in_third_person;		// 0x41BD
		PAD( 0x6 );							// 0x41BE
		float m_third_person_angle[ 3 ];	// 0x41C4

		user_cmd_t* get_user_cmd( ) {
			return *reinterpret_cast< user_cmd_t** >( reinterpret_cast< std::uintptr_t >( this ) + 0xF8 );
		}
	};
}

namespace valve {

}