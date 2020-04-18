# Train fully convolutional neural net for sematic segmentation
#Benjamin Pikus, Rahul Yerrabelli
#July 2019
#MoTrack Therapy

# command line arguments
import argparse

parser = argparse.ArgumentParser(description='Train network to identify hand vs not hand')
parser.add_argument('--model_name', default="MobileV1_quickdeconv",
                    help='Which model to use (needs to be have following files:' + 
                        'transferLearnModel/<NAME>.npy,scripts/modelUtil/BuildNet<NAME>.py', required=False)
parser.add_argument('--max_iteration', default=50000, type=int,
                    help='The maximum number of iterations before quiting', required=False)
parser.add_argument('--save_interval', default=50, type=int, help='How often (in units of iteration) should the model and data be saved', required=False)
parser.add_argument('--use_gpu', default='cpu', type=str, help='Should you use gpu or cpu', required=False)
parser.add_argument('--batch_size', default=4, type=int,
                    help='The batch size to use for training (number of images per iterations)', required=False)
parser.add_argument('--learning_rate', default=1e-5, type=float,
                    help='Learning rate to use for model (default for Adam optimizer is 1e-5)', required=False)
parser.add_argument('--rows', default=-1, type=int,
                    help='What to resize the input rows to (if -1, then its None)', required=False)
parser.add_argument('--cols', default=-1, type=int,
                    help='What to resize the input cols to (if -1, then its None)', required=False)
parser.add_argument('--dir', default='', type=str, help='Directory to save in in training logs', required=False)
parser.add_argument('--dontuseval', help='If flag is present, wont use validation set ',
    action='store_false')
parser.add_argument('--partition', default=None, type=str, help='Partition file to use to only train on certain images', required=False)
parser.add_argument('--prune', help='If flag is present, will prune ',
    action='store_true')
parser.add_argument('--prune_itr', default=500, type=int,
                    help='At what iteration to prune (if prune flag is present)', required=False)
parser.add_argument('--prune_start', default=20000, type=int,
                    help='At what iteration to start prune (if prune flag is present)', required=False)
parser.add_argument('--prevmask_set', help='If flag is present, train using previous mask method for sets ',
    action='store_true')
parser.add_argument('--cropLarge', help='If flag is present, will crop to rectangle that is just label ',
    action='store_true')
parser.add_argument('--loss', default='ce', type=str, help='Loss function to use', required=False)
parser.add_argument('--addContourClass', type = int, default = -1, help='If addContourClas > 0, will add an extra contour class with the thickness specified')

args = parser.parse_args()

# imports
import tensorflow as tf
import numpy as np
import scipy.misc as misc
import sys
import os

from tensorflow.contrib.model_pruning.python import pruning
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
elif args.model_name =="MobileNetV1_unet_transpose":
    from modelUtils import BuildNetMobileV1_unet_transpose as BuildNet
elif args.model_name =="MobileNetV1_unet":
    from modelUtils import BuildNetMobileV1_unet as BuildNet
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
elif args.model_name =="MobileNetV1_lite":
    from modelUtils import BuildNetMobileV1_lite as BuildNet
    #from modelUtils import CheckModelMobileV1_lite as CheckNet
elif args.model_name =="MobileNetV1_2lite":
    from modelUtils import BuildNetMobileV1_2lite as BuildNet
elif args.model_name =="MobileNetV1_2lite_v2":
    from modelUtils import BuildNetMobileV1_2lite_v2 as BuildNet
elif args.model_name =="MobileNetV1_2lite_v3":
    from modelUtils import BuildNetMobileV1_2lite_v3 as BuildNet
elif args.model_name =="MobileNetV1_connected":
    from modelUtils import BuildNetMobileV1_connected as BuildNet
elif args.model_name =="MobileNetV1_lite_1fcn":
    from modelUtils import BuildNetMobileV1_lite_1fcn as BuildNet
elif args.model_name =="MobileNetV1_deep":
    from modelUtils import BuildNetMobileV1_deep as BuildNet
elif args.model_name =="MobileNetV1_lite_nobatchnorm":
    from modelUtils import BuildNetMobileV1_lite_nobatchnorm as BuildNet
    #from modelUtils import CheckModelMobileV1_lite_nobatchnorm as CheckNet]
elif args.model_name =="MobileNetV1_superlite":
    from modelUtils import BuildNetMobileV1_superlite as BuildNet
elif args.model_name =="MobileNetV1_superlite_smallkernel":
    from modelUtils import BuildNetMobileV1_superlite_smallkernel as BuildNet
