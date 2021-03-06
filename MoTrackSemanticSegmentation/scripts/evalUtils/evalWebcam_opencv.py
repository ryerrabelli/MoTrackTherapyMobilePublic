# Run prediction and genertae pixelwise annotation for every pixels in the testing images using fully coonvolutional neural net

#--------------------------------------------------------------------------------------------------------------------

# command line arguments
import argparse

parser = argparse.ArgumentParser(description='Evaluate how network did to identify hand vs not hand')
parser.add_argument('--model_name', default="Vgg16",
                    help='Which model to use (needs to be have following files:' + 
                        'transferLearnModel/<NAME>.npy,scripts/modelUtil/BuildNet<NAME>.py', required=False)

args = parser.parse_args()

#import tensorflow as tf
import numpy as np
import scipy.misc as misc
import sys
from trainUtils import TensorflowUtils
import os
import cv2
import time
from hwcounter import *
from trainUtils import Data_Reader
from evalUtils import OverrlayLabelOnImage as Overlay
import datetime
logs_dir= "../training_logs/" + args.model_name + "/"# "path to logs directory where trained model and information will be stored"
#logs_dir="../../MoTrackAndroid/app/src/main/assets/dnnmodels/"
#Image_Dir="../data/test/raw/"# Test image folder
#Label_Dir="../data/test/labeled/"# Test label fodler
w=0.6# weight of overlay on image
Pred_Dir="../Output_Prediction/Webcam/"+args.model_name+"/" # Library where the output prediction will be written

#model_file_name = args.model_name + ".npy"
#model_path=model_dir +model_file_name # "Path to pretrained vgg16 model for encoder"
NameEnd="" # Add this string to the ending of the file name optional
NUM_CLASSES = 2 # Number of classes
#-------------------------------------------------------------------------------------------------------------------------
#CheckNet.CheckNet(model_dir,model_file_name)# Check if pretrained vgg16 model available and if not try to download it

################################################################################################################################################################################
def main(argv=None):
    tensorflowNet = cv2.dnn.readNetFromTensorflow(logs_dir+'tensorflowModel.pb')#, logs_dir+'tensorflowModel.pbtxt')
#--------------------Create output directories for predicted label, one folder for each granulairy of label prediciton---------------------------------------------------------------------------------------------------------------------------------------------

    if not os.path.exists(Pred_Dir): os.makedirs(Pred_Dir)
    #if not os.path.exists(Pred_Dir+"/OverLay"): os.makedirs(Pred_Dir+"/OverLay")
    #if not os.path.exists(Pred_Dir + "/Label"): os.makedirs(Pred_Dir + "/Label")

    
    print("Running Predictions:")
    #print("Saving output to:" + Pred_Dir)

    #data = {"images" : {}} #will be put in a json file

#----------------------Go over all images and predict semantic segmentation in various of classes-------------------------------------------------------------
    fim = 0
    #print("Start Predicting " + str(ValidReader.NumFiles) + " images")
    cap=cv2.VideoCapture(0)
    totalElapsedCycles = 0
    totalFPS = 0
    while (True):#(ValidReader.itr < ValidReader.NumFiles):
        #print(str(fim * 100.0 / ValidReader.NumFiles) + "%")
        fim += 1
        
        # ..................................Load image.......................................................................................
        #FileName=ValidReader.OrderedFiles[ValidReader.itr] #Get input image name
        #Images,GTLabels = ValidReader.ReadNextBatchClean()  # load testing image
        ret,img=cap.read()
        #img = cv2.imread("../data/test/raw/IMG_0003_C_RAW.jpg")
        og_shape = img.shape
        #cv2.imshow("og",img)
        print(og_shape)
        img = cv2.resize(img,(96,128))
        #cv2.imshow("resized",img)
        print(img.shape)
        # Predict annotation using net
        tensorflowNet.setInput(cv2.dnn.blobFromImage(img, swapRB=False, crop=False))
        start = time.time()
        startClock = count()
        networkOutput = tensorflowNet.forward()
        end = time.time()
        elapsedClock = count_end() - start
        print(f'elapsed cycles: {elapsedClock}')
        fps = 1/(end - start)
        print("Estimated frames per second : {0}".format(fps))
        print(networkOutput.shape)
        totalElapsedCycles += elapsedClock
        totalFPS += fps
        #cv2.imshow("a",cv2.dnn.imagesFromBlob(networkOutput)[0])
        LabelPred = np.argmax(cv2.dnn.imagesFromBlob(networkOutput)[0],axis=2)
        print(np.sum(LabelPred))
        # Calculate test loss
        #TLoss=sess.run(Loss, feed_dict={image: Images, GTLabel:GTLabels , keep_prob:1.0})

        #------------------------Calculate some values on the images---------------------------------------------------------------------------------------------
        testImage = img
        
        predImage =  LabelPred
        
        #------------------------Save predicted labels overlay on images---------------------------------------------------------------------------------------------
        overlay_img = Overlay.OverLayLabelOnImage(testImage,predImage, w)
        overlay_img=cv2.resize(overlay_img,(og_shape[0],og_shape[1]))
        #cv2.putText(overlay_img,str(round(TLoss,4)),(int(overlay_img.shape[1]*0.5),int(overlay_img.shape[0]*0.2)), cv2.FONT_HERSHEY_SIMPLEX, 2,(0,0,255),2,cv2.LINE_AA)
        filename = Pred_Dir + "/"+ str(datetime.datetime.now().date()) + "_" + str(fim) + ".jpg"
        misc.imsave(filename, overlay_img ) #Overlay label on image
        #misc.imsave(Pred_Dir + "/Label/" + FileName[:-4] + ".png" + NameEnd, LabelPred[0].astype(np.uint8))
        imgToDisplay = cv2.imread(filename)
        
        cv2.imshow("overlay",imgToDisplay)
        
        k = cv2.waitKey(1) & 0xff
        if k == 27: 
            break

    cap.release()
    cv2.destroyAllWindows()
    print("Average FPS: " + str(int(totalFPS/fim)))
    print("Average Clock Cycles: " + str(int(totalElapsedCycles/fim)))

##################################################################################################################################################


main()#Run script
print("Finished")
