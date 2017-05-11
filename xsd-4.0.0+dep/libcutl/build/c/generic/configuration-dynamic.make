ifeq ($(filter $(origin CC),undefined default),)
c_generic := $(CC)
else
c_generic := gcc
endif
