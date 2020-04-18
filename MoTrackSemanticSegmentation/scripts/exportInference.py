# Train fully convolutional neural net for sematic segmentation
#Benjamin Pikus, Rahul Yerrabelli
#July 2019
#MoTrack Therapy

# command line arguments
import argparse

parser = argparse.ArgumentParser(description='Train network to identify hand vs not hand')
parser.add_argument('--model_name', default="Vgg16",
                    help='Which model to use (needs to be have following files:' + 
                        'transferLearnModel/<NAME>.npy,scripts/modelUtil/BuildNet<NAME>.py', required=False)
parser.add_argument('--use_gpu', default='cpu', type=str, help='Should you use gpu or cpu', required=False)
parser.add_argument('--dontquantize', help='If flag is present, wont quantize',
    action='store_false')
parser.add_argument('--dir', default='', type=str, help='Directory to save in in training logs', required=False)
parser.add_argument('--addContourClass', type = int, default = -1, help='If addContourClas > 0, will add an extra contour class with the thickness specified')

args = parser.parse_args()

# imports
import tensorflow as tf
import numpy as np
import scipy.misc as misc
import sys
import os
import struct
from trainUtils import Data_Reader
from tensorflow.python.tools import freeze_graph

from tensorflow.python.tools import optimize_for_inference_lib
from tensorflow.tools.graph_transforms import TransformGraph

# import script for building model
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
elif args.model_name =="MobileNetV1_2lite":
    from modelUtils import BuildNetMobileV1_2lite as BuildNet
    #from modelUtils import CheckModelMobileV1_lite as CheckNet
elif args.model_name =="MobileNetV1_deep":
    from modelUtils import BuildNetMobileV1_deep as BuildNet
elif args.model_name =="MobileNetV1_lite_nobatchnorm":
    from modelUtils import BuildNetMobileV1_lite_nobatchnorm as BuildNet
    #from modelUtils import CheckModelMobileV1_lite_nobatchnorm as CheckNet]
elif args.model_name =="MobileNetV1_superlite":
    from modelUtils import BuildNetMobileV1_superlite as BuildNet
elif args.model_name =="MultiLayers":
    from modelUtils import BuildNetMultiLayers as BuildNet

elif args.model_name =="MobileNetV1_lite_upsampling_half":
    from modelUtils import BuildNetMobileV1_lite_upsampling_half as BuildNet
elif args.model_name =="test":
    from modelUtils import test as BuildNet
#...........................................Input and output folders.................................................
Train_Image_Dir="../data/train/raw/" #"../data/1559216300.3407464/"#Data_Zoo/Materials_In_Vessels/Train_Images/" # Images and labels for training
Train_Label_Dir="../data/train/labeled/" #"Data_Zoo/Materials_In_Vessels/LiquidSolidLabels/"# Annotetion in png format for train images and validation images (assume the name of the images and annotation images are the same (but annotation is always png format))
UseValidationSet=True # do you want to use validation set in training
Valid_Image_Dir="../data/val/raw/" #"../data/1559216300.3407464/"#"Data_Zoo/Materials_In_Vessels/Test_Images_All/"# Validation images that will be used to evaluate training
Valid_Labels_Dir="../data/val/labeled/" #"../labeled_data/1559216300.3407464"#"Data_Zoo/Materials_In_Vessels/LiquidSolidLabels/"#  (the  Labels are in same folder as the training set)
model_dir ="../transferLearnModels/" 
#model_file_name = args.model_name + ".npy"
#model_path=model_dir +model_file_name # "Path to pretrained vgg16 model for encoder"
if args.dir != '':
    logs_dir= "../training_logs/"+args.dir+"/" # "path to logs directory where trained model and information will be stored"
else:
    logs_dir= "../training_logs/"+args.model_name+"/" # "path to logs directory where trained model and information will be stored"
if not os.path.exists(logs_dir): assert("No Log directory: " + logs_dir )

#CheckNet.CheckNet(model_dir,model_file_name) # Check if pretrained vgg16 model avialable and if not try to download it

tf.compat.v1.logging.set_verbosity(tf.compat.v1.logging.ERROR)
#-----------------------------Other Paramaters------------------------------------------------------------------------
TrainLossTxtFile=logs_dir+"TrainLoss.txt" #Where train losses will be writen
ValidLossTxtFile=logs_dir+"ValidationLoss.txt"# Where validation losses will be writen

if args.addContourClass > 0:
    NUM_CLASSES = 3
