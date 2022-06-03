// -*- Mode: C++; -*-
//                            Package   : omniORB
// httpCrypto.cc              Created on: 2 July 2018
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2018-2019 Apasphere Ltd
//    Copyright (C) 2018      Apasphere Ltd, BMC Software
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
//    Implementations of the httpCrypto and httpCryptoManager abstract classes.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/httpContext.h>
#include <omniORB4/httpCrypto.h>
#include <omniORB4/connectionInfo.h>

#include <openssl/conf.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rsa.h>

#include <stdio.h>
#include <string.h>

#include <string>
#include <map>
#include <sstream>

#if defined(_MSC_VER)
#  if (_MSC_VER < 1800)
#    define snprintf _snprintf
#  endif
#endif

namespace {
  static const size_t IDENT_SIZE = 18;
  static const size_t KEY_SIZE   = 32;
  static const size_t BLOCK_SIZE = 16;
  static const size_t IV_SIZE    = 16;

  typedef CORBA::Octet AESKey[KEY_SIZE];

  struct SessionKey {
    std::string peer_ident;
    std::string key_ident;
    std::string key;
    omni_time_t deadline;
  };
  
  typedef std::map<std::string, RSA*>       RSAKeyMap;
  typedef std::map<std::string, SessionKey> AESKeyMap;
};


namespace omni {
  class httpCryptoManager_AES_RSA_impl;

  class httpCrypto_AES_RSA : public httpCrypto {
  public:
    
    inline httpCrypto_AES_RSA(httpCryptoManager_AES_RSA_impl* impl,
                              const std::string&              peer_ident,
                              const std::string&              self_ident,
                              RSA*                            self_rsa,
                              RSA*                            peer_rsa)
      : pd_impl(impl),
        pd_i_cipher_ctx(0),
        pd_o_cipher_ctx(0),
        pd_peer_ident(peer_ident),
        pd_self_ident(self_ident),
        pd_self_rsa(self_rsa),
        pd_peer_rsa(peer_rsa),
        pd_key_set(0)
    {
      // Client side constructor that will construct a key on first use
    }

    inline httpCrypto_AES_RSA(httpCryptoManager_AES_RSA_impl* impl,
                              const CORBA::Octet*             key,
                              const std::string&              key_ident,
                              const std::string&              peer_ident,
                              const std::string&              self_ident,
                              RSA*                            self_rsa,
                              RSA*                            peer_rsa,
                              const omni_time_t&              deadline)
      : pd_impl(impl),
        pd_i_cipher_ctx(0),
        pd_o_cipher_ctx(0),
        pd_key_ident(key_ident),
        pd_peer_ident(peer_ident),
        pd_self_ident(self_ident),
        pd_self_rsa(self_rsa),
        pd_peer_rsa(peer_rsa),
        pd_deadline(deadline),
        pd_key_set(1)
    {
      // Client side constructor given an existing key
      memcpy(pd_key, key, KEY_SIZE);
    }

    inline httpCrypto_AES_RSA(httpCryptoManager_AES_RSA_impl* impl,
                              const CORBA::Octet*             key,
                              const std::string&              key_ident,
                              const std::string&              peer_ident,
                              const omni_time_t&              deadline)
      : pd_impl(impl),
        pd_i_cipher_ctx(0),
        pd_o_cipher_ctx(0),
        pd_key_ident(key_ident),
        pd_peer_ident(peer_ident),
        pd_self_rsa(0),
        pd_peer_rsa(0),
        pd_deadline(deadline),
        pd_key_set(1)
    {
      // Server side constructor
      memcpy(pd_key, key, KEY_SIZE);
    }
    
    inline ~httpCrypto_AES_RSA()
    {
      if (pd_i_cipher_ctx)
        EVP_CIPHER_CTX_free(pd_i_cipher_ctx);

      if (pd_o_cipher_ctx)
        EVP_CIPHER_CTX_free(pd_o_cipher_ctx);
    }

    virtual const char*    peerIdent();
    
    virtual size_t         writeAuthHeader(char* buf, size_t buf_space);
    virtual CORBA::Boolean matchAuthHeader(const char* val);
    virtual size_t         encryptedSize(size_t giop_size);
    virtual size_t         encryptOverhead();

