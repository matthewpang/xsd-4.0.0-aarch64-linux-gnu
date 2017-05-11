ifeq ($(filter $(origin CXX),undefined default),)
cxx_generic := $(CXX)
else
cxx_generic := g++
endif
