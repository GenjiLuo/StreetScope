//==============================================================================
// Projection.cpp
// Created July 20, 2012
//==============================================================================

#include "Projection.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

using namespace std;


//==============================================================================
// Projection Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
void Projection::project (std::string panoPath, PhotoMetadata const& data) {
   // load the image
   Magick::Image panorama(panoPath);
   panorama.flop();
   cout << "Projecting " << panoPath << ".\n";

   // create base filename
   ostringstream fileBase;
   fileBase << _rootDir << data.id() << '_' << projectionName();

   // chop into pieces
   Magick::Image subimage;
   AngleRectangle projectedTag;
   for (start(); !done(); ++(*this)) {
      // project the image
      subimage = projectImage(panorama, data);
      int width  = subimage.size().width();
      int height = subimage.size().height();

      // construct filenames
      ostringstream panoFilename, tagFilename;
      panoFilename << fileBase.str() << '_' << projectionStep() << ".jpg";
      tagFilename  << fileBase.str() << '_' << projectionStep() << ".txt";

      // save the image
      subimage.write(panoFilename.str());

      // create the tag stack
      unsigned totalTags = data.tags().items();
      unsigned containedTags = 0;
      AngleRectangle projectedTag;
      Rectangle* pixelTags = new Rectangle [totalTags];
      TagID*     tagIDs    = new TagID     [totalTags];

      // project tags
      for (auto tagitr = data.tags().constIterator(); tagitr.valid(); ++tagitr) {
         tagitr.cref().bounds().normalize();
         if (projectTag(tagitr.cref().bounds(), projectedTag)) {
            tagIDs[containedTags] = tagitr.cref().tagID();
            quantize(projectedTag, pixelTags[containedTags], width, height);
            ++containedTags;
         }
      }

      // save tags
      ofstream tagFile(tagFilename.str().c_str());
      if (!tagFile) {
         cout << "Couldn't open " << tagFilename.str() << "! Be aware the projected image may be saved.\n";
         return;
      }
      tagFile << "Projection Parameters: ";
      projectionParameters(tagFile);
      tagFile << '\n';
      tagFile << "Projected Tags: " << containedTags << '\n';
      tagFile << "TagID, x1, x2, y1, y2\n";
      for (unsigned i=0; i<containedTags; ++i) {
         tagFile << tagIDs[i] << ", ";
         tagFile << pixelTags[i].x1 << ", " << pixelTags[i].x2 << ", ";
         tagFile << pixelTags[i].y1 << ", " << pixelTags[i].y2 << '\n';
      }

      // free the tag stack
      delete[] pixelTags;
      delete[] tagIDs;
   }
}

//------------------------------------------------------------------------------
void Projection::project (PhotoDatabase const& database) {
   for (auto itr = database.panoCItr(); itr.valid(); ++itr) {
      project(database.panoPath(itr.cref().id()), itr.cref());
   }
}


//==============================================================================
// Global Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
Rectangle anglesToPixels (AngleRectangle const& angles, PhotoMetadata const& data, Magick::Geometry const& geometry) {
   // extract width and height in pixels
   float width = static_cast<float>(geometry.width());
   float height = static_cast<float>(geometry.height());
   Rectangle r;

   // Thetas are stored counterclockwise from east. The returned pixel values are
   // for correctly oriented images (ie left to right is clockwise ie north -> east -> south).
   // (Note that panos are stored mirrored - flip them back to normal before using this function.)
   // Thus the lower x pixel value correponds to theta2.
   Angle theta1 = 180.0 + data.panoYaw() - angles.theta2;
   Angle theta2 = 180.0 + data.panoYaw() - angles.theta1;
   theta1.normalize1();
   theta2.normalize1();
   // now we have clockwise angles
   r.x1 = static_cast<unsigned>(theta1 * width / 360.0 + 0.5);
   r.x2 = static_cast<unsigned>(theta2 * width / 360.0 + 0.5);

   // Imagemagick measures y from the top, we measure phi from the horizon
   // where positive angles are toward the sky.
   r.y1 = static_cast<unsigned>((90.0 - angles.phi2) * height / 180.0 + 0.5);
   r.y2 = static_cast<unsigned>((90.0 - angles.phi1) * height / 180.0 + 0.5);

   return r;
}

