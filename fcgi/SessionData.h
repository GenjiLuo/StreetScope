//==============================================================================
// SessionData.h
// Created 3/13/12.
//==============================================================================

#ifndef SESSION_DATA
#define SESSION_DATA

#include "Database.h"


//==============================================================================
// Class SessionData
//==============================================================================

// This object contains the information needed to maintain the state of the server
// as seen by one user.
class SessionData {
private:
   PhotoDatabase& _database;
   float _latlow;
   float _lonlow;
   float _lathigh;
   float _lonhigh;
   LinkedList<PhotoKey>* _photoList;
   LinkedList<PhotoKey>::ConstIterator _currentPhoto;

public:
   inline SessionData (PhotoDatabase& database);
   
   void setTagRange (float latlow, float lonlow, float lathigh, float lonhigh);
   
   float lowlat  () const { return _latlow; }
   float highlat () const { return _lathigh; }
   float lowlon  () const { return _lonlow; }
   float highlon () const { return _lonhigh; }
   
   void nextPhoto () { ++_currentPhoto; }

   PhotoDatabase const& database () const { return _database; }
   LinkedList<PhotoKey> const* photoList () const { return _photoList; }
   LinkedList<PhotoKey>::ConstIterator currentPhoto () const { return _currentPhoto; }
   PhotoID currentPhotoID () const { return _currentPhoto.cref().id(); }
   PhotoMetadata const& currentMetadata () const { return _database.getMetadata(currentPhotoID()); }
};


//==============================================================================
// Inline Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
inline SessionData::SessionData (PhotoDatabase& database)
: _database(database),
_latlow(0.0), _lonlow(0.0), _lathigh(0.0), _lonhigh(0.0),
_photoList(0)
{}


#endif // SESSION_DATA
