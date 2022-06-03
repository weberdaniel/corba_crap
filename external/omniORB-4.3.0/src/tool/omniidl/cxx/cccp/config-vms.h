#ifdef __VMS

#define OMNI_SIZEOF_UNSIGNED_CHAR 1
#define OMNI_SIZEOF_INT 4
#ifndef VAXC
/* assume decc */
#define OMNI_HAVE_STDLIB_H 1
#define OMNI_HAVE_STRERROR 1
#if defined(__ALPHA) || defined(__VAX) && defined(__DECC) && __DECC_VER > 60000000
#include <builtins.h>
#define alloca __ALLOCA
#define OMNI_HAVE_ALLOCA 1
#endif

#define OMNI_HAVE_FCNTL_H 1
#define OMNI_HAVE_STDLIB_H 1
#define OMNI_HAVE_SYS_TIME_H 1
#define OMNI_HAVE_UNISTD_H 1
#define OMNI_STDC_HEADERS 1
/* but not:								    */
/*									    */
/* #define OMNI_TIME_WITH_SYS_TIME					    */
/*									    */
/* (actually, most VMS compilers treat					    */
/*									    */
/*  #include <sys/time.h>						    */
/*									    */
/* the same as								    */
/*									    */
/*  #include <time.h>							    */
/*									    */
/* but why bother. */

#endif /* VAXC not defined						    */

#endif
