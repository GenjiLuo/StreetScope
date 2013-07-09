// Dynamics.js

var TAGGER = TAGGER || {}
TAGGER.dhtml = (function () {
   var dhtml = {};

   var showBoxes = false;

   // takes a panorama object as created in Cloud.js
   dhtml.changePanoMetadata = function (pano) {
      $("#pano_id").html(pano.id);
      $("#pano_lat").html(pano.loc.lat.toFixed(5));
      $("#pano_lon").html(pano.loc.lon.toFixed(5));
   };

   dhtml.updateTagNumber = function (tagnumber) {
      if (tagnumber === 1) {
         $('#tagtitle').text(tagnumber + ' Tag');
      } else {
         $('#tagtitle').text(tagnumber + ' Tags');
      }
   };

   // expects params to have the following keys:
   // id: tag id
   // featureID: feature id
   // featureName: feature name
   // box: {theta1, phi1, theta2, phi2}
   dhtml.addTag = function (params) {
      // set up container
      var tag = document.createElement('div');
      tag.id = params.id;
      tag.className = 'tag';

      // create keys
      var id_k = document.createElement('div');
      id_k.className = 'tagkey';
      id_k.innerHTML = 'id';
      var type_k = document.createElement('div');
      type_k.className = 'tagkey';
      type_k.innerHTML = 'feature';

      if (showBoxes) {
         var t1_k = document.createElement('div');
         var p1_k = document.createElement('div');
         var t2_k = document.createElement('div');
         var p2_k = document.createElement('div');
         t1_k.className = 'tagkey';
         p1_k.className = 'tagkey';
         t2_k.className = 'tagkey';
         p2_k.className = 'tagkey';
         t1_k.innerHTML = '&theta;<sub>1</sub>';
         p1_k.innerHTML = '&phi;<sub>1</sub>';
         t2_k.innerHTML = '&theta;<sub>2</sub>';
         p2_k.innerHTML = '&phi;<sub>2</sub>';
      }

      // create values
      var id_v = document.createElement('div');
      id_v.className = 'tagvalue tagid';
      id_v.innerHTML = params.id;
      var type_v = $("#feature_selector").clone();
      type_v.val(params.featureID);
      type_v.attr("id", "fs_" + params.id);
      type_v.className = 'tagvalue';
      type_v.change( function() { TAGGER.tags.changeTagFeature(params.id, $("#fs_" + params.id).val()); } );

      if (showBoxes) {
         var t1_v = document.createElement('div');
         var p1_v = document.createElement('div');
         var t2_v = document.createElement('div');
         var p2_v = document.createElement('div');
         t1_v.className = 'tagvalue';
         p1_v.className = 'tagvalue';
         t2_v.className = 'tagvalue';
         p2_v.className = 'tagvalue';
         t1_v.innerHTML = params.box.theta1.toFixed(3);
         p1_v.innerHTML = params.box.phi1.toFixed(3);
         t2_v.innerHTML = params.box.theta2.toFixed(3);
         p2_v.innerHTML = params.box.phi2.toFixed(3);
      }

      // create controls
      var tagcontrols = document.createElement('div');
      var delete_btn = document.createElement('span');
      var locate_btn = document.createElement('span');
      tagcontrols.appendChild(delete_btn);
      tagcontrols.appendChild(locate_btn);
      tagcontrols.className = 'tagcontrols';
      delete_btn.className = 'delete btn';
      locate_btn.className = 'locate btn';
      delete_btn.innerHTML = 'delete';
      locate_btn.innerHTML = 'locate';
      $(delete_btn).on('click', params.delete_fun);
      $(locate_btn).on('click', params.locate_fun);
      $(delete_btn).on('click', function () { TAGGER.tags.deleteTag(params.id); });
      $(locate_btn).on('click', function () { TAGGER.tags.locateTag(params.id); });
      
      // create separators
      var separator = [];
      for (i=0; i<7; ++i) {
         separator[i] = document.createElement('div');
         separator[i].className = 'tagclear';
      }

      // glue
      tag.appendChild(id_k);
      tag.appendChild(id_v);
      tag.appendChild(separator[0]);
      tag.appendChild(type_k);
      type_v.appendTo(tag);
      tag.appendChild(separator[1]);

      if (showBoxes) {
         tag.appendChild(t1_k);
         tag.appendChild(t1_v);
         tag.appendChild(separator[2]);
         tag.appendChild(p1_k);
         tag.appendChild(p1_v);
         tag.appendChild(separator[3]);
         tag.appendChild(t2_k);
         tag.appendChild(t2_v);
         tag.appendChild(separator[4]);
         tag.appendChild(p2_k);
         tag.appendChild(p2_v);
         tag.appendChild(separator[5]);
      }

      tag.appendChild(tagcontrols);
      tag.appendChild(separator[6]);

      // plug it in
      $(tag).hide();
      $('#tagdiv').prepend(tag);
      $(tag).show(500);
   };

   dhtml.removeTag = function (tagid) {
      $('#' + tagid).hide(500, function () {
         $('#' + tagid).remove();
      });
   };

   dhtml.updateTagID = function (oldid, newid) {
      // get important nodes
      var tag = $('#' + oldid);
      var $id_v = tag.find(".tagid");
      var $delete_btn = tag.find(".delete");
      var $locate_btn = tag.find(".locate");

      // update the controls
      $delete_btn.off('click');
      $locate_btn.off('click');
      $delete_btn.on('click', function () { TAGGER.tags.deleteTag(newid); });
      $locate_btn.on('click', function () { TAGGER.tags.locateTag(newid); });

      // update the id
      tag.attr('id', newid);
      $id_v.hide(500);
      $id_v.text(newid);
      $id_v.show(500);
   };


   dhtml.addEdge = function (edge, clickHandler) {
      // create container
      var edgediv = document.createElement('div');
      edgediv.className = 'edge';

      // create key and value
      var angle_k = document.createElement('div');
      angle_k.className = 'tagkey';
      angle_k.innerHTML = 'angle';
      var angle_vd = document.createElement('div');
      angle_vd.className = 'tagkey';
      var angle_vs = document.createElement('span');
      angle_vs.className = 'btn';
      angle_vs.innerHTML = (edge.theta * TAGGER.common.radToDeg).toFixed(1);

      // add event handlers
      $(angle_vs).on('click', clickHandler);

      // create separators
      var separator = document.createElement('div');
      separator.className = 'tagclear';

      // assemble
      angle_vd.appendChild(angle_vs);
      edgediv.appendChild(angle_k);
      edgediv.appendChild(angle_vd);
      edgediv.appendChild(separator);
      $('#edgediv').append(edgediv);
   };

   dhtml.removeEdges = function () {
      $('#edgediv').find('.edge').each( function (i) {
         $(this).remove();
      });
   };

   dhtml.addFeature = function (feature) {
      var htmlstring = "<option value='" + feature.id + "'>" + feature.name + "</option>";
      $(htmlstring).appendTo("#feature_selector");
   };

   return dhtml;
}());
