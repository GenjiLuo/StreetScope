//==============================================================================
// Database.h
// Created January 6, 2012.
//==============================================================================

#ifndef DATABASE
#define DATABASE

#include <string>
#include "PhotoTypes.h"
#include "TagTypes.h" // already included by PhotoTypes.h
#include "Location.h"
#include "QTree.hpp"
#include "Random.h"
//#include "MemoryPool.h"
#include "SimpleCharPool.h"
#include "LinkedList.hpp"

// estdlib
#include "HashSet.hpp"
#include "MemoryPoolF.h"

// debug
#include <iostream>


//==============================================================================
// Data Management Classes
//==============================================================================

class PhotoDatabase {
//private:
public:
   QTree<PhotoKey, 2> _photokeys;    // allows quick lookup of photos by Location
   HashSet<PhotoMetadata, MemoryPoolF> _metadata; // stores all the metadata
   HashSet<TagKey, MemoryPoolF> _tagkeys;         // allows lookup of tag regardless of photo
   MemoryPoolF _tagpool;             // shared by all TagSets
   MemoryPoolF _edgepool;            // shared by all EdgeSets
   XorShift32 _rand;                 // generates ids
   unsigned _tags;                   // total number of tags
   SimpleCharPool _panoids;          // panoids of images from Google Street View
   SimpleCharPool _epanoids;         // panoids of edges
//   SimpleCharPool _tagTargets;       // stores the targets of the tags

   std::string _rootDir;      // all files are written/read from _rootDir (and subdirs)
   std::string _panoDir;      // where the panoramas are stored
   CharPoolIndex _lastPanoid; // index of most recently added panoid

public:
   //----------------------------------------------------------------------------
   // Basics
   
   // initialCapacity is a rough estimate of how many photos will be in the database
   PhotoDatabase (unsigned initialCapacity);
   // Include the trailing '/' character!
   void setRootDir (std::string rootDir);
   void setPanoDir (std::string panoDir);
   void setRandState (unsigned x, unsigned y) { _rand.setState(x, y); }

   // Accessor Methods
   unsigned size () const { return _photokeys.size(); }
   unsigned tags () const { return _tags; }
   char const* rootDirectory () const { return _rootDir.c_str(); }
   char const* panoDirectory () const { return _panoDir.c_str(); }
   QTree<PhotoKey, 2> const& keys () const { return _photokeys; }
   
   //----------------------------------------------------------------------------
   // Database Operations
   
   // Adds metadata (without any photo) to the database. Useful for testing.
   //inline bool addMetadata (char const* panoid, Location const& l, Location const& ol, time_t cd);
   
   // Used to add a new photo. Right now I'm assuming all photos are coming from Google Street View.
   // Note: the returned pointer is only good for immediate use (after the hashset resizes it'll be junk)
   //PhotoMetadata* addNewPhoto (char const* panoid, Location const& l, Location const& ol, time_t cd, char const* data, unsigned size);

   // Adds a new meta data entry to the database.
   PhotoMetadata* addNewMetaData (Location const& l,
                                  char const* panoid,
                                  Location const& origLoc,
                                  time_t capDate,
                                  Angle panoYaw,
                                  Angle tiltYaw,
                                  Angle tiltPitch);

   // Adds an edge to the specified PhotoMetadata object.
   void addEdge (PhotoMetadata* md, Angle angle, char const* panoid);
   
   // Writes _metadata to disk. Returns true if successful, false otherwise.
   bool saveDatabase () const;
   
   // Reads the database from disk
   bool loadDatabase ();
   
   // Writes out all the information to a plaintext file. Useful for debugging.
   bool savePlaintext (char const* fname) const;
   void printMetadata () const { _metadata.print(); }

   //----------------------------------------------------------------------------
   // Photo Operations

