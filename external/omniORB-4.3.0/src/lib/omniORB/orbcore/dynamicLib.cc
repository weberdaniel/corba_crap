// -*- Mode: C++; -*-
//                            Package   : omniORB
// dynamicLib.cc              Created on: 15/9/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2003-2004 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//    Stubs for dynamic library 'hook' functions.
//

#include <omniORB4/CORBA.h>
#include <dynamicLib.h>
#include <exceptiondefs.h>
#include <initialiser.h>
#include <orbOptions.h>
#include <orbParameters.h>

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////

OMNI_NAMESPACE_BEGIN(omni)

CORBA::Boolean  orbParameters::tcAliasExpand          = 0;
// This flag is used to indicate whether TypeCodes associated with anys
// should have aliases removed. This functionality is included because
// some ORBs will not recognise an Any containing a TypeCode containing
// aliases to be the same as the actual type contained in the Any. Note
// that omniORB will always remove top-level aliases, but will not remove
// aliases from TypeCodes that are members of other TypeCodes (e.g.
// TypeCodes for members of structs etc.), unless tcAliasExpand is set to 1.
// There is a performance penalty when inserting into an Any if 
// tcAliasExpand is set to 1. The default value is 0 (i.e. aliases of
// member TypeCodes are not expanded). Note that aliases won't be expanded
// when one of the non-type-safe methods of inserting into an Any is
// used (i.e. when the replace() member function or non - type-safe Any
// constructor is used. )
//
//  Valid values = 0 or 1

CORBA::Boolean  orbParameters::diiThrowsSysExceptions = 0;
// If the value of this variable is 1 then the Dynamic Invacation Interface
// functions (Request::invoke, send_oneway, send_deferred, get_response,
// poll_response) will throw system exceptions as appropriate. Otherwise 
// the exception will be stored in the Environment pseudo object associated
// with the Request. By default system exceptions are passed through the 
// Environment object.
//
// Valid values = 0 or 1


CORBA::Boolean  orbParameters::useTypeCodeIndirections = 1;
// If true (the default), typecode indirectional will be used. Set
// this to false to disable that. Setting this to false might be
// useful to interoperate with another ORB implementation that cannot
// handle indirectional properly.
//
// Valid values = 0 or 1

CORBA::Boolean  orbParameters::acceptMisalignedTcIndirections = 0;
// If true, try to fix a mis-aligned indirection in a typecode. This
// could be used to work around some versions of Visibroker's Java ORB.
//
// Valid values = 0 or 1

CORBA::Boolean  orbParameters::exceptionIdInAny = 1;
// When an exception is transmitted inside an Any, should the
// repository id be transmitted at the start of the value?  The id
// is in the TypeCode, so the id is redundant inside the value, and
// the CORBA specification is not especially clear. The common
// interpretation of the CORBA specification is that the repository
// id should always be sent at the start of the marshalled exception
// value, even when inside an Any.
//
// Versions of omniORB for C++ prior to 4.3 did not send the
// repository id when transmitting an exception inside an Any. Set
// this to false for compatibility with previous omniORB versions.
//
// Valid values = 0 or 1


////////////////////////////////////////////////////////////////////////////
static void init();
static void deinit();
static void lookup_id_lcfn(omniCallDescriptor* cd, omniServant* svnt);


static omniDynamicLib orbcore_ops = {
  init,
  deinit,
  lookup_id_lcfn
};

omniDynamicLib* omniDynamicLib::ops = &orbcore_ops;
omniDynamicLib* omniDynamicLib::hook = 0;


static void
init()
{
  omniORB::logs(2, "Information: the omniDynamic library is not linked.");
}


static void
deinit()
{
}

static void
lookup_id_lcfn(omniCallDescriptor* cd, omniServant* svnt)
{
  // Can't have a local call to a repository if the dynamic
  // library is not linked ...
  OMNIORB_ASSERT(0);
}

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class tcAliasExpandHandler : public orbOptions::Handler {
public:

  tcAliasExpandHandler() : 
    orbOptions::Handler("tcAliasExpand",
			"tcAliasExpand = 0 or 1",
			1,
			"-ORBtcAliasExpand < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::tcAliasExpand = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::tcAliasExpand,
			     result);
  }
};

static tcAliasExpandHandler tcAliasExpandHandler_;

/////////////////////////////////////////////////////////////////////////////
class diiThrowsSysExceptionsHandler : public orbOptions::Handler {
public:

  diiThrowsSysExceptionsHandler() : 
    orbOptions::Handler("diiThrowsSysExceptions",
			"diiThrowsSysExceptions = 0 or 1",
			1,
			"-ORBdiiThrowsSysExceptions < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::diiThrowsSysExceptions = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::diiThrowsSysExceptions,
			     result);
  }
};

static diiThrowsSysExceptionsHandler diiThrowsSysExceptionsHandler_;

/////////////////////////////////////////////////////////////////////////////
class useTypeCodeIndirectionsHandler : public orbOptions::Handler {
public:

  useTypeCodeIndirectionsHandler() : 
    orbOptions::Handler("useTypeCodeIndirections",
			"useTypeCodeIndirections = 0 or 1",
			1,
			"-ORBuseTypeCodeIndirections < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::useTypeCodeIndirections = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::useTypeCodeIndirections,
			     result);
  }
};

static useTypeCodeIndirectionsHandler useTypeCodeIndirectionsHandler_;

/////////////////////////////////////////////////////////////////////////////
class acceptMisalignedTcIndirectionsHandler : public orbOptions::Handler {
public:

  acceptMisalignedTcIndirectionsHandler() : 
    orbOptions::Handler("acceptMisalignedTcIndirections",
			"acceptMisalignedTcIndirections = 0 or 1",
			1,
			"-ORBacceptMisalignedTcIndirections < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::acceptMisalignedTcIndirections = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::acceptMisalignedTcIndirections,
			     result);
  }
};

static acceptMisalignedTcIndirectionsHandler acceptMisalignedTcIndirectionsHandler_;

/////////////////////////////////////////////////////////////////////////////
class exceptionIdInAnyHandler : public orbOptions::Handler {
public:

  exceptionIdInAnyHandler() : 
    orbOptions::Handler("exceptionIdInAny",
			"exceptionIdInAny = 0 or 1",
			1,
			"-ORBexceptionIdInAny < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::exceptionIdInAny = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::exceptionIdInAny,
			     result);
  }
};

static exceptionIdInAnyHandler exceptionIdInAnyHandler_;

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_dynamiclib_initialiser : public omniInitialiser {
public:

  omni_dynamiclib_initialiser() {
    orbOptions::singleton().registerHandler(tcAliasExpandHandler_);
    orbOptions::singleton().registerHandler(diiThrowsSysExceptionsHandler_);
    orbOptions::singleton().registerHandler(useTypeCodeIndirectionsHandler_);
    orbOptions::singleton().registerHandler(acceptMisalignedTcIndirectionsHandler_);
    orbOptions::singleton().registerHandler(exceptionIdInAnyHandler_);
  }

  void attach() {
    if( omniDynamicLib::hook )
      omniDynamicLib::ops = omniDynamicLib::hook;
    omniDynamicLib::ops->init();
  }
  void detach() {
    omniDynamicLib::ops->deinit();
  }
};


static omni_dynamiclib_initialiser initialiser;

omniInitialiser& omni_dynamiclib_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
