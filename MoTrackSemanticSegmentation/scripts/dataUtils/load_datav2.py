#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#Benjamin Pikus, Rahul Yerrabelli
#July 9th, 2019
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
import datetime

# get last json file
def get_last_json_file():
    json_dir = '../../data/jsons/'
    list_of_files = glob.glob(json_dir+'*.json') # * means all if need specific format then *.csv
    ## below goes by timestamp last edited - which could be wrong
    #latest_file = max(list_of_files, key=os.path.getctime)
    all_dates = []
    for s in list_of_files:
        year,month,day = s.split("T")[0].split("-")[1:]
        all_dates.append(datetime.date(int(year),int(month),int(day)))
    latest_file = list_of_files[all_dates.index(max(all_dates))]
    print(latest_file)
    return latest_file

issues = []
def download_data():
    latest_file = get_last_json_file()
    with open(latest_file) as json_file:
        data = json.load(json_file)


        set_ct = 0
        import re
        #IMG_\d{4,}_[A-Z]+_RAW.(?i)(JPG,JPEG,PNG,GIF)(?-i)
        regex = "^IMG_\d{4,}_[A-Z]+_RAW.[a-zA-Z]{3,5}$" #^ in the beginning and $ in the end
        #data_organized = [x for x in data if re.match(regex,data['External ID'])]

        def get_set_from_id(external_id):
            if re.match(regex,external_id):
                return re.split( "_[A-Z]+_RAW", external_id )[0]
            else:
                return ""

        set_names = [get_set_from_id(i['External ID']) for i in data]
        fixed_train = ['']
        fixed_train = np.append(fixed_train, ["IMG_0043","IMG_0044","IMG_0045","IMG_0046","IMG_0047","IMG_0048","IMG_0049","IMG_0050","IMG_0051","IMG_0052","IMG_0053","IMG_0054","IMG_0055"])

        #43 to 55
        unique_set_names = list(set(set_names)) #find unique ones only
        print(unique_set_names)
        for num,i in enumerate(fixed_train):
            try:
                unique_set_names.remove(fixed_train[num])
                unique_set_names.insert(0, fixed_train[num]) #prepend
            except ValueError:
                print("Value Error. fixed_train[num]="+str(fixed_train[num]))

        varying_size = len(unique_set_names)-len(fixed_train)
        shuffled_data = np.arange(0,varying_size)
        np.random.shuffle(shuffled_data)
        train_nums = len(fixed_train)+shuffled_data[0:int(varying_size*0.8)]
        test_nums  = len(fixed_train)+shuffled_data[len(train_nums):len(train_nums)+int(varying_size*0.15)]
        val_nums   = len(fixed_train)+shuffled_data[len(train_nums)+len(test_nums):]
        train_nums = np.append(train_nums, np.arange(len(fixed_train)) ) #add fixed train items to the train

        train_img_ct = 0
        test_img_ct = 0
        val_img_ct = 0

        print(shuffled_data)
        print(train_nums)
        print(test_nums)
        print(val_nums)

        possibleDirs = ["../../data/train/","../../data/test/","../../data/val/"]
        newFiles = []
        for num,i in enumerate(data):
            #print( unique_set_names )
            #print( "a"+get_set_from_id(i['External ID']) )
            #print( np.array(unique_set_names)==get_set_from_id(i['External ID']) )
            #break
            #set_i = np.which(np.array(unique_set_names)==get_set_from_id(i['External ID']) )
            set_i = unique_set_names.index(get_set_from_id(i['External ID']))

            if set_i in train_nums:
                which_dir = "train"
                train_img_ct += 1
            elif set_i in test_nums:
                which_dir = "test"
                test_img_ct += 1
            elif set_i in val_nums:
                which_dir="val"
                val_img_ct += 1
            else:
                raise Exception('Images of set #' + str(set_i) + ' not allocated to any set (training, testing, or evaluating)')

            

            external_id = i['External ID'].split(".")[0]

            # check if file is new to data set
            isNew = True
            for g in possibleDirs: # need to check if its in test, train, or val
                if os.path.isfile(g + "raw/" + external_id  + ".jpg"):
                    isNew = False
                    break
            print("Processing image #" + str(num) + ", external id='" + i['External ID'] + "',  which is in myset=" + str(set_i) + " (Set Name='" + get_set_from_id(i['External ID']) + "'). Will go into " + which_dir + ". Is it new - " + str(isNew) )

            #continue
                

            # get labeled data
            label_info = i['Label']
            if label_info != "Skip" and isNew:
                if 'segmentationMaskURL' in label_info:
                    url = label_info['segmentationMaskURL']
                    r = requests.get(url, allow_redirects=True)
                    labeled_dir_to_save = ("../../data/" + which_dir + "/labeled/" + external_id + ".png")
                    open(labeled_dir_to_save, 'wb').write(r.content)
                    transform_labeled_img(labeled_dir_to_save)
                    # get raw data
                    url = i['Labeled Data']
                    r = requests.get(url, allow_redirects=True)
                    open("../../data/" + which_dir + "/raw/" + external_id + ".jpg", 'wb').write(r.content)
                    if which_dir == 'train':
                        newFiles.append(external_id + ".jpg")
                else:
                    issues.append(external_id)
        print("Results: Total Images Done=" + str(train_img_ct+test_img_ct+val_img_ct))
        print("" + str(train_img_ct) + " images are in the training set, " + str(test_img_ct) + " images are in the testing set, and " + str(val_img_ct) + " images are in the evaluation set.")
    # now write the new filenames to a partition file
    
    with open("../../data/partitions/newFiles_" + latest_file.split("/")[-1].split(".")[0] + ".txt", "w") as text_file:
        # iterate through all the files
        for f in newFiles:
            text_file.write(f + "\n")

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
