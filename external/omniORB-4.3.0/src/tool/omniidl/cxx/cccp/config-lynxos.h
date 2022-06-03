#if defined(__x86__) || defined(__powerpc__)

#define OMNI_SIZEOF_UNSIGNED_CHAR 1
#define OMNI_SIZEOF_INT 4
#define OMNI_HAVE_STDLIB_H 1
#define OMNI_HAVE_STRERROR 1
#else
#error "You must set definitions for your architecture in config-lynxos.h"

#endif
