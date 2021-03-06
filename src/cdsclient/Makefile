#++++++++++++++++
# Copyright:    (C) 2008-2017 UDS/CNRS
# License:       GNU General Public License
#.IDENTIFICATION Makefile.in
#.LANGUAGE       Make (makefile script)
#.AUTHOR         Francois Ochsenbein [CDS]
#.ENVIRONMENT    Unix
#.KEYWORDS       
#.VERSION  1.0   03-Oct-1991
#.VERSION  1.1   12-Nov-1997: Bug in "install" corrected !
#.VERSION  1.2   22-Nov-2000: Use Autoconf + configure
#.VERSION  1.3   25-Jan-2007: More files. Use atCDS variable for CDS locale.
#.VERSION  1.4   23-Apr-2012: New catalogs with find_gen1
#.PURPOSE        Template Makefile for Portable Packages
#.COMMENTS      
#	The "install" option in the Makefile copies the executables and 
#	man pages to public directories. Just edit the Makefile, and modify 
#	the PREFIX definition according to your installation.
#	Some alternatives are proposed as comments.
#----------------

SHELL=/bin/sh
# ------------------------------------------
# DIRECTORIES (SHSDIR for Shell Scripts)
#   We assume here a general installation;
#   other choice may e.g. be
#################################################################
# For a system-wide installation, suggested to use /usr/local
# For your own installation, we propose your ~/lib ~/bin ~/man ..
#    i.e.  run ./configure -prefix=$HOME
#################################################################
#PREFIX  = /usr/local
#PREFIX  = $(HOME)
PREFIX  = /home/kirx/current_work/vast/lib
PACKAGE = cdsclient
VERSION = 3.84
DISTRIB = $(PACKAGE)-$(VERSION)
#################################################################

INCDIR  = $(PREFIX)/include
LIBDIR  = $(PREFIX)/lib
BINDIR  = $(PREFIX)/bin
SHSDIR  = $(PREFIX)/bin
MANDIR  = $(PREFIX)/man
###########
# NOTE: The definition below is required only on SOLARIS
#       Comment the following line if required
###########
#LIBSYS  = -lsocket -lnsl
LIBSYS = 

# PROGRAMS	--------------------------------
#	 Possible alternatives are commented out.
CC     = gcc
RANLIB = ranlib
STRIP  = strip
COPY   = cp -p
RM     = rm
AWK    = gawk

# C options	--------------------------------
DEBUG = 
C_OPT = -O
SYS = 
CFLAGS = $(C_OPT) $(DEBUG) $(SYS) -I.

# FILES		--------------------------------
VERFILE = $(PREFIX)/versions
EXPORT_COMPIL=export_compil
EXPORT_INIT=
EXPORT_CVS_OPT=
EXPORT_INSTALL=export_install
BIN = aclient #wwwget
SRC = aclient.c wwwget.c skio.c skclient.c sk.h
INC = sk.h
MAN = sk.3 aclient.1 wwwget.1 aclient.tex
MANTEX=man.tex
SHS =  vizquery catcat lscat findcat find_gen find_gen1 aclient_cgi \
    	sesame simref simbib abibcode abibcode.awk Detex \
	find_cats find_cats.gz find[2a-z]* 
#	`ls find[2a-z]* | fgrep -v old` 
#	find2mass find2massi findcmc14 \
#	findenis findenis1 findenis2 findenis3 \
#	findgsc findgsc1.1 findgsc1.2 findgsc1.3 findgsc2.2 findgsc2.3 \
#	findppmx findppmxl findglimpse findkic \
#	findpmm1 findpmm2 findusnoa1 findusnoa2 findusnob1 findnomad1 \
#	findsdss3 findsdss4 findsdss5 findsdss6 findsdss7 findsdss8 findsdss9 \
#	finducac1 finducac2 finducac3 finducac4 findspm4 findurat1 \
#	findwisep findwise findmc2 find2mass6x findsage findmc_irsf \
#	findtycab findacro \
PYS = catClient.py
SHSatCDS = gsc4sim exo4sim viz4sim oldsim cdspack_x86 lessh
RACatCDS = /ftp/incoming/outils:raccord
OBJ = skclient.o skio.o

#LN_DENIS = findenis1 findenis2 findenis3 findenis-p
#LN_GSC  = findgsc2.2 findgsc2.3
#LN_PMM2 = findpmm findusnoa2
#LN_UCAC = finducac1 


# RULES		--------------------------------
.SUFFIXES: .o .c 
.c.o:
	$(CC) $(CFLAGS) -c $<

# DEPENDENCIES	--------------------------------

all: $(EXPORT_COMPIL)

export_compil: $(BIN) $(MAN)

#init:
#	for f in $(LN_DENIS); do \
#		rm -f $$f; ln findenis $$f; done
#	for f in $(LN_GSC); do \
#		rm -f $$f; ln findgsc  $$f; done
#	for f in $(LN_PMM2); do \
#		rm -f $$f; ln findpmm2 $$f; done
#	for f in $(LN_UCAC); do \
#		rm -f $$f; ln finducac2 $$f; done

