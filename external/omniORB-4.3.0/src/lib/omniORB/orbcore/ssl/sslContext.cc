// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslContext.cc              Created on: 29 May 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2012 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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

#include <omniORB4/CORBA.h>

#include <stdlib.h>
#ifndef __WIN32__
#include <unistd.h>
#else
#include <process.h>
#endif
#include <omniORB4/minorCode.h>
#include <omniORB4/sslContext.h>
#include <exceptiondefs.h>
#include <ssl/sslTransportImpl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(sslContext);

OMNI_USING_NAMESPACE(omni)

static void report_error();

const char*    sslContext::certificate_authority_file  = 0;
const char*    sslContext::certificate_authority_path  = 0;
const char*    sslContext::key_file                    = 0;
const char*    sslContext::key_file_password           = 0;
const char*    sslContext::cipher_list                 = 0;
int            sslContext::verify_mode                 =
                            (SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT);
int            sslContext::verify_mode_incoming        = -1;
sslContext::omni_verify_cb sslContext::verify_callback = 0;
sslContext::omni_info_cb   sslContext::info_callback   = 0;
CORBA::Boolean sslContext::full_peerdetails            = 1;

sslContext*    sslContext::singleton                   = 0;


/////////////////////////////////////////////////////////////////////////
sslContext::sslContext(const char* cafile,
		       const char* keyfile,
		       const char* password)
  : pd_cafile(cafile), pd_keyfile(keyfile), pd_password(password), pd_ctx(0)
{
}

sslContext::sslContext(const char* cafile,
                       const char* capath,
		       const char* keyfile,
		       const char* password)
  : pd_cafile(cafile), pd_capath(capath),
    pd_keyfile(keyfile), pd_password(password), pd_ctx(0)
{
}

sslContext::sslContext()
  : pd_ctx(0)
{
}

