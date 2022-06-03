// -*- Mode: C++; -*-
//                            Package   : omniORB
// httpAddress.h              Created on: 18 April 2018
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

#ifndef __HTTPADDRESS_H__
#define __HTTPADDRESS_H__

OMNI_NAMESPACE_BEGIN(omni)

class httpAddress : public giopAddress {
public:

  httpAddress(const char* url, httpContext* ctx);

  httpAddress(const char*          url,
              CORBA::Boolean       secure,
              CORBA::Boolean       websocket,
              const IIOP::Address& address,
              const char*          orig_host,
              const char*          host_header,
              const char*          path,
              httpContext*         ctx);

  const char*  type()      const;
  const char*  address()   const;
  const char*  host()      const;
  giopAddress* duplicate() const;
  giopAddress* duplicate(const char* host) const;

  giopActiveConnection* Connect(const omni_time_t& deadline,
                                CORBA::ULong       strand_flags,
                                CORBA::Boolean&    timed_out) const;
  CORBA::Boolean Poke() const;
  ~httpAddress() {}

private:
  CORBA::String_var  pd_url;            // full original URL
  CORBA::Boolean     pd_secure;         // true if https
  CORBA::Boolean     pd_websocket;      // true if using WebSocket
  IIOP::Address      pd_address;        // host and port to connect to
  CORBA::String_var  pd_orig_host;      // original hostname before resolution
  CORBA::String_var  pd_host_header;    // host and port component of URL
  CORBA::String_var  pd_path;           // path component of URL
  CORBA::String_var  pd_address_string;
  httpContext*       pd_ctx;

  void setAddrString();
  
  httpAddress();
  httpAddress(const httpAddress&);
};

OMNI_NAMESPACE_END(omni)

#endif // __HTTPADDRESS_H__
