// -*- Mode: C++; -*-
//                            Package   : omniORB
// httpEndpoint.cc            Created on: 18 April 2018
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2002-2019 Apasphere Ltd
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
#include <omniORB4/connectionInfo.h>
#include <SocketCollection.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/httpContext.h>
#include <orbParameters.h>
#include <objectAdapter.h>
#include <libcWrapper.h>
#include <tcpSocket.h>
#include <http/httpTransportImpl.h>
#include <http/httpConnection.h>
#include <http/httpAddress.h>
#include <http/httpEndpoint.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(httpEndpoint);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
httpEndpoint::httpEndpoint(const char* param, httpContext* ctx) :
  SocketHolder(RC_INVALID_SOCKET),
  pd_address_param(param), pd_secure(0), pd_ctx(ctx),
  pd_new_conn_socket(RC_INVALID_SOCKET), pd_callback_func(0),
  pd_callback_cookie(0), pd_poked(0)
{
}

/////////////////////////////////////////////////////////////////////////
httpEndpoint::~httpEndpoint() {
  if (pd_socket != RC_INVALID_SOCKET) {
    CLOSESOCKET(pd_socket);
    pd_socket = RC_INVALID_SOCKET;
  }
}

/////////////////////////////////////////////////////////////////////////
const char*
httpEndpoint::type() const {
  return "giop:http";
}

/////////////////////////////////////////////////////////////////////////
const char*
httpEndpoint::address() const {
  return pd_addresses[0];
}

/////////////////////////////////////////////////////////////////////////
const _CORBA_Unbounded_Sequence_String*
httpEndpoint::addresses() const {
  return &pd_addresses;
}

/////////////////////////////////////////////////////////////////////////
static inline char* prefixForScheme(const char* scheme)
{
  const char* fmt = "giop:http:%s://";
  
  char* prefix = CORBA::string_alloc(15 + strlen(scheme));
  sprintf(prefix, fmt, scheme);
  return prefix;
}


static CORBA::Boolean
publish_one(const char*              publish_spec,
            const char*              ep,
            CORBA::Boolean           no_publish,
            orbServer::EndpointList& published_eps)
{
  OMNIORB_ASSERT(!strncmp(ep, "giop:http:", 9));

  CORBA::String_var to_add;

  CORBA::String_var ep_scheme, ep_host, ep_path, ep_fragment;
  CORBA::UShort     ep_port;

  CORBA::Boolean ok = omniURI::extractURL(ep + 10, ep_scheme.out(),
                                          ep_host.out(), ep_port,
                                          ep_path.out(), ep_fragment.out());
  OMNIORB_ASSERT(ok);

  CORBA::String_var prefix = prefixForScheme(ep_scheme);
  
  if (!strncmp(publish_spec, "giop:http:", 10)) {

    CORBA::String_var ps_scheme, ps_host, ps_path, ps_fragment;
    CORBA::UShort     ps_port;
    
    ok = omniURI::extractURL(publish_spec + 10, ps_scheme.out(), ps_host.out(),
                             ps_port, ps_path.out(), ps_fragment.out());
    if (!ok) {
      if (omniORB::trace(1)) {
        omniORB::logger l;
        l << "Invalid endpoint '" << publish_spec
          << "' in publish specification.\n";
      }
      OMNIORB_THROW(INITIALIZE,
                    INITIALIZE_EndpointPublishFailure,
                    CORBA::COMPLETED_NO);
    }
    if (strlen(ps_host) == 0)
      ps_host = ep_host;

    if (!ps_port)
      ps_port = ep_port;

    prefix = prefixForScheme(ps_scheme);
    to_add = omniURI::buildURI(prefix, ps_host, ps_port, ps_path);
  }
  else if (no_publish) {
    // Suppress all the other options
    return 0;
  }
  else if (omni::strMatch(publish_spec, "addr")) {
    to_add = ep;
  }
  else if (omni::strMatch(publish_spec, "ipv6")) {
    if (!LibcWrapper::isip6addr(ep_host))
      return 0;
    to_add = ep;
  }
  else if (omni::strMatch(publish_spec, "ipv4")) {
    if (!LibcWrapper::isip4addr(ep_host))
      return 0;
    to_add = ep;
  }
  else if (omni::strMatch(publish_spec, "name")) {
    LibcWrapper::AddrInfo_var ai = LibcWrapper::getAddrInfo(ep_host, 0);
    if (!ai.in())
      return 0;

    CORBA::String_var name = ai->name();
    if (!(char*)name)
      return 0;

    to_add = omniURI::buildURI(prefix, name, ep_port, ep_path);
  }
  else if (omni::strMatch(publish_spec, "hostname")) {
    char self[OMNIORB_HOSTNAME_MAX];

    if (gethostname(&self[0],OMNIORB_HOSTNAME_MAX) == RC_SOCKET_ERROR)
      return 0;

    to_add = omniURI::buildURI(prefix, self, ep_port, ep_path);
  }
  else if (omni::strMatch(publish_spec, "fqdn")) {
    char self[OMNIORB_HOSTNAME_MAX];

    if (gethostname(&self[0],OMNIORB_HOSTNAME_MAX) == RC_SOCKET_ERROR)
      return 0;

    LibcWrapper::AddrInfo_var ai = LibcWrapper::getAddrInfo(self, 0);
    if (!ai.in())
      return 0;

    char* name = ai->name();
    if (name && !(omni::strMatch(name, "localhost") ||
                  omni::strMatch(name, "localhost.localdomain"))) {
      to_add = omniURI::buildURI(prefix, name, ep_port, ep_path);
    }
    else {
      to_add = omniURI::buildURI(prefix, self, ep_port, ep_path);
    }
  }
  else {
    // Don't understand the spec.
    return 0;
  }

  if (!omniObjAdapter::endpointInList(to_add, published_eps)) {
    if (omniORB::trace(20)) {
      omniORB::logger l;
      l << "Publish endpoint '" << to_add << "'\n";
    }
    giopEndpoint::addToIOR(to_add);
    published_eps.length(published_eps.length() + 1);
    published_eps[published_eps.length() - 1] = to_add._retn();
  }
  return 1;
}

