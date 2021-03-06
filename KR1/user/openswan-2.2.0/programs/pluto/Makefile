# Pluto Makefile
# Copyright (C) 1997 Angelos D. Keromytis.
# Copyright (C) 1998-2001  D. Hugh Redelmeier
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# RCSID $Id: Makefile,v 1.1 2004/12/24 07:17:32 rupert Exp $

# All of the USE_ and HAVE_ variables are controlled from openswan/Makefile.inc


ifeq ($(USE_LDAP),true)
# Everyone (should) be using LDAPv3, however LDAP_VERSION=2 is an option
# if you require LDAPv2 
LDAP_VERSION=3
endif

# Use SMARTCARDS
ifeq ($(USE_SMARTCARD),true)
SMARTCARD=1
endif


ifeq ($(USE_XAUTH),true)
XAUTH=1
# compile with PAM support will increase the size of the distribution
# and thus it may not be the best solution for embeded systems. XAUTH
# will use MD5/DES crypt() lib and a password file by default.
ifeq ($(USE_XAUTHPAM),true)
XAUTH_USEPAM=1
endif
endif

# relative path to top directory of Openswan source
# Note: referenced in ${OPENSWANSRCDIR}/Makefile.inc
OPENSWANSRCDIR=../..

include ${OPENSWANSRCDIR}/Makefile.inc

FMANDIR=$(MANTREE)/man5
PMANDIR=$(MANTREE)/man8

# -O on Linux makes gcc coredump when compiling sha1.c
# -Wundef is nice but RHL5.2 compiler doesn't support it
CFLAGS += -g -Wall -W -Wmissing-prototypes -Wpointer-arith -Wbad-function-cast \
	-Wcast-qual -Wmissing-declarations -Wwrite-strings \
	-Wstrict-prototypes $(INCGMP)# -Wundef

# where to find klips headers and Openswan headers
# and 2.6 kernel's <rtnetlink.h> and <xfrm.h>
HDRDIRS = -I${OPENSWANSRCDIR}/programs/pluto/linux26 -I${OPENSWANSRCDIR}/include -I$(KLIPSINC) 

# On non-LINUX systems, these one of these may be needed (see endian.h)
# BYTE_ORDER = -DBIG_ENDIAN=4321 -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=BIG_ENDIAN
# BYTE_ORDER = -DBIG_ENDIAN=4321 -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN

# -DKLIPS enables interface to Kernel LINUX IPsec code
# -DDEBUG enables debugging code, allowing for debugging output
#    (note that output must also be selected at runtime, so it is
#    reasonable to always define this)
# -DX509_VERSION is used to transfer the X.509 version number to Pluto
# -DPLUTO_SENDS_VENDORID enables pluto to send out a VendorID payload.
#    this can be used by remote nodes to work around faults (bugs),
#    but is most useful to humans who are debugging things.
# -DGCC_LINT uses gcc-specific declarations to improve compile-time
#    diagnostics.
# -DLEAK_DETECTIVE enables crude code to find memory allocation leaks.
# -DOLD_RESOLVER.  At some point, the resolver interface changed.
#    This macro enables Pluto support for the old interface.
#    It is automatically defined, based on the value of the <resolver.h>
#    macro __RES.  We don't know the correct threshold, so you may
#    find that you must manually define this.  If so, please inform
#    us so that we can refine the threshold.

# The following are best left undefined -- each can be overridden at runtime
# if need be.
# -DPORT=n sets the default UDP port for IKE messages (otherwise 500)
# -DSHARED_SECRETS_FILE=string overrides /etc/ipsec.secrets as the
#    default name of the file containing secrets used to authenticate other
#    IKE daemons.  In the Makefile, two levels of quoting are needed:
#    -DSHARED_SECRETS_FILE='"/etc/ipsec.secrets"'
# -DDEFAULT_CTLBASE=string overrides /var/run/pluto as default directory
#    and basename for pluto's lockfile (.pid) and control socket (.ctl).
#    Double quoting may be needed.
# -DAPPLY_CRISCO enables some workarounds for interop with broken Cisco
#    routers.  Currently, it disables cookie checks on DPD packets


BINNAMEPLUTO = pluto
BINNAMEWHACK = whack
BINNAMEADNS = _pluto_adns

ifeq ($(USE_LWRES),true)
LWRESDEF=-DUSE_LWRES
USE_ADNS=false
BINNAMEADNSIFNEEDED=
else
USE_ADNS=true
BINNAMEADNSIFNEEDED=$(BINNAMEADNS)
endif

IPSECPOLICY_DIST_SRCS=rcv_info.c
ifeq ($(USE_IPSECPOLICY),true)
IPSECPOLICY_SRCS=${IPSECPOLICY_DIST_SRCS}
IPSECPOLICY_DEFINES=-DIPSECPOLICY
IPSECPOLICY_LIBS=$(POLICYLIB)
IPSECPOLICY_OBJS=rcv_info.o
endif

ifeq ($(USE_VENDORID),true)
VENDORID=-DPLUTO_SENDS_VENDORID
endif

