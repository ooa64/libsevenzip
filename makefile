# makefile

PROJECT_VER_MAJOR = 1
PROJECT_VER_MINOR = 1

SEVENZIPSRC ?= ../7zip
SEVENZIPBIN ?= env PATH=$(PATH):$(CURDIR) \
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SEVENZIPPATH) \
	DYLD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SEVENZIPPATH) \
	7z
SEVENZIPPATH ?= $(CURDIR)

ifdef UNICODE
	CFLAGS += -DUNICODE -D_UNICODE
endif

ifdef DEBUG
	R = libsevenzip$(PROJECT_VER_MAJOR).$(PROJECT_VER_MINOR)d
	O = outputs/debug
	CFLAGS += -ggdb -O0
	SEVENZIPFLAGS += DEBUG_BUILD=1
else
	R = libsevenzip$(PROJECT_VER_MAJOR).$(PROJECT_VER_MINOR)
	O = outputs/release
endif

ifdef DEBUG_IMPL
	CFLAGS += -DDEBUG_IMPL
endif

ifdef ASAN
	CFLAGS += -fsanitize=address -fno-omit-frame-pointer -fno-common -g3
endif

TARGET = libsevenzip.a
EXAMPLES = example0 example1 example2 example3 example4 example5 example6 example7 example8 example9

CFLAGS += -fPIC -Wall -Wextra -I.
CFLAGS += -DPROJECT_VER_MAJOR=$(PROJECT_VER_MAJOR) -DPROJECT_VER_MINOR=$(PROJECT_VER_MINOR)
LDFLAGS +=

CXX17 ?= $(CXX) -std=c++17
CXXFLAGS += $(CFLAGS)

# NOTE: uncomment to build p7zip compatible 7z.so using 7-Zip SDK v.23+
# SEVENZIPFLAGS += LOCAL_FLAGS="-DZ7_USE_VIRTUAL_DESTRUCTOR_IN_IUNKNOWN"

# NOTE: workaround for addon codecs linking issue (p7zip)
SEVENZIPFLAGS += MY_LIBS="-L$(CURDIR)/$O/7zip/lib/7z_addon_codec"

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
	$O/ErrorMsg.o

all: $(TARGET)

release: example
	-@rm -fr $R 2>> /dev/null
	-@rm -fr $O/$R 2>> /dev/null	
	-@mkdir -p $O/$R/C
	cp -p README.md $O/$R
	cp -p DOCUMENTATION.md $O/$R
	cp -p sevenzip.h $O/$R
	cp -p C/7zVersion.h $O/$R/C
	cp -p C/7zTypes.h $O/$R/C
	cp -p libsevenzip.a $O/$R
	cp -p example $O/$R
	cp -p examples/example.cpp $O/$R
	test -f $(CURDIR)/7z.so && cp -p $(CURDIR)/7z.so $O/$R || true
	cd $O && $(SEVENZIPBIN) a $(CURDIR)/$R.zip $R > /dev/null
	               
clean:
	-rm -fr $O temps/example.txt

cleanall: clean
	-rm -f libsevenzip.a example[0-9H] example
	-rm -fr C temps example[0-9H].dSYM example.dSYM

libsevenzip.a: $(OBJS)
	ar rcs $@ $^
	-@mkdir -p C
	@cp -p $(SEVENZIPSRC)/C/7zVersion.h C/7zVersion.h	
	@cp -p $(SEVENZIPSRC)/C/7zTypes.h C/7zTypes.h		

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
exampleT: exampleT.cpp sevenzip.h libsevenzip.a
exampleH: exampleH.cpp sevenzip.h libsevenzip.a
	$(CXX17) $(CXXFLAGS) $(OBJS) -o $@ $< libsevenzip.a

