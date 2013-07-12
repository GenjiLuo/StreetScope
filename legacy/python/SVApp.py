#!/usr/bin/python

import Image, ImageOps, ImageDraw
import json
import math
import os
import time
import StringIO

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *

import SVGraphics
import SVMap
import SVAppState
import Orientation
import SVMouse
import SVCloud

from common import *
from openanything import fetch
from third_party.adq_street_view import GetPanoramaTile


# Some api in the chain is translating the keystrokes to this octal string
# so instead of saying: ESCAPE = 27, we use the following.
ESCAPE = '\033'


# ==================== Class SVApp ==================== 
# Main application object
class SVApp:
    def __init__(self):
        # Basic components, options and state
        self.svmap = SVMap.SVMap()
        self.state = SVAppState.AppState()  # global application state
        self.pano = None            # current panorama object
        self.tagind = 0             # index of tag used when reviewing
        self.imagedir = ""          # directory where static images (prompts and such) live

        # Orientation variables
        self.orientation = Orientation.Orientation()
        self.orientation.setFov(60., 60.)
        self.mouse = SVMouse.SVMouse()
        self.delta_deg   = 5   # Number of degrees we move from each key press
        self.zoom = 2          # zoom level of temp images pulled from google maps

        # Initialize graphics
        self.graphics = SVGraphics.SVGraphics()
        self.graphics.initializeMap(self.svmap.mapwidth, self.svmap.mapheight, self.mapMouseFunc)
        self.graphics.initializeMainWindow(self.display, self.mouseFunc, self.motionFunc, self.keyPressed)
        self.graphics.initializeView(self.orientation.fov_theta, self.orientation.fov_phi)

    # Sets the image directory
    def set_image_dir(self, imagedir):
        self.imagedir = imagedir

    # Changes the current panorama
    def set_pano(self, pano):
        print ""
        self.pano = pano
        self.orientation.setPano(pano)
        if pano.indb:
            self.graphics.set_pano_texture( SVCloud.get_db_pano(pano.dbid) )
        else:
            self.graphics.set_pano_texture( SVCloud.get_sv_pano(pano.panoid, self.zoom) )
        self.svmap.render(pano.location.lat, pano.location.lon)

        # print edges
        print 'Edges:'
        for i, edge in enumerate(pano.edges):
            print i+1, edge.angle, edge.panoid

    def get_prompt(self):
        img = Image.open(os.path.join(self.imagedir, "save_prompt.jpg"))
        return img

    def get_review(self):
        img = Image.open(os.path.join(self.imagedir, "review.jpg"))
        return img

    def viewDirection(self):
        theta = self.orientation.theta
        phi   = self.orientation.phi
        if self.state.isDragging():
            delta = self.mouse.delta()
            theta -= delta.theta
            phi   -= delta.phi
            if phi < -90.0: phi = -90.0
            if phi >  90.0: phi =  90.0
        return SDirection(theta, phi)

    def traverseEdge(self, n):
        pano = SVCloud.get_db_metadata(self.pano.location, self.pano.edges[n].panoid)
        if not pano:
            print "Could not find the panorama in the database."
            pano = SVCloud.get_sv_metadata(self.pano.edges[n].panoid)
        if not pano:
            print "Could not find the panorama in Street View."
            return
        self.set_pano(pano)

    def jumpNear(self, location):
        # Get a list of nearby panoramas
        panos = SVCloud.get_nearby_db_metadata(location)
        if len(panos) == 0:
            print "No nearby panoramas are in the database."
            sv_pano = SVCloud.get_nearby_sv_metadata(location)
            if sv_pano:
               panos.append(sv_pano)
            else:
               print "No nearby panoramas are in Street View."
               return

        # Find the closest one, using the taxicab metric for now...
        closest_pano = panos[0]
        min_d = abs(location.lat - closest_pano.location.lat) + abs(location.lon - closest_pano.location.lon)
        for pano in panos:
            d = abs(location.lat - pano.location.lat) + abs(location.lon - pano.location.lon)
            if d < min_d:
                closest_pano = pano
                min_d = d
        self.set_pano(closest_pano)

    def downloadPano(self):
        newpano = SVCloud.download_pano(self.pano.panoid)
        if not newpano: return False
        self.pano = newpano
        self.set_pano(newpano)
        return True

    def moveToTag(self):
        tagdir = self.pano.tags[self.tagind].center()
        self.orientation.theta = tagdir.theta
        self.orientation.phi   = tagdir.phi

    # Note: it may be wiser to have the server send all the remaining tags again,
    # to ensure our tag indeces stay synced
    def deleteTag(self):
        result = SVCloud.remove_tag(self.pano.dbid, self.tagind) 
        if not result:
            print "Error: tag was not removed."
            return
        print "Tag removed."
        del self.pano.tags[self.tagind]
        if len(self.pano.tags) == 0:
            self.state.setNormal()
        else:
            if self.tagind == len(self.pano.tags):
                self.tagind = 0
            self.moveToTag()

    # Redraws the Sphere Viewer and map windows.
    def display(self):
        try:
            # Get info
            lat = self.pano.location.lat
            lon = self.pano.location.lon
            viewDir = self.viewDirection()

            # draw map
            self.svmap.render_overlay(lat, lon, viewDir.theta)
            self.graphics.drawMap(self.svmap.mapwidth, self.svmap.mapheight, self.svmap.mapimgstr)

            # draw main display
            glutSetWindow(self.graphics.mainwin)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            glPushMatrix()

            # set camera angle
            glRotatef( viewDir.phi,   0, 1, 0)
            glRotatef(-viewDir.theta, 0, 0, 1)
            # Now x and y are in the ground plane, with x pointing east.
            # (So z is normal to the ground, not antiparallel to the pull of gravity.)
