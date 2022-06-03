// -*- Mode: C++; -*-
//                            Package   : omniORB
// httpTransportImpl.cc       Created on: 18 April 2018
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2002-2019 Apasphere Ltd
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

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <SocketCollection.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/httpContext.h>
#include <objectAdapter.h>
#include <tcpSocket.h>
#include <http/httpConnection.h>
#include <http/httpAddress.h>
#include <http/httpEndpoint.h>
#include <http/httpTransportImpl.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <initialiser.h>
#include <orbParameters.h>
#include <orbOptions.h>
#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(httpAddress);
OMNI_FORCE_LINK(httpConnection);
OMNI_FORCE_LINK(httpEndpoint);
OMNI_FORCE_LINK(httpActive);

OMNI_EXPORT_LINK_FORCE_SYMBOL(omnihttpTP);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
omni_time_t httpTransportImpl::httpsAcceptTimeOut(10);


/////////////////////////////////////////////////////////////////////////
httpTransportImpl::httpTransportImpl(httpContext* ctx)
  : giopTransportImpl("giop:http"), pd_ctx(ctx) {
}

/////////////////////////////////////////////////////////////////////////
httpTransportImpl::~httpTransportImpl() {
}

/////////////////////////////////////////////////////////////////////////
giopEndpoint*
httpTransportImpl::toEndpoint(const char* param)
{
  if (isValid(param))
    return (giopEndpoint*)(new httpEndpoint(param, pd_ctx));
  else
    return 0;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
httpTransportImpl::isValid(const char* param) {

  CORBA::String_var scheme, host, path, fragment;
  CORBA::UShort     port;

  CORBA::Boolean ok = omniURI::extractURL(param,
                                          scheme.out(), host.out(),
                                          port, path.out(), fragment.out());

  return ok && (!strcmp(scheme, "https") ||
                !strcmp(scheme, "http")  ||
                !strcmp(scheme, "wss")   ||
                !strcmp(scheme, "ws"));
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
httpTransportImpl::toAddress(const char* param) {

  if (isValid(param))
    return (giopAddress*)(new httpAddress(param, pd_ctx));
  else
    return 0;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
httpTransportImpl::addToIOR(const char* param, IORPublish* eps) {

  if (param) {
    omniIOR::add_TAG_OMNIORB_HTTP_TRANS(param, eps);
    return 1;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
const omnivector<const char*>*
httpTransportImpl::getInterfaceAddress() {
  return giopTransportImpl::getInterfaceAddress("giop:tcp");
}


/////////////////////////////////////////////////////////////////////////////
httpCrypto::~httpCrypto() {}
httpCryptoManager::~httpCryptoManager() {}


//
// URI Handler

/////////////////////////////////////////////////////////////////////////////
class httpURIHandler : public omniURI::URIHandler {
public:
  CORBA::Boolean    supports     (const char* uri);
  CORBA::Object_ptr toObject     (const char* uri, unsigned int cycles);
  CORBA::Boolean    syntaxIsValid(const char* uri);
};

CORBA::Boolean
httpURIHandler::supports(const char* uri)
{
  return (!strncmp(uri, "http://",  7) ||
          !strncmp(uri, "https://", 8) ||
          !strncmp(uri, "ws://",    5) ||
          !strncmp(uri, "wss://",   6));
}

CORBA::Object_ptr
httpURIHandler::toObject(const char* uri, unsigned int cycles)
{
  CORBA::String_var scheme, host, path, fragment;
  CORBA::UShort     port;

  CORBA::Boolean ok = omniURI::extractURL(uri, scheme.out(), host.out(),
                                          port, path.out(), fragment.out());
  if (!ok)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);

  if (!(*(const char*)scheme &&
        *(const char*)host   &&
        *(const char*)path   &&
        *(const char*)fragment)) {
    
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadSchemeSpecificPart,
		  CORBA::COMPLETED_NO);
  }

  // Unescape key from the fragment
  unsigned int      key_size;
  const char*       key_ptr = (const char*)fragment;
  CORBA::String_var key_str = omniURI::unescape(key_ptr, key_size);

  _CORBA_Unbounded_Sequence_Octet key;
  key.replace(key_size, key_size, (CORBA::Octet*)(const char*)key_str);

  // Build URL without the key
  CORBA::String_var prefix = CORBA::string_alloc(strlen(scheme) + 4);
  sprintf((char*)prefix, "%s://", (const char*)scheme);
  
  CORBA::String_var ior_url = omniURI::buildURI(prefix, host, port, path, 0);

  
  // Marshal the HTTP transport tagged component
  IOP::MultipleComponentProfile tagged_components;
  tagged_components.length(1);

  IOP::TaggedComponent& c = tagged_components[0];
  c.tag = IOP::TAG_OMNIORB_HTTP_TRANS;

  cdrEncapsulationStream s(CORBA::ULong(0), CORBA::Boolean(1));
  s.marshalRawString(ior_url);
  s.setOctetSeq(c.component_data);

  IIOP::Address addrs[1];
  addrs[0].host = host;
  addrs[0].port = 0;

  GIOP::Version ver;
  ver.major = 1; ver.minor = 2;

  omniIOR* ior = new omniIOR((const char*)"", key, addrs, 1, ver,
                             omniIOR::NoInterceptor, &tagged_components);

  omniObjRef* objref = omni::createObjRef(CORBA::Object::_PD_repoId,ior,0);
  OMNIORB_ASSERT(objref);

  return (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}

CORBA::Boolean
httpURIHandler::syntaxIsValid(const char* uri)
{
  CORBA::String_var scheme, host, path, fragment;
  CORBA::UShort     port;

  CORBA::Boolean ok = omniURI::extractURL(uri, scheme.out(), host.out(),
                                          port, path.out(), fragment.out());
  return ok;
}


//
// Options handlers

/////////////////////////////////////////////////////////////////////////////
class httpProxyHandler : public orbOptions::Handler {
public:

  httpProxyHandler() : 
    orbOptions::Handler("httpProxy",
                        "httpProxy = <proxy URL>",
                        1,
                        "-ORBhttpProxy <proxy URL>") {}

  void visit(const char* value,orbOptions::Source)
  {    
    httpContext::proxy_url = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
                            httpContext::proxy_url ?
                            httpContext::proxy_url : "<unset>",
                            result);
  }
};

static httpProxyHandler httpProxyHandler_;


/////////////////////////////////////////////////////////////////////////////
class httpProxyUsernameHandler : public orbOptions::Handler {
public:

  httpProxyUsernameHandler() : 
    orbOptions::Handler("httpProxyUsername",
                        "httpProxyUsername = <username>",
                        1,
                        "-ORBhttpProxyUsername <username>") {}

  void visit(const char* value,orbOptions::Source)
  {    
    httpContext::proxy_username = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
                            httpContext::proxy_username ?
                            httpContext::proxy_username : "<unset>",
                            result);
  }
};

static httpProxyUsernameHandler httpProxyUsernameHandler_;


/////////////////////////////////////////////////////////////////////////////
class httpProxyPasswordHandler : public orbOptions::Handler {
public:

  httpProxyPasswordHandler() : 
    orbOptions::Handler("httpProxyPassword",
                        "httpProxyPassword = <password>",
                        1,
                        "-ORBhttpProxyPassword <password>") {}

  void visit(const char* value,orbOptions::Source)
  {    
    httpContext::proxy_password = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
                            httpContext::proxy_password ? "*****" : "<unset>",
                            result);
  }
};

static httpProxyPasswordHandler httpProxyPasswordHandler_;


/////////////////////////////////////////////////////////////////////////////
class httpsCAFileHandler : public orbOptions::Handler {
public:

  httpsCAFileHandler() : 
    orbOptions::Handler("httpsCAFile",
                        "httpsCAFile = <certificate authority file>",
                        1,
                        "-ORBhttpsCAFile <certificate authority file>") {}

  void visit(const char* value,orbOptions::Source)
  {    
    httpContext::certificate_authority_file = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
                            httpContext::certificate_authority_file ?
                            httpContext::certificate_authority_file : "<unset>",
                            result);
  }
};