//------------------------------------------------------------------------------
Magick::Image subpano (Magick::Image const& pano, PhotoMetadata const& data, AngleRectangle const& angleBox, Rectangle& box) {
   // convert to pixels
   Magick::Geometry geo = pano.size();
   box = anglesToPixels(angleBox, data, geo);

   // if the subimage range does not overlap the panorama's seam things are simple
   if (box.x2 >= box.x1) {
      Magick::Geometry subgeo(box.x2 - box.x1, box.y2 - box.y1, box.x1, box.y1);
      Magick::Image subpano = pano;
      subpano.crop(subgeo);
      subpano.page("0x0+0+0");
      return subpano;
   // if it does overlap the seam we have to do some stitching
   } else {
      // create necessary geometries
      unsigned subheight  = box.y2 - box.y1;
      unsigned tile1width = geo.width() - box.x1;
      unsigned tile2width = box.x2;
      Magick::Geometry subgeo( tile1width + tile2width, subheight );
      Magick::Geometry tile1geo( tile1width, subheight, box.x1, box.y1 );
      Magick::Geometry tile2geo( tile2width, subheight, 0,      box.y1 );

      // create subpano
      Magick::Image subpano(subgeo, "white");

      // extract and paste first half of subpano
      Magick::Image tile1 = pano;
      tile1.crop(tile1geo);
      subpano.composite(tile1, 0, 0, Magick::OverCompositeOp);

      // extract and paste second half of subpano
      Magick::Image tile2 = pano;
      tile2.crop(tile2geo);
      subpano.composite(tile2, tile1width, 0, Magick::OverCompositeOp);
      subpano.page("0x0+0+0");
      return subpano;
   }
}


//------------------------------------------------------------------------------
void quantize (AngleRectangle const& abstract, Rectangle& pix, int width, int height) {
   // convert to integers
   pix.x1 = static_cast<int>(abstract.theta1 * width + 0.5);
   pix.x2 = static_cast<int>(abstract.theta2 * width + 0.5);
   pix.y1 = static_cast<int>(abstract.phi1 * height + 0.5);
   pix.y2 = static_cast<int>(abstract.phi2 * height + 0.5);

   // check that everything is within proper ranges
   if (pix.x1 < 0) {
      cout << "x1 too low: " << pix.x1 << '\n';
      pix.x1 = 0;
   } else if (pix.x1 >= width) {
      cout << "x1 too high: " << pix.x1 << '\n';
      pix.x1 = width-1;
   }
   if (pix.x2 < 0) {
      cout << "x2 too low: " << pix.x2 << '\n';
      pix.x2 = 0;
   } else if (pix.x2 >= width) {
      cout << "x2 too high: " << pix.x2 << '\n';
      pix.x2 = width-1;
   }
   if (pix.y1 < 0) {
      cout << "y1 too low: " << pix.y1 << '\n';
      pix.y1 = 0;
   } else if (pix.y1 >= height) {
      cout << "y1 too high: " << pix.y1 << '\n';
      pix.y1 = height-1;
   }
   if (pix.y2 < 0) {
      cout << "y2 too low: " << pix.y2 << '\n';
      pix.y2 = 0;
   } else if (pix.y2 >= height) {
      cout << "y2 too high: " << pix.y2 << '\n';
      pix.y2 = height-1;
   }
}


//==============================================================================
// Cylindrical Projection Methods
//==============================================================================

//------------------------------------------------------------------------------
void CylinderParams::setFov (Angle fieldOfView) {
   fov     = fieldOfView;
   halfFov = 0.5 * fov;
   halfPaddedFov = tan(halfFov * 0.0174532925) * 57.2957795; // tan(fov/2 * pi/180) * 180/pi
   paddedFov = halfPaddedFov * 2.0;
}

//------------------------------------------------------------------------------
void CylinderParams::setCenter (Angle centerAngle) {
   center  = centerAngle;

   // Largest square section of the image that will be viewable in the projection.
   // Note that some area of the image above and below this range will be visible
   // near the image's centerline.
   crop.theta1 = center - halfFov;
   crop.theta2 = center + halfFov;
   crop.phi1 = -90.0;
   crop.phi2 =  90.0;
   crop.normalize();

   // section of the image that we will project (edges get cut off during projection)
   paddedCrop.theta1 = center - halfPaddedFov;
   paddedCrop.theta2 = center + halfPaddedFov;
   paddedCrop.phi1 = -90.0;
   paddedCrop.phi2 =  90.0;
   paddedCrop.normalize();
}

