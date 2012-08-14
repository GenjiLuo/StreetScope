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
   os << "The database root directory is " << _db.rootDirectory() << ".<br/>\n";
   os << "The database pano directory is " << _db.panoDirectory() << ".<br/>\n";
   unsigned x, y;
   _db.getRandState(x, y);
   os << "The random number generator state is " << x << ", " << y << ".<br/>\n";
   os << "The database contains " << _db.size() << " photos and " << _db.tags() << " tags.\n";
   os << "</p>" << '\n';

   return os;
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::metadata (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printXMLHeader(os);
   
   // Print debugging info
//   renderDebuggingInfo(os, cgi);

   // extract info
   unsigned id = cgi["id"]->getIntegerValue();

   // get response from the database, format as xml
   pugi::xml_document doc;
   pugi::xml_node results = prepareDocument(doc);
   PhotoMetadata const& pmd = _db.getMetadata(id);
   _xml.photoMetadataXML(results, pmd);
   doc.save(os);
   
   return os;
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::panosNear (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printXMLHeader(os);
   
   // Print debugging info
//   renderDebuggingInfo(os, cgi);

   // extract info
   float lat = static_cast<float>(cgi["lat"]->getDoubleValue());
   float lon = static_cast<float>(cgi["lon"]->getDoubleValue());

   // get response from the database, format as xml
   pugi::xml_document doc;
   pugi::xml_node results = prepareDocument(doc);
   LinkedList<PhotoKey>* photos = _db.findPanosNear(Location(lat, lon));
   _xml.photoMetadataXML(results, photos);
   doc.save(os);
   delete photos;
   
   return os;
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
ostream& DatabaseServer::panoIdNear (ostream& os, Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printXMLHeader(os);
   
   // extract info
   float lat = static_cast<float>(cgi["lat"]->getDoubleValue());
   float lon = static_cast<float>(cgi["lon"]->getDoubleValue());
   string panoid = cgi["pano_id"]->getValue();

   // get response from the database, format as xml
   pugi::xml_document doc;
   pugi::xml_node results = prepareDocument(doc);
   PhotoKey key;
   if (_db.findByPanoId( Location(lat, lon), panoid.c_str(), key)) {
      PhotoMetadata const& pmd = _db.getMetadata(key);
      _xml.photoMetadataXML(results, pmd);
   }
   doc.save(os);
   
   return os;
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::downloadPano (ostream& os, cgicc::Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printXMLHeader(os);
   
   // extract info
   string panoid = cgi["pano_id"]->getValue();

   pugi::xml_document doc;
   pugi::xml_node results = prepareDocument(doc);
   ImageDownloader downer(&_db, 100000);
   PhotoMetadata* pmd = downer.savePano(panoid.c_str(), 3);
   if (pmd) {
      _xml.photoMetadataXML(results, *pmd);
   } else {
      results.text() = "failure";
   }
   doc.save(os);

   return os;
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::newTag (ostream& os, cgicc::Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printXMLHeader(os);
   
   // extract info
   unsigned id = static_cast<unsigned>(cgi["id"]->getIntegerValue());
   float t1 = static_cast<float>(cgi["t1"]->getDoubleValue());
   float p1 = static_cast<float>(cgi["p1"]->getDoubleValue());
   float t2 = static_cast<float>(cgi["t2"]->getDoubleValue());
   float p2 = static_cast<float>(cgi["p2"]->getDoubleValue());
 
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
   unsigned tagid = static_cast<unsigned>(cgi["tag_id"]->getIntegerValue());

   // remove specified tag from the database
   bool result = _db.removeTag(TagID(tagid));
   pugi::xml_document doc;
   pugi::xml_node results = prepareDocument(doc);
   if (result) {
      results.text() = "success";
   } else {
      results.text() = "failure";
   }
   doc.save(os);

   return os;
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::saveDatabase (ostream& os, cgicc::Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printXMLHeader(os);

   // Save the database.
   bool result = _db.saveDatabase();
   pugi::xml_document doc;
   pugi::xml_node results = prepareDocument(doc);
   if (result) {
      results.text() = "success";
   } else {
      results.text() = "failure";
   }
   doc.save(os);

   return os;
}

//------------------------------------------------------------------------------
ostream& DatabaseServer::savePlaintext (ostream& os, cgicc::Cgicc const& cgi) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   printXMLHeader(os);

   // Get timestamp and format for use as a filename
   time_t timestamp = time(0);
   char buffer[64];
   strcpy(buffer, ctime(&timestamp));
   unsigned i=0;
   while (buffer[i] != '\n') { // ctime adds "\n\0" at end
      if (buffer[i] == ' ')
         buffer[i] = '_';
      ++i;
   }
   buffer[i] = '\0'; // replace '\n' with '\0'

   // Write the plaintext data
   ostringstream filename;
   filename << buffer << ".txt";
   bool result = _db.savePlaintext(filename.str().c_str());
   pugi::xml_document doc;
   pugi::xml_node results = prepareDocument(doc);
   if (result) {
      results.text() = "success";
   } else {
      results.text() = "failure";
   }
   doc.save(os);

   return os;
}

//------------------------------------------------------------------------------
pugi::xml_node DatabaseServer::prepareDocument (pugi::xml_document& doc) {
   pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
   decl.append_attribute("version") = "1.0";
   decl.append_attribute("encoding") = "UTF-8";
   return doc.append_child("Results");
}

//------------------------------------------------------------------------------
void DatabaseServer::addResultStatus (pugi::xml_node& results, bool success) {
   pugi::xml_node status = results.append_child("Status");
   if (success) {
      status.text() = "success";
   } else {
      status.text() = "failure";
   }
}

