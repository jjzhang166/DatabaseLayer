//
// Platform_WIN32.h
#ifndef Platform_WIN32_INCLUDED
#define Platform_WIN32_INCLUDED

// Turn off some annoying warnings
#if	_MSC_VER == 1200		//MS VC++ 6.0 
	#pragma warning(disable:4786) // ʹ��stl��ľ���
	
#endif

#if	_MSC_VER == 1600		//MS VC++ 10.0
  #pragma warning(disable:4996) // ����vs2010ʹ��sprintf�Ⱥ����ľ��� 
#endif

#endif // Platform_WIN32_INCLUDED
