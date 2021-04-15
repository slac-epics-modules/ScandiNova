TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
# Create and install (or just install) into <top>/db
# databases, templates, substitutions like this
DB += $(RECCASTER)/db/reccaster.db
DB += $(devIocStats)/db/ioc_stats.db
DB += waveform.db
DB += py.db

DB_INSTALLS += ../O.$(EPICS_HOST_ARCH)/registers.db

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

../O.$(EPICS_HOST_ARCH)/registers.db: ../registers.csv ../db.py
	python3 ../db.py ../registers.csv registers.db $(TOP)/iocBoot/modbus.cmd
