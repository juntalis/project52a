#pragma once

#define PP_WIDEN2(X) L##X
#define PP_WIDEN(X) PP_WIDEN2(X)

#define PP_STR2(X) #X
#define PP_STRA(X) PP_STR2(X)
#define PP_STRW(X) PP_WIDEN(PP_STRA(X))

#define WSIZE(X) (sizeof(wchar_t) * (X))
#define WLEN(X) ((X) / sizeof(wchar_t))

// MSVC Version Detection
#if defined(_MSC_VER) && !defined(RC_INVOKED)
#	if (_MSC_VER < 1500) && !defined(OLD_MSVC_PROCESSED)
//		TODO: Write a script to preprocess the headers for earlier versions of MSVC.
#		error Steps are required to build this project using a version of MSVC prior to VC 15.0. (Visual Studio 2008) See the README for more details.
#	elif _MSC_VER == 1200
#		define COMPILER_VC6
#	elif _MSC_VER == 1300
#		define COMPILER_VC7
#	elif _MSC_VER == 1310
#		define COMPILER_VC2003
#	elif (_MSC_VER >= 1400) && (_MSC_VER < 1500)
#		define COMPILER_VC2005
#	elif (_MSC_VER >= 1500) && (_MSC_VER < 1600)
#		define COMPILER_VC2008
#	elif (_MSC_VER >= 1600) && (_MSC_VER < 1700)
#		define COMPILER_VC2010
#	elif (_MSC_VER >= 1700) && (_MSC_VER < 1800)
#		define COMPILER_VC2012
#	else
#		pragma message("_MSC_VER defined as " PP_STRA(_MSC_VER))
#		error Did not recognize MSVC version!
#	endif
#	define PP_WARN(M) \
		__pragma(message( __FILE__ "(" PP_STRA(__LINE__) ")" " : user warning: " M ))
#elif defined(RC_INVOKED)
#	define COMPILER_RC
#else
#	define PP_WARN(M) \
		__Pragma(message( __FILE__ "(" PP_STRA(__LINE__) ")" " : user warning: " M ))
#endif

