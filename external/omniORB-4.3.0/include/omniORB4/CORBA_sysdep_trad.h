// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_sysdep.h             Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2020 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library. If not, see http://www.gnu.org/licenses/
//
//
// Description:
//	*** PROPRIETARY INTERFACE ***
//
//      Traditional-style hard-coded system dependencies.

#ifndef __CORBA_SYSDEP_TRAD_H__
#define __CORBA_SYSDEP_TRAD_H__


#ifndef _CORBA_WCHAR_DECL
#  define _CORBA_WCHAR_DECL wchar_t
#endif


#define OMNI_HAS_Cplusplus_const_cast
// Unset this define if the compiler does not support const_cast<T*>

#define OMNI_HAS_Cplusplus_reinterpret_cast
// Unset this define if the compiler does not support reinterpret_cast<T>

#define OMNI_HAS_Cplusplus_catch_exception_by_base
// Unset this define if the compiler does not support catching
// exceptions by base class.

#define OMNI_HAVE_STRDUP 1
// Unset if no strdup()

#define OMNI_HAVE_GETOPT 1
// Unset if no getopt()

#define OMNI_HAVE_UNISTD_H 1
// Unset if no unistd.h header

#ifndef OMNI_HAVE_GETPID
#  define OMNI_HAVE_GETPID 1
#endif
// Unset if no getpid() function

#ifndef OMNI_HAVE_GMTIME
#  define OMNI_HAVE_GMTIME 1
#endif
// Unset if no gmtime() function

#ifndef OMNI_HAVE_LOCALTIME
#  define OMNI_HAVE_LOCALTIME 1
#endif
// Unset if no localtime() function

#ifndef OMNI_HAVE_STRFTIME
#  define OMNI_HAVE_STRFTIME 1
#endif
// Unset if no strftime() function

#define OMNI_HAVE_GETTIMEOFDAY 1
// Unset if no gettimeofday() function

#define OMNI_GETTIMEOFDAY_TIMEZONE
// Unset if gettimeofday() doesn't take a timezone argument

#define OMNI_HAVE_SIGNAL_H 1
// Unset if no signal.h header

#define OMNI_HAVE_SIGACTION 1
// Unset if no sigaction() function

#define OMNI_HAVE_SIG_IGN
// Unset if have sigaction() but have no SIG_IGN define

#define OMNI_HAVE_STRCASECMP 1
// Unset if no strcasecmp() function

#define OMNI_HAVE_STRNCASECMP 1
// Unset if no strncasecmp() function

#define OMNI_HAVE_UNAME  1
// Unset if no uname() function

#define OMNI_HAVE_GETHOSTNAME 1
// Unset if no gethostname() function

#define OMNI_HAVE_VSNPRINTF 1
// Unset if no vsnprintf() function

#define OMNI_HAVE_STRTOUL 1
// Unset if no strtoul() function



//
// Compiler dependencies
//

#if defined(__GNUG__)
// GNU G++ compiler

#  if __GNUG__ == 2 && __GNUC_MINOR__ == 7
#     undef OMNI_HAS_Cplusplus_catch_exception_by_base
#  endif

// Minor version number 91 is for egcs version 1.*  Some older
// versions of 1.* may not support namespaces properly - this is
// only tested for egcs 1.1.1
#  if (__GNUG__ == 2 && (__GNUC_MINOR__ >= 91 || __GNUC_MINOR__ == 9)) || \
      (__GNUG__ >= 3)
#     define OMNI_HAS_Cplusplus_Namespace
#     define OMNI_HAS_Cplusplus_Bool
#  endif

// Since gcc 3.3 old IOstream's are considered deprecated.
#  if (__GNUG__ > 3 || (__GNUG__ == 3 && __GNUC_MINOR__ >= 3))
#     define OMNI_HAS_Cplusplus_Namespace
#     define OMNI_HAS_Std_Namespace
#  endif

// GCC claims to support long long on all platforms
#  define OMNI_HAS_LongLong
#  define OMNI_HAS_LongDouble
#  define _CORBA_LONGLONG_DECL   long long
#  define _CORBA_ULONGLONG_DECL  unsigned long long
#  define _CORBA_LONGDOUBLE_DECL long double 
#  define _CORBA_LONGLONG_CONST(x) (x##LL)

#elif defined(__DECCXX)
// DEC C++ compiler

