CXXFLAGS=-Wall -g -Wextra -pedantic -std=c++20 -I. -g

all: ftp

ftp: main.cpp FTPClient.cpp FTPClient.hpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp FTPClient.cpp

clean:
	rm -f ftp

