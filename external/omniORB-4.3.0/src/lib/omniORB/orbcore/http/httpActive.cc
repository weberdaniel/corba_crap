// -*- Mode: C++; -*-
//                            Package   : omniORB
// httpActive.cc              Created on: 18 April 2018
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2005-2019 Apasphere Ltd
//    Copyright (C) 2018      Apasphere Ltd, BMC Software
//    Copyright (C) 2001      AT&T Laboratories Cambridge
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
//      *** PROPRIETARY INTERFACE ***
//

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <SocketCollection.h>
#include <omniORB4/httpContext.h>
#include <http/httpConnection.h>
#include <http/httpEndpoint.h>
#include <openssl/err.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(httpActive);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
static httpActiveCollection myCollection;

/////////////////////////////////////////////////////////////////////////
httpActiveCollection::httpActiveCollection()
  : pd_n_sockets(0), pd_shutdown(0), pd_lock("httpActiveCollection::pd_lock")
{}

/////////////////////////////////////////////////////////////////////////
httpActiveCollection::~httpActiveCollection() {}

/////////////////////////////////////////////////////////////////////////
const char*
httpActiveCollection::type() const {
  return "giop:http";
}

/////////////////////////////////////////////////////////////////////////
void
httpActiveCollection::Monitor(giopConnection::notifyReadable_t func,
                              void* cookie) {

  pd_callback_func = func;
  pd_callback_cookie = cookie;

  while (!isEmpty()) {
    if (!Select()) break;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
httpActiveCollection::notifyReadable(SocketHolder* conn) {

  pd_callback_func(pd_callback_cookie,(httpConnection*)conn);
  return 1;
}


/////////////////////////////////////////////////////////////////////////
void
httpActiveCollection::addMonitor(SocketHandle_t) {
  omni_tracedmutex_lock sync(pd_lock);
  pd_n_sockets++;
  pd_shutdown = 0;
}

/////////////////////////////////////////////////////////////////////////
void
httpActiveCollection::removeMonitor(SocketHandle_t) {
  omni_tracedmutex_lock sync(pd_lock);
  pd_n_sockets--;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
httpActiveCollection::isEmpty() const {
  // Cast tracedmutex to escape this function's constness.
  omni_tracedmutex_lock sync((omni_tracedmutex&)pd_lock);
  return (pd_n_sockets == 0 || pd_shutdown);
}

/////////////////////////////////////////////////////////////////////////
void
httpActiveCollection::deactivate() {
  omni_tracedmutex_lock sync(pd_lock);
  pd_shutdown = 1;
  wakeUp();
}

/////////////////////////////////////////////////////////////////////////
httpActiveConnection::httpActiveConnection(SocketHandle_t sock,
                                           const char*    host_header,
                                           const char*    path,
                                           const char*    url,
                                           CORBA::Boolean websocket,
                                           CORBA::Boolean via_proxy,
                                           const char*    proxy_auth) :
  httpConnection(sock, 0, &myCollection, host_header, path, url,
                 1, websocket, via_proxy, proxy_auth),
  pd_proxy_peerdetails(0),
  pd_registered(0)
{
  pd_handshake_ok = 1;

  if (httpContext::crypto_manager)
    pd_crypto = httpContext::crypto_manager->cryptoForServer(url, 0);
}

/////////////////////////////////////////////////////////////////////////
httpActiveConnection::~httpActiveConnection() {
  if (pd_registered) {
    myCollection.removeMonitor(pd_socket);
  }
  if (pd_proxy_peerdetails)
    delete pd_proxy_peerdetails;
}


/////////////////////////////////////////////////////////////////////////
giopActiveCollection*
httpActiveConnection::registerMonitor() {

  if (pd_registered) return &myCollection;

  pd_registered = 1;
  myCollection.addMonitor(pd_socket);
  return &myCollection;
}

/////////////////////////////////////////////////////////////////////////
giopConnection&
httpActiveConnection::getConnection() {
  return *this;
}

OMNI_NAMESPACE_END(omni)
