#!/usr/pkg/bin/python

import string

l = {}

for c in range(0, 256):
    l[c] = '      0'
    
for c in string.whitespace:
    l[ord(c)] = '     WS'
    
for c in string.digits:
    l[ord(c)] = '%s|SN|DG' % c

for c in "+-":
    l[ord(c)] = '     SN'
    
for c in string.lowercase:
    l[ord(c)] = '%4d|DG' % (ord(c) - ord('a') + 11)
    
for c in string.uppercase:
    l[ord(c)] = '%4d|DG' % (ord(c) - ord('A') + 11)
    
for c in "{}[]<>()%/":
    l[ord(c)] = '     SP'


vv = []
for c in range(0, 256, 8):
    s = ""
    v = [] 
    for x in range(c, c + 8):
        v.append(l[x])
    vv.append("    " + string.join(v, ', '))

vv = string.join(vv, ",\n")
print "static const short _toktab[256] = {\n%s\n};" % vv
