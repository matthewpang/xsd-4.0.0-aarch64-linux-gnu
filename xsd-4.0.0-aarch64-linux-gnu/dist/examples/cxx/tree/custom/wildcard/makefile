root := ../../../..

include $(root)/build/cxx/rules.make
include $(root)/build/xsd/tree-rules.make


override XSDFLAGS += --generate-inline --generate-ostream \
--generate-serialization --hxx-epilogue '\#include "wildcard-custom.hxx"'

ifneq ($(shell uname -o),Msys)
override XSDFLAGS += --custom-type data=/data_base
else
# Prevent MSYS from converting / to Windows path.
override XSDFLAGS += --custom-type data=//data_base
endif


# Build.
#
driver: driver.o wildcard.o wildcard-custom.o

wildcard.o: wildcard.cxx wildcard.ixx wildcard.hxx wildcard-custom.hxx
driver.o: driver.cxx wildcard.ixx wildcard.hxx wildcard-custom.hxx
wildcard-custom.o: wildcard-custom.cxx wildcard.ixx wildcard.hxx wildcard-custom.hxx

wildcard.cxx wildcard.hxx wildcard.ixx: wildcard.xsd


# Test
#
.PHONY: test
test: driver wildcard.xml
	./driver wildcard.xml


# Clean.
#
.PHONY: clean
clean:
	rm -f wildcard-custom.o wildcard.o wildcard.?xx driver.o driver
