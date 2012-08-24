//==============================================================================
// Types.h
// Created August 23, 2012
//==============================================================================

#ifndef DATABASE_TYPES
#define DATABASE_TYPES

#include <vector>
#include <ctime>


//------------------------------------------------------------------------------
struct Location {
   double lon;
   double lat;

   Location () {}
   Location (double lon, double lat): lon(lon), lat(lat) {}

   Location operator+ (Location const& r) const { return Location(lon + r.lon, lat + r.lat); }
   Location operator- (Location const& r) const { return Location(lon - r.lon, lat - r.lat); }
   Location& operator+= (Location const& r) { lon += r.lon; lat += r.lat; return *this; }
};

//------------------------------------------------------------------------------
struct Angle {
   double a;  // always in radians

   Angle () {};
   Angle (double d): a(d) {};
   double rad () const { return a; };
   double deg () const { return a * 57.2957795131; };
   Angle& setRad (double rad) { a = rad; return *this; }
   Angle& setDeg (double deg) { a = deg * 0.01745329251; return *this; }
};

//------------------------------------------------------------------------------
struct AngleBox {
   Angle theta1;
   Angle phi1;
   Angle theta2;
   Angle phi2;
};

//------------------------------------------------------------------------------
struct Edge {
   Angle angle;
   char const* panoid;
};

//------------------------------------------------------------------------------
struct Panorama {
   Location location;
   Location originalLocation;
   char const* panoid;
   time_t captureDate;
   Angle yaw;
   Angle tiltYaw;
   Angle tiltPitch;
   std::vector<Edge> edges;
};

//------------------------------------------------------------------------------
struct Feature {
   char const* name;
};


#endif // DATABASE_TYPES
