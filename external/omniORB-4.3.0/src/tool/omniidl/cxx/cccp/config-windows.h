#include <io.h>
#include <malloc.h>

#pragma warning(disable:4786 4250)

#define OMNI_SIZEOF_UNSIGNED_CHAR 1
#define OMNI_SIZEOF_INT 4
#define OMNI_HAVE_STDLIB_H 1
#define OMNI_HAVE_STRERROR 1
#define DONT_DECLARE_STRERROR 1
#define OMNI_STDC_HEADERS 1

#ifdef __MINGW32__
#define OMNI_HAVE_VPRINTF
#endif

#pragma warning (disable: 4018)

char *index( char *s, int c );
char *rindex( char *s, int c );
