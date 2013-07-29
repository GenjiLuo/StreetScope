// Graphics.js

var TAGGER = TAGGER || {}
TAGGER.graphics = (function () {
   var graphics = {};

   var _container;
   var camera;

   // set the scene size
   var width = window.innerWidth;
   var height = window.innerHeight;

   // create a WebGL renderer, camera, scene, and projector
   var renderer = new THREE.WebGLRenderer();
   var scene = new THREE.Scene();
   var projector = new THREE.Projector();

   // let there be light
   var ambientLight = new THREE.AmbientLight(0xFFFFFF);
   scene.add(ambientLight);

   // create the sphere
   //var sphereMaterial = new THREE.MeshLambertMaterial({ map: texture });
   var sphereMaterial = new THREE.MeshLambertMaterial( { map: THREE.ImageUtils.loadTexture( 'placeholder.jpg' ) } );
   var radius = 30, segments = 32, rings = 32;
   var sphere = new THREE.Mesh(new THREE.SphereGeometry(radius, segments, rings), sphereMaterial);
   sphere.flipSided = true;
   sphere.useQuaternion = true;
   scene.add(sphere);

   // create the array of tags
   var tags = {};


   // METHODS

   graphics.setFov = function (params) {
      camera.fov = params.phi * 180 / Math.PI;
      var aspect = params.phi / params.theta; // this is an incorrect approximation - not close enough
      camera.aspect = 1 / aspect;
      camera.updateProjectionMatrix();
   }

   graphics.getFov = function () {
      return fov_theta;
   }

   graphics.changeTexture = function (newtexture) {
      sphere.material.map = new THREE.Texture( newtexture ); 
      sphere.material.map.needsUpdate = true;
   }

   graphics.orientQuat = function (quat) {
      sphere.quaternion = quat;
      sphere.updateMatrix();
   };

   graphics.render = function (cView) {
      camera.lookAt(cView);
      renderer.render(scene, camera);
   };

   // Draws a red line indicating the x axis (should be east),
   // a blue line indicating the z axis (should be south),
   // and a green line indicating the y axis (should be up)
   function addAxes () {
      var lineMat1 = new THREE.LineBasicMaterial({ color: 0xdd3333, opacity: 1, linewidth: 3 });
      var lineMat2 = new THREE.LineBasicMaterial({ color: 0x33dd33, opacity: 1, linewidth: 3 });
      var lineMat3 = new THREE.LineBasicMaterial({ color: 0x3333dd, opacity: 1, linewidth: 3 });
      var lineGeo1 = new THREE.Geometry();
      var lineGeo2 = new THREE.Geometry();
      var lineGeo3 = new THREE.Geometry();
      lineGeo1.vertices.push(new THREE.Vector3(25, -3, 0));
      lineGeo1.vertices.push(new THREE.Vector3(25,  3, 0));
      lineGeo2.vertices.push(new THREE.Vector3(0, 25, -3));
      lineGeo2.vertices.push(new THREE.Vector3(0, 25,  3));
      lineGeo3.vertices.push(new THREE.Vector3(-3, 0, 25));
      lineGeo3.vertices.push(new THREE.Vector3( 3, 0, 25));
      var line1 = new THREE.Line(lineGeo1, lineMat1);
      var line2 = new THREE.Line(lineGeo2, lineMat2);
      var line3 = new THREE.Line(lineGeo3, lineMat3);
      scene.add(line1);
      scene.add(line2);
      scene.add(line3);
   }

   function addArc (geo, sCoords, dtheta, dphi, subdiv) {
      for (i=0; i<subdiv; ++i) {
         var theta = sCoords.theta + dtheta * i / subdiv;
         var phi   = sCoords.phi + dphi * i / subdiv;
         var p = TAGGER.orientation.cCoords({'theta': theta, 'phi': phi, 'r': sCoords.r});
         geo.vertices.push(new THREE.Vector3(p.x, p.y, p.z));
      }
      var p = TAGGER.orientation.cCoords({'theta': sCoords.theta + dtheta, 'phi': sCoords.phi + dphi, 'r': sCoords.r});
      geo.vertices.push(new THREE.Vector3(p.x, p.y, p.z));
   }

   graphics.addTag = function (tagbox, id) {
      var lineMat = new THREE.LineBasicMaterial({ color: 0x00dd0a, opacity: 1, linewidth: 3 });
      var lineGeo = new THREE.Geometry();
      var dtheta = tagbox.dtheta();
      var dphi   = tagbox.dphi();
      var corner_11 = {theta: tagbox.theta1, phi: tagbox.phi1, r: 10};
      var corner_21 = {theta: tagbox.theta2, phi: tagbox.phi1, r: 10};
      var corner_22 = {theta: tagbox.theta2, phi: tagbox.phi2, r: 10};
      var corner_12 = {theta: tagbox.theta1, phi: tagbox.phi2, r: 10};
      var subdivs_per_rad = 64 / (2*Math.PI);
      addArc(lineGeo, corner_11,  dtheta,     0, dtheta * subdivs_per_rad);
      addArc(lineGeo, corner_21,       0,  dphi,   dphi * subdivs_per_rad);
      addArc(lineGeo, corner_22, -dtheta,     0, dtheta * subdivs_per_rad);
      addArc(lineGeo, corner_12,       0, -dphi,   dphi * subdivs_per_rad);
      var line = new THREE.Line(lineGeo, lineMat);
      tags[id] = line;
      scene.add(line);
   };

   graphics.removeTag = function (id) {
      if (tags[id]) {
         scene.remove(tags[id]);
         delete tags[id];
      }
   };
   
   graphics.updateTagID = function (oldid, newid) {
      var tag = tags[oldid];
      delete tags[oldid];
      tags[newid] = tag;
   };

   // ToDo: the projection doesn't always find an intersection!
   // I'm not sure why this is and it doesn't make anything crash,
   // but we should hanhdle this case without generating a runtime error.
   graphics.eventCoords = function (ev) {
      var vector = new THREE.Vector3((ev.clientX/width)*2-1, -(ev.clientY/height)*2+1, 0.5);
      projector.unprojectVector(vector, camera);
      var ray = new THREE.Ray(camera.position, vector.subSelf(camera.position).normalize());
      var intersects = ray.intersectObject(sphere);
      if (!intersects[0]) {
         console.log("Error: Could not find intersection with sphere!");
      }
      //console.log(intersects[0].point);
      return intersects[0].point;
   };

   graphics.domElement = function () { return renderer.domElement; };

   function resize (fov_phi) {
      renderer.setSize( _container.clientWidth, _container.clientHeight );
      camera.fov = fov_phi * TAGGER.common.radToDeg;
      camera.aspect = _container.clientWidth / _container.clientHeight;
      camera.updateProjectionMatrix();
   }
   graphics.resize = resize;

   graphics.initialize = function (container, fov_phi) {
      _container = container;
      width = window.innerWidth;
      height = window.innerHeight;

      // make the camera
      near = 0.1,
      far = 1000;
      camera = new THREE.PerspectiveCamera(
         fov_phi,
         width / height,
         near,
         far
      );
      scene.add(camera);

      renderer.setSize(width, height);
      _container.appendChild(renderer.domElement);

      //addAxes();
   };

   return graphics;

}());

