#include <iostream>
#include <iomanip>
#include <omp.h>
#include <fstream>

#include "Database.h"
#include "DatabaseXML.h"
#include "QTree.hpp"
#include "Hypergrid.hpp"
#include "GoogleMaps.h"
#include "DatabaseTester.h"

#include "Polyline.h"
#include <ctime>

#include <sstream>

using namespace std;


int main (int argc, char * const argv[]) {

   double time1 = omp_get_wtime();
   
 
   
   unsigned n = 10000;
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
