#pragma once

class c_ctx {
public:
	c_ctx( );
protected:
	void init( );

	std::FILE* m_file;

	struct entity_offsets_t {

	};
public:
	// thread safe initialized check.
	std::atomic< bool > m_ready{ false };

	std::atomic< entity_offsets_t* > m_entity_offsets{ nullptr };
};

inline std::unique_ptr< c_ctx > g_ctx;