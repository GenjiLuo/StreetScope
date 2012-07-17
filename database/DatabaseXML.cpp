//==============================================================================
// DatabaseXML.cpp
// Created May 31 2012
//==============================================================================

#include "DatabaseXML.h"
#include <sstream>
#include <iomanip>

using namespace std;
using namespace pugi;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
xml_node& DatabaseXML::timestampXML (xml_node& node, time_t const* time, char const* name) {
   xml_node n = node.append_child(name);
   char buffer[128];
   strcpy(buffer, ctime(time));
   buffer[strlen(buffer)-1] = '\0';
   n.text().set(buffer);
   return node;
}

//------------------------------------------------------------------------------
xml_node& DatabaseXML::locationXML (xml_node& node, Location const& l, char const* name) {
   xml_node n = node.append_child(name);
   ostringstream lat, lon;
   lat << fixed << setprecision(6) << l.lat();
   lon << fixed << setprecision(6) << l.lon();
   n.append_attribute("lat") = lat.str().c_str();
   n.append_attribute("lon") = lon.str().c_str();
   return node;
}

//------------------------------------------------------------------------------
xml_node& DatabaseXML::tagXML (xml_node& node, Tag const& tag) {
   xml_node tagnode = node.append_child("Tag");
   tagnode.append_attribute("id") = tag.tagID().u32();
   tagnode.append_attribute("target") = tag.target();
   tagnode.append_attribute("theta1") = tag.theta1();
   tagnode.append_attribute("phi1")   = tag.phi1();
   tagnode.append_attribute("theta2") = tag.theta2();
   tagnode.append_attribute("phi2")   = tag.phi2();
   return node;
}

//------------------------------------------------------------------------------
xml_node& DatabaseXML::edgeXML (xml_node& node, Edge const& edge) {
   xml_node edgenode = node.append_child("Edge");
   edgenode.append_attribute("angle") = edge.angle();
   edgenode.append_attribute("pano_id") = _db._epanoids.getString(edge.panoid());
   return node;
}

//------------------------------------------------------------------------------
xml_node& DatabaseXML::photoMetadataXML (xml_node& node, PhotoMetadata const& pmd) {
   xml_node n = node.append_child("PhotoMetadata");
   n.append_attribute("id") = pmd.id().u32();
   n.append_attribute("pano_id") = _db._panoids.getString(pmd.panoid());

   timestampXML(n, pmd.timestamp(), "Timestamp");
   locationXML(n, pmd.location(), "Location");
   locationXML(n, pmd.originalLocation(), "OriginalLocation");
   timestampXML(n, pmd.captureDate(), "CaptureDate");

   xml_node orientation = n.append_child("Orientation");
   orientation.append_attribute("panoYaw") = pmd.panoYaw();
   orientation.append_attribute("tiltYaw") = pmd.tiltYaw();
   orientation.append_attribute("tiltPitch") = pmd.tiltPitch();

   xml_node edges = n.append_child("Edges");
   LinkedList<Edge>::ConstIterator edgeitr;
   for (edgeitr = pmd.edges().constIterator(); edgeitr.valid(); ++edgeitr) {
      edgeXML(edges, edgeitr.cref());
   }

   xml_node tags = n.append_child("Tags");
   LinkedList<Tag>::ConstIterator tagitr;
   for (tagitr = pmd.tags().constIterator(); tagitr.valid(); ++tagitr) {
      tagXML(tags, tagitr.cref());
   }

   return node;
}

//------------------------------------------------------------------------------
xml_node& DatabaseXML::photoMetadataXML (xml_node& node, LinkedList<PhotoKey> const* keys) {
   LinkedList<PhotoKey>::ConstIterator itr;
   for (itr = keys->constIterator(); itr.valid(); ++itr) {
      photoMetadataXML(node, _db.getMetadata(itr.cref()));
   }
   return node;
}

