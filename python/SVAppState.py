#!/usr/bin/python

# ==================== Class AppState ==================== 
# The SVApp object contains an AppState to keep track of its state.
# In particular the AppState object is used to determine the correct response to mouse events.
# 0 = normal (we don't track the mouse)
# 1 = dragging (we track the mouse to move the Sphere)
# 2 = tagging  (we track the mouse to draw a tag)
# 3 = confirming (we don't track the mouse)
class AppState:
    def __init__(self):
        self.state = 0
	self.dragging = False
    def setNormal(self):
        self.state = 0
    def setTagging(self):
        self.state = 1
    def setConfirming(self):
        self.state = 2
    def setReviewing(self):
        self.state = 3
    def startDragging(self):
        self.dragging = True
    def stopDragging(self):
        self.dragging = False
    def isNormal(self):
        if self.state == 0: return True
    def isTagging(self):
        if self.state == 1: return True
    def isConfirming(self):
        if self.state == 2: return True
    def isReviewing(self):
        if self.state == 3: return True
    def isDragging(self):
        return self.dragging
    def trackMouse(self):
        if self.dragging or self.isTagging(): return True
    def drawTagBox(self):
        if self.isTagging() or self.isConfirming(): return True


