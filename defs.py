#!/usr/pkg/bin/python
#
# $Endicor: defs.py,v 1.1 1999/01/20 01:36:21 tsarna Exp $


early = """
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% EARLY DEFINITIONS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% The odd style here is a result of the fact that it's early in
% interpreter initialization, and we have only the C-implemented
% operators available.  In fact, the point here is to define them. 

% define the ']' operator.

systemdict (]) [
    /counttomark load .doexec
    /array load .doexec
    /astore load .doexec
    /exch load .doexec
    /pop load .doexec
counttomark array astore cvx exch pop put

% define the '}' operator

systemdict (}) [
    /.decscan load .doexec
    (]) load .doexec
    /cvx load .doexec
] .doexec put

%systemdict/def{currentdict 3 1 roll put}put
%/store{exch dup where {3 1 roll exch put} {exch undefined} ifelse}bind def

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
"""



import string, re, sys

def defs(name, s):
    s = re.sub(r"\%.*\n", " ", s)
    s = re.sub(r"\s+", " ", s)
    s = string.strip(s)
    s = re.sub(r"\s+\(", "(", s)
    s = re.sub(r"\s+\/", "/", s)
    s = re.sub(r"\s+\{", "{", s)
    s = re.sub(r"\s+\[", "[", s)
    s = 'static char %s_defs[] = "%s\\n";\n' % (name, s)
    return s

sys.stdout.write(defs("early", early))
