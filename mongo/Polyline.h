//==============================================================================
// Polyline.h
// Created 2/13/12.
//==============================================================================

#ifndef POLYLINE
#define POLYLINE

#include "Types.h"


//==============================================================================
// Declarations
//==============================================================================

// Encodes an array of Locations as a polyline. User must free returned string.
char* encode (Location* locations, unsigned length);
// Encodes a single float. Returns the number of characters it wrote to s.
inline unsigned encodeFloat (float val, char* s);

// First pass for decoding a polyline. Returns number of Locations in the polyline.
inline unsigned decode1 (char* s);
// Finishes decoding a polyline. Writes locations Locations to output.
inline void decode2 (char* s, Location* output, unsigned locations);
// Decodes a single float. It will advance s to point to the next float in the polyline.
inline float decodeFloat (char*& s);

// It appears that google rounds latitudes, and truncates longitudes...
// Why they would do this is beyond me.
inline float lat_e5 (float lat) { return lat*100000.0f + 0.5; }
inline float lon_e5 (float lon) { return lon*100000.0f; }


//==============================================================================
// Inline Definitions
//==============================================================================

//------------------------------------------------------------------------------
unsigned encodeFloat (float val_e5, char* s) {
   // initial bit twiddles
   int x = static_cast<int> (val_e5);
   if (x < 0) {
      x <<= 1;
      x = ~x;
   } else {
      x <<= 1;
   }
   
   // break 5 bit chunks into characters
   unsigned const mask1 = (1<<5)-1; // first 5 bits set
   char     const mask2 = (1<<5);   // just 6th bit set
   unsigned lastNonzero = 0;
   unsigned offset = 0;
   for (unsigned i=0; i<6; ++i) {
      s[i] = (x & (mask1 << offset)) >> offset;
      if (s[i] != 0) {
         lastNonzero = i;
      }
      offset += 5;
   }
   for (unsigned i=0; i<lastNonzero; ++i) {
      s[i] |= mask2;
      s[i] += 63;
   }
   s[lastNonzero] += 63;
   return lastNonzero+1;
}

//------------------------------------------------------------------------------
unsigned decode1 (char* s) {
   // subtract 63 from all characters, figure out how many Locations there are
   unsigned floats = 0;
   while (*s) {
      *s -= 63;
      if (!(*s & (1<<5)))
         ++floats;
      ++s;
   }
   return floats >> 1;
}

//------------------------------------------------------------------------------
/* We need to know how many locations are expected, because the strings passed
 * to decode2 may have zeros in them besides the null terminator.
 */
void decode2 (char* s, Location* output, unsigned locations) {
   // unpack the starting lat / lon
   output[0].lon = decodeFloat(s);
   output[0].lat = decodeFloat(s);
   
   // the rest of the lat / lon pairs encode differences
   unsigned i = 1;
   while (i < locations) {
      output[i].lon = output[i-1].lon + decodeFloat(s);
      output[i].lat = output[i-1].lat + decodeFloat(s);
      ++i;
   }
}

//------------------------------------------------------------------------------
float decodeFloat (char*& s) {
   int x = 0;
   char const mask1 = (1<<5);
   char const mask2 = (1<<5)-1;
   unsigned offset = 0;
   while (true) {
      x |= (*s & mask2) << offset;
      if (!(*s & mask1)) {
         ++s;
         break;
      }
      ++s;
      offset += 5;
   }
   
   if (x & 1) {
      x = ~x;
   }
   x >>= 1;
   
   return (float)x / 100000.0f;
}


#endif // POLYLINE
