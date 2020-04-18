#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Jun  8 15:02:22 2019

@author: benka
"""
import sys
sys.path.append('..')   # Adds higher directory to python modules path.
# command line arguments
import argparse
import os

parser = argparse.ArgumentParser(description='Evaluate how network did to identify hand vs not hand')
parser.add_argument('--model_name', default="Vgg16",
                    help='Which model to use (needs to be have following files:' + 
                        'transferLearnModel/<NAME>.npy,scripts/modelUtil/BuildNet<NAME>.py', required=False)

args = parser.parse_args()

import tensorflow as tf
from tensorflow.python.tools import freeze_graph
# script for building model
if args.model_name =="Vgg16":
    from modelUtils import BuildNetVgg16 as BuildNet
elif args.model_name =="MobileNetV1":
    from modelUtils import BuildNetMobileV1 as BuildNet
elif args.model_name =="MobileNetV1_lite":
    from modelUtils import BuildNetMobileV1_lite as BuildNet


logs_dir= "../../training_logs/"+args.model_name+"/" # "path to logs directory where trained model and information will be stored"

model_dir ="../../transferLearnModels/" 
model_file_name = args.model_name + ".npy"
model_path=model_dir +model_file_name # "Path to pretrained vgg16 model for encoder"

if not os.path.exists(args.model_name): os.makedirs(args.model_name)

NUM_CLASSES = 2 # Number of classes
keep_prob = tf.placeholder(tf.float32, name="keep_probabilty")  # Dropout probability
image = tf.placeholder(tf.float32, shape=[None, None, None, 3], name="input_image")  # Input image batch first dimension image number second dimension width third dimension height 4 dimension RGB

 # -------------------------Build Net----------------------------------------------------------------------------------------------
Net = BuildNet.BUILD_NET(npy_path=model_path) # Create class instance for the net
Net.build(image, NUM_CLASSES, keep_prob)  # Build net and load intial weights (weights before training)

freeze_graph.freeze_graph(logs_dir + 'tensorflowModel.pbtxt', "", False, 
                          tf.train.latest_checkpoint(logs_dir), "Pred",#Net.Pred,
                           "save/restore_all", "save/Const:0",
                           args.model_name + '/frozentensorflowModel.pb', True, ""  
                         )
