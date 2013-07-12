//==============================================================================
// TagTypes.cpp
// Created 1/24/12.
//==============================================================================

#include "TagTypes.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
string TagID::hex () const {
   ostringstream hexstring;
   hexstring << std::hex << _id;
   return hexstring.str();
}

//------------------------------------------------------------------------------
bool AngleRectangle::containsTheta (AngleRectangle const& ar) const {
   // normal intervals
   if (theta1 <= theta2) {
      if (ar.theta1 < theta1 or ar.theta2 > theta2)
         return false;
   // intervals that cross the seam
   } else if ((ar.theta1 < theta1 and ar.theta1 > theta2) or (ar.theta2 < theta1 and ar.theta2 > theta1)) {
      return false;
   }
   return true;
}

//------------------------------------------------------------------------------
void AngleRectangle::print () const {
   cout << "theta1 " << theta1 << '\n';
   cout << "phi1   " << phi1 << '\n';
   cout << "theta2 " << theta2 << '\n';
   cout << "phi2   " << phi2 << '\n';
   cout << '\n';
}

//------------------------------------------------------------------------------
Tag::Tag (TagID tagid, Target target, Angle t1, Angle p1, Angle t2, Angle p2)
: _id(tagid), _target(target), _bounds(t1, p1, t2, p2)
{
   _timestamp = time(0);
}

//------------------------------------------------------------------------------
void Tag::setArea (Angle theta1, Angle phi1, Angle theta2, Angle phi2) {
   _bounds.theta1 = theta1;
   _bounds.phi1   = phi1;
   _bounds.theta2 = theta2;
   _bounds.phi2   = phi2;
}

//------------------------------------------------------------------------------
TagSet::Iterator TagSet::getTag (TagID tagid) {
   Iterator itr = iterator();
   while (itr.valid() and itr.cref().tagID() != tagid) {
      ++itr;
   }
   // Iterator may be valid or not
   return itr;
}

//------------------------------------------------------------------------------
void TagSet::leakTag (TagID tagid) {
   Iterator itr = iterator();
   if (itr.cref().tagID() == tagid) {
      leakFirst();
      return;
   }
   Iterator next = itr;
   ++next;
   while (next.valid() and next.cref().tagID() != tagid) {
      ++itr;
      ++next;
   }
   if (next.valid())
      leakNext(itr);
}

//------------------------------------------------------------------------------
/*
void TagSet::leakAllTags () {
   this->leakAll();
}
*/

