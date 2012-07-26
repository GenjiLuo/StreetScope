#!/usr/bin/python

import sys
import SVApp
import SVCloud
from common import *

# starting coordinates

# Lombard St. (SF)
#lat, lng = 37.8015, -122.4242

# set to false to redownload map data near the starting point
pre_cached_map = True

# Manhattan
lat, lng = 40.7178, -74.0144

# Mt. Auburn
lat, lng = 42.3709, -71.1157


# ==================== Main ====================

def main():
    # Initialize main application object
    svapp = SVApp.SVApp()
    svapp.set_image_dir("/home/erik/Code/streetview/python/images/")
    svapp.graphics.loadImages(svapp.get_prompt(), svapp.get_review())
    svapp.svmap.set_map_dir("/home/erik/Code/streetview/python/map_data/")

    # Get starting location
    if len(sys.argv) < 3:
        print "You need to specify a starting location (lat/lon)."
        exit()
    lat = float(sys.argv[1])
    lon = float(sys.argv[2])

    # load map
    if not pre_cached_map:
        svapp.svmap.cache_map(lat - 0.08, lon - 0.1, lat + 0.08, lon + 0.1)
    else:
        svapp.svmap.load_cached_map()

    # go to the starting location
    svapp.jumpNear(Location(lat, lon))


    #pano = SVCloud.get_db_metadata(Location(lat, lon), panoid)
#    if not pano:
#        print "Could not find the specified initial panorama in the database."
#        pano = SVCloud.get_nearby_sv_metadata(panoid)
#    if not pano:
#        print "Could not find the specified initial panorama in Street View."
#        print "Aborting."
#        exit()

    # Start the graphics (GLUT) loop
    print "Press Escape to exit."
    svapp.graphics.startLoop()
    return


if __name__ == '__main__': 
    main()

