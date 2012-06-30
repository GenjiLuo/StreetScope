# Makefile for Database

# compilation variables
CXX = g++
CXXFLAGS = -Wall -o3 -std=c++11
#CXXFLAGS = -Wall -g
LFLAGS = -lcurl `Magick++-config --ldflags --libs`

# convenience variables
odir = bin
libdir = lib

bare_objects = Database.o DatabaseXML.o PugiXML.o GoogleMaps.o Location.o PhotoTypes.o TagTypes.o Polyline.o SimpleCharPool.o MemoryPool.o
objects = $(patsubst %,$(odir)/%,$(bare_objects))


# rules
$(odir)/main : main.cpp $(objects)
	$(CXX) $(CXXFLAGS) -o bin/main main.cpp $(objects) $(LFLAGS)

lib : $(objects)
	ar vrs lib/libdatabase.a $(objects)

$(odir)/Database.o : Database.cpp Database.h PhotoTypes.h TagTypes.h Location.h QTree.hpp HashSet.hpp MemoryPool.h SimpleCharPool.h LinkedList.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(odir)/DatabaseXML.o : DatabaseXML.cpp DatabaseXML.h Database.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(odir)/PugiXML.o : pugixml.cpp pugixml.hpp pugiconfig.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

$(odir)/GoogleMaps.o : GoogleMaps.cpp GoogleMaps.h Database.h Polyline.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< `Magick++-config --cppflags --cxxflags`

$(odir)/Location.o : Location.cpp Location.h Hypergrid.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(odir)/PhotoTypes.o : PhotoTypes.cpp PhotoTypes.h Location.h Hypergrid.hpp TagTypes.h SimpleCharPool.h MemoryPool.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(odir)/TagTypes.o : TagTypes.cpp TagTypes.h LinkedList.hpp MemoryPool.h SimpleCharPool.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(odir)/Polyline.o : Polyline.cpp Polyline.h Location.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(odir)/SimpleCharPool.o : SimpleCharPool.cpp SimpleCharPool.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(odir)/MemoryPool.o : MemoryPool.cpp MemoryPool.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY : clean
clean :
	rm $(odir)/*
	rm $(libdir)/*

