#!/usr/bin/env python

from xml.dom import minidom
import sys

doc = minidom.parse(sys.argv[1])

filters = doc.getElementsByTagName('filter')

print "char * stringlst = ["

for filter in filters:
	label = filter.getAttribute('inkscape:label')
	menu = filter.getAttribute('inkscape:menu')
	desc = filter.getAttribute('inkscape:menu-tooltip')
	comment = ""

	if "NR" in label:
		comment = '/* TRANSLATORS: NR means non-realistic. See menu Filters > Non realistic shaders */\n'
	
	print comment + "N_(\"" + label + "\"), N_(\"" + menu + "\"), N_(\"" + desc + "\"),"

print "];"
