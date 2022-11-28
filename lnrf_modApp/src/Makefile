TOP=../..

include $(TOP)/configure/CONFIG
#include $(PYDEVSUP)/configure/CONFIG_PY
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
lnrf_mod_DBD += event_log.dbd


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
lnrf_mod_SRCS_DEFAULT += event_log.cpp

lnrf_mod_LIBS += $(EPICS_BASE_IOC_LIBS)
lnrf_mod_LIBS += modbus
lnrf_mod_LIBS += asyn
lnrf_mod_LIBS += devIocStats

#===========================

include $(TOP)/configure/RULES
#include $(PYDEVSUP)/configure/RULES_PY
#----------------------------------------
#  ADD RULES AFTER THIS LINE

