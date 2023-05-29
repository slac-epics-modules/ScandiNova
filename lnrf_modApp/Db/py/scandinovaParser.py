import csv
from epicsDB import Record, Info

# A whole bunch of crud because Scandinova xml is inconsistent and weird 
# Most functions are specific to current xml implemenation/modbus map
# Note modbus-documenation.pdf in $(TOP)/docs directory along with Resource.xml

class BaseRecord(Record):
    def __init__(self, rec_type, row):
        super().__init__(rec_type, '$(P)$(R)' + row['PVName'])

        self['DESC'] = row['Description']

        if row['ASG'] != '' and row['ASG'].lower() != 'default':
            self['ASG'] = row['ASG'] 

        if row['Unit'] != '':
            self['EGU'] = row['Unit']
        if row['ZNAM'] != '':
            self['ZNAM'] = row['ZNAM'] 
        if row['ONAM'] != '':
            self['ONAM'] = row['ONAM'] 
        if row['ZSV'] != '':
            self['ZSV'] = row['ZSV']
        if row['OSV'] != '':
            self['OSV'] = row['OSV']
        if row['HighHigh'] != '':
            self['HIHI'] = row['HighHigh']
            self['HHSV'] = 'MAJOR'
        if row['LowLow'] != '':
            self['LOLO'] = row['LowLow']
            self['LLSV'] = 'MAJOR'
        if row['High'] != '':
            self['HIGH'] = row['High']
            if 'Reset' not in row['PVName']:
                self['HSV'] = 'MINOR'
        if row['Low'] != '':
            self['LOW'] = row['Low']
            self['LSV'] = 'MINOR'
        if row['UNSV'] != '':
            self['UNSV'] = row['UNSV']
        if row['ZRSV'] != '':
            self['ZRSV'] = row['ZRSV']
        if row['ONSV'] != '':
            self['ONSV'] = row['ONSV']
        if row['TWSV'] != '':
            self['TWSV'] = row['TWSV']
        if row['THSV'] != '':
            self['THSV'] = row['THSV'] 
        if row['DRVH'] != '':
            self['DRVH'] = row['DRVH'] 
        if row['DRVL'] != '':
            self['DRVL'] = row['DRVL'] 
        if row['Archive'] != '' and row['Archive'] != None and row['Archive'].lower() != 'none':
            self.add(Info('archive', 'policy:{}'.format(row['Archive'])))

    def addLink(self, link):
        raise NotImplementedError('You need to define a addLink method!')

class InputRecord(BaseRecord):
    def __init__(self, rec_type, row, link=None):
        super().__init__(rec_type, row)

        self['SCAN'] = 'I/O Intr'

        if not link:
            link = '@asyn(read{})'.format(row['StartAddress'])
        self['INP'] = link

class OutputRecord(BaseRecord):
    def __init__(self, rec_type, row, link=None):
        super().__init__(rec_type, row)

        self['PINI'] = 'YES'

        if not link:
            link = '@asyn(write{})'.format(row['StartAddress'])
        self['OUT'] = link


class bo(OutputRecord):
    def __init__(self, row):
        super().__init__('bo', row, '@asynMask(write{portNum} 0 0x{mask:02X} 500)'.format(portNum=row['StartAddress'], mask=2**int(row['Bit'])))

        self['DTYP'] = 'asynUInt32Digital'


class bi(InputRecord):
    def __init__(self, row, offset='0'):
        super().__init__('bi', row, '@asynMask(read{portNum} {offset} 0x{mask:02X} 500)'.format(portNum=row['StartAddress'], offset=offset, mask=2**int(row['Bit'])))

        self['DTYP'] = 'asynUInt32Digital'


class longin(InputRecord):
    def __init__(self, row, offset='0'):
        super().__init__('longin', row, '@asyn(read{portNum} {offset})'.format(portNum=row['StartAddress'], offset=offset))

        self['DTYP'] = 'asynInt32'

class longout(OutputRecord):
    def __init__(self, row):
        super().__init__('longout', row)

        self['DTYP'] = 'asynInt32'

class ai(InputRecord):
    def __init__(self, row, offset='0'):
        super().__init__('ai', row, '@asyn(read{portNum} {offset})'.format(portNum=row['StartAddress'], offset=offset))

        self['DTYP'] = 'asynFloat64'

class ao(OutputRecord):
    def __init__(self, row):
        super().__init__('ao', row)

        self['DTYP'] = 'asynFloat64'

class mbbo(OutputRecord):
    def __init__(self, row, fields):
        super().__init__('mbbo', row)
        
        mbFields = ['ZR', 'ON', 'TW', 'TH', 'FR', 'FV', 'SX', 'SV', 'EI', 'NI', 'TE', 'EL', 'TV', 'TT', 'FT', 'FF']

        self['DTYP'] = 'asynInt32'

        for (k, v), i in zip(fields.items(), mbFields[:len(fields)]):
            self[i + 'VL'] = k
            self[i + 'ST'] = v
        

