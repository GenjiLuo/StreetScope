// Tags.js

var TAGGER = TAGGER || {}
TAGGER.tags = (function () {
   var tags = {};

   var _newTags = 0;
   var _pano = {};

   // shows a tag
   function showTag (tag, id) {
      TAGGER.dhtml.addTag({
         'id': id,
         'box': tag.box
      });
      TAGGER.graphics.addTag(tag.box, id);
   }

   // creates a tag
   function createTag (tagbox) {
      TAGGER.app.showMessage('Saving tag to database...');

      var tempid = 'newtag_' + _newTags;
      var tag = { saved: false, box: tagbox };
      _pano.tags[tempid] = tag;
      _pano.ntags += 1;
      _newTags += 1;

      TAGGER.dhtml.updateTagNumber(_pano.ntags);
      showTag(tag, tempid);

      // send data to database
      TAGGER.cloud.save_tag(tag, tempid, _pano.id).done( function(newid) {
         delete _pano.tags[tempid];
         tag.saved = true;
         _pano.tags[newid] = tag;
         TAGGER.dhtml.updateTagID(tempid, newid);
         TAGGER.graphics.updateTagID(tempid, newid);
         TAGGER.app.showMessage('The tag was saved to the database.');
      }).fail( function(errorObj) {
         if (errorObj.type === TAGGER.error.databaseError) {
            tagger.app.showError('Database was unable to save tag.');
         } else {
            tagger.app.showError('Could not communicate with the database.');
         }
         delete _pano.tags[tempid];
         hideTag(tempid);
      });
   }
   tags.createTag = createTag;

   // hides a tag
   function hideTag (tagid) {
      TAGGER.dhtml.removeTag(tagid);
      TAGGER.graphics.removeTag(tagid);
   }

   // deletes a tag
   function deleteTag (tagid) {
      if (_pano.tags[tagid]) {
         TAGGER.app.showMessage('Removing tag from database...');
         hideTag(tagid);
         _pano.ntags -= 1;
         TAGGER.dhtml.updateTagNumber(_pano.ntags);
         TAGGER.cloud.delete_tag(tagid).done( function() {
            delete _pano.tags[tagid];
            TAGGER.app.showMessage('The tag was removed from the database.');
         }).fail( function (errorObj) {
            if (errorObj.type === TAGGER.error.databaseError) {
               tagger.app.showError('Database was unable to delete the tag.');
            } else {
               tagger.app.showError('Could not communicate with the database.');
            }
            delete _pano.tags[tagid];
         });
      }
   }
   tags.deleteTag = deleteTag;

   // looks at a tag
   function locateTag (tagid) {
      TAGGER.orientation.lookAt(_pano.tags[tagid].box.center());
   }
   tags.locateTag = locateTag;


   function changePano (pano) {
      // remove old pano's tags and edges
      if (_pano.tags) {
         for (tagid in _pano.tags) {
            hideTag(tagid);
         }
      }

      // display new ones
      TAGGER.dhtml.updateTagNumber(pano.ntags);
      for (tagid in pano.tags) {
         showTag(pano.tags[tagid], tagid);
      }

      // change pano object
      _newTags = 0;
      _pano = pano;
   }
   tags.changePano = changePano;

   tags.initialize = function (pano) {
      changePano(pano);
   };


   return tags;
}());
