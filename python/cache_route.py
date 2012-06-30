#!/usr/bin/python
# usage: cache_route.py "starting location" "ending location" <outdir>
# route info will be saved to routes/outdir, which can be passed as an argument to street_view_sphere.

import eventlet
import json
import math
import os
import sys
import time
import urllib2
import ImageOps

from common import *
import get_panorama
import polyline

# given a starting and ending point, feed them to Directions and
# return the response JSON object.
def get_route_json(src, dst):
    conn = urllib2.urlopen('https://maps.googleapis.com/maps/api/directions/json?origin=%s&destination=%s&sensor=false' % (src, dst))
    data = conn.read()
    print >>open('route.json', 'w'), data
    conn.close()
    js = json.loads(data)

    if js['status'] != 'OK':
        print 'Directions query failed with status of:', js['status']
        exit()
    return js

# given the result of a call to Directions, fetch and store the
# relevant map data, and return the bounds of the data.
def write_map_data(js, fn):
    bounds = js['routes'][0]['bounds']
    buf = .02
    minlat = bounds['southwest']['lat'] - buf
    maxlat = bounds['northeast']['lat'] + buf
    minlng = bounds['southwest']['lng'] - buf
    maxlng = bounds['northeast']['lng'] + buf

    conn = urllib2.urlopen('http://overpass-api.de/api/xapi?map?bbox=%f,%f,%f,%f' %
                           (minlng, minlat, maxlng, maxlat))
    with open(fn, 'w') as f:
        s = conn.read(65536)
        while s:
            f.write(s)
            s = conn.read(65536)
    conn.close()

    return {'west': minlng,
            'south': minlat,
            'east': maxlng,
            'north': maxlat}

def get_route_pts(js):
    pts = [(js['routes'][0]['legs'][0]['start_location']['lat'],
            js['routes'][0]['legs'][0]['start_location']['lng'])]

    for step in js['routes'][0]['legs'][0]['steps']:
        pts.extend(polyline.decode(step['polyline']['points'])[1:])
    return pts

# fill in with more points, so we can find all the corresponding
# Street View panoramas.

# TODO use pano link data to avoid having to do this?
def fill_pts(pts):
    # max distance, in degrees of latitude, between adjacent points
    delta = 1e-4

    res = [pts[0]]
    for pt in pts[1:]:
        last_pt = res[-1]

        dlat = pt[0] - last_pt[0]
        dlng = cos(pt[0]) * (pt[1] - last_pt[1])
        dist = math.hypot(dlat, dlng)
        n = int(math.ceil(dist / delta))
        for i in range(n):
            lat = last_pt[0] + (pt[0] - last_pt[0]) * (i+1) / n
            lng = last_pt[1] + (pt[1] - last_pt[1]) * (i+1) / n
            res.append((lat, lng))
    return res

def get_pt_panos(pts):
    res = []
    last_panoid = None

    pile = eventlet.GreenPile(100)

    for pt in pts:
        pile.spawn(nearby_panorama, *pt)

    for i, pano in enumerate(pile):
        if not pano:
            continue
        panoid = id_from_pano(pano)
        # assume that a duplicate would be the last one
        if panoid != last_panoid:
            res.append(pano)
            last_panoid = panoid

    return res

def mkdir(d):
    try:
        os.makedirs(d)
    except OSError, e:
        # ignore 'file exists' error
        if e.errno != 17:
            raise e

if __name__ == '__main__':
    src = sys.argv[1].replace(' ', '+')
    dst = sys.argv[2].replace(' ', '+')
    outdir = os.path.join('routes', sys.argv[3])
    outjs = {}
    mkdir(outdir)

    print 'Computing route...',
    sys.stdout.flush()
    js = get_route_json(src, dst)
    pts = fill_pts(get_route_pts(js))
    print len(pts), 'points.'
    #print '\n'.join('%f %f' % pt for pt in pts)

    print 'Fetching map data...'
    outjs['bounds'] = write_map_data(js, os.path.join(outdir, 'dat.osm'))

    print 'Fetching metadata...',
    sys.stdout.flush()
    panos = get_pt_panos(pts)
    print len(panos), 'panoramas.'

    panoids = []
    for pano in panos:
        panoid = id_from_pano(pano)
        panoids.append(panoid)

        with open(os.path.join(outdir, panoid + '.json'), 'w') as out:
            json.dump(pano, out)

    outjs['panoids'] = panoids

    with open(os.path.join(outdir, 'route.txt'), 'w') as f:
        json.dump(outjs, f)

    print 'Fetching and processing images...'

    mkdir(os.path.join(outdir, 'bin'))
    def get_image(panoid):
        fn = os.path.join(outdir, panoid + '.jpg')
        print fn
        img = ImageOps.mirror(get_panorama.get_panorama_image(panoid, 5))

        with open(os.path.join(outdir, 'bin', panoid + '.bin'), 'w') as f:
            f.write(img.resize((8192, 4096)).tostring('raw', 'RGBX', 0, -1))

        img = img.resize((1664, 832))
        img.save(fn)

    pool = eventlet.GreenPool(10)
    for panoid in panoids:
        pool.spawn(get_image, panoid)
    pool.waitall()
