// Map.js

var TAGGER = TAGGER || {}
TAGGER.map = (function () {
   var map = {};

   var mapobj;
   var marker = null;
   var geocoder = new google.maps.Geocoder();

   function moveMarker (lat, lon) {
      var newloc = new google.maps.LatLng(lat, lon);
      if (marker) marker.setMap(null);
      marker = new google.maps.Marker({
         position: newloc,
         map: mapobj,
         clickable: false,
         draggable: false,
      });
   }
   map.moveMarker = moveMarker;

   function moveMap (lat, lon) {
      mapobj.setCenter(new google.maps.LatLng(lat, lon));
   }
   map.moveMap = moveMap;

   function initialize (lat, lon) {
      var mapOptions = {
         center: new google.maps.LatLng(lat, lon),
         zoom: 14,
         mapTypeId: google.maps.MapTypeId.ROADMAP,
         streetViewControl: false
      };
      mapobj = new google.maps.Map(document.getElementById("map"), mapOptions);

      google.maps.event.addListener(mapobj, 'click', function(event) {
         var lat = event.latLng.lat();
         var lon = event.latLng.lng();
         moveMarker(lat, lon)
         if (mapobj.onMoveMarker) {
            mapobj.onMoveMarker(lat, lon);
         }
      });
   }
   map.initialize = initialize;

   map.setOnMoveMarker = function (onMoveMarker) {
      mapobj.onMoveMarker = onMoveMarker;
   }

   return map;
}());
