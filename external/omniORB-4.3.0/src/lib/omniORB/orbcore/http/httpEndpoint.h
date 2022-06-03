// -*- Mode: C++; -*-
//                            Package   : omniORB
// httpEndpoint.h             Created on: 18 April 2018
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2013-2019 Apasphere Ltd
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

#ifndef __HTTPENDPOINT_H__
#define __HTTPENDPOINT_H__

#include <omniORB4/omniServer.h>

OMNI_NAMESPACE_BEGIN(omni)

class httpConnection;

class httpEndpoint : public giopEndpoint,
                     public SocketCollection,
                     public SocketHolder {
public:

  httpEndpoint(const char* param, httpContext* ctx);

  // The following implement giopEndpoint abstract functions
  const char* type() const;
  const char* address() const;
  const orbServer::EndpointList* addresses() const;
  CORBA::Boolean publish(const orbServer::PublishSpecs& publish_specs,
                         CORBA::Boolean                 all_specs,
                         CORBA::Boolean                 all_eps,
                         orbServer::EndpointList&       published_eps);
  CORBA::Boolean Bind();
  giopConnection* AcceptAndMonitor(giopConnection::notifyReadable_t,void*);
  void Poke();
  void Shutdown();

  ~httpEndpoint();

protected:
  CORBA::Boolean notifyReadable(SocketHolder*);
  // implement SocketCollection::notifyReadable
  

private:
  const char*                      pd_address_param;
  CORBA::Boolean                   pd_secure;
  CORBA::Boolean                   pd_websocket;
  IIOP::Address                    pd_address;
  CORBA::String_var                pd_path;
  orbServer::EndpointList          pd_addresses;
  httpContext*                     pd_ctx;

  SocketHandle_t                   pd_new_conn_socket;
  giopConnection::notifyReadable_t pd_callback_func;
  void*                            pd_callback_cookie;
  CORBA::Boolean                   pd_poked;

  httpEndpoint();
  httpEndpoint(const httpEndpoint&);
  httpEndpoint& operator=(const httpEndpoint&);
};


class httpActiveConnection;

class httpActiveCollection : public giopActiveCollection, 
                             public SocketCollection {
public:
  const char* type() const;
  // implement giopActiveCollection::type

  void Monitor(giopConnection::notifyReadable_t func, void* cookie);
  // implement giopActiveCollection::Monitor

  CORBA::Boolean isEmpty() const;
  // implement giopActiveCollection::isEmpty

  void deactivate();
  // implement giopActiveCollection::deactivate

  httpActiveCollection();
  ~httpActiveCollection();

  friend class httpActiveConnection;

protected:
  CORBA::Boolean notifyReadable(SocketHolder*);
  // implement SocketCollection::notifyReadable

  void addMonitor(SocketHandle_t);
  void removeMonitor(SocketHandle_t);

private:
  CORBA::ULong      pd_n_sockets;
  CORBA::Boolean    pd_shutdown;
  omni_tracedmutex  pd_lock;

  giopConnection::notifyReadable_t pd_callback_func;
  void*                            pd_callback_cookie;

  httpActiveCollection(const httpActiveCollection&);
  httpActiveCollection& operator=(const httpActiveCollection&);
};

OMNI_NAMESPACE_END(omni)

#endif // __HTTPENDPOINT_H__