static httpsCAFileHandler httpsCAFileHandler_;


/////////////////////////////////////////////////////////////////////////////
class httpsCAPathHandler : public orbOptions::Handler {
public:

  httpsCAPathHandler() : 
    orbOptions::Handler("httpsCAPath",
                        "httpsCAPath = <certificate authority path>",
                        1,
                        "-ORBhttpsCAPath <certificate authority path>") {}

  void visit(const char* value,orbOptions::Source)
  {    
    httpContext::certificate_authority_path = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
                            httpContext::certificate_authority_path ?
                            httpContext::certificate_authority_path : "<unset>",
                            result);
  }
};

static httpsCAPathHandler httpsCAPathHandler_;


/////////////////////////////////////////////////////////////////////////////
class httpsKeyFileHandler : public orbOptions::Handler {
public:

  httpsKeyFileHandler() : 
    orbOptions::Handler("httpsKeyFile",
                        "httpsKeyFile = <key file>",
                        1,
                        "-ORBhttpsKeyFile <key file>") {}

  void visit(const char* value,orbOptions::Source)
  {    
    httpContext::key_file = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
                            httpContext::key_file ?
                            httpContext::key_file : "<unset>",
                            result);
  }
};

static httpsKeyFileHandler httpsKeyFileHandler_;