elif args.model_name =="MobileNetV1_lite_upsampling_half":
    from modelUtils import BuildNetMobileV1_lite_upsampling_half as BuildNet
elif args.model_name =="MultiLayers":
    from modelUtils import BuildNetMultiLayers as BuildNet
elif args.model_name =="MobileNetV1_quickdeconv":
    from modelUtils import BuildNetMobileV1_quickdeconv as BuildNet
elif args.model_name =="MobileNetV1_quickdeconv2":
    from modelUtils import BuildNetMobileV1_quickdeconv2 as BuildNet
elif args.model_name =="MobileNetV1_quickdeconv3":
    from modelUtils import BuildNetMobileV1_quickdeconv3 as BuildNet
elif args.model_name =="MobileNetV1_fcn":
    from modelUtils import BuildNetMobileV1_fcn as BuildNet
elif args.model_name =="MobileNetV1_unet_transpose":
    from modelUtils import BuildNetMobileV1_unet_transpose as BuildNet
elif args.model_name =="MobileNetV1_unet_lite2_smooth":
    from modelUtils import BuildNetMobileV1_unet_lite2_smooth as BuildNet
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
elif args.model_name =="test":
    from modelUtils import test as BuildNet
elif args.model_name =="1x1":
    from modelUtils import BuildNet_1x1 as BuildNet
elif args.model_name =="unet":
    from modelUtils import BuildNet_Unet as BuildNet
#...........................................Input and output folders.................................................
Train_Image_Dir="../data/train/raw/" #"../data/1559216300.3407464/"#Data_Zoo/Materials_In_Vessels/Train_Images/" # Images and labels for training
Train_Label_Dir="../data/train/labeled/" #"Data_Zoo/Materials_In_Vessels/LiquidSolidLabels/"# Annotetion in png format for train images and validation images (assume the name of the images and annotation images are the same (but annotation is always png format))
UseValidationSet=args.dontuseval # do you want to use validation set in training
Valid_Image_Dir="../data/val/raw/" #"../data/1559216300.3407464/"#"Data_Zoo/Materials_In_Vessels/Test_Images_All/"# Validation images that will be used to evaluate training
Valid_Labels_Dir="../data/val/labeled/" #"../labeled_data/1559216300.3407464"#"Data_Zoo/Materials_In_Vessels/LiquidSolidLabels/"#  (the  Labels are in same folder as the training set)
model_dir ="../transferLearnModels/" 
partition_dir = "../data/partitions/" 
if args.prevmask_set:
    prevmask_set_dir = "../data/prevMask/fromSet_"
#model_file_name = args.model_name + ".npy"
#model_path=model_dir +model_file_name # "Path to pretrained vgg16 model for encoder"
if args.dir != '':
    logs_dir= "../training_logs/"+args.dir+"/" # "path to logs directory where trained model and information will be stored"
else:
    logs_dir= "../training_logs/"+args.model_name+"/" # "path to logs directory where trained model and information will be stored"
if not os.path.exists(logs_dir): os.makedirs(logs_dir)

learning_rate=args.learning_rate #Learning rate
#CheckNet.CheckNet(model_dir,model_file_name) # Check if pretrained vgg16 model avialable and if not try to download it

tf.compat.v1.logging.set_verbosity(tf.compat.v1.logging.ERROR)
#-----------------------------Other Paramaters------------------------------------------------------------------------
TrainLossTxtFile=logs_dir+"TrainLoss.txt" #Where train losses will be writen
ValidLossTxtFile=logs_dir+"ValidationLoss.txt"# Where validation losses will be writen
Batch_Size=args.batch_size # Number of files per training iteration
Weight_Loss_Rate=5e-4 # Weight for the weight decay loss function
MAX_ITERATION = args.max_iteration # Max number of training iteration
SAVE_INTERVAL = args.save_interval # Save data every how many iterations
if args.addContourClass > 0:
    NUM_CLASSES = 3
else:
    NUM_CLASSES = 2 # Number of class for fine grain +number of class for solid liquid+Number of class for empty none empty +Number of class for vessel background

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
        transforms = ["fold_constants(ignore_errors=True)"]
        if quantize:
            transforms += ["quantize_weights(minimum_size=0)"]
        transforms += ["sort_by_execution_order"]
        graph_def = TransformGraph(graph_def, [in_node], [out_node], transforms)
    # Serialize
    with tf.gfile.FastGFile(out_graph, 'wb') as f:
        f.write(graph_def.SerializeToString())
