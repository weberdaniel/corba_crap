#############################################################################
#   Make variables for building Python modules                              #
#############################################################################

PYVERSION := $(shell $(PYTHON) -c 'import sys; sys.stdout.write(".".join(sys.version.split(".")[:2]))')
PYPREFIX  := $(shell $(PYTHON) -c 'import sys; sys.stdout.write(sys.exec_prefix.replace("\\","/"))')
PYINCDIR  := $(shell $(PYTHON) -c 'import sys, distutils.sysconfig; sys.stdout.write(distutils.sysconfig.get_python_inc().replace("\\","/"))')

PythonSHAREDLIB_SUFFIX = $(shell $(PYTHON) -c 'import sys, distutils.sysconfig; sys.stdout.write((distutils.sysconfig.get_config_var("SO") or ".so").lstrip("."))')

PY_MODULE_SUFFIX := $(shell $(PYTHON) -c 'import sys; sys.stdout.write((sys.hexversion < 0x3000000 and not hasattr(sys, "pypy_version_info")) and "module" or "")')

PYINCFILE := "<Python.h>"
PYINCTHRD := "<pythread.h>"

DIR_CPPFLAGS += -I$(PYINCDIR) -DPYTHON_INCLUDE=$(PYINCFILE) -DPYTHON_THREAD_INC=$(PYINCTHRD)
