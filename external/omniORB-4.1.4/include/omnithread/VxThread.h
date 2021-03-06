#ifndef __VXTHREAD_H__
#define __VXTHREAD_H__
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Project:     omniORB
%% Filename:    $Filename$
%% Author:      Guillaume/Bill ARRECKX
%%              Copyright Wavetek Wandel & Goltermann, Plymouth.
%% Description: OMNI thread implementation classes for VxWorks threads
%% Notes:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Log: VxThread.h,v $
%% Revision 1.1.4.5  2008/09/19 09:29:03  dgrisby
%% omni_mutex trylock method. Thanks Luke Deller and Alex Tingle.
%%
%% Revision 1.1.4.4  2006/12/11 10:39:37  dgrisby
%% Silly duplicated member in vxWorks omnithread.
%%
%% Revision 1.1.4.3  2006/10/29 15:09:08  dgrisby
%% omni_condition broken on VxWorks. Thanks Aleksander Matveyev.
%%
%% Revision 1.1.4.2  2005/01/06 23:08:27  dgrisby
%% Big merge from omni4_0_develop.
%%
%% Revision 1.1.4.1  2003/03/23 21:03:40  dgrisby
%% Start of omniORB 4.1.x development branch.
%%
%% Revision 1.1.2.1  2003/02/17 02:03:07  dgrisby
%% vxWorks port. (Thanks Michael Sturm / Acterna Eningen GmbH).
%%
%% Revision 1.1.1.1  2002/11/19 14:55:21  sokcevti
%% OmniOrb4.0.0 VxWorks port
%%
%% Revision 1.2  2002/06/14 12:45:50  engeln
%% unnecessary members in condition removed.
%% ---
%%
%% Revision 1.1.1.1  2002/04/02 10:08:49  sokcevti
%% omniORB4 initial realease
%%
%% Revision 1.1  2001/03/23 16:50:23  hartmut
%% Initial Version 2.8
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/


///////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////
#include <vxWorks.h>
#include <semLib.h>
#include <taskLib.h>


///////////////////////////////////////////////////////////////////////////
// Externs prototypes
///////////////////////////////////////////////////////////////////////////
extern "C" void omni_thread_wrapper(void* ptr);


///////////////////////////////////////////////////////////////////////////
// Exported macros
// Note: These are added as private members in each class implementation.
///////////////////////////////////////////////////////////////////////////
#define OMNI_MUTEX_IMPLEMENTATION \
   SEM_ID mutexID;	\
   bool m_bConstructed;

#define OMNI_CONDITION_IMPLEMENTATION \
   SEM_ID sema_;

#define OMNI_SEMAPHORE_IMPLEMENTATION \
   SEM_ID semID;

#define OMNI_MUTEX_LOCK_IMPLEMENTATION                  \
	if(semTake(mutexID, WAIT_FOREVER) != OK)	\
	{	\
		throw omni_thread_fatal(errno);	\
	}

#define OMNI_MUTEX_TRYLOCK_IMPLEMENTATION               \
	return semTake(mutexID, NO_WAIT) == OK;

#define OMNI_MUTEX_UNLOCK_IMPLEMENTATION                \
	if(semGive(mutexID) != OK)	\
	{	\
		throw omni_thread_fatal(errno);	\
	}

#define OMNI_THREAD_IMPLEMENTATION \
   friend void omni_thread_wrapper(void* ptr); \
   static int vxworks_priority(priority_t); \
   omni_condition *running_cond; \
   void* return_val; \
   int tid; \
   public: \
   static void attach(void); \
   static void detach(void); \
   static void show(void);


///////////////////////////////////////////////////////////////////////////
// Porting macros
///////////////////////////////////////////////////////////////////////////
// This is a wrapper function for the 'main' function which does not exists
//  as such in VxWorks. The wrapper creates a launch function instead,
//  which spawns the application wrapped in a omni_thread.
// Argc will always be null.
///////////////////////////////////////////////////////////////////////////
#define main( discarded_argc, discarded_argv ) \
        omni_discard_retval() \
          { \
          throw; \
          } \
        int omni_main( int argc, char **argv ); \
        void launch( ) \
          { \
          omni_thread* th = new omni_thread( (void(*)(void*))omni_main );\
          th->start();\
          }\
        int omni_main( int argc, char **argv )


#endif // ndef __VXTHREAD_H__
