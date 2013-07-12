#!/usr/bin/python

import json
import math
import collections
from openanything import fetch

# A lat / lon pair
class Location:
   def __init__(self, lat = 0.0, lon = 0.0):
      self.lat = lat
      self.lon = lon

# A link between two adjacent panoramas
class Edge:
   def __init__(self, angle = 0.0, panoid = 0.0):
      self.angle = angle
      self.panoid = panoid

# A single tag added to a panorama.
class Tag:
    def __init__(self, box):
        # thetas are measured counterclockwise from east in the ground plane.
        # phis are measured up from the ground plane.
        # Note that we do not constrain lats / lons to be in [0, 360) and [-90,90).
        # If we did there would be no way to draw a tag crossing either branch point.
        self.theta1 = box[0]
        self.phi1   = box[1]
        self.theta2 = box[2]
        self.phi2   = box[3]
    def center(self):
        theta = (self.theta1 + self.theta2) / 2.
        phi = (self.phi1 + self.phi2) / 2.
        return SDirection(theta, phi)


# All the metadata associated with a panorama
class Panorama:
   def __init__(self):
      self.dbid = 0
      self.panoid = ""
      self.indb = None
      self.location = Location()
      self.origLocation = Location()
      self.capYear = 0
      self.capMonth = 0
      self.panoYaw = 0.0
      self.tiltYaw = 0.0
      self.tiltPitch = 0.0
      self.edges = []
      self.tags = []

   def addEdge(self, angle, panoid):
      return self.edges.append(Edge(angle, panoid))

   def addTag(self, theta1, phi1, theta2, phi2):
      return self.tags.append(Tag((theta1, phi1, theta2, phi2)))


# A point in cartesian coordinates
CPoint = collections.namedtuple('CPoint', ['x', 'y', 'z'])

# A point in spherical coordinates
SPoint = collections.namedtuple('SPoint', ['theta', 'phi', 'r'])

# A direction in spherical coordinates
SDirection = collections.namedtuple('SDirection', ['theta', 'phi'])


# trig functions dealing with degrees
D2R = math.pi / 180
def sin(d): return math.sin(d * D2R)
def cos(d): return math.cos(d * D2R)
def tan(d): return math.tan(d * D2R)
def asin(x): return math.asin(x) / D2R
def acos(x): return math.acos(x) / D2R
def atan(x): return math.atan(x) / D2R
def atan2(y, x): return math.atan2(y, x) / D2R

