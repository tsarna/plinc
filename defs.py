#!/usr/pkg/bin/python


early = """
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% EARLY DEFINITIONS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% The odd style here is a result of the fact that it's early in
% interpreter initialization, and we have only the C-implemented
% operators available.  In fact, the point here is to define
% the operators ususally used in defining stuff.


% define the ']' operator.
% the contained ops need to be .doexec so they're run when called from '}'

(]) [
    /counttomark load .doexec
    /array load .doexec
    /astore load .doexec
    /exch load .doexec
    /pop load .doexec
counttomark array astore cvx exch pop def



% define the '}' operator

(}) [
    /.decscan load .doexec
    (]) load .doexec
    /cvx load .doexec
] .doexec def



% define the 'bind' operator
/bind {} dup exec def



% define the '>>' operator

(>>) {
    counttomark dup 2 mod 1 eq {rangecheck} if  %%% rangecheck XXX
    2 idiv dup dict exch 
        { dup 4 2 roll put }
    repeat 
    exch pop
} bind def



% define the 'store' operator

/store {
    exch dup where not
        { currentdict }
    if 3 1 roll exch put
} bind def

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
"""



import string, re, sys

def defs(name, s):
    s = re.sub(r"\%.*\n", " ", s)
    s = re.sub(r"\s+", " ", s)
    s = string.strip(s)
    s = re.sub(r"\s+\(", "(", s)
    s = re.sub(r"\)\s+", ")", s)
    s = re.sub(r"\s+\/", "/", s)
    s = re.sub(r"\s*\{\s*", "{", s)
    s = re.sub(r"\s*\}\s*", "}", s)
    s = re.sub(r"\s*\[\s*", "[", s)
    s = re.sub(r"\s*\]\s*", "]", s)
    s = 'static const char %s_defs[] = "%s\\n";\n' % (name, s)
    return s

sys.stdout.write(defs("early", early))
