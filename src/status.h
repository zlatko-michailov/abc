#pragma once

#include <cstdint>


namespace abc {

	typedef std::uint16_t status_t;


	namespace severity {
		static const status_t mask		= 0xF000;

		static const status_t abc		= 0x1000;
		static const status_t verbose	= 0x4000;
		static const status_t info		= 0x7000;
		static const status_t warning	= 0xA000;
		static const status_t critical	= 0xC000;
	}


	namespace status {
		static const status_t custom			= 0x800;

		static const status_t debug				= severity::abc | 0x001;

		static const status_t success			= severity::verbose | 0x0000;
		static const status_t custom_verbose	= severity::verbose | custom;

		static const status_t bad_input			= severity::warning | 0x001;
		static const status_t not_ready			= severity::warning | 0x002;
		static const status_t not_found			= severity::warning | 0x003;
		static const status_t custom_warning	= severity::warning | custom;

		static const status_t unexpected		= severity::critical | 0x001;
		static const status_t out_of_memory		= severity::critical | 0x002;
		static const status_t custom_critical	= severity::critical | custom;

	}


	status_t assert(bool condition) noexcept {
		return condition ? status::success : status::unexpected;
	}

} // namespace abc


#define abc_check(st, category, tag) { \
	abc::status_t _st = (st); \
	if ((_st & abc::severity::mask) > abc::severity::success) { \
		abc::log::global::push((category), (tag), _st); \
		return _st; \
	} \
} \

