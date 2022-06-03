HTTP_SRCS = \
           httpActive.cc \
           httpAddress.cc \
           httpConnection.cc \
           httpContext.cc \
           httpEndpoint.cc \
           httpTransportImpl.cc

# CXXDEBUGFLAGS = -g

DIR_CPPFLAGS += -I.. $(patsubst %,-I%/..,$(VPATH))
DIR_CPPFLAGS += $(patsubst %,-I%/include/omniORB4/internal,$(IMPORT_TREES))
DIR_CPPFLAGS += -D_OMNIORB_HTTP_LIBRARY
DIR_CPPFLAGS += $(OMNIORB_CPPFLAGS)
DIR_CPPFLAGS += $(OPEN_SSL_CPPFLAGS)

#########################################################################

HTTP_OBJS     = $(HTTP_SRCS:.cc=.o)
CXXSRCS       = $(HTTP_SRCS)

ifdef Win32Platform
EXTRA_LIBS    = $(SOCKET_LIB) \
                $(patsubst %,$(LibNoDebugSearchPattern),advapi32) \
                $(patsubst %,$(LibNoDebugSearchPattern),user32) \
                $(patsubst %,$(LibNoDebugSearchPattern),gdi32)
vpath %.cc $(VPATH):$(VPATH:%=%/..)
SHARED_ONLY_OBJS = msvcdllstub.o

DIR_CPPFLAGS += -D"NTArchitecture"
MSVC_STATICLIB_CXXNODEBUGFLAGS += -D_WINSTATIC
MSVC_STATICLIB_CXXDEBUGFLAGS += -D_WINSTATIC
endif

ifdef Cygwin
OPEN_SSL_LIB += -lssl.dll -lcrypto.dll
endif

LIB_NAME     := omnihttpTP
LIB_VERSION  := $(OMNIORB_VERSION)
LIB_OBJS     := $(HTTP_OBJS)
LIB_IMPORTS  := $(patsubst %,$(LibPathPattern),../shared) \
                $(OMNIORB_DLL_NAME) $(OMNIORB_SSL_LIB) \
                $(OMNITHREAD_LIB) $(EXTRA_LIBS) $(OPEN_SSL_LIB)
LIB_SHARED_ONLY_OBJS := $(SHARED_ONLY_OBJS)


include $(BASE_OMNI_TREE)/mk/mklib.mk
