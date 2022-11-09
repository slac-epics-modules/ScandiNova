# https://epics-modbus.readthedocs.io/en/latest/
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
# INPUT REGISTERS (read by IOC app)
#

drvModbusAsynConfigure(   "read0", "mod-beckhoff-plc", 0, 4,    0,   1, 4, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure(   "read1", "mod-beckhoff-plc", 0, 4,    1,   1, 4, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure(   "read3", "mod-beckhoff-plc", 0, 4,    3,   1, 4, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure(   "read4", "mod-beckhoff-plc", 0, 4,    4,   1, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure(   "read5", "mod-beckhoff-plc", 0, 4,    5,   1, 4, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure(   "read6", "mod-beckhoff-plc", 0, 4,    6,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure(   "read8", "mod-beckhoff-plc", 0, 4,    8,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure(  "read20", "mod-beckhoff-plc", 0, 4,   20,   1, 4, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure(  "read21", "mod-beckhoff-plc", 0, 4,   21,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure( "read23", "mod-beckhoff-plc", 0, 4,   23,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure(  "read25", "mod-beckhoff-plc", 0, 4,   25,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure( "read27", "mod-beckhoff-plc", 0, 4,   27,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure( "read29", "mod-beckhoff-plc", 0, 4,   29,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure( "read31", "mod-beckhoff-plc", 0, 4,   31,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure( "read33", "mod-beckhoff-plc", 0, 4,   33,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read100", "mod-beckhoff-plc", 0, 4,  100,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read120", "mod-beckhoff-plc", 0, 4,  120,   1, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read200", "mod-beckhoff-plc", 0, 4,  200,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read202", "mod-beckhoff-plc", 0, 4,  202,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read300", "mod-beckhoff-plc", 0, 4,  300,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read302", "mod-beckhoff-plc", 0, 4,  302,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read400", "mod-beckhoff-plc", 0, 4,  400,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read402", "mod-beckhoff-plc", 0, 4,  402,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read404", "mod-beckhoff-plc", 0, 4,  404,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read406", "mod-beckhoff-plc", 0, 4,  406,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read408", "mod-beckhoff-plc", 0, 4,  408,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read410", "mod-beckhoff-plc", 0, 4,  410,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read500", "mod-beckhoff-plc", 0, 4,  500,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read502", "mod-beckhoff-plc", 0, 4,  502,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read504", "mod-beckhoff-plc", 0, 4,  504,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read600", "mod-beckhoff-plc", 0, 4,  600,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read602", "mod-beckhoff-plc", 0, 4,  602,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read700", "mod-beckhoff-plc", 0, 4,  700,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read702", "mod-beckhoff-plc", 0, 4,  702,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read704", "mod-beckhoff-plc", 0, 4,  704,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read800", "mod-beckhoff-plc", 0, 4,  800,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read820", "mod-beckhoff-plc", 0, 4,  820,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read822", "mod-beckhoff-plc", 0, 4,  822,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read824", "mod-beckhoff-plc", 0, 4,  824,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read826", "mod-beckhoff-plc", 0, 4,  826,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read900", "mod-beckhoff-plc", 0, 4,  900,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read902", "mod-beckhoff-plc", 0, 4,  902,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read904", "mod-beckhoff-plc", 0, 4,  904,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read908", "mod-beckhoff-plc", 0, 4,  908,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read910", "mod-beckhoff-plc", 0, 4,  910,   2, 7, 1000, "mod-beckhoff-plc")
#drvModbusAsynConfigure("read912", "mod-beckhoff-plc", 0, 4,  912,   2, 7, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure( "read999", "mod-beckhoff-plc", 0, 4,  999,   1, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1000", "mod-beckhoff-plc", 0, 4, 1000,  50, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1050", "mod-beckhoff-plc", 0, 4, 1050,  50, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1100", "mod-beckhoff-plc", 0, 4, 1100, 100, 5, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1200", "mod-beckhoff-plc", 0, 4, 1200, 100, 5, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1300", "mod-beckhoff-plc", 0, 4, 1300, 100, 5, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1400", "mod-beckhoff-plc", 0, 4, 1400,  50, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1450", "mod-beckhoff-plc", 0, 4, 1450,  50, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1500", "mod-beckhoff-plc", 0, 4, 1500,  50, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1550", "mod-beckhoff-plc", 0, 4, 1550, 100, 5, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1700", "mod-beckhoff-plc", 0, 4, 1700,   2, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1702", "mod-beckhoff-plc", 0, 4, 1702,   4, 5, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1706", "mod-beckhoff-plc", 0, 4, 1706,   2, 5, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1708", "mod-beckhoff-plc", 0, 4, 1708,   3, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1711", "mod-beckhoff-plc", 0, 4, 1711,   2, 5, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1715", "mod-beckhoff-plc", 0, 4, 1715,   2, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1717", "mod-beckhoff-plc", 0, 4, 1717,   4, 5, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1721", "mod-beckhoff-plc", 0, 4, 1721,   2, 5, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1723", "mod-beckhoff-plc", 0, 4, 1723,   3, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read1726", "mod-beckhoff-plc", 0, 4, 1726,   2, 5, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2000", "mod-beckhoff-plc", 0, 4, 2000, 100, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2100", "mod-beckhoff-plc", 0, 4, 2100, 100, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2200", "mod-beckhoff-plc", 0, 4, 2200,  56, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2300", "mod-beckhoff-plc", 0, 4, 2300, 100, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2400", "mod-beckhoff-plc", 0, 4, 2400, 100, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read2500", "mod-beckhoff-plc", 0, 4, 2500,  56, 0, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3000", "mod-beckhoff-plc", 0, 4, 3000,   1, 4, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3001", "mod-beckhoff-plc", 0, 4, 3001, 100, 4, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3101", "mod-beckhoff-plc", 0, 4, 3101, 100, 4, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3201", "mod-beckhoff-plc", 0, 4, 3201, 100, 4, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3301", "mod-beckhoff-plc", 0, 4, 3301, 100, 4, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3401", "mod-beckhoff-plc", 0, 4, 3401, 100, 4, 1000, "mod-beckhoff-plc")
drvModbusAsynConfigure("read3501", "mod-beckhoff-plc", 0, 4, 3501,  12, 4, 1000, "mod-beckhoff-plc")

#
# OUTPUT REGISTERS (written by IOC app)
#

# use op code 6 for 16-bit (word) values, and op code 16 for anything more than 16 bits.
# non-zero pollMsec field means to read once at init time.

drvModbusAsynConfigure(   "write0", "mod-beckhoff-plc", 0,  6,    0, 1, 0,  0, "mod-beckhoff-plc")
drvModbusAsynConfigure(   "write1", "mod-beckhoff-plc", 0,  6,    1, 1, 4, -1, "mod-beckhoff-plc")
drvModbusAsynConfigure(   "write2", "mod-beckhoff-plc", 0,  6,    2, 1, 0, -1, "mod-beckhoff-plc")
drvModbusAsynConfigure( "write100", "mod-beckhoff-plc", 0, 16,  100, 2, 7, -1, "mod-beckhoff-plc")
#drvModbusAsynConfigure("write200", "mod-beckhoff-plc", 0, 16,  200, 2, 7, -1, "mod-beckhoff-plc")
drvModbusAsynConfigure( "write300", "mod-beckhoff-plc", 0, 16,  300, 2, 7, -1, "mod-beckhoff-plc")
#drvModbusAsynConfigure("write400", "mod-beckhoff-plc", 0, 16,  400, 2, 7, -1, "mod-beckhoff-plc")
#drvModbusAsynConfigure("write500", "mod-beckhoff-plc", 0, 16,  500, 2, 7, -1, "mod-beckhoff-plc")
#drvModbusAsynConfigure("write502", "mod-beckhoff-plc", 0, 16,  502, 2, 7, -1, "mod-beckhoff-plc")
#drvModbusAsynConfigure("write504", "mod-beckhoff-plc", 0, 16,  504, 2, 7, -1, "mod-beckhoff-plc")
drvModbusAsynConfigure("write3000", "mod-beckhoff-plc", 0,  6, 3000, 1, 4, -1, "mod-beckhoff-plc")
