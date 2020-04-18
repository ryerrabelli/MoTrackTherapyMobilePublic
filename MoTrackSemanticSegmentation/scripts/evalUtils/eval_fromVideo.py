# Run prediction and genertae pixelwise annotation for every pixels in the testing images using fully coonvolutional neural net

#--------------------------------------------------------------------------------------------------------------------

# command line arguments
import argparse

parser = argparse.ArgumentParser(description='Evaluate how network did to identify hand vs not hand')
parser.add_argument('--model_name', default="Vgg16",
                    help='Which model to use (needs to be have following files:' + 
                        'transferLearnModel/<NAME>.npy,scripts/modelUtil/BuildNet<NAME>.py', required=False)

args = parser.parse_args()

import tensorflow as tf
import numpy as np
import scipy.misc as misc
import sys
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
logs_dir= "../training_logs/" + args.model_name + "/"# "path to logs directory where trained model and information will be stored"
#Image_Dir="../data/test/raw/"# Test image folder
#Label_Dir="../data/test/labeled/"# Test label fodler
w=0.6# weight of overlay on image
Pred_Dir="../Output_Prediction/Webcam/"+args.model_name+"/" # Library where the output prediction will be written
model_dir ="../transferLearnModels/" 
#model_file_name = args.model_name + ".npy"
#model_path=model_dir +model_file_name # "Path to pretrained vgg16 model for encoder"
NameEnd="" # Add this string to the ending of the file name optional
NUM_CLASSES = 2 # Number of classes
#-------------------------------------------------------------------------------------------------------------------------
#CheckNet.CheckNet(model_dir,model_file_name)# Check if pretrained vgg16 model available and if not try to download it

################################################################################################################################################################################
def main(argv=None):
    # .........................Placeholders for input image and labels........................................................................
    keep_prob = tf.placeholder(tf.float32, name="keep_probabilty")  # Dropout probability
    image = tf.placeholder(tf.float32, shape=[1, 128, 96, 3], name="input_image")  # Input image batch first dimension image number second dimension width third dimension height 4 dimension RGB

    # -------------------------Build Net----------------------------------------------------------------------------------------------
    Net =  BuildNet.BUILD_NET(npy_dir=model_dir) # Create class instance for the net
    Net.build(image, NUM_CLASSES, keep_prob)  # Build net and load intial weights (weights before training)
    # -------------------------Data reader for validation/testing images-----------------------------------------------------------------------------------------------------------------------------
    #ValidReader = Data_Reader.Data_Reader(Image_Dir, GTLabelDir=Label_Dir, BatchSize=1)
#......................................Get loss functions for neural net work  one loss function for each set of label....................................................................................................
    GTLabel = tf.placeholder(tf.int32, shape=[None, None, None, 1], name="GTLabel")#Ground truth labels for training
    Loss = tf.reduce_mean((tf.nn.sparse_softmax_cross_entropy_with_logits(labels=tf.squeeze(GTLabel, squeeze_dims=[3]), logits=Net.Prob,name="Loss")))  # Define loss function for training
    #-------------------------Load Trained model if you dont have trained model see: Train.py-----------------------------------------------------------------------------------------------------------------------------

    sess = tf.Session() #Start Tensorflow session

    print("Setting up Saver...")
    saver = tf.train.Saver()

    sess.run(tf.global_variables_initializer())
    ckpt = tf.train.get_checkpoint_state(logs_dir)
    if ckpt and ckpt.model_checkpoint_path: # if train model exist restore it
        saver.restore(sess, ckpt.model_checkpoint_path)
        print("Model restored...")
    else:
        print("ERROR NO TRAINED MODEL IN: "+ckpt.model_checkpoint_path+" See Train.py for creating train network ")
        sys.exit()

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
    while (True):#(ValidReader.itr < ValidReader.NumFiles):
        #print(str(fim * 100.0 / ValidReader.NumFiles) + "%")
        fim += 1
        
        # ..................................Load image.......................................................................................
        #FileName=ValidReader.OrderedFiles[ValidReader.itr] #Get input image name
        #Images,GTLabels = ValidReader.ReadNextBatchClean()  # load testing image
        ret,img=cap.read()
        img = cv2.resize(img,(96,128))
        Images = np.zeros([1,img.shape[0],img.shape[1],3], dtype=np.float)
        Images[0] = img
        # Predict annotation using net
        LabelPred = sess.run(Net.Pred, feed_dict={image: Images, keep_prob: 1.0})
        # Calculate test loss
        #TLoss=sess.run(Loss, feed_dict={image: Images, GTLabel:GTLabels , keep_prob:1.0})

        #------------------------Calculate some values on the images---------------------------------------------------------------------------------------------
        testImage = Images[0]
        testImageHt = testImage.shape[0]
        testImageWd = testImage.shape[1]
        testImageChannelCt = testImage.shape[2] if len(testImage.shape)>2 else 1
        testImageMaskCt = np.count_nonzero(testImage) #cv2.countNonZero(testImage) requires single channel image

        predImage =  LabelPred[0]
        predImageHt = predImage.shape[0]
        predImageWd = predImage.shape[1]
        predImageChannelCt = predImage.shape[2] if len(predImage.shape)>2 else 1
        predImageMaskCt = np.count_nonzero(predImage) #cv2.countNonZero(predImage) requires single channel image

        #------------------------Save predicted labels overlay on images---------------------------------------------------------------------------------------------
        overlay_img = Overlay.OverLayLabelOnImage(testImage,predImage, w)
        print(Net.Prob)
        print(Net.Pred)
        
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


##################################################################################################################################################


main()#Run script
print("Finished")
