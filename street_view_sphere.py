#!/usr/bin/python
# NOTE: Code heavily inspired by NeHe sample code included with PyOpenGL

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *

import sys
import Image, ImageOps, ImageDraw
import json
from mapnik import Osm, Map, Coord, Envelope, load_map, render_to_file, Projection
import math
import numpy as np
import os
import time
import urllib2
import StringIO

import get_panorama
from common import *
from openanything import fetch
from third_party.adq_street_view import GetPanoramaTile

# starting coordinates

# Lombard St. (SF)
#lat, lng = 37.8015, -122.4242

# Manhattan
lat, lng = 40.7178, -74.0144

# Mt. Auburn
lat, lng = 42.3709,-71.1157

# Some api in the chain is translating the keystrokes to this octal string
# so instead of saying: ESCAPE = 27, we use the following.
ESCAPE = '\033'

spheres = False

delta_deg = 5
theta = phi = 0
dtheta = dphi = 0

pano_yaw_deg = tilt_yaw_deg = tilt_pitch_deg = 0

highest_slope_vec = vec = None

links_yaw_deg = []
link_panoids = []

fovy = fovx = 40.

zoom = 2

mx = my = -1

curr_pano = None

mapheight = 400
mapwidth = 500
largemapimg = None
mapimgx = mapimgy = None

# degrees of latitude covered by half the height of the map window
dlat = .007
map_output = 'map.png'

mapn = Map(mapwidth, mapheight)
load_map(mapn, 'osm/mapnik_style.xml')

# https://lists.berlios.de/pipermail/mapnik-users/2010-September/003578.html
projection = Projection("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 "
                        "+lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m "
                        "+nadgrids=@null +no_defs +over")
mapn.srs = projection.params()

routedir = None
routeids = None
routepanos = None
routeind = None
imgstr = None
oldrouteyaw = None

def angle_avg(t1, t2):
    x = (cos(t1) + cos(t2)) / 2
    y = (sin(t1) + sin(t2)) / 2
    return atan2(y, x)

def pano_yaw(pano1, pano2):
    lat1, lng1 = map(float, pan_lat_lng(pano1))
    lat2, lng2 = map(float, pan_lat_lng(pano2))
    dy = lat2 - lat1
    dx = (lng2 - lng1) * cos(lat1)
    return atan2(dx, dy)

def route_yaw(ind):
    if ind == 0:
        return pano_yaw(routepanos[0], routepanos[1])
    if ind == len(routepanos) - 1:
        return pano_yaw(routepanos[-2], routepanos[-1])

    t1 = pano_yaw(routepanos[ind], routepanos[ind+1])
    t2 = pano_yaw(routepanos[ind-1], routepanos[ind])
    return angle_avg(t1, t2)

# argument is provided, indicating a directory with route info
if len(sys.argv) > 1:
    routedir = sys.argv[1]
    js = json.load(open(os.path.join(routedir, 'route.txt')))
    routeids = js['panoids']
    routepanos = [json.load(open(os.path.join(routedir, l + '.json'))) for l in routeids]
    routeind = 0

    #print json.dumps(routepanos[0], indent=4); exit()

    for layer in mapn.layers:
        layer.datasource = Osm(file=os.path.join(routedir, 'dat.osm'))

    bounds = js['bounds']
    west, south, east, north = bounds['west'], bounds['south'], bounds['east'], bounds['north']
    mapn.resize(int(mapwidth * (east - west) / dlat * sin((north + south)/2)),
                int(mapheight * (north - south) / dlat))
    mapn.zoom_to_box(projection.forward(Envelope(west, south, east, north)))
    render_to_file(mapn, map_output)
    largemapimg = Image.open(map_output)

