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
mongo::BSONObj JSONFormatter::panorama (mongo::BSONObj panorama) {
   return panorama;

   /*
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
   */
}

//------------------------------------------------------------------------------
mongo::BSONObj JSONFormatter::feature (mongo::BSONObj feature) {
   return feature;
}

//------------------------------------------------------------------------------
mongo::BSONObj JSONFormatter::features (std::auto_ptr<mongo::DBClientCursor> features) {
   mongo::BSONObjBuilder object;
   mongo::BSONArrayBuilder array;

   while (features->more()) {
      array << feature(features->next());
   }
   object << "features" << array.arr();

   return object.obj();
}

//------------------------------------------------------------------------------
mongo::BSONObj JSONFormatter::tagset (mongo::BSONObj tagset) {
   return tagset;
}

//------------------------------------------------------------------------------
mongo::BSONObj JSONFormatter::tagsets (std::auto_ptr<mongo::DBClientCursor> tagsets) {
   mongo::BSONObjBuilder object;
   mongo::BSONArrayBuilder array;

   while (tagsets->more()) {
      array << tagset(tagsets->next());
   }
   object << "tagsets" << array.arr();

   return object.obj();
}

