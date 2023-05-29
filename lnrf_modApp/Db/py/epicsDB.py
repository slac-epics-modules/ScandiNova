#!/usr/bin/env python

from __future__ import print_function

"""
    File name: epicsDB.py
    Author: Eric Williams <ECWilliams@lbl.gov>
    Date created: 12/03/2018
    Date last modified: 
    Python Version: 2.7.3

    Description:
    Module of objects for reading, parsing, manipulating, and writing EPICS Database files
    
    Bugs:
    Record Information item parser blows up on an empty value.
    Can't change Record name once it's added to a Database
"""

class EpicsDbError(Exception):
	def __init__(self, expr, msg):
		self.expr = expr
		self.msg = msg

	def __str__(self):
		return self.msg % self.expr


class Comment(object):
	"""An EPICS Comment.  Can be associated with a Field, or between Records.
	These are also used for blank lines."""
	def __init__(self, text):
		self.text = text
	
	def __str__(self):
		return self.text + '\n'

class Field(object):
	"""A Field in an EPICS Record.  Consists of tag and value, with optional Comment."""
	def __init__(self, tag, val, pyComment=None):
		self.tag = tag
		self.val = val
		self.pyComment = pyComment
	
	def __str__(self):
		ret = 'field(%s, "%s")' % (self.tag, self.val)
		if self.pyComment:
			ret += '\t%s' % self.pyComment
		ret += '\n'
		return ret

class Info(object):
	"""An Info Field in a Record.  Similar to a Field, has a name and value.
	Like Fields, Info Fields must have a unique name within the Record."""
	def __init__(self, name, value, pyComment=None):
		self.name = name
		self.value = value
		self.pyComment = pyComment
	
	def __str__(self):
		ret = 'info(%s,"%s")' % (self.name, self.value)
		if self.pyComment:
			ret += '\t%s' % self.pyComment
		ret += '\n'
		return ret

class Alias(object):
	"""An Alias within a Record."""
	def __init__(self, name, pyComment=None):
		self.name = name
		self.pyComment = pyComment
	
	def __str__(self):
		ret = 'alias("%s")' % self.name
		if self.pyComment:
			ret += '\t%s' % self.pyComment
		ret += '\n'
		return ret

class Record(object):
	'''EPICS record container'''
	def __init__(self, type, name):
		"""Create an empty Record with provided record type and name"""
		self.type = type
		self.name = name
		self.elements = []
		self.fields = {}
		self.info = {}
		self.alias = []
		self.eleIndex = None

	def add(self, element):
		"""Add an element to the Record.  
		An element is a Field, Info Field, or Alias."""
		self.elements.append(element)
		if isinstance (element, Field): self.fields[element.tag] = element
		if isinstance (element, Info): self.info[element.name] = element
		if isinstance (element, Alias): self.alias.append(element)
	
	def __str__(self):
		ret = 'record (%s,"%s") {\n' % (self.type, self.name)
		for e in self.elements: 
			ret += '\t' + e.__str__()
		ret += '}\n'
		return ret

	def getInfo(self, name):
		"""Retrieve Info Field value from Record by name."""
		if name in self.info:
			return self.info[name].value
		else:
			return None

	# These list methods work on Fields only
	def __contains__(self, name):
		"""True if Record contains named Field"""
		return name in self.fields
	
	def __getitem__(self, name):
		"""Return value for named Field"""
		return self.fields[name].val
		
	def __setitem__(self, name, val):
		"""Set named Field to value.  Create the Field if it doesn't exist."""
		try:
			i = self.elements.index(self.fields[name])
			if type(val) == tuple:
				self.elements[i] = Field(name, val[0], pyComment=val[1])
			else:
				self.elements[i] = Field(name, val)
		except:
			i = len(self.elements)
			if type(val) == tuple:
				self.elements.append(Field(name, val[0], pyComment=val[1]))
			else:
				self.elements.append(Field(name, val))
		self.fields[name] = self.elements[i]
		
	def __delitem__(self, name):
		"""Delete the named Field from the Record."""
		i = self.elements.index(self.fields[name])
		self.elements.pop(i)
		del self.fields[name]

	def __iter__(self):
		self.eleIndex = 0
		return self

	def __next__(self):
		"""Iterator for Field names in Record."""
		try:
			while not isinstance(self.elements[self.eleIndex], Field):
				self.eleIndex += 1
			self.eleIndex += 1
			return self.elements[self.eleIndex - 1].tag
		except IndexError:
			self.eleIndex = None
			raise StopIteration

from pyparsing import *

