// -*- Mode: C++; -*-
//                            Package   : omniORB
// connectionInfo.h           Created on: 2018/08/28
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2018-2019 Apasphere Ltd.
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
// Description:
//    omniORB connection information hook

#ifndef _OMNIORB_CONNECTIONINFO_H_
#define _OMNIORB_CONNECTIONINFO_H_

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)


class ConnectionInfo {
public:

  // ConnectionInfo abstract class. Provide an implementation of this
  // to receive callbacks with connection event information.
  
  enum ConnectionEvent {
    // Generic events
    BIND                    = 0x01,
    ACCEPTED_CONNECTION     = 0x02,

    TRY_CONNECT             = 0x03,
    CONNECTED               = 0x04,
    CONNECT_FAILED          = 0x05,
    SEND_FAILED             = 0x06,
    RECV_FAILED             = 0x07,
    CONNECT_TIMED_OUT       = 0x08,
    SEND_TIMED_OUT          = 0x09,
    RECV_TIMED_OUT          = 0x0a,
    CLOSED                  = 0x0b,

    RESOLVE_NAME            = 0x0c,
    NAME_RESOLVED           = 0x0d,
    NAME_RESOLUTION_FAILED  = 0x0e,

    
    // TLS / SSL events

    TRY_TLS_CONNECT         = 0x101,
    TLS_CONNECTED           = 0x102,
    TLS_CONNECT_FAILED      = 0x103,
    TLS_CONNECT_TIMED_OUT   = 0x104,
    TRY_TLS_ACCEPT          = 0x105,
    TLS_ACCEPTED            = 0x106,
    TLS_ACCEPT_FAILED       = 0x107,
    TLS_ACCEPT_TIMED_OUT    = 0x108,
    TLS_PEER_CERT           = 0x109,
    TLS_PEER_VERIFIED       = 0x10a,
    TLS_PEER_NOT_VERIFIED   = 0x10b,
    TLS_PEER_IDENTITY       = 0x10c,

    
    // HTTP events

    CONNECT_TO_PROXY        = 0x201,
    PROXY_CONNECT_REQUEST   = 0x202,
    PROXY_REQUIRES_AUTH     = 0x203,
    SEND_HTTP_ERROR         = 0x204,
    RECV_HTTP_ERROR         = 0x205,
    HTTP_BUFFER_FULL        = 0x206,
    SEND_WEBSOCKET_REQ      = 0x207,
    RECV_WEBSOCKET_REQ      = 0x208,
    SEND_WEBSOCKET_ACK      = 0x209,
    RECV_WEBSOCKET_ACK      = 0x20a,
    RECV_WEBSOCKET_REJECT   = 0x20b,

    // HTTP crypto
    SEND_SESSION_KEY        = 0x301,
    RECEIVED_SESSION_KEY    = 0x302,
    CRYPTO_CLIENT_UNKNOWN   = 0x303,
    INVALID_SESSION_KEY     = 0x304
  };

  static _core_attr ConnectionInfo* singleton;
  
  virtual void event(ConnectionEvent evt,
                     CORBA::Boolean  is_error,
                     const char*     addr,
                     const char*     info) = 0;

  static const char* toString(ConnectionEvent event);

  static inline void set(ConnectionEvent evt,
                         CORBA::Boolean  is_error,
                         const char*     addr,
                         const char*     info = 0)
  {
    if (singleton)
      singleton->event(evt, is_error, addr, info);
  }
  
  virtual ~ConnectionInfo();
};



class LoggingConnectionInfo : public ConnectionInfo {
public:
  // Implementation of ConnectionInfo that logs to the omniORB logger
  // with a prefix.

  inline LoggingConnectionInfo(CORBA::Boolean errors_only = 0,
                               const char*    prefix      = "Conn info: ")
    : pd_errors_only(errors_only), pd_prefix(prefix)
  {}
  
  virtual void event(ConnectionEvent evt,
                     CORBA::Boolean  is_error,
                     const char*     addr,
                     const char*     info);

private:
  CORBA::Boolean pd_errors_only;
  const char*    pd_prefix;
};


OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // _OMNIORB_CONNECTIONINFO_H_
