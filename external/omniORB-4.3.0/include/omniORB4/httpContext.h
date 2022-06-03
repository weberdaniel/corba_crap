// -*- Mode: C++; -*-
//                            Package   : omniORB
// httpContext.h              Created on: 27 April 2018
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2003-2019 Apasphere Ltd
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
//	*** PROPRIETARY INTERFACE ***
// 

#ifndef __HTTPCONTEXT_H__
#define __HTTPCONTEXT_H__

#include <omniORB4/linkHacks.h>
#include <omniORB4/sslContext.h>
#include <omniORB4/httpCrypto.h>

OMNI_FORCE_LINK(omnihttpTP);


#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_HTTP_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

class httpConnection;

class httpContext : public sslContext {
public:

  static _core_attr httpContext* singleton;

  // httpContext singleton object. This object is used to manage all
  // HTTPS connections in the HTTP transport, and control use of HTTP
  // proxies.
  //
  // Application code can populate this pointer with a suitable
  // httpContext object prior to calling CORBA::ORB_init. If it is not
  // set, a default instance is created. This base class uses the
  // global variables defined below to initialise itself, but an
  // application-provided subclass may behave differently.
  //
  // The singleton is deleted by ORB::destroy(). If the application
  // provides its own object here, and it calls ORB::destroy(), it
  // must set the singleton again if it is going to call ORB_init()
  // again.

  // HTTP proxy

  static _core_attr const char* proxy_url;
  static _core_attr const char* proxy_username;
  static _core_attr const char* proxy_password;
  
  
  // HTTPS certificates and keys

  static _core_attr const char* certificate_authority_file; // In PEM format
  static _core_attr const char* certificate_authority_path; // Path
  static _core_attr const char* key_file;                   // In PEM format
  static _core_attr const char* key_file_password;
  static _core_attr const char* cipher_list;
  
  // These parameters can be overriden to adjust the verify mode and
  // verify callback passed to SSL_CTX_set_verify and the info
  // callback passed to SSL_CTX_set_info_callback.
  //
  // If verify_mode_incoming is not -1 (the default), then incoming
  // connections (i.e. connections accepted by a server) are given
  // that mode instead of verify_mode.
  
  static _core_attr int            verify_mode;
  static _core_attr int            verify_mode_incoming;
  static _core_attr omni_verify_cb verify_callback;
  static _core_attr omni_info_cb   info_callback;


  // Manager for in-message crypto

  static _core_attr httpCryptoManager* crypto_manager;

  
  // Interceptor peerdetails calls return this structure:

  class PeerDetails {
  public:
    inline PeerDetails(SSL* s, X509* c, CORBA::Boolean v)
      : pd_ssl(s), pd_cert(c), pd_verified(v), pd_host_header(0),
        pd_crypto(0) {}

    ~PeerDetails();

    inline SSL*           ssl()         { return pd_ssl; }
    inline X509*          cert()        { return pd_cert; }
    inline CORBA::Boolean verified()    { return pd_verified; }
    inline const char*    host_header() { return pd_host_header; }
    inline httpCrypto*    crypto()      { return pd_crypto; }

  private:
    SSL*           pd_ssl;
    X509*          pd_cert;
    CORBA::Boolean pd_verified;
    const char*    pd_host_header;
    httpCrypto*    pd_crypto;

    friend class httpConnection;
  };

  
  httpContext(const char* cafile, const char* capath,
              const char* keyfile, const char* password);
  // Construct with CA file, CA path, key and password. All may be zero.

  httpContext();
  // Construct with details from the global variables.
  
  virtual ~httpContext();

  void update_proxy(const char* url,
                    const char* username, const char* password);
  // Update proxy details


  //
  // Methods used internally
  
  CORBA::Boolean proxy_info(char*&          url,
                            char*&          host,
                            CORBA::UShort&  port,
                            char*&          auth,
                            CORBA::Boolean& secure);
  // If a proxy is configured, returns true and populates url, host,
  // port, auth, secure. If the proxy requires basic authentication,
  // auth is set to the value that should be set in the Proxy-
  // Authorization header (i.e. base64 encoded username:password);
  // otherwise, auth is set to null.
  //
  // If no proxy is configured, returns false.

  virtual void copy_globals(CORBA::Boolean include_keys);

  static char* b64encode(const char* data, size_t len);
  static char* b64decode(const char* data, size_t& len);
  
protected:
  virtual const char* ctxType();
  
  void real_update_proxy(const char* url,
                         const char* username, const char* password);

  CORBA::String_var pd_proxy_url;
  CORBA::String_var pd_proxy_host;
  CORBA::UShort     pd_proxy_port;
  CORBA::String_var pd_proxy_auth;
  CORBA::Boolean    pd_proxy_secure;
};

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __HTTPCONTEXT_H__
