//==============================================================================
// PhotoTypes.h
// Created 1/24/12.
//==============================================================================

#ifndef PHOTO_TYPES
#define PHOTO_TYPES

#include <time.h>
#include <iostream>
#include "HashFunctions.h"
#include "Location.h"
#include "Hypergrid.hpp"
#include "TagTypes.h"
#include "SimpleCharPool.h"
#include "MemoryPoolF.h"


//==============================================================================
// Photo Types
//==============================================================================

//------------------------------------------------------------------------------
// A unique identifier that we assign for each photo.
class PhotoID {
private:
   unsigned _id;

public:
   PhotoID () {}
   PhotoID (unsigned id): _id(id) {}

   unsigned u32 () const { return _id; }
   inline unsigned hash () const { return hash1(_id); }

   bool operator== (PhotoID photoid) const { return _id == photoid._id; }
};

inline std::ostream& operator<< (std::ostream& os, PhotoID photoid) {
   return os << photoid.u32();
}


//------------------------------------------------------------------------------
// Not unique, but allows for quick lookup of a photo by location.
// Right now this is 32 bits, we might want to make it 64 if we want better
// spatial resolution.
typedef HyperIndex<2> PhotoIndex;

//------------------------------------------------------------------------------
// Allows one to both look up a photo by location and uniquely identify it.
// A PhotoDatabase has a QTree full of these.
class PhotoKey {
private:
   PhotoID _id;         // unique global identifier
   PhotoIndex _index;   // encodes its location
   
public:
   PhotoKey () {}
   PhotoKey (PhotoID id, PhotoIndex const& index): _id(id), _index(index) {}

   PhotoID id () const { return _id; }
   PhotoIndex index () const { return _index; }
   unsigned hash () const { return _id.hash(); }

   bool operator== (PhotoKey const& key) const { return id() == key.id(); }
};

//------------------------------------------------------------------------------
class TagKey {
private:
   TagID _tagid;
   PhotoID _photoid;

public:
   TagKey (TagID tagid, PhotoID photoid): _tagid(tagid), _photoid(photoid) {};
   TagID tagID () const { return _tagid; }
   PhotoID photoID () const { return _photoid; }
   unsigned hash () const { return _tagid.hash(); }
};
inline bool operator== (TagKey const& tagkey, TagID tagid) { return tagkey.tagID() == tagid; }
inline bool operator== (TagID tagid, TagKey const& tagkey) { return tagkey.tagID() == tagid; }

//------------------------------------------------------------------------------
// Note: For now we are only using Google's panoids. We should be smarter and
// replace them with our own ids once the referenced images are in our database.
struct Edge {
   Angle _angle;
   CharPoolIndex _id;

   Edge () {};
   Edge (Angle angle, CharPoolIndex id): _angle(angle), _id(id) {}
   Angle angle () const { return _angle; }
   CharPoolIndex panoid () const { return _id; }
};

//------------------------------------------------------------------------------
class EdgeSet : public LinkedList<Edge> {
public:
   void add (MemoryPoolF& pool, Angle angle, CharPoolIndex id) {
      Link* newlink = static_cast<Link*> (pool.alloc());
      new(&newlink->_item) Edge(angle, id);
      addLink(newlink);
   }
   // called by PhotoMetadata::loadEdges
   void add (MemoryPoolF& pool, Edge const& edge) { LinkedList<Edge>::add(edge, pool); }
};