ifeq ($(USE_KEYRR),true)
KEYRR_DEFINES=-DUSE_KEYRR
endif

KERNEL26_DIST_SRCS=kernel_netlink.c kernel_netlink.h
ifeq ($(USE_KERNEL26),true)
KERNEL26_DEFS=-DKERNEL26_SUPPORT -DKERNEL26_HAS_KAME_DUPLICATES 
KERNEL26_SRCS=${KERNEL26_DIST_SRCS}
KERNEL26_OBJS=kernel_netlink.o
endif

# the files are defined here so that TAGS: can catch them.
#
X509_DIST_OBJS=ac.o asn1.o oid.o pem.o pgp.o pkcs.o x509.o certs.o md2.o ocsp.o x509keys.o
X509_DIST_SRCS=oid.txt oid.pl ${X509_DIST_OBJS:.o=.c}
X509_DIST_SRCS+=ac.h asn1.h certs.h fetch.h oid.h pem.h pgp.h pkcs.h x509.h
HAVE_THREADS_DIST_OBJS=fetch.o 
HAVE_THREADS_DIST_SRCS=${HAVE_THREADS_DIST_OBJS:.o=.c}
SMARTCARD_DIST_OBJS=smartcard.o
SMARTCARD_DIST_SRCS=smartcard.c smartcard.h 
X509_OBJS=${X509_DIST_OBJS}
X509_SRCS=${X509_DIST_SRCS}
X509_DEFS=-DX509 
X509_DEFS=-DX509_VERSION=\"${X509_VERSION}\"

# dynamic LDAP CRL fetching requires OpenLDAP library
ifeq ($(USE_LDAP),true)
X509_LIBS+= -lldap
ifdef LDAP_VERSION
X509_DEFS+= -DLDAP_VER=$(LDAP_VERSION)
endif
endif

ifeq ($(HAVE_THREADS),true)
HAVE_THREADS_DEFS=-DHAVE_THREADS
HAVE_THREADS_OBJS=${HAVE_THREADS_DIST_OBJS}
HAVE_THREADS_SRCS=${HAVE_THREADS_DIST_SRCS}
HAVE_THREADS_LIBS=-lpthread
endif

ifeq ($(USE_SMARTCARD),true)
# smartcard functions are compiled in whether or not we have -lopensc,
# but ifndef SMARTCARD, they are dummy functions.
SMARTCARD_DEFS=-DSMARTCARD
SMARTCARD_SRCS=${SMARTCARD_DIST_SRCS}
SMARTCARD_OBJS=${SMARTCARD_DIST_OBJS}
SMARTCARD_LIBS=-lopensc
endif

XAUTH_DIST_SRCS=xauth.c xauth.h
XAUTH_DIST_OBJS=xauth.o
ifeq ($(USE_XAUTH),true)
# This compile option activates xauth code and modecfg needed by xauth
XAUTH_DEFS=-DXAUTH -DMODECFG
XAUTH_OBJS=${XAUTH_DIST_OBJS}
XAUTH_SRCS=${XAUTH_DIST_SRCS}
# if we use pam for password checking then add it too
ifeq ($(USE_XAUTHPAM),true)
XAUTHPAM_DEFS=-DXAUTH_USEPAM
XAUTHPAM_LIBS=-lpam
endif
endif

ifeq ($(USE_NAT_TRAVERSAL),true)
NAT_DEFS=-DNAT_TRAVERSAL -DVIRTUAL_IP 
endif

ifeq ($(USE_NAT_TRAVERSAL_TRANSPORT_MODE),true)
NAT_DEFS+=-DI_KNOW_TRANSPORT_MODE_HAS_SECURITY_CONCERN_BUT_I_WANT_IT
endif

ifeq ($(USE_LIBCURL),true)
# This compile option activates dynamic URL fetching
# with libcurl in the source code
CURL_DEFS=-DLIBCURL
CURL_LIBS=-lcurl
endif



# End of configuration coping options.

DEFINES = $(EXTRA_DEFINES) \
	$(IPSECPOLICY_DEFINES) ${VENDORID} \
	$(KEYRR_DEFINES) \
	$(BYTE_ORDER) \
	$(LWRESDEF) \
	$(KERNEL26_DEFS) \
	$(X509_DEFS) \
	$(HAVE_THREADS_DEFS) \
	$(SMARTCARD_DEFS) \
	-DPLUTO \
	-DKLIPS \
	-DDEBUG \
	-DGCC_LINT \
	-DIKE_ALG -DKERNEL_ALG -DDB_CONTEXT \
	${XAUTH_DEFS} ${XAUTHPAM_DEFS} \
	${NAT_DEFS} ${CURL_DEFS}\
	# -DLEAK_DETECTIVE

CPPFLAGS = $(HDRDIRS) $(DEFINES) \
	-DSHARED_SECRETS_FILE=\"${FINALCONFDIR}/ipsec.secrets\" \
	-DPOLICYGROUPSDIR=\"${FINALCONFDDIR}/policies\" \
	-DPERPEERLOGDIR=\"${FINALLOGDIR}/pluto/peer\"

