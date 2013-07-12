//==============================================================================
// Projection.h
// Created July 20, 2012
//==============================================================================

#ifndef STREETVIEW_PROJECTION
#define STREETVIEW_PROJECTION

#include <string>
#include <sstream>

#include "Database.h"
#include "Magick++.h"


//------------------------------------------------------------------------------
// This should live somewhere else.
struct Rectangle {
   int x1;
   int x2;
   int y1;
   int y2;

   bool contains (Rectangle const& r) const { return (x1 <= r.x1 and x2 >= r.x2 and y1 <= r.y1 and y2 >= r.y2); }
};


//==============================================================================
// Virtual Class Projection
//==============================================================================

//------------------------------------------------------------------------------
class Projection {
private:
   std::string _rootDir;

public:
   virtual ~Projection () {};

   virtual std::string projectionName () const = 0;
   virtual std::string projectionStep () const = 0;
   virtual void projectionParameters (std::ostream& file) const = 0;

   inline std::string paramsFilePath () const;
   inline std::string tagFilePath () const;
   inline std::string stepPath (PhotoID panoid) const;

   // All Projections have their own setPlan method that takes arguments
   // specific to each derived class.
   // virtual void setPlan (...) = 0;
   virtual void start      () = 0;
   virtual bool done       () = 0;
   virtual void operator++ () = 0;

   virtual Magick::Image projectImage (Magick::Image const& image, PhotoMetadata const& data) = 0;
   virtual bool          projectTag   (AngleRectangle const& tag, AngleRectangle& projectedTag) = 0;

   void setRootDir (std::string dir) { _rootDir = dir; }
   std::string identifier () const;
   void project (std::string panoPath, PhotoMetadata const& data, std::ostream& tagFile);
   void project (PhotoDatabase const& database);

   bool writeParameters () const;
};


//==============================================================================
// Global Helper Methods
//==============================================================================

//------------------------------------------------------------------------------
Rectangle anglesToPixels (AngleRectangle const& angles, PhotoMetadata const& data, Magick::Geometry const& geometry);

//------------------------------------------------------------------------------
// image must be oriented properly (ie clockwise)
Magick::Image subpano (Magick::Image const& image, PhotoMetadata const& data, AngleRectangle const& angleBox, Rectangle& box);

//------------------------------------------------------------------------------
inline Magick::Image subpano (Magick::Image const& image, PhotoMetadata const& data, AngleRectangle const& angleBox) {
   Rectangle box; return subpano(image, data, angleBox, box);
}

//------------------------------------------------------------------------------
void quantize (AngleRectangle const& abstract, Rectangle& pixels, int width, int height);


//==============================================================================
// Cylindrical Projection Methods
//==============================================================================

//------------------------------------------------------------------------------
// Various angles used throughout cylindrical projection.
// All parameters are derived from the first two.
struct CylinderParams {
   Angle center;
   Angle fov;
   Angle halfFov;
   Angle paddedFov;
   Angle halfPaddedFov;
   AngleRectangle crop;
   AngleRectangle paddedCrop;

   CylinderParams () {}
   CylinderParams (Angle fieldOfView, Angle centerAngle) { setFov(fieldOfView); setCenter(centerAngle); }
   void setFov (Angle fieldOfView);
   void setCenter (Angle centerAngle);
};

//------------------------------------------------------------------------------
struct CylinderPlan {
   unsigned thetaSteps;
   Angle deltaTheta;

   CylinderPlan () {}
   void calculate (Angle fov, Angle minOverlap);
   Angle center (unsigned step) { return deltaTheta * step; }
};

//------------------------------------------------------------------------------
struct CylinderProjection : public Projection {
   CylinderPlan plan;
   CylinderParams params;
   unsigned thetaStep;

   CylinderProjection () {}
   void setPlan (Angle fov, Angle minOverlap) { plan.calculate(fov, minOverlap); params.setFov(fov); }
   void setStep (unsigned step) { thetaStep = step; params.setCenter(plan.center(step)); }

   std::string projectionName () const { return "cyl"; }
   std::string projectionStep () const;
   void projectionParameters (std::ostream& file) const;

   void start () { setStep(0); }
   bool done () { return thetaStep >= plan.thetaSteps; }
   void operator++ () { ++thetaStep; params.setCenter(plan.center(thetaStep)); }

   Magick::Image projectImage (Magick::Image const& image, PhotoMetadata const& data);
   bool projectTag (AngleRectangle const& tag, AngleRectangle& projectedTag);
};


//==============================================================================
// Inline Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
std::string Projection::paramsFilePath () const {
   std::ostringstream filename;
   filename << _rootDir << "params.txt";
   return filename.str();
}

//------------------------------------------------------------------------------
std::string Projection::tagFilePath () const {
   std::ostringstream filename;
   filename << _rootDir << "tags.txt";
   return filename.str();
}

//------------------------------------------------------------------------------
std::string Projection::stepPath (PhotoID panoid) const {
   std::ostringstream filename;
   filename << _rootDir << panoid << '_' << projectionName() << '_' << projectionStep() << ".jpg";
   return filename.str();
}


#endif // STREETVIEW_PROJECTION
