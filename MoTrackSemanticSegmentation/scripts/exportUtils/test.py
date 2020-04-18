# command line arguments
import argparse
parser = argparse.ArgumentParser(description='Train network to identify hand vs not hand')
parser.add_argument('--model_name', default="Vgg16",
                    help='Which model to use (needs to be have following files:' + 
                        'transferLearnModel/<NAME>.npy,scripts/modelUtil/BuildNet<NAME>.py', required=False)

args = parser.parse_args()
import sys
sys.path.append('..')   # Adds higher directory to python modules path.
import tensorflow as tf
from trainUtils import Data_Reader
# script for building model
if args.model_name =="Vgg16":
    from modelUtils import BuildNetVgg16 as BuildNet

NUM_CLASSES=2
model_dir ="../../transferLearnModels/" 
model_file_name = args.model_name + ".npy"
model_path=model_dir +model_file_name # "Path to pretrained vgg16 model for encoder"
logs_dir= "../../training_logs/"+args.model_name+"/"# "path to logs directory where trained model and information will be stored"

tf.reset_default_graph()
keep_prob= tf.placeholder(tf.float32, name="keep_probabilty") #Dropout probability
#.........................Placeholders for input image and labels...........................................................................................
image = tf.placeholder(tf.float32, shape=[None, None, None, 3], name="input_image") #Input image batch first dimension image number second dimension width third dimension height 4 dimension RGB
#GTLabel = tf.placeholder(tf.int32, shape=[None, None, None, 1], name="Pred")#Ground truth labels for training
#.........................Build FCN Net...............................................................................
Net =  BuildNet.BUILD_NET(npy_path=model_path) #Create class for the network
Net.build(image, NUM_CLASSES,keep_prob)# Create the net and load intial weights
#......................................Get loss functions for neural net work  one loss function for each set of label....................................................................................................
#Loss = tf.reduce_mean((tf.nn.sparse_softmax_cross_entropy_with_logits(labels=tf.squeeze(GTLabel, squeeze_dims=[3]), logits=Net.Prob,name="Loss")))  # Define loss function for training
#....................................Create solver for the net............................................................................................
#trainable_var = tf.trainable_variables() # Collect all trainable variables for the net
#train_op = train(Loss, trainable_var) #Create Train Operation for the net
#----------------------------------------Create reader for data set--------------------------------------------------------------------------------------------------------------
#TrainReader = Data_Reader.Data_Reader(Train_Image_Dir,  GTLabelDir=Train_Label_Dir,BatchSize=Batch_Size) #Reader for training data
#sess = tf.Session() #Start Tensorflow session
# -------------load trained model if exist-----------------------------------------------------------------    

#print("Setting up Saver...")
#saver = tf.train.Saver()
#sess.run(tf.global_variables_initializer()) #Initialize variables
#ckpt = tf.train.get_checkpoint_state(logs_dir)
#if ckpt and ckpt.model_checkpoint_path: # if train model exist restore it
#    saver.restore(sess, ckpt.model_checkpoint_path)
#    print("Model restored...")
#    converter = tf.lite.TFLiteConverter.from_session(sess, [image], [Net.Pred])#[GTLabel])
#    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS,tf.lite.OpsSet.SELECT_TF_OPS]
converter = tf.lite.TFLiteConverter.from_frozen_graph("frozen_graph.pb", ["input_image"], ["Pred"])
tflite_model = converter.convert()
open("converted_model.tflite", "wb").write(tflite_model)
