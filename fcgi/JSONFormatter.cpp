//==============================================================================
// JSONFormatter.cpp
// Created August 27 2012
//==============================================================================

#include "JSONFormatter.h"
#include "BasicHTML.h"
#include <sstream>

using namespace std;
using namespace mongo;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
mongo::BSONObj JSONFormatter::metadata (mongo::BSONObj panorama) {
   BSONObjBuilder bpano;

   // add basic data
   bpano << "id" << panorama["_id"].OID().toString();
   bpano << "panoid" << panorama.getStringField("panoid");
   bpano << "location" << BSON(
      "lat" << panorama.getObjectField("location")[1].Double()
      << "lon" << panorama.getObjectField("location")[0].Double()
   );
   bpano << "originallocation" << BSON(
      "lat" << panorama.getObjectField("originalLocation")[1].Double()
      << "lon" << panorama.getObjectField("originalLocation")[0].Double()
   );
   bpano << "panoYaw" << panorama.getObjectField("orientation")["yaw"].Double();
   bpano << "tiltYaw" << panorama.getObjectField("orientation")["tiltYaw"].Double();
   bpano << "tiltPitch" << panorama.getObjectField("orientation")["tiltPitch"].Double();

   // add edges

   return bpano.obj();
}

//------------------------------------------------------------------------------
mongo::BSONObj JSONFormatter::tagSets (std::auto_ptr<mongo::DBClientCursor> tagsets) {
   mongo::BSONObjBuilder bdata;

   return bdata.obj();
}

//------------------------------------------------------------------------------
mongo::BSONObj JSONFormatter::metadataAndTagSets (mongo::BSONObj panorama, std::auto_ptr<mongo::DBClientCursor> tagsets) {
   mongo::BSONObjBuilder bdata;

   bdata << "panorama" << metadata(panorama);

   return bdata.obj();
}


