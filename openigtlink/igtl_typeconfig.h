/*=========================================================================

  Program:   The OpenIGTLink Library
  Language:  C

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __IGTL_TYPECONFIG_H
#define __IGTL_TYPECONFIG_H

#define CMAKE_SIZEOF_CHAR
#ifdef CMAKE_SIZEOF_CHAR
  #define IGTL_SIZEOF_CHAR    1
#endif

#define CMAKE_SIZEOF_INT
#ifdef CMAKE_SIZEOF_INT
  #define IGTL_SIZEOF_INT     4
#endif

#define CMAKE_SIZEOF_SHORT
#ifdef CMAKE_SIZEOF_SHORT
  #define IGTL_SIZEOF_SHORT   2
#endif

#define CMAKE_SIZEOF_LONG
#ifdef CMAKE_SIZEOF_LONG
  #define IGTL_SIZEOF_LONG    4
#endif

#define CMAKE_SIZEOF_FLOAT
#ifdef CMAKE_SIZEOF_FLOAT
  #define IGTL_SIZEOF_FLOAT   4
#endif

#define CMAKE_SIZEOF_DOUBLE
#ifdef CMAKE_SIZEOF_DOUBLE
  #define IGTL_SIZEOF_DOUBLE  8
#endif

#define CMAKE_SIZEOF_LONG_LONG
#define CMAKE_SIZEOF___INT64
#define CMAKE_SIZEOF_INT64_T
#ifdef CMAKE_SIZEOF_LONG_LONG
  #define IGTL_TYPE_USE_LONG_LONG 1
  #define IGTL_SIZEOF_LONG_LONG   8
#elif defined(CMAKE_SIZEOF___INT64)
  #define IGTL_TYPE_USE___INT64   1
  #define IGTL_SIZEOF___INT64     8
#elif defined(CMAKE_SIZEOF_INT64_T)
  #define IGTL_TYPE_USE_INT64_T   1
  #define IGTL_SIZEOF_INT64_T     8
#endif

#define CMAKE_SIZEOF_VOID_P

#define OpenIGTLink_PLATFORM_WIN32
#ifdef OpenIGTLink_PLATFORM_WIN32
  #ifndef _WIN32
    #define _WIN32
  #endif
  #ifndef WIN32
    #define WIN32
  #endif
  #define IGTLCommon_EXPORTS
#endif

#endif /*__IGTL_TYPECONFIG_H*/
