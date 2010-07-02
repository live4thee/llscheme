SUBDIRS = runtime

dist_bin_SCRIPTS = llscheme

bin_PROGRAMS = llscheme.bin

llscheme_bin_CXXFLAGS = -I$(srcdir)/include $(LLVM_CPPFLAGS)
llscheme_bin_LDADD = $(LLVM_LIBS)

llscheme_bin_SOURCES = ast.cc \
		       codegen.cc \
		       driver.cc \
		       env.cc \
		       error.cc \
		       lexer.cc \
		       main.cc \
		       parser.cc \
		       codestream.cc \
		       token.cc

noinst_HEADERS = include/driver.hh \
                 include/error.hh \
                 include/env.hh \
                 include/token.hh \
                 include/ast.hh \
                 include/astnodes.hh \
                 include/lexer.hh \
                 include/codestream.hh \
                 include/parser.hh