def render_map(lat, lng):
    dlng = dlat * mapwidth / mapheight / sin(lat)

    global mapimg, mapimgstr
    coord = mapn.view_transform().forward(projection.forward(Coord(lng, lat)))
    if routedir:
        global mapimgx, mapimgy
        mapimgx = int(coord.x - mapwidth / 2)
        mapimgy = int(coord.y - mapheight / 2)
        mapimg = largemapimg.crop((mapimgx, mapimgy, mapimgx + mapwidth, mapimgy + mapheight))

        coord -= Coord(mapimgx, mapimgy)
    else:
        conn = urllib2.urlopen('http://overpass-api.de/api/xapi?map?bbox=%f,%f,%f,%f' %
                               (lng-dlng*1.3, lat-dlat*1.3, lng+dlng*1.3, lat+dlat*1.3))
        with open('osm/dat.osm', 'w') as f:
            s = conn.read(16384)
            while s:
                f.write(s)
                s = conn.read(16384)
            f.close()
        conn.close()

        for layer in mapn.layers:
            layer.datasource = Osm(file='osm/dat.osm')

        mapn.zoom_to_box(projection.forward(Envelope(lng-dlng, lat-dlat, lng+dlng, lat+dlat)))
        render_to_file(mapn, map_output)
        mapimg = Image.open(map_output)

def draw_map_overlay():
    lat, lng = map(float, pan_lat_lng(curr_pano))
    coord = mapn.view_transform().forward(projection.forward(Coord(lng, lat)))
    coord -= Coord(mapimgx, mapimgy)
    img = mapimg.copy()
    draw = ImageDraw.Draw(img)
    draw.ellipse((coord.x - 4, coord.y - 4,
                  coord.x + 4, coord.y + 4), outline='red', fill='red')

    draw.line((coord.x, coord.y,
               coord.x + 12*sin(theta), coord.y - 12*cos(theta)), width=3, fill='red')

    global mapimgstr
    mapimgstr = img.tostring('raw', 'RGBA', 0, -1)

def get_panorama_image(panoid, zoom=2):
    global imgstr
    if routeids and panoid in routeids:
        img = Image.open(os.path.join(routedir, panoid + '.jpg'))
        imgstr = None#open(os.path.join(routedir, panoid + '.bin')).read()
    else:
        img = ImageOps.mirror(get_panorama.get_panorama_image(panoid, zoom))
        imgstr = None
    return img

def set_texture(im):
    ix, iy = im.size[0], im.size[1]
    im_str = imgstr or im.tostring("raw", "RGBX", 0, -1)
    print "Panorama size:", ix, iy
    glutSetWindow(mainwin)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ix, iy, 0, GL_RGBA, GL_UNSIGNED_BYTE, im_str)

def enhance():
    print 'Enhancing!'
    ix, iy = 8192, 4096
    im_str = open(os.path.join(routedir, 'bin', id_from_pano(curr_pano) + '.bin')).read()
    glutSetWindow(mainwin)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ix, iy, 0, GL_RGBA, GL_UNSIGNED_BYTE, im_str)

def set_pano(pano):
    global curr_pano, pano_yaw_deg, tilt_yaw_deg, tilt_pitch_deg
    curr_pano = pano

    print id_from_pano(pano), 'links:            ', '     '.join(p['panoId'] for p in pano['Links'])

    if oldrouteyaw is not None and routepanos and pano in routepanos:
        newrouteyaw = route_yaw(routeind)
        global theta
        theta += newrouteyaw - oldrouteyaw

    pano_yaw_deg = float(pano['Projection']['pano_yaw_deg'])
    tilt_yaw_deg = float(pano['Projection']['tilt_yaw_deg'])
    tilt_pitch_deg = float(pano['Projection']['tilt_pitch_deg'])

    global highest_slope_vec, vec
    highest_slope_vec = (sin(tilt_yaw_deg) * cos(tilt_pitch_deg),
                         cos(tilt_yaw_deg) * cos(tilt_pitch_deg),
                         sin(tilt_pitch_deg))
    vec = np.cross((0, 0, 1), highest_slope_vec)

    global links_yaw_deg, link_panoids
    links_yaw_deg = [float(link['yawDeg']) for link in pano['Links']]
    link_panoids = [link['panoId'] for link in pano['Links']]

    set_texture(get_panorama_image(id_from_pano(pano), zoom=zoom))

    lat, lng = pan_lat_lng(pano)
    render_map(float(lat), float(lng))

