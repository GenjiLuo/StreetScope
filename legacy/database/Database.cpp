//==============================================================================
// Database.cpp
// Created 1/23/12.
//==============================================================================

#include <iostream>
#include <sstream>
#include <fstream>
#include "Database.h"

using namespace std;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
PhotoDatabase::PhotoDatabase (unsigned initialCapacity)
: _photokeys(initialCapacity),          // exactly one PhotoKey per photo
  _metadata(initialCapacity),           // exactly one PhotoMetadata per photo
  _tagkeys(initialCapacity),            // approx 1 tag per photo (is this right?)
  _panoids(initialCapacity<<4),         // roughly 16 characters per id (per photo)
  _epanoids(initialCapacity<<5),        // roughly 2 edges per photo
//  _tagTargets(256)                      // completely arbitrary:
  _lastPanoid(0)
{
   _tagpool.setItemSize(sizeof(LinkedList<Tag>::Link));
   _tagpool.setMinFree(1);
   _tagpool.setNextBlockSize(initialCapacity);
   _tagpool.setMinDonationSize(64);
   _edgepool.setItemSize(sizeof(LinkedList<Edge>::Link));
   _edgepool.setMinFree(1);
   _edgepool.setNextBlockSize(initialCapacity);
   _edgepool.setMinDonationSize(64);
}

//------------------------------------------------------------------------------
void PhotoDatabase::setRootDir (std::string rootDir) {
   _rootDir = rootDir;
}

//------------------------------------------------------------------------------
void PhotoDatabase::setPanoDir (std::string panoDir) {
   _panoDir = panoDir;
}

//------------------------------------------------------------------------------
/* Note that this method should not call addMetadata, because it is useful to only
 * add the PhotoMetadata object to the database once the file has been successfully saved.
 */
/*
PhotoMetadata* PhotoDatabase::addNewPhoto (char const* panoid,
                                           Location const& l,
                                           Location const& ol,
                                           time_t cd,
                                           char const* data,
                                           unsigned dsize) {
   // create a metadata listing with the next available id and given metadata
   unsigned id = this->size();
   CharPoolIndex pid;
   if (strcmp(panoid, _panoids.getString(_lastPanoid)) == 0) {
      pid = _lastPanoid;
   } else {
      pid = _panoids.addString(panoid);
      _lastPanoid = pid;
   }
   PhotoMetadata metadata(id, l, pid, ol, cd);
   PhotoMetadata* result;
   
   // create the right file
   ostringstream fullname;
   fullname << _rootDir;
   fullname << "photos/";
   fullname << id;
   fullname << ".jpg";
   ofstream file(fullname.str().c_str());

   // check for errors
   if (!file.good()) {
      cout << "Couldn't open the file \"" << fullname.str() << "\".\n;";
      file.close();
      return 0;
   }
   file.write(data, dsize);
   file.close();
   
   // add metadata
   result = &(_metadata.add(metadata));
   _photokeys.add(metadata.photoKey());
   return result;
}
*/

//------------------------------------------------------------------------------
PhotoMetadata* PhotoDatabase::addNewMetaData (Location const& l,
                                              char const* panoid,
                                              Location const& origLoc,
                                              time_t capDate,
                                              Angle panoYaw,
                                              Angle tiltYaw,
                                              Angle tiltPitch)
{
   // generate new id
   PhotoID id = newPhotoID();

   // should check for duplicates
   CharPoolIndex pid = _panoids.addString(panoid);

   // add to the database
   PhotoMetadata metadata(id, l, pid, origLoc, capDate, panoYaw, tiltYaw, tiltPitch);
   PhotoMetadata* result = &(_metadata.add(metadata));
   _photokeys.add(metadata.photoKey());

   return result;
}

//------------------------------------------------------------------------------
void PhotoDatabase::addEdge (PhotoMetadata* md, Angle angle, char const* panoid) {
   CharPoolIndex pid = _epanoids.addString(panoid);
   md->edges().add(_edgepool, angle, pid);
}

//------------------------------------------------------------------------------
bool PhotoDatabase::findByPanoId (Location const& l, char const* panoid, PhotoKey& key) const {
   LinkedList<PhotoKey>* results = findPanosNear(l);
   LinkedList<PhotoKey>::Iterator itr = results->iterator();
   for ( ; itr.valid(); ++itr) {
      if ( strcmp(_panoids.getString( getMetadata(itr.cref()).panoid() ), panoid) == 0 ) {
         key = itr.cref();
         return true;
      }
   }
   return false;
}

