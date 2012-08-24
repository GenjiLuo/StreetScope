# Makefile for Database

# compilation variables
CXX = g++
CXXFLAGS = -Wall -o3
#CXXFLAGS = -Wall -g -std=c++11
LFLAGS = -lmongoclient -lboost_system -lboost_thread -lboost_filesystem -lboost_program_options
MAGICK = `Magick++-config --cppflags --cxxflags --ldflags --libs`
LCURL = -lcurl
IMONGO = -I/usr/local/include/mongo

# convenience variables
odir = bin
libdir = lib


bare_objects = Database.o GoogleMaps.o Polyline.o SimpleCharPool.o PugiXML.o
objects = $(patsubst %,$(odir)/%,$(bare_objects))


# rules
$(odir)/main : main.cpp $(objects)
	$(CXX) $(CXXFLAGS) -o bin/main main.cpp $(objects) $(IMONGO) $(LFLAGS) $(LCURL) $(MAGICK)

$(odir)/Database.o : Database.cpp Database.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(IMONGO)

$(odir)/GoogleMaps.o : GoogleMaps.cpp GoogleMaps.h Database.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(IMONGO) $(MAGICK)

$(odir)/Polyline.o : Polyline.cpp Polyline.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(odir)/SimpleCharPool.o : SimpleCharPool.cpp SimpleCharPool.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(odir)/PugiXML.o : pugixml.cpp pugixml.hpp pugiconfig.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY : clean
clean :
	rm -v $(odir)/*
#	rm -v $(libdir)/*

