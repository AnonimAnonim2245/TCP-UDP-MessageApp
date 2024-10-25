# Protocoale de comunicatii
# Laborator 7 - TCP
# Echo Server
# Makefile

# Compiler flags
CFLAGS = -Wall -g -Werror -Wno-error=unused-variable
CXXFLAGS = -Wall -g -std=c++11 -Werror -Wno-error=unused-variable

# Portul pe care asculta serverul
PORT = 12345

# Adresa IP a serverului
IP_SERVER = 192.168.0.2

# Targets
all: server client

# Rule for common.o - compiling common.c to object file
common.o: common.cpp
	$(CC) $(CFLAGS) -c common.cpp -o common.o

# Rule for helpers.o - compiling helpers.cpp (assuming you need this for your project)
helpers.o: helpers.cpp
	$(CXX) $(CXXFLAGS) -c helpers.cpp -o helpers.o

# Compile the server - linking server.c with common.o and helpers.o
server: server.cpp common.o helpers.o
	$(CXX) $(CXXFLAGS) server.cpp common.o helpers.o -o server

# Compile the client - linking client.c with common.o
subscribe: client.cpp common.o
	$(CXX) $(CXXFLAGS) client.cpp common.o -o client

.PHONY: clean run_server run_client

# Rule to run the server
run_server:
	./server ${PORT}

# Rule to run the client 	
run_client:
	./subscribe ${IP_SERVER} ${PORT}

# Clean up generated files
clean:
	rm -rf server client *.o *.dSYM