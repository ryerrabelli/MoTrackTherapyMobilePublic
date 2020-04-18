# Run prediction and genertae pixelwise annotation for every pixels in the testing images using fully coonvolutional neural net

#--------------------------------------------------------------------------------------------------------------------

# command line arguments
import argparse

parser = argparse.ArgumentParser(description='Evaluate how network did to identify hand vs not hand')
parser.add_argument('--model_name', default=None, 
                    type=str, help='Name of model to use', required=False)
parser.add_argument('--img_dir', type=str, help='Directory where the images are saved', required=True)
parser.add_argument('--dataset_name', type=str,
                    help='name which will be appended to beginning to all images written in val and test', 
                    required=True)
#parser.add_argument('--rows', default=-1, type=int,
#                    help='What to resize the input rows to (if -1, then its None)', required=False)
#parser.add_argument('--cols', default=-1, type=int,
#                    help='What to resize the input cols to (if -1, then its None)', required=False)

args = parser.parse_args()

import tensorflow as tf
import datetime
import numpy as np
import scipy.misc as misc
import sys
sys.path.append("..")
# script for building model
if args.model_name =="Vgg16":
    from modelUtils import BuildNetVgg16 as BuildNet
    #from modelUtils import CheckModelVgg16 as CheckNet
elif args.model_name =="MobileNetV1":
    from modelUtils import BuildNetMobileV1 as BuildNet
    #from modelUtils import CheckModelMobileV1 as CheckNet
elif args.model_name =="MobileNetV1_lite":
    from modelUtils import BuildNetMobileV1_lite as BuildNet
    #from modelUtils import CheckModelMobileV1_lite as CheckNet
elif args.model_name =="MobileNetV1_2lite":
    from modelUtils import BuildNetMobileV1_2lite as BuildNet
elif args.model_name =="MobileNetV1_2lite_v2":
    from modelUtils import BuildNetMobileV1_2lite_v2 as BuildNet
elif args.model_name =="MobileNetV1_superlite":
    from modelUtils import BuildNetMobileV1_superlite as BuildNet
elif args.model_name =="MobileNetV1_layers":
    from modelUtils import BuildNetMobileV1_layers as BuildNet
from trainUtils import TensorflowUtils
import os
import cv2
from trainUtils import Data_Reader
from evalUtils import OverrlayLabelOnImage as Overlay
import datetime
if args.model_name is not None:
    logs_dir= "../../training_logs/" + args.model_name + "/"# "path to logs directory where trained model and information will be stored"
else:
    logs_dir = "../../../MoTrackAndroid/app/src/main/assets/dnnmodels/"
data_dir = "../../data/"
Test_Img_Dir=data_dir + "test/raw/"# Test image folder
Test_Label_Dir= data_dir + "test/labeled/"# Test label folder
Val_Img_Dir=data_dir + "val/raw/"# Val image folder
Val_Label_Dir=data_dir + "val/labeled/"# val label folder
Train_Img_Dir=data_dir + "train/raw/"# train image folder
Train_Label_Dir=data_dir + "train/labeled/"# train label fodler
needs_label_dir = data_dir + "needs_labeling/"
if not os.path.exists(needs_label_dir): os.makedirs(needs_label_dir)

w=0.6# weight of overlay on image
if args.model_name is not None:
    Pred_Dir="../../Output_Prediction/Webcam/"+args.model_name+"/" # Library where the output prediction will be written
else:
    Pred_Dir="../../Output_Prediction/Webcam/default/"
model_dir ="../../transferLearnModels/" 
#model_file_name = args.model_name + ".npy"
#model_path=model_dir +model_file_name # "Path to pretrained vgg16 model for encoder"
NameEnd="" # Add this string to the ending of the file name optional
NUM_CLASSES = 2 # Number of classes
#-------------------------------------------------------------------------------------------------------------------------
#CheckNet.CheckNet(model_dir,model_file_name)# Check if pretrained vgg16 model available and if not try to download it

################################################################################################################################################################################
def main(argv=None):
    
    if not os.path.exists(Pred_Dir): os.makedirs(Pred_Dir)
    #if not os.path.exists(Pred_Dir+"/OverLay"): os.makedirs(Pred_Dir+"/OverLay")
    #if not os.path.exists(Pred_Dir + "/Label"): os.makedirs(Pred_Dir + "/Label")
    
    tensorflowNet = cv2.dnn.readNetFromTensorflow(logs_dir+'tensorflowModel.pb')
    
    print("Running Predictions:")
    #print("Saving output to:" + Pred_Dir)

    #data = {"images" : {}} #will be put in a json file

