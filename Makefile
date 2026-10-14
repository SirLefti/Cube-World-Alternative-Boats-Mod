CXX = g++

CXXFLAGS = -O2 -Wall
LDFLAGS = -shared -static -static-libgcc -static-libstdc++ -s

HEADERS = target_boat1.h target_boat3.h

.PHONY: all
all: AlternativeBoat1.dll AlternativeBoat3.dll

AlternativeBoat1.dll: main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -DTARGET_BOAT1 -o $@ $< $(LDFLAGS)

AlternativeBoat3.dll: main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -DTARGET_BOAT3 -o $@ $< $(LDFLAGS)

