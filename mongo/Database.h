//==============================================================================
// Database.h
// Created January 6, 2012.
//==============================================================================

#ifndef DATABASE
#define DATABASE

#include <boost/filesystem.hpp>
#include "client/dbclient.h"
#include <string>
#include "Types.h"

// debug
#include <iostream>


//==============================================================================
// Constants
//==============================================================================

char const* const panoramaCollection = "streetview.panoramas";
char const* const featureCollection = "streetview.features";
char const* const tagsetCollection = "streetview.tagsets";


//==============================================================================
// Typedefs
//==============================================================================

// These are provided for convenience.
typedef mongo::OID PanoramaID;
typedef mongo::OID FeatureID;
typedef mongo::OID TagID;


//==============================================================================
// Data Management Classes
//==============================================================================

class Database {
friend class DatabaseTester;
//private:
public:
   mongo::DBClientConnection _mongo;
   boost::filesystem::path _panoDir;

public:
   //---------------------------------------------------------------------------
   // Basics
   
   // Initialization
   // All of these methods must be called before any others.
   Database () {};
   bool connect ();
   void setPanoDir (std::string const& panoDir);

   // Indexing
   void ensureIndexes ();

   // Counts
   unsigned panoramas () { return _mongo.count(panoramaCollection); }
   unsigned features () { return _mongo.count(featureCollection); }
   unsigned tagsets () { return _mongo.count(tagsetCollection); }
   //unsigned tags () const { return _tags; }

   // Accessors
   std::string panoDirectory () const { return _panoDir.string(); }
   std::string panoPath (mongo::OID panoramaID) const;


   //---------------------------------------------------------------------------
   // Insertion Operations
   
   mongo::OID insertPanorama (Panorama const& panorama);
   mongo::OID insertFeature (Feature const& feature);

   // Adds an edge to the specified PhotoMetadata object.
   void insertEdge (mongo::OID panoramaID, Angle angle, char const* panoid);
   
   // Adds a new tag to a photo
   mongo::OID insertTag (mongo::OID panoramaID, mongo::OID featureID, AngleBox const& box);


   //---------------------------------------------------------------------------
   // Query Operations

   mongo::BSONObj findPanorama (mongo::OID panoramaID);
   mongo::BSONObj findPanorama (Location location);
   mongo::BSONObj findPanorama (char const* panoid);

   mongo::BSONObj findFeature (mongo::OID featureID);

   std::auto_ptr<mongo::DBClientCursor> findTagSets (mongo::OID panoramaID);


   //---------------------------------------------------------------------------
   // Removal Operations

   void removePanorama (mongo::OID panoramaID);
   void removeFeature (mongo::OID featureID);
   void removeTag (mongo::OID featureID);

};


#endif // DATABASE
