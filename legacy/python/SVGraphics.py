#!/usr/bin/python

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
from common import *
import Image

class SVGraphics:
    def __init__(self):
        glutInit(sys.argv)

    def initializeMap(self, mapwidth, mapheight, mapMouseFunc):
        # create map window
        glutInitWindowSize(mapwidth, mapheight)
        self.mapwin = glutCreateWindow('Map')
        glutMouseFunc(mapMouseFunc)

    def loadImages(self, prompt, review):
        self.prompt = prompt
        self.review = review

    def initializeMainWindow(self, displayFunc, mouseFunc, motionFunc, keyFunc):
        # create main window
        # the first glutCreate must be before the first glutInitDisplayMode
        # to prevent a segfault on some systems
        glutInitWindowSize(1000, 1000)
        self.mainwin = glutCreateWindow('Sphere Viewer')
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)

        # Create a texture (image will be loaded later)
        glEnable(GL_TEXTURE_2D);
        self.tid = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.tid)
        glPixelStorei(GL_UNPACK_ALIGNMENT,1)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)

        # Initialize color and shading settings
        glClearColor(0.,0.,0.,1.)
        glShadeModel(GL_SMOOTH)
        #glEnable(GL_CULL_FACE)
        glEnable(GL_DEPTH_TEST)
        # We don't want lighting because it changes the colors of our tags
        #glEnable(GL_LIGHTING)

        # Make a Quadric object (will be our sphere)
        self.quadric = gluNewQuadric(); # // Create A Pointer To The Quadric Object
        gluQuadricOrientation(self.quadric, GLU_INSIDE)  # this line doesn't seem to matter
        gluQuadricNormals(self.quadric, GLU_SMOOTH); # // Create Smooth Normals
        gluQuadricTexture(self.quadric, GL_TRUE); # // Create Texture Coords

        # Bind callbacks
        glutDisplayFunc(displayFunc)
        glutIdleFunc(displayFunc)
        glutMouseFunc(mouseFunc)
        glutMotionFunc(motionFunc)
        glutKeyboardFunc(keyFunc)

    def setFov(self, fov_theta, fov_phi):
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(fov_phi, fov_theta / fov_phi, 1., 40.)
        glMatrixMode(GL_MODELVIEW)

    def initializeView(self, fovx, fovy):
        # We want the camera to start at the origin,
        # looking along the x axis, with the z axis facing up.
        self.setFov(fovx, fovy)
        gluLookAt(0,0,0,
                  1,0,0,
                  0,0,1)

    def set_pano_texture(self, im):
        print "Setting panorama image."
        ix, iy = im.size[0], im.size[1]
        pano_str = im.tostring("raw", "RGBX", 0, -1)
        print "Panorama size:", ix, iy
        self.set_pano_texture_binary(pano_str, ix, iy)

    def set_pano_texture_binary(self, im_str, ix, iy):
        self.ix = ix
        self.iy = iy
        self.pano_str = im_str
        glutSetWindow(self.mainwin)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ix, iy, 0, GL_RGBA, GL_UNSIGNED_BYTE, im_str)

    def set_texture(self, im):
        ix, iy = im.size[0], im.size[1]
        im_str = im.tostring("raw", "RGBX", 0, -1)
        glutSetWindow(self.mainwin)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ix, iy, 0, GL_RGBA, GL_UNSIGNED_BYTE, im_str)

    def recoverPano(self):
        self.set_pano_texture_binary(self.pano_str, self.ix, self.iy)

    def startLoop(self):
        glutMainLoop()
       
    def drawMap(self, mapwidth, mapheight, mapimgstr):
        glutSetWindow(self.mapwin)
        glDrawPixels(mapwidth, mapheight, GL_RGBA, GL_UNSIGNED_BYTE, mapimgstr)
        glutSwapBuffers()

    def prepareMainDisplay(self, xangle, zangle):
        # switch to the main window and save our current viewpoint
        glutSetWindow(self.mainwin)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glPushMatrix()
        drawAxes()

        # Set camera position
        # glRotatef(phi, x, y, z) rotates phi degrees about the vector (x, y, z)
        # where (x, y, z) is expressed in the local coordinates of the camera
        # Recall our camera starts looking along +y with +z pointing up.
        glRotatef(xangle, 1, 0, 0)
        glRotatef(zangle, 0, 0, 1)
        drawAxes()

    def getMainWindowSize(self):
        w = glutGet(GLUT_WINDOW_WIDTH)
        h = glutGet(GLUT_WINDOW_HEIGHT)
        return w, h

    def drawLinkSphere(self, theta, phi):
        glColor3f(1, 1, 1)
        glEnable(GL_TEXTURE_2D)
        glPushMatrix()
        glRotatef(theta, 0, 0, 1)
        glRotatef(-phi,   0, 1, 0)
        glTranslatef(15, 0, 0)
        gluSphere(self.quadric, 1., 20, 20)
        glPopMatrix()

    def drawPrompt(self, theta, phi, screenCoords, right, down):
        glPushMatrix()
        glRotatef(theta, 0, 0, 1)
        glRotatef(-phi, 0, 1, 0)
        glTranslatef(screenCoords.x, screenCoords.y, screenCoords.z)
        glColor3f(1, 1, 1)
        glEnable(GL_TEXTURE_2D)
        self.set_texture(self.prompt)
        glBegin(GL_QUADS)
        glTexCoord2f(0, 1); glVertex3f(0,  0,    0)
        glTexCoord2f(1, 1); glVertex3f(0, right, 0)
        glTexCoord2f(1, 0); glVertex3f(0, right, down)
        glTexCoord2f(0, 0); glVertex3f(0,  0,    down) 
        glEnd()
        glPopMatrix()
        self.recoverPano()

    def drawReview(self, theta, phi, screenCoords, right, down):
        glPushMatrix()
        glRotatef(theta, 0, 0, 1)
        glRotatef(-phi, 0, 1, 0)
        glTranslatef(screenCoords.x, screenCoords.y, screenCoords.z)
        glColor3f(1, 1, 1)
        glEnable(GL_TEXTURE_2D)
        self.set_texture(self.review)
        glBegin(GL_QUADS)
        glTexCoord2f(0, 1); glVertex3f(0,  0,    0)
        glTexCoord2f(1, 1); glVertex3f(0, right, 0)
        glTexCoord2f(1, 0); glVertex3f(0, right, down)
        glTexCoord2f(0, 0); glVertex3f(0,  0,    down) 
        glEnd()
        glPopMatrix()
        self.recoverPano()
        


