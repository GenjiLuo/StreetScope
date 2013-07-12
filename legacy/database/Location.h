//==============================================================================
// Location.h
// Created 1/24/12.
//==============================================================================

#ifndef LOCATION_H
#define LOCATION_H

#include <iostream>
#include "Hypergrid.hpp"


//==============================================================================
// Location
//==============================================================================

//------------------------------------------------------------------------------
/*
 * Two different systems are used to keep track of Location: latitude/longitude,
 * and our own 1D indexing system. Our indeces are used as hash codes, and are
 * roughly those you get by drawing a Hilbert Curve that covers the Earth.
 *
 * More exactly, imagine drawing a grid on the earth whose lines follow lines
 * of equal latitude and longitude. Draw it so that there are 2^32 subdivisions
 * in total. If you were to take this grid off the earth and stretch it appropriately we
 * would have 2^32 evenly sized squares, so for this reason we will sometimes
 * call the bins of this grid squares.
 * Now we assign 32 bit indeces to these squares as follows:
 * The two least significant bits of the index specify the quadrant of the grid
 * where we find a given point. The lower left quadrant is 0, upper left is 1,
 * upper right 2, and lower right 3. The next two bits specify which quarter of
 * this quadrant you are in, using the same numbering scheme. By recursively
 * specifying quarters 16 times, we obtain a unique 32 bit index for each square
 * on our grid.
 */
class Location {
private:
   static constexpr float EC = 40075040; // circumference of the earth in meters
   // side length (at equator) of one bin in our grid = EC / 2^HilbertRes
//   static float BinSize;

   // Do we want doubles? They may be unneccessarily large.
   float _lat;
   float _lon;

public:
   Location () {};
   Location (float latitude, float longitude): _lat(latitude), _lon(longitude) {}
   
   float  lat () const { return _lat; }
   float  lon () const { return _lon; }
   float& lat ()       { return _lat; }
   float& lon ()       { return _lon; }

   HyperCoords<2> gridCoords () const; // returns its coordinates in our index grid
   inline HyperIndex<2> index () const;          // returns the index of the Location
   
   inline Location& operator=  (Location const& loc);
   inline Location  operator+  (Location const& loc) const;
   inline Location  operator-  (Location const& loc) const;
   Location& operator+= (Location const& loc) { *this = *this + loc; return *this; }
   Location& operator-= (Location const& loc) { *this = *this - loc; return *this; }
};

inline std::ostream& operator<< (std::ostream& os, Location const& loc);

//==============================================================================
// Inline Methods and Functions
//==============================================================================

//------------------------------------------------------------------------------
HyperIndex<2> Location::index () const {
   return getIndex(gridCoords());
}

//------------------------------------------------------------------------------
Location& Location::operator= (Location const& loc) {
   _lat = loc._lat;
   _lon = loc._lon;
   return *this;
}

//------------------------------------------------------------------------------
Location Location::operator+ (Location const& loc) const {
   return Location(lat() + loc.lat(), lon() + loc.lon());
}

//------------------------------------------------------------------------------
Location Location::operator- (Location const& loc) const {
   return Location(lat() - loc.lat(), lon() - loc.lon());
}

//------------------------------------------------------------------------------
std::ostream& operator<< (std::ostream& os, Location const& loc) {
   os << loc.lat() << ", " << loc.lon();
   return os;
}

#endif // LOCATION_H
