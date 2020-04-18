#Rahul Yerrabelli
#MoTrack Therapy
#Created Fri Aug 30, 2019
#GOAL: Convert standard iOS file names ("IMG_1750.JPG") to MoTrack data image standard names ("IMG_0001_A_RAW.JPG").
#Description:
#Doesn't rename the files in place in case there is a bug. Instead takes input images in one folder, and makes output in another folder
#Assumes that the images are sequentially named in the original pairing (but not necessarily consecutive if some bad images were deleted)
#Assumes that the RAW image is first, followed by the CV image, for each pair

import cv2
import os
import re
import shutil

#VALUES THAT MIGHT NEED CHANGING
start_set_num = 180 #The first set number
num_letters_per_set = 8
images_in_each_pair = 2 #RAW vs CV

original_files_folder = 'original_names' #What folder the original files with their original names are located in
new_files_folder = 'new_names' #Where to put the renamed files

#GET INPUT FILES IN FOLDER
all_orig_file_names = os.listdir(original_files_folder)
all_orig_image_names = [k for k in all_orig_file_names if re.match(r'IMG_\d{4}', k)]
all_orig_image_names.sort() #put the files in alphabetical order. IMPORTANT!
print( "# of Files to Rename: " + str(len(all_orig_image_names)))


ct = 0
all_new_file_names = []


#DO THE RENAMING OPERATIONS
for orig_image_name in all_orig_image_names:
    set_num = start_set_num + int(ct/(num_letters_per_set*images_in_each_pair))
    letter = chr(65+(int(ct/images_in_each_pair)%num_letters_per_set)) #65 represents first letter in alphabet, "A"
    category = "RAW" if ct%2==0 else "CV"
    new_file_name = "IMG_{:04d}_{}_{}.JPG".format(set_num,letter,category)

    #UNCOMMENT THIS TO ACTUALLY DO THE REMAINING, NOT JUST TO TEST
    shutil.copyfile(original_files_folder+'/'+orig_image_name, new_files_folder+'/'+new_file_name)

    all_new_file_names.append(new_file_name)
    ct = ct + 1


#PRINT OUT THE FILE NAMES TO PUT IN EXCEL
# NOTE: Before Oct 14, 2019 there was a bug where only the new raw names were printed out, but all the old names were
# printed out (regardless of whether they'd become raw or cv), causing a mismatch in the table
print(all_new_file_names)
#all_new_raw_file_names = [k for k in all_new_file_names if "RAW" in k]
for i,new_file_name in enumerate(all_new_file_names):
    print(all_orig_image_names[i] + "\t" + new_file_name)
