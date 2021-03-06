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
char const* const tagCollection = "streetview.tags";
char const* const tagsetCollection = "streetview.tagsets";
char const* const configCollection = "streetview.config";


//==============================================================================
// Typedefs
//==============================================================================

// These are provided for convenience.
typedef mongo::OID PanoramaID;
typedef mongo::OID FeatureID;
typedef mongo::OID TagID;
typedef mongo::OID TagSetID;


//==============================================================================
// Data Management Classes
//==============================================================================

class Database {
friend class DatabaseTester;
//private:
public:
   mongo::DBClientConnection _mongo;
   boost::filesystem::path _panoDir;
   boost::filesystem::path _logFile;

public:
   //---------------------------------------------------------------------------
   // Basics
   
   // Initialization
   Database () {};
   bool connect ();

   // Indexing
   void ensureIndices ();

   // Counts
   unsigned panoramas () { return _mongo.count(panoramaCollection); }
   unsigned features () { return _mongo.count(featureCollection); }
   unsigned tagsets () { return _mongo.count(tagsetCollection); }

   // Accessors
   std::string panoramaDirectory () const { return _panoDir.string(); }
   std::string panoramaPath (mongo::OID panoramaID) const;
   std::string logFile () const { return _logFile.string(); }


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

   mongo::BSONObj findTag (mongo::OID tagID);
   std::auto_ptr<mongo::DBClientCursor> findTagsByPanorama (mongo::OID panoramaID);
   std::auto_ptr<mongo::DBClientCursor> findTagsByFeature (mongo::OID featureID);
   std::auto_ptr<mongo::DBClientCursor> findTags (mongo::OID panoramaID, mongo::OID featureID);


   //---------------------------------------------------------------------------
   // Update Operations

   void changeTagFeature (mongo::OID tagID, mongo::OID newFeatureID);


   //---------------------------------------------------------------------------
   // Removal Operations

   void removePanorama (mongo::OID panoramaID);
   void removeFeature (mongo::OID featureID);
   void removeTag (mongo::OID tagID);

};


#endif // DATABASE
