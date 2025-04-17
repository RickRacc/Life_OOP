
.DEFAULT_GOAL := all
SHELL         := bash

ifeq ($(shell uname -p), arm) # Apple M processor
    ASTYLE        := astyle
    BOOST         := /usr/local/include/boost
    CHECKTESTDATA := checktestdata
    CPPCHECK      := cppcheck
    CXX           := clang++
    DOCKER        := gcc-apple
    DOXYGEN       := doxygen
    GCOV          := llvm-cov gcov
    GTESTINC      := /usr/local/include/
    GTESTLIB      := /usr/local/lib/
    VALGRIND      :=
    CXXFLAGS      := --coverage -fsanitize=undefined -g -std=c++20 -Wall -Wextra -Wpedantic
    LDFLAGS       := -lgmp -lgmpxx -lgtest -lgtest_main
else ifeq ($(shell uname -p), i386) # Apple Intel processor
    ASTYLE        := astyle
    BOOST         := /usr/local/include/boost
    CHECKTESTDATA := checktestdata
    CPPCHECK      := cppcheck
    CXX           := clang++
    DOCKER        := gcc-intel
    DOXYGEN       := doxygen
    GCOV          := llvm-cov gcov
    GTESTINC      := /usr/local/include/
    GTESTLIB      := /usr/local/lib/
    VALGRIND      :=
    CXXFLAGS      := --coverage -fsanitize=undefined -g -std=c++20 -Wall -Wextra -Wpedantic
    LDFLAGS       := -lgmp -lgmpxx -lgtest -lgtest_main
else ifeq ($(shell uname -p), x86_64) # CS machines
    ASTYLE        := astyle
    BOOST         := /lusr/opt/boost-1.82/include/boost
    CHECKTESTDATA := checktestdata
    CPPCHECK      := cppcheck
    CXX           := g++-14
    DOCKER        :=
    DOXYGEN       := doxygen
    GCOV          := gcov-14
    GTESTINC      := /lusr/opt/googletest-1.15.2/include/gtest
    GTESTLIB      := /lusr/opt/googletest-1.15.2/lib
    VALGRIND      := valgrind
    CXXFLAGS      := --coverage -g -std=c++20 -Wall -Wextra -Wpedantic
    LDFLAGS       := -lgtest -lgtest_main -pthread
else ifeq ($(shell uname -p), unknown) # Docker
    ASTYLE        := astyle
    BOOST         := /usr/include/boost
    CHECKTESTDATA := checktestdata
    CPPCHECK      := cppcheck
    CXX           := g++
    DOCKER        := gcc-intel
    DOXYGEN       := doxygen
    GCOV          := gcov
    GTESTINC      := /usr/include/gtest
    GTESTLIB      := /usr/lib
    VALGRIND      := valgrind
    CXXFLAGS      := --coverage -fsanitize=undefined -g -std=c++20 -Wall -Wextra -Wpedantic
    LDFLAGS       := -lgtest -lgtest_main -pthread
endif

# cpp/files/Git-CS371p.mk

# run/test files, compile with make all
FILES :=               \
    run_LifeCell  \
	run_LifeConway \
	run_LifeFredkin \
    test_Life

# cpp/rules/Git-CS371p.mk

# run docker
docker:
	docker run -it -v '$(PWD)/..:/usr/gcc' -w /usr/gcc gpdowning/$(DOCKER)

# get git config
config:
	git config -l

# get git log
Life.log.txt:
	git log > Life.log.txt

# get git status
status:
	make --no-print-directory clean
	@echo
	git branch
	git remote -v
	git status

# download files from the Life code repo
pull:
	make --no-print-directory clean
	@echo
	git pull
	git status

# upload files to the Life code repo
push:
	make --no-print-directory clean
	@echo
	git add .gitignore
	git add .gitlab-ci.yml
	-git add Life.csv
	-git add Life.ctd.txt
	git add Life.hpp
	-git add Life.log.txt
	-git add html
	git add Makefile
	git add README.md
	git add run_LifeCell.cpp
	git add run_LifeConway.cpp
	git add run_LifeFredkin.cpp
	git add test_Life.cpp
	git commit -m "another commit"
	git push
	git status

# compile run harness
run_Life: Life.hpp run_LifeCell.cpp run_LifeConway.cpp run_LifeFredkin.cpp
	-$(CPPCHECK) run_LifeCell.cpp run_LifeConway.cpp run_LifeFredkin.cpp
	$(CXX) $(CXXFLAGS) run_LifeCell.cpp -o run_LifeCell
	$(CXX) $(CXXFLAGS) run_LifeConway.cpp -o run_LifeConway
	$(CXX) $(CXXFLAGS) run_LifeFredkin.cpp -o run_LifeFredkin
	

# compile test harness
test_Life: Life.hpp test_Life.cpp
	-$(CPPCHECK) test_Life.cpp
	$(CXX) $(CXXFLAGS) test_Life.cpp -o test_Life $(LDFLAGS)

# compile all
all: $(FILES)

# execute test harness with coverage
test: test_Life
	$(VALGRIND) ./test_Life
ifeq ($(shell uname -s), Life)
	$(GCOV) test_Life-test_Life.cpp | grep -B 2 "hpp.gcov"
else
	$(GCOV) test_Life.cpp | grep -B 2 "hpp.gcov"
endif

# clone the Life test repo
../cs371p-life-tests:
	git clone https://gitlab.com/gpdowning/cs371p-life-tests.git ../cs371p-life-tests

# generate a random input file
ctd-generate:
	$(CHECKTESTDATA) -g Life.ctd.txt >> Life.gen.txt

