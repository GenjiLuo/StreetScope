//==============================================================================
// GoogleMaps.cpp
// Created February 10 2012
//==============================================================================

#include "GoogleMaps.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <fstream>
#include <cmath>
#include "Polyline.h"
#include "Magick++.h"
#include "pugixml.hpp"
#include <iomanip>

using namespace std;


//==============================================================================
// Member Function Definitions for DownloadBuffer and LocationBuffer
//==============================================================================

//------------------------------------------------------------------------------
DownloadBuffer::DownloadBuffer (unsigned initialCapacity)
: _capacity(initialCapacity), _filled(0)
{
   _data = static_cast<char*> (malloc(_capacity));
}

//------------------------------------------------------------------------------
unsigned DownloadBuffer::append (char const* source, unsigned size) {
   // resize if we have to
   if (_filled + size + 1 > _capacity) {
      _capacity <<= 2;
      _data = static_cast<char*> (realloc(_data, _capacity));
      if (!_data)
         return 0;
   }
   
   // copy over the new data
   memcpy(&_data[_filled], source, size);
   _filled += size;
   return size;
}

//------------------------------------------------------------------------------
LocationBuffer::LocationBuffer (unsigned initialCapacity)
: _capacity(initialCapacity), _filled(0)
{
   _data = (Location*) malloc(sizeof(Location)*_capacity);
}

//------------------------------------------------------------------------------
Location* LocationBuffer::alloc (unsigned length) {
   Location* writeHere = _data + _filled;
   _filled += length;
   if (_filled > _capacity) {
      _capacity = _filled << 1;
      _data = (Location*) realloc(_data, sizeof(Location)*_capacity);
   }
   if (_data)
      return writeHere;
   return 0;
}


//==============================================================================
// Public Member Function Definitions for ImageDownloader
//==============================================================================

//------------------------------------------------------------------------------
bool ImageDownloader::getPanoNear (Location const& location, Panorama& panorama) {
   // empty strings buffer and clear edge list
   _strings.clear();
   panorama.edges.clear();

   // fetch xml data from google maps
   cout << "Finding pano near " << location.lat << ", " << location.lon << "...";
   ostringstream url;
   renderPanoramaURL(url, location);
   if (!fillBuffer(_textBuffer, url.str().c_str())) {
      cout << " download error.\n";
      return false;
   }
   _textBuffer.addTerminator();
   return read_sv_xml(panorama);
}

//------------------------------------------------------------------------------
bool ImageDownloader::getPano (char const* panoid, Panorama& panorama) {
   // empty strings buffer and clear edge list
   _strings.clear();
   panorama.edges.clear();

   // fetch xml data from google maps
   ostringstream url;
   renderPanoramaURL(url, panoid);
   if (!fillBuffer(_textBuffer, url.str().c_str())) {
      cout << " download error.\n";
      return false;
   }
   _textBuffer.addTerminator();
   return read_sv_xml(panorama);
}

//------------------------------------------------------------------------------
bool ImageDownloader::read_sv_xml (Panorama& panorama) {
   // Construct a DOM from _textBuffer
   pugi::xml_document doc;
   doc.load_buffer_inplace(_textBuffer.data(), _textBuffer.filled());
   pugi::xml_node pano_node = doc.child("panorama");
   pugi::xml_node data_node = pano_node.child("data_properties");
   pugi::xml_node proj_node = pano_node.child("projection_properties");
   pugi::xml_node anno_node = pano_node.child("annotation_properties");
   if (!data_node) {
      cout << " none found.\n";
      return false;
   }

   // extract the panoid
   CharPoolIndex pid = _strings.addString(data_node.attribute("pano_id").value());
   panorama.panoid = _strings.editString(pid);

   // extract location and original location
   panorama.location.lon = data_node.attribute("lng").as_double();
   panorama.location.lat = data_node.attribute("lat").as_double();
   panorama.originalLocation.lon = data_node.attribute("original_lng").as_double();
   panorama.originalLocation.lat = data_node.attribute("original_lat").as_double();

   // extract orientation
   panorama.yaw.setRad( convertYaw( proj_node.attribute("pano_yaw_deg").as_double() ) );
   panorama.tiltYaw.setRad( convertYaw( proj_node.attribute("tilt_yaw_deg").as_double() ) );
   panorama.tiltPitch.setRad( convertPitch( proj_node.attribute("tilt_pitch_deg").as_double() ) );

   // extract capture date
   CharPoolIndex did = _strings.addString(data_node.attribute("image_date").value());
   char* datestr = _strings.editString(did);
   unsigned length = 0;
   while (*(datestr + length) != '-') {
      ++length;
   }
   *(datestr + length) = ' ';
   istringstream iss(datestr);
   unsigned year, month;
   iss >> year >> month;
   struct tm time_components;
   time_components.tm_year  = year - 1900;
   time_components.tm_mon   = month-1;
   time_components.tm_mday  = 1;
   time_components.tm_hour  = 0;
   time_components.tm_min   = 0;
   time_components.tm_sec   = 0;
   time_components.tm_isdst = -1;
   panorama.captureDate = mktime(&time_components);

   // extract edges
   pugi::xml_node edge_node = anno_node.first_child();
   Edge edge;
   while (edge_node) {
      edge.angle.setRad( convertYaw( edge_node.attribute("yaw_deg").as_double() ) );
      pid = _strings.addString(edge_node.attribute("pano_id").value());
      edge.panoid = _strings.getString(pid);
      panorama.edges.push_back(edge);
      edge_node = edge_node.next_sibling();
   }

   cout << " found " << panorama.panoid << ".\n";
   return true;
}

