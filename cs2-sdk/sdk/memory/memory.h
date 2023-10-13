#pragma once
#include <functional>

namespace sdk {
	namespace memory {
		// forward declare
		struct _base_module_t;

		template <typename _base_type_t>
		struct address_64_t;

		template <typename _base_type_t = std::uintptr_t>
		using address_t = address_64_t< _base_type_t >;
	}

	ALWAYS_INLINE std::add_pointer_t< PEB > _peb( );
}

namespace sdk::memory {

	/*
	* @brief - rxvan: base address class, handles major address operations (i.e. dereferencing, offsetting, etc.)
	* @note - this struct is not meant to be used directly, instead use the address_(32/64)_t struct
	*/
	template <typename _base_type_t = std::uintptr_t>
	struct _base_address_t {
	private:
		_base_type_t m_address;
	public:
		constexpr _base_address_t( ) = default;

		constexpr _base_address_t( const _base_type_t address ) : m_address( address ) {}

		template <typename _cast_type_t = void*>
		ALWAYS_INLINE _cast_type_t as( ) {
			return reinterpret_cast< _cast_type_t >( m_address );
		}

		ALWAYS_INLINE _base_address_t< _base_type_t >& self_deref( ptrdiff_t count );

		constexpr ALWAYS_INLINE _base_address_t< _base_type_t >& self_offset( ptrdiff_t offset );

		ALWAYS_INLINE _base_type_t operator()( ) const {
			return m_address;
		}

		operator bool( ) const {
			return m_address > _base_type_t( 0x0 );
		}
	};

	template <typename _base_type_t = std::uint64_t>
	struct address_64_t : public _base_address_t< _base_type_t > {
		constexpr address_64_t( ) : _base_address_t< _base_type_t >( ) {}
		constexpr address_64_t( const _base_type_t address ) : _base_address_t< _base_type_t >( address ) {}
	public:
		ALWAYS_INLINE address_64_t deref( ptrdiff_t count );

		ALWAYS_INLINE address_64_t offset( ptrdiff_t offset );

		// operator from _base_address_t to address_64_t
		ALWAYS_INLINE address_64_t( const _base_address_t< _base_type_t >& address ) : _base_address_t< _base_type_t >( address( ) ) {}
	};

	/*
	* @brief - rxvan: base module class, handles major module operations (i.e. get exports, get imports, get base address, etc.)
	* @note - this struct is not meant to be used directly, instead use the module_t struct
	*/
	struct _base_module_t {
	protected:
		std::size_t m_size;
		address_t< > m_base;
		const std::string m_name;

		IMAGE_NT_HEADERS64* m_nt_headers;
		IMAGE_DOS_HEADER* m_dos_header;
	public:
		constexpr ALWAYS_INLINE _base_module_t( const std::string& module_name ) :
			m_name( module_name ),
			m_base( 0x0 ),
			m_size( 0x0 ),
			m_nt_headers( nullptr ),
			m_dos_header( nullptr )
		{ }

		ALWAYS_INLINE bool load( bool throw_on_fail = true );

		ALWAYS_INLINE void for_each_export( const std::function< void( const char*, address_t< std::uintptr_t > ) >& fn );

		ALWAYS_INLINE address_t< > get_export( const char* name );

		 /*template < typename _array >
		 ALWAYS_INLINE address_t< > find_pattern( const _array& bytes );

		template < const char* pattern, std::size_t _size = 0u >
		ALWAYS_INLINE address_t< > find_pattern( std::array< std::uint8_t*, _size > = stoba::make< pattern >( ) );*/

		ALWAYS_INLINE address_t< > find_pattern( const std::vector< std::uint8_t > pattern_bytes);

		ALWAYS_INLINE void free( bool exit_thread = true ) {
			if ( exit_thread )
				FreeLibraryAndExitThread( m_base.as< HMODULE >( ), 0 );
			else
				FreeLibrary( m_base.as< HMODULE >( ) );
		}
	};

	/*
	* @brief - rxvan: parent module class, allows separate loading of the module
	*/
	struct module_t : public _base_module_t {
		ALWAYS_INLINE module_t( bool load_on_construct = false, const char* name = nullptr );
	};
}

#include "impl/memory.inl"