#include "timestamp.h"


namespace abc { namespace test { namespace timestamp {

	//template <typename Log>
	bool test_null_timestamp(test_context<abc::test_log>& context) {
		abc::timestamp ts(nullptr);

		bool passed = true;
		passed = context.are_equal<std::int32_t>(ts.year(),			1970, 0x101, "%d") && passed;
		passed = context.are_equal<std::int32_t>(ts.month(),		   1, 0x102, "%d") && passed;
		passed = context.are_equal<std::int32_t>(ts.day(),			   1, 0x103, "%d") && passed;

		passed = context.are_equal<std::int32_t>(ts.hours(),		   0, 0x104, "%d") && passed;
		passed = context.are_equal<std::int32_t>(ts.minutes(),		   0, 0x105, "%d") && passed;
		passed = context.are_equal<std::int32_t>(ts.seconds(),		   0, 0x106, "%d") && passed;
		passed = context.are_equal<std::int32_t>(ts.milliseconds(),	   0, 0x107, "%d") && passed;
		passed = context.are_equal<std::int32_t>(ts.microseconds(),	   0, 0x108, "%d") && passed;
		passed = context.are_equal<std::int32_t>(ts.nanoseconds(),	   0, 0x109, "%d") && passed;

		return true;
	}

}}}

