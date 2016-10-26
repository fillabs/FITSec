export LANG=en_US
ALL_CONFIGURATIONS := POSIX WIN32
.PHONY: all clean tests docs cleandocs distr DUMMY

CSHAREDDIR ?= cshared 
CXMLDIR ?= cxml

ifeq ($(ARCH),)
 ARCH = $(shell gcc -dumpmachine)
 GCC := gcc
else
 GCC := $(addprefix $(addsuffix -,$(ARCH)), gcc)
endif

ifneq ($(findstring w32,$(ARCH)),)
 packages := $(filter-out readline threads, $(packages))
 CFG  += WIN32
else 
 CFG  += POSIX
 ifeq ($(findstring cygwin,$(ARCH)),)
  cflags += -fPIC
 endif
endif

cflags   += -Wall

ifeq ($(DEBUG),)
 DEBUG=no
endif

ifeq ($(DEBUG),yes)
  cflags  += -g -O0
  defines += DEBUG
  dsuffix = -d
else
  defines += NDEBUG
  cflags  += -O2
endif

ifneq ($(filter readline, $(packages)),)
  defines += USE_READLINE
  libs    += -lreadline
endif

ifneq ($(filter dmalloc, $(packages)),)
  defines  += DMALLOC DMALLOC_FUNC_CHECK
  libs     += -ldmalloc
  dsuffix   = -dmalloc
endif

ifneq ($(filter thread, $(packages)),)
  defines += USE_THREADS
  libs    += -lpthread
endif

ifneq ($(filter profile, $(packages)),)
  cflags += -pg
endif

ifneq ($(filter openssl, $(packages)),)
 ifneq ($(findstring mingw32,$(ARCH)),)
  includes += C:/OpenSSL/Win32/include
  libs     += C:/OpenSSL/Win32/lib/MinGW/libeay32.a C:/OpenSSL/Win32/lib/MinGW/ssleay32.a
 else
  libs    += -lssl -lcrypto
 endif
endif

ifneq ($(filter cxml, $(packages)),)
  predirs += $(CXMLDIR)
  includes += $(CXMLDIR)
endif

ifneq ($(filter cshared, $(packages)),)
  predirs += $(CSHAREDDIR)
  includes += $(CSHAREDDIR)
endif

ifeq ($(testdir), )
  testdir := tests
endif

includes  += $(foreach cfg,$(CFG),$(includes-$(cfg)))
defines   += $(foreach cfg,$(CFG),$(defines-$(cfg)))
libs      += $(foreach cfg,$(CFG),$(libs-$(cfg)))
sources   += $(foreach cfg,$(CFG),$(sources-$(cfg)))
headers   += $(foreach cfg,$(CFG),$(headers-$(cfg)))
tests     += $(foreach cfg,$(CFG),$(tests-$(cfg)))
distfiles += $(foreach cfg,$(CFG),$(distfiles-$(cfg)))
predirs   += $(foreach cfg,$(CFG),$(predirs-$(cfg)))
postdirs  += $(foreach cfg,$(CFG),$(postdirs-$(cfg)))

tests     := $(addprefix $(addsuffix /,$(testdir)),$(tests))
sources   := $(addprefix $(addsuffix /,$(srcdir)),$(sources))
headers   := $(addprefix $(addsuffix /,$(incdir)),$(headers))

cflags    += $(addprefix -I, $(includes)) $(addprefix -D, $(defines))

ifeq ($(BUILDROOT),)
 BUILDROOT = .
endif

outdir    := $(BUILDROOT)/$(ARCH)$(dsuffix)
objdir    := $(outdir)/$(PROJECT)
objects   := $(patsubst %.c, $(objdir)/%.o, $(sources))
testbins  := $(patsubst %.c, $(outdir)/%, $(tests))
dirs      := $(objdir) $(outdir)/tests

alibnames  := $(patsubst %, $(outdir)/lib%.a,  $(alibs))
solibnames := $(patsubst %, $(outdir)/lib%.so, $(solibs))
binnames   := $(patsubst %, $(outdir)/%, $(bins))

ldflags += $(patsubst %, -L%, $(outdir) $(libdirs))

ifneq ($(filter cxml, $(packages)),)
  libs += $(outdir)/libcxml.a
endif

ifneq ($(filter cshared, $(packages)),)
  libs += $(outdir)/libcshared.a
endif


all: $(dirs) $(predirs) $(alibnames) $(solibnames) $(binnames) $(postdirs)

tests: all $(testbins)

$(predirs) $(postdirs): DUMMY
	$(MAKE) -C $@ BUILDROOT=$(realpath $(BUILDROOT)) DEBUG=$(DEBUG)

$(alibnames): $(outdir)/lib%.a : $(objects)
	ar rcs $@ $^

$(solibnames): $(outdir)/lib%.so : $(objects)
	$(GCC) $(cflags) -shared $(ldflags) -o $@ $^ $(csharedlib) $(libs)

$(binnames): $(outdir)/% : $(objects)
	$(GCC) $(cflags)  $(ldflags) -o $@ $^ $(csharedlib) $(libs)

$(testbins): $(alibnames)
$(testbins): $(outdir)/tests/% : tests/%.c
	$(GCC) $(cflags) -o $@ $< $(alibnames) $(libs)

$(dirs):
	mkdir -p $@

$(objects): $(objdir)/%.o: %.c
	@mkdir -p $(dir $@)
	$(GCC)  $(cflags) -o $@ -MMD -MF $(objdir)/$*.d -c $<

clean:
	rm -rf $(alibnames) $(solibnames) $(binnames) $(testbins) $(objects)

distfiles += $(wildcard Makefile $(DOXYFILE))
dist:
	-rm -rf $(PROJECT) $(PROJECT)-$(shell date -u '+%Y%m%d').tar.gz
	mkdir $(PROJECT)
	cp --parents $(sources) $(headers) $(distfiles) $(addprefix tests/, $(tests)) $(PROJECT)
	tar -zcvf $(PROJECT)-$(shell date -u '+%Y%m%d').tar.gz $(PROJECT)
	rm -rf $(PROJECT)
#	tar -zcvf $(PROJECT)-$(shell date -u '+%Y%m%d').tar.gz $(sources) $(headers) $(distfiles) $(addprefix tests/, $(tests))

ifneq (,$(DOXYFILE))
docs: $(DOXYFILE)
	doxygen $(DOXYFILE)

cleandocs:
	rm -rf doc/html
endif

include $(wildcard $(addsuffix /*.d, $(objdir)))