//------------------------------------------------------------------------------
void CylinderPlan::calculate (Angle fov, Angle minOverlap) {
   // set initial values
   thetaSteps = (unsigned)(360.0 / fov) + 1;
   deltaTheta = 360.0 / thetaSteps;

   // refine thetaSteps and deltaTheta
   while (fov - deltaTheta < minOverlap) {
      ++thetaSteps;
      deltaTheta = 360.0 / thetaSteps;
   }
}

//------------------------------------------------------------------------------
std::string CylinderProjection::projectionStep () const {
   std::ostringstream out;
   out << thetaStep;
   return out.str();
}

//------------------------------------------------------------------------------
void CylinderProjection::projectionParameters (std::ostream& file) const {
   file << params.crop.theta1 << " - " << params.crop.theta2;
}

//------------------------------------------------------------------------------
Magick::Image CylinderProjection::projectImage (Magick::Image const& image, PhotoMetadata const& data) {
   // project the image
   double paddedFov = params.paddedFov;
   Magick::Image subimage = subpano(image, data, params.paddedCrop);
   subimage.distort(MagickCore::Cylinder2PlaneDistortion, 1, &paddedFov, false);
   return subimage;
}

//------------------------------------------------------------------------------
bool CylinderProjection::projectTag (AngleRectangle const& tag, AngleRectangle& projectedTag) {
   if (!params.crop.containsTheta(tag))
      return false;   // too far left or right
   // at this point the tag may be too far up or down - we'll see soon

   projectedTag.theta1 = tag.theta1 - params.center; // deg
   projectedTag.theta2 = tag.theta2 - params.center; // deg
   projectedTag.theta1.normalize1();
   projectedTag.theta2.normalize1();

   float sampletheta;
   // if tag does not overlap center vertical line...
   if (projectedTag.theta1 <= projectedTag.theta2) {
      sampletheta = abs(0.5 * (projectedTag.theta1 + projectedTag.theta2)); // deg
   // if it does...
   } else {
      float maxtheta = projectedTag.theta2 > (360.0 - projectedTag.theta1) ? (float)projectedTag.theta2 : (360.0 - projectedTag.theta1);
      sampletheta = 0.5 * maxtheta; // deg
   }
   float tansamp = tan(0.0174532925 * sampletheta); // "rad"
   float sampleheight = sqrt(tansamp*tansamp + 1); // "rad"

   // Project polar angles.
   projectedTag.phi1 = (0.0174532925 * tag.phi1) * sampleheight; // "rad"
   projectedTag.phi2 = (0.0174532925 * tag.phi2) * sampleheight; // "rad"
   if (projectedTag.phi1 < -1.57079633 or projectedTag.phi2 > 1.57079633)
      // definitely too far up or down
      return false;

   // If we've made it this far the tag is in the projection.
   // Project azimuthal angles.
   projectedTag.theta1 = tan(projectedTag.theta1 * 0.0174532925) * 57.2957795;
   projectedTag.theta2 = tan(projectedTag.theta2 * 0.0174532925) * 57.2957795;
   // Shift.
   projectedTag.theta1 += params.halfPaddedFov;
   projectedTag.theta2 += params.halfPaddedFov;
   // Flip (angles measured ccw, but pixels cw).
   Angle temp = projectedTag.theta1;
   projectedTag.theta1 = params.paddedFov - projectedTag.theta2;
   projectedTag.theta2 = params.paddedFov - temp;
   // Rescale.
   projectedTag.theta1 = projectedTag.theta1 / params.paddedFov;
   projectedTag.theta2 = projectedTag.theta2 / params.paddedFov;

   // Shift.
   projectedTag.phi1 += 1.57079633;
   projectedTag.phi2 += 1.57079633;
   // Flip (positive phi means up, but pixels measured down from top).
   temp = projectedTag.phi1;
   projectedTag.phi1 = 3.14159265 - projectedTag.phi2;
   projectedTag.phi2 = 3.14159265 - temp;
   // Rescale.
   projectedTag.phi1 = projectedTag.phi1 / 3.14159265;
   projectedTag.phi2 = projectedTag.phi2 / 3.14159265;

   return true;
}

