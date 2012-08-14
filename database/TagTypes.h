//==============================================================================
// TagTypes.h
// Created 1/24/12.
//==============================================================================

#ifndef TAG_TYPES
#define TAG_TYPES

#include <time.h>
#include "MemoryPoolF.h"
#include "LinkedList.hpp"
#include "SimpleCharPool.h"


//==============================================================================
// Tag Types
//==============================================================================

//------------------------------------------------------------------------------
struct Angle {
   mutable float _a; // mutable so we can +/- 360 as needed

   Angle () {}
   Angle (float a): _a(a) {}

   operator float () const { return _a; }
   // normalizes an azimuthal angle (ie a "theta")
   void normalize1 () const { while (_a < 0) _a += 360.0; while (_a >= 360.0) _a -= 360.0; }
   // normalizes a polar angle (ie a "phi")
   void normalize2 () const { if (_a < -180.0) _a = -180.0; else if (_a > 90.0) _a = 90.0; }

   Angle operator+= (Angle a) { return _a += a._a; }
};

//------------------------------------------------------------------------------
// A unique identifier that we assign to each tag.
class TagID {
private:
   unsigned _id;

public:
   TagID () {}
   TagID (unsigned id): _id(id) {}

   unsigned u32 () const { return _id; }
   unsigned hash () const { return _id; }

   bool operator== (TagID tagid) const { return _id == tagid._id; }
   bool operator!= (TagID tagid) const { return _id != tagid._id; }
};

inline std::ostream& operator<< (std::ostream& os, TagID tagid) {
   return os << std::hex << tagid.u32() << std::dec;
}

//------------------------------------------------------------------------------
// A Target is an object that we want our taggers to tag.
enum Target {
   Trash,
   Pothole,
   VacantLot
};

//------------------------------------------------------------------------------
struct AngleRectangle {
   Angle theta1;
   Angle phi1;
   Angle theta2;
   Angle phi2;

   AngleRectangle () {};
   AngleRectangle (Angle theta1, Angle phi1, Angle theta2, Angle phi2)
   : theta1(theta1), phi1(phi1), theta2(theta2), phi2(phi2) {}
   void normalize () const { theta1.normalize1(); theta2.normalize1(); phi1.normalize2(); phi2.normalize2(); }

   bool containsTheta (AngleRectangle const& ar) const;
   void print () const;
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
   AngleRectangle _bounds;

public:
   Tag () {}
   Tag (TagID tagid, Target target, Angle t1, Angle p1, Angle t2, Angle p2);
   void setArea (Angle theta1, Angle phi1, Angle theta2, Angle phi2);
   
   TagID tagID () const { return _id; }
   Target   target () const { return _target; }
   time_t const* timestamp () const { return &_timestamp; }
   AngleRectangle const& bounds () const { return _bounds; }
   AngleRectangle      & bounds ()       { return _bounds; }
   Angle theta1 () const { return _bounds.theta1; }
   Angle phi1   () const { return _bounds.phi1;   }
   Angle theta2 () const { return _bounds.theta2; }
   Angle phi2   () const { return _bounds.phi2;   }
};

//------------------------------------------------------------------------------
// A collection of tags for a single photo. Each tagged photo has a single
// associated TagSet.
class TagSet : public LinkedList<Tag> {
public:
   TagSet () {}
   
   Tag& add (MemoryPoolF& pool, TagID tagid, Target target, Angle t1, Angle p1, Angle t2, Angle p2) {
      Link* newlink = static_cast<Link*> (pool.alloc());
      new(&newlink->_item) Tag(tagid, target, t1, p1, t2, p2);
      addLink(newlink);
      return newlink->_item.ref();
   }
   
   // called by PhotoMetadata::loadTags
   void add (MemoryPoolF& pool, Tag const& tag) { LinkedList<Tag>::add(tag, pool); }

   // temp - used to get around inability of my MemoryPool to be free()
   /*
   void add (TagID tagid, Target target, Angle t1, Angle p1, Angle t2, Angle p2) {
      Link* newlink = new Link;
      new(&newlink->_item) Tag(tagid, target, t1, p1, t2, p2);
      addLink(newlink);
   }
   */

   TagSet::Iterator getTag (TagID tagid);
   
   /*
   void leakAllTags ();
   void leakTag (unsigned tagIndex);
   */
   void leakTag (TagID tagid);
};


#endif // TAG_TYPES