else:
    NUM_CLASSES = 2 # Number of classes
######################################Solver for model training#####################################################################################################################
def train(loss_val, var_list):
    optimizer = tf.train.AdamOptimizer(learning_rate)
    grads = optimizer.compute_gradients(loss_val, var_list=var_list)
    return optimizer.apply_gradients(grads)

def prepare_for_dnn(sess, graph_def, in_node, out_node, out_graph, dtype, optimize=True, quantize=False):
    # Freeze graph. Replaces variables to constants.
    graph_def = tf.graph_util.convert_variables_to_constants(sess, graph_def, [out_node])
    if optimize:
        # Optimize graph. Removes training-only ops, unused nodes.
        graph_def = optimize_for_inference_lib.optimize_for_inference(graph_def, [in_node], [out_node], dtype.as_datatype_enum)
        # Fuse constant operations.
        transforms = ["fold_constants(ignore_errors=True)"]#,
                      #'remove_nodes(op=Identity)',
                      #  'merge_duplicate_nodes',
                      # 'strip_unused_nodes',
                      #  'fold_batch_norms',
                       # 'fold_old_batch_norms',
                       #'round_weights']
        if quantize:
            transforms += ["quantize_weights(minimum_size=0)"]
            #transforms += ["quantize_weights"]
        transforms += ["sort_by_execution_order"]
        graph_def = TransformGraph(graph_def, [in_node], [out_node], transforms)
    # Serialize
    with tf.gfile.FastGFile(out_graph, 'wb') as f:
        f.write(graph_def.SerializeToString())
################################################################################################################################################################################
################################################################################################################################################################################
def main(argv=None):
    if args.use_gpu=='gpu':
        device_name ="/job:localhost/replica:0/task:0/device:XLA_GPU:0" 
    else:
        device_name="cpu"
    with tf.device(device_name):
        tf.reset_default_graph()
        keep_prob= tf.placeholder(tf.float32, name="keep_probabilty") #Dropout probability
#.........................Placeholders for input image and labels...........................................................................................
        image = tf.placeholder(tf.float32, shape=[1, 128, 96, 3], name="input_image") #Input image batch first dimension image number second dimension width third dimension height 4 dimension RGB
        GTLabel = tf.placeholder(tf.int32, shape=[None, None, None, 1], name="GTLabel")#Ground truth labels for training
  #.........................Build FCN Net...............................................................................................
        Net =  BuildNet.BUILD_NET(npy_dir=model_dir) #Create class for the network
        Net.build(image, NUM_CLASSES,keep_prob, is_training = False)# Create the net and load intial weights
   #....................................Create solver for the net............................................................................................
 #----------------------------------------Create reader for data set--------------------------------------------------------------------------------------------------------------
    sess = tf.Session() #Start Tensorflow session
# -------------load trained model if it exists-----------------------------------------------------------------
    print("Setting up model saver...")
    saver = tf.train.Saver()
    sess.run(tf.global_variables_initializer()) #Initialize variables
    ckpt = tf.train.get_checkpoint_state(logs_dir)
    if ckpt and ckpt.model_checkpoint_path: # if train model exist restore it
        saver.restore(sess, ckpt.model_checkpoint_path)
        last_itr = int(ckpt.model_checkpoint_path.split("/")[-1].split("-")[-1])
        print("Model restored. Starting from iteration=" + str(last_itr) + ".")
    else:
        last_itr = 0
    print(device_name)
    inp = image
    out = Net.Prob
    name = 'deconvolution'
    prepare_for_dnn(sess, sess.graph.as_graph_def(), inp.name[:inp.name.rfind(':')],
            out.name[:out.name.rfind(':')], logs_dir+name + '_net.pb', inp.dtype,
            optimize = True, quantize = args.dontquantize)
    with tf.gfile.FastGFile(logs_dir + name + '_net.pb', 'rb') as f:
        graph_def = tf.GraphDef()
        graph_def.ParseFromString(f.read())
        for node in graph_def.node:
            #print(node)
            if 'value' in node.attr:
                halfs = node.attr["value"].tensor.half_val
                if not node.attr["value"].tensor.tensor_content and halfs:
                    node.attr["value"].tensor.tensor_content = struct.pack('H' * len(halfs), *halfs)
                    node.attr["value"].tensor.ClearField('half_val')
        tf.train.write_graph(graph_def, "", logs_dir + 'tensorflowModel.pb', as_text=False)
         
##################################################################################################################################################
main() #Run script
print("Finished!")