/////////////////////////////////////////////////////////////////////////////
class httpsKeyPasswordHandler : public orbOptions::Handler {
public:

  httpsKeyPasswordHandler() : 
    orbOptions::Handler("httpsKeyPassword",
                        "httpsKeyPassword = <key file password>",
                        1,
                        "-ORBhttpsKeyPassword <key file password>") {}

  void visit(const char* value,orbOptions::Source)
  {    
    httpContext::key_file_password = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
                            httpContext::key_file_password ? "****" : "<unset>",
                            result);
  }
};

static httpsKeyPasswordHandler httpsKeyPasswordHandler_;


/////////////////////////////////////////////////////////////////////////////
class httpsCipherListHandler : public orbOptions::Handler {
public:

  httpsCipherListHandler() : 
    orbOptions::Handler("httpsCipherList",
                        "httpsCipherList = <cipher list>",
                        1,
                        "-ORBhttpsCipherList <cipher list>") {}

  void visit(const char* value, orbOptions::Source)
  {    
    httpContext::cipher_list = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
                            httpContext::cipher_list ?
                            httpContext::cipher_list : "<unset>",
                            result);
  }
};

static httpsCipherListHandler httpsCipherListHandler_;


/////////////////////////////////////////////////////////////////////////////
static int
verifyModeValue(const char* key, const char* value, CORBA::Boolean unset_ok)
{
  if (!strcmp(value, "none"))
    return SSL_VERIFY_NONE;

  if (unset_ok && !strcmp(value, "unset"))
    return -1;
  
  int mode = 0;
  CORBA::String_var val(value); // Copy
  char* valc = (char*)val;
  char* c;

  while (*valc) {
    for (c=valc; *c && *c != ','; ++c) {}

    if (*c == ',')
      *c++ = '\0';

    if (!strcmp(valc, "peer"))
      mode |= SSL_VERIFY_PEER;

    else if (!strcmp(valc, "fail"))
      mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;

    else if (!strcmp(valc, "once"))
      mode |= SSL_VERIFY_CLIENT_ONCE;

    else
      throw orbOptions::BadParam(key, val, "Invalid verify option");
      
    valc = c;
  }
  if (mode && !(mode & SSL_VERIFY_PEER))
    throw orbOptions::BadParam(key, val, "Invalid verify option");
  
  return mode;
}

