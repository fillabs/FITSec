######################################################################
##
##  Created by: Denis Filatov
##
##  Copyleft (c) 2015
##  This code is provided under the CeCill-C license agreement.
######################################################################
PROJECTROOT    = ..
BUILDROOT      = ../build
PROJECT        = fitsec_tests
DEBUG          = yes
bins           = test_engine
sources       := test_engine.c
includes      += ../libfitsec
packages      += openssl cshared
libs           = $(outdir)/libfitsec.so -lm
CSHAREDDIR   = ../cshared
include ../common.mk
