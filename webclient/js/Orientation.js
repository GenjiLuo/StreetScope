// Orientation.js

var TAGGER = TAGGER || {}

TAGGER.orientation = (function () {
   var o = {};

   // converts cartesian to spherical
   function sCoords (cCoords) {
      var sCoords = {};
      var flat_r = Math.sqrt(cCoords.x * cCoords.x + cCoords.z * cCoords.z);
      sCoords.theta = Math.atan2(-cCoords.z, cCoords.x);
      sCoords.phi   = Math.atan2(cCoords.y, flat_r);
      return sCoords;
   }
   o.sCoords = sCoords;

   // converts spherical to cartesian
   function cCoords (sCoords) {
      var cCoords = {};
      var r = sCoords.r || 1;
      var flat_r = r * Math.cos(sCoords.phi);
      cCoords.y  = r * Math.sin(sCoords.phi);
      cCoords.x  =  flat_r * Math.cos(sCoords.theta);
      cCoords.z  = -flat_r * Math.sin(sCoords.theta);
      return cCoords;
   }
   o.cCoords = cCoords;

   // rotates a cartesian vector about the z axis
   function rotz (p, theta) {
      var cost = Math.cos(theta);
      var sint = Math.sin(theta);
      return { x: p.x * cost - p.y * sint,
               y: p.y * cost + p.x * sint,
               z: p.z };
   }


   // Field of view and derivative variables.
   o.fov_theta = 0.0;
   o.fov_phi   = 0.0;
   o.spos  = { x: 0, y: 0, z: 0 };
   o.right = { x: 0, y: 0, z: 0 };
   o.down  = { x: 0, y: 0, z: 0 };

   // the direction we're facing (in the ground plane)
   o.view        = { theta: 0.0, phi: 0.0 };

   // Used for dragging. Set by cacheView.
   o.cachedView  = { theta: 0.0, phi: 0.0 };
   var cachedSpos  = { x: 0, y: 0, z: 0 };
   var cachedRight = { x: 0, y: 0, z: 0 };
   var cachedDown  = { x: 0, y: 0, z: 0 };


   // sets the field of view
   o.setFov = function (params) {
      var fovt = params.theta;
      var fovp = params.phi;
      o.fov_theta = fovt;
      o.fov_phi = fovp;
      var tant = Math.tan(fovt / 2);
      var tanp = Math.tan(fovp / 2);
      o.spos  = { x: 1, y:    tanp, z:   -tant };
      o.right = { x: 0, y:       0, z:  2*tant };
      o.down  = { x: 0, y: -2*tanp, z:       0 };
   };

   // caches the current view
   o.cacheView = function () {
      o.cachedView = { theta: o.view.theta, phi: o.view.phi };
      cachedSpos = rotz(o.spos, o.cachedView.phi);
      // we don't need to rotate this because it lies along the z axis
      cachedRight = o.right;
      cachedDown = rotz(o.down, o.cachedView.phi);
   };

   // retrieves cached view
   o.getCachedView = function () {
      return o.cachedView;
   };

   // changes view to look at given coords
   function lookAt (sCoords) {
      o.view = sCoords;
   }
   o.lookAt = lookAt;

   // returns the current view in cartesian coordinates
   o.cView = function () {
      return cCoords(o.view);
   };

   // Given normalized screen coordinates, this function
   // returns the sperhical coordinates of the mouse's projection.
   function project (screenCoords) {
      var mcoords = { x: cachedSpos.x + screenCoords.y * cachedDown.x,
                      y: cachedSpos.y + screenCoords.y * cachedDown.y,
                      z: cachedSpos.z + screenCoords.x * cachedRight.z };
      return sCoords(mcoords);
   }
   o.project = project;

   return o;
}());


TAGGER.mouse = (function () {
   var mouse = {};

   // mouse variables
   down_theta  = 0.0;
   down_phi    = 0.0;
   track_theta = 0.0;
   track_phi   = 0.0;

   mouse.downtheta = function () { return down_theta; };
   mouse.downphi = function () { return down_phi; };
   mouse.ttheta = function () { return track_theta; };
   mouse.tphi = function () { return track_phi; };

   mouse.down = function (loc) {
      down_theta = loc.theta;
      down_phi   = loc.phi;
   };

   function track (loc) {
      track_theta = loc.theta;
      track_phi   = loc.phi;
   }
   mouse.track = track;

   mouse.delta = function () {
      return {
         theta: track_theta - down_theta,
         phi:   track_phi   - down_phi
      };
   };

   mouse.box = function () {
      return TAGGER.common.box({
         theta1: down_theta,
         phi1: down_phi,
         theta2: track_theta,
         phi2: track_phi
      });
   };

   return mouse;
}());

   
