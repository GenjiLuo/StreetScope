// Tags.js

var TAGGER = TAGGER || {}
TAGGER.tags = (function () {
   var tags = {};

   var _pano = {};

   // stores all tags
   var _newTags = 0;
   var _ntags = 0;
   var _tagtable = {};

   // stores all features
   var _features = {
      length: 0,
      list: [],
      idToName: {},
      nameToID: {},
      addFeature: function (feature) {
         this.list.push(feature);
         this.idToName[feature.id] = feature.name;
         this.nameToID[feature.name] = feature.id;
         this.length += 1;
      },
      initialize: function (feats) {
         console.log(feats);
         for (var i=0; i<feats.length; i+=1) {
            this.addFeature(feats[i]);
         }
      },
      findByName: function (name) {
         if (this.nameToID.hasOwnProperty(name)) {
            return { id: this.nameToID[name], name: name };
         } else {
            return {};
         }
      },
      findByID: function (id) {
         if (this.idToName.hasOwnProperty(id)) {
            return { id: id, name: this.idToName[id] };
         } else {
            return {};
         }
      }
   };

   var _defaultFeature;

   // shows a tag
   function showTag (tag, id) {
      TAGGER.dhtml.addTag({
         'id': id,
         'featureID': tag.feature,
         'featureName': _features.findByID(tag.feature).name,
         'box': tag.box
      });
      TAGGER.graphics.addTag(tag.box, id);
   }

   // hides a tag
   function hideTag (tagid) {
      TAGGER.dhtml.removeTag(tagid);
      TAGGER.graphics.removeTag(tagid);
   }

   // creates a tag
   function createTag (tagbox) {
      TAGGER.app.showMessage('Saving tag to database...');

      var tempid = 'newtag_' + _newTags;
      var tag = {
         saved: false,
         feature: _defaultFeature,
         box: tagbox
      };
      _ntags += 1;
      _newTags += 1;

      TAGGER.dhtml.updateTagNumber(_ntags);
      showTag(tag, tempid);

      // send data to database
      TAGGER.cloud.save_tag(tagbox, _pano.id, _defaultFeature).done( function(newtag) {
         _tagtable[newtag.id] = newtag;
         TAGGER.dhtml.updateTagID(tempid, newtag.id);
         TAGGER.graphics.updateTagID(tempid, newtag.id);
         TAGGER.app.showMessage('The tag was saved to the database.');
      }).fail( function(errorObj) {
         if (errorObj.type === TAGGER.error.databaseError) {
            TAGGER.app.showError('Database was unable to save tag.');
         } else {
            TAGGER.app.showError('Could not communicate with the database.');
         }
         hideTag(tempid);
         _ntags -= 1;
         TAGGER.dhtml.updateTagNumber(_ntags);
      });
   }
   tags.createTag = createTag;

   // deletes a tag
   function deleteTag (tagid) {
      if (_tagtable[tagid]) {
         TAGGER.app.showMessage('Removing tag from database...');
         hideTag(tagid);
         _ntags -= 1;
         TAGGER.dhtml.updateTagNumber(_ntags);
         TAGGER.cloud.delete_tag(tagid).done( function() {
            delete _tagtable[tagid];
            TAGGER.app.showMessage('The tag was removed from the database.');
         }).fail( function (errorObj) {
            if (errorObj.type === TAGGER.error.databaseError) {
               TAGGER.app.showError('Database was unable to delete the tag.');
            } else {
               TAGGER.app.showError('Could not communicate with the database.');
            }
            _ntags += 1;
            TAGGER.dhtml.updateTagNumber(_ntags);
            showTag(_tagtable[tagid], tagid);
         });
      }
   }
   tags.deleteTag = deleteTag;

   // looks at a tag
   function locateTag (tagid) {
      TAGGER.orientation.lookAt(_tagtable[tagid].box.center());
   }
   tags.locateTag = locateTag;

   // changes a tag's feature
   function changeTagFeature (tagID, featureID) {
      console.log("changing feature of tag " + tagID + " to " + featureID);
   }
   tags.changeTagFeature = changeTagFeature;


   function changePano (pano, tags) {
      // remove old pano's tags and edges
      for (tag in _tagtable) {
         if (_tagtable.hasOwnProperty(tag)) {
            hideTag(tag);
         }
      }

      // display new ones
      _ntags = 0;
      var newtagtable = {};
      for (var i=0; i<tags.length; i+=1) {
         showTag(tags[i], tags[i].id);
         newtagtable[tags[i].id] = tags[i];
         _ntags += 1;
      }
      TAGGER.dhtml.updateTagNumber(_ntags);

      // change pano and tagsets objects, reset newTags
      _newTags = 0;
      _pano = pano;
      _tagtable = newtagtable;
   }
   tags.changePano = changePano;

   tags.initialize = function (feats) {
      _features.initialize(feats);
      _defaultFeature = _features.list[0].id;
   };

   tags.changeDefaultFeature = function (featureID) {
      _defaultFeature = featureID;
   };


   return tags;
}());
