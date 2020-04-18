# Run prediction and genertae pixelwise annotation for every pixels in the testing images using fully coonvolutional neural net

#--------------------------------------------------------------------------------------------------------------------

# command line arguments
import argparse
import tensorflow as tf
from tensorflow.lite.python import interpreter as interpreter_wrapper
parser = argparse.ArgumentParser(description='Evaluate how network did to identify hand vs not hand')
parser.add_argument('--model_name', default="Vgg16",
                    help='Which model to use (needs to be have following files:' + 
                        'transferLearnModel/<NAME>.npy,scripts/modelUtil/BuildNet<NAME>.py', required=False)

args = parser.parse_args()

#import tensorflow as tf
import numpy as np
import scipy.misc as misc
import sys
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

input_mean = 0
input_std =  1
################################################################################################################################################################################
def main(argv=None):
    # setup tflite model
    model_file = logs_dir + "model.tflite"
    interpreter = interpreter_wrapper.Interpreter(model_path=model_file)
    interpreter.allocate_tensors()

    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    floating_model = False
    # check the type of the input tensor
    if input_details[0]['dtype'] == type(np.float32(1.0)):
        floating_model = True
    height = input_details[0]['shape'][1]
    width = input_details[0]['shape'][2]
    
    
    print(output_details)
    cap=cv2.VideoCapture(0)
    while (True):
        # ..................................Load image.......................................................................................
        #FileName=ValidReader.OrderedFiles[ValidReader.itr] #Get input image name
        #Images,GTLabels = ValidReader.ReadNextBatchClean()  # load testing image
        ret,img=cap.read()
        img = cv2.resize(img,(width, height))
        input_data = np.expand_dims(img, axis=0)
        if floating_model:
            input_data = (np.float32(input_data) - input_mean) / input_std
        interpreter.set_tensor(input_details[0]['index'], input_data)

        interpreter.invoke()
        output_data = interpreter.get_tensor(output_details[0]['index'])
        results = np.squeeze(output_data)
        img[np.where(results==1)] = [0,0,255]
        cv2.imshow("results",img)
        k = cv2.waitKey(1) & 0xff
        if k == 27: 
            break

    cap.release()
    cv2.destroyAllWindows()


##################################################################################################################################################


main()#Run script
print("Finished")
