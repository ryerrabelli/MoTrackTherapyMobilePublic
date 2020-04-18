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
       
        #Layer 1
        self.conv1_1 = self.conv_layer(bgr, "conv_1-weights",stride=2) #Build Convolution layer and load weights
        self.conv1_1_BN = self.batch_norm_layer(self.conv1_1,"conv_1", activation=tf.nn.relu6)
        self._debug(self.conv1_1_BN)
        # Layer 2
        self.conv2_1 = self.depthwise_separable_conv2d(self.conv1_1_BN, "conv_ds_2")#Build depthwise separable Convolution layer and load weights
        self.conv2_2 = self.depthwise_separable_conv2d(self.conv2_1, "conv_ds_3", stride =2)#Build depthwise separable Convolution layer and load weights
        self._debug(self.conv2_2)
        # Layer 3
        self.conv3_1 = self.depthwise_separable_conv2d(self.conv2_2, "conv_ds_4", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv3_2 = self.depthwise_separable_conv2d(self.conv3_1, "conv_ds_5", stride =2)#Build depthwise separable Convolution layer and load weights
        self._debug(self.conv3_2)
        # Layer 4
        self.conv4_1 = self.depthwise_separable_conv2d(self.conv3_2, "conv_ds_6", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv4_2 = self.depthwise_separable_conv2d(self.conv4_1, "conv_ds_7", stride =2)#Build depthwise separable Convolution layer and load weights
        self._debug(self.conv4_2)
        # Layer 5
        self.conv5_1 = self.depthwise_separable_conv2d(self.conv4_2, "conv_ds_8", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv5_2 = self.depthwise_separable_conv2d(self.conv5_1, "conv_ds_9", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv5_3 = self.depthwise_separable_conv2d(self.conv5_2, "conv_ds_10", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv5_4 = self.depthwise_separable_conv2d(self.conv5_3, "conv_ds_11", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv5_5 = self.depthwise_separable_conv2d(self.conv5_4, "conv_ds_12", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv5_6 = self.depthwise_separable_conv2d(self.conv5_5, "conv_ds_13", stride =2)#Build depthwise separable Convolution layer and load weights
        self._debug(self.conv5_1)
        self._debug(self.conv5_6)
        # Layer 6
        self.conv6_1 = self.depthwise_separable_conv2d(self.conv5_6, "conv_ds_14", stride =1)#Build depthwise separable Convolution layer and load weights
        self._debug(self.conv6_1)
        
        self.fcn = tf.layers.conv2d(self.conv6_1, filters=NUM_CLASSES, kernel_size=1, name="fcn")
        self._debug(self.fcn)
        
        self.conv_t1 = tf.layers.conv2d_transpose(inputs=self.fcn,
            filters=self.conv5_1.get_shape()[3],
            kernel_size=4,
            strides=(2, 2),
            padding='same')
        self._debug(self.conv_t1)
        self.reshape_1 = tf.image.resize_bilinear(self.conv_t1, (tf.shape(self.conv5_1)[1],tf.shape(self.conv5_1)[2]))
        self.fuse_1 = tf.add(self.reshape_1, self.conv5_1, name="fuse_1")
        self._debug(self.fuse_1)
        self.conv_t2 = tf.layers.conv2d_transpose(inputs=self.fuse_1,
            filters=self.conv4_1.get_shape()[3],
            kernel_size=4,
            strides=(2, 2),
            padding='same')
        self.reshape_2 =  tf.image.resize_bilinear(self.conv_t2, (tf.shape(self.conv4_1)[1],tf.shape(self.conv4_1)[2]))
        self.fuse_2 = tf.add(self.reshape_2, self.conv4_1, name="fuse_2")
        
        self.conv_t3 = tf.layers.conv2d_transpose(inputs=self.fuse_2,
            filters=rgb.get_shape()[3],
            kernel_size=4,
            strides=(8, 8),
            padding='same')
        
        self.Prob = tf.image.resize_bilinear(self.conv_t3, (tf.shape(rgb)[1],tf.shape(rgb)[2]))
        self._debug(self.Prob)
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
    


