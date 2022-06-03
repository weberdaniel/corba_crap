// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA_sysdep.h             Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
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
//      Define various symbols based on things detected by autoconf

#ifndef __CORBA_SYSDEP_AUTO_H__
#define __CORBA_SYSDEP_AUTO_H__


#ifdef OMNI_HAVE_BOOL
#  define OMNI_HAS_Cplusplus_Bool
#endif

#ifdef OMNI_HAVE_CATCH_BY_BASE
#  define OMNI_HAS_Cplusplus_catch_exception_by_base
#endif

#ifdef OMNI_HAVE_CONST_CAST
#  define OMNI_HAS_Cplusplus_const_cast
#endif

#ifdef OMNI_HAVE_REINTERPRET_CAST
#  define OMNI_HAS_Cplusplus_reinterpret_cast
#endif

#ifdef OMNI_HAVE_NAMESPACES
#  define OMNI_HAS_Cplusplus_Namespace
#endif

#define OMNI_SIZEOF_PTR OMNI_SIZEOF_VOIDP

#if defined(OMNI_SIZEOF_LONG) && (OMNI_SIZEOF_LONG == 8)
#  define OMNI_HAS_LongLong
#  define _CORBA_LONGLONG_DECL     long
#  define _CORBA_ULONGLONG_DECL    unsigned long
#  define _CORBA_LONGLONG_CONST(x) (x)

#elif defined(OMNI_SIZEOF_LONG_LONG) && (OMNI_SIZEOF_LONG_LONG == 8)
#  define OMNI_HAS_LongLong
#  define _CORBA_LONGLONG_DECL     long long
#  define _CORBA_ULONGLONG_DECL    unsigned long long
#  define _CORBA_LONGLONG_CONST(x) (x##LL)
#endif


#if !defined(OMNI_DISABLE_LONGDOUBLE)
#  if defined(OMNI_SIZEOF_LONG_DOUBLE) && (OMNI_SIZEOF_LONG_DOUBLE == 16)
#    define OMNI_HAS_LongDouble
#    define _CORBA_LONGDOUBLE_DECL long double
#  endif

#  if defined(OMNI_SIZEOF_LONG_DOUBLE) && (OMNI_SIZEOF_LONG_DOUBLE == 12) && defined(__i386__)
#    define OMNI_HAS_LongDouble
#    define _CORBA_LONGDOUBLE_DECL long double
#  endif
#endif

#ifndef _CORBA_WCHAR_DECL
#  if defined(OMNI_SIZEOF_WCHAR_T) && (OMNI_SIZEOF_WCHAR_T > 0)
#    define _CORBA_WCHAR_DECL wchar_t
#    define OMNI_SIZEOF_WCHAR OMNI_SIZEOF_WCHAR_T
#  else
#    define _CORBA_WCHAR_DECL _CORBA_Short
#    define OMNI_SIZEOF_WCHAR 2
#  endif
#endif

#if !defined(OMNI_SIZEOF_FLOAT) || (OMNI_SIZEOF_FLOAT == 0)
#  define OMNI_NO_FLOAT
#endif

#ifdef OMNI_WORDS_BIGENDIAN
#  define _OMNIORB_HOST_BYTE_ORDER_ 0
#else
#  define _OMNIORB_HOST_BYTE_ORDER_ 1
#endif


#endif // __CORBA_SYSDEP_AUTO_H__
