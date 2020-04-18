# Run prediction and genertae pixelwise annotation for every pixels in the testing images using fully coonvolutional neural net

#--------------------------------------------------------------------------------------------------------------------

# command line arguments
import argparse

parser = argparse.ArgumentParser(description='Evaluate how network did to identify hand vs not hand')
parser.add_argument('--model_name', default="Vgg16",
                    help='Which model to use (needs to be have following files:' + 
                        'transferLearnModel/<NAME>.npy,scripts/modelUtil/BuildNet<NAME>.py', required=False)
parser.add_argument('--method', default="normal",
                    help='Which method to use: normal (just use model on every frame),'
                    + 'backproj: (use backprojection for 9/10 frames'
                    + 'mog2 (use mog2 for 9/10 frames'
                    + 'knn (use knn for 9/10 frames'
                    + 'grabcut (use grabcut for 9/10 frames', required=False)

args = parser.parse_args()

#import tensorflow as tf
import numpy as np
import scipy.misc as misc
import sys
from trainUtils import TensorflowUtils
import matplotlib.pyplot as plt
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
    tensorflowNet = cv2.dnn.readNetFromTensorflow(logs_dir+'tensorflowModel.pb')
    if not os.path.exists(Pred_Dir): os.makedirs(Pred_Dir)

    
    print("Running Predictions:")
    fim = 0
    cap=cv2.VideoCapture(0)
    totalElapsedCycles = 0
    totalFPS = 0
    if args.method == 'backproj':
        element = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (7,7))
        hue_fig = plt.figure()
        sat_fig = plt.figure()
    elif args.method == 'mog2':
        backSub = cv2.createBackgroundSubtractorMOG2()
    elif args.method == 'knn':
        backSub = cv2.createBackgroundSubtractorKNN()


    while (True):
        ret,img=cap.read()
        og_shape = img.shape
        img = cv2.resize(img,(96,128))
        startClock = count()
        start = time.time()
        if fim%100 == 0 or args.method == 'normal':
            tensorflowNet.setInput(cv2.dnn.blobFromImage(img, swapRB=False, crop=False))
            networkOutput = tensorflowNet.forward()
            LabelPred_org = np.argmax(cv2.dnn.imagesFromBlob(networkOutput)[0],axis=2).astype(np.uint8)
            
            if args.method == 'backproj':
                LabelPred = cv2.erode(LabelPred_org, element)
                LabelPred[LabelPred == 1] = 255
                LabelMask = img.copy()
                cv2.imshow("LabelPred_erode",LabelPred)
            elif args.method == 'mog2' or args.method == 'knn':
                HandMask = img.copy()
                # set all hand pixels to 0
                HandMask[LabelPred_org==1] = 0
                fgMask = backSub.apply(HandMask, learningRate = 1)
                cv2.imshow("fgmask",fgMask)
            #cv2.imshow("LabelPred",LabelPred_org)
            testImage = img
            overlay_img = Overlay.OverLayLabelOnImage(testImage,LabelPred_org, w)
            overlay_img=cv2.resize(overlay_img,(og_shape[0],og_shape[1]))
            filename = Pred_Dir + "/"+ str(datetime.datetime.now().date()) + "_" + str(fim) + ".jpg"
            misc.imsave(filename, overlay_img ) #Overlay label on image
            imgToDisplay = cv2.imread(filename)
        elif args.method == 'backproj':
            hsv = cv2.cvtColor(LabelMask,cv2.COLOR_BGR2HSV)
            hsvt = cv2.cvtColor(img,cv2.COLOR_BGR2HSV)
            # calculating object histogram
            roihist = cv2.calcHist([hsv], [0, 1], LabelPred, [180,256], [0, 180, 0, 256] )
            #roihist = cv2.calcHist([hsv], [0], LabelPred, [25], [0, 180], accumulate=False)
            # normalize histogram and apply backprojection
            cv2.normalize(roihist,roihist,0,255,cv2.NORM_MINMAX)
            dst = cv2.calcBackProject([hsvt],[0,1],roihist,[0,180,0,256],1)
            #dst = cv2.calcBackProject([hsvt],[0],roihist,[0,180],1)
            # Now convolute with circular disc
            disc = cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(5,5))
            cv2.filter2D(dst,-1,disc,dst)
            # threshold and binary AND
            ret,thresh = cv2.threshold(dst,50,255,0)
            thresh = cv2.merge((thresh,thresh,thresh))
            og = cv2.bitwise_and(img,thresh)
            og=cv2.resize(og,(og_shape[0],og_shape[1]))
            cv2.imshow("og",og)
            # DRAW HISTOGRAM
            #w = 400
            #h = 400
            #bin_w = int(round(400 / 180))
            #histImg = np.zeros((400, 400, 3), dtype=np.uint8)
            #for i in range(180):
            #    cv2.rectangle(histImg, (i*bin_w, 400), ( (i+1)*bin_w, 400 - int(round( roihist[i]*400/255.0 )) ), (0, 0, 255), cv2.FILLED)
            #cv2.imshow('Hue', histImg)
            #fig  = plt.figure()
            
            #plt.hist(roihist[:,0])
            #fig.draw()
            #plt.pause(0.0002)
            #fig.clf()
        elif args.method == 'mog2' or args.method == 'knn':
            newMask = backSub.apply(img, learningRate = 0.01)
            cv2.imshow("newmask",newMask) 
        elif args.method == 'grabcut':
            bgdModel = np.zeros((1,65),np.float64)
            fgdModel = np.zeros((1,65),np.float64)
            '''mask = np.zeros(img.shape[:2],np.uint8)
            zeros = np.where(LabelPred_org==0)
            min_x = np.min(zeros[0])
            min_y = np,min(zeros[1])
            rect = (min_x,min_y,img.shape[0] - min_x,np.max(zeros[1]))'''
            mask = LabelPred_org.copy()
            mask[mask==0] = cv2.GC_BGD
            mask[mask==1] = cv2.GC_FGD
            if np.sum(mask) > 0:
                mask, bgdModel, fgdModel = cv2.grabCut(img,mask,None,bgdModel,fgdModel,5,cv2.GC_INIT_WITH_MASK)
                mask[mask==cv2.GC_FGD] = 255
                #mask2 = np.where((mask==2)|(mask==0),0,1).astype('uint8')
                #img = img*mask2[:,:,np.newaxis]
                #cv2.rectangle(img, (min_x,min_y), (img.shape[0],np.max(zeros[1])))
                cv2.imshow("img",mask)
        cv2.imshow("overlay",imgToDisplay)
        end = time.time()
        elapsedClock = count_end() - start
        print(f'elapsed cycles: {elapsedClock}')
        fps = 1/(end - start)
        print("Estimated frames per second : {0}".format(fps))
        #print(networkOutput.shape)
        totalElapsedCycles += elapsedClock
        totalFPS += fps
        fim += 1
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
