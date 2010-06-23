bin_PROGRAMS = llscheme.bin

llscheme_bin_CXXFLAGS = -I./include $(LLVM_CPPFLAGS)
llscheme_bin_LDADD = $(LLVM_LIBS)

llscheme_bin_SOURCES = ast2.cc \
		       codegen.cc \
		       driver.cc \
		       env.cc \
		       error.cc \
		       lexer.cc \
		       main.cc \
		       parser.cc \
		       scanner.cc \
		       token.cc
