# Generated automatically from Makefile.in by configure.
#======================================================================
#
# $RCSfile: Makefile.in,v $
# $Revision: 1.15 $
# $Author: dhakim $
# $Date: 2002/09/13 23:15:49 $
#
# Makefile for a moto extension 
#
#======================================================================
# Standard Variables
#

include ../../make.vars

#----------------------------------------------------------------------
# Variables

LIBRARY = .library
CFILES = $(wildcard *.c)
MXCFILES = $(wildcard *.mx.c)
IFILES = $(wildcard *.i)
HEADERS = $(wildcard *.h)
CLEAN_FILES = ${MXCFILES} lib*.so* lib*.a *.o mx.mk *.mx.* core

MXC = ${BIN_DIR}/mxc

#----------------------------------------------------------------------
# Rules

all: ${LIBRARY}

${LIBRARY}: ${CFILES} ${IFILES}
	${MXC} .
	
clean:
	rm -f ${CLEAN_FILES}	

distclean:
	${MAKE} clean
	rm -f make.vars		

maintainer-clean:
	${MAKE} distclean

install: all
	@${MKDIR} `pwd | sed -e "s;${MX_DIR};;" -e "s;^;${INST_MX_DIR};"`
	@${INSTALL} `pwd | sed -e "s;${MX_DIR};;" -e "s;/;_;g" -e "s;^_;libmx_;" -e "s;$$;.so.1.0;"` \
		`pwd | sed -e "s;${MX_DIR};;" -e "s;^;${INST_MX_DIR};"`
	@${INSTALL} `pwd | sed -e "s;${MX_DIR};;" -e "s;/;_;g" -e "s;^_;libmx_;" -e "s;$$;.a;"` \
		`pwd | sed -e "s;${MX_DIR};;" -e "s;^;${INST_MX_DIR};"`
	@${INSTALL} interface.mx.html \
		`pwd | sed -e "s;${MX_DIR};;" -e "s;^;${INST_MX_DIR};"`
	@${RANLIB} `pwd | sed -e "s;${MX_DIR};;" -e "s;^;${INST_MX_DIR};"`/`pwd | sed -e "s;${MX_DIR};;" -e "s;/;_;g" -e "s;^_;libmx_;" -e "s;$$;.a;"`  
	@cp *.h `pwd | sed -e "s;${MX_DIR};;" -e "s;^;${INST_MX_DIR};"`  
	@cd `pwd | sed -e "s;${MX_DIR};;" -e "s;^;${INST_MX_DIR};"` ; \
	rm headers.mx.h ; \
	if [ ! -f `pwd | sed -e "s;${INST_MX_DIR};;" -e "s;/;_;g" -e "s;^_;libmx_;" -e "s;$$;.so.1;"` ]; then \
		ln -s `pwd | sed -e "s;${INST_MX_DIR};;" -e "s;/;_;g" -e "s;^_;libmx_;" -e "s;$$;.so.1.0;"` \
			`pwd | sed -e "s;${INST_MX_DIR};;" -e "s;/;_;g" -e "s;^_;libmx_;" -e "s;$$;.so.1;"` ; \
	fi; \
	if [ ! -f `pwd | sed -e "s;${INST_MX_DIR};;" -e "s;/;_;g" -e "s;^_;libmx_;" -e "s;$$;.so;"` ]; then \
		ln -s `pwd | sed -e "s;${INST_MX_DIR};;" -e "s;/;_;g" -e "s;^_;libmx_;" -e "s;$$;.so.1;"` \
			`pwd | sed -e "s;${INST_MX_DIR};;" -e "s;/;_;g" -e "s;^_;libmx_;" -e "s;$$;.so;"` ; \
	fi;

#======================================================================
# end
#======================================================================

