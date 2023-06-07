// This file is part of ScandiNova MKS control system application. It is
// subject to the license terms in the LICENSE.txt file found in the top-level
// directory of this distribution and at
// https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. No part of
// the ScandiNova MKS control system application, including this file, may be
// copied, modified, propagated, or distributed except according to the terms
// contained in the LICENSE.txt file.

#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "epicsExit.h"
#include "epicsThread.h"
#include "iocsh.h"

int main(int argc,char *argv[])
{
    if(argc>=2) {
        iocsh(argv[1]);
        epicsThreadSleep(.2);
    }
    iocsh(NULL);
    epicsExit(0);
    return(0);
}
