PROJECTROOT  = ..
BUILDROOT    = ../build/
PROJECT      = fitsec
DEBUG        = yes
CSHAREDDIR   = ../cshared
packages     := openssl cshared

alibs        = $(PROJECT)
solibs       = $(PROJECT)

sources       := src/fitsec_cert.c src/fitsec_certhash.c src/fitsec_engine.c \
                 src/fitsec_region.c src/fitsec_types.c src/fitsec_crypt.c  src/fitsec_error.c

sources       += plugins/fitsec_openssl.c
                 
headers       := fitsec.h src/fitsec_i.h src/fitsec_error.h src/fitsec_types.h src/fitsec_unstats.h khash.h
defines       := LIBFITSEC_EXPORTS
include ../common.mk
