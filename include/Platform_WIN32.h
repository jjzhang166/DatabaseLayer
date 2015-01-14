//
// Platform_WIN32.h
#ifndef Platform_WIN32_INCLUDED
#define Platform_WIN32_INCLUDED

// Turn off some annoying warnings
#if	_MSC_VER == 1200		//MS VC++ 6.0 
	#pragma warning(disable:4786) // 使用stl库的警告
	
#endif

#if	_MSC_VER == 1600		//MS VC++ 10.0
  #pragma warning(disable:4996) // 消除vs2010使用sprintf等函数的警告 
#endif

#endif // Platform_WIN32_INCLUDED