   // Returns a linked list of all photos within the specified lat/lon box
   inline LinkedList<PhotoKey>* findPanos (float latlow, float lonlow, float lathigh, float lonhigh) const;
   // Returns a linked list of photos nearby the specified location
   inline LinkedList<PhotoKey>* findPanosNear (Location const& l, float tolerance = 0.0003) const;
   // Attempts to find a panorama with a specific panoid near Location
   bool findByPanoId (Location const& l, char const* panoid, PhotoKey& key) const;
   bool findByPanoId (Location const& l, char const* panoid) const;

   // Looks up the metadata associated with a PhotoKey
   inline PhotoMetadata const& getMetadata (PhotoID photoid) const;
   inline PhotoMetadata&       getMetadata (PhotoID photoid);
   inline PhotoMetadata const& getMetadata (PhotoKey const& key) const;
   
   // Finds a specific tag on a specific photo
   inline Tag* getTag (PhotoID photoid, TagID tagid);
   
   // generates a new (as of yet unused tag id)
   TagID newTagID ();
   // Adds a new tag to a photo
   bool addTag (PhotoID photoid, Target target, Angle theta1, Angle phi1, Angle theta2, Angle phi2);
   
   // Leaks all the tags belonging to a photo
   //inline void leakTags (PhotoID photoid);
   
   // Leaks a specific tag of a specific photo
   //inline void leakTag (PhotoID photoid, unsigned tagIndex);

   // Removes a tag - should be updated to use MemoryPool!
   bool removeTag (PhotoID photoid, TagID tagid);
};


//==============================================================================
// Inline Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
//void PhotoDatabase::reserveNewPhotoID ();

//------------------------------------------------------------------------------
/*
bool PhotoDatabase::addMetadata (char const* panoid, Location const& l, Location const& ol, time_t cd) {
   // create a metadata listing with the next available id and given metadata
   unsigned newid = this->size();
   CharPoolIndex pid = _panoids.addString(panoid);
   PhotoMetadata metadata(newid, l, pid, ol, cd);
   
   // add metadata
   _metadata.add(metadata);
   _photokeys.add(metadata.photoKey());
   return true;
}
*/

//------------------------------------------------------------------------------
LinkedList<PhotoKey>* PhotoDatabase::findPanos (float latlow, float lonlow, float lathigh, float lonhigh) const {
   LinkedList<PhotoKey>* result;
   result = _photokeys.find( HyperBox<2>( Location(latlow, lonlow).gridCoords(),
                                          Location(lathigh, lonhigh).gridCoords() ) );
   return result;
}

//------------------------------------------------------------------------------
LinkedList<PhotoKey>* PhotoDatabase::findPanosNear (Location const& l, float tolerance) const {
   return findPanos(l.lat()-tolerance,
                    l.lon()-tolerance,
                    l.lat()+tolerance,
                    l.lon()+tolerance);
}

//------------------------------------------------------------------------------
PhotoMetadata const& PhotoDatabase::getMetadata (PhotoID photoid) const {
   return *(_metadata.find(photoid));
}

//------------------------------------------------------------------------------
PhotoMetadata& PhotoDatabase::getMetadata (PhotoID photoid) {
   return *(_metadata.find(photoid));
}

//------------------------------------------------------------------------------
PhotoMetadata const& PhotoDatabase::getMetadata (PhotoKey const& key) const {
   return *(_metadata.find(key.id()));
}

//------------------------------------------------------------------------------
Tag* PhotoDatabase::getTag (PhotoID photoid, TagID tagid) {
   TagSet::Iterator itr = getMetadata(photoid).tags().getTag(tagid);
   if (itr.valid())
      return itr.ptr();
   else
      return nullptr;
}

//------------------------------------------------------------------------------
/*
void PhotoDatabase::leakTags (PhotoID photoid) {
   _metadata.find(photoid)->tags().leakAllTags();
}
*/

//------------------------------------------------------------------------------
/*
void PhotoDatabase::leakTag (PhotoID photoid, unsigned tagIndex) {
   getMetadata(photoid).tags().leakTag(tagIndex);
}
*/

#endif // DATABASE
