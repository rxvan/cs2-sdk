#pragma once

namespace sdk {
	enum e_exception_level : int {
		exception_level_none = 0,
		exception_level_warning,
		exception_level_error,
		exception_level_fatal,
		exception_level_not_implemented,
#ifdef _DEBUG
		exception_level_debug
#endif
	};
}

namespace sdk::error_handler {
	ALWAYS_INLINE void init( );
}

#include "impl/error_handler.inl"
