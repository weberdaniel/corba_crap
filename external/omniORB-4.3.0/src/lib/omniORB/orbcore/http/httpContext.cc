// -*- Mode: C++; -*-
//                            Package   : omniORB
// httpContext.cc             Created on: 27 April 2018
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2003-2019 Apasphere Ltd
//    Copyright (C) 2018      Apasphere Ltd, BMC Software
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
//      *** PROPRIETARY INTERFACE ***
//

#include <omniORB4/CORBA.h>

#include <stdlib.h>
#ifndef __WIN32__
#include <unistd.h>
#else
#include <process.h>
#endif
#include <omniORB4/minorCode.h>
#include <omniORB4/httpContext.h>
#include <omniORB4/omniURI.h>
#include <exceptiondefs.h>
#include <http/httpTransportImpl.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(httpContext);

OMNI_USING_NAMESPACE(omni)

const char*                httpContext::proxy_url                  = 0;
const char*                httpContext::proxy_username             = 0;
const char*                httpContext::proxy_password             = 0;

const char*                httpContext::certificate_authority_file = 0;
const char*                httpContext::certificate_authority_path = 0;
const char*                httpContext::key_file                   = 0;
const char*                httpContext::key_file_password          = 0;
const char*                httpContext::cipher_list                = 0;
int                        httpContext::verify_mode                =
                                              (SSL_VERIFY_PEER |
                                               SSL_VERIFY_FAIL_IF_NO_PEER_CERT);
int                        httpContext::verify_mode_incoming       = -1;
sslContext::omni_verify_cb httpContext::verify_callback            = 0;
sslContext::omni_info_cb   httpContext::info_callback              = 0;
httpCryptoManager*         httpContext::crypto_manager             = 0;

httpContext*               httpContext::singleton                  = 0;


/////////////////////////////////////////////////////////////////////////
httpContext::httpContext(const char* cafile,
                         const char* capath,
                         const char* keyfile,
                         const char* password)
  : sslContext(cafile, capath, keyfile, password)
{
}

httpContext::httpContext()
{
}

const char*
httpContext::ctxType()
{
  return "httpContext";
}

/////////////////////////////////////////////////////////////////////////
void
httpContext::copy_globals(CORBA::Boolean include_keys)
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

  real_update_proxy(proxy_url, proxy_username, proxy_password);
}


/////////////////////////////////////////////////////////////////////////
httpContext::~httpContext() {
}


/////////////////////////////////////////////////////////////////////////
void
httpContext::update_proxy(const char* url,
                          const char* username, const char* password)
{
  omni_tracedmutex_lock sync(pd_ctx_lock);
  real_update_proxy(url, username, password);
}


/////////////////////////////////////////////////////////////////////////
void
httpContext::real_update_proxy(const char* url,
                               const char* username, const char* password)
{
  if (!url) {
    pd_proxy_url    = (const char*)0;
    pd_proxy_host   = (const char*)0;
    pd_proxy_port   = 0;
    pd_proxy_auth   = (const char*)0;
    pd_proxy_secure = 0;
  }
  else {
    CORBA::String_var scheme, path, fragment;

    if (!omniURI::extractURL(url, scheme.out(), pd_proxy_host.out(),
                             pd_proxy_port, path.out(), fragment.out()))
      OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);

    pd_proxy_url = url;
    
    if (!strcmp(scheme, "https"))
      pd_proxy_secure = 1;

    else if (!strcmp(scheme, "http"))
      pd_proxy_secure = 0;

    else
      OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);

    if (!pd_proxy_port)
      pd_proxy_port = pd_proxy_secure ? 443 : 80;
    
    if (username) {
      if (!password)
        password = "";

      // The username and password are Base64 encoded
      
      size_t            ul = strlen(username) + strlen(password) + 1;
      CORBA::String_var up = CORBA::string_alloc(ul);

      sprintf((char*)up, "%s:%s", (const char*)username, (const char*)password);

      CORBA::String_var b64 = b64encode(up, ul);

      pd_proxy_auth = CORBA::string_alloc(sizeof("basic ") + strlen(b64));
      sprintf((char*)pd_proxy_auth, "basic %s", (const char*)b64);
    }
    else {
      pd_proxy_auth = (const char*)0;
    }
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
httpContext::proxy_info(char*&          url,
                        char*&          host,
                        CORBA::UShort&  port,
                        char*&          auth,
                        CORBA::Boolean& secure)
{
  omni_tracedmutex_lock sync(pd_ctx_lock);

  if (!pd_proxy_host.in())
    return 0;

  url    = CORBA::string_dup(pd_proxy_url);
  host   = CORBA::string_dup(pd_proxy_host);
  port   = pd_proxy_port;
  auth   = pd_proxy_auth.in() ? CORBA::string_dup(pd_proxy_auth) : 0;
  secure = pd_proxy_secure;

  return 1;
}


/////////////////////////////////////////////////////////////////////////
char*
httpContext::b64encode(const char* data, size_t len) {

  // Output to a memory BIO with a Base64 filter BIO
  BIO* mem_bio = BIO_new(BIO_s_mem());
  BIO* b64_bio = BIO_new(BIO_f_base64());

  BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);
  BIO_set_close(b64_bio, BIO_CLOSE);
  BIO_push(b64_bio, mem_bio);

  // Write to the Base64 BIO
  BIO_write(b64_bio, data, len);
  BIO_flush(b64_bio);

  // Extract the data from the memory BIO
  BUF_MEM* bm;
  BIO_get_mem_ptr(mem_bio, &bm);

  char* ret = CORBA::string_alloc(bm->length);
  memcpy(ret, bm->data, bm->length);
  ret[bm->length] = '\0';

  BIO_free_all(b64_bio);

  return ret;
}


/////////////////////////////////////////////////////////////////////////
char*
httpContext::b64decode(const char* data, size_t& len) {

  // Read from a memory BIO with a Base64 filter BIO
  size_t b64_len = strlen(data);
  BIO*   mem_bio = BIO_new_mem_buf(data, b64_len);
  BIO*   b64_bio = BIO_new(BIO_f_base64());

  BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);
  BIO_set_close(b64_bio, BIO_CLOSE);
  BIO_push(b64_bio, mem_bio);

  // Read from the Base64 BIO
  size_t            out_len = (b64_len * 3) / 4;
  CORBA::String_var out     = CORBA::string_alloc(out_len);  
  int               l       = BIO_read(b64_bio, (void*)(char*)out, out_len);

  BIO_free_all(b64_bio);

  if (l < 0)
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidBase64Data, CORBA::COMPLETED_NO);

  out[l] = '\0';
  
  len = (size_t)l;
  return out._retn();
}


/////////////////////////////////////////////////////////////////////////
httpContext::PeerDetails::
~PeerDetails()
{
  if (pd_cert)
    X509_free(pd_cert);
}
