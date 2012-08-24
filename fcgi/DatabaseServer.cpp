//==============================================================================
// DatabaseServer.cpp
// Created May 31 2012
//==============================================================================

#include "DatabaseServer.h"
#include "BasicHTML.h"
#include "GoogleMaps.h"
#include <time.h>
#include <sstream>

using namespace std;
using namespace cgicc;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
ostream& DatabaseServer::status (ostream& os, cgicc::Cgicc const& cgi, bool failure) {
   printHeader(os);
   os << "<p>" << '\n';
   if (failure) {
      os << "The database could not be loaded." << '\n';
   } else {
      os << "The database loaded successfully." << '\n';
   }
   os << "</p>" << '\n';

   os << "<p>" << '\n';
   os << "The database pano directory is " << _db.panoramaDirectory() << ".<br/>\n";
   os << "The database contains " << _db.panoramas() << " photos.\n";
   os << "The database contains " << _db.features() << " features.\n";
   os << "</p>" << '\n';

   return os;
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::metadata (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   //printXMLHeader(os);
   
   // extract info
   PanoramaID panoramaID(cgi["panorama"]->getStrippedValue());

   // get response from the database
   mongo::BSONObj panorama = _db.findPanorama(panoramaID);
   return os << panorama.jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::panoramaNear (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   //printXMLHeader(os);
   
   // extract info
   double lat = cgi["lat"]->getDoubleValue();
   double lon = cgi["lon"]->getDoubleValue();

   // get response from the database
   mongo::BSONObj panorama = _db.findPanorama(Location(lon, lat));
   return os << panorama.jsonString();
}

//------------------------------------------------------------------------------
/*
ostream& DatabaseServer::panosInRange (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printXMLHeader(os);
   
   // extract arguments
   float lat1 = static_cast<float>(cgi["lat1"]->getDoubleValue());
   float lon1 = static_cast<float>(cgi["lon1"]->getDoubleValue());
   float lat2 = static_cast<float>(cgi["lat2"]->getDoubleValue());
   float lon2 = static_cast<float>(cgi["lon2"]->getDoubleValue());

   // get response from the database, format as xml
   pugi::xml_document doc;
   pugi::xml_node results = prepareDocument(doc);
   LinkedList<PhotoKey>* photos = _db.findPanos(lat1, lon1, lat2, lon2);
   _xml.photoMetadataXML(results, photos);
   doc.save(os);
   delete photos;
   
   return os;
}
*/

//------------------------------------------------------------------------------
ostream& DatabaseServer::panoramaByPanoid (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   //printXMLHeader(os);
   
   // extract info
   double lat = cgi["lat"]->getDoubleValue();
   double lon = cgi["lon"]->getDoubleValue();
   string panoid = cgi["pano_id"]->getValue();

   // get response from the database
   mongo::BSONObj panorama = _db.findPanorama(panoid);
   return os << panorama.jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::downloadPanorama (ostream& os, cgicc::Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   //printXMLHeader(os);
   
   // extract info
   string panoid = cgi["pano_id"]->getValue();

   // download the panorama
   ImageDownloader downer(&_db, 100000);
   try {
      mongo::BSONObj panorama = downer.savePano(panoid.c_str(), 3);
      return os << panorama.jsonString();
   }
   catch (DownloadError) {
      mongo::BSONObjBuilder result;
      result << "failure" << "true";
      return os << result.obj().jsonString();
   }
}

/*
//------------------------------------------------------------------------------
ostream& DatabaseServer::newTag (ostream& os, cgicc::Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printXMLHeader(os);
   
   // extract info
   unsigned id = getHexValue(cgi["id"]);
   double t1 = cgi["t1"]->getDoubleValue();
   double p1 = cgi["p1"]->getDoubleValue();
   double t2 = cgi["t2"]->getDoubleValue();
   double p2 = cgi["p2"]->getDoubleValue();
 
   // add to the database
   Tag* newtag = _db.addTag(id, Trash, t1, p1, t2, p2);
   pugi::xml_document doc;
   pugi::xml_node results = prepareDocument(doc);
   if (newtag) {
      addResultStatus(results, true);
      ostringstream tagstring;
      tagstring << newtag->tagID();
      pugi::xml_node tagid = results.append_child("TagID");
      tagid.text() = tagstring.str().c_str();
   } else {
      addResultStatus(results, false);
   }
   doc.save(os);

   return os;
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::removeTag (ostream& os, cgicc::Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printXMLHeader(os);
   
   // extract info
   unsigned tagid = getHexValue(cgi["tag_id"]);

   // remove specified tag from the database
   bool result = _db.removeTag(TagID(tagid));
   pugi::xml_document doc;
   pugi::xml_node results = prepareDocument(doc);
   if (result) {
      addResultStatus(results, true);
   } else {
      addResultStatus(results, false);
   }
   doc.save(os);

   return os;
}
*/

//------------------------------------------------------------------------------
pugi::xml_node DatabaseServer::prepareDocument (pugi::xml_document& doc) {
   pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
   decl.append_attribute("version") = "1.0";
   decl.append_attribute("encoding") = "UTF-8";
   return doc.append_child("Results");
}

/*
//------------------------------------------------------------------------------
void DatabaseServer::addResultStatus (pugi::xml_node& results, bool success) {
   pugi::xml_node status = results.append_child("Status");
   if (success) {
      status.text() = "success";
   } else {
      status.text() = "failure";
   }
}

//------------------------------------------------------------------------------
unsigned DatabaseServer::getHexValue (cgicc::const_form_iterator const& formentry) {
   stringstream hexstring(formentry->getStrippedValue());
   unsigned intval;
   hexstring >> hex >> intval;
   return intval;
}
*/

