# This file is part of ScandiNova MKS control system application. It is
# subject to the license terms in the LICENSE.txt file found in the top-level
# directory of this distribution and at
# https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. No part of
# the ScandiNova MKS control system application, including this file, may be
# copied, modified, propagated, or distributed except according to the terms
# contained in the LICENSE.txt file.

TOP = ..
include $(TOP)/configure/CONFIG
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *src*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Src*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *db*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Db*))
include $(TOP)/configure/RULES_DIRS

