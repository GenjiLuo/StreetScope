//==============================================================================
// DatabaseTester.cpp
// Created July 17, 2012
//==============================================================================

#include "DatabaseTester.h"
#include <sstream>

using namespace std;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
void DatabaseTester::addRandomEntries (unsigned n) {
   Location loc;
   char panoid[32];
   Location origLoc;
   tm capDateRaw;
   time_t capDate;
   Angle panoYaw;
   Angle tiltYaw;
   Angle tiltPitch;

   panoid[31] = '\0';
   capDateRaw.tm_sec   = 0;
   capDateRaw.tm_min   = 0;
   capDateRaw.tm_hour  = 0;
   capDateRaw.tm_mday  = 1;
   capDateRaw.tm_isdst = 0;

   for (unsigned i=0; i<n; ++i) {
      loc.lat() = _rand.f64() * 180.0 - 90.0;
      loc.lon() = _rand.f64() * 360.0 - 180.0;
      // approximate panoid as 30 random lowercase letters
      for (unsigned j=0; j<30; ++j) {
         panoid[j] = 'a' + (_rand.u32() % 26);
      }
      origLoc.lat() = _rand.f64() * 180.0 - 90.0;
      origLoc.lon() = _rand.f64() * 360.0 - 180.0;
      // pick a random month and year
      capDateRaw.tm_mon   = static_cast<int>(_rand.u32() % 12u);
      capDateRaw.tm_year  = static_cast<int>(100u + _rand.u32() % 13u);
      capDate = mktime(&capDateRaw);
      panoYaw = _rand.f64() * 360.0;
      tiltYaw = _rand.f64() * 360.0;
      tiltPitch = _rand.f64() * 180.0 - 90.0;

      _db1.addNewMetaData(loc, panoid, origLoc, capDate, panoYaw, tiltYaw, tiltPitch);
   }
}

//------------------------------------------------------------------------------
void DatabaseTester::addRandomTags (unsigned n) {
   unsigned ntags;
   Target target = Target::Trash;
   Angle theta1;
   Angle phi1;
   Angle theta2;
   Angle phi2;

   // n is used as a modulus (_rand.u32() % n) so if we ever want to get exactly
   // n tags we actually want to mod out by n+1.
   ++n;

   HashSet<PhotoMetadata, MemoryPoolF>::Iterator itr = _db1._metadata.iterator();
   for ( ; itr.valid(); ++itr) {
      ntags = _rand.u32() % n;
      for (unsigned i=0; i<ntags; ++i) {
         theta1 = _rand.f64() * 360.0;
         phi1   = _rand.f64() * 180.0 - 90.0;
         theta2 = _rand.f64() * 360.0;
         phi2   = _rand.f64() * 180.0 - 90.0;
         _db1.addTag(itr.ref().id(), target, theta1, phi1, theta2, phi2);
      }
   }
}

//------------------------------------------------------------------------------
void DatabaseTester::deleteRandomTags (unsigned n) {
   unsigned nTags;
   unsigned min;
   unsigned nDelete;
   unsigned index;

   // n is used as a modulus (_rand.u32() % n) so if we ever want to delete exactly
   // n tags we actually want to mod out by n+1.
   ++n;

   // for each photo in the database...
   HashSet<PhotoMetadata, MemoryPoolF>::Iterator itr = _db1._metadata.iterator();
   for ( ; itr.valid(); ++itr) {
      // choose a number of tags to remove
      TagSet& tags = itr.ref().tags();
      nTags = tags.items();
      if (nTags == 0) continue;
      min = (nTags < n) ? nTags : n;
      nDelete = _rand.u32() % min;
      cout << hex << itr.cref().id() << dec << ": " << nDelete << '\n';

      for (unsigned i=0; i<nDelete; ++i) {
         // move a random number of steps down the linked list of tags
         nTags = tags.items();
         index = _rand.u32() % nTags;
         TagSet::Iterator tagitr = tags.iterator();
         for (unsigned j=0; j<index; ++j) {
            ++tagitr;
         }
         _db1.removeTag(tagitr.cref().tagID());
      }
   }
}

//------------------------------------------------------------------------------
bool DatabaseTester::save1 () {
   if (_db1.saveDatabase()) {
      cout << "Saved database one (binary).\n";
      return true;
   }
   cout << "Failed to save database one (binary).\n";
   return false;
}

//------------------------------------------------------------------------------
bool DatabaseTester::load2 () {
   if (_db2.loadDatabase()) {
      cout << "Loaded database two.\n";
      return true;
   }
   cout << "Failed to load database two.\n";
   return false;
}

//------------------------------------------------------------------------------
bool DatabaseTester::savePlaintext1 () {
   ostringstream name;
   name << "test1_" << _save1++;
   if (_db1.savePlaintext(name.str().c_str())) {
      cout << "Saved database one (plaintext).\n";
      return true;
   }
   cout << "Failed to save database one (plaintext).\n";
   return false;
}

//------------------------------------------------------------------------------
bool DatabaseTester::savePlaintext2 () {
   ostringstream name;
   name << "test2_" << _save2++;
   if (_db2.savePlaintext(name.str().c_str())) {
      cout << "Saved database two (plaintext).\n";
      return true;
   }
   cout << "Failed to save database two (plaintext).\n";
   return false;
}


