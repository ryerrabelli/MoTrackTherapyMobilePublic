#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Jun  7 10:42:43 2019

@author: benka
"""
import tensorflow as tf
import Data_Reader
import BuildNetVgg16
logs_dir= "logs/"# "path to logs directory where trained model and information will be stored"

model_path="Model_Zoo/vgg16.npy"# "Path to pretrained vgg16 model for encoder"
NUM_CLASSES = 2 # Number of classes
keep_prob = tf.placeholder(tf.float32, name="keep_probabilty")  # Dropout probability
image = tf.placeholder(tf.float32, shape=[None, None, None, 3], name="input_image")  # Input image batch first dimension image number second dimension width third dimension height 4 dimension RGB

 # -------------------------Build Net----------------------------------------------------------------------------------------------
Net = BuildNetVgg16.BUILD_NET_VGG16(vgg16_npy_path=model_path)  # Create class instance for the net
Net.build(image, NUM_CLASSES, keep_prob)  # Build net and load intial weights (weights before training)
#-------------------------Load Trained model if you dont have trained model see: Train.py-----------------------------------------------------------------------------------------------------------------------------
sess = tf.Session() #Start Tensorflow session

print("Setting up Saver...")
saver = tf.train.Saver()

sess.run(tf.global_variables_initializer())
ckpt = tf.train.get_checkpoint_state(logs_dir)
if ckpt and ckpt.model_checkpoint_path: # if train model exist restore it
    saver.restore(sess, ckpt.model_checkpoint_path)
    print("Model restored...")
    tf.profiler.profile(
        sess.graph,
        options=tf.profiler.ProfileOptionBuilder.float_operation())
else:
    print("ERROR NO TRAINED MODEL IN: "+ckpt.model_checkpoint_path+" See Train.py for creating train network ")
    sys.exit()

'''
g = tf.Graph()
run_meta = tf.RunMetadata()
with g.as_default():
    A = tf.Variable(tf.random_normal( [25,16] ))
    B = tf.Variable(tf.random_normal( [16,9] ))
    C = tf.matmul(A,B) # shape=[25,9]

    opts = tf.profiler.ProfileOptionBuilder.float_operation()    
    flops = tf.profiler.profile(g, run_meta=run_meta, cmd='op', options=opts)
    if flops is not None:
        print('Flops should be ~',2*25*16*9)
        print('25 x 25 x 9 would be',2*25*25*9) # ignores internal dim, repeats first
        print('TF stats gives',flops.total_float_ops)'''