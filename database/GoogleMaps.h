//==============================================================================
// GoogleMaps.h
// Created 2/10/12.
//==============================================================================

#ifndef GOOGLE_MAPS
#define GOOGLE_MAPS

#include <cstdlib>
#include <curl/curl.h>
#include "Database.h"


//==============================================================================
// Documentation
//==============================================================================

/*
 * GoogleMaps.h defines a number of classes used to populate a Database with
 * images from Google Maps.
 */


//==============================================================================
// Classes
//==============================================================================

//------------------------------------------------------------------------------
struct RawEdge {
   float angle;
   char const* panoid;
};

//------------------------------------------------------------------------------
struct Panorama {
   char const* panoid;
   Location location;
   Location origLocation;
   time_t captureDate;
   float panoYaw;         // orientation of panorama
   float tiltYaw;         // orientation of (ground plane) gradient
   float tiltPitch;       // slope of gradient
   LinkedList<RawEdge> edges; // adjacent panoramas
};

//------------------------------------------------------------------------------
// A buffer used for downloading data stored as a sequence of characters.
// We use it for text files as well as jpg files.
class DownloadBuffer {
private:
   char* _data;
   unsigned _capacity;  // units of chars
   unsigned _filled;    // units of chars
   
public:
   DownloadBuffer (unsigned initialCapacity);
   ~DownloadBuffer () { free(_data); }
   
   unsigned capacity () const { return _capacity; }
   unsigned filled () const { return _filled; }
   char const* data () const { return _data; }
   char* data () { return _data; }
   
   // copies size bytes from source and adds it to _data
   // returns the number of bytes copied (ie size, unless there's an error)
   unsigned append (char const* source, unsigned size);
   unsigned addTerminator () { return append("\0", 1); }
   void clear () { _filled = 0; }
   
   inline static size_t curlHandle (void *buffer, size_t size, size_t nmemb, void *userp);
};

//------------------------------------------------------------------------------
// A buffer of Locations.
class LocationBuffer {
private:
   Location* _data;
   unsigned _capacity;  // units of Locations
   unsigned _filled;    // units of Locations
   
public:
   LocationBuffer (unsigned initialCapacity);

   unsigned capacity () const { return _capacity; }
   unsigned filled () const { return _filled; }
   Location const* data () const { return _data; }
   Location* data () { return _data; }
   
   void clear () { _filled = 0; }
   Location* alloc (unsigned length);
};

//------------------------------------------------------------------------------
class ImageDownloader {
public:
   PhotoDatabase* _database;
   DownloadBuffer _imageBuffer;
   DownloadBuffer _textBuffer;
   LocationBuffer _locationBuffer;
   SimpleCharPool _strings;
   CURL* _curl;
   CURLcode _result;
   
public:
   ImageDownloader (PhotoDatabase* database, unsigned bufferCapacity)
   : _database(database), _imageBuffer(bufferCapacity), _textBuffer(1024), _locationBuffer(64), _strings(128), _curl(curl_easy_init()) {}
   ~ImageDownloader () { curl_easy_cleanup(_curl); }
   
   // Finds a panorama near the specified location, if one exists.
   bool getPanoNear (Location const& location, Panorama& panorama);
   // Gets metadata from Google Maps.
   bool getPano (char const* panoid, Panorama& panorama);
   // Extracts panorama metadata from _textBuffer.
   bool read_sv_xml (Panorama& panorama);

   // Downloads a panorama and saves it to the database.
   PhotoMetadata* downloadPano (Panorama const& panorama, unsigned zoom);
   // Saves a panorama in the database.
   PhotoMetadata* savePano (Panorama const& panorama);
   // Saves a panorama in the database.
   PhotoMetadata* savePano (char const* panoid, unsigned zoom);

   // Tries to find a driving route between source and destination.
   // Returns true if it succeeds, false otherwise.
   // When it works _locationBuffer is filled with a sequence of lat / lon
   // pairs leading from source to destination.
   bool findRoute (char* origin, char* destination);
   // Downloads all images along the route stored in _locationBuffer.
   // Returns the number of images downloaded.
   unsigned downloadRoute (float maxStep = 0.0001, bool metadataOnly = false);
   
   // return the url used to get directions between the specified locations
   std::ostream& renderDirectionsURL (std::ostream& url, char* origin, char* destination);
   // returns the url used to download panorama metadata
   std::ostream& renderPanoramaURL (std::ostream& url, char const* panoid);
   // returns the url used to query if there is a panorama near the specified location
   std::ostream& renderPanoramaURL (std::ostream& url, Location const& location);
   // returns the url of the image tile with the given panoid
   // and x value (y is assumed to be 0, x in {0, 1})
   std::ostream& renderImageURL (std::ostream& url, char const* panoid, unsigned x, unsigned y, unsigned zoom);

   // Converts a yaw as stored by Google (degrees clockwise from north)
   // to our azimuthal angle convention (degrees counterclockwise from east).
   inline void convertYaw (float& yaw) const;
   
private:
   // downloads the contents of url to buffer
   // Warning: it doesn't know the difference between text and data, so make sure
   // you add the null terminator yourself when working with text!
   bool fillBuffer (DownloadBuffer& buffer, char const* url);
   
   // downloads the image from url and saves it as filename
//   bool downloadImage (char const* url, char const* filename);
   
   // Calculates the right increment given that you never want any single coordinate
   // to change more than maxStep in a single step.
   Location calculateStep (Location const& loc1, Location const& loc2, float maxStep, unsigned& steps);
};



//==============================================================================
// Inline Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
size_t DownloadBuffer::curlHandle (void *buffer, size_t size, size_t nmemb, void *userp) {
   DownloadBuffer* ibuffer = static_cast<DownloadBuffer*> (userp);
   return ibuffer->append(static_cast<char*> (buffer), size * nmemb);
}

//------------------------------------------------------------------------------
void ImageDownloader::convertYaw (float& yaw) const {
   if (yaw <= 90.0) 
      yaw = 90.0 - yaw;
   else
      yaw = 450.0 - yaw;
}


#endif // GOOGLE_MAPS