//------------------------------------------------------------------------------
PanoramaID ImageDownloader::downloadPano (Panorama const& panorama, unsigned zoom) {
   cout << "Downloading " << panorama.panoid << ".\n";
   /*
   // This code was inspired by the information provided on Jamie Thompson's
   // guide to the Google Street View API. However his tile sizes were incorrect
   // when I tested them.
   unsigned xTilesArray[5] = { 2, 4, 6, 13, 26 };
   unsigned yTilesArray[5] = { 1, 2, 3,  7, 16 };
   unsigned xTiles = xTilesArray[zoom-1];
   unsigned yTiles = yTilesArray[zoom-1];
   */

   unsigned height = 208 * (1 << zoom);
   unsigned width  = height << 1;
   unsigned yTiles = (height + 511) / 512;
   unsigned xTiles = (width + 511) / 512;

   //Magick::Image panoImage( Magick::Geometry(512*xTiles, 512*yTiles), "white" );
   Magick::Image panoImage( Magick::Geometry(width, height), "white" );
   Magick::Image tile( Magick::Geometry(512, 512), "white" );
   Magick::Blob magickBlob;

   // download tiles and stitch them together
   for (unsigned x=0; x<xTiles; ++x) {
      for (unsigned y=0; y<yTiles; ++y) {
         ostringstream url;
         renderImageURL(url, panorama.panoid, x, y, zoom);
         //cout << "Pulling image from " << url.str() << '\n';
         if (!fillBuffer(_imageBuffer, url.str().c_str())) {
            cout << "Could not download first tile from panorama \"" << panorama.panoid << "\".\n";
            throw(DownloadError(panorama.panoid, "Could not download panorama."));
            //return 0;
         }

         // note: This copies the data into the blob. Useless, but I think it prevents data from
         // being copied in the next line. (We use the blob not for this reason, but so that
         // timeouts and such are detected by libcurl and not ImageMagick++.)
         magickBlob.update(_imageBuffer.data(), _imageBuffer.filled());

         tile.read(magickBlob, Magick::Geometry(512, 512), "jpg");
         //tile.write(tempfile.str().c_str());

         panoImage.composite(tile, 512*x, 512*y, Magick::OverCompositeOp);
      }
   }

   // add the panorama to the database
   PanoramaID panoramaID = _database->insertPanorama(panorama);

   // mirror and save the photo (its name is the id we just got from the database)
   panoImage.flop();
   panoImage.write(_database->panoramaPath(panoramaID));

   return panoramaID;
}

//------------------------------------------------------------------------------
PanoramaID ImageDownloader::savePano (char const* panoid, unsigned zoom) {
   Panorama pano;
   if (!getPano(panoid, pano))
      throw(DownloadError(panoid, "Could not find specified panorama."));
   return downloadPano(pano, zoom);
}

//------------------------------------------------------------------------------
bool ImageDownloader::findRoute (char* origin, char* destination) {
   // construct the url (replacing spaces with %20s)
   ostringstream url;
   renderDirectionsURL(url, origin, destination);
//   cout << "Getting directions from: " << url.str() << '\n';
   
   // fetch directions from google maps
   if (!fillBuffer(_textBuffer, url.str().c_str())) {
      return false;
   }
   _textBuffer.addTerminator();
   
   // extract and decode the polylines in the (first) route
   char* routeStart = strstr(_textBuffer.data(), "<route>");
   if (!routeStart) {
      return false;
   }
   char* routeEnd = strstr(_textBuffer.data(), "</route>");
   char* pointsStart = strstr(routeStart, "<polyline>");
   char* pointsEnd;
   // I'm not sure if google will ever give us two routes, but this ensures we
   // only take the first if they do.
   _locationBuffer.clear();
   Location* write;
   while (pointsStart and pointsStart < routeEnd) {
      pointsStart = strstr(pointsStart, "<points>");
      pointsStart += 8;
      pointsEnd   = strstr(pointsStart, "</points>");
      *pointsEnd  = 0;
      unsigned length = decode1(pointsStart);
      write = _locationBuffer.alloc(length);
      if (!write) {
         cout << "Could not allocate memory for route!\n";
         return false;
      }
      decode2(pointsStart, write, length);
      pointsStart = strstr(pointsEnd+1, "<polyline>");
   }
   
   return true;
}