    virtual size_t encrypt(CORBA::Octet*       write_buf,
                           const CORBA::Octet* read_buf,
                           size_t              read_size,
                           CORBA::Boolean      last);

    virtual size_t decryptOverhead();
  
    virtual size_t decrypt(CORBA::Octet*       write_buf,
                           const CORBA::Octet* read_buf,
                           size_t              read_size,
                           CORBA::Boolean      last);

  private:
    httpCryptoManager_AES_RSA_impl* pd_impl;
    EVP_CIPHER_CTX*                 pd_i_cipher_ctx;
    EVP_CIPHER_CTX*                 pd_o_cipher_ctx;
    AESKey                          pd_key;
    std::string                     pd_key_ident;
    std::string                     pd_peer_ident;
    std::string                     pd_self_ident;
    RSA*                            pd_self_rsa;
    RSA*                            pd_peer_rsa;
    omni_time_t                     pd_deadline;
    CORBA::Boolean                  pd_key_set;
  };


  class httpKeyScavenger;
  
  class httpCryptoManager_AES_RSA_impl {
  public:
    httpCryptoManager_AES_RSA_impl();
    ~httpCryptoManager_AES_RSA_impl();
    
    void
    init(const char*    ident,
         const char*    private_key,
         CORBA::Boolean is_filename,
         CORBA::ULong   key_lifetime);

    void
    addClient(const char*    ident,
              const char*    public_key,
              CORBA::Boolean is_filename);

    CORBA::Boolean
    removeClient(const char* ident);

    void
    addServer(const char*    url,
              const char*    public_key,
              CORBA::Boolean is_filename);

    CORBA::Boolean
    removeServer(const char* url);

    httpCrypto*
    cryptoForServer(const char* peer_address, CORBA::Boolean new_key);

    httpCrypto*
    readAuthHeader(const char* host, const char* auth);

    void
    assignedKey(const std::string&  peer_address,
                const CORBA::Octet* key,
                const std::string&  key_ident,
                const omni_time_t&  deadline);

    void
    scavenge();
    
    inline CORBA::ULong
    key_lifetime()
    {
      return pd_key_lifetime;
    }

  private:
    CORBA::Boolean    pd_initialised;
    CORBA::ULong      pd_key_lifetime;

    std::string       pd_self_ident;
    RSA*              pd_self_rsa;

    RSAKeyMap         pd_clients;      // ident string -> public key
    RSAKeyMap         pd_servers;      // URL -> public key
    AESKeyMap         pd_session_keys; // key ident / URL -> key details

    httpKeyScavenger* pd_scavenger;
    omni_tracedmutex  pd_lock;
  };

  
  class httpKeyScavenger : public omni_thread {
  public:
    inline httpKeyScavenger(httpCryptoManager_AES_RSA_impl* impl,
                            omni_tracedmutex&               lock)
      : pd_impl(impl), pd_go(1), pd_lock(lock),
        pd_cond(&pd_lock, "httpKeyScavenger::pd_cond")
    {
      start_undetached();
    }

    void* run_undetached(void*)
    {
      omniORB::logs(25, "httpCrypto key scavenger execute.");
      
      omni_tracedmutex_lock l(pd_lock);
      
      while (pd_go) {
        pd_impl->scavenge();

        omni_time_t wakeup;
        omni_thread::get_time(wakeup, pd_impl->key_lifetime() / 10 + 1);
        pd_cond.timedwait(wakeup);
      }

      omniORB::logs(25, "httpCrypto key scavenger stop.");
      return 0;
    }

    inline void stop()
    {
      {
        omni_tracedmutex_lock l(pd_lock);
        pd_go = 0;
        pd_cond.signal();
      }
      join(0);
    }

  private:
    httpCryptoManager_AES_RSA_impl* pd_impl;
    CORBA::Boolean                  pd_go;
    omni_tracedmutex&               pd_lock;
    omni_tracedcondition            pd_cond;
  };

  
};

using namespace omni;

static int
logError(const char* msg, size_t len, void*)
{
  omniORB::logs(10, msg);
  return 0;
}


const char*
httpCrypto_AES_RSA::
peerIdent()
{
  if (!pd_peer_ident.empty())
    return pd_peer_ident.c_str();
  else
    return 0;
}


