from xml.etree import ElementTree as ET
from collections import OrderedDict

class ScandinovaXmlParser(object):
    def __init__(self, resource_xml_path='./Resource.xml', matrix_info_xml_path='./MatrixInfo.xml'):
        self.create_dicts(resource_xml_path)

        self.create_matrix(matrix_info_xml_path)

    def create_dicts(self, file_path):
        self.event_type_dict = OrderedDict()
        self.event_type_dict['0'] = 'State'
        self.event_type_dict['1'] = 'Warning'
        self.event_type_dict['2'] = 'Interlock'
        self.event_type_dict['3'] = 'Error'
        self.event_type_dict['4'] = 'Parameter'
        self.event_type_dict['5'] = 'Message'

        self.event_log_dict = OrderedDict()
        self.event_log_dict['0'] = 'No data'
        self.event_log_dict['1'] = 'Real'
        self.event_log_dict['2'] = 'Bool'
        self.event_log_dict['3'] = 'Int'
        self.event_log_dict['4'] = 'Uint'
        self.event_log_dict['5'] = 'Word'
        self.event_log_dict['6'] = 'Dint'
        self.event_log_dict['7'] = 'Udint'
        self.event_log_dict['8'] = 'Dword'

        tree = ET.parse(file_path)
        root = tree.getroot()

        self.state_dict = OrderedDict()
        self.message_dict = OrderedDict() 
        self.warning_dict = OrderedDict() 
        self.interlock_dict = OrderedDict()
        self.error_dict = OrderedDict()
        self.matrix_dict = OrderedDict()
        self.param_dict = OrderedDict()

        for child in root:
            if child.tag == 'Strings':
                for c in child:
                    if c.tag == 'State':
                        for k in c:
                            for t in k:
                                if t.tag == 'Text':
                                    self.state_dict[k.tag.replace('no', '')] = t.text
                    elif c.tag == 'Message':
                        for k in c:
                            for t in k:
                                if t.tag == 'Text':
                                    self.message_dict[k.tag.replace('no', '')] = t.text
                    elif c.tag == 'Warning':
                        for k in c:
                            for t in k:
                                if t.tag == 'Text':
                                    self.warning_dict[k.tag.replace('no', '')] = t.text
                    elif c.tag == 'Interlock':
                        for k in c:
                            for t in k:
                                if t.tag == 'Text':
                                    self.interlock_dict[k.tag.replace('no', '')] = t.text
                    elif c.tag == 'Error':
                        for k in c:
                            for t in k:
                                if t.tag == 'Text':
                                    self.error_dict[k.tag.replace('no', '')] = t.text
                    elif c.tag == 'Param':
                        for k in c:
                            for t in k:
                                if t.tag == 'Text':
                                    self.param_dict[k.tag.replace('no', '')] = t.text
            elif child.tag == 'MatrixItems':
                for c in child:
                    m_dict = {}
                    for k in c:
                        if k.tag == 'Name':
                            m_dict['Name'] = k.text
                        elif k.tag == 'Unit':
                            m_dict['Unit'] = k.text
                        elif k.tag == 'BitSpec':
                            bit_list = []
                            for b in k:
                                bit_list.append(b.text)
                            m_dict['BitSpec'] = bit_list
                    self.matrix_dict[c.tag.replace('Index', '')] = m_dict 

    def create_matrix(self, file_path):
        tree = ET.parse(file_path)
        root = tree.getroot()

        self.matrix_info = {}
        self.matricies_used = []

        for child in root:
            m_dict = {}
            for c in child:
                if c.tag == 'SSMName':
                    m_dict['SSMName'] = c.text
                elif c.tag == 'Name':
                    m_dict['Name'] = c.text
                elif c.tag == 'Index':
                    index = c.text
                elif c.tag == 'TargetStateStr':
                    m_dict['TargetStateStr'] = c.text
                elif c.tag == 'TargetStateNbr':
                    m_dict['TargetStateNbr'] = c.text
                else:
                    continue
            self.matrix_info[index] = m_dict
            self.matricies_used.append(index)


if __name__ == "__main__":
    s = ScandinovaXmlParser('./Resource.xml', './MatrixInfo.xml')
    print(s.interlock_dict)
    print(s.state_dict)
    print(s.message_dict)
    print(s.warning_dict)
    print(s.interlock_dict)
    print(s.error_dict)
    print(s.param_dict)
