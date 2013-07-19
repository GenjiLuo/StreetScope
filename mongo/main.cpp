#include <iostream>
#include "client/dbclient.h"
#include "Database.h"

using namespace mongo;

int main() {

   Database database;
   if (database.connect()) {
      cout << "Connected to MongoDB.\n";
   } else {
      cout << "Error connecting to the database. Aborting.\n";
      return 0;
   }
   database.ensureIndexes();

   Panorama pano;
   pano.location.lon = 10;
   pano.location.lat = 11;
   pano.originalLocation.lat = 42.4;
   pano.originalLocation.lon = -67.4;
   pano.panoid = "test pano 2";
   pano.captureDate = time(0);
   pano.yaw.a = .12;
   pano.tiltYaw.a = .2;
   pano.tiltPitch.a = .3;

   //Feature feature;
   //feature.name = "person";

   AngleBox tag1;
   tag1.theta1.a = 1.1;
   tag1.phi1.a = 1.2;
   tag1.theta2.a = 1.3;
   tag1.phi2.a = 1.4;

   Location loc1;
   loc1.lon = 10;
   loc1.lat = 10;

   cout << "panos: " << database.panoramas() << '\n';
   cout << "panorama directory: " << database.panoramaDirectory() << '\n';

   //database.insertFeature(feature);
   //mongo::OID newid = database.insertPanorama(pano);
   //database.insertEdge(newid, 0.123, "edge edge");
   //database.insertTag(OID("50368a87f3f742496dc124c7"), OID("50367f136dd024d9a1d5072b"), tag1);

   std::string idstring = "50649538a74375bd11776d9f";
   PanoramaID idobj(idstring);
   cout << "Looking for a panorama with id " << idstring << ".\n";
   cout << database.findPanorama(idobj).jsonString() << '\n';

   Location testloc(-71.1166, 42.378741);
   cout << "Looking for a panorama near " << testloc.lat << ", " << testloc.lon << ".\n";
   cout << database.findPanorama(testloc).jsonString() << '\n';

   char const* panoid = "JTDCjycbFYmc_wEwID7qMA";
   cout << "Looking for a panorama with panoid " << panoid << ".\n";
   cout << database.findPanorama(panoid).jsonString() << '\n';

   cout << "Empty object looks like: \"" << BSONObj().jsonString() << "\".\n";

   //cout << "new id: " << newid.str() << '\n';



   return 0;
}
