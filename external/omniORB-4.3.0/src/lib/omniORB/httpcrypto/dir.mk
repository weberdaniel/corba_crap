# dir.mk for omniORB HTTP Crypto library

ORB_SRCS = httpCrypto.cc

DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(patsubst %,-I%/include/omniORB4/internal,$(IMPORT_TREES))
DIR_CPPFLAGS += -D_OMNIORB_HTTP_CRYPTO_LIBRARY
DIR_CPPFLAGS += $(OMNIORB_CPPFLAGS)
DIR_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)

##########################################################################
ifdef UnixPlatform
  DIR_CPPFLAGS += -DUnixArchitecture
  #CXXDEBUGFLAGS = -g
endif

ifdef Win32Platform
  DIR_CPPFLAGS += -D"NTArchitecture"
  EXTRA_LIBS = $(patsubst %,$(LibNoDebugSearchPattern),advapi32)
  MSVC_STATICLIB_CXXNODEBUGFLAGS += -D_WINSTATIC
  MSVC_STATICLIB_CXXDEBUGFLAGS += -D_WINSTATIC
  vpath %.cc $(VPATH):$(VPATH:%=%/../orbcore)
  SHARED_ONLY_OBJS = msvcdllstub.o
endif

#########################################################################

ORB_OBJS      = $(ORB_SRCS:.cc=.o)
CXXSRCS       = $(ORB_SRCS)

ifdef Cygwin
OPEN_SSL_LIB += -lssl.dll -lcrypto.dll
endif

LIB_NAME     := omnihttpCrypto
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(ORB_OBJS)
LIB_IMPORTS  := $(patsubst %,$(LibPathPattern),../orbcore/shared) \
                $(OMNIORB_DLL_NAME) $(OMNIORB_HTTP_LIB) \
                $(OMNITHREAD_LIB) $(EXTRA_LIBS)
LIB_SHARED_ONLY_OBJS := $(SHARED_ONLY_OBJS)

include $(BASE_OMNI_TREE)/mk/mklib.mk
