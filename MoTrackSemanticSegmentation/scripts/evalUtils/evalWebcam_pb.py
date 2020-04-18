# Run prediction and genertae pixelwise annotation for every pixels in the testing images using fully coonvolutional neural net

#--------------------------------------------------------------------------------------------------------------------

# command line arguments
import argparse
import tensorflow as tf
from tensorflow.python.platform import gfile
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
from trainUtils import Data_Reader
from evalUtils import OverrlayLabelOnImage as Overlay
import datetime
logs_dir= "../training_logs/" + args.model_name + "/"# "path to logs directory where trained model and information will be stored"
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
    with tf.Graph().as_default() as graph: # Set default graph as graph
           with tf.Session() as sess:
                # Load the graph in graph_def
                print("load graph")
                # We load the protobuf file from the disk and parse it to retrive the unserialized graph_drf
                with gfile.FastGFile(logs_dir+'tensorflowModel.pb','rb') as f:
                    # Set FCN graph to the default graph
                    graph_def = tf.GraphDef()
                    graph_def.ParseFromString(f.read())
                    sess.graph.as_default()
                    
                    tf.import_graph_def(
                                graph_def,
                                input_map=None,
                                return_elements=None,
                                name="",
                                op_dict=None,
                                producer_op_list=None
                                )

                    # Print the name of operations in the session
                    for op in graph.get_operations():
                            print("Operation Name :",op.name)        # Operation name
                            print("Tensor Stats :",str(op.values()))     # Tensor name
                    
                    # INFERENCE Here
                    l_input = graph.get_tensor_by_name('input_image:0') # Input Tensor
                    #l_keep_prob = graph.get_tensor_by_name('keep_probabilty:0')
                    l_output = graph.get_tensor_by_name('Prob:0') # Output Tensor

                    #initialize_all_variables
                    #tf.global_variables_initializer()
                    sess.run(tf.global_variables_initializer())
                    

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
                        Images = np.zeros([1,img.shape[0],img.shape[1],3], dtype=np.float)
                        Images[0] = img
                        # Predict annotation using net
                        # Run  model on image
                        
                        LabelPred = sess.run( l_output, feed_dict = {l_input : Images})#, l_keep_prob : 1.0} )
                        # Calculate test loss
                        #TLoss=sess.run(Loss, feed_dict={image: Images, GTLabel:GTLabels , keep_prob:1.0})
                
                        #------------------------Calculate some values on the images---------------------------------------------------------------------------------------------
                        testImage = img
                        
                        predImage =  LabelPred[0]
                        
                        #------------------------Save predicted labels overlay on images---------------------------------------------------------------------------------------------
                        overlay_img = Overlay.OverLayLabelOnImage(testImage,predImage, w)
                        
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
