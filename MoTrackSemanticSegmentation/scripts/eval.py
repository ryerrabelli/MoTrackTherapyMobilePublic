# Run prediction and genertae pixelwise annotation for every pixels in the testing images using fully coonvolutional neural net

#--------------------------------------------------------------------------------------------------------------------

# command line arguments
import argparse

parser = argparse.ArgumentParser(description='Evaluate how network did to identify hand vs not hand')
parser.add_argument('--model_name', default="Vgg16",
                    help='Which model to use (needs to be have following files:' + 
                        'transferLearnModel/<NAME>.npy,scripts/modelUtil/BuildNet<NAME>.py', required=False)
parser.add_argument('--rows', default=-1, type=int,
                    help='What to resize the input rows to (if -1, then its None)', required=False)
parser.add_argument('--cols', default=-1, type=int,
                    help='What to resize the input cols to (if -1, then its None)', required=False)
parser.add_argument('--dir', default=None, type=str, help='Directory to save in in output predictions', required=False)
parser.add_argument('--prevmask_set', help='If flag is present, eval using previous mask method for sets ',
    action='store_true')
parser.add_argument('--cropLarge', help='If flag is present, will crop to rectangle that is just label ',
    action='store_true')
parser.add_argument('--addContourClass', type = int, default = -1, help='If addContourClas > 0, will add an extra contour class with the thickness specified')
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
elif args.model_name =="MobileNetV1_unet":
    from modelUtils import BuildNetMobileV1_unet as BuildNet
elif args.model_name =="MobileNetV1_unet_transpose":
    from modelUtils import BuildNetMobileV1_unet_transpose as BuildNet
elif args.model_name =="MobileNetV1_unet_activation":
    from modelUtils import BuildNetMobileV1_unet_activation as BuildNet
elif args.model_name =="MobileNetV1_unet_lite1":
    from modelUtils import BuildNetMobileV1_unet_lite1 as BuildNet
elif args.model_name =="MobileNetV1_unet_lite2":
    from modelUtils import BuildNetMobileV1_unet_lite2 as BuildNet
elif args.model_name =="MobileNetV1_unet_lite3":
    from modelUtils import BuildNetMobileV1_unet_lite3 as BuildNet
elif args.model_name =="MobileNetV1_unet_lite4":
    from modelUtils import BuildNetMobileV1_unet_lite4 as BuildNet
elif args.model_name =="MobileNetV1_unet_lite2_smooth":
    from modelUtils import BuildNetMobileV1_unet_lite2_smooth as BuildNet
elif args.model_name =="MobileNetV1_2lite_v2":
    from modelUtils import BuildNetMobileV1_2lite_v2 as BuildNet
elif args.model_name =="MobileNetV1_2lite_v3":
    from modelUtils import BuildNetMobileV1_2lite_v3 as BuildNet
elif args.model_name =="MobileNetV1_lite":
    from modelUtils import BuildNetMobileV1_lite as BuildNet
    #from modelUtils import CheckModelMobileV1_lite as CheckNet
elif args.model_name =="PrevMask1":
    from modelUtils import PrevMask1 as BuildNet
elif args.model_name =="PrevMask2":
    from modelUtils import PrevMask2 as BuildNet
elif args.model_name =="PrevMask3":
    from modelUtils import PrevMask3 as BuildNet
elif args.model_name =="PrevMask4":
    from modelUtils import PrevMask4 as BuildNet
elif args.model_name =="PrevMask4Modified":
    from modelUtils import PrevMask4Modified as BuildNet
elif args.model_name =="PrevMask_big":
    from modelUtils import PrevMask_big as BuildNet
elif args.model_name =="MobileNetV1_2lite":
    from modelUtils import BuildNetMobileV1_2lite as BuildNet
elif args.model_name =="MobileNetV1_connected":
    from modelUtils import BuildNetMobileV1_connected as BuildNet