#  if __DECCXX_VER >= 60000000
#     define OMNI_HAS_LongLong
//#     define OMNI_HAS_LongDouble
#     define _CORBA_LONGLONG_DECL   long long
#     define _CORBA_ULONGLONG_DECL  unsigned long long
#     define _CORBA_LONGDOUBLE_DECL long double
#     define _CORBA_LONGLONG_CONST(x) (x##LL)
#     ifndef NO_Cplusplus_Bool
#       define OMNI_HAS_Cplusplus_Bool
#     endif
#     define OMNI_HAS_Cplusplus_Namespace
#     define OMNI_HAS_Std_Namespace
#     define OMNI_HAS_pch
#     if __DECCXX_VER < 70390009
#       define OMNI_REQUIRES_FQ_BASE_CTOR
#     endif
// Uncomment the following lines to enable the use of namespace with cxx v5.6
// Notice that the source code may have to be patched to compile.
//#  elif __DECCXX_VER >= 50600000
//#     define OMNI_HAS_Cplusplus_Namespace
//#     define OMNI_NEED_DUMMY_RETURN
#  else
//    Compaq C++ 5.x
#     undef  OMNI_HAS_Cplusplus_const_cast
#     undef  OMNI_HAS_Cplusplus_reinterpret_cast
#     define OMNI_REQUIRES_FQ_BASE_CTOR

#  endif

#elif defined(__SUNPRO_CC) 
// SUN C++ compiler
#  if __SUNPRO_CC >= 0x500
#    if __SUNPRO_CC_COMPAT >= 5
#      define OMNI_HAS_Cplusplus_Namespace
#      define OMNI_HAS_Std_Namespace
#      define OMNI_HAS_Cplusplus_Bool
#    endif
#  endif

#  define OMNI_HAS_LongLong
#  define _CORBA_LONGLONG_DECL   long long
#  define _CORBA_ULONGLONG_DECL  unsigned long long
#  define _CORBA_LONGDOUBLE_DECL long double 
#  define _CORBA_LONGLONG_CONST(x) (x##LL)

#  define OMNI_HAS_LongDouble


#elif defined(_MSC_VER)
//  Microsoft Visual C++ compiler
#  if _MSC_VER >= 1000
#    ifndef NO_Cplusplus_Bool
#      define OMNI_HAS_Cplusplus_Bool
#    endif
#    define OMNI_HAS_Cplusplus_Namespace
#    define OMNI_HAS_Std_Namespace
#  endif

#  if defined(_WIN64)
#    define OMNI_SIZEOF_PTR  8
#  endif

#  define OMNI_HAS_LongLong
#  define _CORBA_LONGLONG_DECL   __int64
#  define _CORBA_ULONGLONG_DECL  unsigned __int64
#  define _CORBA_LONGLONG_CONST(x) (x)

#elif defined(__DMC__)
//  Digital Mars C++
#  define OMNI_HAS_Cplusplus_Bool
#  define OMNI_HAS_Cplusplus_Namespace
#  define OMNI_HAS_Std_Namespace

#  define OMNI_HAVE_STRTOULL

#  define OMNI_HAS_LongDouble
#  define OMNI_HAS_LongLong
#  define _CORBA_LONGDOUBLE_DECL long double
#  define _CORBA_LONGLONG_DECL   long long
#  define _CORBA_ULONGLONG_DECL  unsigned long long
#  define _CORBA_LONGLONG_CONST(x) (x##LL)

#  define OMNI_REQUIRES_FQ_BASE_CTOR

#elif defined(__BCPLUSPLUS__)
// Borland C++ Builder
#  define OMNI_HAS_Cplusplus_Namespace
#  define OMNI_HAS_Std_Namespace

#  define OMNI_HAS_LongLong
#  define _CORBA_LONGLONG_DECL   __int64
#  define _CORBA_ULONGLONG_DECL  unsigned __int64
#  define _CORBA_LONGLONG_CONST(x) (x)

#  define OMNI_REQUIRES_FQ_BASE_CTOR

#elif defined(__KCC)
// Kai C++
#  define OMNI_HAS_Cplusplus_Namespace
#  define OMNI_HAS_Std_Namespace
#  define OMNI_HAS_Cplusplus_Bool

#elif defined(__sgi)

#  if _COMPILER_VERSION >= 721
#    define OMNI_HAS_Cplusplus_Namespace
#    define OMNI_HAS_Cplusplus_Bool
#    define OMNI_HAS_Cplusplus_const_cast
#    define OMNI_REQUIRES_FQ_BASE_CTOR
#    define OMNI_HAS_LongLong
#    define OMNI_HAS_LongDouble
#    define _CORBA_LONGLONG_DECL long long
#    define _CORBA_ULONGLONG_DECL unsigned long long
#    define _CORBA_LONGDOUBLE_DECL long double
#    define _CORBA_LONGLONG_CONST(x) (x##LL)
#  endif
#  if  _MIPS_SZINT == 64
#    define OMNI_SIZEOF_INT 8
#  endif
#  if _MIPS_SZLONG == 64
#    define OMNI_SIZEOF_LONG 8
#  endif
#  if _MIPS_SZPTR == 64
#    define OMNI_SIZEOF_PTR 8
#  endif