ALLFLAGS = $(CPPFLAGS) $(CFLAGS)

# libefence is a free memory allocation debugger
# Solaris 2 needs -lsocket -lnsl
LIBSPLUTO =$(OBJSGCRYPT) $(LIBDESLITE) $(LIBAES) $(OPENSWANLIB) 
LIBSPLUTO+=$(IPSECPOLICY_LIBS) $(X509_LIBS) $(SMARTCARD_LIBS) 
LIBSPLUTO+=$(HAVE_THREADS_LIBS) ${XAUTHPAM_LIBS}
LIBSPLUTO+=${CURL_LIBS} 
LIBSPLUTO+= -lgmp -lresolv # -lefence

ifneq ($(LD_LIBRARY_PATH),)
LDFLAGS=-L$(LD_LIBRARY_PATH)
endif
LDFLAGS += -L$(ROOTDIR)/lib/libgmp
LIBSADNS = $(OPENSWANLIB)
LIBSADNS += -lresolv # -lefence

# Solaris needs -lsocket -lnsl
LIBSWHACK = ${OPENSWANLIB}


RM = /bin/rm
RMFLAGS = -f

.SUFFIXES:
.SUFFIXES: .c .o

# files for a (source) distribution

DISTMISC = CHANGES PLUTO-CONVENTIONS TODO ipsec.secrets Makefile routing.txt \
	 pluto.8 ipsec.secrets.5 .cvsignore

DISTGCRYPT = \
	gcryptfix.c gcryptfix.h \
	dsa.c dsa.h \
	elgamal.c elgamal.h \
	primegen.c \
	smallprime.c

DISTSRC = \
	connections.c connections.h \
	foodgroups.c foodgroups.h \
	cookie.c cookie.h \
	pluto_constants.c \
	crypto.h crypto.c \
	db_ops.c \
	defs.h defs.c \
	demux.c demux.h \
	dnskey.c dnskey.h \
	endian.h \
	id.c id.h \
	ipsec_doi.c ipsec_doi.h \
	kernel.c kernel.h \
	kernel_netlink.c kernel_netlink.h \
	kernel_pfkey.c kernel_pfkey.h \
	kernel_noklips.c kernel_noklips.h \
	ike_alg.c ike_alg.h \
	ike_alg_aes.c ike_alginit.c \
	rcv_whack.c rcv_whack.h \
	$(IPSECPOLICY_DIST_SRCS) \
	log.c log.h \
	plutomain.c plutoalg.c \
	md2.c md2.h \
	md5.c md5.h \
	ocsp.c ocsp.h \
	packet.c packet.h \
	lex.c lex.h \
	keys.c keys.h \
	rnd.c rnd.h \
	server.c server.h \
	sha1.c sha1.h \
	${SMARTCARD_DIST_SRCS} \
	spdb.c spdb.h \
	state.c state.h \
	timer.c timer.h \
	$(DISTGCRYPT) \
	$(X509_DIST_SRCS) \
	$(HAVE_THREADS_DIST_SRCS) \
	vendor.c nat_traversal.c virtual.c \
	adns.c adns.h \
	whack.c whack.h whacklib.c\
	${XAUTH_DIST_SRCS}

DIST = $(DISTMISC) $(DISTSRC)


# start of support for DSS/DSA.  Not currently used.
# OBJSGCRYPT =  gcryptfix.o dsa.o elgamal.o primegen.o smallprime.o
OBJSGCRYPT =

OBJSLIBPLUTO = whacklib.o

OBJSPLUTO  = connections.o cookie.o crypto.o defs.o 
OBJSPLUTO += foodgroups.o log.o state.o plutomain.o plutoalg.o server.o
OBJSPLUTO += timer.o pluto_constants.o
OBJSPLUTO += id.o ipsec_doi.o kernel.o 
OBJSPLUTO += $(KERNEL26_OBJS) 
OBJSPLUTO += kernel_pfkey.o kernel_noklips.o rcv_whack.o 
OBJSPLUTO += ${IPSECPOLICY_OBJS} demux.o packet.o lex.o keys.o dnskey.o
OBJSPLUTO += rnd.o spdb.o sha1.o md5.o 
OBJSPLUTO += vendor.o nat_traversal.o virtual.o 
OBJSPLUTO += ike_alg_aes.o ike_alginit.o 
OBJSPLUTO += ${XAUTH_OBJS}
OBJSPLUTO += ${SMARTCARD_OBJS} ${X509_OBJS} ${HAVE_THREADS_OBJS}
OBJSPLUTO += ${OBJSLIBPLUTO}

OBJSADNS = adns.o

OBJSWHACK = whack.o


all: $(BINNAMEPLUTO) $(BINNAMEADNSIFNEEDED) $(BINNAMEWHACK)
programs: $(BINNAMEPLUTO) $(BINNAMEADNSIFNEEDED) $(BINNAMEWHACK)

