TOP=../..

include $(TOP)/configure/CONFIG
include $(PYDEVSUP)/configure/CONFIG_PY
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
lnrf_mod_DBD += pyDevSup.dbd
lnrf_mod_DBD += reccaster.dbd
lnrf_mod_DBD += devIocStats.dbd
lnrf_mod_DBD += retools.dbd

# Add all the support libraries needed by this IOC
lnrf_mod_LIBS += modbus 
lnrf_mod_LIBS += asyn 
lnrf_mod_LIBS += pyDevSup$(PY_LD_VER) 
lnrf_mod_LIBS += reccaster 
lnrf_mod_LIBS += devIocStats 
lnrf_mod_LIBS += retools 

# lnrf_mod_registerRecordDeviceDriver.cpp derives from lnrf_mod.dbd
lnrf_mod_SRCS += lnrf_mod_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
lnrf_mod_SRCS_DEFAULT += lnrf_modMain.cpp
lnrf_mod_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#lnrf_mod_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
lnrf_mod_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
include $(PYDEVSUP)/configure/RULES_PY
#----------------------------------------
#  ADD RULES AFTER THIS LINE