#elif defined(__xlC__)
#  if (__xlC__ <= 0x0306)
#    undef OMNI_HAS_Cplusplus_const_cast
#    undef OMNI_HAS_Cplusplus_reinterpret_cast
#  elif (__xlC__ >= 0x0500) // added in xlC 5.0 (a.k.a. Visual Age 5.0)
#    define OMNI_HAS_Cplusplus_Bool
#    define OMNI_HAS_Cplusplus_Namespace
#    define OMNI_HAS_Std_Namespace
#    define OMNI_HAS_LongLong
#    define OMNI_HAS_LongDouble
#    define _CORBA_LONGLONG_DECL long long
#    define _CORBA_ULONGLONG_DECL unsigned long long
#    define _CORBA_LONGDOUBLE_DECL long double
#    define _CORBA_LONGLONG_CONST(x) (x##LL)
#  endif

#elif defined(__hpux__)
// Recent versions of HP aCC (A01.18 and A.03.13) have an identifying macro.
// In the future, we should be able to remove the gcc test.
// In case this is an older compiler aCC, test if this is gcc, if not assume 
// it is aCC.
#  if defined(__HP_aCC) || !defined(__GNUG__)
#    define OMNI_HAS_Cplusplus_Namespace
#    define OMNI_HAS_Cplusplus_Bool
#    define OMNI_HAS_LongLong
#    define _CORBA_LONGLONG_DECL   long long
#    define _CORBA_ULONGLONG_DECL  unsigned long long
#    define _CORBA_LONGLONG_CONST(x) (x##LL)
#    if defined(_FPWIDETYPES)
#      define OMNI_HAS_LongDouble
#      define _CORBA_LONGDOUBLE_DECL long double
#    endif
// ia64 in 64-bit mode
#    if defined(__LP64__)
#      define OMNI_SIZEOF_CHAR 1
#      define OMNI_SIZEOF_DOUBLE 8
#      define OMNI_SIZEOF_FLOAT 4
#      define OMNI_SIZEOF_INT 4
#      define OMNI_SIZEOF_LONG 8
#      define OMNI_SIZEOF_LONG_LONG 8
#      define OMNI_SIZEOF_PTR 8
#    endif
#  endif

#elif defined(__MWERKS__)
// Metrowerks CodeWarrior Pro 8 or later for Mac OS Classic or Carbon
#  define OMNI_HAS_Cplusplus_Bool
#  define OMNI_HAS_Cplusplus_Namespace
#  define OMNI_HAS_Std_Namespace
#  define OMNI_HAS_LongLong
#  define _CORBA_LONGLONG_DECL long long
#  define _CORBA_ULONGLONG_DECL unsigned long long
#  define _CORBA_LONGLONG_CONST(x) (x##LL)

#endif


//
// Processor dependencies
//

#if defined(__x86__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 1
#  define OMNI_SIZEOF_LONG_DOUBLE 12

#elif defined(__sparc__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__alpha__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 1
#  ifndef __VMS
#    define OMNI_SIZEOF_LONG 8
#    define OMNI_SIZEOF_INT  4
#    define OMNI_SIZEOF_PTR  8
#  endif

#elif defined(__hppa__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__ia64__)
// IA64 is big-endian on HPUX, little endian on everything else
#  if defined(__hpux__)
#    define _OMNIORB_HOST_BYTE_ORDER_ 0
#  else
#    define _OMNIORB_HOST_BYTE_ORDER_ 1
#  endif

#elif defined(__powerpc__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__mips__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__s390__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__arm__)
// armv5teb is big-endian
#  if defined(__armv5teb__)
#    define _OMNIORB_HOST_BYTE_ORDER_ 0
#  else
#    define _OMNIORB_HOST_BYTE_ORDER_ 1
#  endif

#elif defined(__m68k__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 0

#elif defined(__vax__)
#  define _OMNIORB_HOST_BYTE_ORDER_ 1

#endif


//
// OS dependencies
//

#if defined(__linux__)
#  define OMNI_SOCKNAME_SIZE_T socklen_t
#  define OMNI_HAVE_STRTOULL 1

#elif defined(__sunos__)
#  define OMNI_HAVE_STRTOULL 1
#  define OMNI_HAVE_ISNANORINF
#  define OMNI_HAVE_NAN_H
#  if __OSVERSION__ == 4
#    define OMNI_SOCKNAME_SIZE_T int
#  elif __OSVERSION__ == 5 || __OSVERSION__ == 6
#    define OMNI_SOCKNAME_SIZE_T size_t
#  else
#    define OMNI_SOCKNAME_SIZE_T socklen_t
#  endif
#  if __OSVERSION__ == 5 && (!defined(__GNUG__) || __GNUG__ < 3)
#    define NEED_GETHOSTNAME_PROTOTYPE
#  endif

