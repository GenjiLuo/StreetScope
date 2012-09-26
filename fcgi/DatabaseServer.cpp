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
ostream& DatabaseServer::panorama (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract info
   PanoramaID panoramaID(cgi["id"]->getStrippedValue());

   // get response from the database
   mongo::BSONObj panorama = _db.findPanorama(panoramaID);
   return os << _json.panorama(panorama).jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::feature (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract info
   FeatureID featureID(cgi["id"]->getStrippedValue());

   // get response from the database
   mongo::BSONObj feature = _db.findFeature(featureID);
   return os << _json.feature(feature).jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::tagset (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract arguments
   PanoramaID panoramaID(cgi["panorama"]->getStrippedValue());
   PanoramaID featureID(cgi["feature"]->getStrippedValue());

   // get data from the database
   mongo::BSONObj tagset = _db.findTagSet(panoramaID, featureID);

   return os << _json.tagset(tagset).jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::panoramaTagsets (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract arguments
   PanoramaID panoramaID(cgi["panorama"]->getStrippedValue());

   // get data from the database
   std::auto_ptr<mongo::DBClientCursor> tagsets = _db.findPanoramaTagSets(panoramaID);

   return os << _json.tagsets(tagsets).jsonString();
}


//------------------------------------------------------------------------------
ostream& DatabaseServer::panoramaNear (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract info
   double lat = cgi["lat"]->getDoubleValue();
   double lon = cgi["lon"]->getDoubleValue();

   // get response from the database
   mongo::BSONObj panorama = _db.findPanorama(Location(lon, lat));
   return os << _json.panorama(panorama).jsonString();
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
   printJSONHeader(os);
   
   // extract info
   //double lat = cgi["lat"]->getDoubleValue();
   //double lon = cgi["lon"]->getDoubleValue();
   string panoid = cgi["pano_id"]->getValue();

   // get response from the database
   mongo::BSONObj panorama = _db.findPanorama(panoid.c_str());
   return os << _json.panorama(panorama).jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::downloadPanorama (ostream& os, cgicc::Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract info
   string panoid = cgi["pano_id"]->getValue();

   // download the panorama
   ImageDownloader downer(&_db, 100000);
   try {
      PanoramaID panorama = downer.savePano(panoid.c_str(), 3);
      mongo::BSONObj pObj = _db.findPanorama(panorama);
      return os << _json.panorama(pObj).jsonString();
   }
   catch (DownloadError error) {
      mongo::BSONObjBuilder result;
      result << "failure" << "true";
      return os << result.obj().jsonString();
   }
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::insertTag (ostream& os, cgicc::Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract info
   PanoramaID panorama = PanoramaID(cgi["panorama"]->getStrippedValue());
   FeatureID feature = FeatureID(cgi["feature"]->getStrippedValue());
   double t1 = cgi["t1"]->getDoubleValue();
   double p1 = cgi["p1"]->getDoubleValue();
   double t2 = cgi["t2"]->getDoubleValue();
   double p2 = cgi["p2"]->getDoubleValue();
   AngleBox box;
   box.theta1 = t1;
   box.phi1 = p1;
   box.theta2 = t2;
   box.phi2 = p2;
 
   // add to the database
   TagSetID newtag = _db.insertTag(panorama, feature, box);

   // see if we actually added anything
   mongo::BSONObj tObj = _db.findTagSetWithTag(newtag);
   if (!tObj.isEmpty()) {
      return os << _json.tagset(tObj).jsonString();
   } else {
      mongo::BSONObjBuilder result;
      result << "failure" << "true";
      return os << result.obj().jsonString();
   }
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::removeTag (ostream& os, cgicc::Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract info
   TagID tagid = TagID(cgi["tag"]->getStrippedValue());

   // check that the tag exists
   mongo::BSONObj tagset1 = _db.findTagSetWithTag(tagid);
   if (!tagset1.isEmpty()) {
      _db.removeTag(tagid);
      mongo::BSONObj tagset2 = _db.findTagSet(tagset1["_id"].OID());
      return os << _json.tagset(tagset2).jsonString();
   } else {
      mongo::BSONObjBuilder result;
      result << "failure" << "true";
      return os << result.obj().jsonString();
   }
}

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

