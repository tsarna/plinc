#!/usr/pkg/bin/python
#
# $Endicor$


early = """
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% EARLY DEFINITIONS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% The odd style here is a result of the fact that it's early in
% interpreter initialization, and we have only the C-implemented
% operators available.  In fact, the point here is to define them. 

% define the ']' operator.

systemdict (]) [
    /counttomark load
    /array load
    /astore load
    /exch load
    /pop load
counttomark array astore cvx exch pop put

% define the 
%systemdict({)[/mark load/.incscan load].doexec put
%systemdict(})[/.decscan load(])load/cvx load].doexec put
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
