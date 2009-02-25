LIB=plinc

COPTS=-Wall -g
CFLAGS=-g
CPPFLAGS=-I${.CURDIR} -I. -DDEBUG -DWITH_REAL -DWITH_MATRIX \
	-DWITH_SIMPLE_CTM -DWITH_DEFAULT_DEFAULT_MATRIX
NOPROFILE=1
NOPIC=1

INCS=	file.h fpmath.h heap.h interp.h stack.h token.h types.h version.h
FLTSRCS=flt_hex.c flt_pstr.c flt_str.c
SRCS=	arith.c array.c cvt.c dict.c edit.c exec.c file.c heap.c \
	interp.c loop.c matrix.c polymorph.c print.c real.c relate.c \
	stack.c string.c token.c type.c vm.c main.c stdio.c \
	${FLTSRCS}
PYTHON=	defs.py

all: main lib${LIB}.a

defs.h : defs.py
	python ${.CURDIR}/defs.py >defs.h

main : lib${LIB}.a
	${CC} -o x -L. -l${LIB} -lm

testnames : testnames.c lib${LIB}.a
	${CC} ${CPPFLAGS} -o testnames ${.CURDIR}/testnames.c -L. -l${LIB}

ci:
	(cd ${.CURDIR}; ci -l ${SRCS} ${INCS} ${PYTHON})

size:
	@(cd ${.CURDIR}; cat *.[ch] *.py) | grep -v ^$$ | wc -l | tail -1 | \
		awk '{printf("%s lines\n", $$1);}'
	@size -t lib${LIB}.a | tail -1 | \
		awk '{printf("%s K text\n%d bytes data\n%d bytes bss\n", \
		$$1/1024.0, $$2, $$3);}'

.include <bsd.lib.mk>


interp.c : defs.h
