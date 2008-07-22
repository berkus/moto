dnl -------------------------------------------------------------------
dnl
dnl Local autoconf macros 
dnl
dnl $RCSfile: aclocal.m4,v $
dnl $Revision: 1.28 $
dnl $Author: dhakim $
dnl $Date: 2003/03/10 21:45:23 $
dnl
dnl -------------------------------------------------------------------
dnl
dnl Sets the root directory for the build
dnl
dnl Usage: AC_FLAGS_C
dnl
AC_DEFUN(AC_FLAGS_C,
[AC_MSG_CHECKING([flags for C compiler])
ac_cv_flags_CPP='-I. -I@ROOT_DIR@ -DHAVE_RCONFIG_H'
ac_cv_flags_CPP_APACHE='-I@APACHE_INCLUDE_DIR@'
ac_cv_flags_CPP_MOTO='-I@ROOT_DIR@/include'
ac_cv_flags_DEFS=''
ac_cv_flags_C='-g -O2 -Wall'
AC_MSG_RESULT(ok)
])dnl
dnl
dnl -------------------------------------------------------------------
dnl
dnl Check that the cwd is a directory that ends with the path in $REQ
dnl
dnl Usage: AC_PATH_END_CHECK(PROGRAM_NAME, END_DIR)
dnl
AC_DEFUN(AC_PATH_END_CHECK,
[AC_MSG_CHECKING([configuration directory])
REQ=$2
# this mess escapes slashes
SREQ=`echo $REQ | sed -e 's/\//\\\\\//g'`
CWD=`pwd`
CHK=`echo $CWD | sed -e "s/\/$SREQ\$//"`
if test "$CWD" != "$CHK"; then
	ac_cv_path_END_CHECK=$CHK	
	AC_MSG_RESULT(ok)
else	
	PATH=`echo $1 | sed -e "s/configure$//"`	
	AC_MSG_RESULT([no])	
	$PATH/config.err dir $REQ
	exit 1
fi
])dnl
dnl
dnl -------------------------------------------------------------------
dnl
dnl Sets the root directory for the build
dnl
dnl Usage: AC_PATH_SET_ROOT(PROGRAM_NAME)
dnl
AC_DEFUN(AC_PATH_SET_ROOT,
[AC_MSG_CHECKING([build root directory])
ac_cv_path_ROOT=`pwd`
AC_MSG_RESULT(ok)
])dnl
dnl
dnl -------------------------------------------------------------------
dnl
dnl Check for a complete Apache installation
dnl
dnl Usage: AC_APACHE()
dnl
dnl
AC_DEFUN(AC_APACHE,
[AC_MSG_CHECKING([for Apache module support])
APACHE_ROOT_DIR=
APACHE_BIN_DIR=
APACHE_INCLUDE_DIR=
APACHE_LIBEXEC_DIR=
AC_ARG_WITH(apache,[  --with-apache[=PREFIX]  include apache module support 
                          requires apxs is in your path or apache was 
                          installed with --prefix PREFIX],

	if test "$withval" != "yes"; then
		APACHE_ROOT_DIR=$withval;
		APACHE_BIN_DIR=$APACHE_ROOT_DIR/bin;
		APACHE_INCLUDE_DIR=$APACHE_ROOT_DIR/include;
		APACHE_LIBEXEC_DIR=$APACHE_ROOT_DIR/libexec;
		APXS=$APACHE_BIN_DIR/apxs;
	else [
		AC_PATH_PROG(APXS,apxs)
		if test -z "$ac_cv_path_APXS"; then
       			$CONFIG_ERR apxs
       			exit 1
		fi
		changequote(<<, >>)dnl
		<<
	        APACHE_ROOT_DIR=`grep '^my $CFG_PREFIX' $ac_cv_path_APXS | \
			sed -e 's/.*q[(]//' -e "s/.*[ ]'//" -e 's/[)];.*//' -e "s/';.*//" `;
       		APACHE_BIN_DIR=`grep '^my $CFG_SBINDIR' $ac_cv_path_APXS | \
                        sed -e 's/.*q[(]//' -e "s/.*[ ]'//" -e 's/[)];.*//' -e "s/';.*//" `;
                APACHE_INCLUDE_DIR=`grep '^my $CFG_INCLUDEDIR' $ac_cv_path_APXS | \
                        sed -e 's/.*q[(]//' -e "s/.*[ ]'//" -e 's/[)];.*//' -e "s/';.*//" `;
                APACHE_LIBEXEC_DIR=`grep '^my $CFG_LIBEXECDIR' $ac_cv_path_APXS | \
                        sed -e 's/.*q[(]//' -e "s/.*[ ]'//" -e 's/[)];.*//' -e "s/';.*//" `;
		>>
		changequote([, ])dnl
	] fi

	if (test -d "$APACHE_ROOT_DIR" || test "$withval" != "yes") &&
		test -d "$APACHE_BIN_DIR" &&
		test -d "$APACHE_INCLUDE_DIR" &&
		test -d "$APACHE_LIBEXEC_DIR" &&
		test -f "$APACHE_INCLUDE_DIR/httpd.h" &&
		test -f "$APACHE_INCLUDE_DIR/http_config.h" &&
		test -f "$APACHE_INCLUDE_DIR/http_protocol.h" &&
		test -f "$APACHE_INCLUDE_DIR/http_log.h" &&
		test -f "$APACHE_INCLUDE_DIR/util_script.h" &&
		test -f "$APACHE_INCLUDE_DIR/http_main.h" &&
		test -f "$APACHE_INCLUDE_DIR/http_request.h" &&
		(test -f "$APACHE_BIN_DIR/apxs"||test -f "$ac_cv_path_APXS"); then
		AC_MSG_RESULT(ok)
  	else
		AC_MSG_ERROR([

********************************************************************************
   Can not include Apache module support
   This directory does not contain a complete Apache installation:      
   Directory: \`$APACHE_ROOT_DIR' 
********************************************************************************
		])
	fi
	,
	AC_MSG_RESULT(no)
)
AC_SUBST(APACHE_ROOT_DIR)
AC_SUBST(APACHE_BIN_DIR)
AC_SUBST(APACHE_INCLUDE_DIR)
AC_SUBST(APACHE_LIBEXEC_DIR)
AC_SUBST(APXS)
AC_DEFINE_UNQUOTED(APACHE_ROOT_DIR, "$APACHE_ROOT_DIR")
])dnl

dnl
dnl -------------------------------------------------------------------
dnl
dnl Check for a complete MySQL installation
dnl
dnl Usage: AC_MYSQL()
dnl
dnl
AC_DEFUN(AC_MYSQL,
[AC_MSG_CHECKING([for MySQL module support])
MYSQL_CONFIG=
MYSQL_INCLUDE_DIR=
MYSQL_LIBRARY_DIR=
AC_ARG_WITH(mysql,[  --with-mysql[=PREFIX]   include MySQL module support
                          requires an installed MySQL database at PREFIX
                          or one that may be located based on the default
                          OS layout. When in doubt try setting prefix to
                          /usr/local],

	if test "$withval" != "yes"; then
		MYSQL_INCLUDE_DIR=$withval/include/mysql
		MYSQL_LIBRARY_DIR=$withval/lib/mysql
      MYSQL_CONFIG=$withval/bin/mysql_config
	else [ 
      MYSQL_CONFIG=`which mysql_config`
      MYSQL_INCLUDE_DIR=`$MYSQL_CONFIG --cflags|sed -e "s/-I'\(.*\)'/\1/g"`
      MYSQL_LIBRARY_DIR=`$MYSQL_CONFIG --libs|sed -e "s/-L'\(.*\)' .*/\1/g"`

      if test -d "$MYSQL_LIBRARY_DIR" && test -d "$MYSQL_INCLUDE_DIR" ; then
         echo -n ""
      else
		   AC_CANONICAL_HOST
         case "$host" in
            *linux*)
			      MYSQL_INCLUDE_DIR=/usr/include/mysql ;
               if test -f /usr/lib/mysql/libmysqlclient.a ; then
				      MYSQL_LIBRARY_DIR=/usr/lib/mysql ;
			      fi ;
			      if test -f /usr/lib/libmysqlclient.a ; then
				      MYSQL_LIBRARY_DIR=/usr/lib ;
			      fi
            ;;
		      *apple*)
               MYSQL_ROOT_DIR=/usr/local/mysql ;
			      if test -d /usr/local/mysql/include/mysql ; then
                  MYSQL_INCLUDE_DIR=/usr/local/mysql/include/mysql ;
			      else
				      MYSQL_INCLUDE_DIR=/usr/local/mysql/include ;
			      fi ;
			      if test -d /usr/local/mysql/lib/mysql ; then
				      MYSQL_LIBRARY_DIR=/usr/local/mysql/lib/mysql ;
			      else 
                  MYSQL_LIBRARY_DIR=/usr/local/mysql/lib ;
			      fi ;
         esac
      fi
	] fi

   if test -d "$MYSQL_INCLUDE_DIR" && 
		test -f "$MYSQL_INCLUDE_DIR/mysql.h" &&
		test -d "$MYSQL_LIBRARY_DIR" &&
		test -f "$MYSQL_LIBRARY_DIR/libmysqlclient.a" ; then

		AC_MSG_RESULT(ok)
  else
        AC_MSG_ERROR([
********************************************************************************
   Can not include MySQL module support
   This directory does not contain a complete MySQL installation:
   Directory: \`$MYSQL_INCLUDE_DIR $MYSQL_LIBRARY_DIR'
********************************************************************************
        ])
        fi
        ,
        AC_MSG_RESULT(no)
)
AC_SUBST(MYSQL_CONFIG)
AC_SUBST(MYSQL_BIN_DIR)
AC_SUBST(MYSQL_INCLUDE_DIR)
AC_SUBST(MYSQL_LIBRARY_DIR)
])dnl

dnl
dnl -------------------------------------------------------------------
dnl
dnl Check for a complete PGSQL installation
dnl
dnl Usage: AC_PGSQL()
dnl
dnl
AC_DEFUN(AC_PGSQL,
[AC_MSG_CHECKING([for PGSQL module support])
PGSQL_CONFIG=
PGSQL_INCLUDE_DIR=
PGSQL_LIBRARY_DIR=
AC_ARG_WITH(pgsql,[  --with-pgsql[=PREFIX]   include Postgres module support
                          requires an installed Postgres database at PREFIX
                          or one that may be located based on the default
                          OS layout. When in doubt try setting prefix to
                          /usr/local],

   if test "$withval" != "yes" ; then
		PGSQL_INCLUDE_DIR=$withval/include
		PGSQL_LIBRARY_DIR=$withval/lib
      PGSQL_CONFIG=$withval/bin/pg_config
   else  
      PGSQL_CONFIG=`which pg_config`
      PGSQL_INCLUDE_DIR=`$PGSQL_CONFIG --includedir`
      PGSQL_LIBRARY_DIR=`$PGSQL_CONFIG --libdir`

      if test -d "$PGSQL_LIBRARY_DIR" && test -d "$PGSQL_INCLUDE_DIR" ; then
         echo -n ""
      else [
		   AC_CANONICAL_HOST
         case "$host" in
            *linux*)
			      PGSQL_INCLUDE_DIR=/usr/include ;
               if test -f /usr/lib/pgsql/libpq.a ; then
				      PGSQL_LIBRARY_DIR=/usr/lib/pgsql ;
			      fi ;
			      if test -f /usr/lib/libpq.a ; then
				      PGSQL_LIBRARY_DIR=/usr/lib ;
			      fi
               ;;
		      *apple*)
			      PGSQL_INCLUDE_DIR=/usr/local/pgsql/include ;
			      PGSQL_LIBRARY_DIR=/usr/local/pgsql/lib ;
         esac
      fi
	]fi

   if test -d "$PGSQL_INCLUDE_DIR" && 
		test -f "$PGSQL_INCLUDE_DIR/libpq-fe.h" &&
		test -d "$PGSQL_LIBRARY_DIR" &&
		test -f "$PGSQL_LIBRARY_DIR/libpq.a" ; then

		AC_MSG_RESULT(ok)
  else
        AC_MSG_ERROR([
********************************************************************************
   Can not include PGSQL module support
   This directory does not contain a complete Postgres installation:
   Directory: \`$PGSQL_INCLUDE_DIR $PGSQL_LIBRARY_DIR'
********************************************************************************
        ])
        fi
        ,
        AC_MSG_RESULT(no)
)
AC_SUBST(PGSQL_CONFIG)
AC_SUBST(PGSQL_BIN_DIR)
AC_SUBST(PGSQL_INCLUDE_DIR)
AC_SUBST(PGSQL_LIBRARY_DIR)
])dnl

dnl -------------------------------------------------------------------
dnl
dnl Check whether shared memory support has been requested
dnl
dnl Usage: AC_CHECK_SHARED_MALLOC()
dnl
dnl
AC_DEFUN(AC_CHECK_SHARED_MALLOC,
[AC_MSG_CHECKING([if shared memory feature enabled])
AC_ARG_ENABLE(shared_memory, [  --enable-shared-memory   include shared memory support], 
	AC_DEFINE(SHARED_MALLOC)	
	AC_MSG_RESULT(ok),
	AC_MSG_RESULT(no)
)
])dnl

dnl -------------------------------------------------------------------
dnl
dnl Check if modules should be linked with libgcc
dnl
dnl Usage: AC_CHECK_GCCLIB()
dnl
dnl
AC_DEFUN(AC_CHECK_GCCLIB,
[AC_MSG_CHECKING([if modules should be linked with libgcc])
AC_ARG_ENABLE(gcclib, [  --enable-gcclib         link modules with libgcc
                          this is only known to be necessary on certain solaris
                          systems but it probably doesn't hurt anywhere  ],[
        changequote(<<, >>)dnl
        GCCLDPATH='-L'`${CC} --print-libgcc-file-name | sed -e 's;/[^/]*$;;'`
        GCCLIB='-lgcc'
        changequote([, ])dnl
        AC_SUBST(GCCLDPATH)
	AC_SUBST(GCCLIB)
        AC_MSG_RESULT(yes)],
        AC_MSG_RESULT(no)
)
])dnl

dnl -------------------------------------------------------------------
dnl
dnl Check for a layout
dnl
dnl Usage: AC_LAYOUT()
dnl
dnl
AC_DEFUN(AC_LAYOUT,
[AC_MSG_CHECKING([for layout])
INST_ROOT_DIR=
INST_BIN_DIR=
INST_LIB_DIR=
INST_INC_DIR=
INST_MX_DIR=
INST_MOD_DIR=
INST_LAYOUT=UNIFIED
AC_ARG_WITH(layout,[  --with-layout=LAYOUT    specify the layout for the installation
                          currently accepted values are REDHAT and 
                          UNIFIED (the default)],

	if (test "$withval" = "UNIFIED" || test "$withval" = "REDHAT"); then
		INST_LAYOUT=$withval
	else
        	AC_MSG_ERROR([

********************************************************************************
   Not a recognized layout option \`$withval'
********************************************************************************
      		])
        fi
	,
	AC_MSG_RESULT(no)
)

if test "$prefix" != "NONE"; then
        INST_ROOT_DIR=$prefix
fi

if test "$INST_LAYOUT" = "REDHAT"; then
	if test "$prefix" = "NONE"; then
		INST_ROOT_DIR=/usr
	fi
        INST_LAYOUT=REDHAT
        INST_BIN_DIR=$INST_ROOT_DIR/bin
        INST_LIB_DIR=$INST_ROOT_DIR/lib/moto
        INST_INC_DIR=$INST_ROOT_DIR/include/moto
        INST_MX_DIR=$INST_ROOT_DIR/lib/moto/mx
        INST_MOD_DIR=$INST_ROOT_DIR/lib/apache
elif test "$INST_LAYOUT" = "UNIFIED"; then
        if test "$prefix" = "NONE"; then
                INST_ROOT_DIR=/usr/local/moto
        fi
        INST_LAYOUT=UNIFIED
        INST_BIN_DIR=$INST_ROOT_DIR/bin
        INST_LIB_DIR=$INST_ROOT_DIR/lib
        INST_INC_DIR=$INST_ROOT_DIR/include
        INST_MX_DIR=$INST_ROOT_DIR/mx
        INST_MOD_DIR=$INST_ROOT_DIR/mod
fi

AC_SUBST(INST_ROOT_DIR)
AC_SUBST(INST_BIN_DIR)
AC_SUBST(INST_LIB_DIR)
AC_SUBST(INST_INC_DIR)
AC_SUBST(INST_MX_DIR)
AC_SUBST(INST_MOD_DIR)
])dnl
