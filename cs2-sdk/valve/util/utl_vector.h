#pragma once

namespace valve::util {
	template <typename _type_t>
	struct utl_memory {
		_type_t* memory;
		int allocation_count;
		int grow_size;
	};

	template <typename _type_t>
	struct utl_vector {
		int size;
		utl_memory* memory;
		int allocation_count;
		int grow_size;
	};
}