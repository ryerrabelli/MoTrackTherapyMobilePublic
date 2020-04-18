#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Aug 28 08:04:40 2019

@author: benka
"""

import cv2
import os
from os import listdir
from os.path import  join

Train_Image_Dir="../data/train/raw/"
Specify_Dir = "../data/partitions/"
if not os.path.exists(Specify_Dir): os.makedirs(Specify_Dir)
partition_name = input("Enter name of this data partition: ") 

# iterate through all the train images and select if they go in partition
def iterate_through():
    # write to file
    with open(Specify_Dir + partition_name + ".txt", "w") as text_file:
        # iterate through all the files
        for f in listdir(Train_Image_Dir):
            filename = join(Train_Image_Dir,f)
            img = cv2.imread(filename)
            cv2.imshow("img",img)
            k = cv2.waitKey(0)
            if k == ord('q'): 
                break
            elif k == ord('1'): # put in partition
                text_file.write(f + "\n")
            elif k == ord('2'): # put in partition
                continue
    cv2.destroyAllWindows()

# put all images in a set number in the partition
def set_partition():
    in_set = [180,181,182, 184, 185,186, 187,188,190,191,
              192,193,194,196,197,198,199,200,201,202,203,204]
    letters = ['A','B','C','D','E','F','G','H']
    # write to file
    with open(Specify_Dir + partition_name + ".txt", "w") as text_file:
        # iterate through all the files
        for f in in_set:
            for l in letters:
                img_name = "IMG_0" + str(f) + "_" + l + "_RAW.jpg"
                filename = join(Train_Image_Dir, img_name )
                if os.path.exists(filename):
                    text_file.write(img_name + '\n')
    
set_partition()