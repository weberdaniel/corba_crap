// -*- Mode: C++; -*-
//                            Package   : omniORB
// zstdCompressor.cc          Created on: 2021/04/08
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2012-2021 Apasphere Ltd.
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
// Description:
//    zstd compressor

#include <omniORB4/CORBA.h>

#include "zstdCompressor.h"
#include <zstd.h>


OMNI_NAMESPACE_BEGIN(omni)


//
// Factory

zstdCompressorFactory::~zstdCompressorFactory() {}

Compression::Compressor_ptr
zstdCompressorFactory::
get_compressor(Compression::CompressionLevel compression_level)
{
  if (compression_level > ZSTD_maxCLevel())
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidCompressionLevel,
                  CORBA::COMPLETED_NO);

  return new zstdCompressor(this, compression_level);
}

Compression::CompressorId
zstdCompressorFactory::
compressor_id()
{
  return Compression::COMPRESSORID_OMNI_ZSTD;
}

void
zstdCompressorFactory::
_add_ref()
{
  omni_tracedmutex_lock l(pd_lock);
  ++pd_refcount;
}

void
zstdCompressorFactory::
_remove_ref()
{
  {
    omni_tracedmutex_lock l(pd_lock);
    if (--pd_refcount != 0)
      return;
  }
  delete this;
}


//
// Compressor

zstdCompressor::~zstdCompressor() {}

void
zstdCompressor::
compress(const Compression::Buffer& source, Compression::Buffer& target)
{
  // omniORB pre-populates target with a suitable buffer.
  size_t target_len = target.length();

  if (target_len == 0) {
    // In case application code calls this without pre-populating target.
    target_len = ZSTD_compressBound(source.length());
    target.length(target_len);
  }

  const void* src  = source.NP_data();
  void*       dest = target.NP_data();
  
  size_t ret = ZSTD_compress(dest, target_len, src, source.length(), pd_level);

  if (!ZSTD_isError(ret)) {
    target.length(ret);
    {
      omni_tracedmutex_lock l(pd_lock);
      pd_compressed_bytes   += ret;
      pd_uncompressed_bytes += source.length();
    }

    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Compressed zstd(" << pd_level << ") "
          << source.length() << " -> " << target.length() << "\n";
    }
  }
  else {
    throw Compression::CompressionException(ret, ZSTD_getErrorName(ret));
  }
}

void
zstdCompressor::
decompress(const Compression::Buffer& source, Compression::Buffer& target)
{
  // target length is initialised with correct uncompressed length.

  const void* src  = source.NP_data();
  void*       dest = target.NP_data();
  size_t      dlen = target.length();

  size_t ret = ZSTD_decompress(dest, dlen, src, source.length());

  if (!ZSTD_isError(ret)) {
    if (ret != target.length())
      target.length(ret);

    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Decompressed zstd "
          << source.length() << " -> " << target.length() << "\n";
    }
    return;
  }
  else {
    throw Compression::CompressionException(ret, ZSTD_getErrorName(ret));
  }
}


Compression::CompressorFactory_ptr
zstdCompressor::
compressor_factory()
{
  return Compression::CompressorFactory::_duplicate(pd_factory);
}

Compression::CompressionLevel
zstdCompressor::
compression_level()
{
  return pd_level;
}

CORBA::ULongLong
zstdCompressor::
compressed_bytes()
{
  omni_tracedmutex_lock l(pd_lock);
  return pd_compressed_bytes;
}

CORBA::ULongLong
zstdCompressor::
uncompressed_bytes()
{
  omni_tracedmutex_lock l(pd_lock);
  return pd_uncompressed_bytes;
}

Compression::CompressionRatio
zstdCompressor::
compression_ratio()
{
  omni_tracedmutex_lock l(pd_lock);
  return (((Compression::CompressionRatio)pd_compressed_bytes) /
          ((Compression::CompressionRatio)pd_uncompressed_bytes));
}

void
zstdCompressor::
_add_ref()
{
  omni_tracedmutex_lock l(pd_lock);
  ++pd_refcount;
}

void
zstdCompressor::
_remove_ref()
{
  {
    omni_tracedmutex_lock l(pd_lock);
    if (--pd_refcount != 0)
      return;
  }
  delete this;
}


OMNI_NAMESPACE_END(omni)
