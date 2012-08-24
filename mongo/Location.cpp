//==============================================================================
// Location.cpp
// Created 1/24/12.
//==============================================================================

#include "Location.h"


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
HyperCoords<2> Location::gridCoords () const {
   HyperCoords<2> coords;
   coords.zero();
   coords.setCoord(0, unsigned( (_lon + 180.) * float(One << HyperIndex<2>::Cuts) / 360. ) );
   coords.setCoord(1, unsigned( (_lat + 90.)  * float(One << HyperIndex<2>::Cuts) / 180.) );
   return coords;
}

// set static members of Location
//float Location::BinSize = Location::EC / (float)(1<<HyperIndex<2>::Cuts);
