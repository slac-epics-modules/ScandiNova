from scandinovaUtils import ScandinovaXmlParser

scandinova = ScandinovaXmlParser()

xml_str = \
'''<widget type="embedded" version="2.0.0">
  <name>Embedded Display_3</name>
  <file>_matrix-intk.bob</file>
  <macros>
    <N>{N}</N>
    <T>{T}</T>
  </macros>
  <x>8</x>
  <y>{Y}</y>
  <width>300</width>
  <height>20</height>
  <resize>2</resize>
</widget>'''

y=29 
for k, v in scandinova.matrix_info.items():
    if v['SSMName'] == None or v['SSMName'] == "":
        continue
    print(xml_str.format(N=k,T=v['SSMName'],Y=y))
    y += 20
