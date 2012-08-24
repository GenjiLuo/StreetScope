//==============================================================================
// Database.cpp
// Created 1/23/12.
//==============================================================================

#include <iostream>
#include <sstream>
#include <fstream>
#include "Database.h"

using namespace std;
using namespace mongo;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
bool Database::connect () {
   try {
      _mongo.connect("localhost");
   } catch( DBException &e ) {
      cout << "caught " << e.what() << endl;
      return false;
   }
   return true;
}

//------------------------------------------------------------------------------
void Database::setPanoramaDirectory (std::string const& panoDir) {
   _panoDir = panoDir;
}

//------------------------------------------------------------------------------
void Database::ensureIndexes () {
   _mongo.ensureIndex(panoramaCollection, BSON("location" << "2d"));
   _mongo.ensureIndex(panoramaCollection, BSON("panoid" << 1));
   _mongo.ensureIndex(tagsetCollection, BSON("panorama" << 1 << "feature" << 1));
   _mongo.ensureIndex(tagsetCollection, BSON("feature" << 1));
   _mongo.ensureIndex(tagsetCollection, BSON("tags._id" << 1));
}

//------------------------------------------------------------------------------
string Database::panoramaPath (OID panoramaID) const {
   ostringstream path;
   path << _panoDir << panoramaID.toString() << ".jpg";
   return path.str();
}

//------------------------------------------------------------------------------
OID Database::insertPanorama (Panorama const& panorama) {
   // make sure we don't have this panorama already
   // ...

   // construct the BSON object
   BSONObjBuilder pano;
   BSONArrayBuilder location;
   BSONArrayBuilder originalLocation;
   BSONObjBuilder orientation;

   location << panorama.location.lon;
   location << panorama.location.lat;
   originalLocation << panorama.originalLocation.lon;
   originalLocation << panorama.originalLocation.lat;
   orientation << "yaw" << panorama.yaw.rad();
   orientation << "tiltYaw" << panorama.tiltYaw.rad();
   orientation << "tiltPitch" << panorama.tiltPitch.rad();

   pano.genOID();
   pano << "insertDate" << DATENOW;
   pano << "captureDate" << Date_t(panorama.captureDate);
   pano << "location" << location.arr();
   pano << "originalLocation" << originalLocation.arr();
   pano << "panoid" << panorama.panoid;
   pano << "orientation" << orientation.obj();
   pano << "edges" << BSONArray();

   BSONObj p = pano.obj();
   _mongo.insert(panoramaCollection, p);
   return p.getField("_id").OID();
}

//------------------------------------------------------------------------------
OID Database::insertFeature (Feature const& feature) {
   BSONObjBuilder feat;
   feat.genOID();
   feat << "name" << feature.name;
   BSONObj f = feat.obj();
   _mongo.insert(featureCollection, f);
   return f.getField("_id").OID();
}

//------------------------------------------------------------------------------
void Database::insertEdge (OID panoramaID, Angle angle, char const* panoid) {
   _mongo.update(
      panoramaCollection,
      BSON( "_id" << panoramaID ),
      BSON( "$push" << BSON( "edges" << BSON( "panoid" << panoid << "yaw" << angle.rad() ) ) )
   );
}

//------------------------------------------------------------------------------
OID Database::insertTag (OID panoramaID, OID featureID, AngleBox const& box) {
   _mongo.update(
      tagsetCollection,
      BSON( "panorama" << panoramaID << "feature" << featureID ),
      BSON( "$push" << BSON( "tags" << BSON( GENOID << "box" << BSON(
         "theta1" << box.theta1.rad()
         << "phi1" << box.phi1.rad()
         << "theta2" << box.theta2.rad()
         << "phi2" << box.phi2.rad()
      ) ) ) ),
      true
   );
   return OID();
}

//------------------------------------------------------------------------------
BSONObj Database::findPanorama (OID panoramaID) {
   BSONObj query = BSON( "_id" << panoramaID );

   auto_ptr<DBClientCursor> cursor = _mongo.query( panoramaCollection, query );
   if (cursor->more()) {
      return cursor->next();
   }
   return BSONObj();
}

//------------------------------------------------------------------------------
BSONObj Database::findPanorama (char const* panoid) {
   BSONObj query = BSON( "panoid" << panoid );

   auto_ptr<DBClientCursor> cursor = _mongo.query( panoramaCollection, query );
   if (cursor->more()) {
      return cursor->next();
   }
   return BSONObj();
}

//------------------------------------------------------------------------------
BSONObj Database::findPanorama (Location location) {
   BSONObj query = BSON( "location" << BSON(
      "$nearSphere" << BSONArrayBuilder().append(location.lon).append(location.lat).arr()
      << "$maxDistance" << 0.003   // 20 meters / radius of earth = 0.003
   ) );

   auto_ptr<DBClientCursor> cursor = _mongo.query( panoramaCollection, query );
   if (cursor->more()) {
      return cursor->next();
   }
   return BSONObj();
}

//------------------------------------------------------------------------------
BSONObj Database::findFeature (OID featureID) {
   BSONObj query = BSON( "_id" << featureID );

   auto_ptr<DBClientCursor> cursor = _mongo.query( featureCollection, query );
   if (cursor->more()) {
      return cursor->next();
   }
   return BSONObj();
}

//------------------------------------------------------------------------------
std::auto_ptr<mongo::DBClientCursor> Database::findTagSets (mongo::OID panoramaID) {
   BSONObj query = BSON( "panorama" << panoramaID );
   return _mongo.query( tagsetCollection, query );
}

//------------------------------------------------------------------------------
void Database::removePanorama (mongo::OID panoramaID) {
   _mongo.remove( panoramaCollection, BSON( "_id" << panoramaID ) );
}

//------------------------------------------------------------------------------
void Database::removeFeature (mongo::OID featureID) {
   _mongo.remove( featureCollection, BSON( "_id" << featureID ) );
}

//------------------------------------------------------------------------------
void Database::removeTag (mongo::OID tagID) {
   _mongo.update(
      tagsetCollection,
      BSON( "tags._id" << tagID ),
      BSON( "$pull" << BSON( "tags" << BSON( "_id" << tagID ) ) )
   );
}