//------------------------------------------------------------------------------
bool PhotoDatabase::findByPanoId (Location const& l, char const* panoid) const {
   PhotoKey key;
   return findByPanoId(l, panoid, key);
}

//------------------------------------------------------------------------------
std::string PhotoDatabase::panoPath (PhotoID photoid) const {
   ostringstream path;
   path << _panoDir << photoid << ".jpg";
   return path.str();
}

//------------------------------------------------------------------------------
bool PhotoDatabase::saveDatabase () const {
   // open file
   string fmetadata = _rootDir + "metadata/metadata.bin";
   cout << "Saving metadata to " << fmetadata << '.' << '\n';
   ofstream file(fmetadata.c_str(), ios_base::binary | ios_base::out | ios_base::trunc);
   if (!file.good())
      return false;

   // save _rand state
   unsigned x, y;
   _rand.state(x, y);
   file.write(reinterpret_cast<char*>(&x), sizeof(unsigned));
   file.write(reinterpret_cast<char*>(&y), sizeof(unsigned));

   // save metadata
   HashSet<PhotoMetadata, MemoryPoolF>::ConstIterator itr = _metadata.constIterator();
   for ( ; itr.valid(); ++itr) {
      itr.cref().save(file);
   }
   file.close();

   // Save Panoids
   string fpanoids = _rootDir + "metadata/panoids.bin";
   cout << "Saving panoids to " << fpanoids << ".\n";
   if (!_panoids.save(fpanoids.c_str())) {
      cout << "Couldn't save panoids.\n";
      return false;
   }

   // Save Edge Panoids
   string fepanoids = _rootDir + "metadata/epanoids.bin";
   cout << "Saving edge panoids to " << fepanoids << ".\n";
   if (!_epanoids.save(fepanoids.c_str())) {
      cout << "Couldn't save edge panoids.\n";
      return false;
   }

   return true;
}

//------------------------------------------------------------------------------
bool PhotoDatabase::loadDatabase () {
   // open file
   string fmetadata = _rootDir + "metadata/metadata.bin";
   cout << "Loading metadata from " << fmetadata << '.' << '\n';
   ifstream file(fmetadata.c_str(), ios_base::binary | ios_base::in);
   if (!file.good()) {
      cout << "Couldn't load metadata!\n";
      return false;
   }

   // load _rand state
   unsigned x, y;
   file.read(reinterpret_cast<char*>(&x), sizeof(unsigned));
   file.read(reinterpret_cast<char*>(&y), sizeof(unsigned));
   setRandState(x, y);

   // load metdata
   PhotoMetadata metadata;
   metadata.loadData(file);
   while (file.good()) {
      _photokeys.add(metadata.photoKey());
      PhotoMetadata& newMetadata = _metadata.add(metadata);
      newMetadata.loadTags(file, _tagpool);
      for (auto itr = newMetadata.tags().constIterator(); itr.valid(); ++itr) {
         _tagkeys.add( TagKey( itr.cref().tagID(), newMetadata.id() ) );
      }
      newMetadata.loadEdges(file, _edgepool);
      metadata.loadData(file);
   }
   file.close();
   
   // Load Panoids
   string fpanoids = _rootDir + "metadata/panoids.bin";
   cout << "Loading panoids from " << fpanoids << ".\n";
   if (!_panoids.load(fpanoids.c_str())) {
      cout << "Couldn't load panoids!\n";
      return false;
   }
           
   // Load Edge Panoids
   string fepanoids = _rootDir + "metadata/epanoids.bin";
   cout << "Loading edge panoids from " << fepanoids << ".\n";
   if (!_epanoids.load(fepanoids.c_str())) {
      cout << "Couldn't load edge panoids!\n";
      return false;
   }
           
   return true;
}

