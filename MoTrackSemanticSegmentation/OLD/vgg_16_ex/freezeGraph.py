#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Jun  8 15:02:22 2019

@author: benka
"""
import tensorflow as tf
from tensorflow.python.tools import freeze_graph
import BuildNetVgg16
logs_dir= "logs/"# "path to logs directory where trained model and information is stored"

model_path="Model_Zoo/vgg16.npy"# "Path to pretrained vgg16 model for encoder"
NUM_CLASSES = 2 # Number of classes
keep_prob = tf.placeholder(tf.float32, name="keep_probabilty")  # Dropout probability
image = tf.placeholder(tf.float32, shape=[None, None, None, 3], name="input_image")  # Input image batch first dimension image number second dimension width third dimension height 4 dimension RGB

 # -------------------------Build Net----------------------------------------------------------------------------------------------
Net = BuildNetVgg16.BUILD_NET_VGG16(vgg16_npy_path=model_path)  # Create class instance for the net
Net.build(image, NUM_CLASSES, keep_prob)  # Build net and load intial weights (weights before training)

freeze_graph.freeze_graph(logs_dir + 'tensorflowModel.pbtxt', "", False, 
                          tf.train.latest_checkpoint(logs_dir), "Pred",#Net.Pred,
                           "save/restore_all", "save/Const:0",
                           'models/frozentensorflowModel.pb', True, ""  
                         )