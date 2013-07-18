// Cloud.js

var TAGGER = TAGGER || {};
TAGGER.cloud = (function () {
   var cloud = {};

   var db_url = "http://23.23.248.157/test/fcgi/main.fcgi";
   var db_pano_dir = "panos/";


   // parses feature json
   function read_feature_json (json) {
      console.log(json);

      var feature = {
         id: json._id.$oid,
         name: json.name
      };

      return feature;
   }

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
         nedges: 0,
         edges: []
      };

      for (var i=0; i<json.edges.length; i+=1) {
         pano.edges.push({
            panoid: json.edges[i].panoid,
            theta: json.edges[i].yaw
         });
         pano.nedges += 1;
      }

      return pano;
   }

   function read_tag_json (json) {
      console.log(json);

      var tag = {
         id: json._id.$oid,
         panorama: json.panorama.$oid,
         feature: json.feature.$oid,
         box: TAGGER.common.box(json.box)
      };

      return tag;
   }

   function read_tags_json (json) {
      var tags = [];
      for (var i=0; i<json.tags.length; i+=1) {
         tags.push(read_tag_json(json.tags[i]));
      }
      return tags;
   }


   // Gets a list of all the features in the database
   function get_features () {
      var params = { cmd: 'features' };
      var defer = $.Deferred();
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'json'
      }).done( function (data) {
         if (!$.isEmptyObject(data)) {
            var features = [];
            for (var i=0; i<data.features.length; i+=1) {
               features.push(read_feature_json(data.features[i]));
            }
            defer.resolve(features);
         } else {
            defer.reject({ type: TAGGER.error.databaseError, description: 'could not load features' });
         }
      }).fail( function (jqXHR, textStatus) {
         defer.reject({ type: TAGGER.error.ajaxError, description: textStatus });
      });
      return defer.promise();
   }
   cloud.get_features = get_features;


   // Loads metadata from the database.
   function get_metadata (id) {
      var params = { cmd: 'panorama', id: id };
      var defer = $.Deferred();
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'json'
      }).done( function (data) { 
         if (!$.isEmptyObject(data)) {
            defer.resolve(read_pano_json(data));
         } else {
            defer.reject({ type: TAGGER.error.databaseError, description: 'invalid panorama id' });
         }
      }).fail( function (jqXHR, textStatus) {
         defer.reject({ type: TAGGER.error.ajaxError, description: textStatus });
      });
      return defer.promise();
   }
   cloud.get_metadata = get_metadata;

   // Loads the panorama itself from the database.
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

   // Loads all the tags of the specified panorama from the database.
   function get_tags_by_panorama (id) {
      var params = { cmd: 'tags_by_panorama', panorama: id };
      var defer = $.Deferred();
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'json'
      }).done( function (data) { 
         if (!$.isEmptyObject(data)) {
            defer.resolve(read_tags_json(data));
         } else {
            defer.reject({ type: TAGGER.error.databaseError, description: 'invalid panorama id' });
         }
      }).fail( function (jqXHR, textStatus) {
         defer.reject({ type: TAGGER.error.ajaxError, description: textStatus });
      });
      return defer.promise();
   }
   cloud.get_tags_by_panorama = get_tags_by_panorama;

   // Saves a new tag.
   function save_tag (tagbox, panoramaID, featureID) {
      var defer = $.Deferred();
      var params = {
         cmd: 'insert_tag',
         panorama: panoramaID,
         feature: featureID,
         t1: tagbox.theta1,
         p1: tagbox.phi1,
         t2: tagbox.theta2,
         p2: tagbox.phi2
      };
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'json'
      }).done( function (data) {
         if (!$.isEmptyObject(data) && !data.hasOwnProperty('failure')) {
            defer.resolve(read_tag_json(data));
         } else {
            defer.reject({ type: TAGGER.error.databaseError, description: 'invalid panorama id' });
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
         id: tagid,
      };
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'json'
      }).done( function (data) {
         if (data.hasOwnProperty("success")) {
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


   // Attempts to find a panorama in the database with the specified (street view) panoid.
   function panoByPanoid (panoid) {
      var defer = $.Deferred();
      var params = {
         cmd: 'panorama_by_panoid',
         panoid: panoid
      };
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'json'
      }).done( function (data) {
         if (!$.isEmptyObject(data)) {
            defer.resolve(read_pano_json(data));
         } else {
            defer.reject({ type: TAGGER.error.databaseError, description: 'no such panorama exists' });
         }
      }).fail( function(jqXHR, textStatus) {
         defer.reject({ type: TAGGER.error.ajaxError, description: textStatus });
      });
      return defer.promise();
   }
   cloud.panoByPanoid = panoByPanoid;


   // Attempts to find a panorama in the database near the specified location.
   function panoNear (lat, lon) {
      var defer = $.Deferred();
      var params = {
         cmd: 'panorama_near',
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

   // Tells the database to download a panorama.
   function downloadPanorama (panoid) {
      var defer = $.Deferred();
      var params = {
         cmd: 'download_pano',
         'panoid': panoid
      };
      $.ajax({
         url: db_url,
         data: params,
         dataType: 'json'
      }).done( function (data) {
         if ( !$.isEmptyObject(data) && !data.hasOwnProperty('failure') ) {
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

