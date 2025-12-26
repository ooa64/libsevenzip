# makefile

I = install
O = outputs

#SEVENZIPSRC ?= externals/p7zip
SEVENZIPSRC ?= externals/7zip
SEVENZIPPATH ?=$(SEVENZIPSRC)/CPP/7zip/Bundles/Format7zF/_o

ifdef UNICODE
	CFLAGS += -DUNICODE -D_UNICODE
endif

ifdef DEBUG
	O = outputs/debug
	CFLAGS += -ggdb -O0
else
	O = outputs/release
endif

ifdef DEBUG_IMPL
	CFLAGS += -DDEBUG_IMPL
endif

ifdef ASAN
	CFLAGS += -fsanitize=address -fno-omit-frame-pointer -fno-common -g3
endif

#CXX=g++-13
#CXX17=g++-13 -std=c++17
CXX17=$(CXX) -std=c++17

TARGET = libsevenzip.a
EXAMPLES = example0 example1 example2 example3 example4 example5 example6 example7 example8

CFLAGS += -fPIC -Wall -Wextra -I$(SEVENZIPSRC) -I.
CXXFLAGS += $(CFLAGS)
LDFLAGS +=

OBJS = \
	$O/sevenzip.o \
	$O/sevenzip_impl.o \
	$O/IntToString.o \
	$O/MyString.o \
	$O/MyWindows.o \
	$O/PropVariant.o \
	$O/TimeUtils.o \
	$O/StringConvert.o \
	$O/UTFConvert.o \
	$O/ErrorMsg.o \
	$O/DLL.o

all: $(TARGET)

clean:
	-rm $O/*.o
	-rmdir $O

cleanall: clean
	-rm libsevenzip.a example[0-9H] example
	-rm -fr temps

libsevenzip.a: $(OBJS)
	ar rcs $@ $^

examples: $(EXAMPLES) example_dir

example0: example0.cpp sevenzip.h libsevenzip.a
example1: example1.cpp sevenzip.h libsevenzip.a
example2: example2.cpp sevenzip.h libsevenzip.a
example3: example3.cpp sevenzip.h libsevenzip.a
example4: example4.cpp sevenzip.h libsevenzip.a
example5: example5.cpp sevenzip.h libsevenzip.a
example6: example6.cpp sevenzip.h libsevenzip.a
example7: example7.cpp sevenzip.h libsevenzip.a
example8: example8.cpp sevenzip.h libsevenzip.a

example: example.cpp sevenzip.h libsevenzip.a
exampleH: exampleH.cpp sevenzip.h libsevenzip.a
	$(CXX17) $(CXXFLAGS) $(OBJS) -o $@ $< libsevenzip.a

example_dir:
	@echo "Example directory prepared."
	-@mkdir -p temps
	-@rm temps/example* >> /dev/null || true
	@cd temps && seq 64 | sed -E 's/.+/0123456789ABCDEF/' > example.txt
	@cd temps && 7zz a example1.7z example.txt >> /dev/null
	@cd temps && 7zz a example2.7z example.txt >> /dev/null
	@cd temps && 7zz a -pexample3 example3.7z example.txt >> /dev/null
	@cd temps && 7zz a -mx0 -v500 example4.7z example.txt >> /dev/null	
	@cd temps && 7zz a example5.7z example.txt >> /dev/null
	@cd temps && cp example.txt example6.txt >> /dev/null
	@cd temps && cp example.txt example7.txt >> /dev/null
	@cd temps && cp example.txt example8.txt >> /dev/null
	@cd temps && rm example.txt >> /dev/null
	
test: examples
	@for ex in $(EXAMPLES); do \
	    echo -n "Running test on $$ex ... "; \
	    LD_LIBRARY_PATH=$(SEVENZIPPATH) \
	    DYLD_LIBRARY_PATH=$(SEVENZIPPATH) \
	    ./$$ex 2>> _dev_null | grep "TEST"; \
	done

valgrind: examples
	@for ex in $(EXAMPLES); do \
	    echo -n "Running valgrind on $$ex ... "; \
	    LD_LIBRARY_PATH=$(SEVENZIPPATH) \
	    DYLD_LIBRARY_PATH=$(SEVENZIPPATH) \
	    valgrind --leak-check=full --show-leak-kinds=all ./$$ex 2>&1 | grep "ERROR SUMMARY"; \
	done

$O/sevenzip_impl.o: sevenzip.h sevenzip_compat.h sevenzip_impl.h sevenzip_impl.cpp
$O/sevenzip.o: sevenzip.h sevenzip_compat.h sevenzip_impl.h sevenzip.cpp

VPATH = examples:tests:$(SEVENZIPSRC)/CPP/Common:$(SEVENZIPSRC)/CPP/Windows

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< libsevenzip.a

$O/%.o: %.cpp | $O
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$O:
	mkdir -p $O