oid.c: oid.txt oid.pl
	perl oid.pl

oid.h: oid.txt oid.pl
	perl oid.pl

install: all
	mkdir -p ${LIBEXECDIR} ${LIBDIR}
	mkdir -p -m 755 $(CONFDIR)/ipsec.d
	mkdir -p -m 755 $(CONFDIR)/ipsec.d/cacerts
	mkdir -p -m 755 $(CONFDIR)/ipsec.d/ocspcerts
	mkdir -p -m 755 $(CONFDIR)/ipsec.d/certs
	mkdir -p -m 755 $(CONFDIR)/ipsec.d/crls
	mkdir -p -m 700 $(CONFDIR)/ipsec.d/private
	$(INSTALL) $(INSTBINFLAGS) $(BINNAMEPLUTO) $(BINNAMEWHACK) $(LIBEXECDIR)
	if $(USE_ADNS) ; then $(INSTALL) $(INSTBINFLAGS) $(BINNAMEADNS)  $(LIBEXECDIR) ; fi
	$(INSTALL) $(INSTMANFLAGS) pluto.8 $(PMANDIR)/ipsec_pluto.8
	sh ${OPENSWANSRCDIR}/packaging/utils/manlink pluto.8 | \
		while read from to ; \
		do \
			ln -s -f ipsec_$$from $(PMANDIR)/$$to; \
		done
	$(INSTALL) $(INSTMANFLAGS) ipsec.secrets.5 $(FMANDIR)
	sh ${OPENSWANSRCDIR}/packaging/utils/manlink ipsec.secrets.5 | \
		while read from to ; \
		do \
		 ln -s -f $$from $(FMANDIR)/$$to; \
		done

install_file_list:
	@echo $(LIBEXECDIR)/$(BINNAMEPLUTO)
	@if $(USE_ADNS) ; then echo $(LIBDIR)/$(BINNAMEADNS) ; fi
	@echo $(LIBEXECDIR)/$(BINNAMEWHACK)
	@echo $(PMANDIR)/ipsec_pluto.8
	@sh ${OPENSWANSRCDIR}/packaging/utils/manlink pluto.8 | \
		while read from to; \
		do\
			 echo $(PMANDIR)/$$to; \
		done
	@echo $(FMANDIR)/ipsec.secrets.5
	@sh ${OPENSWANSRCDIR}/packaging/utils/manlink ipsec.secrets.5 | \
		while read from to; \
		do \
			echo $(FMANDIR)/$$to; \
		done

alg_info_test: alg_info_test.o ike_alg.o constants.o defs.o log.o db_ops.o crypto.o $(LIBDESLITE) $(OPENSWANLIB)
	$(CC) -o $@ $^ $(LIBSPLUTO)

# alg/libalg.o contains an already resolved object built with
# additional crypto algos inside.
OBJSPLUTO:= ike_alg.o db_ops.o $(OBJSPLUTO) 

$(BINNAMEPLUTO): $(OBJSPLUTO) $(ALG_LIBS) $(OPENSWANLIB)
	$(CC) -o $(BINNAMEPLUTO) $(LDFLAGS) $(OBJSPLUTO) $(LIBSPLUTO)

$(BINNAMEADNS): $(OBJSADNS)
	$(CC) -o $(BINNAMEADNS) $(OBJSADNS) $(LIBSADNS)

$(BINNAMEWHACK): $(OBJSWHACK)
	$(CC) -o $(BINNAMEWHACK) $(OBJSWHACK) $(LIBSWHACK)

distlist:
	@echo $(DIST)

# Exuberant Ctags doesn't work if LC_ALL is set to something other than C

CTAGSFLAGS = -N --format=1 # fishy options required for Exuberant Ctags

tags:	$(DISTSRC)
	LC_ALL=C ctags $(CTAGSFLAGS) $(DISTSRC) $(LIBOPENSWANDIR)/*.[ch]

TAGS:	$(DISTSRC)
	LC_ALL=C etags $(ETAGSFLAGS) $(DISTSRC) $(LIBOPENSWANDIR)/*.[ch]

cleanall: clean

distclean: clean

mostlyclean: clean

realclean: clean

clean:
	$(RM) $(RMFLAGS) *.core core *~ a.out ktrace.out \
		$(OBJSPLUTO) $(BINNAMEPLUTO) \
		$(OBJSWHACK) $(BINNAMEWHACK) \
		$(OBJSADNS) $(BINNAMEADNS)

check:
	echo no checks in lib right now.

checkprograms:

.c.o:
	$(CC) $(COPTS) $(ALLFLAGS) -c $<

# Gather dependencies caused by explicit #includes within .c files
#
# Each .c is assumed to compile into a .o with the corresponding name.
# Only dependencies on based on "" includes are considered, not <>.
# Dependencies caused by includes within headers are not noticed.
# Unlike dependencies generated by the compiler, these include dependencies
# suppressed by conditional compilation (good, we think).
# This code can be tricked by embeding #include in comments or
# vice-versa, but we're among friends.

gatherdeps:
	@ls $(DISTSRC) | grep '\.c' | sed -e 's/\(.*\)\.c$$/\1.o: \1.c/'
	@echo
	@ls $(DISTSRC) | grep '\.c' | xargs grep '^#[ 	]*include[ 	]*"' | \
		sed -e 's/\.c:#[ 	]*include[ 	]*"/.o: /' -e 's/".*//'

