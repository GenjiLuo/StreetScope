//==============================================================================
// DatabaseServer.h
// Created May 31 2012
//==============================================================================

#ifndef DATABASE_SERVER
#define DATABASE_SERVER

#include <iostream>

#include "Cgicc.h"
#include "Database.h"
#include "DatabaseXML.h"
#include "pugixml.hpp"


//==============================================================================
// Class DatabaseServer
//==============================================================================

class DatabaseServer {
private:
   PhotoDatabase& _db;
   DatabaseXML _xml;

public:
   DatabaseServer (PhotoDatabase& db): _db(db), _xml(db) {}

   // cmd=status
   std::ostream& status (std::ostream& os, cgicc::Cgicc const& cgi, bool failure);

   // cmd=metadata, expects variable panorama
   std::ostream& metadata (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=panos_near, expects variables lat and lon
   std::ostream& panoramaNear (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=panos_in_range, expects variables lat1, lon1, lat2, lon2
   //std::ostream& panosInRange (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=pano_id_near, expects variables lat, lon, and pano_id
   std::ostream& panoramaByPanoid (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=download_pano, expects variable pano_id
   std::ostream& downloadPanorama (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=new_tag, expects variables id, t1, p1, t2, p2
   //std::ostream& newTag (std::ostream& os, cgicc::Cgicc const& cgi);

   // cmd=remove_tag, expects variable tag_id
   //std::ostream& removeTag (std::ostream& os, cgicc::Cgicc const& cgi);


private:
   pugi::xml_node prepareDocument (pugi::xml_document& doc);
   //void addResultStatus (pugi::xml_node& results, bool success);
   //unsigned getHexValue (cgicc::const_form_iterator const& formentry);
};

#endif // DATABASE_SERVER

