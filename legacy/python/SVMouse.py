#!/usr/bin/python

from common import *

class SVMouse:
    def __init__(self):
        # down_theta and down_phi are updated when the mouse button is first depressed
        self.down_theta = 0.0
        self.down_phi   = 0.0
        # track_theta and track_phi are updated by track(self, coords)
        self.track_theta = 0.0
        self.track_phi   = 0.0

    def down(self, coords):
        self.down_theta = self.track_theta = coords.theta
        self.down_phi   = self.track_phi   = coords.phi

    def track(self, coords):
        self.track_theta = coords.theta
        self.track_phi   = coords.phi

    def box(self):
        if self.down_theta < self.track_theta:
            theta1 = self.down_theta
            theta2 = self.track_theta
        else:
            theta1 = self.track_theta
            theta2 = self.down_theta
        if self.down_phi < self.track_phi:
            phi1 = self.down_phi
            phi2 = self.track_phi
        else:
            phi1 = self.track_phi
            phi2 = self.down_phi
        return theta1, phi1, theta2, phi2

    def delta(self):
        return SDirection(self.track_theta - self.down_theta, self.track_phi - self.down_phi)

    def clear(self):
        self.down_theta = 0.0
        self.down_phi   = 0.0
        self.track_theta = 0.0
        self.track_phi   = 0.0
        