# Dependencies generated by "make gatherdeps":

ac.o: ac.c
adns.o: adns.c
asn1.o: asn1.c
certs.o: certs.c
connections.o: connections.c
constants.o: constants.c
cookie.o: cookie.c
crypto.o: crypto.c
defs.o: defs.c
demux.o: demux.c
dnskey.o: dnskey.c
dsa.o: dsa.c
elgamal.o: elgamal.c
fetch.o: fetch.c
foodgroups.o: foodgroups.c
gcryptfix.o: gcryptfix.c
id.o: id.c
ike_alg.o: ike_alg.c
ipsec_doi.o: ipsec_doi.c
kernel_alg.o: kernel_alg.c
kernel.o: kernel.c
kernel_netlink.o: kernel_netlink.c
kernel_noklips.o: kernel_noklips.c
kernel_pfkey.o: kernel_pfkey.c
keys.o: keys.c
lex.o: lex.c
log.o: log.c
md2.o: md2.c
md2.o: md2.c
md5.o: md5.c
ocsp.o: ocsp.c
nat_traversal.o: nat_traversal.c
oid.o: oid.c
packet.o: packet.c
pem.o: pem.c
pgp.o: pgp.c
pkcs.o: pkcs.c
plutomain.o: plutomain.c
primegen.o: primegen.c
rcv_info.o: rcv_info.c
rcv_whack.o: rcv_whack.c
rnd.o: rnd.c
server.o: server.c
sha1.o: sha1.c
smallprime.o: smallprime.c
smartcard.o: smartcard.c
spdb.o: spdb.c
state.o: state.c
timer.o: timer.c
vendor.o: vendor.c
virtual.o: virtual.c
whack.o: whack.c
whacklib.o: whacklib.c
x509.o: x509.c
xauth.o: xauth.c

