#include <iostream>
#include <iomanip>
#include <omp.h>
#include <fstream>

#include "Database.h"
#include "DatabaseXML.h"
#include "QTree.hpp"
#include "Hypergrid.hpp"
#include "GoogleMaps.h"

#include "Polyline.h"
#include <ctime>

#include <sstream>

using namespace std;


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

void DatabaseTester::addRandomTags (unsigned n) {
   unsigned ntags;
   Target target = Target::Trash;
   Angle theta1;
   Angle phi1;
   Angle theta2;
   Angle phi2;

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

bool DatabaseTester::save1 () {
   if (_db1.saveDatabase()) {
      cout << "Saved database one (binary).\n";
      return true;
   }
   cout << "Failed to save database one (binary).\n";
   return false;
}

bool DatabaseTester::load2 () {
   if (_db2.loadDatabase()) {
      cout << "Loaded database two.\n";
      return true;
   }
   cout << "Failed to load database two.\n";
   return false;
}

bool DatabaseTester::savePlaintext1 () {
   if (_db1.savePlaintext("test1")) {
      cout << "Saved database one (plaintext).\n";
      return true;
   }
   cout << "Failed to save database one (plaintext).\n";
   return false;
}

bool DatabaseTester::savePlaintext2 () {
   if (_db2.savePlaintext("test2")) {
      cout << "Saved database two (plaintext).\n";
      return true;
   }
   cout << "Failed to save database two (plaintext).\n";
   return false;
}

int main (int argc, char * const argv[]) {

   double time1 = omp_get_wtime();
   
 
   
   unsigned n = 100;
   unsigned maxTagsPerPhoto = 3;
   DatabaseTester tester(n);
   tester.setRootDir("/home/erik/Code/streetview/database/data/");
   tester.setPanoDir("/home/erik/Code/streetview/database/data/panos/");
   tester.setRandState(123, 456);
   tester.setTesterRandState(612, 1990);

   tester.addRandomEntries(n);
   tester.addRandomTags(maxTagsPerPhoto);
   tester.savePlaintext1();
   tester.save1();
   tester.load2();
   tester.savePlaintext2();



   /*
   unsigned n = 226;
   PhotoDatabase database(n);
   database.setRandState(123, 456);

   // correct paths for Erik's development machine
   database.setRootDir("/home/erik/Code/streetview/database/data/");
   database.setPanoDir("/home/erik/Code/streetview/database/panos");

   // correct paths for our Amazon machine
   //database.setRootDir("/home/ubuntu/streetview/data/");
   //database.setPanoDir("/var/www/panos/");

   ImageDownloader downer(&database, 128000);
   //*/






   /*
   //Location loc(49.791636, -87.596483);
   //Location loc(41.791636, -87.596483);
   Location loc(41.7967, -87.6063);
   char const* panoid = "ywB0rTqjQyQ71IbTQ_jOCw";
   downer.renderPanoramaURL(cout, panoid);
   cout << '\n';
   //downer.renderImageURL(cout, "RIBNzrWiOeoBRRGTjSWo6w", 0, 0, 2);
   //cout << '\n';

   cout << "Loading Database.\n";
   if (!database.loadDatabase()) {
      cout << "Failed to load the database.\n";
   }

   PhotoMetadata* newmd = 0;
   if ((newmd = downer.savePano(panoid, 3))) {
      DatabaseXML dbxml(database);
      pugi::xml_document doc;
      dbxml.photoMetadataXML(doc, *newmd);
      doc.save(cout);
   } else {
      cout << "Failure.\n";
   }
   return 0;
   //*/

   /*
   Panorama panorama;
   cout << '\n';

   char text[256];
   strcpy(text, "54th and maryland, chicago, il");
   strcpy(&text[100], "57th and university, chicago, il");
   if (downer.findRoute(text, &text[100])) {
      cout << "Garbled " << downer.downloadRoute(0.0001, false) << " images.\n";
   }
   cout << '\n';

   database.savePlaintext("plaintext.txt");

   cout << "Saving Database.\n";
   if (!database.saveDatabase())
      cout << "Failed to Save Database.\n";
   cout << '\n';
   //*/
   

   double time2 = omp_get_wtime();
   
   
   
   /*
   cout << "Loading Database.\n";
   if (!database.loadDatabase()) {
      cout << "Failed to load the database.\n";
   }
   cout << "The database contains " << database.size() << " photos.\n";
   database.savePlaintext("newplaintext.txt");

   
   QTree<PhotoKey, 2>::ConstIterator qitr = database._photokeys.constIterator();
   PhotoID pid = database.getMetadata(qitr.cref()).id();
   database.addTag(pid, Trash, 10, 20, 8, 15);
   DatabaseXML dbxml(database);

   float latsearch =  41.791420;
   float lonsearch = -87.598938;

   LinkedList<PhotoKey>* photos = database.findPanosNear( Location(latsearch, lonsearch) );
   pugi::xml_document doc;
   dbxml.photoMetadataXML(doc, photos);
   doc.save(cout);
   delete photos;
   //*/


   /*
   cout << "Here's the photos...\n";
   QTree<PhotoKey, 2>::ConstIterator it = database._photokeys.constIterator();
   while (it.valid()) {
      cout << it.cref().index() << '\n';
      ++it;
   }
   */

   /*
   float latlow  =  41.7912007;
   float lathigh =  41.7917126;
   float lonlow  = -87.6013112;
   float lonhigh = -87.5980926;
   
   LinkedList<PhotoKey>* photos = database.findPanos(latlow, lonlow, lathigh, lonhigh);
   pugi::xml_document doc;
   dbxml.photoMetadataXML(doc, photos);
   doc.save(cout);
   cout << '\n';
   cout << "Found " << photos->items() << " photos." << '\n';
   LinkedList<PhotoKey>::ConstIterator itr = photos->constIterator();
//   cout << "Adding a tag to photo " << itr.cref().id() << '\n';
//   database.addTag(itr.cref(), Pothole, 25, 40, 100, 80);
   while (itr.valid()) {
      cout << database.getMetadata(itr.cref()).id() << '\n';
      ++itr;
   }
   */

//   database.getMetadata(PhotoID(178u)).tags().leakAllTags();

//   database.dump("dump.txt");

//   cout << "Saving Database.\n";
//   if (!database.saveDatabase())
//      cout << "Failed to Save Database.\n";
//   cout << '\n';




   double time3 = omp_get_wtime();
   
   
   cout << "\nTiming Summary\n";
   cout << time2 - time1 << " seconds between times 1 and 2.\n";
   cout << time3 - time2 << " seconds between times 2 and 3.\n";
   cout << "Total runtime = " << time3 - time1 << '\n';
 
 
   return 0;
}
