#include "../error_handler.h"
#ifndef ALWAYS_INLINE
#include "../../sdk.h"
#endif // !ALWAYS_INLINE


#include <signal.h>

namespace sdk {

}

#define _quote( x ) #x
#define quote( x ) _quote( x )
#define concat( x ) { x, quote( x ) }

ALWAYS_INLINE const char* exception_to_string( int exception ) {
	static std::map< int, const char* > exception_strings = {
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

	return exception_strings.find( exception ) != exception_strings.end( ) ? exception_strings[ exception ] : "UNKNOWN";
}

namespace sdk::error_handler {
	static LONG WINAPI vectored_exception_handler( EXCEPTION_POINTERS* exception_info ) {

		// i didnt write this bit, copilot did, if it doesnt work blame him:-)
		if ( exception_info->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW ) {
			uintptr_t stack_pointer = exception_info->ContextRecord->Rsp;
			uintptr_t stack_base = stack_pointer & ~( 0x1000 - 1 );
			uintptr_t stack_limit = stack_base - 0x100000;

			for ( uintptr_t stack_pointer = stack_base; stack_pointer > stack_limit; stack_pointer -= 0x1000 ) {
				// check if we can write to this page
				MEMORY_BASIC_INFORMATION mbi;
				if ( !VirtualQuery( reinterpret_cast< void* >( stack_pointer ), &mbi, sizeof( mbi ) ) )
					break;

				// if we can, write a null byte and return
				if ( mbi.Protect & ( PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY ) ) {
					*reinterpret_cast< char* >( stack_pointer ) = '\0';
					return EXCEPTION_CONTINUE_EXECUTION;
				}
			}
			// we failed to find a place to write to, so just return EXCEPTION_EXECUTE_HANDLER to let
			// the exception handler catch it
			return EXCEPTION_EXECUTE_HANDLER;
		}
		else {
			// important registers are:
			// Rsp		- stack pointer
			// Rbp		- base pointer
			// Rip		- instruction pointer
			// Rdi		- first argument
			// Rsi		- second argument
			// Rdx		- third argument
			// Rcx		- fourth argument
			// R8		- fifth argument
			// R9		- sixth argument
			// R10		- seventh argument
			// R11		- eighth argument
			// Rax		- return value
			// Rbx		- callee saved
			// RFlags	- flags register
			static constexpr std::string_view fmt = R"(
Exception caught:
	Exception code: 0x%08X (%s)
	Exception address: 0x%016llX
	Stack pointer: 0x%016llX
	Stack base: 0x%016llX
	Stack limit: 0x%016llX
	Stack size: 0x%016llX
	Registers:
		Rax: 0x%016llX
		Rbx: 0x%016llX
		Rcx: 0x%016llX
		Rdx: 0x%016llX
		Rsi: 0x%016llX
		Rdi: 0x%016llX
		Rbp: 0x%016llX
		Rsp: 0x%016llX
		R8: 0x%016llX
		R9: 0x%016llX
		R10: 0x%016llX
		R11: 0x%016llX
                                                                                                                                                                      \\\\\\\\\\\\\)";

			const CONTEXT* record = exception_info->ContextRecord;
			const EXCEPTION_RECORD* exception = exception_info->ExceptionRecord;
			const uintptr_t stack_base = record->Rsp & ~( 0x1000 - 1 ),
				stack_limit = stack_base - 0x100000,
				stack_size = stack_base - stack_limit;

			std::printf( fmt.data( ), exception->ExceptionCode, exception_to_string( exception->ExceptionCode ), *reinterpret_cast< std::uintptr_t* >( exception->ExceptionAddress ),
				record->Rsp, stack_base, stack_limit, stack_size,
				record->Rax, record->Rbx, record->Rcx, record->Rdx, record->Rsi, record->Rdi, record->Rbp, record->Rsp, record->R8, record->R9, record->R10, record->R11 );

			// return
			return EXCEPTION_EXECUTE_HANDLER;
		}
	}

	ALWAYS_INLINE void init( ) {
		AddVectoredExceptionHandler( 1, vectored_exception_handler );

		// also add a signal handler for SIGSEGV
		signal( SIGSEGV, [ ]( int ) {
			CONTEXT context;
			context.ContextFlags = CONTEXT_ALL;
			RtlCaptureContext( &context );
			sdk::memory::address_t address = static_cast< std::uint64_t >( context.Rip );
			std::printf( "SIGSEGV caught at 0x%016llX\n", address( ) );
		} );
	}
}