ac.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
ac.o: defs.h
ac.o: asn1.h
ac.o: oid.h
ac.o: ac.h
ac.o: id.h
ac.o: x509.h
ac.o: pgp.h
ac.o: certs.h
ac.o: log.h
ac.o: whack.h
ac.o: fetch.h
adns.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
adns.o: adns.h
asn1.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
asn1.o: defs.h
asn1.o: asn1.h
asn1.o: oid.h
asn1.o: log.h
certs.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
certs.o: defs.h
certs.o: log.h
certs.o: asn1.h
certs.o: id.h
certs.o: x509.h
certs.o: pgp.h
certs.o: pem.h
certs.o: certs.h
certs.o: pkcs.h
connections.o: kameipsec.h
connections.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
connections.o: defs.h
connections.o: id.h
connections.o: x509.h
connections.o: pgp.h
connections.o: certs.h
connections.o: ac.h
connections.o: smartcard.h
connections.o: connections.h
connections.o: foodgroups.h
connections.o: packet.h
connections.o: demux.h
connections.o: state.h
connections.o: timer.h
connections.o: ipsec_doi.h
connections.o: server.h
connections.o: kernel.h
connections.o: log.h
connections.o: keys.h
connections.o: adns.h
connections.o: dnskey.h
connections.o: whack.h
connections.o: nat_traversal.h
connections.o: virtual.h
constants.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
constants.o: defs.h
constants.o: log.h
constants.o: packet.h
constants.o: xauth.h
cookie.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
cookie.o: defs.h
cookie.o: sha1.h
cookie.o: rnd.h
cookie.o: cookie.h
crypto.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
crypto.o: defs.h
crypto.o: state.h
crypto.o: log.h
crypto.o: md5.h
crypto.o: sha1.h
crypto.o: crypto.h
crypto.o: ike_alg.h
defs.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
defs.o: defs.h
defs.o: log.h
defs.o: whack.h
demux.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
demux.o: defs.h
demux.o: cookie.h
demux.o: id.h
demux.o: x509.h
demux.o: pgp.h
demux.o: certs.h
demux.o: smartcard.h
demux.o: connections.h
demux.o: state.h
demux.o: packet.h
demux.o: md5.h
demux.o: sha1.h
demux.o: crypto.h
demux.o: ike_alg.h
demux.o: log.h
demux.o: demux.h
demux.o: ipsec_doi.h
demux.o: timer.h
demux.o: whack.h
demux.o: server.h
demux.o: xauth.h
demux.o: nat_traversal.h
demux.o: vendor.h
dnskey.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
dnskey.o: adns.h
dnskey.o: defs.h
dnskey.o: id.h
dnskey.o: log.h
dnskey.o: x509.h
dnskey.o: pgp.h
dnskey.o: certs.h
dnskey.o: smartcard.h
dnskey.o: connections.h
dnskey.o: keys.h
dnskey.o: dnskey.h
dnskey.o: packet.h
dnskey.o: timer.h
dsa.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
dsa.o: defs.h
dsa.o: log.h
dsa.o: rnd.h
dsa.o: gcryptfix.h
dsa.o: dsa.h
elgamal.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
elgamal.o: defs.h
elgamal.o: log.h
elgamal.o: rnd.h
elgamal.o: gcryptfix.h
elgamal.o: elgamal.h
fetch.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
fetch.o: defs.h
fetch.o: log.h
fetch.o: id.h
fetch.o: asn1.h
fetch.o: pem.h
fetch.o: x509.h
fetch.o: whack.h
fetch.o: ocsp.h
fetch.o: fetch.h
foodgroups.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
foodgroups.o: defs.h
foodgroups.o: id.h
foodgroups.o: x509.h
foodgroups.o: pgp.h
foodgroups.o: certs.h
foodgroups.o: smartcard.h
foodgroups.o: connections.h
foodgroups.o: foodgroups.h
foodgroups.o: kernel.h
foodgroups.o: lex.h
foodgroups.o: log.h
foodgroups.o: whack.h
gcryptfix.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
gcryptfix.o: defs.h
gcryptfix.o: log.h
gcryptfix.o: rnd.h
gcryptfix.o: gcryptfix.h
id.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
id.o: defs.h
id.o: id.h
id.o: log.h
id.o: x509.h
id.o: pgp.h
id.o: certs.h
id.o: smartcard.h
id.o: connections.h
id.o: packet.h
id.o: whack.h
ike_alg.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
ike_alg.o: defs.h
ike_alg.o: sha1.h
ike_alg.o: md5.h
ike_alg.o: crypto.h
ike_alg.o: state.h
ike_alg.o: packet.h
ike_alg.o: log.h
ike_alg.o: whack.h
ike_alg.o: spdb.h
ike_alg.o: ike_alg.h
ike_alg.o: db_ops.h
ike_alg.o: id.h
ike_alg.o: connections.h
ike_alg.o: kernel.h
ipsec_doi.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
ipsec_doi.o: defs.h
ipsec_doi.o: state.h
ipsec_doi.o: id.h
ipsec_doi.o: x509.h
ipsec_doi.o: pgp.h
ipsec_doi.o: certs.h
ipsec_doi.o: smartcard.h
ipsec_doi.o: connections.h
ipsec_doi.o: keys.h
ipsec_doi.o: packet.h
ipsec_doi.o: demux.h
ipsec_doi.o: adns.h
ipsec_doi.o: dnskey.h
ipsec_doi.o: kernel.h
ipsec_doi.o: log.h
ipsec_doi.o: cookie.h
ipsec_doi.o: server.h
ipsec_doi.o: spdb.h
ipsec_doi.o: timer.h
ipsec_doi.o: rnd.h
ipsec_doi.o: ipsec_doi.h
ipsec_doi.o: whack.h
ipsec_doi.o: fetch.h
ipsec_doi.o: pkcs.h
ipsec_doi.o: asn1.h
ipsec_doi.o: sha1.h
ipsec_doi.o: md5.h
ipsec_doi.o: crypto.h
ipsec_doi.o: xauth.h
ipsec_doi.o: vendor.h
ipsec_doi.o: nat_traversal.h
ipsec_doi.o: virtual.h
ipsec_doi.o: x509more.h
kernel.o: kameipsec.h
kernel.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
kernel.o: defs.h
kernel.o: rnd.h
kernel.o: id.h
kernel.o: connections.h
kernel.o: state.h
kernel.o: timer.h
kernel.o: kernel.h
kernel.o: kernel_netlink.h
kernel.o: kernel_pfkey.h
kernel.o: kernel_noklips.h
kernel.o: packet.h
kernel.o: x509.h
kernel.o: log.h
kernel.o: server.h
kernel.o: whack.h
kernel.o: keys.h
kernel.o: packet.h
kernel.o: nat_traversal.h
kernel_netlink.o: kameipsec.h
kernel_netlink.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
kernel_netlink.o: defs.h
kernel_netlink.o: id.h
kernel_netlink.o: connections.h
kernel_netlink.o: kernel.h
kernel_netlink.o: kernel_netlink.h
kernel_netlink.o: kernel_pfkey.h
kernel_netlink.o: log.h
kernel_netlink.o: whack.h
kernel_noklips.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
kernel_noklips.o: defs.h
kernel_noklips.o: id.h
kernel_noklips.o: connections.h
kernel_noklips.o: kernel.h
kernel_noklips.o: kernel_noklips.h
kernel_noklips.o: log.h
kernel_noklips.o: whack.h
kernel_pfkey.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
kernel_pfkey.o: defs.h
kernel_pfkey.o: id.h
kernel_pfkey.o: connections.h
kernel_pfkey.o: kernel.h
kernel_pfkey.o: kernel_pfkey.h
kernel_pfkey.o: log.h
kernel_pfkey.o: whack.h
kernel_pfkey.o: packet.h
kernel_pfkey.o: nat_traversal.h
keys.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
keys.o: defs.h
keys.o: id.h
keys.o: x509.h
keys.o: pgp.h
keys.o: certs.h
keys.o: smartcard.h
keys.o: connections.h
keys.o: state.h
keys.o: lex.h
keys.o: keys.h
keys.o: adns.h
keys.o: dnskey.h
keys.o: log.h
keys.o: whack.h
keys.o: timer.h
keys.o: fetch.h
keys.o: x509more.h
keys.o: nat_traversal.h
lex.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
lex.o: defs.h
lex.o: log.h
lex.o: whack.h
lex.o: lex.h
log.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
log.o: defs.h
log.o: log.h
log.o: server.h
log.o: state.h
log.o: id.h
log.o: x509.h
log.o: pgp.h
log.o: certs.h
log.o: smartcard.h
log.o: connections.h
log.o: kernel.h
log.o: whack.h
log.o: timer.h
log.o: ike_alg.h
log.o: db_ops.h
md2.o: md2.h
md2.o: md2.h
md5.o: md5.h
md5.o: endian.h
ocsp.o: defs.h
ocsp.o: id.h
ocsp.o: log.h
ocsp.o: x509.h
ocsp.o: rnd.h
ocsp.o: asn1.h
ocsp.o: pgp.h
ocsp.o: certs.h
ocsp.o: smartcard.h
ocsp.o: oid.h
ocsp.o: whack.h
ocsp.o: keys.h
ocsp.o: fetch.h
ocsp.o: ocsp.h
nat_traversal.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
nat_traversal.o: defs.h
nat_traversal.o: log.h
nat_traversal.o: server.h
nat_traversal.o: state.h
nat_traversal.o: id.h
nat_traversal.o: x509.h
nat_traversal.o: pgp.h
nat_traversal.o: certs.h
nat_traversal.o: smartcard.h
nat_traversal.o: connections.h
nat_traversal.o: packet.h
nat_traversal.o: demux.h
nat_traversal.o: kernel.h
nat_traversal.o: whack.h
nat_traversal.o: timer.h
nat_traversal.o: cookie.h
nat_traversal.o: sha1.h
nat_traversal.o: md5.h
nat_traversal.o: crypto.h
nat_traversal.o: vendor.h
nat_traversal.o: nat_traversal.h
oid.o: oid.h
packet.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
packet.o: defs.h
packet.o: log.h
packet.o: packet.h
packet.o: whack.h
pem.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
pem.o: defs.h
pem.o: log.h
pem.o: md5.h
pem.o: whack.h
pem.o: pem.h
pgp.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
pgp.o: defs.h
pgp.o: log.h
pgp.o: id.h
pgp.o: pgp.h
pgp.o: x509.h
pgp.o: certs.h
pgp.o: md5.h
pgp.o: whack.h
pgp.o: keys.h
pkcs.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
pkcs.o: defs.h
pkcs.o: asn1.h
pkcs.o: oid.h
pkcs.o: log.h
pkcs.o: id.h
pkcs.o: x509.h
pkcs.o: pgp.h
pkcs.o: certs.h
pkcs.o: pkcs.h
plutomain.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
plutomain.o: defs.h
plutomain.o: id.h
plutomain.o: x509.h
plutomain.o: pgp.h
plutomain.o: certs.h
plutomain.o: ac.h
plutomain.o: smartcard.h
plutomain.o: connections.h
plutomain.o: foodgroups.h
plutomain.o: packet.h
plutomain.o: demux.h
plutomain.o: server.h
plutomain.o: kernel.h
plutomain.o: log.h
plutomain.o: keys.h
plutomain.o: adns.h
plutomain.o: dnskey.h
plutomain.o: rnd.h
plutomain.o: state.h
plutomain.o: ipsec_doi.h
plutomain.o: ocsp.h
plutomain.o: fetch.h
plutomain.o: sha1.h
plutomain.o: md5.h
plutomain.o: crypto.h
plutomain.o: virtual.h
plutomain.o: nat_traversal.h
primegen.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
primegen.o: defs.h
primegen.o: log.h
primegen.o: rnd.h
primegen.o: gcryptfix.h
rcv_info.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
rcv_info.o: defs.h
rcv_info.o: id.h
rcv_info.o: connections.h
rcv_info.o: foodgroups.h
rcv_info.o: whack.h
rcv_info.o: packet.h
rcv_info.o: demux.h
rcv_info.o: state.h
rcv_info.o: ipsec_doi.h
rcv_info.o: kernel.h
rcv_info.o: rcv_whack.h
rcv_info.o: log.h
rcv_info.o: keys.h
rcv_info.o: adns.h
rcv_info.o: dnskey.h
rcv_info.o: server.h
rcv_info.o: ../../linux/include/openswan/ipsec_policy.h
rcv_info.o: rcv_info.h
rcv_whack.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
rcv_whack.o: defs.h
rcv_whack.o: id.h
rcv_whack.o: x509.h
rcv_whack.o: pgp.h
rcv_whack.o: certs.h
rcv_whack.o: ac.h
rcv_whack.o: smartcard.h
rcv_whack.o: connections.h
rcv_whack.o: foodgroups.h
rcv_whack.o: whack.h
rcv_whack.o: packet.h
rcv_whack.o: demux.h
rcv_whack.o: state.h
rcv_whack.o: ipsec_doi.h
rcv_whack.o: kernel.h
rcv_whack.o: rcv_whack.h
rcv_whack.o: log.h
rcv_whack.o: keys.h
rcv_whack.o: adns.h
rcv_whack.o: dnskey.h
rcv_whack.o: server.h
rcv_whack.o: fetch.h
rcv_whack.o: ike_alg.h
rcv_whack.o: ocsp.h
rnd.o: sha1.h
rnd.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
rnd.o: defs.h
rnd.o: rnd.h
rnd.o: log.h
rnd.o: timer.h
server.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
server.o: defs.h
server.o: state.h
server.o: id.h
server.o: x509.h
server.o: pgp.h
server.o: certs.h
server.o: smartcard.h
server.o: connections.h
server.o: kernel.h
server.o: log.h
server.o: server.h
server.o: timer.h
server.o: packet.h
server.o: demux.h
server.o: rcv_whack.h
server.o: rcv_info.h
server.o: keys.h
server.o: adns.h
server.o: dnskey.h
server.o: whack.h
server.o: kameipsec.h
server.o: nat_traversal.h
sha1.o: sha1.h
sha1.o: endian.h
smallprime.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
smallprime.o: defs.h
smallprime.o: gcryptfix.h
smartcard.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
smartcard.o: defs.h
smartcard.o: id.h
smartcard.o: log.h
smartcard.o: x509.h
smartcard.o: pgp.h
smartcard.o: certs.h
smartcard.o: smartcard.h
smartcard.o: whack.h
smartcard.o: fetch.h
spdb.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
spdb.o: defs.h
spdb.o: id.h
spdb.o: x509.h
spdb.o: pgp.h
spdb.o: certs.h
spdb.o: smartcard.h
spdb.o: connections.h
spdb.o: state.h
spdb.o: packet.h
spdb.o: keys.h
spdb.o: kernel.h
spdb.o: log.h
spdb.o: spdb.h
spdb.o: whack.h
spdb.o: sha1.h
spdb.o: md5.h
spdb.o: crypto.h
spdb.o: ike_alg.h
spdb.o: db_ops.h
spdb.o: nat_traversal.h
state.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
state.o: defs.h
state.o: id.h
state.o: x509.h
state.o: pgp.h
state.o: certs.h
state.o: smartcard.h
state.o: connections.h
state.o: state.h
state.o: kernel.h
state.o: log.h
state.o: packet.h
state.o: keys.h
state.o: rnd.h
state.o: timer.h
state.o: whack.h
state.o: demux.h
state.o: ipsec_doi.h
state.o: sha1.h
state.o: md5.h
state.o: crypto.h
timer.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
timer.o: defs.h
timer.o: id.h
timer.o: x509.h
timer.o: pgp.h
timer.o: certs.h
timer.o: smartcard.h
timer.o: connections.h
timer.o: state.h
timer.o: packet.h
timer.o: demux.h
timer.o: ipsec_doi.h
timer.o: kernel.h
timer.o: server.h
timer.o: log.h
timer.o: rnd.h
timer.o: timer.h
timer.o: whack.h
timer.o: nat_traversal.h
vendor.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
vendor.o: defs.h
vendor.o: log.h
vendor.o: md5.h
vendor.o: id.h
vendor.o: x509.h
vendor.o: pgp.h
vendor.o: certs.h
vendor.o: smartcard.h
vendor.o: connections.h
vendor.o: packet.h
vendor.o: demux.h
vendor.o: whack.h
vendor.o: vendor.h
vendor.o: nat_traversal.h
virtual.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
virtual.o: defs.h
virtual.o: log.h
virtual.o: id.h
virtual.o: x509.h
virtual.o: pgp.h
virtual.o: certs.h
virtual.o: smartcard.h
virtual.o: connections.h
virtual.o: whack.h
virtual.o: virtual.h
whack.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
whack.o: defs.h
whack.o: whack.h
x509.o: ../../include/constants.h ../../include/ietf_constants.h ../../include/pluto_constants.h
x509.o: defs.h
x509.o: log.h
x509.o: id.h
x509.o: asn1.h
x509.o: oid.h
x509.o: x509.h
x509.o: pgp.h
x509.o: certs.h
x509.o: keys.h
x509.o: packet.h
x509.o: demux.h
x509.o: connections.h
x509.o: state.h
x509.o: md2.h
x509.o: md5.h
x509.o: sha1.h
x509.o: whack.h
x509.o: fetch.h
x509.o: pkcs.h
x509.o: x509more.h
x509.o: ocsp.h