example_dir:
	-@rm -rf temps/example* >> /dev/null
	-@mkdir -p temps/example
	@cd temps && seq 64 | sed -E 's/.+/0123456789ABCDEF/' > example.txt
	@cd temps && $(SEVENZIPBIN) a example1.7z example.txt >> /dev/null
	@cd temps && $(SEVENZIPBIN) a example2.7z example.txt >> /dev/null
	@cd temps && $(SEVENZIPBIN) a -pexample3 example3.7z example.txt >> /dev/null
	@cd temps && $(SEVENZIPBIN) a -mx0 -v500 example4.7z example.txt >> /dev/null	
	@cd temps && $(SEVENZIPBIN) a example5.7z example.txt >> /dev/null
	@cd temps && cp example.txt example6.txt >> /dev/null
	@cd temps && cp example.txt example7.txt >> /dev/null
	@cd temps && cp example.txt example8.txt >> /dev/null
	@cd temps && $(SEVENZIPBIN) a example9.7z example.txt example >> /dev/null
	@cd temps && rm -rf example.txt example/ >> /dev/null

examples: $(EXAMPLES) example_dir
	@for ex in $(EXAMPLES); do \
	    echo -n "Running test on $$ex ... "; \
	    LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SEVENZIPPATH) \
	    DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):$(SEVENZIPPATH) \
	    ./$$ex 2>> /dev/null | grep "TEST"; \
	done

valgrind: $(EXAMPLES) example_dir
	@for ex in $(EXAMPLES); do \
	    echo -n "Running valgrind on $$ex ... "; \
	    LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SEVENZIPPATH) \
	    DYLD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SEVENZIPPATH) \
	    valgrind --leak-check=full --show-leak-kinds=all ./$$ex 2>&1 | grep "ERROR SUMMARY"; \
	done

leaks: $(EXAMPLES) example_dir
	@for ex in $(EXAMPLES); do \
	    echo "Running leaks on $$ex ... "; \
	    LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SEVENZIPPATH) \
	    DYLD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SEVENZIPPATH) \
	    leaks -list -quiet -atExit -- ./$$ex 2>$O/leaks.log >$O/leaks.lst; \
	    grep "^Process .* leak" <$O/leaks.lst; \
	done

tests/tests: tests/tests.cpp tests/test_*.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ libsevenzip.a

tests: tests/tests
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SEVENZIPPATH) \
	DYLD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(SEVENZIPPATH) \
		./tests/tests

$O/sevenzip_impl.o: sevenzip.h sevenzip_compat.h sevenzip_impl.h sevenzip_impl.cpp

$O/sevenzip.o: sevenzip.h sevenzip_compat.h sevenzip_impl.h sevenzip.cpp

7zip: 7z.so
7z.so:
	cd $(SEVENZIPSRC)/CPP/7zip/Bundles/Format7zF && make -f makefile.gcc $(SEVENZIPFLAGS) O=$(CURDIR)/$O/7zip
	-test -f $O/7zip/7z.so && cp -p $O/7zip/7z.so 7z.so
	-test -f $O/7zip/lib/7z.so && cp -p $O/7zip/lib/7z.so 7z.so
	-test -d $O/7zip/lib/7z_addon_codec && cp -rp $O/7zip/lib/7z_addon_codec 7z_addon_codec

7z: 
	cd $(SEVENZIPSRC)/CPP/7zip/UI/Console && make -f makefile.gcc $(SEVENZIPFLAGS) O=$(CURDIR)/$O/7zip
	-test -f $O/7zip/7z && cp -p $O/7zip/7z 7z && chmod +x 7z
	-test -f $O/7zip/lib/7z && cp -p $O/7zip/lib/7z 7z && chmod +x 7z

VPATH = examples:tests:$(SEVENZIPSRC)/CPP/Common:$(SEVENZIPSRC)/CPP/Windows

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< libsevenzip.a

$O/%.o: %.cpp | $O
	$(CXX) -c $(CXXFLAGS) -I$(SEVENZIPSRC) -o $@ $<

$O:
	mkdir -p $O
