# Makefile for StreetViewProject/TrainingData/Server/fcgi

# compilation variables
CXX = g++
CXXFLAGS  = -Wall -o3
FCGI_LIBS = -lcgicc -lfcgi++ -lfcgi
INCLUDES  = -I/usr/include/cgicc -Iexternal -I$(databasedir)
IMONGO = -I/usr/local/include/mongo
LFLAGS = -lcurl `Magick++-config --ldflags --libs`
LMONGO = -lmongoclient -lboost_system -lboost_thread -lboost_filesystem -lboost_program_options

# convenience variables
odir = bin
databasedir = ../mongo

bare_objects = PugiXML.o BasicHTML.o DatabaseServer.o JSONFormatter.o #SessionData.o 
objects = $(patsubst %,$(odir)/%,$(bare_objects))


# rules
$(odir)/main : $(odir)/main.o $(objects) external/FCgiIO.o $(databasedir)/lib/libdatabase.a
	$(CXX) $(CXXFLAGS) -o bin/main.fcgi bin/main.o $(objects) external/FCgiIO.o $(FCGI_LIBS) -ldatabase $(LFLAGS) $(LMONGO) -I$(databasedir) -L$(databasedir)/lib

$(odir)/main.o : main.cpp $(databasedir)/Database.h BasicHTML.h SessionData.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES) $(IMONGO) -I/usr/include/fastcgi

$(odir)/PugiXML.o : pugixml.cpp pugixml.hpp pugiconfig.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

$(odir)/BasicHTML.o : BasicHTML.cpp BasicHTML.h SessionData.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

$(odir)/DatabaseServer.o : DatabaseServer.cpp DatabaseServer.h $(databasedir)/Database.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES) $(IMONGO)

$(odir)/JSONFormatter.o : JSONFormatter.cpp JSONFormatter.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES) $(IMONGO)

$(odir)/SessionData.o : SessionData.cpp SessionData.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

external/FCgiIO.o : external/FCgiIO.cpp external/FCgiIO.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES) -I/usr/include/fastcgi

.PHONY : clean
clean :
	rm $(odir)/*

