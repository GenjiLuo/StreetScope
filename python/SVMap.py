#!/usr/bin/python

import os
import Image, ImageOps, ImageDraw
import mapnik2
import urllib2
import pickle
from common import *

class SVMap:
    def __init__(self):
        self.mapdir = ""
        self.map_output = 'map.png' # where the map image is stored
        self.mapheight = 400        # height of the map
        self.mapwidth = 500         # width of the map
        self.dlat = .007            # degrees of latitude covered by half the height of the map window
        self.largemapimg = None     # Large version of the base map image. Used when we have a cachedir.
        self.mapimg = None          # Properly cropped version of the base map image.
        self.mapimgstr = None       # Raw data associated with mapimg + overlay that can be passed to openGL
        self.mapimgx = self.mapimgy = None # size of mapimg
        # https://lists.berlios.de/pipermail/mapnik-users/2010-September/003578.html
        self.projection = mapnik2.Projection("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 "
                                     "+lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m "
                                     "+nadgrids=@null +no_defs +over")

    # include trailing '/'
    def set_map_dir(self, mapdir):
        self.mapdir = mapdir
        self.mapn = mapnik2.Map(self.mapwidth, self.mapheight)
        mapnik2.load_map(self.mapn, self.mapdir + 'mapnik_style.xml')
        self.mapn.srs = self.projection.params()

    def cache_map(self, minlat, minlng, maxlat, maxlng):
        print "Downloading map data."
        conn = urllib2.urlopen('http://overpass-api.de/api/xapi?map?bbox=%f,%f,%f,%f' %
                              (minlng, minlat, maxlng, maxlat))
        fn = self.mapdir + 'dat.osm'
        with open(fn, 'w') as f:
            s = conn.read(65536)
            while s:
                f.write(s)
                s = conn.read(65536)
        conn.close()

        print "Saving bounds."
        bounds_dictionary = { 'minlat': minlat, 'minlng': minlng, 'maxlat': maxlat, 'maxlng': maxlng }
        fn = self.mapdir + 'bounds.pkl'
        with open(fn, 'w') as f:
            pickle.dump(bounds_dictionary, f)

        for layer in self.mapn.layers:
            layer.datasource = mapnik2.Osm(file=os.path.join(self.mapdir, 'dat.osm'))

        self.mapn.resize(int(self.mapwidth * (maxlng - minlng) / self.dlat * sin((maxlat + minlat)/2)),
                         int(self.mapheight * (maxlat - minlat) / self.dlat))
        self.mapn.zoom_to_box(self.projection.forward(mapnik2.Envelope(minlng, minlat, maxlng, maxlat)))

        print "Saving base map image."
        filename = self.mapdir + self.map_output
        mapnik2.render_to_file(self.mapn, filename)
        self.largemapimg = Image.open(filename)

    def load_cached_map(self):
        print "Loading cached map."

        fn = self.mapdir + 'bounds.pkl'
        with open(fn, 'r') as f:
            bounds_dictionary = pickle.load(f)
        minlat = bounds_dictionary['minlat']
        minlng = bounds_dictionary['minlng']
        maxlat = bounds_dictionary['maxlat']
        maxlng = bounds_dictionary['maxlng']

        for layer in self.mapn.layers:
            layer.datasource = mapnik2.Osm(file=os.path.join(self.mapdir, 'dat.osm'))

        self.mapn.resize(int(self.mapwidth * (maxlng - minlng) / self.dlat * sin((maxlat + minlat)/2)),
                         int(self.mapheight * (maxlat - minlat) / self.dlat))
        self.mapn.zoom_to_box(self.projection.forward(mapnik2.Envelope(minlng, minlat, maxlng, maxlat)))

        filename = self.mapdir + self.map_output
        self.largemapimg = Image.open(filename)

    # Renders the base map image, saving it in mapimg. 
    def render(self, lat, lng):
        dlng = self.dlat * self.mapwidth / self.mapheight / sin(lat)
        coord = self.mapn.view_transform().forward( self.projection.forward(mapnik2.Coord(lng, lat)) )

        self.mapimgx = int(coord.x - self.mapwidth / 2)
        self.mapimgy = int(coord.y - self.mapheight / 2)
        self.mapimg = self.largemapimg.crop((self.mapimgx, self.mapimgy, self.mapimgx + self.mapwidth, self.mapimgy + self.mapheight))
        coord -= mapnik2.Coord(self.mapimgx, self.mapimgy)

    # theta is measured counterclockwise from east, in degrees (as always)
    def render_overlay(self, lat, lng, theta):
        coord = self.mapn.view_transform().forward(self.projection.forward(mapnik2.Coord(lng, lat)))
        coord -= mapnik2.Coord(self.mapimgx, self.mapimgy)
        img = self.mapimg.copy()

        # ImageDraw coordinates are measured from the top left corner (x horizontal and y vertical)
        draw = ImageDraw.Draw(img)
        draw.ellipse((coord.x - 4, coord.y - 4,
                      coord.x + 4, coord.y + 4), outline='red', fill='red')
        draw.line((coord.x, coord.y,
                   coord.x + 12*cos(theta), coord.y - 12*sin(theta)), width=3, fill='red')

        self.mapimgstr = img.tostring('raw', 'RGBA', 0, -1)

    def getCoords(self, x, y):
        coords = self.projection.inverse(self.mapn.view_transform().backward(mapnik2.Coord(x + self.mapimgx, y + self.mapimgy)))
        return Location(coords.y, coords.x)