CORBA::Boolean
httpEndpoint::publish(const orbServer::PublishSpecs& publish_specs,
                      CORBA::Boolean                 all_specs,
                      CORBA::Boolean                 all_eps,
                      orbServer::EndpointList&       published_eps)
{
  CORBA::ULong i, j;
  CORBA::Boolean result = 0;

  if (publish_specs.length() == 1 &&
      omni::strMatch(publish_specs[0], "fail-if-multiple") &&
      pd_addresses.length() > 1) {

    omniORB::logs(1, "HTTP endpoint has multiple addresses. "
                  "You must choose one to listen on.");
    OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError,
                  CORBA::COMPLETED_NO);
  }
  for (i=0; i < pd_addresses.length(); ++i) {

    CORBA::Boolean ok = 0;
    
    for (j=0; j < publish_specs.length(); ++j) {
      if (omniORB::trace(25)) {
        omniORB::logger l;
        l << "Try to publish '" << publish_specs[j]
          << "' for endpoint " << pd_addresses[i] << "\n";
      }
      ok = publish_one(publish_specs[j], pd_addresses[i], no_publish(),
                       published_eps);
      result |= ok;

      if (ok && !all_specs)
        break;
    }
    if (result && !all_eps)
      break;
  }
  return result;
}


/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
httpEndpoint::Bind() {

  OMNIORB_ASSERT(pd_socket == RC_INVALID_SOCKET);

  CORBA::String_var scheme, host, fragment;
  CORBA::UShort     port;

  CORBA::Boolean ok = omniURI::extractURL(pd_address_param,
                                          scheme.out(), host.out(),
                                          port, pd_path.out(), fragment.out());
  OMNIORB_ASSERT(ok);

  const char* prefix = 0;
  
  if (!strcmp(scheme, "https")) {
    pd_secure = 1;
    prefix    = "giop:http:https://";
  }
  else if (!strcmp(scheme, "http")) {
    pd_secure = 0;
    prefix    = "giop:http:http://";
  }
  else if (!strcmp(scheme, "wss")) {
    pd_secure = 1;
    prefix    = "giop:http:wss://";
  }
  else if (!strcmp(scheme, "ws")) {
    pd_secure = 0;
    prefix    = "giop:http:ws://";
  }
  else {
    // Caller should have already validated the scheme
    ok = 0;
    OMNIORB_ASSERT(ok);
  }
  
  char*         bound_host;
  CORBA::UShort bound_port;

  
  pd_socket = tcpSocket::Bind(host,
                              port,
                              port,
                              type(),
                              bound_host,
                              bound_port,
                              pd_addresses,
                              prefix,
                              pd_path);

  if (pd_socket == RC_INVALID_SOCKET)
    return 0;

  pd_address.host = bound_host;
  pd_address.port = bound_port;

  // Never block in accept
  tcpSocket::setNonBlocking(pd_socket);

  // Add the socket to our SocketCollection.
  addSocket(this);

  return 1;
}