elif args.model_name =="MobileNetV1_superlite":
    from modelUtils import BuildNetMobileV1_superlite as BuildNet
elif args.model_name =="MobileNetV1_superlite_smallkernel":
    from modelUtils import BuildNetMobileV1_superlite_smallkernel as BuildNet
elif args.model_name =="MobileNetV1_fcn":
    from modelUtils import BuildNetMobileV1_fcn as BuildNet
elif args.model_name =="unet":
    from modelUtils import BuildNet_Unet as BuildNet
elif args.model_name =="MobileNetV1_quickdeconv":
    from modelUtils import BuildNetMobileV1_quickdeconv as BuildNet
elif args.model_name == "MobileNetV1_quickdeconv2":
    from modelUtils import BuildNetMobileV1_quickdeconv2 as BuildNet
elif args.model_name == "MobileNetV1_quickdeconv3":
    from modelUtils import BuildNetMobileV1_quickdeconv3 as BuildNet
from trainUtils import TensorflowUtils
import os
import cv2
from trainUtils import Data_Reader
from evalUtils import OverrlayLabelOnImage as Overlay

if args.prevmask_set:
    prevmask_set_dir = "../data/prevMask/fromSet_"
Image_Dir="../data/test/raw/"# Test image folder
Label_Dir="../data/test/labeled/"# Test label fodler
w=0.6# weight of overlay on image
if args.dir is None:
    Pred_Dir="../Output_Prediction/"+args.model_name+"/" # Library where the output prediction will be written
    logs_dir= "../training_logs/" + args.model_name + "/"# "path to logs directory where trained model and information will be stored"
else:
    logs_dir= "../training_logs/" + args.dir + "/"# "path to logs directory where trained model and information will be stored"
    Pred_Dir="../Output_Prediction/"+args.dir+"/"
model_dir ="../transferLearnModels/" 
#model_file_name = args.model_name + ".npy"
#model_path=model_dir +model_file_name # "Path to pretrained vgg16 model for encoder"
NameEnd="" # Add this string to the ending of the file name optional
if args.addContourClass > 0:
    NUM_CLASSES = 3
else:
    NUM_CLASSES = 2 # Number of classes
#-------------------------------------------------------------------------------------------------------------------------
#CheckNet.CheckNet(model_dir,model_file_name)# Check if pretrained vgg16 model available and if not try to download it

################################################################################################################################################################################
def main(argv=None):
    # .........................Placeholders for input image and labels........................................................................
    keep_prob = tf.placeholder(tf.float32, name="keep_probabilty")  # Dropout probability
    #if args.rows == -1 and args.cols == -1:
    depth = 3
    if args.prevmask_set:
        depth = 4
    image = tf.placeholder(tf.float32, shape=[None, None, None, depth], name="input_image") #Input image batch first dimension image number second dimension width third dimension height 4 dimension RGB
    #else:
    #image = tf.placeholder(tf.float32, shape=[1, args.rows, args.cols, 3], name="input_image") #Input image batch first dimension image number second dimension width third dimension height 4 dimension RGB
    
    # -------------------------Build Net----------------------------------------------------------------------------------------------
    if not args.prevmask_set:      
        Net =  BuildNet.BUILD_NET(npy_dir=model_dir) #Create class for the network
    else:
        Net =  BuildNet.BUILD_NET() # prev masks don't have the pretrained encoder weights loaded
    Net.build(image, NUM_CLASSES, keep_prob)  # Build net and load intial weights (weights before training)
    # -------------------------Data reader for validation/testing images-----------------------------------------------------------------------------------------------------------------------------
    
    prevSetFile_test = None
    if args.prevmask_set:
        prevSetFile_test = prevmask_set_dir + "test.txt"
    ValidReader = Data_Reader.Data_Reader(Image_Dir, GTLabelDir=Label_Dir, BatchSize=1, prevMask_SetFile = prevSetFile_test)
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
    if not os.path.exists(Pred_Dir+"/OverLay"): os.makedirs(Pred_Dir+"/OverLay")
    if not os.path.exists(Pred_Dir + "/Label"): os.makedirs(Pred_Dir + "/Label")

    
    print("Running Predictions:")
    print("Saving output to:" + Pred_Dir)

    data = {"images" : {}} #will be put in a json file

