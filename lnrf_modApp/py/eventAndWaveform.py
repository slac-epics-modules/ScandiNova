import datetime
import signal
import time
import struct
import os
import sys
import numpy as np
import devsup.ptable as PT
from devsup.hooks import addHook
from devsup.db import Record 
from threading import Thread, Event
from scandinovaUtils import ScandinovaXmlParser 


class eventSupport(PT.TableBase):
    main_type = PT.Parameter(iointr=True)
    main_time = PT.Parameter(iointr=True)
    main_trig = PT.Parameter(iointr=True)
    main_text = PT.Parameter(iointr=True)
    main_text2 = PT.Parameter(iointr=True)
    intrlk_type = PT.Parameter(iointr=True)
    intrlk_time = PT.Parameter(iointr=True)
    intrlk_trig = PT.Parameter(iointr=True)
    intrlk_text = PT.Parameter(iointr=True)
    intrlk_text2 = PT.Parameter(iointr=True)
    waveform = PT.Parameter(iointr=True)
    waveform_time = PT.Parameter(iointr=True)
    waveform_pulse_id = PT.Parameter(iointr=True)
    waveform_rbv = PT.Parameter(iointr=True)

    def __init__(self, name, pv_prefix, toppath, eventpath):
        super().__init__(name=name)
        self.shutdown = False
        self.prefix = pv_prefix
        self.top_path = toppath 
        self.event_file_path = eventpath
        self.main_event_out_dict = {'Type': self.main_type, 'Time': self.main_time, 
                                    'Trig': self.main_trig, 'Text': self.main_text, 
                                    'Text2': self.main_text2 }
        self.last_event_out_dict = {'Type': self.intrlk_type, 'Time': self.intrlk_time, 
                                    'Trig': self.intrlk_trig, 'Text': self.intrlk_text, 
                                    'Text2': self.intrlk_text2 }

    def stop_threads(self):
        self.shutdown = True
        print('Shutting down python threads')
        time.sleep(1)

    def log_error(self, function_name, error_msg):
        print('{} - {} - {}'.format(function_name, datetime.datetime.now().strftime("%Y-%m-%d %H:%M"), error_msg))

    def update_waveform(self):
        function_name = 'update_waveform'
        # sleep and allow waveform to be initialized
        time.sleep(5)
        waveform_size = 503
        waveform_pvs = [Record(self.prefix + 'Waveform{}'.format(i)) for i in range(waveform_size)]

        lower_32_bits_timestamp_pv = Record(self.prefix + 'WaveformTimeLower')
        upper_32_bits_timestamp_pv = Record(self.prefix + 'WaveformTimeUpper')
        pulse_id1_pv = Record(self.prefix + 'WaveformPulseIDLower')
        pulse_id2_pv = Record(self.prefix + 'WaveformPulseIDUpper')
        num_samples_pv = Record(self.prefix + 'WaveformNumSamples')
        waveform_choice_pv = Record(self.prefix + 'WaveformType')

        error_count =  0
        waveform_array = np.empty(waveform_size)
        while True:
            #self.waveform.value = [waveform_pvs[i].get() for i in range(num_samples_pv.get())]
            waveform_to_fetch = waveform_choice_pv.VAL
            if waveform_to_fetch is None:
                self.log_error(update_waveform, 'Cannot get PV: {}. Sleeping for: {}'.format(waveform_choice_pv.name(), str(error_count**2)))
                if error_count < 8:
                    error_count += 1
                time.sleep(error_count**2)
                continue
            else:
                error_count = 0 
            if not 140 > waveform_to_fetch > 98:
                self.log_error(function_name, 'Waveform to fetch value is not within the correct range (99-139). PV: {} = {}'.format(waveform_choice_pv.name(), waveform_to_fetch))
                time.sleep(1)
                continue
            waveform_string = self.waveform_strings.get(waveform_to_fetch, None)
            if waveform_string is None:
                self.log_error(function_name, 'Waveform fetch value index is not found in waveform_strings dictionary')
                continue
            else:
                self.waveform_rbv.value = self.waveform_strings[waveform_to_fetch]
                self.waveform_rbv.notify()

            for idx, pv in enumerate(waveform_pvs):
                pv_value = pv.VAL
                if pv_value is None:
                    self.log_error('Waveform pv is disconnected: {}'.format(pv.name()))
                    pv_value = 0
                else:
                    waveform_array[idx] = pv_value
            self.waveform.value = waveform_array
            self.waveform.notify()

            t1 = lower_32_bits_timestamp_pv.VAL
            t2 = upper_32_bits_timestamp_pv.VAL 
            if t1 is None:
                self.log_error(function_name, 'Waveform timestamp PV is disconnected: {}'.format(lower_32_bits_timestamp_pv.name()))
            elif t2 is None:
                self.log_error(function_name, 'Waveform timestamp PV is disconnected: {}'.format(upper_32_bits_timestamp_pv.name()))
            else:
                time64 = (t2 << 32) + t1
                time64 = (time64 / 10000000) - 11644473600
                date = datetime.datetime.fromtimestamp(time64).strftime('%Y-%m-%d %H:%M:%S')
                self.waveform_time.value = date
                self.waveform_time.notify()

            p1 = pulse_id1_pv.VAL
            p2 = pulse_id2_pv.VAL
            if p1 is None:
                self.log_error(function_name, 'Waveform pulse ID PV is disconnected: {}'.format(pulse_id1_pv.name()))
            elif p2 is None:
                self.log_error(function_name, 'Waveform pulse ID PV is disconnected: {}'.format(pulse_id2_pv.name()))
            else:
                self.waveform_pulse_id.value = str((p2 << 32) + p1)
                self.waveform_pulse_id.notify()

            time.sleep(.2)

    def event_loop(self):
        event_file = self.event_file_path + '/{}events.txt'.format(self.prefix.replace(':', '_'))
        if not os.path.exists(self.event_file_path):
            os.makedirs(self.event_file_path)
        if os.path.isfile(event_file):
            os.rename(event_file, self.event_file_path + '/{}events-{}.txt'.format(self.prefix, datetime.datetime.now().strftime("%m-%d-%Y-%H-%M-%S")).replace(':', '_'))
        with open(event_file, 'w') as event_f:
            os.chmod(event_file, 0o644)
            event_f.write("{0:<16}{1:<32}{2:<16}{3:<32}\n".format("Type", "Timestamp", "TrigID", "Text"))
            prev_idx = -1
            event_idx = Record(self.prefix + 'EventIndex')
            while True:
                if self.shutdown:
                    break
                curr_idx = event_idx.VAL
                if curr_idx != None:
                    curr_idx = curr_idx % 50
                    while curr_idx != prev_idx:
                        prev_idx += 1
                        if prev_idx == 50:
                            prev_idx = 0
                        time.sleep(.5)

                        event_str = self.parse_event(self.event_dict, prev_idx)
                        if event_str is not None:
                            event_f.write(event_str)
                            event_f.flush()
                    else:
                        time.sleep(.2)
                else:
                    self.log_error('event_loop', 'Could not get index PV: {}'.format(self.prefix +'EventIndex'))
                    time.sleep(.2)

    def update_main(self):
        main_event_pv = Record(self.prefix + 'MainEventIncrement')
        self.parse_event(self.main_event_dict, 0, self.main_event_out_dict)
        curr_v = -1 
        while True:
            if self.shutdown:
                break
            curr_inc = main_event_pv.VAL
            if curr_inc is None:
                self.log_error('update_main', 'Main event increment PV is disconnected: {}'.format(main_event_pv.name()))
                time.sleep(2)
                continue
            else:
                if curr_v != curr_inc:
                    self.parse_event(self.main_event_dict, 0, self.main_event_out_dict)
                    curr_v = curr_inc 
                    time.sleep(.2)
                else:
                    time.sleep(.5)

    def update_last(self):
        last_event_pv = Record(self.prefix + 'LastIntlkIncrement')
        self.parse_event(self.last_event_dict, 0, self.last_event_out_dict)
        curr_v = -1 
        while True:
            if self.shutdown:
                break
            curr_inc = last_event_pv.VAL
            if curr_inc is None:
                self.log_error('update_last', 'Last interlock event increment PV is disconnected: {}'.format(last_event_pv.name()))
                time.sleep(2)
                continue
            else:
                if curr_v != curr_inc:
                    self.parse_event(self.last_event_dict, 0, self.last_event_out_dict)
                    curr_v = curr_inc 
                    time.sleep(.2)
                else:
                    time.sleep(.5)

    # get event pvs for index and either write to file or write to PVs
    def parse_event(self, modbus_dict, idx, out_pvs=None):
        function_name = 'parse_event'
        e_type = Record(modbus_dict['Type'].format(idx)).VAL
        if e_type is None:
            self.log_error(function_name, 'Event type PV is disconnected: {}. Skipping any updates to event PVs'.format(modbus_dict['Type'].format(idx)))
            return
        e_type_str = self.scandinova.event_type_dict[str(e_type)]
        time_lower_32_bits = Record(modbus_dict['Time1'].format(idx*4)).VAL
        if time_lower_32_bits is None:
            self.log_error(function_name, 'Event lower time PV is disconnected: {}. Skipping any updates to event PVs'.format(modbus_dict['Time1'].format(idx*4)))
            return
        time_upper_32_bits = Record(modbus_dict['Time2'].format(idx*4 + 2)).VAL
        if time_upper_32_bits is None:
            self.log_error(function_name, 'Event upper time PV is disconnected: {}. Skipping any updates to event PVs'.format(modbus_dict['Time2'].format(idx*4+ 2)))
            return
        trig = Record(modbus_dict['Trig'].format(idx*2)).VAL
        if trig is None:
            self.log_error(function_name, 'Event trigger PV is disconnected: {}. Skipping any updates to event PVs'.format(modbus_dict['Trig'].format(idx*2)))
            return
        item = Record(modbus_dict['Item'].format(idx)).VAL
        if item is None:
            self.log_error(function_name, 'Event item PV is disconnected: {}. Skipping any updates to event PVs'.format(modbus_dict['Item'].format(idx)))
            return
        txt_num = Record(modbus_dict['TxtNum'].format(idx)).VAL
        if txt_num is None:
            self.log_error(function_name, 'Event text number PV is disconnected: {}. Skipping any updates to event PVs'.format(modbus_dict['TxtNum'].format(idx)))
            return
        dtype = Record(modbus_dict['DType'].format(idx)).VAL
        if dtype is None:
            self.log_error(function_name, 'Event data type PV is disconnected: {}. Skipping any updates to event PVs'.format(modbus_dict['DType'].format(idx)))
            return
        data = Record(modbus_dict['Data'].format(idx*2)).VAL
        if data is None:
            self.log_error(function_name, 'Event data PV is disconnected: {}. Skipping any updates to event PVs'.format(modbus_dict['Data'].format(idx*2)))
            return

        time64 = (time_upper_32_bits << 32) + time_lower_32_bits 
        time64 = (time64 / 10000000) - 11644473600
        date = datetime.datetime.fromtimestamp(time64).strftime('%Y-%m-%d %H:%M:%S')
        if e_type == 0:
            d = self.scandinova.state_dict
        elif e_type == 1:
            d = self.scandinova.warning_dict
        elif e_type == 2:
            d = self.scandinova.interlock_dict
        elif e_type == 3:
            d = self.scandinova.error_dict
        elif e_type == 4:
            d = self.scandinova.param_dict
        elif e_type == 5:
            d = self.scandinova.message_dict
        else:
            self.log_error(function_name, 'Event type number is out of range, skipping')
            return
        try:
            event_str = d[str(txt_num)]
        except KeyError:
            self.log_error(function_name, 'Key Error at index: {} . For the follwing dictionary: {}'.format(str(txt_num), d))
            event_str = ' '

        if dtype == 1: # real
            try:
                real_rpr = struct.unpack('f', struct.pack('I', int(data)))[0]
                event_str += " {}".format(round(real_rpr,3))
            except struct.error:
                self.log_error(function_name, 'Error converting bits of data into float')
        elif dtype == 2: # bool
            event_str += " {}".format(bool(data))
        elif dtype in [5, 8]: # word, dword
            try:
                event_bits = self.scandinova.matrix_dict[str(item)]['BitSpec']
            except KeyError:
                event_bits = None
            if event_bits is not None:
                binary_data = bin(data)[2:]
                errored_bits = "" 
                for idx, bit in enumerate(event_bits):
                    data_bit = int(binary_data[idx*-1 - 1])
                    if data_bit == 0 and bit is not None:
                        errored_bits += bit + ", "
                event_str += ". Following bits errored: {}".format(errored_bits[:-2])
            else:
                event_str += " {}".format(str(data))
        elif dtype in [3, 4, 6, 7]: # int, uint, dint, udint
            event_str += " {}".format(int(data))

        try:
            event_name = self.scandinova.matrix_dict[str(item)]['Name']
            event_unit = self.scandinova.matrix_dict[str(item)]['Unit'] 
        except KeyError:
            event_name = None
        if event_name != None:
            event_str = event_name + ' ' + event_str
        if event_unit != None:
            event_str = event_str + ' ' + event_unit
            event_str = event_str.replace('µ', 'u')

        if out_pvs != None:
            out_pvs['Type'].value = e_type_str
            out_pvs['Time'].value = date 
            out_pvs['Trig'].value = str(trig)
            if len(event_str) > 39:
                out_pvs['Text'].value = event_str[0:39]
                out_pvs['Text2'].value = event_str[39:] 
            else:
                out_pvs['Text'].value = event_str 
                out_pvs['Text2'].value = '' 

            out_pvs['Type'].notify()
            out_pvs['Time'].notify()
            out_pvs['Trig'].notify()
            out_pvs['Text'].notify()
            out_pvs['Text2'].notify()

        return "{0:<16}{1:<32}{2:<16}{3:<32}\n".format(e_type_str, date, trig, event_str)

    def create_dicts(self):
        resource_file = self.top_path + '/lnrf_modApp/py/Resource.xml'
        matrix_file = self.top_path + '/lnrf_modApp/py/MatrixInfo.xml'
        if not os.path.isfile(resource_file):
            print('Resource.xml file not found. Expected to be here: {}'.format(resource_file))
            return False 
        if not os.path.isfile(matrix_file):
            print('MatrixInfo.xml file not found. Expected to be here: {}'.format(matrix_file))
            return False 

        self.scandinova = ScandinovaXmlParser(  self.top_path + '/lnrf_modApp/py/Resource.xml', 
                                                self.top_path  + '/lnrf_modApp/py/MatrixInfo.xml')
         
        self.event_dict = {
            'Type':     self.prefix + 'EventType{}',
            'Time1':    self.prefix + 'EventTime{}',
            'Time2':    self.prefix + 'EventTime{}',
            'Trig':     self.prefix + 'EventTrigger{}',
            'Item':     self.prefix + 'EventItem{}',
            'TxtNum':   self.prefix + 'EventTextNum{}',
            'DType':    self.prefix + 'EventDataType{}',
            'Data':     self.prefix + 'EventData{}',
        }
        self.main_event_dict = {
            'Type':     self.prefix + 'MainEventType',
            'Time1':    self.prefix + 'MainEventTimeUpper',
            'Time2':    self.prefix + 'MainEventTimeLower',
            'Trig':     self.prefix + 'MainEventTrigger',
            'Item':     self.prefix + 'MainEventItem',
            'TxtNum':   self.prefix + 'MainEventTextNum',
            'DType':    self.prefix + 'MainEventDataType',
            'Data':     self.prefix + 'MainEventData',
        }
        self.last_event_dict = {
            'Type':     self.prefix + 'LastIntlkType',
            'Time1':    self.prefix + 'LastIntlkTimeUpper',
            'Time2':    self.prefix + 'LastIntlkTimeLower',
            'Trig':     self.prefix + 'LastIntlkTrigger',
            'Item':     self.prefix + 'LastIntlkItem',
            'TxtNum':   self.prefix + 'LastIntlkTextNum',
            'DType':    self.prefix + 'LastIntlkDataType',
            'Data':     self.prefix + 'LastIntlkData',
        }
        waveform_choices = ['T-0', 'T-1', 'T-2', 'T-3', 'T-4', 'Saved Reference', 'Upper Interlock Boundary', 'Lower Interlock Boundary',
                            'Upper Warning Boundary', 'Lower Warning Boundary']
        waveform_options = ['Cvd', 'Ct', 'RfOutFwd', 'RfOutRfl'] 
        index = 99
        self.waveform_strings = { index: 'Error during waveform fetch' }
        index += 1
        for option in waveform_options:
            for choice in waveform_choices:
                self.waveform_strings[index] = option + ' ' + choice
                index += 1
        return True

        
def build(prefix, filepath, eventpath):
    stopEvent = Event()

    sup = eventSupport(name='event', pv_prefix=prefix, toppath=filepath, eventpath=eventpath)
    if sup.create_dicts() == False:
        print("Error in create_dicts(), no threads will be run")
        return

    # Workaround CTRL+C not working
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    def launcher(stopEvent):
        main_thread = Thread(target=sup.update_main)
        main_thread.start()

        event_thread = Thread(target=sup.event_loop)
        event_thread.start()

        last_thread = Thread(target=sup.update_last)
        last_thread.start()

        waveform_thread = Thread(target=sup.update_waveform)
        waveform_thread.start()

        # Keep thread alive until it is time to go
        stopEvent.wait()

    t = Thread(target=launcher, args=(stopEvent,))

    def stopLauncher():
        sup.stop_threads()

        stopEvent.set()

        t.join()

    addHook('AtIocExit', stopLauncher)

    # iocInit somehow messes up contexts, so start the thread
    # after iocInit
    addHook('AfterIocRunning', t.start)

    return sup