//------------------------------------------------------------------------------
// All the data associated with a photo.
// A PhotoDatabase has a HashSet full of these.
class PhotoMetadata {
private:
   PhotoID _id;            // global identifier (same as in PhotoKey)
   time_t _timestamp;      // when photo was added
   Location _location;     // lat / lon
   CharPoolIndex _panoid;  // google maps panoid
   Location _origLocation; // "original location" from google maps
   time_t _captureDate;    // "capture date" from google maps
   Angle _panoYaw;         // The next 3 fields denote the orientation of the
   Angle _tiltYaw;         // panorama relative to the cardinal axes and are
   Angle _tiltPitch;       // taken directly from Google Maps.
   // Should include pano links!
   TagSet _tags;           // all of this photo's tags
   EdgeSet _edges;         // links to adjacent panoramas

public:
   // Constructors
   PhotoMetadata () {}
   //PhotoMetadata (PhotoID id, Location const& l, CharPoolIndex panoid);
   PhotoMetadata (PhotoID id,
                  Location const& loc,
                  CharPoolIndex panoid,
                  Location const& origloc,
                  time_t capdate,
                  Angle panoYaw,
                  Angle tiltYaw,
                  Angle tiltPitch);

   // Setters
   void setOrigLocation (Location const& loc) { _origLocation = loc; }
   void setCaptureDate (time_t time) { _captureDate = time; }
   void setPanoYaw (Angle angle) { _panoYaw = angle; }
   void setTiltYaw (Angle angle) { _tiltYaw = angle; }
   void setTiltPitch (Angle angle) { _tiltPitch = angle; }

   // Const Accessors
   PhotoID id () const { return _id; }
   time_t const* timestamp () const { return &_timestamp; }
   Location const& location () const { return _location; }
   CharPoolIndex const& panoid () const { return _panoid; }
   Location const& originalLocation () const { return _origLocation; }
   time_t const* captureDate () const { return &_captureDate; }
   Angle const& panoYaw   () const { return _panoYaw; }
   Angle const& tiltYaw   () const { return _tiltYaw; }
   Angle const& tiltPitch () const { return _tiltPitch; }

   // General Accessors
   TagSet&       tags ()       { return _tags; }
   TagSet const& tags () const { return _tags; }
   EdgeSet&       edges ()       { return _edges; }
   EdgeSet const& edges () const { return _edges; }

   // returns the Photo's hash (the hash is a function of its id, not the actual image)
   inline unsigned hash () const { return _id.hash(); }
   PhotoKey photoKey () const { return PhotoKey(_id, _location.index()); }

   // equality test
   bool operator== (PhotoMetadata const& metadata) const { return id() == metadata.id(); }
   
   void print () const {
      std::cout << id();
//      std::cout << "Id:       " << id() << '\n';
//      std::cout << "Added:    " << ctime(timestamp());
//      std::cout << "Location: " << location() << '\n';
//      std::cout << "Orig Loc: " << originalLocation() << '\n';
//      std::cout << "Cap Date: " << ctime(captureDate());
//      std::cout << "Tags:     " << tags().items() << '\n';
//      std::cout << '\n';
   }
   
   // writes the PhotoMetadata to the ostream in binary format.
   std::ostream& save (std::ostream& os) const;

   // loads the PhotoMetadata (minus tags) from the ostream.
   std::istream& loadData  (std::istream& is);
   // Returns the number of tags loaded
   unsigned loadTags  (std::istream& is, MemoryPoolF& pool);
   std::istream& loadEdges (std::istream& is, MemoryPoolF& pool);

private:
   // returns the size of the plain old data portion of a PhotoMetadata object
   inline unsigned dataSize () const;
};


//==============================================================================
// Inline Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
unsigned PhotoMetadata::dataSize () const {
   return sizeof(PhotoMetadata) - sizeof(TagSet) - sizeof(EdgeSet);
}


//==============================================================================
// Inline Global Functions
//==============================================================================

//------------------------------------------------------------------------------
inline bool operator== (PhotoMetadata const& metadata, PhotoID photoid) {
   return metadata.id() == photoid;
}

inline bool operator== (PhotoID photoid, PhotoMetadata const& metadata) {
   return metadata == photoid;
}

//------------------------------------------------------------------------------
inline bool operator== (PhotoMetadata const& metadata, PhotoKey const& key) {
   return metadata.id() == key.id();
}

inline bool operator== (PhotoKey const& key, PhotoMetadata const& metadata) {
   return metadata == key;
}


#endif // PHOTO_TYPES
