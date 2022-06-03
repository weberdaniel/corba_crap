// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslContext.h               Created on: 29 May 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2005-2012 Apasphere Ltd
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

#ifndef __SSLCONTEXT_H__
#define __SSLCONTEXT_H__

#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(omnisslTP);


#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_SSL_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

#define crypt _openssl_broken_crypt
#include <openssl/ssl.h>
#undef crypt

OMNI_NAMESPACE_BEGIN(omni)

class sslContext {
public:

  static _core_attr sslContext* singleton;

  // sslContext singleton object. This object is used to manage all
  // connections in the SSL transport.
  //
  // Application code can populate this pointer with a suitable
  // sslContext object prior to calling CORBA::ORB_init. If it is not
  // set, a default instance is created. This base class uses the
  // global variables defined below to initialise itself, but an
  // application-provided subclass may behave differently.
  //
  // The singleton is deleted by ORB::destroy(). If the application
  // provides its own object here, and it calls ORB::destroy(), it
  // must set the singleton again if it is going to call ORB_init()
  // again.
  
  
  // These parameters must be set or else the default way to
  // initialise a sslContext singleton will not be used.
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

  typedef int  (*omni_verify_cb)(int preverify_ok, X509_STORE_CTX *x509_ctx);
  typedef void (*omni_info_cb)  (const SSL *ssl, int where, int ret);
  
  static _core_attr int            verify_mode;
  static _core_attr int            verify_mode_incoming;
  static _core_attr omni_verify_cb verify_callback;
  static _core_attr omni_info_cb   info_callback;

  // If this parameter is true (the default), interceptor
  // peerdetails() calls return a pointer to an
  // sslContext::PeerDetails object; if false, peerdetails() returns
  // an X509*.

  static _core_attr CORBA::Boolean full_peerdetails;

  class PeerDetails {
  public:
    inline PeerDetails(SSL* s, X509* c, CORBA::Boolean v)
      : pd_ssl(s), pd_cert(c), pd_verified(v) {}

    ~PeerDetails();

    inline SSL*           ssl()      { return pd_ssl; }
    inline X509*          cert()     { return pd_cert; }
    inline CORBA::Boolean verified() { return pd_verified; }

  private:
    SSL*           pd_ssl;
    X509*          pd_cert;
    CORBA::Boolean pd_verified;
  };


  sslContext(const char* cafile, const char* keyfile, const char* password);
  // Construct with CA file, key and password.

  sslContext(const char* cafile, const char* capath,
             const char* keyfile, const char* password);
  // Construct with CA file, CA path, key and password. Either cafile
  // or capath, but not both, may be zero.

  sslContext();
  // Construct empty, ready to set with set_globals().
  
  virtual ~sslContext();

  virtual void reinit(CORBA::Boolean read_globals=1);
  // Re-initialise, replacing the SSL_CTX with a new one, with
  // different keys or parameters. Note that the old one is leaked,
  // because it may be in use in existing connections.
  //
  // If read_globals is true, reads new configuration from the global
  // variables defined above; if false, uses the members set in this
  // sslContext object.


  //
  // Methods to set parameters in this sslContext object. Start-up
  // parameters must be set before ORB_init(). After ORB_init, load
  // these parameters by calling reinit(false).

  void update_CA(const char* cafile, const char* capath);
  void update_key(const char* keyfile, const char* password);
  void update_cipher_list(const char* cipher_list);
  void update_verify_mode(int mode, int mode_incoming, omni_verify_cb callback);
  void update_info_cb(omni_info_cb callback);


  //
  // Methods used internally
  
  virtual void copy_globals(CORBA::Boolean include_keys);
  // Copy the state from the global variables into this context.
  
  virtual void internal_initialise();
  // Initialise the SSL_CTX from the parameters. Only called
  // internally to omniORB.

  inline ::SSL* ssl_new()
  {
    omni_tracedmutex_lock sync(pd_ctx_lock);
    return SSL_new(pd_ctx);
  }
  
  void set_incoming_verify(SSL* ssl);
  // Set the verify mode on an incoming connection.
  
protected:
  virtual const char* ctxType();
  // Returns string form of the context type.
  
  //
  // Virtual functions that populate the SSL_CTX in pd_ctx. Subclasses
  // may override these if required. Thse are called while holding
  // pd_ctx_lock.

  SSL_CTX* get_SSL_CTX() const
  {
    // Get the SSL_CTX without locking. Should only be used by legacy
    // subclasses that override the virtual set_... methods defined
    // below,
    return pd_ctx;
  }

  virtual SSL_METHOD* set_method(); 
  // Method to pass to SSL_CTX_new. Defaults to return TLS_method().

  virtual void set_supported_versions(); 
  // Default to
  //   SSL_CTX_set_options(ssl_ctx,
  //                       SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
  //                       SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
  // That is, only accept TLS 1.2 or later.

  virtual void set_CA();
  // Set CA details. Calls both SSL_CTX_load_verify_locations and
  // SSL_CTX_set_client_CA_list.

  virtual void set_certificate();
  // Sets the certificate of this server by calling
  // SSL_CTX_use_certificate_chain_file.

  virtual void set_cipher_list();
  // Sets the cipher list control string with SSL_CTX_set_cipher_list.
  
  virtual void set_privatekey();
  // Sets the private key to the configured key file with
  // SSL_CTX_use_PrivateKey_file. Notice that this file also contains
  // the server's certificate.

  virtual void set_DH();
  // Set the Diffie-Hellman parameters.
  
  virtual void set_ephemeralRSA();
  // Sets the RSA key for ephemeral RSA key exchange. The default
  // implementation does nothing.

  virtual void set_verify();
  // Set the SSL verify mode and verify callback.

  virtual void set_info_cb();
  // Sets the info callback.

  
  virtual void create_ctx();
  // Create the SSL_CTX.
  
  CORBA::String_var pd_cafile;
  CORBA::String_var pd_capath;
  CORBA::String_var pd_keyfile;
  CORBA::String_var pd_password;
  CORBA::String_var pd_password_in_ctx;
  CORBA::String_var pd_cipher_list;
  int               pd_verify_mode;
  int               pd_verify_mode_incoming;
  omni_verify_cb    pd_verify_cb;
  omni_info_cb      pd_info_cb;

  SSL_CTX*          pd_ctx;
  omni_tracedmutex  pd_ctx_lock;
};

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __SSLCONTEXT_H__
