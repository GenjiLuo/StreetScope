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
   // Basic Database Queries (ie plain wrapper around Database call)
   
   // cmd=panorama, expects variable id
   std::ostream& panorama (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=feature, expects variable id
   std::ostream& feature (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=features, expects no variables
   std::ostream& features (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=tagset, expects variables panorama and feature
   std::ostream& tagset (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=panorama_tagsets, expects variable panorama
   std::ostream& panoramaTagsets (std::ostream& os, cgicc::Cgicc const& cgi);


   //---------------------------------------------------------------------------
   // Smart Queries (some processing required after Database call)
   
   // cmd=panos_near, expects variables lat and lon
   std::ostream& panoramaNear (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=panos_in_range, expects variables lat1, lon1, lat2, lon2
   //std::ostream& panosInRange (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=pano_id_near, expects variables lat, lon, and pano_id
   std::ostream& panoramaByPanoid (std::ostream& os, cgicc::Cgicc const& cgi);


   //---------------------------------------------------------------------------
   // Inserts and Removals
   
   // cmd=download_pano, expects variable pano_id
   std::ostream& downloadPanorama (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=insert_tag, expects variables panorama, feature, t1, p1, t2, p2
   std::ostream& insertTag (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=remove_tag, expects variable tag
   std::ostream& removeTag (std::ostream& os, cgicc::Cgicc const& cgi);


private:
   pugi::xml_node prepareDocument (pugi::xml_document& doc);
   //void addResultStatus (pugi::xml_node& results, bool success);
   //unsigned getHexValue (cgicc::const_form_iterator const& formentry);
};

#endif // DATABASE_SERVER

