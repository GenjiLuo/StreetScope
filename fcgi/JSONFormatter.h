//==============================================================================
// JSONFormatter.h
// Created August 27 2012
//==============================================================================

#ifndef JSON_FORMATTER
#define JSON_FORMATTER

#include "client/dbclient.h"


//==============================================================================
// Class JSONFormatter
//==============================================================================

class JSONFormatter {

public:
   mongo::BSONObj panorama (mongo::BSONObj panorama);
   mongo::BSONObj feature (mongo::BSONObj feature);
   mongo::BSONObj features (std::auto_ptr<mongo::DBClientCursor> features);
   mongo::BSONObj tag (mongo::BSONObj tag);
   mongo::BSONObj tags (std::auto_ptr<mongo::DBClientCursor> tags);
};

#endif // JSON_FORMATTER