size_t
httpCrypto_AES_RSA::
writeAuthHeader(char* buf, size_t buf_space)
{
  int n;

  if (pd_key_set) {
    omni_time_t now;
    omni_thread::get_time(now);

    if (pd_deadline > now) {
      // Use existing key
      
      n = snprintf(buf, buf_space, "omni 1;0;%s", pd_key_ident.c_str());

      if (n < 0 || (size_t)n > buf_space)
        OMNIORB_THROW(MARSHAL, MARSHAL_HTTPBufferFull, CORBA::COMPLETED_NO);

      return (size_t)n;
    }
  }

  // Construct new key and send it to the server
  int             self_size = RSA_size(pd_self_rsa);
  int             peer_size = RSA_size(pd_peer_rsa);

  CORBA::OctetSeq self_seq; self_seq.length(self_size);
  CORBA::OctetSeq peer_seq; peer_seq.length(peer_size);

  CORBA::Octet    key_data[KEY_SIZE * 2 + IDENT_SIZE];
  CORBA::Octet*   self_enc = self_seq.NP_data();
  CORBA::Octet*   peer_enc = peer_seq.NP_data();
  
  if (!RAND_bytes(key_data, KEY_SIZE * 2 + IDENT_SIZE)) {
    ERR_print_errors_cb(logError, 0);
    OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);
  }

  // Encrypt first part of key data with my private key
  if (RSA_private_encrypt(KEY_SIZE, key_data,
                          self_enc, pd_self_rsa, RSA_PKCS1_PADDING) == -1) {

    ERR_print_errors_cb(logError, 0);
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEncryptedData, CORBA::COMPLETED_NO);
  }

  // Encrypt second part of key data with peer's public key
  if (RSA_public_encrypt(KEY_SIZE, key_data + KEY_SIZE,
                         peer_enc, pd_peer_rsa, RSA_PKCS1_PADDING) == -1) {

    ERR_print_errors_cb(logError, 0);
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEncryptedData, CORBA::COMPLETED_NO);
  }

  // The AES key for the session is the XOR of the first two parts
  // of key data.

  for (size_t i=0; i != KEY_SIZE; ++i)
    pd_key[i] = key_data[i] ^ key_data[KEY_SIZE + i];

  pd_key_set = 1;
    
  CORBA::String_var key_ident =
    httpContext::b64encode((const char*)key_data + KEY_SIZE * 2, IDENT_SIZE);

  CORBA::String_var self_b64 =
    httpContext::b64encode((const char*)self_enc, self_size);

  CORBA::String_var peer_b64 =
    httpContext::b64encode((const char*)peer_enc, peer_size);

  n = snprintf(buf, buf_space, "omni 1;1;%s;%s;%s;%s",
               pd_self_ident.c_str(),
               (const char*)key_ident,
               (const char*)self_b64,
               (const char*)peer_b64);

  pd_key_ident = (const char*)key_ident;
  
  if (n < 0 || (size_t)n > buf_space)
    OMNIORB_THROW(MARSHAL, MARSHAL_HTTPBufferFull, CORBA::COMPLETED_NO);

  omni_thread::get_time(pd_deadline, pd_impl->key_lifetime());
  pd_impl->assignedKey(pd_peer_ident, pd_key, pd_key_ident, pd_deadline);

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "Send new session key to " << pd_peer_ident.c_str() << "\n";
  }
  ConnectionInfo::set(ConnectionInfo::SEND_SESSION_KEY, 0,
                      pd_peer_ident.c_str());
  
  return (size_t)n;
}


CORBA::Boolean
httpCrypto_AES_RSA::
matchAuthHeader(const char* val)
{
  if (strncmp(val, "omni 1;0;", 9) ||
      strcmp(val+9, pd_key_ident.c_str()))
    return 0;

  omni_time_t now;
  omni_thread::get_time(now);

  return pd_deadline > now;
}


size_t
httpCrypto_AES_RSA::
encryptedSize(size_t giop_size)
{
  // AES pads the output to the block size. If the input is a multiple
  // of the block size, a whole extra block is output. We also send
  // the Initialisation Vector first, so that is added to the size.
  return IV_SIZE + giop_size + (BLOCK_SIZE - (giop_size % BLOCK_SIZE));
}


size_t
httpCrypto_AES_RSA::
encryptOverhead()
{
  return IV_SIZE + BLOCK_SIZE;
}


