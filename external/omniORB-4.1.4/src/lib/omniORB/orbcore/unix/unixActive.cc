// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixActive.cc              Created on: 6 Aug 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2005-2006 Apasphere Ltd
//    Copyright (C) 2001      AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//    02111-1307, USA
//
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//

/*
  $Log: unixActive.cc,v $
  Revision 1.1.4.4  2009/05/06 16:14:45  dgrisby
  Update lots of copyright notices.

  Revision 1.1.4.3  2006/05/02 13:07:12  dgrisby
  Idle giopMonitor SocketCollections would not exit at shutdown.

  Revision 1.1.4.2  2005/01/13 21:10:03  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.1  2003/03/23 21:01:58  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.3  2002/08/21 06:23:16  dgrisby
  Properly clean up bidir connections and ropes. Other small tweaks.

  Revision 1.1.2.2  2001/08/07 15:42:17  sll
  Make unix domain connections distinguishable on both the server and client
  side.

  Revision 1.1.2.1  2001/08/06 15:47:43  sll
  Added support to use the unix domain socket as the local transport.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <SocketCollection.h>
#include <unix/unixConnection.h>
#include <unix/unixEndpoint.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixActive);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
static unixActiveCollection myCollection;

/////////////////////////////////////////////////////////////////////////
unixActiveCollection::unixActiveCollection(): pd_n_sockets(0),pd_shutdown(0) {}

/////////////////////////////////////////////////////////////////////////
unixActiveCollection::~unixActiveCollection() {}

/////////////////////////////////////////////////////////////////////////
const char*
unixActiveCollection::type() const {
  return "giop:unix";
}

/////////////////////////////////////////////////////////////////////////
void
unixActiveCollection::Monitor(giopConnection::notifyReadable_t func,
			     void* cookie) {

  pd_callback_func = func;
  pd_callback_cookie = cookie;

  CORBA::Boolean doit;
  while (!isEmpty()) {
    if (!Select()) break;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixActiveCollection::notifyReadable(SocketHolder* conn) {

  pd_callback_func(pd_callback_cookie,(unixConnection*)conn);
  return 1;
}


/////////////////////////////////////////////////////////////////////////
void
unixActiveCollection::addMonitor(SocketHandle_t) {
  omni_tracedmutex_lock sync(pd_lock);
  pd_n_sockets++;
  pd_shutdown = 0;
}

/////////////////////////////////////////////////////////////////////////
void
unixActiveCollection::removeMonitor(SocketHandle_t) {
  omni_tracedmutex_lock sync(pd_lock);
  pd_n_sockets--;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixActiveCollection::isEmpty() const {
  omni_tracedmutex_lock sync((omni_tracedmutex&)pd_lock);
  return (pd_n_sockets == 0 || pd_shutdown);
}

/////////////////////////////////////////////////////////////////////////
void
unixActiveCollection::deactivate() {
  omni_tracedmutex_lock sync(pd_lock);
  pd_shutdown = 1;
  wakeUp();
}

/////////////////////////////////////////////////////////////////////////
unixActiveConnection::unixActiveConnection(SocketHandle_t sock,
					   const char* filename) : 
  unixConnection(sock,&myCollection,filename,1), pd_registered(0) {
}

/////////////////////////////////////////////////////////////////////////
unixActiveConnection::~unixActiveConnection() {
  if (pd_registered) {
    myCollection.removeMonitor(pd_socket);
  }
}


/////////////////////////////////////////////////////////////////////////
giopActiveCollection*
unixActiveConnection::registerMonitor() {

  if (pd_registered) return &myCollection;

  pd_registered = 1;
  myCollection.addMonitor(pd_socket);
  return &myCollection;
}

/////////////////////////////////////////////////////////////////////////
giopConnection&
unixActiveConnection::getConnection() {
  return *this;
}


OMNI_NAMESPACE_END(omni)
