CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

SRC = src/Page.cpp \
      src/Block.cpp \
      src/SSD.cpp \
      src/HDD.cpp \
      src/Cache.cpp \
      src/Statistics.cpp \
      src/Controller.cpp \
      src/TestSuite.cpp \
      src/main.cpp

ssd_sim: $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o ssd_sim

clean:
	rm -f ssd_sim
	rm -f data/ssd_state.bin
