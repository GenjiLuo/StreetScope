//==============================================================================
// DatabaseTester.h
// Created July 17 2012
//==============================================================================

#ifndef DATABASE_TESTER
#define DATABASE_TESTER

#include "Database.h"
#include "Random.h"


//==============================================================================
// Struct DatabaseTester
//==============================================================================

struct DatabaseTester {
public:
   PhotoDatabase _db1;
   PhotoDatabase _db2;
   XorShift32 _rand;

public:
   DatabaseTester (unsigned initialCapacity): _db1(initialCapacity), _db2(initialCapacity) {}
   void setRootDir (std::string rootDir) { _db1.setRootDir(rootDir); _db2.setRootDir(rootDir); }
   void setPanoDir (std::string panoDir) { _db1.setPanoDir(panoDir); _db2.setPanoDir(panoDir); }
   void setRandState (unsigned x, unsigned y) { _db1.setRandState(x, y); _db2.setRandState(x, y); }
   void setTesterRandState (unsigned x, unsigned y) { _rand.setState(x, y); }
   void addRandomEntries (unsigned n);
   void addRandomTags (unsigned n);
   bool save1 ();
   bool load2 ();
   bool savePlaintext1 ();
   bool savePlaintext2 ();
};


#endif // DATABASE_TESTER
