# Makefile for StreetViewProject/TrainingData/Server/fcgi

# compilation variables
CXX = g++
CXXFLAGS  = -Wall -o3 -std=c++11
FCGI_LIBS = -lcgicc -lfcgi++ -lfcgi
INCLUDES  = -I/usr/include/cgicc -Iexternal -I$(databasedir)
LFLAGS = -lcurl `Magick++-config --ldflags --libs`

# convenience variables
odir = bin
databasedir = ../database

bare_objects = PugiXML.o BasicHTML.o DatabaseServer.o SessionData.o 
objects = $(patsubst %,$(odir)/%,$(bare_objects))


# rules
$(odir)/main : $(odir)/main.o $(objects) external/FCgiIO.o $(databasedir)/lib/libdatabase.a
	$(CXX) $(CXXFLAGS) -o bin/main.fcgi bin/main.o $(objects) external/FCgiIO.o $(FCGI_LIBS) -ldatabase $(LFLAGS) -I$(databasedir) -L$(databasedir)/lib

$(odir)/main.o : main.cpp $(databasedir)/Database.h BasicHTML.h SessionData.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES) -I/usr/include/fastcgi

$(odir)/PugiXML.o : pugixml.cpp pugixml.hpp pugiconfig.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

$(odir)/BasicHTML.o : BasicHTML.cpp BasicHTML.h SessionData.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

$(odir)/DatabaseServer.o : DatabaseServer.cpp DatabaseServer.h $(databasedir)/Database.h $(databasedir)/DatabaseXML.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

$(odir)/SessionData.o : SessionData.cpp SessionData.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

external/FCgiIO.o : external/FCgiIO.cpp external/FCgiIO.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES) -I/usr/include/fastcgi

.PHONY : clean
clean :
	rm $(odir)/*

