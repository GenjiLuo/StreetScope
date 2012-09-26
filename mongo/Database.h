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
char const* const configCollection = "streetview.config";


//==============================================================================
// Typedefs
//==============================================================================

// These are provided for convenience.
typedef mongo::OID PanoramaID;
typedef mongo::OID FeatureID;
typedef mongo::OID TagSetID;
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
   Database () {};
   bool connect ();

   // Indexing
   void ensureIndexes ();

   // Counts
   unsigned panoramas () { return _mongo.count(panoramaCollection); }
   unsigned features () { return _mongo.count(featureCollection); }
   unsigned tagsets () { return _mongo.count(tagsetCollection); }

   // Accessors
   std::string panoramaDirectory () const { return _panoDir.string(); }
   std::string panoramaPath (mongo::OID panoramaID) const;


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
   std::auto_ptr<mongo::DBClientCursor> findFeatures ();

   // returns the specified tagset
   mongo::BSONObj findTagSet (mongo::OID tagsetID);
   // returns the tagset for the given panorama and feature
   mongo::BSONObj findTagSet (mongo::OID panoramaID, mongo::OID featureID);
   // this returns the tagset containing the specified tag
   mongo::BSONObj findTagSetWithTag (mongo::OID tagID);
   // returns all tagsets of the given panorama
   std::auto_ptr<mongo::DBClientCursor> findPanoramaTagSets (mongo::OID panoramaID);
   // returns all tagsets of the given feature
   std::auto_ptr<mongo::DBClientCursor> findFeatureTagSets (mongo::OID featureID);


   //---------------------------------------------------------------------------
   // Removal Operations

   void removePanorama (mongo::OID panoramaID);
   void removeFeature (mongo::OID featureID);
   void removeTag (mongo::OID tagID);

};


#endif // DATABASE
