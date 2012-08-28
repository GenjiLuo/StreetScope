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
   mongo::BSONObj metadata (mongo::BSONObj panorama);
   mongo::BSONObj tagSets (std::auto_ptr<mongo::DBClientCursor> tagsets);
   mongo::BSONObj metadataAndTagSets (mongo::BSONObj panorama, std::auto_ptr<mongo::DBClientCursor> tagsets);


};

#endif // JSON_FORMATTER

