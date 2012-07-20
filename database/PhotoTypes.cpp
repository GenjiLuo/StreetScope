//==============================================================================
// PhotoTypes.cpp
// Created 1/24/12.
//==============================================================================

#include "PhotoTypes.h"

#include <iostream>

using namespace std;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
/*
PhotoMetadata::PhotoMetadata (PhotoID id, Location const& loc, CharPoolIndex panoid)
: _id(id),
  _location(loc),
  _panoid(panoid),
{
   _timestamp = time(0);
   
}
*/

//------------------------------------------------------------------------------
PhotoMetadata::PhotoMetadata (PhotoID id,
                              Location const& loc,
                              CharPoolIndex panoid,
                              Location const& origloc,
                              time_t capdate,
                              Angle panoYaw,
                              Angle tiltYaw,
                              Angle tiltPitch)
: _id(id),
  _location(loc),
  _panoid(panoid),
  _origLocation(origloc),
  _captureDate(capdate),
  _panoYaw(panoYaw),
  _tiltYaw(tiltYaw),
  _tiltPitch(tiltPitch)
{
   _timestamp = time(0);
   
}

//------------------------------------------------------------------------------
void PhotoMetadata::print () const {
   std::cout << "Id:       " << hex << id() << dec << '\n';
   std::cout << "Added:    " << ctime(timestamp());
   std::cout << "Location: " << location() << '\n';
   std::cout << "Orig Loc: " << originalLocation() << '\n';
   std::cout << "Cap Date: " << ctime(captureDate());
   std::cout << "Tags:     " << tags().items() << '\n';
   std::cout << "Edges:    " << edges().items() << '\n';
   std::cout << '\n';
}

//------------------------------------------------------------------------------
std::ostream& PhotoMetadata::save (std::ostream& os) const {
   // Write plain old data
   os.write(reinterpret_cast<char const*>(this), dataSize());
   
   // Write tags
   unsigned items = _tags.items();
   os.write(reinterpret_cast<char const*>(&items), sizeof(unsigned));
   for (LinkedList<Tag>::ConstIterator itr = _tags.constIterator(); itr.valid(); ++itr) {
      os.write(reinterpret_cast<char const*>(itr.cptr()), sizeof(Tag));
   }

   // Write edges
   unsigned edges = _edges.items();
   os.write(reinterpret_cast<char const*>(&edges), sizeof(unsigned));
   for (LinkedList<Edge>::ConstIterator itr = _edges.constIterator(); itr.valid(); ++itr) {
      os.write(reinterpret_cast<char const*>(itr.cptr()), sizeof(Edge));
   }

   return os;
}

//------------------------------------------------------------------------------
std::istream& PhotoMetadata::loadData (std::istream& is) {
   // Read plain old data
   is.read(reinterpret_cast<char*>(this), dataSize());
   return is;
}

//------------------------------------------------------------------------------
unsigned PhotoMetadata::loadTags (std::istream& is, MemoryPoolF& pool) {
   // Read tags
   unsigned items;
   Tag tag;
   is.read(reinterpret_cast<char*>(&items), sizeof(unsigned));
   for (unsigned i=0; i<items; ++i) {
      is.read(reinterpret_cast<char*>(&tag), sizeof(Tag));
      _tags.add(pool, tag);
   }
   return items;
}

//------------------------------------------------------------------------------
std::istream& PhotoMetadata::loadEdges (std::istream& is, MemoryPoolF& pool) {
   // Read tags
   unsigned edges;
   Edge edge;
   is.read(reinterpret_cast<char*>(&edges), sizeof(unsigned));
   for (unsigned i=0; i<edges; ++i) {
      is.read(reinterpret_cast<char*>(&edge), sizeof(Edge));
      _edges.add(pool, edge);
   }
   return is;
}

