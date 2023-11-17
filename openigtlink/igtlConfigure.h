/*=========================================================================

  Program:   OpenIGTLink Library
  Module:    $HeadURL: http://svn.na-mic.org/NAMICSandBox/trunk/OpenIGTLink/igtlConfigure.h.in $
  Language:  C
  Date:      $Date: 2010-06-09 16:16:36 -0400 (Wed, 09 Jun 2010) $
  Version:   $Revision: 6525 $

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __IGTL_CONFIGURE_H
#define __IGTL_CONFIGURE_H

#ifdef _WIN32
  #define OpenIGTLink_PLATFORM_WIN32
  #define OpenIGTLink_USE_WIN32_THREADS
  #define IGTLCommon_EXPORTS
#endif

/* #undef OpenIGTLink_USE_SPROC */
/* #undef OpenIGTLink_HAVE_GETSOCKNAME_WITH_SOCKLEN_T */
#define OpenIGTLink_HAVE_STRNLEN
/* #undef OpenIGTLink_USE_H264 */
/* #undef OpenIGTLink_USE_VP9 */
/* #undef OpenIGTLink_USE_X265 */
/* #undef OpenIGTLink_USE_OpenHEVC */
/* #undef OpenIGTLink_USE_AV1 */
/* #undef OpenIGTLink_USE_WEBSOCKET */
/* #undef OpenIGTLink_ENABLE_VIDEOSTREAMING */

#define OpenIGTLink_PROTOCOL_VERSION_1 1
#define OpenIGTLink_PROTOCOL_VERSION_2 2
#define OpenIGTLink_PROTOCOL_VERSION_3 3
#define OpenIGTLink_PROTOCOL_VERSION 3
#define OpenIGTLink_HEADER_VERSION 2
#endif /*__IGTL_CONFIGURE_H*/