#            SVGraphics.drawCrosshair(self.orientation.mtheta, self.orientation.mphi)

            # draw tag selection box
            if self.state.drawTagBox():
                box = self.mouse.box()
                SVGraphics.drawBox(box[0], box[1], box[2], box[3]);

            # draw confirming prompt
            if self.state.isConfirming():
                self.graphics.drawPrompt(viewDir.theta,
                                         viewDir.phi,
                                         28.*self.orientation.spos,
                                         28.*0.30*self.orientation.sright.y,
                                         28.*0.05*self.orientation.sdown.z)

            # draw reviewing prompt
            if self.state.isReviewing():
                self.graphics.drawReview(viewDir.theta,
                                         viewDir.phi,
                                         28.*self.orientation.spos,
                                         28.*0.30*self.orientation.sright.y,
                                         28.*0.15*self.orientation.sdown.z)

            # draw tags
            for tag in self.pano.tags:
                SVGraphics.drawBox(tag.theta1, tag.phi1, tag.theta2, tag.phi2)

            # Now let's set the correct tilt.
            glRotatef( self.orientation.tiltedYaw(), 0, 0, 1) # now x points along the tilt axis
            #SVGraphics.drawAxes() # ground axes
            glRotatef( self.orientation.tilt_pitch,  0, 1, 0) # now the tilt has been applied
            glRotatef(-self.orientation.tilt_yaw,    0, 0, 1)
            # now z points away from the center of the earth
            #SVGraphics.drawAxes()  # cardinal axes

            # draw markers for links
            # This code might be useful for giving a visual indication of edge locations
            #if self.spheres:
            #    yawdif = self.orientation.tilt_yaw - 90.0
            #    for yaw_deg in self.links_yaw_deg:
            #        # this is the yaw measured from the line where the cardinal and ground planes intersect
            #        relyaw = yaw_deg - yawdif
            #        self.graphics.drawLinkSphere(yaw_deg, self.orientation.tiltedPitch(relyaw))

            # Finally we are in position and may draw the sphere.
            # Our last rotation makes the -y axis point toward the center of the pano.
            # (This is where our texture map applies the center of the panorama.)
            glRotatef(self.orientation.pano_yaw + 90.0, 0, 0, 1)
            glColor3f(1.0, 1.0, 1.0)
            glEnable(GL_TEXTURE_2D)
            gluSphere(self.graphics.quadric,30.0,32,32)

            glPopMatrix()
            glutSwapBuffers()

        except Exception, e:
            print type(e)
            print '-----', e
            exit()


    # ==================== GUI Callbacks ====================

    # on left click, move to the panorama indicated by the marker closest
    # to straight forward (?)
    def mapMouseFunc(self, button, isrelease, x, y):
        if isrelease:
            location = self.svmap.getCoords(x, y)
            self.jumpNear(location)

    # handles mouse clicks in the Sphere Viewer
    def mouseFunc(self, button, isrelease, x, y):
        # Right Click
        #if button == 2 and isrelease:
        #    new_panoid = min(zip([absanglediff(self.theta, q) for q in self.links_yaw_deg], self.link_panoids))[1]
        #    self.set_pano(pano_from_id(new_panoid))
        # Left Button
        if button == GLUT_LEFT_BUTTON:     #equivalently if button == 0
            if isrelease:
                if self.state.isDragging():
                    self.doneDragging()
                elif self.state.isTagging():
                    self.doneTagging()
            else:
                windowSize = self.graphics.getMainWindowSize()
                mouseCoords = self.orientation.screenCoords(float(x) / windowSize[0], float(y) / windowSize[1])
                if self.state.isNormal():
                    self.mouse.down(mouseCoords)
                    # if the shift key is pressed we are starting to define a tag
                    if glutGetModifiers() & GLUT_ACTIVE_SHIFT:
                        self.state.setTagging()
                    else:
                        self.state.startDragging()
                elif self.state.isReviewing():
                    if glutGetModifiers() & GLUT_ACTIVE_SHIFT:
                        print "Press 'r' to exit review mode before drawing new tags."
                    else:
                        self.mouse.down(mouseCoords)
                        self.state.startDragging()

        dz = .9
        # zoom in (or scroll down)
        if button == 3:
            self.orientation.zoomIn()
            self.graphics.setFov(self.orientation.fov_theta, self.orientation.fov_phi)

        # zoom out (or scroll up)
        elif button == 4:
            self.orientation.zoomOut()
            self.graphics.setFov(self.orientation.fov_theta, self.orientation.fov_phi)

    # when the sphere is being dragged, this function updates the global variables dtheta and dphi 
    def motionFunc(self, x, y):
        if self.state.trackMouse():
            windowSize = self.graphics.getMainWindowSize()
            self.mouse.track(self.orientation.screenCoords(float(x) / windowSize[0], float(y) / windowSize[1]))

    # Applies dtheta and dphi to theta and phi.
    # Should be called after the user finishes dragging the sphere.
    def doneDragging(self):
        delta = self.mouse.delta()
        self.orientation.theta -= delta.theta
        self.orientation.phi   -= delta.phi
        if self.orientation.phi < -90.0: self.orientation.phi = -90.0
        if self.orientation.phi >  90.0: self.orientation.phi =  90.0
        self.mouse.clear()
        self.state.stopDragging()

    def doneTagging(self):
        self.state.setConfirming()

    def doneConfirming(self, saveTag):
        if saveTag:
            if self.pano.indb or self.downloadPano():
                newtag = Tag(self.mouse.box())
                result = SVCloud.new_tag(self.pano.dbid, newtag.theta1, newtag.phi1, newtag.theta2, newtag.phi2)
                if result:
                    # Note: we have to add to the front, to imitate a linked list
                    self.pano.tags.insert(0, newtag)
                    print "Tag saved."
                else:
                    print "Error: tag was not saved."
            else:
                print "Cannot save tag until the pano is in the database."
        self.mouse.clear()
        self.state.setNormal()

    # The function called whenever a key is pressed. Note the use of Python tuples to pass in: (key, x, y)  
    def keyPressed(self, *args):
        # If escape is pressed, kill everything.
        if args[0] == ESCAPE:
            sys.exit()

        elif args[0] == 'r':
            if self.state.isReviewing():
                self.state.setNormal()
            elif self.state.isNormal() and len(self.pano.tags) > 0:
                self.tagind = 0
                self.moveToTag()
                self.state.setReviewing()

        elif self.state.isNormal():
            if args[0] >= '1' and args[0] <= str(len(self.pano.edges)):
                self.traverseEdge(int(args[0]) - 1)
            elif args[0] == 'd':
                if self.pano.indb:
                    print "The panorama is already in the database"
                else:
                    self.downloadPano()

        elif self.state.isConfirming():
            if args[0] == 'y':
                self.doneConfirming(True)
            elif args[0] == 'n':
                self.doneConfirming(False)

        elif self.state.isReviewing():
            if args[0] == 'j':
                if self.tagind == len(self.pano.tags) - 1:
                    self.tagind = 0
                else:
                    self.tagind = self.tagind + 1
                self.moveToTag()
            elif args[0] == 'k':
                if self.tagind == 0:
                    self.tagind = len(self.pano.tags) - 1
                else:
                    self.tagind = self.tagind - 1
                self.moveToTag()
            elif args[0] == 'd':
                self.deleteTag()

        elif args[0] == '-':
            self.zoom -= 1
            self.set_pano(self.curr_pano)
        elif args[0] == '=':
            self.zoom += 1
            self.set_pano(self.curr_pano)