size_t
httpCrypto_AES_RSA::
encrypt(CORBA::Octet*       write_buf,
        const CORBA::Octet* read_buf,
        size_t              read_size,
        CORBA::Boolean      last)
{
  size_t written = 0;
  
  if (!pd_o_cipher_ctx) {
    OMNIORB_ASSERT(pd_key_set);

    pd_o_cipher_ctx = EVP_CIPHER_CTX_new();

    if (!pd_o_cipher_ctx) {
      ERR_print_errors_cb(logError, 0);
      OMNIORB_THROW(NO_MEMORY, NO_MEMORY_BadAlloc, CORBA::COMPLETED_NO);
    }

    // Write a random Initialisation Vector to the buffer
    if (!RAND_bytes(write_buf, IV_SIZE)) {
      ERR_print_errors_cb(logError, 0);
      OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);
    }      

    // Initialise encryption
    EVP_EncryptInit_ex(pd_o_cipher_ctx, EVP_aes_256_cbc(), 0, pd_key, write_buf);

    written   += IV_SIZE;
    write_buf += IV_SIZE;
  }

  int write_size;

  if (read_size) {
    if (!EVP_EncryptUpdate(pd_o_cipher_ctx, write_buf, &write_size,
                           read_buf, (int)read_size)) {

      ERR_print_errors_cb(logError, 0);
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEncryptedData, CORBA::COMPLETED_NO);
    }
  }
  else {
    write_size = 0;
  }    
    
  written += (size_t)write_size;

  if (last) {
    if (!EVP_EncryptFinal_ex(pd_o_cipher_ctx, write_buf + write_size,
                             &write_size)) {

      ERR_print_errors_cb(logError, 0);
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEncryptedData, CORBA::COMPLETED_NO);
    }
    
    written += (size_t)write_size;

    EVP_CIPHER_CTX_free(pd_o_cipher_ctx);
    pd_o_cipher_ctx = 0;
  }

  return written;
}


size_t
httpCrypto_AES_RSA::
decryptOverhead()
{
  return BLOCK_SIZE;
}


size_t
httpCrypto_AES_RSA::
decrypt(CORBA::Octet*       write_buf,
        const CORBA::Octet* read_buf,
        size_t              read_size,
        CORBA::Boolean      last)
{
  if (!pd_i_cipher_ctx) {
    OMNIORB_ASSERT(pd_key_set);

    pd_i_cipher_ctx = EVP_CIPHER_CTX_new();

    if (!pd_i_cipher_ctx) {
      ERR_print_errors_cb(logError, 0);
      OMNIORB_THROW(NO_MEMORY, NO_MEMORY_BadAlloc, CORBA::COMPLETED_NO);
    }

    if (read_size < IV_SIZE)
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEncryptedData, CORBA::COMPLETED_NO);

    // The message starts with the IV.
    
    if (!EVP_DecryptInit_ex(pd_i_cipher_ctx, EVP_aes_256_cbc(), 0,
                            pd_key, read_buf)) {
      ERR_print_errors_cb(logError, 0);
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEncryptedData, CORBA::COMPLETED_NO);
    }

    read_buf  += IV_SIZE;
    read_size -= IV_SIZE;
  }

  int write_size;

  if (read_size) {
    if (!EVP_DecryptUpdate(pd_i_cipher_ctx, write_buf, &write_size,
                           read_buf, read_size)) {

      ERR_print_errors_cb(logError, 0);
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEncryptedData, CORBA::COMPLETED_NO);
    }
  }
  else {
    write_size = 0;
  }
    
  size_t written = (size_t)write_size;

  if (last) {
    if (!EVP_DecryptFinal_ex(pd_i_cipher_ctx, write_buf + write_size,
                             &write_size)) {

      ERR_print_errors_cb(logError, 0);
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEncryptedData, CORBA::COMPLETED_NO);
    }
      
    written += (size_t)write_size;
    
    EVP_CIPHER_CTX_free(pd_i_cipher_ctx);
    pd_i_cipher_ctx = 0;
  }
  
  return written;
}


//
// httpCryptoManager_AES_RSA

httpCryptoManager_AES_RSA::
httpCryptoManager_AES_RSA()
{
  omniORB::logs(5, "Create AES/RSA HTTP crypto manager.");
  pd_impl = new httpCryptoManager_AES_RSA_impl();
}


httpCryptoManager_AES_RSA::
~httpCryptoManager_AES_RSA()
{
  delete pd_impl;
}


