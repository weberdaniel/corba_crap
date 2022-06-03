# This file selects the platform you are building for.

# If you have a Unix-like platform, you should use the configure
# script than using this configuration mechanism.

# Uncomment one of the following platform lines to build for the
# target platform
#
#  x86_win32_vs_16           x86 Windows, MS VC++ 16.x (2019)
#  x86_win32_vs_15           x86 Windows, MS VC++ 15.x (2017)
#  x86_win32_vs_14           x86 Windows, MS VC++ 14.x (2015)
#  x86_win32_vs_12           x86 Windows, MS VC++ 12.x (2013)
#  x86_win32_vs_11           x86 Windows, MS VC++ 11.x (2012)
#  x86_win32_vs_10           x86 Windows, MS VC++ 10.x (2010)
#  x86_win32_mingw           x86 Windows, mingw/g++ build
#  x86_win32_dmc             x86 Win32, Digital Mars C++ (>= 8.32.14)
#  x86_ets                   Phar Lap Realtime ETS-kernel
#  x86_LynxOS_4.0            x86, LynxOS 4.0, gcc 2.95.3
#  pc486_rtems_4.5.0         x86, RTEMS, gcc 2.95.2

# You should also look at <top>/mk/platforms/$(platform).mk and if necessary
# edit the make variables, such as CC and CXX, in the file.

#platform = x86_win32_vs_16
#platform = x86_win32_vs_15
#platform = x86_win32_vs_14
#platform = x86_win32_vs_12
#platform = x86_win32_vs_11
#platform = x86_win32_vs_10
#platform = x86_win32_mingw
#platform = x86_win32_dmc
#platform = powerpc_LynxOS_4.0
#platform = x86_ets
#platform = x86_LynxOS_4.0
#platform = pc486_rtems_4.5.0


# This setting is used on Windows platforms to build debug versions of
# the omniORB executables. It is not required if you want to debug
# your own code, or for building debug versions of the omniORB
# libraries. Setting this variable causes the build to break in subtle
# ways. DO NOT SET THIS UNLESS YOU KNOW WHAT YOU ARE DOING.
#
#BuildDebugBinary = 1
#

EXPORT_TREE =  $(TOP)

IMPORT_TREES = $(TOP)

override VPATH := .

THIS_IMPORT_TREE := $(TOP)
ifneq ($(wildcard $(THIS_IMPORT_TREE)/mk/beforedir.mk),)
include $(THIS_IMPORT_TREE)/mk/beforedir.mk
endif

include dir.mk

THIS_IMPORT_TREE := $(TOP)
ifneq ($(wildcard $(THIS_IMPORT_TREE)/mk/afterdir.mk),)
include $(THIS_IMPORT_TREE)/mk/afterdir.mk
endif

