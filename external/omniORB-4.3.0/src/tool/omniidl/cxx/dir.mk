include $(BASE_OMNI_TREE)/mk/python.mk

IDLMODULE_MAJOR   = $(OMNIORB_MAJOR_VERSION)
IDLMODULE_MINOR   = $(OMNIORB_MINOR_VERSION)
IDLMODULE_VERSION = 0x2630# => CORBA 2.6, front-end 3.0

DIR_CPPFLAGS += -DIDLMODULE_VERSION="\"$(IDLMODULE_VERSION)\""


ifndef PYTHON
all::
	@$(NoPythonError)
export::
	@$(NoPythonError)
endif


SUBDIRS = cccp

all::
	@$(MakeSubdirs)
export::
	@$(MakeSubdirs)

ifdef INSTALLTARGET
install::
	@$(MakeSubdirs)
endif

OBJS  = y.tab.o lex.yy.o idlerr.o idlutil.o idltype.o \
	idlrepoId.o idlscope.o idlexpr.o idlast.o idlvalidate.o \
	idldump.o idlconfig.o idlfixed.o

PYOBJS = idlpython.o

CXXSRCS = y.tab.cc lex.yy.cc idlerr.cc idlutil.cc idltype.cc \
	idlrepoId.cc idlscope.cc idlexpr.cc idlast.cc idlvalidate.cc \
	idldump.cc idlconfig.cc idlfixed.cc idlpython.cc idlc.cc

YYSRC = idl.yy
LLSRC = idl.ll

FLEX = flex -t
BISON = bison -d -o y.tab.c

idlc = $(patsubst %,$(BinPattern),idlc)

# y.tab.h y.tab.cc: $(YYSRC)
# 	@-$(RM) $@
# 	$(BISON) $<
# 	mv -f y.tab.c y.tab.cc

# lex.yy.cc: $(LLSRC) y.tab.h
# 	$(FLEX) $< | sed -e 's/^#include <unistd.h>//' -e 's/<stdout>/lex.yy.cc/' > $@
# 	echo '#ifdef __VMS' >> $@
# 	echo '// Some versions of DEC C++ for OpenVMS set the module name used by the' >> $@
# 	echo '// librarian based on the last #line encountered.' >> $@
# 	echo '#line' `cat $@ | wc -l` '"lex_yy.cc"' >> $@
# 	echo '#endif' >> $@

#############################################################################
#   Test executable                                                         #
#############################################################################

# all:: $(idlc)

# $(idlc): $(OBJS) idlc.o
# 	@(libs=""; $(CXXExecutable))



#############################################################################
#   Make variables for Unix platforms                                       #
#############################################################################

ifdef UnixPlatform
#CXXDEBUGFLAGS = -g
endif

ifeq ($(platform),autoconf)

namespec := _omniidl$(PY_MODULE_SUFFIX) _ $(IDLMODULE_MAJOR) $(IDLMODULE_MINOR)

ifdef PythonSHAREDLIB_SUFFIX
SHAREDLIB_SUFFIX = $(PythonSHAREDLIB_SUFFIX)
endif

SharedLibraryFullNameTemplate = $$1$$2.$(SHAREDLIB_SUFFIX).$$3.$$4
SharedLibrarySoNameTemplate   = $$1$$2.$(SHAREDLIB_SUFFIX).$$3
SharedLibraryLibNameTemplate  = $$1$$2.$(SHAREDLIB_SUFFIX)

ifdef PythonLibraryPlatformLinkFlagsTemplate
SharedLibraryPlatformLinkFlagsTemplate = $(PythonLibraryPlatformLinkFlagsTemplate)
endif

shlib := $(shell $(SharedLibraryFullName) $(namespec))

DIR_CPPFLAGS += $(SHAREDLIB_CPPFLAGS)

#### ugly AIX section start
ifdef AIX

DIR_CPPFLAGS += -I. -I/usr/local/include -DNO_STRCASECMP

libinit = init_omniidl
py_exp = $(PYPREFIX)/lib/python$(PYVERSION)/config/python.exp
ld_so_aix = $(PYPREFIX)/lib/python$(PYVERSION)/config/ld_so_aix

ifdef Compiler_GCC
$(shlib): $(OBJS) $(PYOBJS)
	@(set -x; \
	$(RM) $@; \
	$(ld_so_aix) $(CXX) \
		-o $(shlib) \
		-e $(libinit) \
		-bI:$(py_exp) \
		-Wl,-blibpath:/lib:/usr/lib:$(prefix)/lib \
		$(IMPORT_LIBRARY_FLAGS) \
		$(filter-out $(LibSuffixPattern),$^); \
	)
else


# Previously, xlc builds used this, but modern xlc works correctly
# with the standard rule.

# CXXLINK = makeC++SharedLib_r
#
# $(shlib): $(OBJS) $(PYOBJS)
# 	@(set -x; \
# 	$(RM) $@; \
# 	$(CXXLINK) \
# 		-n $(libinit) \
# 		-o $(shlib) \
# 		-bI:$(py_exp) \
# 		$(IMPORT_LIBRARY_FLAGS) \
# 		-bhalt:4 -T512 -H512 \
# 		$(filter-out $(LibSuffixPattern),$^) \
# 		-p 40 \
# 		; \
# 	)
$(shlib): $(OBJS) $(PYOBJS)
	@(namespec="$(namespec)"; extralibs="$(extralibs)"; $(MakeCXXSharedLibrary))

endif

else
#### ugly AIX section end, normal build command

$(shlib): $(OBJS) $(PYOBJS)
	@(namespec="$(namespec)"; extralibs="$(extralibs)"; $(MakeCXXSharedLibrary))
endif

all:: $(shlib)

export:: $(shlib)
	@(namespec="$(namespec)"; $(ExportSharedLibrary))

ifdef INSTALLTARGET

install:: $(shlib)
	@(dir="$(INSTALLPYEXECDIR)"; namespec="$(namespec)"; \
          $(ExportSharedLibraryToDir))
endif

clean::
	$(RM) *.o
	(dir=.; $(CleanSharedLibrary))

veryclean::
	$(RM) *.o
	(dir=.; $(CleanSharedLibrary))

ifdef Cygwin

SharedLibraryPlatformLinkFlagsTemplate = -shared -Wl,-soname=$$soname,--export-dynamic,--enable-auto-import

extralibs += -L$(PYPREFIX)/lib/python$(PYVERSION)/config \
 -lpython$(PYVERSION).dll

endif

else

#############################################################################
#   Make rules for Windows                                                  #
#############################################################################

ifdef Win32Platform

DIR_CPPFLAGS += -DMSDOS -DOMNIIDL_EXECUTABLE

PYLIBDIR := $(PYPREFIX)/libs $(PYPREFIX)/lib/x86_win32

ifdef MinGW32Build
PYLIB     := -lpython$(subst .,,$(PYVERSION))
CXXLINKOPTIONS += $(patsubst %,-L%,$(PYLIBDIR))
else
PYLIB     := python$(subst .,,$(PYVERSION)).lib
CXXLINKOPTIONS += $(patsubst %,-libpath:%,$(PYLIBDIR))
endif

omniidl = $(patsubst %,$(BinPattern),omniidl)

all:: $(omniidl)

export:: $(omniidl)
	@$(ExportExecutable)

clean::
	$(RM) $(omniidl)

$(omniidl): $(OBJS) $(PYOBJS)
	@(libs="$(PYLIB)"; $(CXXExecutable))

endif

endif