void
httpCryptoManager_AES_RSA::
init(const char*    ident,
     const char*    private_key,
     CORBA::Boolean is_filename,
     CORBA::ULong   key_lifetime)
{
  pd_impl->init(ident, private_key, is_filename, key_lifetime);
}


void
httpCryptoManager_AES_RSA::
addClient(const char*    ident,
          const char*    public_key,
          CORBA::Boolean is_filename)
{
  pd_impl->addClient(ident, public_key, is_filename);
}


CORBA::Boolean
httpCryptoManager_AES_RSA::
removeClient(const char* ident)
{
  return pd_impl->removeClient(ident);
}


void
httpCryptoManager_AES_RSA::
addServer(const char*    url,
          const char*    public_key,
          CORBA::Boolean is_filename)
{
  pd_impl->addServer(url, public_key, is_filename);
}


CORBA::Boolean
httpCryptoManager_AES_RSA::
removeServer(const char* url)
{
  return pd_impl->removeServer(url);
}


httpCrypto*
httpCryptoManager_AES_RSA::
cryptoForServer(const char* peer_address, CORBA::Boolean new_key)
{
  return pd_impl->cryptoForServer(peer_address, new_key);
}


httpCrypto*
httpCryptoManager_AES_RSA::
readAuthHeader(const char* host, const char* auth)
{
  return pd_impl->readAuthHeader(host, auth);
}


//
// httpCryptoManager_AES_RSA_impl

httpCryptoManager_AES_RSA_impl::
httpCryptoManager_AES_RSA_impl()
  : pd_initialised(0), pd_self_rsa(0), pd_scavenger(0),
    pd_lock("httpCryptoManager_AES_RSA::pd_lock")
{
}


httpCryptoManager_AES_RSA_impl::
~httpCryptoManager_AES_RSA_impl()
{
  pd_scavenger->stop();
  pd_scavenger = 0;

  RSAKeyMap::iterator it;

  for (it = pd_clients.begin(); it != pd_clients.end(); ++it)
    RSA_free(it->second);

  for (it = pd_servers.begin(); it != pd_servers.end(); ++it)
    RSA_free(it->second);

  if (pd_self_rsa) {
    RSA_free(pd_self_rsa);
    pd_self_rsa = 0;
  }
}


void
httpCryptoManager_AES_RSA_impl::
scavenge()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);

  omni_time_t now;
  omni_thread::get_time(now);
  
  AESKeyMap::iterator it, next;

  for (it = pd_session_keys.begin(); it != pd_session_keys.end(); it = next) {
    next = it; ++next;

    if (it->second.deadline < now) {
      if (omniORB::trace(30)) {
        omniORB::logger log;
        log << "Expire encryption key id " << it->second.key_ident.c_str()
            << "\n";
      }
      pd_session_keys.erase(it);
    }
  }
}


void
httpCryptoManager_AES_RSA_impl::
init(const char*    ident,
     const char*    private_key,
     CORBA::Boolean is_filename,
     CORBA::ULong   key_lifetime)
{
  omni_tracedmutex_lock l(pd_lock);

  pd_self_ident = ident;
  
  if (pd_self_rsa) {
    RSA_free(pd_self_rsa);
    pd_self_rsa = 0;
  }

  BIO* bio;
  
  if (is_filename) {
    bio = BIO_new_file(private_key, "r");
    if (!bio) {
      if (omniORB::trace(1)) {
        omniORB::logger log;
        log << "Unable to open private key file '" << private_key << "'\n";
      }
      OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);
    }
  }
  else {
    bio = BIO_new_mem_buf(private_key, -1);
  }

  pd_self_rsa = PEM_read_bio_RSAPrivateKey(bio, 0, 0, 0);
  BIO_free(bio);
  
  if (!pd_self_rsa) {
    ERR_print_errors_cb(logError, 0);
    OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);
  }

  pd_key_lifetime = key_lifetime;

  if (!pd_scavenger)
    pd_scavenger = new httpKeyScavenger(this, pd_lock);

  pd_initialised = 1;
}



