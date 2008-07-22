#!/bin/sh
USES=`../bin/moto -X ../mx -u $1.moto`
LIBRARYPATHS="`../bin/mxtool  -X ../mx -e -L $USES | sed -e s/^/-L/g`"
LIBRARIES="`../bin/mxtool -X ../mx -e -l $USES`"
INCLUDEPATHS="-I../include `../bin/mxtool -X ../mx -e -I $USES | sed -e s/^/-I/g`"
ARCHIVES="`../bin/mxtool -X ../mx -e -a $USES` ../lib/libcodex.a"

$2 -g ${INCLUDEPATHS} ${LIBRARYPATHS} -D__MAIN__=main -DSHARED_MALLOC $1.c ${ARCHIVES} ${LIBRARIES}