class Database(object):
	'''EPICS database container'''
	
	def __init__(self, verbose=0):
		"""Create a blank Database.  Set verbose to turn on parse debugging."""
		self.db = []
		self.recs = []
		self.ndx = {}
		self.recIndex = None
		self.verbose = verbose
		self._rec = None		# Local Record for parser semantics
	
	def add(self, element):
		"""Add an element to a Database.  Can be a Record or Comment."""
		if isinstance (element, Record):
			if element.name in self.ndx:
				raise EpicsDbError(element.name, "Attempt to add Record '%s' already in Database")
			self.ndx[element.name] = element
			self.recs.append(element)
		self.db.append(element)
	
	def __str__(self):
		ret = ""
		for e in self.db:
			ret += str(e)
		return ret

	@staticmethod
	def expand(field, macros):
		'''Return field with macros expanded (if any)'''
		left = len(field)
		while left >= 0:
			try:
				# Scan field right-to-left for '$'
				end = left
				left = -1
				left = field.rindex('$', 0, end)	# exception if not found
				# Validate '$' is followed by '(' or '{'
				lbracket = field[left+1]
				if   lbracket == '(' : rbracket = ')'
				elif lbracket == '{' : rbracket = '}'
				else: continue
				# Extract string within brackets
				right = field.index(rbracket, left+2)	# exception if not found
				m = field[left+2:right]
				# Split out default substitution, if there is one
				try:
					(m,default) = m.split('=')
				except:
					default = None
				if m in macros:
					field = field[:left] + macros[m] + field[right+1:]
				elif default != None:
					field = field[:left] +  default  + field[right+1:]
			except: continue
		return field
		
	def parse(self, dbText, subst=''):
		'''Parse EPICS database string into Database'''
		
		# Transform dbLoadRecords() substitution string into dictionary
		macros = {}
		for m in subst.split(','):
			m2 = m.split('=')
			if len(m2) == 2:
				macros[m2[0]] = m2[1]

		# Semantics
		def startRec(s,l,t):
			decl = t[0]
			name = decl['name']
			name = self.expand(name, macros)
			if self.verbose: print ('startRec', name)
			self._rec = Record(decl['type'], name)
			
		def doField(s,l,t):
			if self.verbose: print ('doFields', t, s[l:s.find('\n',l)])
			f = t[0]
			if self.verbose > 1: print (f.dump())
			tag = f['tag']
			val = f.get('val', '')
			val = self.expand(val, macros)
			if len(t) > 1:
				self._rec.add(Field(tag, val, t[1]))	# Comment
			else:
				self._rec.add(Field(tag, val))

		def doAlias(s,l,t):
			if self.verbose: print ('doAlias', t, s[l:s.find('\n',l)])
			if self.verbose > 1: print (t.dump())
			alias_name = self.expand(t['alias_name'], macros)
			if len(t) > 1:
				self._rec.add(Alias(alias_name, t[1]))
			else:
				self._rec.add(Alias(alias_name))

		def doInfo(s,l,t):
			if self.verbose: print ('doInfo', t, s[l:s.find('\n',l)])
			f = t[0]
			val = self.expand(f['val'], macros)
			if len(t) > 1:
				self._rec.add(Info(f['tag'], val, t[1]))
			else:
				self._rec.add(Info(f['tag'], val))

		def endRec(s,l,t):
			if self.verbose: print ('endRec')
			self.add(self._rec)
			self._rec = None

		def lineComment(s,l,t):
			if self.verbose: print ('lineComment')
			if self.verbose > 1: print (t.dump())
			self.add(Comment(t[0]))

		def bl(s,l,t):
			if self.verbose: print ('blankLine')
			self.add(Comment(''))

			
		# Grammar
		EOL = LineEnd().suppress()
		fieldTag = Word(alphanums + '_')
		unquotedString = Word(alphanums + '_+-:.[]<>;')
		fieldVal = QuotedString('"') | unquotedString;
		tagGroup = Group(Suppress('(') + fieldTag('tag') + Suppress(',') +
			fieldVal('val') + Suppress(')'))
		recField = Suppress(Literal('field')) + tagGroup + Optional(pythonStyleComment)
		recField.setParseAction(doField)
		recInfo = Suppress(Literal('info')) + tagGroup + Optional(pythonStyleComment)
		recInfo.setParseAction(doInfo)
		aliasName = Suppress('(') + fieldVal('alias_name') + Suppress(')')
		recAlias = Suppress(Literal('alias')) + aliasName + Optional(pythonStyleComment)
		recAlias.setParseAction(doAlias)
		recType = Word(alphas)
		recName = QuotedString('"')
		recordGroup = Group(Suppress('(') + recType('type') + Suppress(',') +
			recName('name') + Suppress(')'))
		recordGroup.setParseAction(startRec)
		prop = recField | recInfo | recAlias | pythonStyleComment
		recStr = Literal('record') | Literal('grecord')
		dbRecord = Group(Suppress(recStr) + recordGroup +
			Suppress('{') + ZeroOrMore(prop) + Suppress('}'))
		dbRecord.setParseAction(endRec)
		commentLine = lineStart + pythonStyleComment
		commentLine.setParseAction(lineComment)
		blankLine = Regex(r"\n\s*?(?=\n)").setWhitespaceChars(' \t')
		blankLine.setParseAction(bl)
		element = blankLine | dbRecord | commentLine
		EPICSdb = OneOrMore(element)
		##########################################
		
		#EPICSdb.setDebug()
		EPICSdb.parseString(dbText, parseAll=True)

	# These list methods work on Records only
	def __contains__(self, name):
		"""True if Database contains named Record."""
		return name in self.ndx
	
	def __getitem__(self, name):
		"""Return named Record"""
		return self.ndx[name]
	
	def __setitem__(self, rec):
		"""Replace existing named Record in Database."""
		i = self.recs.index(self.ndx[rec.name])
		self.recs[i] = rec
		self.ndx[rec.name] = self.recs[i]
	
	def __delitem__(self, name):
		"""Delete named Record from Database."""
		if not name in self.ndx:
			raise ValueError('Record \'%s\' is not in Database' % name)
		r = self.ndx[name]
		self.db.pop(self.db.index(r))		
		self.recs.pop(self.recs.index(r))
		del self.ndx[name]
	
	def __iter__(self):
		self.recIndex = 0
		return self
	
	def __next__(self):
		"""Iterator for Records in Database."""
		try:
			self.recIndex += 1
			return iter(self.recs[self.recIndex - 1])
		except IndexError:
			self.recIndex = None
			raise StopIteration

