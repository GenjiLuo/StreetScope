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
   unsigned _save1;    // number of times _db1 has been saved to plaintext
   unsigned _save2;    // number of times _db2 has been saved to plaintext

public:
   DatabaseTester (unsigned initialCapacity): _db1(initialCapacity), _db2(initialCapacity), _save1(0), _save2(0) {}
   void setRootDir (std::string rootDir) { _db1.setRootDir(rootDir); _db2.setRootDir(rootDir); }
   void setPanoDir (std::string panoDir) { _db1.setPanoDir(panoDir); _db2.setPanoDir(panoDir); }
   void setRandState (unsigned x, unsigned y) { _db1.setRandState(x, y); _db2.setRandState(x, y); }
   void setTesterRandState (unsigned x, unsigned y) { _rand.setState(x, y); }

   // adds n panoramas (just metadata - no jpg) populated with random data
   void addRandomEntries (unsigned n);
   // adds between 0 and n random tags to each panorama
   void addRandomTags (unsigned n);
   // deletes between 0 and min{#tags, n} tags from each panorama
   void deleteRandomTags (unsigned n);

   bool save1 ();
   bool load2 ();
   bool savePlaintext1 ();
   bool savePlaintext2 ();
};


#endif // DATABASE_TESTER
