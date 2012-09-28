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
   const_form_iterator id = cgi["id"];
   if (id == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   PanoramaID panoramaID(id->getStrippedValue());

   // get response from the database
   mongo::BSONObj panorama = _db.findPanorama(panoramaID);
   return os << _json.panorama(panorama).jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::panoramaNear (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract info
   const_form_iterator latform = cgi["lat"];
   if (latform == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   double lat = latform->getDoubleValue();

   const_form_iterator lonform = cgi["lon"];
   if (lonform == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   double lon = lonform->getDoubleValue();

   // get response from the database
   mongo::BSONObj panorama = _db.findPanorama(Location(lon, lat));
   return os << _json.panorama(panorama).jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::panoramaByPanoid (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract info
   const_form_iterator panoform = cgi["panoid"];
   if (panoform == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   string panoid = panoform->getStrippedValue();

   // get response from the database
   mongo::BSONObj panorama = _db.findPanorama(panoid.c_str());
   return os << _json.panorama(panorama).jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::feature (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract info
   const_form_iterator id = cgi["id"];
   if (id == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   FeatureID featureID(id->getStrippedValue());

   // get response from the database
   mongo::BSONObj feature = _db.findFeature(featureID);
   return os << _json.feature(feature).jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::features (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // get response from the database
   std::auto_ptr<mongo::DBClientCursor> features = _db.findFeatures();

   return os << _json.features(features).jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::tags (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract arguments
   const_form_iterator panorama = cgi["panorama"];
   if (panorama == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   PanoramaID panoramaID(panorama->getStrippedValue());

   const_form_iterator feature = cgi["feature"];
   if (feature == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   FeatureID featureID(feature->getStrippedValue());

   // get data from the database
   std::auto_ptr<mongo::DBClientCursor> tags = _db.findTags(panoramaID, featureID);

   return os << _json.tags(tags).jsonString();
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::downloadPanorama (ostream& os, cgicc::Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printJSONHeader(os);
   
   // extract info
   const_form_iterator panoform = cgi["panoid"];
   if (panoform == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   string panoid = panoform->getStrippedValue();

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
   const_form_iterator panorama = cgi["panorama"];
   if (panorama == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   PanoramaID panoramaID(panorama->getStrippedValue());

   const_form_iterator feature = cgi["feature"];
   if (feature == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   FeatureID featureID(feature->getStrippedValue());

   const_form_iterator t1form = cgi["t1"];
   if (t1form == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   double t1 = t1form->getDoubleValue();

   const_form_iterator p1form = cgi["p1"];
   if (p1form == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   double p1 = p1form->getDoubleValue();

   const_form_iterator t2form = cgi["t2"];
   if (t2form == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   double t2 = t2form->getDoubleValue();

   const_form_iterator p2form = cgi["p2"];
   if (p2form == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   double p2 = p2form->getDoubleValue();

   AngleBox box;
   box.theta1 = t1;
   box.phi1 = p1;
   box.theta2 = t2;
   box.phi2 = p2;
 
   // add to the database
   TagSetID newtag = _db.insertTag(panoramaID, featureID, box);

   // see if we actually added anything
   mongo::BSONObj tObj = _db.findTag(newtag);
   if (!tObj.isEmpty()) {
      return os << _json.tag(tObj).jsonString();
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
   const_form_iterator id = cgi["id"];
   if (id == cgi.getElements().end()) {
      return os << mongo::BSONObj().jsonString();
   }
   TagID tagid(id->getStrippedValue());

   // check that the tag exists
   mongo::BSONObj tag = _db.findTag(tagid);
   if (!tag.isEmpty()) {
      _db.removeTag(tagid);
      mongo::BSONObjBuilder result;
      result << "success" << "true";
      return os << result.obj().jsonString();
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