def drawCrosshair(theta, phi):
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0, 0.0, 0.0)            # Set color to black
    glPushMatrix()
    glRotate(theta, 0, 0, 1)
    glRotate(-phi,  0, 1, 0)
    glBegin(GL_LINES)
    glVertex3f(28,  0, -1)
    glVertex3f(28,  0,  1)
    glVertex3f(28, -1,  0)
    glVertex3f(28,  1,  0)
    glEnd()
    glPopMatrix()

def drawBox(theta1, phi1, theta2, phi2):
    # The radius at which the box is drawn.
    # It should be slightly smaller than the radius of the quadrics sphere.
    radius = 29.0
    # these variables determine how many lines we draw per arc
    dtheta = dphi = 5.0
    theta = theta1
    phi = phi1

    # make sure we draw the box the right way
    # (ex a box from 300 to 10 degrees shouldn't be from 10 to 300)

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_LINE_LOOP)               # Start drawing a line loop
    glColor3f(0.0, 1.0, 0.0)            # Set color to green

    # draw vertical arc at theta1
    while phi < phi2: 
        glVertex3f(radius * cos(phi) * cos(theta),
                   radius * cos(phi) * sin(theta),
                   radius * sin(phi))
        phi += dphi
    phi = phi2
    glVertex3f(radius * cos(phi) * cos(theta),
               radius * cos(phi) * sin(theta),
               radius * sin(phi))

    # draw horizontal arc at phi2
    while theta < theta2: 
        glVertex3f(radius * cos(phi) * cos(theta),
                   radius * cos(phi) * sin(theta),
                   radius * sin(phi))
        theta += dtheta
    theta = theta2
    glVertex3f(radius * cos(phi) * cos(theta),
               radius * cos(phi) * sin(theta),
               radius * sin(phi))

    # draw vertical arc at theta2
    while phi > phi1: 
        glVertex3f(radius * cos(phi) * cos(theta),
                   radius * cos(phi) * sin(theta),
                   radius * sin(phi))
        phi -= dphi
    phi = phi1
    glVertex3f(radius * cos(phi) * cos(theta),
               radius * cos(phi) * sin(theta),
               radius * sin(phi))

    # draw horizontal arc at phi1
    while theta > theta1: 
        glVertex3f(radius * cos(phi) * cos(theta),
                   radius * cos(phi) * sin(theta),
                   radius * sin(phi))
        theta -= dtheta
    # Finish drawing the line loop
    glEnd()

def drawAxes():
    d = 2.5 * 0.707107
    glDisable(GL_TEXTURE_2D)
    glBegin(GL_LINE_LOOP)
    glColor3f(0.0, 0.0, 0.0)
    glVertex3f(30.0,  0.0,      0.0)
    glVertex3f(d,     30.0 - d, 0.0)
    glColor3f(1.0, 0.0, 0.0)
    glVertex3f(d,     30.0 - d, 0.0)
    glVertex3f(0.0,   30.0    , 0.0)
    glVertex3f(0.0,   30.0 - d, d  )
    glColor3f(0.0, 0.0, 0.0)
    glVertex3f(0.0,   30.0 - d, d  )
    glVertex3f(0.0,   0.0,      30.0)
    glEnd()