#----------------------Go over all images and predict semantic segmentation in various of classes-------------------------------------------------------------
    fim = 0
    print("Start Predicting " + str(ValidReader.NumFiles) + " images")
    while (ValidReader.itr < ValidReader.NumFiles):
        print(str(fim * 100.0 / ValidReader.NumFiles) + "%")
        fim += 1
        # ..................................Load image.......................................................................................
        FileName=ValidReader.OrderedFiles[ValidReader.itr] #Get input image name
        if args.prevmask_set:
            FileName = FileName[0]
        Images,GTLabels = ValidReader.ReadNextBatchClean(args.rows,args.cols, cropLarge = args.cropLarge)  # load testing image

        # Predict annotation using net
        LabelPred = sess.run(Net.Pred, feed_dict={image: Images, keep_prob: 1.0})
        # Calculate test loss
        TLoss=sess.run(Loss, feed_dict={image: Images, GTLabel:GTLabels , keep_prob:1.0})

        #------------------------Calculate some values on the images---------------------------------------------------------------------------------------------
        testImage = Images[0][:,:,0:3]
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
        cv2.putText(overlay_img,str(round(TLoss,4)),(int(overlay_img.shape[1]*0.5),int(overlay_img.shape[0]*0.2)), cv2.FONT_HERSHEY_SIMPLEX, 0.5,(0,0,255),2,cv2.LINE_AA)
        misc.imsave(Pred_Dir + "/OverLay/"+ FileName+NameEnd, overlay_img ) #Overlay label on image
        misc.imsave(Pred_Dir + "/Label/" + FileName[:-4] + ".png" + NameEnd, LabelPred[0].astype(np.uint8))

        data["images"][FileName] = {
            "ct" : fim,
            "analysis" : {
                "loss" : TLoss,
            },
            "labelImage" : {
                "Ht" : testImageHt,
                "Wd" : testImageWd,
                "channelCt" : testImageChannelCt,
                "maskCt" : testImageMaskCt,
                "maskFrac" : testImageMaskCt*1.0/(testImageHt*testImageWd),
            },
            "predictedImage" : {
                "Ht" : predImageHt,
                "Wd" : predImageWd,
                "channelCt" : predImageChannelCt,
                "maskCt" : predImageMaskCt,
                "maskFrac" : predImageMaskCt*1.0/(predImageHt*predImageWd),
            },
            "fileName": FileName
        }

    SCRIPT_VERSION = 11
    from time import gmtime, strftime
    import time
    data["runParameters"] = {
        "language" : "python",
        "scriptName" : "eval.py",
        "scriptVersion" : SCRIPT_VERSION,
        "saveSuffix" : NameEnd,
        "timeOfRunningStr" : strftime("%y-%m-%d %H:%M:%S %z", gmtime()),
        "timeOfRunningEpoch" : time.time()
    }

    print(data)
    import json
    class MyEncoder(json.JSONEncoder): #Fixed numpy serialization issue per suggestion of https://stackoverflow.com/questions/27050108/convert-numpy-type-to-python
        def default(self, obj):
            if isinstance(obj, np.integer):
                return int(obj)
            elif isinstance(obj, np.floating):
                return float(obj)
            elif isinstance(obj, np.ndarray):
                return obj.tolist()
            else:
                return super(MyEncoder, self).default(obj)
    with open(Pred_Dir+"/OverLay/data_unanalyzed_ML.json", 'w') as f:
        json.dump(data, f, cls=MyEncoder)


##################################################################################################################################################


main()#Run script
print("Finished")
