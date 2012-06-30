//==============================================================================
// TagTypes.h
// Created 1/24/12.
//==============================================================================

#ifndef TAG_TYPES
#define TAG_TYPES

#include <time.h>
#include "HashFunctions.h"
#include "LinkedList.hpp"
#include "SimpleCharPool.h"


//==============================================================================
// Tag Types
//==============================================================================

//------------------------------------------------------------------------------
typedef float Angle;

//------------------------------------------------------------------------------
// A unique identifier that we assign to each tag.
class TagID {
private:
   unsigned _id;

public:
   TagID () {}
   TagID (unsigned id): _id(id) {}

   unsigned u32 () const { return _id; }
   inline unsigned hash () const { return hash1(_id); }

   bool operator== (TagID tagid) const { return _id == tagid._id; }
   bool operator!= (TagID tagid) const { return _id != tagid._id; }
};

inline std::ostream& operator<< (std::ostream& os, TagID tagid) {
   return os << tagid.u32();
}


//------------------------------------------------------------------------------
// A Target is an object that we want our taggers to tag.
enum Target {
   Trash,
   Pothole,
   VacantLot
};

//------------------------------------------------------------------------------
// A single tagged target.
class Tag {
private:
   // unique id
   TagID _id;

   // What target is tagged.
   Target _target;
   
   // The date/time the tag was added.
   time_t _timestamp;
   
   // These define the box containing the tagged target.
   // _theta1 and _theta2 are measured in degrees counterclockwise from east
   // in the ground plane. _phi1 and _phi2 are measured in degrees up
   // from the ground plane.
   Angle _theta1;
   Angle _phi1;
   Angle _theta2;
   Angle _phi2;

public:
   Tag () {}
   Tag (TagID tagid, Target target, Angle t1, Angle p1, Angle t2, Angle p2);
   void setArea (Angle theta1, Angle phi1, Angle theta2, Angle phi2);
   
   TagID id () const { return _id; }
   Target   target () const { return _target; }
   time_t const* timestamp () const { return &_timestamp; }
   Angle theta1   () const { return _theta1; }
   Angle phi1  () const { return _phi1; }
   Angle theta2    () const { return _theta2; }
   Angle phi2 () const { return _phi2; }
};

//------------------------------------------------------------------------------
// A collection of tags for a single photo. Each tagged photo has a single
// associated TagSet.
class TagSet : public LinkedList<Tag> {
public:
   TagSet () {}
   
   void add (MemoryPool& pool, TagID tagid, Target target, Angle t1, Angle p1, Angle t2, Angle p2) {
      Link* newlink = static_cast<Link*> (pool.alloc(sizeof(Link)));
      new(&newlink->_item) Tag(tagid, target, t1, p1, t2, p2);
      addLink(newlink);
   }
   
   void add (MemoryPool& pool, Tag const& tag) { LinkedList<Tag>::add(tag, pool); }

   // temp - used to get around inability of my MemoryPool to be free()
   void add (TagID tagid, Target target, Angle t1, Angle p1, Angle t2, Angle p2) {
      Link* newlink = new Link;
      new(&newlink->_item) Tag(tagid, target, t1, p1, t2, p2);
      addLink(newlink);
   }

   TagSet::Iterator getTag (TagID tagid);
   
   /*
   void leakAllTags ();
   void leakTag (unsigned tagIndex);
   */
   void removeTag (TagID tagid);
};


#endif // TAG_TYPES
