#ifndef _FAST_EVENT_SYSTEM_API_
#define _FAST_EVENT_SYSTEM_API_

#if defined(_WIN32) || defined(WIN32)
#ifdef fast_event_system_EXPORTS
#define fast_event_system_API __declspec(dllexport)
#else
#ifndef fast_event_system_STATIC
#define fast_event_system_API __declspec(dllimport)
#else
#define fast_event_system_API
#endif
#endif
#else
#define fast_event_system_API
#endif

#ifdef WINDOWS

	typedef __int64				Int64;
	typedef signed int			Int32;
	typedef signed short		Int16;
	typedef signed char			Int8;

	typedef unsigned __int64	UInt64;
	typedef unsigned int		Uint32;
	typedef unsigned short		Uint16;
	typedef unsigned char		Uint8;

	typedef double				Real64;
	typedef float				Real32;

#else

	typedef long long			Int64;
	typedef signed int			Int32;
	typedef signed short		Int16;
	typedef signed char			Int8;

	typedef unsigned long long	UInt64;
	typedef unsigned int		Uint32;
	typedef unsigned short		Uint16;
	typedef unsigned char		Uint8;

	typedef double				Real64;
	typedef float				Real32;

#endif

typedef Int32 Int;
typedef Uint32 Uint;
typedef Real32 Real;

#endif
