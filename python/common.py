import json
import math

from openanything import fetch

# trig functions dealing with degrees
D2R = math.pi / 180
def sin(d): return math.sin(d * D2R)
def cos(d): return math.cos(d * D2R)
def tan(d): return math.tan(d * D2R)
def asin(x): return math.asin(x) / D2R
def acos(x): return math.acos(x) / D2R
def atan(x): return math.atan(x) / D2R
def atan2(y, x): return math.atan2(y, x) / D2R

# functions to handle panorama metadata
def pan_lat_lng(pan):
    return pan['Location']['lat'], pan['Location']['lng']

def pan_links_panoids(pan):
    return [link['panoId'] for link in pan['Links']]

def nearby_panorama(lat, lng):
    xml_url = 'http://cbk0.google.com/cbk?output=xml&ll=%s,%s' % (lat, lng)
    json_url = 'http://cbk0.google.com/cbk?output=json&ll=%s,%s' % (lat, lng)
    resp = fetch(json_url)
    if resp['status'] != 200:
        return None
    else:
        pan = json.loads(resp['data'])
        return pan

def id_from_pano(pano):
    return pano['Location']['panoId']

def pano_from_id(panoid):
    json_url = 'http://cbk0.google.com/cbk?output=json&panoid=%s&pitch=0&yaw=0' % panoid
    resp = fetch(json_url)

    if resp['status'] != 200:
        return None
    else:
        pan = json.loads(resp['data'])
        return pan

