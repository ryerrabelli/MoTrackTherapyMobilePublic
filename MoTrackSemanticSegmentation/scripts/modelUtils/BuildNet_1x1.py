################Class which build the fully convolutional neural net###########################################################

import inspect
import os
import math
from trainUtils import TensorflowUtils as utils
import numpy as np
import tensorflow as tf


VGG_MEAN = [103.939, 116.779, 123.68]# Mean value of pixels in R G and B channels
# this is the base model  for the encoder
base_model="MobileNetV1.npy"

#========================Class for building the FCN neural network==================================================================================
class BUILD_NET:
    def __init__(self, npy_dir=None):
        self.data_dict = np.load(npy_dir+"/"+base_model, encoding='latin1', allow_pickle = True).item() #Load weights of trained VGG16 for encoder
        print("npy file loaded")
        
    @staticmethod
    def _debug(operation):
        print("Layer_name: " + operation.op.name + " -Output_Shape: " + str(operation.shape.as_list()))

########################################Build Net#####################################################################################################################
    def build(self, rgb,NUM_CLASSES,keep_prob, is_training=True):  #Build the fully convolutional neural network (FCN) and load weight for decoder based on trained VGG16 network
        """
        load variable from npy to build the VGG

        :param rgb: rgb image [batch, height, width, 3] values 0-255
        """
        self.SumWeights = tf.constant(0.0, name="SumFiltersWeights") #Sum of weights of all filters for weight decay loss


        print("build model started")
        # rgb_scaled = rgb * 255.0

        # Convert RGB to BGR and substract pixels mean
        red, green, blue = tf.split(axis=3, num_or_size_splits=3, value=rgb)

        bgr = tf.concat(axis=3, values=[
            blue - VGG_MEAN[0],
            green - VGG_MEAN[1],
            red - VGG_MEAN[2],
        ])
        print("BGR: " + str(bgr.get_shape().as_list()))
#-----------------------------Build network encoder based on MobileNetV1 network and load the trained VGG16 weights-----------------------------------------
        self.conv1 = tf.layers.conv2d(inputs=bgr,
            filters=8,
            kernel_size=1,
            strides=1,
            padding='same', activation = tf.nn.relu6)
        self._debug(self.conv1)
        
        self.conv2 = tf.layers.conv2d(inputs=self.conv1,
            filters=64,
            kernel_size=1,
            strides=1,
            padding='same', activation = tf.nn.relu6)
        self._debug(self.conv2)
        
        self.conv3 = tf.layers.conv2d(inputs=self.conv2,
            filters=16,
            kernel_size=1,
            strides=1,
            padding='same', activation = tf.nn.relu6)
        self._debug(self.conv3)
        
        self.Prob = tf.layers.conv2d(inputs=self.conv3,
            filters=2,
            kernel_size=1,
            strides=1,
            padding='same', activation = tf.nn.relu6)
        self._debug(self.Prob)
        
        #--------------------Transform  probability vectors to label maps-----------------------------------------------------------------
        if (is_training):
            self.Pred = tf.argmax(self.Prob, dimension=3, name="Pred")

        print("FCN model built")
    
    @staticmethod
    def _debug(operation):
        print("Layer_name: " + operation.op.name + " -Output_Shape: " + str(operation.shape.as_list()))

############################################################################################################################################################
    def conv_layer(self, bottom, name,stride=1):
        with tf.variable_scope(name):
            filt = self.get_conv_filter(name)
    
            conv = tf.nn.conv2d(bottom, filt, [1, stride, stride, 1], padding='SAME')
    
            #conv_biases = self.get_bias(name)
            #bias = tf.nn.bias_add(conv, conv_biases)
    
            relu = tf.nn.relu(conv)
            return relu

############################################################################################################################################################
    def conv_layer_NoRelu(self, bottom, name, stride=1):
        with tf.variable_scope(name):
            filt = self.get_conv_filter(name)

            conv = tf.nn.conv2d(bottom, filt, strides=[1, stride,stride, 1], padding='SAME')

            conv_biases = self.get_bias(name)
            bias = tf.nn.bias_add(conv, conv_biases)
            return bias
######################################Get MobileNet filter ############################################################################################################
    def get_conv_filter(self, name):
        var=tf.Variable(self.data_dict[name], name="filter_" + name)
        self.SumWeights+=tf.nn.l2_loss(var)
        return var

############################################################################################################################################################
    def batch_norm_layer(self, input_layer, name,activation=None, isTraining = False):
        # TODO: do we need to apply loss to this layer?
        # get all BN weights
        with tf.variable_scope(name):
            moving_variance_weights = self.data_dict[name+"-batch_normalization-moving_variance"]
            moving_mean_weights = self.data_dict[name+"-batch_normalization-moving_mean"]
            gamma_weights = self.data_dict[name+"-batch_normalization-gamma"]
            beta_weights = self.data_dict[name+"-batch_normalization-beta"]
            
            conv_o_bn = tf.layers.batch_normalization(input_layer, beta_initializer = tf.constant_initializer(beta_weights),
                                                       gamma_initializer = tf.constant_initializer(gamma_weights),
                                                       moving_mean_initializer = tf.constant_initializer(moving_mean_weights),
                                                       moving_variance_initializer=tf.constant_initializer(moving_variance_weights),
                                                       training = isTraining)
            if not activation:
                conv_a = conv_o_bn
            else:
                conv_a = activation(conv_o_bn)
            return conv_a
    
    def depthwise_separable_conv2d(self,bottom,name, stride=1, use_batchnorm=True):
        with tf.variable_scope(name):
            # depthwise
            filt_depthwise = self.get_conv_filter(name+"-depthwise-weights")
            conv_depthwise = tf.nn.depthwise_conv2d(bottom, filt_depthwise, [1, stride, stride, 1], padding='SAME')
            if use_batchnorm:
                conv_depthwise=self.batch_norm_layer(conv_depthwise,name+"-depthwise", activation=tf.nn.relu6)
            # pointwise
            filt_pointwise = self.get_conv_filter(name+"-pointwise-weights")
            conv_pointwise = tf.nn.conv2d(conv_depthwise, filt_pointwise, [1, 1, 1, 1], padding='SAME')
            if use_batchnorm:
                conv_pointwise=self.batch_norm_layer(conv_pointwise,name+"-pointwise", activation=tf.nn.relu6)
            return conv_pointwise
#########################################Build fully convolutional layer##############################################################################################################
'''    def fc_layer(self, bottom, name):
        with tf.variable_scope(name):
            shape = bottom.get_shape().as_list()
            dim = 1
            for d in shape[1:]:
                dim *= d
            x = tf.reshape(bottom, [-1, dim])

            weights = self.get_fc_weight(name)
            biases = self.get_bias(name)

            # Fully connected layer. Note that the '+' operation automatically
            # broadcasts the biases.
            fc = tf.nn.bias_add(tf.matmul(x, weights), biases)

            return fc'''

##################################################################################################################################################
#    def get_bias(self, name):
#        return tf.Variable(self.data_dict[name][1], name="biases_"+name)
#############################################################################################################################################
#     def get_fc_weight(self, name):
#        return tf.Variable(self.data_dict[name][0], name="weights_"+name)
    


