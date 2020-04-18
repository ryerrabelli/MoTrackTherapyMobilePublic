#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu May 30 12:23:43 2019

@author: benka
"""

import cv2
import os
import time
# check if data directory exists, if not make it
if not os.path.isdir("data"):
    os.mkdir("data")
    
# get Title
title = input("Type quick title: ")
# create new directory to store data 
dataDir = "data/"+str(time.time())
os.mkdir(dataDir)

#now capture and record data
cap = cv2.VideoCapture(0)
i = 0
while(True):
    ret,frame = cap.read()
    #frame = cv2.resize(frame, None, fx = 1/resize_modifier, fy = 1/resize_modifier, interpolation = cv2.INTER_CUBIC)
    i = i+1
    if i%30 == 0:
        cv2.imwrite(dataDir + "/" + title + "_" + str(i)  + ".jpg",frame)
    
    cv2.imshow("frame",frame)
    k = cv2.waitKey(30) & 0xff
    if k== ord('q'):
        break
cap.release()
cv2.destroyAllWindows()