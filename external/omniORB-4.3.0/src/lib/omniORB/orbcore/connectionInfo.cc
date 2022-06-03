// -*- Mode: C++; -*-
//                            Package   : omniORB
// connectionInfo.cc          Created on: 2018/08/28
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

#include <omniORB4/CORBA.h>
#include <omniORB4/connectionInfo.h>


OMNI_NAMESPACE_BEGIN(omni)

ConnectionInfo* ConnectionInfo::singleton = 0;


const char*
ConnectionInfo::toString(ConnectionInfo::ConnectionEvent evt)
{
  switch (evt) {
  case BIND:
    return "Bind to address";

  case ACCEPTED_CONNECTION:
    return "Accepted connection";

  case TRY_CONNECT:
    return "Attempt to connect";

  case CONNECTED:
    return "Connected";

  case CONNECT_FAILED:
    return "Connect failed";

  case SEND_FAILED:
    return "Send failed";

  case RECV_FAILED:
    return "Receive failed";

  case CONNECT_TIMED_OUT:
    return "Connect timed out";

  case SEND_TIMED_OUT:
    return "Send timed out";

  case RECV_TIMED_OUT:
    return "Receive timed out";

  case CLOSED:
    return "Connection closed";

  case RESOLVE_NAME:
    return "Resolve name";

  case NAME_RESOLVED:
    return "Name resolved";

  case NAME_RESOLUTION_FAILED:
    return "Failed to resolve name";


    // TLS / SSL
    
  case TRY_TLS_CONNECT:
    return "Attempt TLS connect";

  case TLS_CONNECTED:
    return "TLS connected";

  case TLS_CONNECT_FAILED:
    return "TLS connect failed";

  case TLS_CONNECT_TIMED_OUT:
    return "TLS connect timed out";

  case TRY_TLS_ACCEPT:
    return "Attempt TLS accept";

  case TLS_ACCEPTED:
    return "TLS connection accepted";

  case TLS_ACCEPT_FAILED:
    return "TLS accept failed";

  case TLS_ACCEPT_TIMED_OUT:
    return "TLS accept timed out";

  case TLS_PEER_CERT:
    return "TLS peer certificate";

  case TLS_PEER_VERIFIED:
    return "TLS peer verified";

  case TLS_PEER_NOT_VERIFIED:
    return "TLS peer not verified";

  case TLS_PEER_IDENTITY:
    return "TLS peer identity";


    // HTTP
    
  case CONNECT_TO_PROXY:
    return "Connect to HTTP proxy";

  case PROXY_CONNECT_REQUEST:
    return "Send HTTP CONNECT to proxy";

  case PROXY_REQUIRES_AUTH:
    return "HTTP proxy requires authentication";

  case SEND_HTTP_ERROR:
    return "Send HTTP error";

  case RECV_HTTP_ERROR:
    return "Receive HTTP error";

  case HTTP_BUFFER_FULL:
    return "HTTP buffer full";

  case SEND_WEBSOCKET_REQ:
    return "Send WebSocket upgrade request";

  case RECV_WEBSOCKET_REQ:
    return "Receive WebSocket upgrade request";

  case SEND_WEBSOCKET_ACK:
    return "Send WebSocket upgrade acknowledgement";

  case RECV_WEBSOCKET_ACK:
    return "Receive WebSocket upgrade acknowledgement";

  case RECV_WEBSOCKET_REJECT:
    return "WebSocket upgrade rejected";


    // HTTP crypto
    
  case SEND_SESSION_KEY:
    return "Send HTTP crypto session key";

  case RECEIVED_SESSION_KEY:
    return "Received HTTP crypto session key";

  case CRYPTO_CLIENT_UNKNOWN:
    return "Unknown HTTP crypto client";

  case INVALID_SESSION_KEY:
    return "Invalid HTTP crypto session key";

  default:
    return "Unknown connection event";
  }
}

ConnectionInfo::~ConnectionInfo()
{
}


void
LoggingConnectionInfo::
event(ConnectionEvent evt,
      CORBA::Boolean  is_error,
      const char*     addr,
      const char*     info)
{
  if (is_error || !pd_errors_only) {
    omniORB::logger log(pd_prefix);
    log << toString(evt) << " : " << addr;

    if (info)
      log << " : " << info;

    log << "\n";
  }
}


OMNI_NAMESPACE_END(omni)
