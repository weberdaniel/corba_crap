#include <omniconfig.h>

#if defined(__WIN32__)
#  include "config-windows.h"

#elif defined(OMNI_CONFIG_TRADITIONAL)

#  if defined(__VMS)
#    include "config-vms.h"

#  elif defined(__Lynx__) || defined(__lynxos__)
#    include "config-lynxos.h"

#  else
#    error "You must create a cccp config file for your platform"

#  endif

#else

/* autoconf based build */

/* On some platforms, we do not have a working alloca at all. In that
   case, we just #define alloca to be xmalloc, meaning memory
   allocated with alloca is never freed. That should not be a problem
   because cccp never allocates much memory anyway, and the OS will
   clear up when it exits.

   Note that on Linux platforms, glibc's stdlib.h includes alloca.h,
   which overrides our #define of alloca. That doesn't matter because
   gcc always correctly supports alloca.
*/

#  ifdef OMNI_DISABLE_ALLOCA
#    ifdef alloca
#      undef alloca
#    endif
#    define alloca xmalloc
#    ifndef OMNI_HAVE_ALLOCA
#      define OMNI_HAVE_ALLOCA 1  /* to prevent alloca.c trying to build one */
#    endif
#  else
#    ifdef OMNI_HAVE_ALLOCA_H
#      include <alloca.h>
#    endif
#  endif
#endif


#define BITS_PER_UNIT OMNI_SIZEOF_UNSIGNED_CHAR
#define BITS_PER_WORD OMNI_SIZEOF_INT
#define HOST_BITS_PER_INT OMNI_SIZEOF_INT
#define HOST_BITS_PER_LONG OMNI_SIZEOF_LONG

#define TARGET_BELL '\a'
#define TARGET_BS '\b'
#define TARGET_FF '\f'
#define TARGET_NEWLINE '\n'
#define TARGET_CR '\r'
#define TARGET_TAB '\t'
#define TARGET_VT '\v'

#define INCLUDE_DEFAULTS { { 0, 0, 0 } }
#define GCC_INCLUDE_DIR "/usr/include"

#define FATAL_EXIT_CODE 1
#define SUCCESS_EXIT_CODE 0