//------------------------------------------------------------------------------
bool PhotoDatabase::savePlaintext (char const* fname) const {
   // Open File
   string filename = _rootDir + "plaintext/" + fname;
   cout << "Saving plaintext database to " << filename << '.' << '\n';
   ofstream file(filename.c_str(), ios_base::binary | ios_base::out | ios_base::trunc);
   if (!file.good())
      return false;

   // save _rand state
   unsigned x, y;
   _rand.state(x, y);
   file << "Random number generator state: " << x << ", " << y << '\n';
   file << "Total Panoramas: " << _metadata.size() << '\n';
   file << "Total Tags: " << _tagkeys.size() << '\n';
   file << '\n';
   
   // Save Metadata
   HashSet<PhotoMetadata, MemoryPoolF>::ConstIterator itr = _metadata.constIterator();
   LinkedList<Tag>::ConstIterator tagitr;
   LinkedList<Edge>::ConstIterator eitr;
   unsigned totalPhotos = 0;
   unsigned totalTags = 0;
   for ( ; itr.valid(); ++itr) {
      file << "Id:         " << hex << itr.cref().id() << dec           << '\n';
      file << "Added:      " << ctime(itr.cref().timestamp());
      file << "Location:   " << itr.cref().location()                   << '\n';
      file << "Panoid:     " << _panoids.getString(itr.cref().panoid()) << '\n';
      file << "Orig Loc:   " << itr.cref().originalLocation()           << '\n';
      file << "Cap Date:   " << ctime(itr.cref().captureDate());
      file << "Pano Yaw:   " << itr.cref().panoYaw()                    << '\n';
      file << "Tilt Yaw:   " << itr.cref().tiltYaw()                    << '\n';
      file << "Tilt Pitch: " << itr.cref().tiltPitch()                  << '\n';
      file << "Tags:       " << itr.cref().tags().items()               << '\n';
      if (itr.cref().tags().items() > 0) {
         unsigned i = 0;
         for (tagitr = itr.cref().tags().constIterator(); tagitr.valid(); ++tagitr) {
            Tag const& tag = tagitr.cref();
            file << "Tag " << i++ << ":\n";
            file << "   Id:         " << hex << tag.tagID() << dec      << '\n';
            file << "   Target:     " << tag.target()                   << '\n';
            file << "   Timestamp:  " << ctime(tag.timestamp());
            file << "   Theta Low:  " << tag.theta1()                   << '\n';
            file << "   Phi Low:    " << tag.phi1()                     << '\n';
            file << "   Theta High: " << tag.theta2()                   << '\n';
            file << "   Phi High:   " << tag.phi2()                     << '\n';
            ++totalTags;
         }
      }
      file << "Edges:      " << itr.cref().edges().items()              << '\n';
      if (itr.cref().edges().items() > 0) {
         unsigned i = 0;
         for (eitr = itr.cref().edges().constIterator(); eitr.valid(); ++eitr) {
            Edge const& edge = eitr.cref();
            file << "Edge " << i++ << ":\n";
            file << "   Angle:   " << edge.angle()                   << '\n';
            file << "   PanoId:  " << _epanoids.getString(edge.panoid()) << '\n';
         }
      }
      file << '\n';
      ++totalPhotos;
   }

   file << "Total Photos: " << totalPhotos << '\n';
   file << "Total Tags: " << totalTags << '\n';
   file << '\n';
   
   file.close();

   return true;
}

//------------------------------------------------------------------------------
PhotoID PhotoDatabase::newPhotoID () {
   unsigned newid;
   do {
      newid = _rand.u32();
   } while (_metadata.find(PhotoID(newid)));
   return PhotoID(newid);
}

//------------------------------------------------------------------------------
TagID PhotoDatabase::newTagID () {
   unsigned newid;
   do {
      newid = _rand.u32();
   } while (_tagkeys.find(TagID(newid)));
   return TagID(newid);
}

//------------------------------------------------------------------------------
Tag* PhotoDatabase::addTag (PhotoID photoid, Target target, Angle theta1, Angle phi1, Angle theta2, Angle phi2) {
   PhotoMetadata* photo = _metadata.find(photoid);
   if (!photo)
      return nullptr;
   TagID tagid = newTagID();
   Tag& newtag = photo->tags().add(_tagpool, tagid, target, theta1, phi1, theta2, phi2);
   _tagkeys.add(TagKey(tagid, photoid));
   return &newtag;
}

//------------------------------------------------------------------------------
bool PhotoDatabase::removeTag (TagID tagid) {
   // ensure said tag exists and remove from _tagkeys
   TagKey* tagkey = _tagkeys.find(tagid);
   if (!tagkey)
      return false;
   _tagkeys.remove(tagid);

   // remove from metadata listing
   TagSet& tagset = _metadata.find(tagkey->photoID())->tags();
   TagSet::Iterator tagitr = tagset.getTag(tagid);
   tagset.leakTag(tagid);
   _tagpool.free(tagitr.ptr());
   return true;
}