class mbbi(InputRecord):
    def __init__(self, row, fields, offset='0'):
        super().__init__('mbbi', row, '@asyn(read{portNum} {offset})'.format(portNum=row['StartAddress'], offset=offset))

        mbFields = ['ZR', 'ON', 'TW', 'TH', 'FR', 'FV', 'SX', 'SV', 'EI', 'NI', 'TE', 'EL', 'TV', 'TT', 'FT', 'FF']

        self['DTYP'] = 'asynInt32'

        for (k, v), i in zip(fields.items(), mbFields[:len(fields)]):
            self[i + 'VL'] = k
            self[i + 'ST'] = v


class Parser(object):
    def __init__(self, database_file, start_cmd_file, scandinova_util, asyn_modbus_config_cmd, default_timeout):
        self.db_f = database_file
        self.st_f = start_cmd_file
        self.scandinova = scandinova_util
        self.config_cmd = asyn_modbus_config_cmd
        self.timeout = default_timeout

    def write_event_type(self, row, length, pvname):
        for i in range(length):
            row['PVName'] = pvname + str(i)
            rec = mbbi(row, self.scandinova.event_type_dict, i)
            self.db_f.write(str(rec))

    def write_event_time(self, row, addr, pvname):
        cmd = self.config_cmd.format(t='read', addr=addr, modbusCmd='4', 
                                    length='100', dtype='5', timeout=self.timeout)
        self.st_f.write(cmd)
        for i in range(0, 100, 2):
            row['PVName'] = pvname + str(i)
            rec = longin(row, i)
            self.db_f.write(str(rec))

        addr = str(int(addr) + 100)
        row['StartAddress'] = addr 
        cmd = self.config_cmd.format(t='read', addr=addr, modbusCmd='4', 
                                    length='100', dtype='5', timeout=self.timeout)
        self.st_f.write(cmd)
        for i in range(100, 200, 2):
            row['PVName'] = pvname + str(i)
            rec = longin(row, i - 100)
            self.db_f.write(str(rec))

    def write_event_longin_32bit(self, row, length, pvname):
        for i in range(0, length, 2):
            row['PVName'] = pvname + str(i)
            rec = longin(row, i)
            self.db_f.write(str(rec))

    def write_event_longin_16bit(self, row, length, pvname):
        for i in range(length):
            row['PVName'] = pvname + str(i)
            rec = longin(row, i)
            self.db_f.write(str(rec))

    def write_event_data_type(self, row, length, pvname, scandinova):
        for i in range(length):
            row['PVName'] = pvname + str(i)
            rec = mbbi(row, self.scandinova.event_log_dict, i)
            self.db_f.write(str(rec))

    def write_waveform(self, row, initial_addr, pvname):
        # byte 0-7      timestamp           3001-3004
        # byte 8-15     pulse id            3005-3008
        # byte 16-17    number of samples   3009
        # byte 18-1023  waveform            3010-3512

        row['PVName'] = pvname + 'TimeLower'
        rec = longin(row)
        self.db_f.write(str(rec))
        row['PVName'] = pvname + 'TimeUpper'
        rec = longin(row, 2)
        self.db_f.write(str(rec))
        row['PVName'] = pvname + 'PulseIDLower'
        rec = longin(row, 4)
        self.db_f.write(str(rec))
        row['PVName'] = pvname + 'PulseIDUpper'
        rec = longin(row, 6)
        self.db_f.write(str(rec))
        row['PVName'] = pvname + 'NumSamples'
        rec = longin(row, 8)
        self.db_f.write(str(rec))

        cmd = self.config_cmd.format(t='read', addr=initial_addr, modbusCmd='4', 
                                    length='100', dtype='4', timeout=self.timeout)
        self.st_f.write(cmd)
        for i in range(9,100):
            row['PVName'] = pvname + str(i - 9)
            rec = longin(row, i)
            self.db_f.write(str(rec))

        for addr_count in range(1,5):
            offset = 100 * addr_count
            addr = str(int(initial_addr) + offset)
            row['StartAddress'] = addr 
            cmd = self.config_cmd.format(t='read', addr=addr, modbusCmd='4', 
                                        length='100', dtype='4', timeout=self.timeout)
            self.st_f.write(cmd)
            for i in range(offset, offset+100):
                row['PVName'] = pvname + str(i - 9)
                rec = longin(row, i - offset)
                self.db_f.write(str(rec))

        addr = str(int(addr) + 100)
        row['StartAddress'] = addr
        cmd = self.config_cmd.format(t='read', addr=addr, modbusCmd='4', 
                                    length='12', dtype='4', timeout=self.timeout)
        self.st_f.write(cmd)
        for i in range(500,512):
            row['PVName'] = pvname + str(i - 9)
            rec = longin(row, i - 500)
            self.db_f.write(str(rec))


    def write_state_read_array(self, row, initial_addr, pvname):
        offsets = [100, 100, 56]
        for addr_count in range(len(offsets)):
            if addr_count > 0:
                offset = offsets[addr_count - 1] * addr_count
            else:
                offset = 0
            addr = str(int(initial_addr) + offset)
            row['StartAddress'] = addr 
            cmd = self.config_cmd.format(t='read', addr=addr, modbusCmd='4', 
                                        length=str(offsets[addr_count]), dtype='0', timeout=self.timeout)
            self.st_f.write(cmd)

            for i in range(offset, offset+offsets[addr_count]):
                matrix_dict = self.scandinova.matrix_info.get(str(i),None)
                if matrix_dict == None:
                    continue
                else:
                    split_string_list = matrix_dict['SSMName'].split('\\')
                    if len(split_string_list) == 2:
                        matrix_name = split_string_list[1]
                    else:
                        matrix_name = split_string_list[0]
                    matrix_pvname = matrix_name.replace(' ','')
                row['PVName'] = matrix_pvname + ':IntlkState'
                row['Description'] = 'State of mod where interlock is enabled'
                rec = mbbi(row, self.scandinova.state_dict, i - offset)
                self.db_f.write(str(rec))


    def write_status_bit_array(self, row, initial_addr, pvname):
        offsets = [100, 100, 56]
        for addr_count in range(len(offsets)):
            if addr_count > 0:
                offset = offsets[addr_count - 1] * addr_count
            else:
                offset = 0
            addr = str(int(initial_addr) + offset)
            row['StartAddress'] = addr 
            cmd = self.config_cmd.format(t='read', addr=addr, modbusCmd='4', 
                                        length=str(offsets[addr_count]), dtype='0', timeout=self.timeout)
            self.st_f.write(cmd)

            for i in range(offset, offset+offsets[addr_count]):
                matrix_dict = self.scandinova.matrix_info.get(str(i),None)
                if matrix_dict == None:
                    continue
                else:
                    split_string_list = matrix_dict['SSMName'].split('\\')
                    if len(split_string_list) == 2:
                        matrix_name = split_string_list[1]
                    else:
                        matrix_name = split_string_list[0]
                    matrix_pvname = matrix_name.replace(' ','')
                for j in range(2):
                    row['Bit'] = str(j)
                    if j == 0:
                        row['PVName'] = matrix_pvname + ':WarningON'
                        row['Description'] = 'ON if warning limit has been reached'
                    else:
                        row['PVName'] = matrix_pvname + ':IntlkON'
                        row['Description'] = 'ON if interlock limit has been reached'
                    rec = bi(row, i - offset)
                    self.db_f.write(str(rec))


    def write_event(self, row, initial_addr, pvname):
        cmd = self.config_cmd.format(t='read', addr=initial_addr, modbusCmd='4', 
                                    length='2', dtype='0', timeout=self.timeout)
        self.st_f.write(cmd)
        row['PVName'] = pvname + 'Increment' 
        rec = longin(row)
        self.db_f.write(str(rec))
        row['PVName'] = pvname + 'Type'
        rec = mbbi(row, self.scandinova.event_type_dict, 1)
        self.db_f.write(str(rec))

        addr = str(int(initial_addr) + 2)
        cmd = self.config_cmd.format(t='read', addr=addr, modbusCmd='4', 
                                    length='4', dtype='5', timeout=self.timeout)
        self.st_f.write(cmd)
        row['StartAddress'] = addr 
        row['PVName'] = pvname + 'TimeUpper'
        rec = longin(row)
        self.db_f.write(str(rec))
        row['PVName'] = pvname + 'TimeLower'
        rec = longin(row, 2)
        self.db_f.write(str(rec))
        
        addr = str(int(addr) + 4)
        cmd = self.config_cmd.format(t='read', addr=addr, modbusCmd='4', 
                                    length='2', dtype='5', timeout=self.timeout)
        self.st_f.write(cmd)
        row['StartAddress'] = addr 
        row['PVName'] = pvname + 'Trigger'
        rec = longin(row)
        self.db_f.write(str(rec))

        addr = str(int(addr) + 2)
        cmd = self.config_cmd.format(t='read', addr=addr, modbusCmd='4', 
                                    length='3', dtype='0', timeout=self.timeout)
        self.st_f.write(cmd)
        row['StartAddress'] = addr
        row['PVName'] = pvname + 'Item'
        rec = longin(row)
        self.db_f.write(str(rec))
        row['PVName'] = pvname + 'TextNum'
        rec = longin(row, 1)
        self.db_f.write(str(rec))
        row['PVName'] = pvname + 'DataType'
        rec = longin(row, 2)
        self.db_f.write(str(rec))

        addr = str(int(addr) + 3)
        cmd = self.config_cmd.format(t='read', addr=addr, modbusCmd='4', 
                                    length='2', dtype='5', timeout=self.timeout)
        self.st_f.write(cmd)
        row['StartAddress'] = addr 
        row['PVName'] = pvname + 'Data'
        rec = longin(row)
        self.db_f.write(str(rec))
