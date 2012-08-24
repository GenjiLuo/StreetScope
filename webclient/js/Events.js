// Events.js

var TAGGER = TAGGER || {};
TAGGER.handlers = (function () {
   var handlers = {};

   var _taggingDisabled = true;
   var _onGooglePano = false;

   function disableTagging () {
      _taggingDisabled = true;
   }
   handlers.disableTagging = disableTagging;

   function enableTagging () {
      _taggingDisabled = false;
   }
   handlers.enableTagging = enableTagging;

   handlers.onGooglePano = function (bool) {
      _onGooglePano = bool;
   };

   // gets the spherical coordinates of a mouse event
   function sphericalCoords (ev) {
      var point = TAGGER.graphics.eventCoords(ev);
      return TAGGER.orientation.sCoords(point);
   }

   // mouse down handler for dragging
   function down_drag (coords) {
      TAGGER.orientation.cacheView();
      var cache = TAGGER.orientation.getCachedView();
      TAGGER.mouse.down({ theta: coords.theta - cache.theta, phi: coords.phi });
      $("body").on('mousemove', move_drag);
      $("body").on('mouseup', up_drag);
   }

   // mouse down handler for tagging
   function down_tag (coords) {
      TAGGER.mouse.down(coords);
      $("body").on('mousemove', move_tag);
      $("body").on('mouseup', up_tag);
   }

   // handles all mouse down events
   function down_all (e) {
      if (e.target === TAGGER.graphics.domElement()) {
         e.preventDefault();
         var coords = sphericalCoords(e);
         if (!e.shiftKey) {
            down_drag(coords);
         } else {
            if (_taggingDisabled) {
               return;
            }
            if (_onGooglePano) {
               TAGGER.app.showMessage('You must save this panorama to the database before adding tags.');
               return;
            }
            down_tag(coords);
         }
      }
   }

   // movement handler while dragging
   function move_drag (e) {
      var screenCoords = { x: e.clientX/window.innerWidth, y: e.clientY/window.innerHeight };
      var scoords = TAGGER.orientation.project(screenCoords);
      TAGGER.mouse.track(scoords);
      var delta = TAGGER.mouse.delta();
      TAGGER.orientation.view.theta = TAGGER.orientation.cachedView.theta - delta.theta;
      TAGGER.orientation.view.phi   = TAGGER.orientation.cachedView.phi   - delta.phi;
   }
   //handlers.move_drag = move_drag;

   // movement handler while tagging
   function move_tag (e) {
      var coords = sphericalCoords(e);
      TAGGER.mouse.track(coords);
      TAGGER.graphics.removeTag('temp');
      TAGGER.graphics.addTag(TAGGER.mouse.box(), 'temp');
   }
   //handlers.move_tag = move_tag;


   // called in all up_xxx functions
   function up_common () {
      $("body").off('mousemove');
      $("body").off('mouseup');
      //state == 0;
   }
   //handlers.up_common = up_common;

   // mouse up handler when dragging
   function up_drag (e) {
      up_common();
      move_drag(e);
   }
   //handlers.up_drag = up_drag;

   // mouse up handler when tagging
   function up_tag (e) {
      up_common();
      TAGGER.graphics.removeTag('temp');
      TAGGER.tags.createTag(TAGGER.mouse.box());
   }
   //handlers.up_tag = up_tag;


   handlers.initialize = function (element) {
      $(element).mousedown(down_all);
   };
      


   return handlers;
}());

