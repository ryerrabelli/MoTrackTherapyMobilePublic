#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Jun  8 14:38:39 2019

@author: benka
"""

import tensorflow as tf
import Data_Reader
import BuildNetVgg16


converter = tf.lite.TFLiteConverter.from_saved_model("logs/")
tflite_model = converter.convert()
open("models/converted_model.tflite", "wb").write(tflite_model)
'''

image = tf.placeholder(tf.float32, shape=[None, None, None, 3], name="input_image")  # Input image batch first dimension image number second dimension width third dimension height 4 dimension RGB

model_path="Model_Zoo/vgg16.npy"# "Path to pretrained vgg16 model for encoder"
NUM_CLASSES = 2 # Number of classes
keep_prob = tf.placeholder(tf.float32, name="keep_probabilty")  # Dropout probability

 # -------------------------Build Net----------------------------------------------------------------------------------------------
#Net = BuildNetVgg16.BUILD_NET_VGG16(vgg16_npy_path=model_path)  # Create class instance for the net
#Net.build(image, NUM_CLASSES, keep_prob)  # Build net and load intial weights (weights before training)

# from pb file
graph_def_file = "models/frozentensorflowModel.pb"
#input_arrays = ["input"]
#output_arrays = ["MobilenetV1/Predictions/Softmax"]

converter = tf.lite.TFLiteConverter.from_frozen_graph(
  graph_def_file, image, "Pred")#Net.Pred)
tflite_model = converter.convert()
open("models/converted_model.tflite", "wb").write(tflite_model)'''
'''
logs_dir= "logs/"# "path to logs directory where trained model and information will be stored"

#-------------------------Load Trained model if you dont have trained model see: Train.py-----------------------------------------------------------------------------------------------------------------------------
sess = tf.Session() #Start Tensorflow session

print("Setting up Saver...")
saver = tf.train.Saver()


sess.run(tf.global_variables_initializer())
ckpt = tf.train.get_checkpoint_state(logs_dir)
if ckpt and ckpt.model_checkpoint_path: # if train model exist restore it
    saver.restore(sess, ckpt.model_checkpoint_path)
    print("Model restored...")
    sess.run(tf.global_variables_initializer())
    converter = tf.lite.TFLiteConverter.from_session(sess, [image], [Net.Pred])
    tflite_model = converter.convert()
    open("converted_model.tflite", "wb").write(tflite_model)
else:
    print("ERROR NO TRAINED MODEL IN: "+ckpt.model_checkpoint_path+" See Train.py for creating train network ")
    sys.exit()

converter = tf.lite.TFLiteConverter.from_saved_model("logs/")
tflite_model = converter.convert()
open("converted_model.tflite", "wb").write(tflite_model)'''
    