#pragma once

#include <cstdint>


namespace abc {

	typedef std::uint16_t	status_t;

	namespace status {
		constexpr status_t min_success		= 0x0000;
		constexpr status_t success			= 0x0000;
		constexpr status_t custom_success	= 0x0010;
		// constexpr status_t sample_custom_success	= custom_success + 0x0001;
		constexpr status_t max_success		= 0x00FF;

		constexpr status_t bad_input		= 0x0101;
		constexpr status_t bad_state		= 0x0102;
		constexpr status_t abort			= 0x0103;
		constexpr status_t not_found		= 0x0104;
		constexpr status_t unexpected		= 0x0105;
		constexpr status_t out_of_memory	= 0x0106;
		constexpr status_t assert_failed	= 0x0107;
		constexpr status_t todo				= 0x0108;
		constexpr status_t custom_error		= 0x1000;
		// constexpr status_t sample_custom_error	= custom_error + 0x0001;

		inline bool succeeded(status_t st) noexcept {
			return min_success <= st && st <= max_success;
		}

		inline bool failed(status_t st) noexcept {
			return !succeeded(st);
		}
	}


	typedef std::uint8_t	severity_t;

	namespace severity {
		constexpr severity_t debug_abc	= 0x1;
		constexpr severity_t debug		= 0x4;
		constexpr severity_t info		= 0x7;
		constexpr severity_t warning	= 0xA;
		constexpr severity_t critical	= 0xC;
	}


	typedef std::uint16_t	category_t;

	namespace category {
		constexpr category_t log		= 0x0001;
		constexpr category_t timestamp	= 0x0002;
		constexpr category_t custom		= 0x8000;
		// constexpr category_t sample_custom_category	= custom + 0x0001;
	}


	typedef std::uint32_t	tag_t;

}

