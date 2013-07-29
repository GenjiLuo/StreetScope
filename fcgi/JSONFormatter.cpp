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
   // if the panorama is empty there's nothing to be done
   if (panorama.isEmpty()) {
      return panorama;
   }

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
   bpano << "orientation" << BSON(
      "panoYaw" << panorama.getObjectField("orientation")["yaw"].Double()
      << "tiltYaw" << panorama.getObjectField("orientation")["tiltYaw"].Double()
      << "tiltPitch" << panorama.getObjectField("orientation")["tiltPitch"].Double()
   );

   // add edges
   mongo::BSONArrayBuilder array;
   mongo::BSONObjIterator edges(panorama.getObjectField("edges"));
   while (edges.more()) {
      array << edges.next();
   }
   bpano << "edges" << array.arr();

   return bpano.obj();
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
mongo::BSONObj JSONFormatter::tag (mongo::BSONObj tag) {
   return tag;
}

//------------------------------------------------------------------------------
mongo::BSONObj JSONFormatter::tags (std::auto_ptr<mongo::DBClientCursor> tags) {
   mongo::BSONObjBuilder object;
   mongo::BSONArrayBuilder array;

   while (tags->more()) {
      array << tag(tags->next());
   }
   object << "tags" << array.arr();

   return object.obj();
}

