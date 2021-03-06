// Loader.js
// Functions that help load data from Google Street View

var TAGGER = TAGGER || {}
TAGGER.gsv = (function () {
   var gsv = {};

   var loader = null;
   var sv_url = "http://maps.google.com/cbk"; 

   // converts from Google's yaw system (degrees clockwise from north)
   // to ours (radians counterclockwise from east)
   function convertYaw (theta) {
      var newTheta;
      if (theta <= 90) {
         newTheta = 90 - theta;
      } else {
         newTheta = 450 - theta;
      }
      return newTheta * TAGGER.common.degToRad;
   }

   // converts from Google's pitch system (degrees up from perpendicular to gravitiational up)
   // to ours (radians up from perpendicular to gravitational up)
   function convertPitch (phi) {
      return phi * TAGGER.common.degToRad;
   }

   gsv.initialize = function () {
      loader = new GSVPANO.PanoLoader();

      /*
      loader.onPanoramaLoad = function () {
         TAGGER.app.log("on pano load");
         TAGGER.graphics.changeTexture(this.canvas);
         //showMessage( 'Panorama tiles loaded.<br/>The images are ' + this.copyright );
         //showProgress( false );
      };
      */

      /*
      loader.onProgress = function( p ) {
         TAGGER.app.setProgress( p );
      };
      */
			
      loader.onNoPanoramaData = function () {
         console.log("no pano data");
      };

      loader.onPanoramaData = function () {
         console.log("loaded pano metadata. loading tiles...");
      };

      gsv.setZoom = function (z) {
         loader.setZoom(z);
      }
   };


   function read_pano_json (json) {
      var pano = {
         id: 'n/a',
         panoid: json.Location.panoId,
         indb: false,
         loc: { lat: parseFloat(json.Location.lat),
                lon: parseFloat(json.Location.lng) },
         panoYaw: convertYaw(json.Projection.pano_yaw_deg),
         tiltYaw: convertYaw(json.Projection.tilt_yaw_deg),
         tiltPitch: convertPitch(json.Projection.tilt_pitch_deg),
         unsavedTags: 0,
         ntags: 0,
         tags: {},
         nedges: 0,
         edges: []
      };

      for (i in json.Links) {
         pano.edges.push({
            panoid: json.Links[i].panoId,
            theta: convertYaw(json.Links[i].yawDeg)
         });
         pano.nedges += 1;
      }

      return pano;
   }


   function panoidNear (lat, lon) {
      var defer = $.Deferred();
      var loc = new google.maps.LatLng(lat, lon);
      loader.panoClient().getPanoramaByLocation(loc, 50, function (result, status) {
         if (status === google.maps.StreetViewStatus.OK) {
            defer.resolve(result.location.pano);
         } else if (status === google.maps.StreetViewStatus.ZERO_RESULTS) {
            defer.reject({ type: TAGGER.error.streetViewError, description: 'no nearby panorama' });
         } else {
            defer.reject({ type: TAGGER.error.ajaxError, description: 'could not communicate with Google Street View' });
         }
      });
      return defer.promise();
   }
   gsv.panoidNear = panoidNear;

   function getMetadata (panoid) {
      var defer = $.Deferred();
      var params = { 'output': 'json', 'panoid': panoid };
      $.ajax({
         url: sv_url,
         data: params,
         dataType: 'jsonp'
      }).done( function (data) {
         // Google returns 404 if the panoid is incorrect,
         // so if we've made it this far we must have data.
         defer.resolve(read_pano_json(data));
      }).fail( function(jqXHR, textStatus) {
         if (jqXHR.status === 404) {
            defer.reject({ type: TAGGER.error.streetViewError, description: 'invalid pano id' });
         } else {
            defer.reject({ type: TAGGER.error.ajaxError, description: 'could not communicate with Google Street View' });
         }
      });
      return defer.promise();
   }
   gsv.getMetadata = getMetadata;


   function getData (panoid) {
      var defer = $.Deferred();
      loader.onPanoramaLoad = function () {
         defer.resolve(loader.canvas);
      };
      loader.composePanorama(panoid);
      return defer.promise();
   }
   gsv.getData = getData;

   return gsv;
}());
