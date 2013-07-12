#!/usr/bin/python

from common import *

class Vector:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

    def __add__(self, a):
        return Vector(self.x + a.x, self.y + a.y, self.z + a.z)

    def __sub__(self, a):
        return Vector(self.x - a.x, self.y - a.y, self.z - a.z)
        
    def __rmul__(self, s):
        return Vector(s*self.x, s*self.y, s*self.z)

# Rotates a Vector about the x axis
def rotx(theta, p):
    cost = cos(theta)
    sint = sin(theta)
    return Vector(p.x, p.y*cost - p.z*sint, p.z*cost + p.y*sint)

# Rotates a Vector about the y axis
def roty(theta, p):
    cost = cos(theta)
    sint = sin(theta)
    return Vector(p.x*cost + p.z*sint, p.y, p.z*cost - p.x*sint)

# Rotates a Vector about the z axis
def rotz(theta, p):
    cost = cos(theta)
    sint = sin(theta)
    return Vector(p.x*cost - p.y*sint, p.y*cost + p.x*sint, p.z)

# Converts Cartesian to Spherical
def cartesianToSpherical(point):
    r     = math.sqrt(point.x*point.x + point.y*point.y + point.z*point.z)
    theta = atan2(point.y, point.x)
    phi   = asin(point.z / r)
    return SPoint(theta, phi, r)

# Converts Spherical to Cartesian
def sphericalToCartesian(point):
    r = point.r * cos(point.phi) # distance from origin of point projected onto xy plane
    x = r * cos(point.theta)
    y = r * sin(point.theta)
    z = point.r * sin(point.phi)
    return CPoint(x, y, z)

# All orientation related variables needed by the SVApp
class Orientation:
    def __init__(self):
        # direction we're facing
        self.theta      = 0.0   # degrees counterclockwise from east, along the plane of the ground
        self.phi        = 0.0   # degrees up or down from the ground

        # orientation of panorama relative to cardinal directions
        # these are just copies of the same variables in SVApp.pano
        self.pano_yaw   = 0.0   # yaw of the center of the current panorama
        self.tilt_yaw   = 0.0   # yaw of the direction of greatest slope (of the ground)
        self.tilt_pitch = 0.0   # pitch of the ground relative to cardinal plane when facing tilt_yaw

        # field of view
        self.fov_theta  = 0.0   # degrees visible along the azimuth
        self.fov_phi    = 0.0   # degrees visible up and down
        self.fov_mult   = 0.9   # fov is multiplied or divided by this when zooming in / out
        self.fov_min    = 7.5   # mimimum allowed fov
        self.fov_max    = 140.  # maximum allowed fov
        self.spos       = Vector(0,0,0)
        self.sright     = Vector(0,0,0)
        self.sleft      = Vector(0,0,0)

    # Makes convenience copies of the panorama's orientation
    def setPano(self, pano):
        self.pano_yaw = pano.panoYaw
        self.tilt_yaw = pano.tiltYaw
        self.tilt_pitch = pano.tiltPitch

    # Defines the field of view. Necessary for figuring out spherical mouse coordinates.
    def setFov(self, fovt, fovp):
        self.fov_theta = fovt
        self.fov_phi   = fovp
        tant = tan(self.fov_theta / 2.)
        tanp = tan(self.fov_phi / 2.)
        x = 1.0 / math.sqrt( 1 + tant*tant + tanp*tanp )
        self.spos   = Vector(x, x*tant, x*tanp)
        self.sright = Vector(0, -2*x*tant, 0)
        self.sdown  = Vector(0, 0, -2*x*tanp)

    def zoomIn(self):
        fovt = self.fov_theta * self.fov_mult
        fovp = self.fov_phi * self.fov_mult
        if fovt < self.fov_min: fovt = self.fov_min
        if fovp < self.fov_min: fovp = self.fov_min
        self.setFov(fovt, fovp)

    def zoomOut(self):
        fovt = self.fov_theta / self.fov_mult
        fovp = self.fov_phi / self.fov_mult
        if fovt > self.fov_max: fovt = self.fov_max
        if fovp > self.fov_max: fovp = self.fov_max
        self.setFov(fovt, fovp)

    # Takes the pixel coordinates of the mouse and calculates where on the sphere it points.
    # mousex and mousey should both be normalized to the range (0, 1),
    # and measured from the top left of the window (x horizontal, y vertical).
    # The returned angles are measured in the ground plane: counterclockwise from east and
    # up from the ground.
    def screenCoords(self, right, down):
        m = cartesianToSpherical( rotz(self.theta, roty(-self.phi, self.spos + right*self.sright + down*self.sdown)) )
        # Right now m.theta is in the range (-180, 180]. We want to put it in the range
        # (self.phi - 180, self.phi + 180] so the branch point is as far away as it can be.
        theta = m.theta
        if theta < self.theta - 180.: theta += 360.
        if theta > self.theta + 180.: theta -= 360.
        return SDirection(theta, m.phi)

    def cartesianScreenCoords(self):
        return rotz(self.theta, roty(-self.phi, self.spos))

    # Finds the angle in the ground plane from east to the tilt axis
    def tiltedYaw(self):
        if self.tilt_yaw <= 180.0:
            bigyaw = False
            cosy = cos(self.tilt_yaw)
        else:
            bigyaw = True
            cosy = cos(self.tilt_yaw - 180.0)
        cosp = cos(self.tilt_pitch)
        sinp = sin(self.tilt_pitch)
        temp = acos( cosy / math.sqrt(cosp*cosp + cosy*cosy * sinp*sinp) )
        if bigyaw:
            return temp + 180.0
        else:
            return temp

    # Finds the pitch angle that places the x axis in the ground plane
    # when the x axis is theta degrees councterclockwise from east in the cardinal plane.
    def tiltedPitch(self, theta):
        cosp = cos(self.tilt_pitch)
        sinp = sin(self.tilt_pitch)
        siny = sin(theta)
        temp = acos( cosp / math.sqrt(cosp*cosp + sinp*sinp * siny*siny) )
        if theta > 180.0:
            return -temp
        else:
            return temp

