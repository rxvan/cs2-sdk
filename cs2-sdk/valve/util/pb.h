#pragma once
#include <limits>

namespace valve::util {
	struct pb_base_t {
		void* m_vtable;                                                 // 0x0
		bool m_has_data;                                                // 0x4
		unsigned long m_size;										    // 0x8
	}; // Size: 0xC

    template< typename _data_type_t >
    struct repeated_field_t {
        struct repeated_t {
            int m_allocated_size;
            _data_type_t* elements[ ( INT_MAX - 2 * sizeof( int ) ) /
                sizeof( void* ) ];
        }; // Size: 0x1000

        void* arena;                                                    // 0x0
		int m_current_size;											    // 0x4
        int m_total_size;												// 0x8
        repeated_t* repeated_field;										// 0xC
    };// Size: 0x10

    struct pb_qangle_t : pb_base_t {
        float x, y, z;                                                  // 0xC - 0x14 - 0x1C - 0x20 - 0x24 - 0x28 - 0x2C = 0x30
    }; // Size: 0x30

    struct pb_vector_t : public pb_qangle_t{};                          // Size: 0x30

    struct pb_button_state_t : pb_base_t {
        unsigned long m_buttons[ 3 ];                                   // 0xC
    }; // Size: 0x18
}