/////////////////////////////////////////////////////////////////////////
void
httpEndpoint::Poke() {

  httpAddress* target = new httpAddress(pd_address_param, pd_ctx);

  pd_poked = 1;
  if (!target->Poke()) {
    if (omniORB::trace(5)) {
      omniORB::logger log;
      log << "Warning: fail to connect to myself ("
          << (const char*) pd_addresses[0] << ") via http.\n";
    }
  }
  // Wake up the SocketCollection in case the connect did not work and
  // it is idle and blocked with no timeout.
  wakeUp();

  delete target;
}

/////////////////////////////////////////////////////////////////////////
void
httpEndpoint::Shutdown() {
  SHUTDOWNSOCKET(pd_socket);
  removeSocket(this);
  decrRefCount();
  omniORB::logs(20, "HTTP endpoint shut down.");
}

/////////////////////////////////////////////////////////////////////////
giopConnection*
httpEndpoint::AcceptAndMonitor(giopConnection::notifyReadable_t func,
                              void* cookie) {

  OMNIORB_ASSERT(pd_socket != RC_INVALID_SOCKET);

  pd_callback_func = func;
  pd_callback_cookie = cookie;
  setSelectable(1,0);

  while (1) {
    pd_new_conn_socket = RC_INVALID_SOCKET;
    if (!Select()) break;
    if (pd_new_conn_socket != RC_INVALID_SOCKET) {

      ::SSL* ssl = 0;
      if (pd_secure) {
        ssl = pd_ctx->ssl_new();

        pd_ctx->set_incoming_verify(ssl);

        SSL_set_fd(ssl, pd_new_conn_socket);
        SSL_set_accept_state(ssl);
      }
      httpConnection* nc = new httpConnection(pd_new_conn_socket, ssl, this,
                                              pd_address.host, pd_path);
      ConnectionInfo::set(ConnectionInfo::ACCEPTED_CONNECTION, 0,
                          nc->peeraddress());
      return nc;
    }
    if (pd_poked)
      return 0;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
httpEndpoint::notifyReadable(SocketHolder* sh) {

  if (sh == (SocketHolder*)this) {
    // New connection
    SocketHandle_t sock;
again:
    sock = ::accept(pd_socket,0,0);
    if (sock == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EBADF) {
        omniORB::logs(20, "accept() returned EBADF, unable to continue");
        return 0;
      }
      else if (ERRNO == RC_EINTR) {
        omniORB::logs(20, "accept() returned EINTR, trying again");
        goto again;
      }
#ifdef UnixArchitecture
      else if (ERRNO == RC_EAGAIN) {
        omniORB::logs(20, "accept() returned EAGAIN, will try later");
      }
#endif
      if (omniORB::trace(20)) {
        omniORB::logger log;
        log << "accept() failed with unknown error " << ERRNO << "\n";
      }
    }
    else {
#if defined(__vxWorks__)
      // vxWorks "forgets" socket options
      static const int valtrue = 1;
      if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                     (char*)&valtrue, sizeof(valtrue)) == ERROR) {
        return 0;
      }
#endif
      // On some platforms, the new socket inherits the non-blocking
      // setting from the listening socket, so we set it blocking here
      // just to be sure.
      tcpSocket::setBlocking(sock);

      pd_new_conn_socket = sock;
    }
    setSelectable(1,0);
    return 1;
  }
  else {
    // Existing connection
    pd_callback_func(pd_callback_cookie,(httpConnection*)sh);
    return 1;
  }
}

OMNI_NAMESPACE_END(omni)
