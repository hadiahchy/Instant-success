CC=g++
CC_FLAGS=--std=c++17
INCLUDE_DIRS=-I$(HOME)/lib/boost/boost_1_67_0/include/ -I$(CURDIR)/lib -I$(CURDIR)/include
LIB_FLAGS=-lpthread -L$(HOME)/lib/boost/boost_1_67_0/lib -lboost_system -lssl -lcrypto
BUILD_DIR=./build
SRC_DIR=./src

all: gen_ssl copy_ssl build

gen_ssl:
	@echo Generating SSL keys
	./create_certificate.sh

copy_ssl:
	@echo Copying SSL certificates...
	cp ./ssl/cert.pem $(BUILD_DIR)/cert.pem
	cp ./ssl/key.pem $(BUILD_DIR)/key.pem

websocketpp_client: $(SRC_DIR)/websocketpp_client.cpp copy_ssl
	@echo Building target $(BUILD_DIR)$@
	$(CC) $(CC_FLAGS) $(INCLUDE_DIRS) -o $(BUILD_DIR)/$@ $(SRC_DIR)/$@.cpp $(LIB_FLAGS)

websocketpp_server: $(SRC_DIR)/websocketpp_server.cpp copy_ssl
	@echo Building target $(BUILD_DIR)$@
	$(CC) $(CC_FLAGS) $(INCLUDE_DIRS) -o $(BUILD_DIR)/$@ $(SRC_DIR)/$@.cpp $(LIB_FLAGS)

websocketpp_chat_server: $(SRC_DIR)/websocketpp_chat_server.cpp copy_ssl
	@echo Building target $(BUILD_DIR)$@
	$(CC) $(CC_FLAGS) $(INCLUDE_DIRS) -o $(BUILD_DIR)/$@ $(SRC_DIR)/$@.cpp $(LIB_FLAGS)

websocketpp_chat_client: $(SRC_DIR)/websocketpp_chat_client.cpp copy_ssl
	@echo Building target $(BUILD_DIR)$@
	$(CC) $(CC_FLAGS) $(INCLUDE_DIRS) -o $(BUILD_DIR)/$@ $(SRC_DIR)/$@.cpp $(LIB_FLAGS)

build: websocketpp_client websocketpp_server websocketpp_chat_server websocketpp_chat_client

run_client:
	cd $(BUILD_DIR) && LD_LIBRARY_PATH=$(HOME)/lib/boost/boost_1_67_0/lib ./websocketpp_client

run_server:
	cd $(BUILD_DIR) && LD_LIBRARY_PATH=$(HOME)/lib/boost/boost_1_67_0/lib ./websocketpp_server

run_chat_server:
	cd $(BUILD_DIR) && LD_LIBRARY_PATH=$(HOME)/lib/boost/boost_1_67_0/lib ./websocketpp_chat_server

run_chat_client:
	cd $(BUILD_DIR) && LD_LIBRARY_PATH=$(HOME)/lib/boost/boost_1_67_0/lib ./websocketpp_chat_client

install_tools:
	@echo Installing platform tools
	./install_tools.sh

clean:
	@echo Running target $@
	rm -f $(BUILD_DIR)/websocketpp_client
	rm -f $(BUILD_DIR)/websocketpp_server
	rm -f $(BUILD_DIR)/websocketpp_chat_server
	rm -f $(BUILD_DIR)/websocketpp_chat_client
	rm -f $(BUILD_DIR)/cert.pem
	rm -f $(BUILD_DIR)/key.pem

.PHONY: all install_tools gen_ssl build run_client run_server run_chat_client run_chat_server clean
