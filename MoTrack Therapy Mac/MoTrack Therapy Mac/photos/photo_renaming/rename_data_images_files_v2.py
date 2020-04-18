#Rahul Yerrabelli
#MoTrack Therapy
#Created Mon Oct 14, 2019
#GOAL: Convert standard iOS file names ("IMG_1750.JPG") to MoTrack data image standard names ("IMG_0001_A_RAW.JPG").
#Description:
#Doesn't rename the files in place in case there is a bug. Instead takes input images in one folder, and makes output in another folder
#Assumes that the images are sequentially named in the original pairing (but not necessarily consecutive if some bad images were deleted)
#Assumes that the RAW image is first, followed by the CV image, for each pair


import os
import re
import shutil



################################################################
############################ PART A ############################
###################### CONFIGURE DETAILS  ######################
################################################################

#VALUES THAT MIGHT NEED CHANGING
num_letters_per_set = -1 #Make this 8 for A->H system. Do -1 to enter in specific points
set_starters = [8, 34, 36, 52, 78, 102, 104, 108, 128, 130, 152, 172, 192, 214, 230, 248, 266, 284, 286,
                310, 312, 314, 316, 334, 352, 370] #Enter first image in each set
start_set_num = 205 #The first set number
image_pair_names = ["RAW", "CV"] #makes images_in_each_pair 2

original_files_folder = 'original_names' #What folder the original files with their original names are located in
new_files_folder = 'new_names' #Where to put the renamed files



################################################################
############################ PART B ############################
###################### DO THE OPERATIONS  ######################
################################################################

#GET INPUT FILES IN FOLDER
all_orig_file_names = os.listdir(original_files_folder)
all_orig_image_names = [k for k in all_orig_file_names if re.match(r'IMG_\d{4}', k)]
all_orig_image_names.sort() #put the files in alphabetical order. IMPORTANT!
print( "# of Files to Rename: " + str(len(all_orig_image_names)))

#(STARTING) CONSTANTS
images_in_each_pair = len(image_pair_names) #2 for RAW and CV
ct = 0
ct_within_set = 0 #doesn't count duplicate _A1 _A2 etc
tot_ct_within_set = 0
set_num = start_set_num
set_names = []
set_lengths = []
set_A_cts = {}
all_new_file_names = []
set_starters_str = ["IMG_{:04d}.JPG".format(img_num) for img_num in set_starters]
error_str = ""
renamed_count = 0

#DO THE RENAMING OPERATIONS
for orig_image_name in all_orig_image_names:
    if num_letters_per_set>0 and ct % (num_letters_per_set*images_in_each_pair)==0:
        set_names.append("{:04d}".format(set_num))
        set_lengths.append(tot_ct_within_set)
        set_num += 1
        tot_ct_within_set = 0
        ct_within_set = 0
    elif orig_image_name in set_starters_str:
        if ct_within_set>2*images_in_each_pair: #2 here because only A and B are allowed to be duplicated. Anything else, make a new set
            set_names.append("{:04d}".format(set_num))
            set_lengths.append(tot_ct_within_set)
            set_num += 1
            tot_ct_within_set = 0
            ct_within_set = 0
        else:
            #Don't increment set number. Just make it named like _A1_RAW.JPG, etc
            ct_within_set -= images_in_each_pair
            if ct_within_set < 0:
                ct_within_set = 0

    if ct_within_set >= 26*images_in_each_pair:
        error_str += "ERROR: EXCEEDED ALL 26 ALPHABET LETTER OPTIONS.\n"
        print("Error. Breaking out of loop.")
        break

    letter = chr(65+(int(ct_within_set/images_in_each_pair))) #65 represents first letter in alphabet, "A"
    if letter=="A":
        set_A_cts["{:04d}".format(set_num)] = 0
    category = image_pair_names[ct%images_in_each_pair]
    subcategoryNum = 0
    while True:
        new_file_name = "IMG_{:04d}_{}{}_{}.JPG".format(set_num, letter, "" if subcategoryNum==0 else str(subcategoryNum), category )
        if letter=="A":
            set_A_cts["{:04d}".format(set_num)] = set_A_cts["{:04d}".format(set_num)] + 1
        if new_file_name not in all_new_file_names:
            break
        print("While renaming '" + orig_image_name + "', already found intended a file that is already named '"+new_file_name+"'.")
        subcategoryNum += 1



    #UNCOMMENT THIS TO ACTUALLY DO THE REMAINING, NOT JUST TO TEST
    #shutil.copyfile(original_files_folder+'/'+orig_image_name, new_files_folder+'/'+new_file_name)
    renamed_count += 1

    all_new_file_names.append(new_file_name)
    ct += 1
    ct_within_set += 1
    tot_ct_within_set += 1

#FINISH OFF FOR LOOP BY COMPLETING LAST SET
set_names.append("{:04d}".format(set_num))
set_lengths.append(tot_ct_within_set)



################################################################
############################ PART C ############################
####################### PRINT OUT OUTPUT #######################
################################################################

#PRINT OUT THE FILE NAMES TO PUT IN EXCEL
print()
#print(all_new_file_names)
print("#\tOld File Name\tNew File Name")
for i,new_file_name in enumerate(all_new_file_names):
    print(str(i) + "\t" + all_orig_image_names[i] + "\t" + new_file_name)

#PRINT OUT SET LENGTHS
print()
print("#\tName\tTot\tPairs\tAs\tLetters")
renamed_count_check = 0
for i,set_name in enumerate(set_names):
    set_length_pair = int(set_lengths[i]/images_in_each_pair+0.5)
    str_to_print = str(i)+"\t" + set_name + "\t" + str(set_length_pair) + "\t" + str(set_A_cts[set_name])
    str_to_print += "\tA-{}".format( chr(65+set_lengths[i]-set_length_pair-1) )
    if set_A_cts[set_name] > 1:
        for i2 in range(2,set_A_cts[set_name]+1):
            str_to_print += ",A"+str(i2)
    print(str_to_print)
    renamed_count_check += set_lengths[i]

#PRINT OUT TOTAL NUMBER OF IMAGES RENAMED
print()
print("Renamed a total of " + str(renamed_count) + " images (Double checked value=" + str(renamed_count_check) + ")")
if renamed_count != renamed_count_check:
    error_str += "ERROR: DOUBLING CHECK TOTAL COUNT OF RENAMED IMAGES FAILED." \
                 " {} != {}. len(all_orig_image_names)={}. len(all_new_file_names)={}" \
                 ".\n".format(renamed_count,renamed_count_check,len(all_orig_image_names),len(all_new_file_names))


#PRINT OUT ANY ERRORS
print(error_str)


