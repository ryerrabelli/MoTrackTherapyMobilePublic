## given all the data, generate t

# get images in the same set
import os
import re
import string
import random

regex = "^IMG_\d{4,}_[A-Z]+_RAW.[a-zA-Z]{3,5}$"
def get_set_from_id(external_id):
    if re.match(regex,external_id):
        return re.split( "_[A-Z]+_RAW", external_id )[0]
    else:
        return None

def gen_data(raw_dir, label_dir, dir_to_use, data_save_file):   
    with open(data_save_file,'w+') as file:
        # this has the possible letters to use
        possible_letters = list(string.ascii_uppercase[1:8])
        number_copies_per_img = 3
        for f in os.listdir(raw_dir):
            set_name = get_set_from_id(f)
            # only look at images that are in set form
            if set_name is not None:
                # extract the letter in the filename
                let = f.split("_")[2]
                if not (let=='A'): # A is empty
                    # get set that doesn't include this letter
                    copy_set = possible_letters.copy() # not very efficient
                    copy_set.remove(let)
                    # get letters to use
                    letters_chosen = random.sample(copy_set,number_copies_per_img)
                    for chosen in letters_chosen:
                        img_label = set_name + '_' + chosen + '_RAW.png'
                        if os.path.isfile(label_dir + img_label):
                            file.write( f + "," + img_label+"\n")
dirs = ['train','test','val']

data_dir = "../../data/"
train_raw = data_dir + "train/raw/"
train_lab = data_dir + "train/labeled/"

for d in dirs:
    raw_dir = data_dir + d + "/raw/"
    label_dir = data_dir + d + "/labeled/"
    data_save_file = data_dir + "prevMask/fromSet_" + d + ".txt"
    gen_data(raw_dir, label_dir, d,data_save_file)
                    
    