static RSA*
readPublicKey(const char*    public_key,
              CORBA::Boolean is_filename)
{
  BIO* bio;
  
  if (is_filename) {
    bio = BIO_new_file(public_key, "r");
    if (!bio) {
      if (omniORB::trace(5)) {
        omniORB::logger log;
        log << "Unable to open public key file '" << public_key << "'\n";
      }
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidFile, CORBA::COMPLETED_NO);
    }
  }
  else {
    bio = BIO_new_mem_buf(public_key, -1);
  }

  RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, 0, 0, 0);
  BIO_free(bio);

  if (!rsa) {
    ERR_print_errors_cb(logError, 0);
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidKey, CORBA::COMPLETED_NO);
  }
  return rsa;
}


void
httpCryptoManager_AES_RSA_impl::
addClient(const char*    ident,
          const char*    public_key,
          CORBA::Boolean is_filename)
{
  RSA* rsa = readPublicKey(public_key, is_filename);

  omni_tracedmutex_lock l(pd_lock);

  if (!pd_initialised) {
    RSA_free(rsa);
    OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);
  }
  
  RSA*& entry = pd_clients[ident];
  if (entry)
    RSA_free(entry);

  entry = rsa;
}


CORBA::Boolean
httpCryptoManager_AES_RSA_impl::
removeClient(const char* ident)
{
  omni_tracedmutex_lock l(pd_lock);

  RSAKeyMap::iterator it = pd_clients.find(ident);

  if (it != pd_clients.end()) {
    RSA_free(it->second);
    pd_clients.erase(it);
    return 1;
  }
  return 0;
}


void
httpCryptoManager_AES_RSA_impl::
addServer(const char*    url,
          const char*    public_key,
          CORBA::Boolean is_filename)
{
  RSA* rsa = readPublicKey(public_key, is_filename);

  omni_tracedmutex_lock l(pd_lock);

  if (!pd_initialised) {
    RSA_free(rsa);
    OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError, CORBA::COMPLETED_NO);
  }
  
  RSA*& entry = pd_servers[url];
  if (entry)
    RSA_free(entry);

  entry = rsa;
}


CORBA::Boolean
httpCryptoManager_AES_RSA_impl::
removeServer(const char* url)
{
  omni_tracedmutex_lock l(pd_lock);

  RSAKeyMap::iterator it = pd_servers.find(url);

  if (it != pd_servers.end()) {
    RSA_free(it->second);
    pd_servers.erase(it);
    return 1;
  }
  return 0;
}


httpCrypto*
httpCryptoManager_AES_RSA_impl::
cryptoForServer(const char* url, CORBA::Boolean new_key)
{
  omni_tracedmutex_lock l(pd_lock);

  if (!pd_initialised)
    return 0;

  std::string url_string(url);

  RSAKeyMap::iterator rit = pd_servers.find(url_string);
  if (rit == pd_servers.end())
    return 0;

  RSA* server_rsa = rit->second;

  if (new_key) {
    pd_session_keys.erase(url_string);
  }
  else {
    AESKeyMap::iterator it = pd_session_keys.find(url_string);

    if (it != pd_session_keys.end()) {
      omni_time_t now;
      omni_thread::get_time(now);

      SessionKey& sk = it->second;
      
      if (sk.deadline > now) {
        // Use existing key
        return new httpCrypto_AES_RSA(this, (const CORBA::Octet*)sk.key.data(),
                                      sk.key_ident, url_string, pd_self_ident,
                                      pd_self_rsa, server_rsa, sk.deadline);
      }
    }
  }

  // Generate a new key in writeAuthHeader. Note that we permit
  // multiple threads to decide to do this concurrently because that
  // is more efficient and simpler than trying to synchronise them.
  
  return new httpCrypto_AES_RSA(this, url_string, pd_self_ident,
                                pd_self_rsa, server_rsa);
}


void
httpCryptoManager_AES_RSA_impl::
assignedKey(const std::string&  peer_address,
            const CORBA::Octet* key,
            const std::string&  key_ident,
            const omni_time_t&  deadline)
{
  omni_tracedmutex_lock l(pd_lock);

  SessionKey& sk = pd_session_keys[peer_address];

  sk.key.assign((const char*)key, KEY_SIZE);
  sk.peer_ident = peer_address;
  sk.key_ident  = key_ident;
  sk.deadline   = deadline;
}


