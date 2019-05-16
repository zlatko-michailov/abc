#pragma once

#include "base.h"
#include "log.h"


#define abc_assert_ret(cond, cat, tag, ret) { \
	if (!(cond)) { \
		abc::legacy_log::diag.push(abc::severity::critical, (cat), (tag), abc::status::assert_failed); \
		return (ret); \
	} \
}

#define abc_assert_void(cond, cat, tag) { \
	if (!(cond)) { \
		abc::legacy_log::diag.push(abc::severity::critical, (cat), (tag), abc::status::assert_failed); \
		return; \
	} \
}

#define abc_assert(cond, cat, tag) abc_assert_ret((cond), (cat), (tag), abc::status::assert_failed)


#define abc_check(st, sev, cat, tag) { \
	abc::status_t _st = (st); \
	if (abc::status::succeeded(_st)) { \
		abc::legacy_log::diag.push(abc::severity::debug, (cat), (tag), _st); \
	} \
	else { \
		abc::legacy_log::diag.push((sev), (cat), (tag), _st); \
		return _st; \
	} \
}

#define abc_critical(st, cat, tag) abc_check((st), abc::severity::critical, (cat), (tag))

#define abc_warning(st, cat, tag) abc_check((st), abc::severity::warning, (cat), (tag))