const char*
sslContext::ctxType()
{
  return "sslContext";
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::internal_initialise() {

  if (pd_ctx)
    return;

  create_ctx();
}


/////////////////////////////////////////////////////////////////////////
void
sslContext::update_CA(const char* cafile, const char* capath)
{
  omni_tracedmutex_lock sync(pd_ctx_lock);
  pd_cafile = cafile;
  pd_capath = capath;
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::update_key(const char* keyfile, const char* password)
{
  omni_tracedmutex_lock sync(pd_ctx_lock);

  // A pointer to the password string is held by pd_ctx, so we must
  // not free it yet.
  if (!(const char*)pd_password_in_ctx)
    pd_password_in_ctx = pd_password._retn();

  pd_keyfile  = keyfile;
  pd_password = password;
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::update_cipher_list(const char* cipher_list)
{
  omni_tracedmutex_lock sync(pd_ctx_lock);
  pd_cipher_list = cipher_list;
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::update_verify_mode(int mode, int mode_incoming,
                               omni_verify_cb callback)
{
  omni_tracedmutex_lock sync(pd_ctx_lock);
  pd_verify_mode          = mode;
  pd_verify_mode_incoming = mode_incoming;
  pd_verify_cb            = callback;
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::update_info_cb(omni_info_cb callback)
{
  omni_tracedmutex_lock sync(pd_ctx_lock);
  pd_info_cb = callback;
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::create_ctx()
{
  pd_ctx = SSL_CTX_new(set_method());
  if (!pd_ctx) {
    report_error();
    OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError,
		  CORBA::COMPLETED_NO);
  }

  try {
    static const unsigned char session_id_context[] = "omniORB";
    size_t session_id_len =
      (sizeof(session_id_context) >= SSL_MAX_SSL_SESSION_ID_LENGTH ?
       SSL_MAX_SSL_SESSION_ID_LENGTH : sizeof(session_id_context));

    if (SSL_CTX_set_session_id_context(pd_ctx,
                                       session_id_context,
                                       session_id_len) != 1) {
      report_error();
      OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,
                    CORBA::COMPLETED_NO);
    }

    set_supported_versions();
    set_certificate();
    set_privatekey();
    set_CA();
    set_DH();
    set_ephemeralRSA();
    set_cipher_list();
    set_verify();
    set_info_cb();
  }
  catch (...) {
    SSL_CTX_free(pd_ctx);
    pd_ctx = 0;
    throw;
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::reinit(CORBA::Boolean read_globals)
{
  omni_tracedmutex_lock sync(pd_ctx_lock);

  if (read_globals)
    copy_globals(1);

  // Release the stored password
  SSL_CTX_set_default_passwd_cb_userdata(pd_ctx, 0);
  SSL_CTX_free(pd_ctx);
  pd_password_in_ctx = (const char*)0;
  
  create_ctx();
}


/////////////////////////////////////////////////////////////////////////
void
sslContext::copy_globals(CORBA::Boolean include_keys)
{
  if (include_keys) {
    // A pointer to the password string is held by pd_ctx, so we must
    // not free it yet.
    if (!(const char*)pd_password_in_ctx)
      pd_password_in_ctx = pd_password._retn();
    
    pd_cafile   = certificate_authority_file;
    pd_capath   = certificate_authority_path;
    pd_keyfile  = key_file;
    pd_password = key_file_password;
  }
  pd_cipher_list          = cipher_list;
  pd_verify_mode          = verify_mode;
  pd_verify_mode_incoming = verify_mode_incoming;
  pd_verify_cb            = verify_callback;
  pd_info_cb              = info_callback;
}


/////////////////////////////////////////////////////////////////////////
sslContext::~sslContext() {
  if (pd_ctx) {
    SSL_CTX_free(pd_ctx);
  }
}

/////////////////////////////////////////////////////////////////////////
sslContext::PeerDetails::~PeerDetails() {
  if (pd_cert)
    X509_free(pd_cert);
}



/////////////////////////////////////////////////////////////////////////
void
sslContext::set_incoming_verify(SSL* ssl)
{
  omni_tracedmutex_lock sync(pd_ctx_lock);

  if (pd_verify_mode_incoming != -1 &&
      pd_verify_mode_incoming != pd_verify_mode) {

    SSL_set_verify(ssl, pd_verify_mode_incoming, pd_verify_cb);
  }
}

/////////////////////////////////////////////////////////////////////////
SSL_METHOD*
sslContext::set_method() {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  return OMNI_CONST_CAST(SSL_METHOD*, SSLv23_method());
#else
  return OMNI_CONST_CAST(SSL_METHOD*, TLS_method());
#endif
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_supported_versions() {
  SSL_CTX_set_options(pd_ctx,
                      SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
                      SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_CA() {

  if (pd_cafile.in() || pd_capath.in()) {

    if (!SSL_CTX_load_verify_locations(pd_ctx, pd_cafile, pd_capath)) {
      if (omniORB::trace(1)) {
        omniORB::logger log;
        log << "Failed to set CA";

        if (pd_cafile)
          log << " file '" << pd_cafile << "'";

        if (certificate_authority_path)
          log << " path '" << certificate_authority_path << "'";

        log << ".\n";
      }
      
      report_error();
      OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
    }
  }
    
  if (pd_cafile.in()) {
    // Set the client CA list
    SSL_CTX_set_client_CA_list(pd_ctx, SSL_load_client_CA_file(pd_cafile));
  }
  
  // We no longer set the verify depth to 1, to use the default of 9.
  //  SSL_CTX_set_verify_depth(pd_ctx,1);
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_certificate() {
  {
    if (!pd_keyfile) {
      if (omniORB::trace(5)) {
	omniORB::logger log;
	log << ctxType() << " certificate file is not set.\n";
      }
      return;
    }
  }

  if (!SSL_CTX_use_certificate_chain_file(pd_ctx, pd_keyfile)) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Failed to use certificate file '" << pd_keyfile << "'.\n";
    }
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_cipher_list() {
  if (pd_cipher_list && !SSL_CTX_set_cipher_list(pd_ctx, pd_cipher_list)) {
    report_error();
    OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);
  }
}

/////////////////////////////////////////////////////////////////////////
static int sslContext_password_cb (char *buf, int num, int, void* ud) {
  const char* ssl_password = (const char*)ud;

  int size = strlen(ssl_password);
  if (num < size+1)
    return 0;

  strcpy(buf, ssl_password);
  return size;
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_privatekey() {

  if (!pd_password) {
    if (omniORB::trace(5)) {
      omniORB::logger log;
      log << ctxType() << " private key password is not set.\n";
    }
    return;
  }

  SSL_CTX_set_default_passwd_cb(pd_ctx, sslContext_password_cb);
  SSL_CTX_set_default_passwd_cb_userdata(pd_ctx,
                                         (void*)(const char*)pd_password);

  if (!SSL_CTX_use_PrivateKey_file(pd_ctx, pd_keyfile, SSL_FILETYPE_PEM)) {
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_DH() {

  DH* dh = DH_new();
  if (dh == 0) {
    OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);
  }

  static unsigned char dh2048_p[]={
    0xEC, 0x2A, 0x0E, 0x97, 0x10, 0x9F, 0x3A, 0xA9, 0x79, 0x32, 
    0x5B, 0x99, 0x92, 0x2B, 0x11, 0x6C, 0x11, 0x01, 0x93, 0xEE, 
    0x48, 0xFC, 0xD9, 0x78, 0x43, 0xA4, 0xE0, 0x66, 0x17, 0xD3, 
    0xC8, 0xFE, 0xBF, 0x2F, 0x45, 0x8C, 0xE0, 0x13, 0x8F, 0x60, 
    0x0C, 0x5B, 0xBA, 0xF3, 0x09, 0xAD, 0x63, 0x93, 0x30, 0x1D, 
    0xDB, 0x94, 0xD8, 0xB6, 0x9A, 0xA1, 0x14, 0x66, 0xC6, 0x41, 
    0xF4, 0x5B, 0x4C, 0xB6, 0x94, 0x21, 0xD7, 0x2C, 0x52, 0x93, 
    0xB8, 0x07, 0x93, 0x44, 0x90, 0x15, 0xC2, 0x0C, 0xF8, 0x82, 
    0x2C, 0x10, 0x86, 0x4D, 0xC0, 0x11, 0xD8, 0x47, 0xD3, 0xDD, 
    0xC0, 0xDF, 0xDB, 0x4F, 0x46, 0xB3, 0xF6, 0xF8, 0xDD, 0xCA, 
    0x6C, 0xD1, 0xD5, 0xCC, 0xC1, 0x3E, 0xC5, 0x33, 0x35, 0xF8, 
    0x5D, 0x84, 0xC4, 0x0B, 0x85, 0xF2, 0x2D, 0x54, 0x64, 0x55, 
    0x87, 0x0A, 0x6C, 0xDA, 0x04, 0xD2, 0xAD, 0xA1, 0xC3, 0x0F, 
    0xB3, 0x80, 0xAC, 0x74, 0xAD, 0x33, 0x9C, 0x98, 0xFC, 0x60, 
    0xFB, 0xD2, 0x9F, 0x69, 0xC7, 0xE6, 0xDC, 0x10, 0xB5, 0xD4, 
    0x77, 0x28, 0x98, 0x8C, 0xDA, 0xBC, 0xEE, 0x32, 0x3B, 0xD4, 
    0x7C, 0x63, 0x44, 0x67, 0xB6, 0x32, 0x32, 0xBB, 0x1F, 0xFE, 
    0xA6, 0x0F, 0x8C, 0xFF, 0x76, 0x6D, 0x77, 0xFD, 0xE8, 0xDE, 
    0x20, 0x7B, 0x14, 0x1F, 0xA5, 0x5C, 0xA8, 0x96, 0x4A, 0xDF, 
    0xD4, 0xD3, 0xE0, 0x39, 0xE6, 0x4F, 0x92, 0x20, 0xAF, 0x6F, 
    0x4B, 0x1E, 0x60, 0xBA, 0xA5, 0xCB, 0x10, 0x26, 0x14, 0xEE, 
    0x08, 0x1C, 0x78, 0x94, 0x39, 0x77, 0xE1, 0x8E, 0x70, 0xDD, 
    0x41, 0xEA, 0x6F, 0x8A, 0x25, 0xF5, 0xBF, 0x9E, 0xA0, 0xA4, 
    0xE1, 0xA7, 0x06, 0x8F, 0x5D, 0xB5, 0x93, 0x4A, 0x12, 0xF6, 
    0x03, 0xDC, 0x04, 0x16, 0xCE, 0x63, 0xCE, 0x22, 0x7E, 0xDC, 
    0x4B, 0x70, 0xE1, 0x2C, 0x98, 0x83
  };
  static unsigned char dh2048_g[]={
    0x02,
  };

  BIGNUM* p = BN_bin2bn(dh2048_p, sizeof(dh2048_p), 0);
  BIGNUM* g = BN_bin2bn(dh2048_g, sizeof(dh2048_g), 0);
  
  if (!p || !g) {
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  dh->p = p;
  dh->g = g;
#else
  DH_set0_pqg(dh, p, 0, g);
#endif
  
  SSL_CTX_set_tmp_dh(pd_ctx, dh);
  DH_free(dh);
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_ephemeralRSA() {

  // Default implementation does nothing. To support low-grade
  // ephemeral RSA key exchange, use a subclass with code like the
  // following:

#if 0
  RSA *rsa;

  rsa = RSA_generate_key(512,RSA_F4,NULL,NULL);

  if (!SSL_CTX_set_tmp_rsa(pd_ctx,rsa)) {
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }
  RSA_free(rsa);
#endif
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_verify() {
  SSL_CTX_set_verify(pd_ctx, pd_verify_mode, pd_verify_cb);
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_info_cb() {
  SSL_CTX_set_info_callback(pd_ctx, pd_info_cb);
}

/////////////////////////////////////////////////////////////////////////
static void report_error() {

  if (omniORB::trace(1)) {
    char buf[128];
    ERR_error_string_n(ERR_get_error(),buf,128);
    omniORB::logger log;
    log << "OpenSSL: " << (const char*) buf << "\n";
  }
}