# execute the run harness against your test files in the Life test repo and diff with the expected output
# change gpdowning to your GitLab-ID
run: run_Life ../cs371p-life-tests
	-$(CHECKTESTDATA) Life.ctd.txt ../cs371p-life-tests/rrrakesh-LifeCell.in.txt
	./run_LifeCell < ../cs371p-life-tests/rrrakesh-LifeCell.in.txt > Life.tmp.txt
	diff Life.tmp.txt ../cs371p-life-tests/rrrakesh-LifeCell.out.txt

	-$(CHECKTESTDATA) Life.ctd.txt ../cs371p-life-tests/rrrakesh-LifeConway.in.txt
	./run_LifeConway < ../cs371p-life-tests/rrrakesh-LifeConway.in.txt > Life.tmp.txt
	diff Life.tmp.txt ../cs371p-life-tests/rrrakesh-LifeConway.out.txt

	-$(CHECKTESTDATA) Life.ctd.txt ../cs371p-life-tests/rrrakesh-LifeFredkin.in.txt
	./run_LifeFredkin < ../cs371p-life-tests/rrrakesh-LifeFredkin.in.txt > Life.tmp.txt
	diff Life.tmp.txt ../cs371p-life-tests/rrrakesh-LifeFredkin.out.txt


# execute the run harness against all of the test files in the Allocator test repo and diff with the expected output
run-all: run_Life
	-@time for v in ../cs371p-life-tests/*-LifeConway.in.txt; do \
		echo $(CHECKTESTDATA) Life.ctd.txt $$v; \
		     $(CHECKTESTDATA) Life.ctd.txt $$v; \
		echo ./run_LifeConway \< $$v \> tmp.txt; \
		     ./run_LifeConway < $$v > tmp.txt; \
		echo diff tmp.txt $${v/.in.txt/.out.txt}; \
		     diff tmp.txt $${v/.in.txt/.out.txt}; \
	done

	-@time for v in ../cs371p-life-tests/*-LifeFredkin.in.txt; do \
		echo $(CHECKTESTDATA) Life.ctd.txt $$v; \
		     $(CHECKTESTDATA) Life.ctd.txt $$v; \
		echo ./run_LifeFredkin \< $$v \> tmp.txt; \
		     ./run_LifeFredkin < $$v > tmp.txt; \
		echo diff tmp.txt $${v/.in.txt/.out.txt}; \
		     diff tmp.txt $${v/.in.txt/.out.txt}; \
	done

	-@time for v in ../cs371p-life-tests/*-LifeCell.in.txt; do \
		echo $(CHECKTESTDATA) Life.ctd.txt $$v; \
		     $(CHECKTESTDATA) Life.ctd.txt $$v; \
		echo ./run_LifeCell \< $$v \> tmp.txt; \
		     ./run_LifeCell < $$v > tmp.txt; \
		echo diff tmp.txt $${v/.in.txt/.out.txt}; \
		     diff tmp.txt $${v/.in.txt/.out.txt}; \
	done

	@rm -f tmp.txt


# auto format the code
format:
	$(ASTYLE) Life.hpp
	$(ASTYLE) run_LifeCell.cpp
	$(ASTYLE) run_LifeConway.cpp
	$(ASTYLE) run_LifeFredkin.cpp
	$(ASTYLE) test_Life.cpp

# you must edit Doxyfile and
# set EXTRACT_ALL     to YES
# set EXTRACT_PRIVATE to YES
# set EXTRACT_STATIC  to YES
# create Doxfile
Doxyfile:
	$(DOXYGEN) -g

# create html directory
html: Doxyfile
	$(DOXYGEN) Doxyfile

# check the existence of check files
check: .gitignore .gitlab-ci.yml Life.log.txt html

# remove executables and temporary files
clean:
	rm -f  *.gcda
	rm -f  *.gcno
	rm -f  *.gcov
	rm -f  *.gen.txt
	rm -f  *.tmp.txt
	rm -f  $(FILES)
	rm -rf *.dSYM

# remove executables, temporary files, and generated files
scrub:
	make --no-print-directory clean
	rm -f  Life.log.txt
	rm -f  Doxyfile
	rm -rf html
	rm -rf latex

# output versions of all tools
# cpp/Versions.mk

versions:
	uname -p

	@echo
	uname -s

	@echo
	which $(ASTYLE)
	@echo
	$(ASTYLE) --version

	@echo
	which $(CHECKTESTDATA)
	@echo
	$(CHECKTESTDATA) --version | head -n 1

	@echo
	which cmake
	@echo
	cmake --version | head -n 1

	@echo
	which $(CPPCHECK)
	@echo
	$(CPPCHECK) --version

	@echo
	which $(DOXYGEN)
	@echo
	$(DOXYGEN) --version

	@echo
	which $(CXX)
	@echo
	$(CXX) --version | head -n 1
	@echo
	$(CXX) -std=c++2b -xc++ -E -dM /dev/null | grep __cplusplus

	@echo
	which $(GCOV)
	@echo
	$(GCOV) --version | head -n 1

	@echo
	which git
	@echo
	git --version

	@echo
	which make
	@echo
	make --version | head -n 1

ifneq ($(VALGRIND),)
	@echo
	which $(VALGRIND)
	@echo
	$(VALGRIND) --version
endif

	@echo
	which vim
	@echo
	vim --version | head -n 1

	@echo
	grep "#define BOOST_LIB_VERSION " $(BOOST)/version.hpp

	@echo
	ls -dl $(GTESTINC)/gtest.h
	@echo
	ls -dl $(GTESTLIB)/libgtest*.a
	@echo
	pkg-config --modversion gtest