static void
verifyModeDump(const char* key, int mode, orbOptions::sequenceString& result)
{
  char buf[20];

  buf[0] = '\0';

  if (mode == -1) {
    strcpy(buf, "unset");
  }
  else if (!(mode & SSL_VERIFY_PEER)) {
    strcpy(buf, "none");
  }
  else {
    strcpy(buf, "peer");

    if (mode & SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
      strcat(buf, ",fail");

    if (mode & SSL_VERIFY_CLIENT_ONCE)
      strcat(buf, ",once");
  }
  orbOptions::addKVString(key, buf, result);
}

class httpsVerifyModeHandler : public orbOptions::Handler {
public:

  httpsVerifyModeHandler() : 
    orbOptions::Handler("httpsVerifyMode",
                        "httpsVerifyMode = <mode>",
                        1,
                        "-ORBhttpsVerifyMode < \"none\" | \"peer[,fail][,once]\" >")
  {}

  void visit(const char* value, orbOptions::Source)
  {    
    httpContext::verify_mode = verifyModeValue(key(), value, 0);
  }

  void dump(orbOptions::sequenceString& result)
  {
    verifyModeDump(key(), httpContext::verify_mode, result);
  }
};

static httpsVerifyModeHandler httpsVerifyModeHandler_;


/////////////////////////////////////////////////////////////////////////////
class httpsVerifyModeIncomingHandler : public orbOptions::Handler {
public:

  httpsVerifyModeIncomingHandler() : 
    orbOptions::Handler("httpsVerifyModeIncoming",
                        "httpsVerifyModeIncoming = <mode>",
                        1,
                        "-ORBhttpsVerifyModeIncoming < \"unset\" | \"none\" | \"peer[,fail][,once]\" >")
  {}

  void visit(const char* value, orbOptions::Source)
  {    
    httpContext::verify_mode_incoming = verifyModeValue(key(), value, 1);
  }

  void dump(orbOptions::sequenceString& result)
  {
    verifyModeDump(key(), httpContext::verify_mode, result);
  }
};

static httpsVerifyModeIncomingHandler httpsVerifyModeIncomingHandler_;


/////////////////////////////////////////////////////////////////////////////
class httpsAcceptTimeOutHandler : public orbOptions::Handler {
public:

  httpsAcceptTimeOutHandler() : 
    orbOptions::Handler("httpsAcceptTimeOut",
                        "httpsAcceptTimeOut = n >= 0 in msecs",
                        1,
                        "-ORBhttpsAcceptTimeOut < n >= 0 in msecs >") {}

  void visit(const char* value,orbOptions::Source) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
                                 "Expect n >= 0 in msecs");
    }
    httpTransportImpl::httpsAcceptTimeOut.assign(v / 1000, (v % 1000) * 1000000);
  }

  void dump(orbOptions::sequenceString& result) {
    CORBA::ULong v = httpTransportImpl::httpsAcceptTimeOut.s * 1000 +
                     httpTransportImpl::httpsAcceptTimeOut.ns / 1000000;

    orbOptions::addKVULong(key(),v,result);
  }

};

static httpsAcceptTimeOutHandler httpsAcceptTimeOutHandler_;


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
static httpTransportImpl* _the_httpTransportImpl = 0;
static httpURIHandler     _the_httpURIHandler;

class omni_httpTransport_initialiser : public omniInitialiser {
public:

  omni_httpTransport_initialiser() {
    orbOptions::singleton().registerHandler(httpProxyHandler_);
    orbOptions::singleton().registerHandler(httpProxyUsernameHandler_);
    orbOptions::singleton().registerHandler(httpProxyPasswordHandler_);
    orbOptions::singleton().registerHandler(httpsCAFileHandler_);
    orbOptions::singleton().registerHandler(httpsCAPathHandler_);
    orbOptions::singleton().registerHandler(httpsKeyFileHandler_);
    orbOptions::singleton().registerHandler(httpsKeyPasswordHandler_);
    orbOptions::singleton().registerHandler(httpsCipherListHandler_);
    orbOptions::singleton().registerHandler(httpsVerifyModeHandler_);
    orbOptions::singleton().registerHandler(httpsVerifyModeIncomingHandler_);
    orbOptions::singleton().registerHandler(httpsAcceptTimeOutHandler_);
    omniInitialiser::install(this);
  }

  void attach() {
    if (_the_httpTransportImpl) return;

    omniURI::registerURIHandler(&_the_httpURIHandler);
    
    if (!httpContext::singleton) {

      if (omniORB::trace(5)) {
        omniORB::logger log;
        log << "No HTTP context object supplied. Attempt to create one "
            << "with the default constructor.\n";
      }

      // Create the default singleton
      httpContext::singleton = new httpContext();
      httpContext::singleton->copy_globals(1);
    }

    if (!RAND_status()) {
      omniORB::logs(1, "The OpenSSL random number generator has "
                    "not been seeded.");
      OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);
    }
    
    httpContext::singleton->internal_initialise();
    _the_httpTransportImpl = new httpTransportImpl(httpContext::singleton);
  }

  void detach() { 
    if (_the_httpTransportImpl) delete _the_httpTransportImpl;
    _the_httpTransportImpl = 0;

    if (httpContext::crypto_manager) delete httpContext::crypto_manager;
    httpContext::crypto_manager = 0;
    
    if (httpContext::singleton) delete httpContext::singleton;
    httpContext::singleton = 0;

    omniURI::unregisterURIHandler(&_the_httpURIHandler);
  }
};

static omni_httpTransport_initialiser initialiser;


OMNI_NAMESPACE_END(omni)
