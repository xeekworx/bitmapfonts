#pragma once

#ifdef XWFONTLIBRARY_EXPORTS
#	define XWFONTAPI __declspec(dllexport)
#else
#	ifdef XWFONTLIBRARY_SHARED
#		define XWFONTAPI __declspec(dllimport)
#	else
#		define XWFONTAPI
#	endif
#endif

namespace xeekworx {
	extern "C" {

		XWFONTAPI void test_xwfontapi(void);

	}
}