# This file is part of ScandiNova MKS control system application. It is
# subject to the license terms in the LICENSE.txt file found in the top-level
# directory of this distribution and at
# https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. No part of
# the ScandiNova MKS control system application, including this file, may be
# copied, modified, propagated, or distributed except according to the terms
# contained in the LICENSE.txt file.

TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = lnrf_mod
# lnrf_mod.dbd will be created and installed
DBD += lnrf_mod.dbd

# lnrf_mod.dbd will be made up from these files:
lnrf_mod_DBD += base.dbd

# Include dbd files from all support applications:
lnrf_mod_DBD += modbusSupport.dbd
lnrf_mod_DBD += drvAsynIPPort.dbd
lnrf_mod_DBD += asyn.dbd
lnrf_mod_DBD += iocAdmin.dbd
lnrf_mod_DBD += lnrf_mod_exports.dbd

USR_INCLUDES += -I$(ZLIB_INCLUDE)
USR_INCLUDES += -I$(XZ_INCLUDE)
USR_INCLUDES += -I$(LIBXML2_INCLUDE)

z_DIR             = $(ZLIB_LIB)
lzma_DIR          = $(XZ_LIB)
xml2_DIR          = $(LIBXML2_LIB)

USR_LIBS += xml2
USR_LIBS += z
USR_LIBS += lzma

# lnrf_mod_registerRecordDeviceDriver.cpp derives from lnrf_mod.dbd
lnrf_mod_SRCS += lnrf_mod_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
lnrf_mod_SRCS_DEFAULT += lnrf_modMain.cpp
lnrf_mod_SRCS_DEFAULT += lnrf_mod.cpp

lnrf_mod_LIBS += modbus
lnrf_mod_LIBS += asyn
lnrf_mod_LIBS += devIocStats
lnrf_mod_LIBS += autosave
lnrf_mod_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

