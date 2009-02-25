#!/usr/pkg/bin/python


early = r"""
% define the 'bind' operator

/bind {} dup exec def % XXX

% define the '>>' operator

(>>) {
    counttomark dup 2 mod 1 eq {rangecheck} if  %%% rangecheck XXX
    2 idiv dup dict exch 
        { dup 4 2 roll put }
    repeat 
    exch pop
} bind def

/$error <<
    /newerror false
    /errorname null
    /command null
    /errorinfo null
    /ostack null
    /estack null
    /dstack null
    /recordstacks true
    /binary false
>> def

errordict /handleerror {
    $error begin (Error: ) print errorname =only end ( in ) print command =only
    recordstacks {
        (\nOperand Stack:\n) print ostack {(   ) print =only } forall
        (\nExecution Stack:\n) print estack {(   ) print =only } forall
        (\nDictionary Stack:\n) print dstack {(   ) print =only } forall
    } if (\n) print
} bind put

/handleerror {errordict /handleerror get exec} bind def

errordict /stackunderflow {
    $error dup /newerror true put /errorname /stackunderflow put stop
} bind put

% printing operators

/==array {
        dup xcheck {({)} {([)} ifelse print
        dup dup 0 exch length 1 sub getinterval {==only ( ) print} forall
        dup dup length 1 sub get ==only
        xcheck {(})} {(])} ifelse print
} bind def

/==only {dup type /arraytype eq {==array} {==one} ifelse} bind def
/== {==only (\n) print} bind def
/stack {0 1 count 3 sub {index =} for} bind def
%/pstack {0 1 count 3 sub {index ==} for} bind def

% interactive-related stuff
/prompt {
    (PL) print count 0 ne {(<) print count =only} if (> ) print flush
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