################################################################################################################################################################################
################################################################################################################################################################################
def main(argv=None):
    if args.use_gpu=='gpu':
        device_name ='/gpu:0'#"/job:localhost/replica:0/task:0/device:XLA_GPU:0" 
    else:
        device_name="cpu"
    with tf.device(device_name):
        tf.reset_default_graph()
        keep_prob= tf.placeholder(tf.float32, name="keep_probabilty") #Dropout probability
#.........................Placeholders for input image and labels...........................................................................................
        #if (args.rows == -1 and args.cols == -1):
        depth = 3
        if args.prevmask_set:
            depth = 4
        image = tf.placeholder(tf.float32, shape=[None, None, None, depth], name="input_image") #Input image batch first dimension image number second dimension width third dimension height 4 dimension RGB
        #else:
        #    image = tf.placeholder(tf.float32, shape=[None, args.rows, args.cols, 3], name="input_image") #Input image batch first dimension image number second dimension width third dimension height 4 dimension RGB
        
        GTLabel = tf.placeholder(tf.int32, shape=[None, None, None, 1], name="GTLabel")#Ground truth labels for training
  #.........................Build FCN Net...............................................................................................
        
        if not args.prevmask_set:      
            Net =  BuildNet.BUILD_NET(npy_dir=model_dir) #Create class for the network
        else:
            Net =  BuildNet.BUILD_NET() # prev masks don't have the pretrained encoder weights loaded
        Net.build(image, NUM_CLASSES,keep_prob)# Create the net and load intial weights
#......................................Get loss functions for neural net work  one loss function for each set of label....................................................................................................
        if args.loss == 'ce':
            Loss = tf.reduce_mean((tf.nn.sparse_softmax_cross_entropy_with_logits(labels=tf.squeeze(GTLabel, squeeze_dims=[3]), logits=Net.Prob,name="Loss")))  # Define loss function for training
        #elif args.loss == 'focal':
        #    from trainUtils.custom_loss import focal_loss
        #    Loss = focal_loss(labels=tf.squeeze(GTLabel, squeeze_dims=[3]), logits=Net.Prob,name="Loss")
    #....................................Create solver for the net............................................................................................
        trainable_var = tf.trainable_variables() # Collect all trainable variables for the net
        train_op = train(Loss, trainable_var) #Create Train Operation for the net
        if args.prune:
            global_step = tf.train.get_or_create_global_step()
            # Get, Print, and Edit Pruning Hyperparameters
            pruning_hparams = pruning.get_pruning_hparams()
            
            # Change hyperparameters to meet our needs
            pruning_hparams.begin_pruning_step = 0
            pruning_hparams.end_pruning_step = 250
            pruning_hparams.pruning_frequency = 1
            pruning_hparams.sparsity_function_end_step = 250
            pruning_hparams.target_sparsity = .9
            # Create a pruning object using the pruning specification, sparsity seems to have priority over the hparam
            p = pruning.Pruning(pruning_hparams, global_step=global_step, sparsity=.9)
            prune_op = p.conditional_mask_update_op()
#----------------------------------------Create reader for data set--------------------------------------------------------------------------------------------------------------
    prevSetFile_train = None
    prevSetFile_val = None
    prevSetFile_test = None
    if args.prevmask_set:
        prevSetFile_train = prevmask_set_dir + "train.txt"
        prevSetFile_val = prevmask_set_dir + "val.txt"
        prevSetFile_test = prevmask_set_dir + "test.txt"
    if args.partition is not None:
        partition = []
        with open(partition_dir+args.partition + ".txt") as fp:
            for line in fp:
                partition.append( line.strip())
        TrainReader = Data_Reader.Data_Reader(Train_Image_Dir,  GTLabelDir=Train_Label_Dir,
                                              BatchSize=Batch_Size, partition=partition, prevMask_SetFile = prevSetFile_train) #Reader for training data
    else:
        TrainReader = Data_Reader.Data_Reader(Train_Image_Dir,  GTLabelDir=Train_Label_Dir,
                                              BatchSize=Batch_Size, prevMask_SetFile = prevSetFile_train) #Reader for training data
    if UseValidationSet:
        ValidReader = Data_Reader.Data_Reader(Valid_Image_Dir,  GTLabelDir=Valid_Labels_Dir,BatchSize=Batch_Size, prevMask_SetFile = prevSetFile_val) # Reader for validation data
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
#--------------------------- Create files for saving loss----------------------------------------------------------------------------------------------------------
    f = open(TrainLossTxtFile, "a")
    f.write("Iteration\tloss\t Learning Rate="+str(learning_rate))
    f.close()
    if UseValidationSet:
       f = open(ValidLossTxtFile, "a")
       f.write("Iteration\tloss\t Learning Rate=" + str(learning_rate))
       f.close()
