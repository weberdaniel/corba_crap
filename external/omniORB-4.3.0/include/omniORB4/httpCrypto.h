// -*- Mode: C++; -*-
//                            Package   : omniORB
// httpCrypto.h               Created on: 20 June 2018
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
//	*** PROPRIETARY INTERFACE ***
// 

#ifndef __HTTPCRYPTO_H__
#define __HTTPCRYPTO_H__

OMNI_NAMESPACE_BEGIN(omni)

//
// Abstract classes that provide encryption support.

class httpCrypto {
public:
  virtual ~httpCrypto();

  virtual const char* peerIdent() = 0;
  // Returns an identity string for the peer, or 0 if there is no
  // identified peer. Retains ownership of the string.
  
  virtual size_t writeAuthHeader(char* buf, size_t buf_space) = 0;
  // On the client side, populate the buffer with the contents of an
  // Authorization header.  (The caller writes the terminating \r\n.)
  // If there is insufficient space, must throw MARSHAL_HTTPBufferFull.
  // Returns the number of bytes written.

  virtual CORBA::Boolean matchAuthHeader(const char* val) = 0;
  // On the server side, return true if this object matches the
  // Authorization header.

  virtual size_t encryptedSize(size_t giop_size) = 0;
  // Returns the encrypted size of the input GIOP message size.

  virtual size_t encryptOverhead() = 0;
  // Returns the maximum buffer overhead that an encrypt operation
  // might need, on top of the size of input data.
  
  virtual size_t encrypt(CORBA::Octet*       write_buf,
                         const CORBA::Octet* read_buf,
                         size_t              read_size,
                         CORBA::Boolean      last) = 0;
  // Encrypt read_size octets from read_buf and write into write_buf,
  // which is of at least size read_size plus the buffer overhead. If
  // last is true, this is the last block of data; if false, there is
  // more to come. Returns the number of octets written.

  virtual size_t decryptOverhead() = 0;
  // Returns the maximum buffer overhead that a decrypt operation
  // might need, on top of the size of input data.
  
  virtual size_t decrypt(CORBA::Octet*       write_buf,
                         const CORBA::Octet* read_buf,
                         size_t              read_size,
                         CORBA::Boolean      last) = 0;
  // Decrypt read_size octets from read_buf and write into write_buf.
  // If last is true, this is the last block of data; if false, there
  // is more to come. Returns the number of decrypted octets.
};


class httpCryptoManager {
public:
  virtual ~httpCryptoManager();

  virtual httpCrypto* cryptoForServer(const char*    url,
                                      CORBA::Boolean new_key) = 0;
  // On the client side, for server URL, return a suitable httpCrypto
  // object, or null if no message encryption is used for that
  // server. If new_key is true, force the generation of a new session
  // key.
  
  virtual httpCrypto* readAuthHeader(const char* host, const char* auth) = 0;
  // On the server side, read a Host header and Authorization header,
  // and return a suitable new httpCrypto object. If the header is not
  // understood, must throw MARSHAL_HTTPHeaderInvalid. If the header
  // refers to a previously-agreed key, but the key is not known, must
  // throw TRANSIENT_Renegotiate. If the header comes from an unknown
  // client, must throw NO_PERMISSION_UnknownClient.
};


//
// Concrete class implemented in the omnihttpCrypto library.

class httpCryptoManager_AES_RSA_impl;

class httpCryptoManager_AES_RSA : public httpCryptoManager {
public:
  httpCryptoManager_AES_RSA();
  ~httpCryptoManager_AES_RSA();

  //
  // Control interface

  void
  init(const char*    ident,
       const char*    private_key,
       CORBA::Boolean is_filename,
       CORBA::ULong   key_lifetime = 3600);
  // Initialise the crypto manager.
  //
  //  ident        -- unique string that identifies this process.
  //  private_key  -- PEM file / string for this process' private key.
  //  is_filename  -- if true, private_key is a filename for the PEM file;
  //                  if false, private_key is the actual PEM contents.
  //  key_lifetime -- the number of seconds for which an AES session key
  //                  is retained.

  void
  addClient(const char*    ident,
            const char*    public_key,
            CORBA::Boolean is_filename);
  // Add knowledge of a client, or replace the existing client's key
  // if the ident is already known.
  //
  //  ident       -- unique string that identifies the client.
  //  public_key  -- PEM file / string for the client's public key.
  //  is_filename -- if true, public_key is a filename for the PEM file;
  //                 if false, public_key is the actual PEM contents.

  CORBA::Boolean
  removeClient(const char* ident);
  // Remove the client with the specified ident. Returns true if there
  // was a client to remove; false if it was not known.

  void
  addServer(const char*    url,
            const char*    public_key,
            CORBA::Boolean is_filename);
  // Add knowledge of a server, or replace the existing server's key
  // if the URL is already known.
  //
  //  url         -- URL for the server.
  //  public_key  -- PEM file / string for the server's public key.
  //  is_filename -- if true, public_key is a filename for the PEM file;
  //                 if false, public_key is the actual PEM contents.

  CORBA::Boolean
  removeServer(const char* url);
  // Remove the server with the specified url. Returns true if there
  // was a server to remove; false if it was not known.


  //
  // Implementations of virtual functions

  virtual httpCrypto*
  cryptoForServer(const char*    peer_address,
                  CORBA::Boolean new_key);

  virtual httpCrypto*
  readAuthHeader(const char* host, const char* auth);

private:
  httpCryptoManager_AES_RSA_impl* pd_impl;
};


OMNI_NAMESPACE_END(omni)

#endif // __HTTPCRYPTO_H__