httpCrypto*
httpCryptoManager_AES_RSA_impl::
readAuthHeader(const char* host, const char* auth)
{
  omni_tracedmutex_lock l(pd_lock);

  if (!strncmp(auth, "omni 1;0;", 9)) {
    // Identifier for an existing key

    std::string key_ident(auth+9);

    AESKeyMap::iterator sit = pd_session_keys.find(key_ident);
    if (sit != pd_session_keys.end()) {
      omni_time_t now;
      omni_thread::get_time(now);

      const SessionKey& sk = sit->second;
      
      if (sk.deadline > now) {
        return new httpCrypto_AES_RSA(this, (const CORBA::Octet*)sk.key.data(),
                                      sk.key_ident, sk.peer_ident, sk.deadline);
      }
    }
    // Identifier has expired, or does not match one we know
    OMNIORB_THROW(TRANSIENT, TRANSIENT_Renegotiate, CORBA::COMPLETED_NO);
  }
  if (!strncmp(auth, "omni 1;1;", 9)) {
    // New key encoded in RSA encrypted blocks. Contains client ident;
    // key ident; key part encrypted with client private key; key part
    // encrypted with my public key.

    std::string       client_ident, key_ident, client_b64, self_b64;
    std::string       data(auth+9);
    std::stringstream ss(data);
    
    std::getline(ss, client_ident, ';');
    std::getline(ss, key_ident,    ';');
    std::getline(ss, client_b64,   ';');
    std::getline(ss, self_b64);

    if (!ss || !ss.eof())
      OMNIORB_THROW(MARSHAL, MARSHAL_HTTPHeaderInvalid, CORBA::COMPLETED_NO);
    
    RSAKeyMap::iterator cit = pd_clients.find(client_ident);

    if (cit == pd_clients.end()) {
      // Unknown client identifier
      if (omniORB::trace(10)) {
        omniORB::logger log;
        log << "HTTP crypto client '" << client_ident.c_str()
            << "' is not known.\n";
      }
      ConnectionInfo::set(ConnectionInfo::CRYPTO_CLIENT_UNKNOWN, 1,
                          client_ident.c_str());

      OMNIORB_THROW(NO_PERMISSION, NO_PERMISSION_UnknownClient,
                    CORBA::COMPLETED_NO);
    }

    RSA*                client_rsa = cit->second;
    size_t              client_len, self_len;
    CORBA::String_var   client_enc_s, self_enc_s;
    const CORBA::Octet* client_enc;
    const CORBA::Octet* self_enc;
    
    client_enc_s = httpContext::b64decode(client_b64.c_str(), client_len);
    client_enc   = (const CORBA::Octet*)(const char*)client_enc_s;

    self_enc_s   = httpContext::b64decode(self_b64.c_str(),   self_len);
    self_enc     = (const CORBA::Octet*)(const char*)self_enc_s;

    CORBA::OctetSeq key_seq;
    key_seq.length(RSA_size(client_rsa) + RSA_size(pd_self_rsa));
    
    CORBA::Octet*   key_data = key_seq.NP_data();

    if (RSA_public_decrypt(client_len, client_enc, key_data,
                           client_rsa, RSA_PKCS1_PADDING) != KEY_SIZE) {
      
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEncryptedData, CORBA::COMPLETED_NO);
    }

    if (RSA_private_decrypt(self_len, self_enc, key_data + KEY_SIZE,
                            pd_self_rsa, RSA_PKCS1_PADDING) != KEY_SIZE) {
      
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEncryptedData, CORBA::COMPLETED_NO);
    }

    AESKey key;

    for (size_t i=0; i != KEY_SIZE; ++i)
      key[i] = key_data[i] ^ key_data[KEY_SIZE + i];
    
    SessionKey& sk = pd_session_keys[key_ident];

    sk.peer_ident = client_ident;
    sk.key_ident  = key_ident;
    sk.key.assign((const char*)key, KEY_SIZE);
    omni_thread::get_time(sk.deadline, (pd_key_lifetime * 3) / 2);

    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Received new session key from " << client_ident.c_str() << "\n";
    }
    ConnectionInfo::set(ConnectionInfo::RECEIVED_SESSION_KEY, 0,
                        client_ident.c_str());

    
    return new httpCrypto_AES_RSA(this, key, key_ident, client_ident,
                                  sk.deadline);
  }

  OMNIORB_THROW(MARSHAL, MARSHAL_HTTPHeaderInvalid, CORBA::COMPLETED_NO);
}
