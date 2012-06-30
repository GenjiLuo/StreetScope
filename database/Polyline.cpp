//==============================================================================
// Polyline.cpp
// Created 2/13/12.
//==============================================================================

#include "Polyline.h"
#include <cstdlib>

//==============================================================================
// Definitions
//==============================================================================

//------------------------------------------------------------------------------
// encodes the length long array of locations in a polyline
char* encode (Location* locations, unsigned length) {
   // each float takes up to 6 chars to represent
   // need space for null terminator
   char* result = (char*) malloc(length * 6 * 2 + 1);
   unsigned filled = 0;
   
   // encode the first lat / lon pair
   filled += encodeFloat(lat_e5(locations[0].lat()), result + filled);
   filled += encodeFloat(lon_e5(locations[0].lon()), result + filled);

   // encode differences from here on out (so we have less digits)
   Location temp;
   for (unsigned i=1; i<length; ++i) {
      temp = locations[i] - locations[i-1];
      filled += encodeFloat(lat_e5(temp.lat()), result + filled);
      filled += encodeFloat(lon_e5(temp.lon()), result + filled);
   }
   
   // add null terminator, return
   result[filled] = 0;
   return result;
}