if __name__ == '__main__':
	testDB = """
# Created by plcDbX.py on Tue Nov 27 16:16:13 2018
#
# Configuration for this run:
#   readOnly = False
#   addUserName = False
#
# Dropped Records:
#     BR4_____RFCONT_AC01 R1903:F01 R1903:F02 R1903:F03 R2000:F01
#     R2000:F02 R2000:F03 R2001:F01 R2001:F02 R2001:F03 R2003:F01
#     R2003:F02 R2003:F03 R2004:F01 R2004:F02 R2004:F03 R2006:F01
#     R2006:F02 R2006:F03
#
# Modbus port driver creation calls for st.cmd:
#   drvModbusAsynConfigure("rfplc_R3010","rfplc",1,3,3009,23,0,500,"Horner")
#   drvModbusAsynConfigure("rfplc_R4902","rfplc",1,3,4901,49,0,500,"Horner")
#   drvModbusAsynConfigure("rfplc_R5000","rfplc",1,3,4999,7,0,500,"Horner")
#   drvModbusAsynConfigure("rfplc_W3035","rfplc",1,6,3034,4,0,1,"Horner")
#   drvModbusAsynConfigure("rfplc_W4858","rfplc",1,6,4857,6,0,1,"Horner")
#   drvModbusAsynConfigure("rfplc_W5007","rfplc",1,6,5006,1,0,1,"Horner")

record (ai,"Power_RF_fwd") {
	field(DESC,"R1906 RF_fwd_Power")
	field(DTYP,"asynInt32")
	field(EGU,"PCNT")
	field(EGUF,"10.0")
	field(EGUL,"-10.0")
	field(HHSV,"MAJOR")
	field(HIHI,"135.0")		# Not sure how this works given EGUF and EGUL
	field(HOPR,"135.0")
	field(INP,"@asynMask(rfplc_R4902,4,-16)MODBUS_DATA")
	field(LINR,"NO CONVERSION")
	field(LLSV,"NO_ALARM")
	field(LOLO,"0.0")
	field(LOPR,"0.0")
	field(PREC,"0")
	field(SCAN,"I/O Intr")
	alias("BR00____RFFWD__AM00")
}
record (ai,"Power_RF_ref") {
	field(DESC,"R1907 RF_ref_Power")
	field(DTYP,"asynInt32")
	field(EGU,"W")
	field(EGUF,"3276.7")
	field(EGUL,"-3276.7")
	field(HHSV,"MAJOR")
	field(HIHI,"35.0")
	field(HOPR,"35.0")
	field(INP,"@asynMask(rfplc_R4902,5,-16)MODBUS_DATA")
	field(LINR,"LINEAR")
	field(LLSV,"NO_ALARM")
	field(LOLO,"0.0")
	field(LOPR,"0.0")
	field(PREC,"1")
	field(SCAN,"I/O Intr")
	alias("BR00____RFREF__AM01")
	info(autosavefields_pass0, "HIHI HHSV")
}
"""
	import argparse
	helpText = 'Test EPICS Database'
	argParser = argparse.ArgumentParser(description=helpText)
	argParser.add_argument('filename', nargs='?')
	argParser.add_argument('-m', metavar='MACRO', nargs=1, help='Set macro values for parse')
	argParser.add_argument('-v', '--verbose', action='count', default=0,
		help='Increase verbosity of debugging')
	cmdline = argParser.parse_args()
	print(cmdline)
	verbose = cmdline.verbose
	filename = cmdline.filename
	macro = None if cmdline.m == None else cmdline.m[0]
	
	dbOut = Database(verbose=verbose)
	if filename:
		with open(filename,'r') as f: 
			dbOut.parse(f.read(), subst=macro)
		print (dbOut)
	else:
		dbOut.parse(testDB)
		print (dbOut)

		print ('-+-'*8, 'Now remove Power_RF_ref', '-+-'*8)
		del dbOut['Power_RF_ref']
		print (dbOut)