def main():
    global quadric
    global mainwin, mapwin

    glutInit(sys.argv)

    # create map window
    glutInitWindowSize(mapwidth, mapheight)
    # the first glutCreate must be before the first glutInitDisplayMode
    # to prevent a segfault on some systems
    mapwin = glutCreateWindow('Map')
    glutMouseFunc(mapMouseFunc)

    # create main window
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
    glutReshapeWindow(mapwidth, mapheight)

    glutInitWindowSize(1000,1000)
    mainwin = glutCreateWindow('Sphere Viewer')
    #mapwin = glutCreateSubWindow(mainwin, 0, 0, mapwidth, mapheight)
    #glutSetWindow(mainwin)

    glEnable(GL_TEXTURE_2D);
    tid = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, tid)
    glPixelStorei(GL_UNPACK_ALIGNMENT,1)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)

    if routedir:
        pano = routepanos[0]
    else:
        pano = nearby_panorama(lat, lng)
        print id_from_pano(pano)
    set_pano(pano)

    glClearColor(0.,0.,0.,1.)
    glShadeModel(GL_SMOOTH)
    #glEnable(GL_CULL_FACE)
    glEnable(GL_DEPTH_TEST)
    glEnable(GL_LIGHTING)

    quadric=gluNewQuadric(); # // Create A Pointer To The Quadric Object
    gluQuadricOrientation(quadric, GLU_OUTSIDE)

    gluQuadricNormals(quadric, GLU_SMOOTH); # // Create Smooth Normals
    gluQuadricTexture(quadric, GL_TRUE); # // Create Texture Coords
    #gluQuadricOrientation(quadric, GLU_INSIDE)

    lightZeroColor = [1.0,1.0,1.0,1.0] 

    #lightZeroPosition = [0.,0.,0.,1.]
    #glLightfv(GL_LIGHT0, GL_POSITION, lightZeroPosition)
    #glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor)
    #glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1)
    #glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05)
    #glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 1)
    #glEnable(GL_LIGHT0)

    #glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE)
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightZeroColor)
    glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,lightZeroColor)
    glMaterialfv(GL_BACK,GL_AMBIENT_AND_DIFFUSE,lightZeroColor)

    glutDisplayFunc(display)
    glutIdleFunc(display)

    glMatrixMode(GL_PROJECTION)
    gluPerspective(fovy, fovx / fovy, 1., 40.)
    glMatrixMode(GL_MODELVIEW)
    gluLookAt(0,0,0,
              0,1,0,
              0,0,1)
    glPushMatrix()

    
    glutMouseFunc(mouseFunc)
    glutMotionFunc(motionFunc)
    glutKeyboardFunc(keyPressed)

    print "Press Escape to exit."
    glutMainLoop()
    return

def mapMouseFunc(button, isrelease, x, y):
    if isrelease:
        coord = projection.inverse(mapn.view_transform().backward(Coord(x + mapimgx, y + mapimgy)))
        set_pano(nearby_panorama(coord.y, coord.x))

def absanglediff(s, t):
    a = abs(s - t) % 360
    if a > 180:
        return 360-a
    return a

# on left click, move to the panorama indicated by the marker closest
# to straight forward
def mouseFunc(button, isrelease, x, y):
    global mx, my, phi, theta, dphi, dtheta
    if button == 2 and isrelease:
        new_panoid = min(zip([absanglediff(theta, q) for q in links_yaw_deg], link_panoids))[1]
        set_pano(pano_from_id(new_panoid))
    if button == 0:
        if isrelease:
            mx = my = -1
            phi += dphi
            theta += dtheta
            dphi = dtheta = 0
        else:
            mx = x
            my = y

    global fovx, fovy
    dz = .9
    if button == 3:
        fovx *= dz
        fovy *= dz
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(fovy, fovx / fovy, 1., 40.)
        glMatrixMode(GL_MODELVIEW)
    if button == 4:
        fovx /= dz
        fovy /= dz
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(fovy, fovx / fovy, 1., 40.)
        glMatrixMode(GL_MODELVIEW)

