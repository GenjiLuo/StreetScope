//==============================================================================
// TagTypes.cpp
// Created 1/24/12.
//==============================================================================

#include "TagTypes.h"


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
Tag::Tag (TagID tagid, Target target, Angle t1, Angle p1, Angle t2, Angle p2)
: _id(tagid), _target(target), _theta1(t1), _phi1(p1), _theta2(t2), _phi2(p2)
{
   _timestamp = time(0);
}

//------------------------------------------------------------------------------
void Tag::setArea (Angle theta1, Angle phi1, Angle theta2, Angle phi2) {
   _theta1 = theta1;
   _phi1   = phi1;
   _theta2 = theta2;
   _phi2   = phi2;
}

//------------------------------------------------------------------------------
TagSet::Iterator TagSet::getTag (TagID tagid) {
   Iterator itr = iterator();
   while (itr.valid() and itr.cref().id() != tagid) {
      ++itr;
   }
   // Iterator may be valid or not
   return itr;
}

//------------------------------------------------------------------------------
/*
void TagSet::leakTag (unsigned tagIndex) {
   if (tagIndex == 0) {
      leakFirst();
   } else {
      Iterator itr = iterator();
      for (unsigned i=1; i<tagIndex; ++i) {
         ++itr;
      }
      leakNext(itr);
   }
}
*/

//------------------------------------------------------------------------------
void TagSet::removeTag (TagID tagid) {
   Iterator itr = iterator();
   if (itr.cref().id() == tagid) {
      removeFirst();
      return;
   }
   Iterator next = itr;
   ++next;
   while (next.valid() and next.cref().id() != tagid) {
      ++itr;
      ++next;
   }
   if (next.valid())
      removeNext(itr);
}

//------------------------------------------------------------------------------
/*
void TagSet::leakAllTags () {
   this->leakAll();
}
*/

