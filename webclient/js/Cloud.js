// Cloud.js

var TAGGER = TAGGER || {};
TAGGER.cloud = (function () {
   var cloud = {};

   var db_url = "http://23.23.248.157/fcgi/main.fcgi";
   var db_pano_dir = "panoramas/";


   // returns a panorama object parsed from our database
   function read_pano_json (json) {
      console.log(json);

      var pano = {
         id: json._id.$oid,
         panoid: json.panoid,
         indb: true,
         loc: { lat: parseFloat(json.location[1]),
                lon: parseFloat(json.location[0]) },
         panoYaw: parseFloat(json.orientation.yaw),
         tiltYaw: parseFloat(json.orientation.tiltYaw),
         tiltPitch: parseFloat(json.orientation.tiltPitch),
         unsavedTags: 0,
         ntags: 0,
         tags: {},
         nedges: 0,
         edges: []
      };

      /*
      $tag_nodes.find("Tag").each(function(i) {
         pano.tags[$(this).attr("id")] = {
            saved: true,
            box: TAGGER.common.box({
               theta1: parseFloat($(this).attr("theta1")) * TAGGER.common.degToRad,
               phi1:   parseFloat($(this).attr("phi1")) * TAGGER.common.degToRad,
               theta2: parseFloat($(this).attr("theta2")) * TAGGER.common.degToRad,
               phi2:   parseFloat($(this).attr("phi2")) * TAGGER.common.degToRad
            })
         };
         pano.ntags += 1;
      });
      */

      for (edge in json.edges) {
         pano.edges.push({
            panoid: edge.panoid,
            theta: edge.yaw
         });
         pano.nedges += 1;
      }

      return pano;
   }


   // Takes an id and a callback function, and calls
   // the callback with the requested pano object once the
   // data is loaded.
   function get_metadata (id) {
      var params = { cmd: 'metadata', id: id };
      var defer = $.Deferred();
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'xml'
      }).done( function (data) { 
         var $pano_xml = $(data).find("PhotoMetadata");
         if ($pano_xml.length > 0) {
            defer.resolve(read_pano_json($pano_xml));
         } else {
            defer.reject({ type: TAGGER.error.databaseError, description: 'invalid panorama id' });
         }
      }).fail( function(jqXHR, textStatus) {
         defer.reject({ type: TAGGER.error.ajaxError, description: textStatus });
      });
      return defer.promise();
   }
   cloud.get_metadata = get_metadata;

   function get_data (id) {
      var img = new Image();
      var defer = $.Deferred();
      img.addEventListener('load', function () {
         defer.resolve(img);
      });
      img.addEventListener('error', function () {
         defer.reject({ type: TAGGER.error.ajaxError, description: 'image load failed' });
      });
      img.crossOrigin = '';
      img.src = db_pano_dir + id + '.jpg';
      return defer.promise();
   }
   cloud.get_data = get_data;

   // Saves a new tag.
   function save_tag (tag, tempid, photoid) {
      var defer = $.Deferred();
      var params = {
         cmd: 'new_tag',
         id: photoid,
         t1: tag.box.theta1 * TAGGER.common.radToDeg,
         p1: tag.box.phi1   * TAGGER.common.radToDeg,
         t2: tag.box.theta2 * TAGGER.common.radToDeg,
         p2: tag.box.phi2   * TAGGER.common.radToDeg
      };
      if (params.t1 < 0) { params.t1 += 360; }
      if (params.t2 < 0) { params.t2 += 360; }
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'xml'
      }).done( function (data) {
         var $resultStatus = $(data).find("Status");
         var $tagid = $(data).find("TagID");
         if ($resultStatus.length > 0 && $resultStatus.text() === "success" && $tagid.length > 0) {
            defer.resolve($tagid.text());
         } else {
            defer.reject({type: TAGGER.error.databaseError, description: 'could not save tag' });
         }
      }).fail( function(jqXHR, textStatus) {
         defer.reject({ type: TAGGER.error.ajaxError, description: textStatus });
      });
      return defer.promise();
   }
   cloud.save_tag = save_tag;

   // Deletes an existing tag.
   function delete_tag (tagid) {
      var defer = $.Deferred();
      var params = {
         cmd: 'remove_tag',
         tag_id: tagid,
      };
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'xml'
      }).done( function (data) {
         var $resultStatus = $(data).find("Status");
         if ($resultStatus.text() === "success") {
            defer.resolve();
         } else {
            defer.reject({ type: TAGGER.error.databaseError, description: 'could not remove tag' });
         }
      }).fail( function(jqXHR, textStatus) {
         defer.reject({ type: TAGGER.error.ajaxError, description: textStatus });
      });
      return defer.promise();
   }
   cloud.delete_tag = delete_tag;


   // Attempts to find a panorama in the database with the specified
   // (street view) panoid and location. It calls the callback
   // function once this has been checked.
   function panoByPanoidNear (panoid, lat, lon) {
      var defer = $.Deferred();
      var params = {
         cmd: 'pano_id_near',
         'lat': lat,
         'lon': lon,
         pano_id: panoid
      };
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'xml'
      }).done( function (data) {
         var $pano = $(data).find("PhotoMetadata");
         if ($pano.length > 0) {
            defer.resolve(read_pano_json($pano));
         } else {
            defer.reject({ type: TAGGER.error.databaseError, description: 'no such panorama exists' });
         }
      }).fail( function(jqXHR, textStatus) {
         defer.reject({ type: TAGGER.error.ajaxError, description: textStatus });
      });
      return defer.promise();
   }
   cloud.panoByPanoidNear = panoByPanoidNear;


   function panoNear (lat, lon) {
      var defer = $.Deferred();
      var params = {
         cmd: 'panos_near',
         'lat': lat,
         'lon': lon
      };
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'json'
      }).done( function (data) {
         if (!$.isEmptyObject(data)) {
            defer.resolve(read_pano_json(data));
         } else {
            defer.reject({ type: TAGGER.error.databaseError, description: 'no panorama near this location' });
         }
      }).fail( function (jqXHR, textStatus) {
         defer.reject({ type: TAGGER.error.ajaxError, description: textStatus });
      });
      return defer.promise();
   }
   cloud.panoNear = panoNear;

   // params should have lat, lon, and panos = [panobj0, panobj1, ...].
   // We use the taxi cab metric for now.
   function findClosestPanorama (lat, lon, panos) {
      var minIndex = 0;
      var minDistance = Math.abs(lat - panos[0].loc.lat) + Math.abs(lon - panos[0].loc.lon);
      var distance = null;
      for (i=0; i<panos.length; i += 1) {
         distance = Math.abs(lat - panos[i].loc.lat) + Math.abs(lon - panos[i].loc.lon);
         if (distance < minDistance) {
            minDistance = distance;
            minIndex = i;
         }
      }
      return panos[minIndex];
   }


   function downloadPanorama (panoid) {
      var defer = $.Deferred();
      var params = {
         cmd: 'download_pano',
         'pano_id': panoid
      };
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'xml'
      }).done( function (data) {
         console.log(data);
         if (!$.isEmptyObject(data)) {
            defer.resolve(read_pano_json(data));
         } else {
            defer.reject({ type: TAGGER.error.databaseError, description: 'unable to save the panorama' });
         }
      }).fail( function(jqXHR, textStatus) {
         defer.reject({ type: TAGGER.error.ajaxError, description: textStatus });
      });
      return defer;
   }
   cloud.downloadPanorama = downloadPanorama;


   return cloud;
}());