aclient: aclient.o $(OBJ)
	$(CC) $@.o $(OBJ) $(LIBSYS) -o $@
	$(STRIP) $@

wwwget: wwwget.c
	$(CC) wwwget.c $(LIBSYS) -o $@
	$(STRIP) $@

doc: sk.tex man.tex aclient.tex
	latex man.tex

install: $(EXPORT_INSTALL) install_shs install_info
	rm -rf *.cache

export_install: install_bin install_man 

install_man: $(MAN)
	if [ -d $(MANDIR) ]; then echo $(MANDIR) exists; \
	else mkdir $(MANDIR); fi 
	for f in $(MAN); do \
	    e=`echo $$f | cut -d. -f2`;  \
	    if [ -d $(MANDIR)/man$$e ] ; then echo $(MANDIR)/man$$e exists ; \
	    else mkdir $(MANDIR)/man$$e ; fi ; \
	    $(COPY) $$f $(MANDIR)/man$$e ;  \
	done

install_h: $(INC)
	test -d $(INCDIR) || mkdir $(INCDIR)
	$(COPY) $(INC) $(INCDIR)

install_bin: $(BIN)
	test -d $(BINDIR) || mkdir $(BINDIR)
	$(COPY) $(BIN) $(BINDIR)

install_shs: Makefile
	test -d $(SHSDIR) || mkdir $(SHSDIR)
	ractools=""; test -r $(RACatCDS) && ractools=$(RACatCDS)/*; \
        test -z "$$ractools" || ls -l $$ractools; \
	cdsutil="$(SHSatCDS)"; test -z "$(atCDS)" && cdsutil=""; \
	cp -p $(SHS) $$cdsutil $$ractools $(SHSDIR); \
	for f in $$ractools; do chmod 755 $(SHSDIR)/`basename $$f`; done

install_info:
	touch $(VERFILE)
	v=`egrep '$(PACKAGE)' $(VERFILE) |wc -l` ; \
	if test $$v -eq 0 ; then \
	  echo "$(PACKAGE):V$(VERSION)">>$(VERFILE); \
	else \
	  sed "s/^\($(PACKAGE)\):V.*/\1:V$(VERSION)/g" $(VERFILE) >version.tmp;\
	  cat version.tmp > $(VERFILE); \
	  rm -f version.tmp; \
	fi
	echo "#===Be sure ~/src/cdspack contains all findxxx"
	
clean:
	rm -f *.o
	rm -f $(BIN)
	rm -f *.log *.aux *.dvi *.lis *.ps
	rm -f core config.status
	rm -rf *.cache

export: $(EXPORT_INIT)

tar: $(EXPORT_INIT)

export_init: /tmp/$(DISTRIB).tar

#	cp -p $(SRC) $(INC) $(MAN) $(SHS) /tmp/$(DISTRIB)
#/tmp/$(DISTRIB).tar: $(SRC) Makefile.in *.c
#	cp -p $(SHS) Makefile.in configure /tmp/$(DISTRIB)
#	cd /tmp/$(DISTRIB) && mv -f vizier/*/*.* . ; rm -rf vizier
/tmp/$(DISTRIB).tar: Makefile.in 
	$(MAKE) clean
	-test -d /tmp/$(DISTRIB) && rm -rf /tmp/$(DISTRIB)
	autoconf
	mkdir /tmp/$(DISTRIB)
	cp -p COPYRIGHT /ftp/pub/sw/COPYING /tmp/$(DISTRIB)
	for f in $(SHS) Makefile.in configure; do cp -p $$f /tmp/$(DISTRIB); \
	    add_license $$f > /tmp/$(DISTRIB)/$$f; done
	test -z "$(atCDS)" || cp -p $(SHSatCDS) /tmp/$(DISTRIB)
	cd /tmp/$(DISTRIB) && \
	for f in $(SRC) $(MAN); do \
		cvs $(EXPORT_CVS_OPT) co vizier/sk/$$f ; done
	cd /tmp/$(DISTRIB) && \
	for f in $(MANTEX) ; do \
		cvs $(EXPORT_CVS_OPT) co vizier/anafile/$$f ; done
	cd /tmp/$(DISTRIB) && rm -rf */CVS
	cd /tmp/$(DISTRIB) && for f in vizier/*/*.*; do \
	    add_license $$f > `basename $$f`; done 
	cd /tmp/$(DISTRIB) && rm -rf vizier
	cd /tmp && tar cf $@ $(DISTRIB)
	rm -rf /tmp/$(DISTRIB)
	gzip -v9 $@

.POSIX:
.SILENT : cvs

cvs:
	d=`echo $(PWD)|sed 's/^.*\///g'`; \
	if test $$d = $(PACKAGE); then \
	   d=`cvs diff configure*|wc -l` ; \
	   if test $$d -eq 0; then \
	      v=`echo $(VERSION)|sed 's/\./_/g'` ; echo "version: V$$v"; \
	      cvs tag V$$v . ; \
	   else \
	      echo "****you must commit 'configure*' before" ; \
	   fi \
   	fi
