#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#Benjamin Pikus, Rahul Yerrabelli
#July 2019
#MoTrack Therapy

"""
Reads in json file from labelbox and downloads all appropriate images into proper directories
@author: benka
"""
import os
import json
import requests
import numpy as np
import cv2
import glob

# get last json file
def get_last_json_file():
    json_dir = '../../data/jsons/'
    list_of_files = glob.glob(json_dir+'*.json') # * means all if need specific format then *.csv
    latest_file = max(list_of_files, key=os.path.getctime)
    return json_dir+latest_file

def download_data():
    with open(get_last_json_file()) as json_file:
        data = json.load(json_file)

        set_ct = 0
        import re
        #IMG_\d{4,}_[A-Z]+_RAW.(?i)(JPG,JPEG,PNG,GIF)(?-i)
        regex = "^IMG_\d{4,}_[A-Z]+_RAW.[a-zA-Z]{3,5}$|" #^ in the beginning and $ in the end
        data_organized = [x for x in data if re.match(regex,data['External ID'])]

        # get indices for train,test, and val
        shuffled_data = np.arange(0,len(data))
        np.random.shuffle(shuffled_data)
        train_nums = shuffled_data[0:int(len(shuffled_data)*0.8)]
        test_nums = shuffled_data[len(train_nums):len(train_nums)+int(len(shuffled_data)*0.15)]
        val_nums = shuffled_data[len(train_nums)+len(test_nums):]

        for num,i in enumerate(data):
            print(num)
            if num in train_nums:
                which_dir = "train"
            elif num in test_nums:
                which_dir = "test"
            elif num in val_nums:
                which_dir="val"
            else:
                raise Exception('Image not allocated to any set (training, testing, or evaluating)')

            external_id = i['External ID'].split(".")[0]
            # get labeled data
            label_info = i['Label']
            if label_info != "Skip":
                url = label_info['segmentationMaskURL']
                r = requests.get(url, allow_redirects=True)
                labeled_dir_to_save = ("../../data/" + which_dir + "/labeled/" + external_id + ".png")
                open(labeled_dir_to_save, 'wb').write(r.content)
                transform_labeled_img(labeled_dir_to_save)
                # get raw data
                url = i['Labeled Data']
                r = requests.get(url, allow_redirects=True)
                open("../../data/" + which_dir + "/raw/" + external_id + ".jpg", 'wb').write(r.content)


def transform_labeled_img(img_path):
    background = [255,255,255]
    hand = [67, 128 ,11]
    not_hand = [87 , 20 , 173]
    img = cv2.imread(img_path)
    copy = np.zeros((img.shape[0],img.shape[1]))
    copy[np.where((img == hand).all(axis = 2))] = 1#[1,1,1] # hand
    copy[np.where((img == not_hand).all(axis = 2))] = 0#[0,0,0] # background
    copy[np.where((img == background).all(axis = 2))] = 0#[0,0,0] # background
    cv2.imwrite(img_path,copy)


def visualize_img(f):
    img = cv2.imread(f)
    img[img==1] = 255
    cv2.imshow(f,img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
def visualize(data_timestamp):
    dataDir ="labeled_data/" + data_timestamp + "/"
    for f in os.listdir(dataDir):

        visualize_img(dataDir+f)

if __name__ == "__main__":
    download_data()