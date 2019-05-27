# Makefile for amstyler

#######################
# Configuration below #
#######################

# By default, we build against system clang.

# If building against clang source, uncomment these lines.
#LLVM_SRC_PATH := /home/alliedmodders/llvm-project
#LLVM_BUILD_PATH := /home/alliedmodders/llvm-project/build

# If building against a release tarball, uncomment these lines.
#LLVM_TARBALL_PATH := /home/alliedmodders/llvm-6.0

# If building against an installed clang/llvm, llvm-config
# will be used if found. To change it, specify below.
#LLVM_CONFIG := llvm-config-6

#####################
# Build rules below #
#####################

ifneq (${LLVM_SRC_PATH},)
  LLVM_BIN_PATH := ${LLVM_BUILD_PATH}/bin
else
  ifneq (${LLVM_TARBALL_PATH},)
    LLVM_SRC_PATH := ${LLVM_TARBALL_PATH}
    LLVM_BIN_PATH := ${LLVM_TARBALL_PATH}/bin
  else
    ifeq (${LLVM_CONFIG},)
      LLVM_CONFIG := llvm-config-6.0
      ifeq ($(shell which ${LLVM_CONFIG}),)
        LLVM_CONFIG := llvm-config
      endif
    endif
    LLVM_SRC_PATH := $(shell ${LLVM_CONFIG} --includedir)
    LLVM_BIN_PATH := $(shell ${LLVM_CONFIG} --bindir)
  endif
endif

LLVM_VERSION := $(shell ${LLVM_CONFIG} --version)
ifeq ($(findstring 6.0,${LLVM_VERSION}),)
  $(error llvm version 6.0 is required)
endif

$(info -----------------------------------------------)
$(info Using LLVM_SRC_PATH = $(LLVM_SRC_PATH))
$(info Using LLVM_BIN_PATH = $(LLVM_BIN_PATH))
$(info Using LLVM_VERSION = $(LLVM_VERSION))
$(info -----------------------------------------------)

CXX := clang++
CXXFLAGS := -fno-rtti -O2 -Wall -Wno-c99-extensions -s

ifeq (${LLVM_CONFIG},)
  LLVM_CONFIG := $(LLVM_BIN_PATH)/llvm-config
endif

LLVM_CXXFLAGS_STR := $(shell ${LLVM_CONFIG} --cxxflags)
LLVM_CXXFLAGS := $(subst -std=c++11,-std=c++17,${LLVM_CXXFLAGS_STR})
LLVM_CXXFLAGS := $(subst -Wno-maybe-uninitialized,,${LLVM_CXXFLAGS})
LLVM_LDFLAGS := `${LLVM_CONFIG} --ldflags --libs --system-libs`

CXXFLAGS += ${LLVM_CXXFLAGS}
ifneq ($(ENABLE_DEBUG),)
  CXXFLAGS := $(subst -O2,,${CXXFLAGS})
  CXXFLAGS := $(subst -O1,,${CXXFLAGS})
  CXXFLAGS := $(subst -O3,,${CXXFLAGS})
  CXXFLAGS := $(subst -s ,,${CXXFLAGS})
  CXXFLAGS := $(subst -DNDEBUG,,${CXXFLAGS})
  CXXFLAGS += -O0 -g -ggdb3
endif

# These are required when compiling vs. a source distribution of Clang. For
# binary distributions llvm-config --cxxflags gives the right path.
CLANG_INCLUDES := \
	-I$(LLVM_SRC_PATH)/clang/include

# List of Clang libraries to link. The proper -L will be provided by the
# call to llvm-config
# Note that I'm using -Wl,--{start|end}-group around the Clang libs; this is
# because there are circular dependencies that make the correct order difficult
# to specify and maintain. The linker group options make the linking somewhat
# slower, but IMHO they're still perfectly fine for tools that link with Clang.
CLANG_LIBS := \
	-Wl,--start-group \
	-lclangAST \
	-lclangASTMatchers \
	-lclangAnalysis \
	-lclangBasic \
	-lclangDriver \
	-lclangEdit \
	-lclangFrontend \
	-lclangFrontendTool \
	-lclangLex \
	-lclangParse \
	-lclangSema \
	-lclangEdit \
	-lclangRewrite \
	-lclangRewriteFrontend \
	-lclangStaticAnalyzerFrontend \
	-lclangStaticAnalyzerCheckers \
	-lclangStaticAnalyzerCore \
	-lclangCrossTU \
	-lclangIndex \
	-lclangSerialization \
	-lclangToolingCore \
	-lclangTooling \
	-lclangFormat \
	-Wl,--end-group

# Internal paths in this project: where to find sources, and where to put
# build artifacts.
BUILDDIR := build

.PHONY: all
all: make_builddir \
	$(BUILDDIR)/amstyler

.PHONY: emit_build_config
emit_build_config: make_builddir
	@echo $(LLVM_BIN_PATH) > $(BUILDDIR)/_build_config

.PHONY: make_builddir
make_builddir:
	@test -d $(BUILDDIR) || mkdir $(BUILDDIR)

$(BUILDDIR)/amstyler: amstyler.cpp
	$(CXX) $(CXXFLAGS) $(CLANG_INCLUDES) $^ \
		$(CLANG_LIBS) $(LLVM_LDFLAGS) -o $@

.PHONY: clean

clean:
	rm -rf $(BUILDDIR)/*
