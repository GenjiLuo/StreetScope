//==============================================================================
// DatabaseXML.h
// Created May 31 2012
//==============================================================================

#ifndef DATABASE_XML
#define DATABASE_XML

#include "Database.h"
#include "pugixml.hpp"


//==============================================================================
// Classes
//==============================================================================

//------------------------------------------------------------------------------
class DatabaseXML {
private:
   PhotoDatabase const& _db;

public:
   DatabaseXML (PhotoDatabase const& db): _db(db) {}

   pugi::xml_node& timestampXML (pugi::xml_node& node, time_t const* time, char const* name);
   pugi::xml_node& locationXML (pugi::xml_node& node, Location const& l, char const* name);
   pugi::xml_node& tagXML (pugi::xml_node& node, Tag const& tag);
   pugi::xml_node& edgeXML (pugi::xml_node& node, Edge const& edge);
   pugi::xml_node& photoMetadataXML (pugi::xml_node& node, PhotoMetadata const& pmd);
   pugi::xml_node& photoMetadataXML (pugi::xml_node& node, LinkedList<PhotoKey> const* keys);
};

//------------------------------------------------------------------------------
/*
struct PhotoMetadataXML {
   PhotoMetadata const& md;
   PhotoMetadataXML(PhotoMetadata const& photomd): md(photomd) {}
};
std::ostream& operator<< (std::ostream& os, PhotoMetadataXML& p);
*/


#endif // DATABASE_XML