//------------------------------------------------------------------------------
unsigned ImageDownloader::downloadRoute (float maxStep, bool metadataOnly) {
   unsigned panos = 0;
   unsigned tests = 0;
   Location const* locations = _locationBuffer.data();
   Location loc;
   Location step;
   unsigned steps;
   Panorama pano;
   // this should be plenty long enough to fit all panoids
   char* lastpano = new char[100];
   lastpano[0] = 0;
   
   // download the first image
   if (getPanoNear(locations[0], pano)) {
      if (!metadataOnly) {
         downloadPano(pano, 3);
      } else {
         _database->insertPanorama(pano);
      }
      strcpy(lastpano, pano.panoid);
   }
   
   // download the rest of the images
   for (unsigned i=1; i<_locationBuffer.filled(); ++i) {
      loc = locations[i-1];
      step = calculateStep(loc, locations[i], maxStep, steps);
      tests += steps;
      for (unsigned j=0; j<steps; ++j) {
         // make sure there's a panorama there to download,
         // and don't download duplicates
         if (getPanoNear(loc, pano) and strcmp(lastpano, pano.panoid) != 0) {
            if ( _database->findPanorama(pano.panoid).isEmpty() ) {
	            if (!metadataOnly) {
                  downloadPano(pano, 3);
               } else {
                  _database->insertPanorama(pano);
               }
               ++panos;
               strcpy(lastpano, pano.panoid);
            } else {
               cout << "Photo was already in database.\n";
            }
         }
         loc += step;
      }
   }
   cout << "Tested " << tests << " locations.\n";
   
   return panos;
}

//------------------------------------------------------------------------------
ostream& ImageDownloader::renderDirectionsURL (ostream& url, char* origin, char* destination) {
   // construct the url (replacing spaces with %20s)
   url << "https://maps.googleapis.com/maps/api/directions/xml";
   url << "?origin=";
   char* token = strtok(origin, " ");
   while (token) {
      url << token << "%20";
      token = strtok(0, " ");
   }
   url << "&destination=";
   token = strtok(destination, " ");
   while (token) {
      url << token << "%20";
      token = strtok(0, " ");
   }
   url << "&sensor=false";
   
   return url;
}

//------------------------------------------------------------------------------
std::ostream& ImageDownloader::renderPanoramaURL (std::ostream& url, char const* panoid) {
   url << "http://cbk0.google.com/cbk?output=xml&panoid=" << panoid;
   return url;
}

//------------------------------------------------------------------------------
std::ostream& ImageDownloader::renderPanoramaURL (std::ostream& url, Location const& location) {
   url << "http://cbk0.google.com/cbk?output=xml&ll=";
   url << location.lat << ',' << location.lon;
   return url;
}

//------------------------------------------------------------------------------
ostream& ImageDownloader::renderImageURL (ostream& url, char const* panoid, unsigned x, unsigned y, unsigned zoom) {
   url << "http://maps.google.com/cbk";
   url << "?output=tile";
   url << "&panoid=" << panoid;
   url << "&zoom=" << zoom;
   url << "&x=" << x;
   url << "&y=" << y;
   url << "&fover=2&onerr=3";       // a google maps mystery
   url << "&renderer=spherical";
   url << "&v=4";
   return url;
}



//==============================================================================
// Private Member Function Definitions for ImageDownloader
//==============================================================================

//------------------------------------------------------------------------------
bool ImageDownloader::fillBuffer (DownloadBuffer& buffer, char const* url) {
   buffer.clear();
   curl_easy_setopt(_curl, CURLOPT_URL, url);
   curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, DownloadBuffer::curlHandle);
   curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &buffer);
   _result = curl_easy_perform(_curl);
   if (_result != 0) {
      cout << "Something went wrong fetching data from:\n";
      cout << url << '\n';
      return false;
   }
   return true;
}

//------------------------------------------------------------------------------
Location ImageDownloader::calculateStep (Location const& loc1, Location const& loc2, float maxStep, unsigned& steps) {
   Location step = loc2 - loc1;
   steps = 0;
   double abslat = abs(step.lat);
   double abslon = abs(step.lon);
   if (abslat <= abslon) {
      steps = static_cast<unsigned>(abslon / maxStep);
   } else {
      steps = static_cast<unsigned>(abslat / maxStep);
   }
   ++steps;
   step.lat /= steps;
   step.lon /= steps;
   return step;
}

