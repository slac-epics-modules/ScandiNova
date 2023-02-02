# https://epics-modules.github.io/modbus/
#
# drvModbusAsynConfigure(portName,
#                        tcpPortName,
#                        slaveAddress,
#                        modbusFunction,
#                        modbusStartAddress,
#                        modbusLength,
#                        dataType,
#                        pollMsec,
#                        plcType);
#

#
# if you would like to observe the raw data being transferred via modbus,
# issue these two commands:
#
# epics> asynSetTraceIOMask <portName> 0 ASYN_TRACEIO_HEX
# epics> asynSetTraceMask <portName> 0 ASYN_TRACEIO_DRIVER|ASYN_TRACEIO_DEVICE
#

# these register definitions come from "ScandiCAT ModbusTCP specification" rev 00, DOC-026115

#
# INPUT REGISTERS (read by IOC app)
#

drvModbusAsynConfigure("read0",    "mod-beckhoff-plc", 0, 4,    0,   1, INT16,       1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1",    "mod-beckhoff-plc", 0, 4,    1,   1, INT16,       1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2",    "mod-beckhoff-plc", 0, 4,    2,   1, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3",    "mod-beckhoff-plc", 0, 4,    3,   1, INT16,       1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read4",    "mod-beckhoff-plc", 0, 4,    4,   1, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read5",    "mod-beckhoff-plc", 0, 4,    5,   1, INT16,       1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read6",    "mod-beckhoff-plc", 0, 4,    6,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read8",    "mod-beckhoff-plc", 0, 4,    8,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read10",   "mod-beckhoff-plc", 0, 4,   10,   1, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read11",   "mod-beckhoff-plc", 0, 4,   11,   1, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read12",   "mod-beckhoff-plc", 0, 4,   12,   1, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read20",   "mod-beckhoff-plc", 0, 4,   20,   1, INT16,       1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read21",   "mod-beckhoff-plc", 0, 4,   21,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read25",   "mod-beckhoff-plc", 0, 4,   25,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read80",   "mod-beckhoff-plc", 0, 4,   80,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read82",   "mod-beckhoff-plc", 0, 4,   82,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read84",   "mod-beckhoff-plc", 0, 4,   84,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read86",   "mod-beckhoff-plc", 0, 4,   86,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read88",   "mod-beckhoff-plc", 0, 4,   88,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read90",   "mod-beckhoff-plc", 0, 4,   90,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read92",   "mod-beckhoff-plc", 0, 4,   92,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read94",   "mod-beckhoff-plc", 0, 4,   94,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read100",  "mod-beckhoff-plc", 0, 4,  100,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read102",  "mod-beckhoff-plc", 0, 4,  102,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read200",  "mod-beckhoff-plc", 0, 4,  200,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read202",  "mod-beckhoff-plc", 0, 4,  202,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read300",  "mod-beckhoff-plc", 0, 4,  300,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read302",  "mod-beckhoff-plc", 0, 4,  302,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read304",  "mod-beckhoff-plc", 0, 4,  304,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read400",  "mod-beckhoff-plc", 0, 4,  400,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read402",  "mod-beckhoff-plc", 0, 4,  402,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read500",  "mod-beckhoff-plc", 0, 4,  500,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read502",  "mod-beckhoff-plc", 0, 4,  502,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read504",  "mod-beckhoff-plc", 0, 4,  504,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read506",  "mod-beckhoff-plc", 0, 4,  506,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read600",  "mod-beckhoff-plc", 0, 4,  600,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read602",  "mod-beckhoff-plc", 0, 4,  602,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read700",  "mod-beckhoff-plc", 0, 4,  700,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read702",  "mod-beckhoff-plc", 0, 4,  702,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read704",  "mod-beckhoff-plc", 0, 4,  704,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read706",  "mod-beckhoff-plc", 0, 4,  706,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read800",  "mod-beckhoff-plc", 0, 4,  800,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read802",  "mod-beckhoff-plc", 0, 4,  802,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read804",  "mod-beckhoff-plc", 0, 4,  804,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read806",  "mod-beckhoff-plc", 0, 4,  806,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read808",  "mod-beckhoff-plc", 0, 4,  808,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read810",  "mod-beckhoff-plc", 0, 4,  810,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read852",  "mod-beckhoff-plc", 0, 4,  852,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read920",  "mod-beckhoff-plc", 0, 4,  920,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read922",  "mod-beckhoff-plc", 0, 4,  922,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read924",  "mod-beckhoff-plc", 0, 4,  924,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read926",  "mod-beckhoff-plc", 0, 4,  926,   2, INT32_LE_BS, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read999",  "mod-beckhoff-plc", 0, 4,  999,   1, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1000", "mod-beckhoff-plc", 0, 4, 1000,  50, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1050", "mod-beckhoff-plc", 0, 4, 1050,  50, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1100", "mod-beckhoff-plc", 0, 4, 1100, 100, UINT32_LE,   1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1200", "mod-beckhoff-plc", 0, 4, 1200, 100, UINT32_LE,   1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1300", "mod-beckhoff-plc", 0, 4, 1300, 100, UINT32_LE,   1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1400", "mod-beckhoff-plc", 0, 4, 1400,  50, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1450", "mod-beckhoff-plc", 0, 4, 1450,  50, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1500", "mod-beckhoff-plc", 0, 4, 1500,  50, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1550", "mod-beckhoff-plc", 0, 4, 1550, 100, UINT32_LE,   1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1700", "mod-beckhoff-plc", 0, 4, 1700,  12, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1715", "mod-beckhoff-plc", 0, 4, 1715,  12, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2000", "mod-beckhoff-plc", 0, 4, 2000, 100, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2100", "mod-beckhoff-plc", 0, 4, 2100, 100, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2200", "mod-beckhoff-plc", 0, 4, 2200,  56, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2300", "mod-beckhoff-plc", 0, 4, 2300, 100, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2400", "mod-beckhoff-plc", 0, 4, 2400, 100, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2500", "mod-beckhoff-plc", 0, 4, 2500,  56, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3000", "mod-beckhoff-plc", 0, 4, 3000,   1, INT16,       1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3001", "mod-beckhoff-plc", 0, 4, 3001,   4, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3005", "mod-beckhoff-plc", 0, 4, 3005,   4, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3009", "mod-beckhoff-plc", 0, 4, 3009,   1, UINT16,      1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3010", "mod-beckhoff-plc", 0, 4, 3010, 100, INT16,       1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3110", "mod-beckhoff-plc", 0, 4, 3110, 100, INT16,       1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3210", "mod-beckhoff-plc", 0, 4, 3210, 100, INT16,       1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3310", "mod-beckhoff-plc", 0, 4, 3310, 100, INT16,       1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3410", "mod-beckhoff-plc", 0, 4, 3410, 100, INT16,       1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3510", "mod-beckhoff-plc", 0, 4, 3510,   2, INT16,       1000, "mod-beckhoff-plc")

#
# OUTPUT REGISTERS (written by IOC app)
#

# use op code 6 for 16-bit (word) values, and op code 16 for anything more than 16 bits.
# non-zero pollMsec field means to read once at init time.

drvModbusAsynConfigure(   "write0", "mod-beckhoff-plc", 0,  6,    0, 1, UINT16,       0, "mod-beckhoff-plc")
drvModbusAsynConfigure(   "write1", "mod-beckhoff-plc", 0,  6,    1, 1, INT16,       -1, "mod-beckhoff-plc")
drvModbusAsynConfigure(   "write2", "mod-beckhoff-plc", 0,  6,    2, 1, UINT16,      -1, "mod-beckhoff-plc")
drvModbusAsynConfigure( "write100", "mod-beckhoff-plc", 0, 16,  100, 2, INT32_LE_BS, -1, "mod-beckhoff-plc")
drvModbusAsynConfigure( "write300", "mod-beckhoff-plc", 0, 16,  300, 2, INT32_LE_BS, -1, "mod-beckhoff-plc")
drvModbusAsynConfigure("write3000", "mod-beckhoff-plc", 0,  6, 3000, 1, INT16,       -1, "mod-beckhoff-plc")