#elif defined(__hpux__)

#elif defined(__irix__)
#  undef OMNI_HAVE_GETHOSTNAME

#elif defined(__freebsd__)
#  define OMNI_HAVE_STRTOUQ 1
#  if __OSVERSION__ >= 4
#    define OMNI_SOCKNAME_SIZE_T socklen_t
#  endif

#elif defined(__aix__)
#  define OMNI_SOCKNAME_SIZE_T size_t

#elif defined(__SINIX__)
#  undef OMNI_GETTIMEOFDAY_TIMEZONE
#  define OMNI_SOCKNAME_SIZE_T size_t

#elif defined(__uw7__)
#  define OMNI_SOCKNAME_SIZE_T size_t

#elif defined(__darwin__)
#  define OMNI_HAVE_STRTOUQ 1
#  define OMNI_SOCKNAME_SIZE_T socklen_t
#  define OMNI_HAVE_STRUCT_SOCKADDR_IN_SIN_ZERO 1
#  define OMNI_HAVE_STRUCT_SOCKADDR_IN_SIN_LEN 1

#elif defined(__nextstep__)
#  undef OMNI_HAVE_STRDUP
#  undef OMNI_HAVE_UNAME
#  undef OMNI_HAVE_SIGACTION
#  define OMNI_HAVE_SIGVEC

#elif defined(__VMS)
#  define OMNI_SOCKNAME_SIZE_T size_t
#  undef OMNI_HAVE_STRDUP
#  if __VMS_VER < 70000000
#    undef OMNI_HAVE_GETOPT
#    undef OMNI_HAVE_STRCASECMP
#    undef OMNI_HAVE_STRNCASECMP
#    undef OMNI_HAVE_GETTIMEOFDAY
#  endif
#  if __CRTL_VER >= 70311000
#    define OMNI_HAVE_POLL
#  endif

#elif defined(__WIN32__)
#  define OMNI_SIZEOF_WCHAR 2
#  undef OMNI_HAVE_UNISTD_H
#  undef OMNI_HAVE_GETOPT
#  undef OMNI_HAVE_GETTIMEOFDAY
#  undef OMNI_HAVE_GETPID
#  undef OMNI_HAVE_SIGNAL_H
#  undef OMNI_HAVE_SIGACTION
#  undef OMNI_HAVE_STRCASECMP
#  undef OMNI_HAVE_STRNCASECMP
#  undef OMNI_HAVE_UNAME
#  undef OMNI_HAVE_GETHOSTNAME
#  undef OMNI_HAVE_POLL

#if defined(_MSC_VER) && _MSC_VER >= 1300  // VC++ 7 or greater
#  define OMNI_HAVE_GETADDRINFO 1
#  define OMNI_HAVE_GETNAMEINFO 1
#  define OMNI_HAVE_STRUCT_SOCKADDR_IN6 1
#  define OMNI_HAVE_STRUCT_SOCKADDR_STORAGE 1
#endif

#ifdef __MINGW32__
#  define OMNI_HAVE_STRCASECMP
#  define OMNI_HAVE_STRNCASECMP
#  define OMNI_HAVE_VPRINTF
#endif

#elif defined(__vxWorks__)
#  undef OMNI_HAVE_GETTIMEOFDAY
#  undef OMNI_HAVE_STRCASECMP
#  undef OMNI_HAVE_STRNCASECMP

#elif defined(__macos__)
#  define OMNI_SIZEOF_WCHAR 2
#  define OMNI_SOCKNAME_SIZE_T socklen_t
#  define OMNI_HAVE_STRTOULL 1

#endif



//
// Default flag values if not already overridden above
//

#ifndef OMNI_SIZEOF_BOOL
#define OMNI_SIZEOF_BOOL 1
#endif

#ifndef OMNI_SIZEOF_LONG
#define OMNI_SIZEOF_LONG 4
#endif

#ifndef OMNI_SIZEOF_INT
#define OMNI_SIZEOF_INT 4
#endif

#ifndef OMNI_SIZEOF_PTR
#define OMNI_SIZEOF_PTR  4
#endif

#ifndef OMNI_SIZEOF_LONG_DOUBLE
#define OMNI_SIZEOF_LONG_DOUBLE 16
#endif

#ifndef OMNI_SIZEOF_WCHAR
#  define OMNI_SIZEOF_WCHAR 4
#endif

#ifndef OMNI_SOCKNAME_SIZE_T
#  define OMNI_SOCKNAME_SIZE_T int
#endif

#ifndef _OMNIORB_HOST_BYTE_ORDER_
# error "The byte order of this platform is unknown"
#endif




#endif // __CORBA_SYSDEP_TRAD_H__
