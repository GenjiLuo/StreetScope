//==============================================================================
// DatabaseServer.h
// Created May 31 2012
//==============================================================================

#ifndef DATABASE_SERVER
#define DATABASE_SERVER

#include <iostream>

#include "Cgicc.h"
#include "Database.h"
#include "JSONFormatter.h"
#include "pugixml.hpp"


//==============================================================================
// Class DatabaseServer
//==============================================================================

class DatabaseServer {
private:
   Database& _db;
   JSONFormatter _json;

public:
   DatabaseServer (Database& db): _db(db) {}

   // cmd=status
   std::ostream& status (std::ostream& os, cgicc::Cgicc const& cgi, bool failure);


   //---------------------------------------------------------------------------
   // Queries
   
   // cmd=panorama, expects variable id
   std::ostream& panorama (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=panorama_near, expects variables lat and lon
   std::ostream& panoramaNear (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=panorama_by_panoid, expects variable panoid
   std::ostream& panoramaByPanoid (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=feature, expects variable id
   std::ostream& feature (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=features, expects no variables
   std::ostream& features (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=tags, expects variables panorama and feature
   std::ostream& tags (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=tags_by_panorama, expects variable panorama
   std::ostream& tagsByPanorama (std::ostream& os, cgicc::Cgicc const& cgi);


   //---------------------------------------------------------------------------
   // Updates

   // cmd=change_tag_feature, expects variables tag and feature
   std::ostream& changeTagFeature (std::ostream& os, cgicc::Cgicc const& cgi);

   
   //---------------------------------------------------------------------------
   // Inserts and Removals
   
   // cmd=download_pano, expects variable panoid
   std::ostream& downloadPanorama (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=insert_tag, expects variables panorama, feature, t1, p1, t2, p2
   std::ostream& insertTag (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=remove_tag, expects variable id
   std::ostream& removeTag (std::ostream& os, cgicc::Cgicc const& cgi);


private:
   pugi::xml_node prepareDocument (pugi::xml_document& doc);
   //void addResultStatus (pugi::xml_node& results, bool success);
   //unsigned getHexValue (cgicc::const_form_iterator const& formentry);
};

#endif // DATABASE_SERVER