#..............Start Training loop: Main Training....................................................................
    for itr in range(last_itr+1,MAX_ITERATION+1):
        Images,  GTLabels =TrainReader.ReadAndAugmentNextBatch(args.rows,args.cols, cropLarge = args.cropLarge, addContourClass = args.addContourClass) # Load  augmeted images and ground true labels for training
        feed_dict = {image: Images,GTLabel:GTLabels, keep_prob: 0.5}
        if args.prune and itr%args.prune_itr==0 and itr >= args.prune_start:
            sess.run(prune_op)
        sess.run(train_op, feed_dict=feed_dict) # Train one cycle
# --------------Save trained model------------------------------------------------------------------------------------------------------------------------------------------
        if itr % SAVE_INTERVAL == 0 and itr>0:
            print("Saving model to file in "+logs_dir)
            #tf.saved_model.simple_save(sess,'saver/',inputs={"input_image": image},outputs={"PRed": Net.Pred})
            saver.save(sess, logs_dir + "model.ckpt", itr) #Save model
            tf.train.write_graph(sess.graph.as_graph_def(), '.', logs_dir+'tensorflowModel.pbtxt', as_text=True)
            #freeze_graph.freeze_graph(logs_dir+'tensorflowModel.pbtxt', "", False, logs_dir + "model.ckpt-"+str(itr), "Pred", "save/restore_all", "save/Const:0",  logs_dir+ 'tensorflowModel.pb', True, ""    )
            #tf.train.write_graph(sess.graph.as_graph_def(), '.', logs_dir+'tensorflowModel.pbt', as_text=False)
            inp = image
            out = Net.Pred
            name = 'deconvolution'
            prepare_for_dnn(sess, sess.graph.as_graph_def(), inp.name[:inp.name.rfind(':')],
                    out.name[:out.name.rfind(':')], logs_dir+name + '_net.pb', inp.dtype,
                    optimize = False, quantize = False)
            '''with tf.gfile.FastGFile(logs_dir + name + '_net.pb', 'rb') as f:
                graph_def = tf.GraphDef()
                graph_def.ParseFromString(f.read())
                for node in graph_def.node:
                    #print(node)
                    if 'value' in node.attr:
                        halfs = node.attr["value"].tensor.half_val
                        if not node.attr["value"].tensor.tensor_content and halfs:
                            node.attr["value"].tensor.tensor_content = struct.pack('H' * len(halfs), *halfs)
                            node.attr["value"].tensor.ClearField('half_val')'''
            tf.train.write_graph(sess.graph_def, "", logs_dir + 'tensorflowModel.pb', as_text=False)
                    
#......................Write and display train loss..........................................................................
        if itr % 10 == 0:
            # Calculate train loss
            feed_dict = {image: Images, GTLabel: GTLabels, keep_prob: 1}
            TLoss=sess.run(Loss, feed_dict=feed_dict)
            print("Iteration="+str(itr)+", Train Loss="+str(TLoss))
            if itr % SAVE_INTERVAL and itr>0:
                #Write train loss to file
                with open(TrainLossTxtFile, "a") as f:
                    f.write("\n"+str(itr)+"\t"+str(TLoss))
                    f.close()
#......................Write and display Validation Set Loss by running loss on all validation images.....................................................................
        if UseValidationSet and itr % 500 == 0:
            SumLoss=np.float64(0.0)
            NBatches=np.int(np.ceil(ValidReader.NumFiles/ValidReader.BatchSize))
            print("Calculating Validation on " + str(ValidReader.NumFiles) + " Images")
            for i in range(NBatches):# Go over all validation image
                Images, GTLabels= ValidReader.ReadNextBatchClean(args.rows,args.cols, cropLarge = args.cropLarge) # load validation image and ground true labels
                feed_dict = {image: Images,GTLabel: GTLabels ,keep_prob: 1.0}
                # Calculate loss for all labels set
                TLoss = sess.run(Loss, feed_dict=feed_dict)
                SumLoss+=TLoss
                NBatches+=1
            SumLoss/=NBatches
            print("Validation Loss: "+str(SumLoss))
            if itr>0:
                with open(ValidLossTxtFile, "a") as f:
                    f.write("\n" + str(itr) + "\t" + str(SumLoss))
                    f.close()

##################################################################################################################################################
main() #Run script
print("Finished!")