def motionFunc(x, y):
    if mx < 0 or my < 0: return

    w = glutGet(GLUT_WINDOW_WIDTH)
    h = glutGet(GLUT_WINDOW_HEIGHT)

    x0 = 2 * float(mx) / w - 1
    y0 = 2 * float(my) / h - 1

    x1 = 2 * float(x) / w - 1
    y1 = 2 * float(y) / h - 1

    global dphi, dtheta
    # dtheta = atan(x0 * tan(fovx/2)) - atan(x1 * tan(fovx/2))
    dphi = atan(y1 * tan(fovy/2)) - atan(y0 * tan(fovy/2))

    center_vec = np.array((sin(theta) * cos(phi),
                           cos(theta) * cos(phi),
                           sin(phi)))

    xvec = np.array((cos(theta),
                     -sin(theta),
                     0)) * tan(fovx/2)

    yvec = np.array((-sin(phi) * sin(theta),
                     -sin(phi) * cos(theta),
                      cos(phi))) * tan(fovy/2)

    p0 = center_vec + x0 * xvec + y0 * yvec
    p1 = center_vec + x1 * xvec + y1 * yvec

    dtheta = atan2(p1[1], p1[0]) - atan2(p0[1], p0[0])

# The function called whenever a key is pressed. Note the use of Python tuples to pass in: (key, x, y)  
def keyPressed(*args):
    # If escape is pressed, kill everything.
    if args[0] == ESCAPE:
        sys.exit()

    global phi, theta
    if args[0] == '1':
        phi -= delta_deg
    if args[0] == '2':
        phi += delta_deg
    if args[0] == '5':
        theta -= delta_deg
    if args[0] == '6':
        theta += delta_deg

    if args[0] == 'e':
        enhance()

    if args[0] == 's':
        global spheres
        spheres = not spheres

    if routedir:
        global routeind, oldrouteyaw
        oldrouteyaw = route_yaw(routeind)
        if args[0] == 'j':
            routeind = min(routeind+1, len(routepanos) - 1)
            set_pano(routepanos[routeind])
        if args[0] == 'k':
            routeind = max(routeind-1, 0)
            set_pano(routepanos[routeind])
        if args[0] == ' ':
            print theta, oldrouteyaw
            theta = oldrouteyaw

    global zoom
    if args[0] == '-':
        zoom -= 1
        set_pano(curr_pano)
    if args[0] == '=':
        zoom += 1
        set_pano(curr_pano)

def display():
    try:
        # draw map
        glutSetWindow(mapwin)
        draw_map_overlay()
        glDrawPixels(mapwidth, mapheight, GL_RGBA, GL_UNSIGNED_BYTE, mapimgstr)
        glutSwapBuffers()

        # draw main display
        glutSetWindow(mainwin)
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)
        glPushMatrix()

        # inverse camera position
        glRotatef(-phi - dphi, 1, 0, 0)
        glRotatef(-theta - dtheta, 0, 0, -1)

        global marker_ind
        # draw markers for links
        if spheres:
            for yaw_deg in links_yaw_deg:
                glPushMatrix()
                glRotatef(-yaw_deg, 0, 0, 1)
                glTranslatef(0, 15, 0)

                marker_quadric = gluNewQuadric()
                gluSphere(quadric, 1., 20, 20)

                glPopMatrix()

        glRotatef(-tilt_pitch_deg, *vec)

        # sphere position
        glRotatef(180-pano_yaw_deg, 0, 0, 1)

        gluSphere(quadric,30.0,32,32);
        glPopMatrix()
        glutSwapBuffers()

    except Exception, e:
        print '-----', e
        exit()

if __name__ == '__main__': 
    main()
