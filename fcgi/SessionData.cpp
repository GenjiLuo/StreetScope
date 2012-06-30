//==============================================================================
// SessionData.cpp
// Created 3/13/12.
//==============================================================================

#include "SessionData.h"



//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
void SessionData::setTagRange (float latlow, float lonlow, float lathigh, float lonhigh) {
   _latlow  = latlow;
   _lonlow  = lonlow;
   _lathigh = lathigh;
   _lonhigh = lonhigh;
   
   delete _photoList;
   _photoList = _database.findPanos(_latlow, _lonlow, _lathigh, _lonhigh);
   _currentPhoto = _photoList->constIterator();
}
