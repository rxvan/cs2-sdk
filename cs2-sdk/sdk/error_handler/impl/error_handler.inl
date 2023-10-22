#include <signal.h>

#include "../../sdk.h"

#include "../error_handler.h"



namespace sdk {

}

#define _quote( x ) #x
#define quote( x ) _quote( x )
#define concat( x ) { x, quote( x ) }

static const std::map< int, const char* > exception_strings = {
	concat( EXCEPTION_ACCESS_VIOLATION ),
	concat( EXCEPTION_ARRAY_BOUNDS_EXCEEDED ),
	concat( EXCEPTION_DATATYPE_MISALIGNMENT ),
	concat( EXCEPTION_FLT_DENORMAL_OPERAND ),
	concat( EXCEPTION_FLT_DIVIDE_BY_ZERO ),
	concat( EXCEPTION_FLT_INEXACT_RESULT ),
	concat( EXCEPTION_FLT_INVALID_OPERATION ),
	concat( EXCEPTION_FLT_OVERFLOW ),
	concat( EXCEPTION_FLT_STACK_CHECK ),
	concat( EXCEPTION_FLT_UNDERFLOW ),
	concat( EXCEPTION_ILLEGAL_INSTRUCTION ),
	concat( EXCEPTION_IN_PAGE_ERROR ),
	concat( EXCEPTION_INT_DIVIDE_BY_ZERO ),
	concat( EXCEPTION_INT_OVERFLOW ),
	concat( EXCEPTION_INVALID_DISPOSITION ),
	concat( EXCEPTION_NONCONTINUABLE_EXCEPTION ),
	concat( EXCEPTION_PRIV_INSTRUCTION ),
	concat( EXCEPTION_STACK_OVERFLOW )
};

static ALWAYS_INLINE const char* exception_to_string( int exception ) {
	return exception_strings.find( exception ) != exception_strings.end( ) ? exception_strings.at( exception ) : "UNKNOWN";
}

namespace sdk::error_handler {
    static LONG WINAPI vectored_exception_handler( EXCEPTION_POINTERS* exception_info ) {
        static constexpr char fmt[ ] = R"(Exception caught at 0x%016llX:
    Code: 0x%08X (%s)
    Flags: 0x%08X
    Params: 0x%016llX 0x%016llX 0x%016llX 0x%016llX
    -----------------------------------------------
    |               DEBUG INFORMATION              |
    -----------------------------------------------
    Register values:
        Stack Pointer: 0x%016llX
        Instruction Pointer: 0x%016llX
        Return Value: 0x%016llX
        Base Pointer: 0x%016llX
        Source Index: 0x%016llX
        Destination Index: 0x%016llX
        Flags: 0x%016llX
        Stack Base: 0x%016llX
        Stack Limit: 0x%016llX
        Stack Allocation Base: 0x%016llX
        Stack Allocation Size: 0x%016llX
        Stack Allocation Protect: 0x%08X
        Stack Allocation State: 0x%08X
)";

        const auto& context = *exception_info->ContextRecord;
        const auto& exception = *exception_info->ExceptionRecord;

        MEMORY_BASIC_INFORMATION stack_info;
        VirtualQuery( reinterpret_cast< void* >( context.Rsp ), &stack_info, sizeof( stack_info ) );

        // Print the exception information
        printf( fmt,
            reinterpret_cast< uintptr_t >( exception.ExceptionAddress ),    // address
            exception.ExceptionCode,                                        // code
            exception_to_string( exception.ExceptionCode ),                 // code string
            exception.ExceptionFlags,                                       // flags
            exception.ExceptionInformation[ 0 ],                            // param 1
            exception.ExceptionInformation[ 1 ],                            // param 2
            exception.ExceptionInformation[ 2 ],                            // param 3
            exception.ExceptionInformation[ 3 ],                            // param 4
            context.Rsp,                                                    // stack pointer
            context.Rip,                                                    // instruction pointer
            context.Rax,                                                    // return value
            context.Rbp,                                                    // base pointer
            context.Rsi,                                                    // source index
            context.Rdi,                                                    // destination index
            context.EFlags,                                                 // flags
            reinterpret_cast< uintptr_t >( stack_info.BaseAddress ),        // stack base
            stack_info.RegionSize,                                          // stack limit
            reinterpret_cast< uintptr_t >( stack_info.AllocationBase ),     // stack allocation base
            stack_info.RegionSize,                                          // stack allocation size
            stack_info.AllocationProtect,                                   // stack allocation protect
            stack_info.State                                                // stack allocation state
        );

        // Get the module in which the exception was thrown and the base file name of the module

        // Return
        return EXCEPTION_EXECUTE_HANDLER;
    }

	ALWAYS_INLINE void init( ) {
		AddVectoredExceptionHandler( 1, vectored_exception_handler );

		// also add a signal handler for SIGSEGV
		signal( SIGSEGV, [ ]( int ) {
			CONTEXT context;
			context.ContextFlags = CONTEXT_CONTROL; // we only need to access the instruction pointer (rip) so we only need CONTEXT_CONTROL
			RtlCaptureContext( &context );
			std::printf( "SIGSEGV caught at 0x%016llX\n", context.Rip );
			}
        );
	}
}