#----------------------Go over all images and predict semantic segmentation in various of classes-------------------------------------------------------------
    fim = 0
    needs_labeling = []
    #print("Start Predicting " + str(ValidReader.NumFiles) + " images")
    iqr_name = "IQR_" + args.dataset_name + "_" + datetime.datetime.now().strftime("%Y-%m-%d")
    if not os.path.exists(needs_label_dir + iqr_name): os.makedirs(needs_label_dir + iqr_name)
    with open("../../data/partitions/" +  iqr_name, "w") as text_file:
        for f in os.listdir(args.img_dir):
            #print(str(fim * 100.0 / ValidReader.NumFiles) + "%")
            fim += 1
            
            # ..................................Load image.......................................................................................
            #FileName=ValidReader.OrderedFiles[ValidReader.itr] #Get input image name
            #Images,GTLabels = ValidReader.ReadNextBatchClean()  # load testing image
            orig_img = cv2.imread(args.img_dir + "/" + f)
            img = cv2.resize(orig_img,(96,128))
            tensorflowNet.setInput(cv2.dnn.blobFromImage(img, swapRB=False, crop=False))
            networkOutput = tensorflowNet.forward()
            LabelPred = np.argmax(cv2.dnn.imagesFromBlob(networkOutput)[0],axis=2)
            testImage = img
            predImage =  LabelPred
            #------------------------Save predicted labels overlay on images---------------------------------------------------------------------------------------------
            overlay_img = Overlay.OverLayLabelOnImage(testImage,predImage, w)
            
            #cv2.putText(overlay_img,str(round(TLoss,4)),(int(overlay_img.shape[1]*0.5),int(overlay_img.shape[0]*0.2)), cv2.FONT_HERSHEY_SIMPLEX, 2,(0,0,255),2,cv2.LINE_AA)
            filename = Pred_Dir + "/"+ str(datetime.datetime.now().date()) + "_" + str(fim) + ".jpg"
            misc.imsave(filename, overlay_img ) #Overlay label on image
            #misc.imsave(Pred_Dir + "/Label/" + FileName[:-4] + ".png" + NameEnd, LabelPred[0].astype(np.uint8))
            imgToDisplay = cv2.imread(filename)
            imgToDisplay = cv2.resize(imgToDisplay,(orig_img.shape[1],orig_img.shape[0]))
            cv2.imshow("overlay",imgToDisplay)
            
            k = cv2.waitKey(0)
            if k == ord('g'): # good, put in either test (70%) or val (30%)
                if np.random.uniform() <= 0.7: # put in test
                    cv2.imwrite(Test_Img_Dir + args.dataset_name + "_" + f.split(".")[0] + ".jpg",orig_img)
                    cv2.imwrite(Test_Label_Dir + args.dataset_name+"_" + f.split(".")[0] + ".png",LabelPred[0])
                else: # put in val
                    cv2.imwrite(Val_Img_Dir + args.dataset_name + "_" + f.split(".")[0] + ".jpg",orig_img)
                    cv2.imwrite(Val_Label_Dir + args.dataset_name+"_" + f.split(".")[0] + ".png",LabelPred[0])
            elif k == ord('b'): # bad, put in train (80%), test(15%) or val (5%)
                num = np.random.uniform()
                file_name = args.dataset_name + "_" + f.split(".")[0] + ".jpg"
                if num <= 0.8: # put in train
                    cv2.imwrite(Train_Img_Dir + file_name,orig_img)
                    #cv2.imwrite(Train_Label_Dir + args.dataset_name+"_" + f.split(".")[0] + ".png",LabelPred[0])
                    text_file.write(file_name + "\n")
                    needs_labeling.append("train/raw/"+file_name)
                elif num <= 0.95: # put in test
                    cv2.imwrite(Test_Img_Dir +file_name,orig_img)
                    #cv2.imwrite(Test_Label_Dir + args.dataset_name+"_" + f.split(".")[0] + ".png",LabelPred[0])
                    needs_labeling.append("test/raw/"+file_name)
                else: # put in val
                    cv2.imwrite(Val_Img_Dir + file_name,orig_img)
                    #cv2.imwrite(Val_Label_Dir + args.dataset_name+"_" + f.split(".")[0] + ".png",LabelPred[0])
                    needs_labeling.append("val/raw/"+file_name)
                cv2.imwrite(needs_label_dir+iqr_name + "/"+ file_name,orig_img )
            elif k == ord('i'): # ignore
                pass
            elif k == 27: 
                break
    cv2.destroyAllWindows()
    with open(needs_label_dir + iqr_name + ".txt", "w") as text_file:
        # iterate through all the files
        for g in needs_labeling:
            text_file.write(g + "\n")

##################################################################################################################################################


main()#Run script
print("Finished")
