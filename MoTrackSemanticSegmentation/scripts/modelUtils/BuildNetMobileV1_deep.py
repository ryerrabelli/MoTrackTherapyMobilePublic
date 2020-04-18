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
    def build(self, rgb,NUM_CLASSES,keep_prob):  #Build the fully convolutional neural network (FCN) and load weight for decoder based on trained VGG16 network
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
        ], name = "BGR")
        self._debug(bgr)
#-----------------------------Build network encoder based on MobileNetV1 network and load the trained VGG16 weights-----------------------------------------
       #Layer 1
        self.conv1_1 = self.conv_layer(bgr, "conv_1-weights",stride=2) #Build Convolution layer and load weights
        self.conv1_1_BN = self.batch_norm_layer(self.conv1_1,"conv_1", activation=tf.nn.relu6)
        self._debug(self.conv1_1_BN)
        # Layer 2
        self.conv2_1 = self.depthwise_separable_conv2d(self.conv1_1_BN, "conv_ds_2")#Build depthwise separable Convolution layer and load weights
        self._debug(self.conv2_1)
        self.conv2_2 = self.depthwise_separable_conv2d(self.conv2_1, "conv_ds_3", stride =2)#Build depthwise separable Convolution layer and load weights
        self._debug(self.conv2_2)
        # Layer 3
        self.conv3_1 = self.depthwise_separable_conv2d(self.conv2_2, "conv_ds_4", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv3_2 = self.depthwise_separable_conv2d(self.conv3_1, "conv_ds_5", stride =2)#Build depthwise separable Convolution layer and load weights
        self._debug(self.conv3_1)
        self._debug(self.conv3_2)
        # Layer 4
        self.conv4_1 = self.depthwise_separable_conv2d(self.conv3_2, "conv_ds_6", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv4_2 = self.depthwise_separable_conv2d(self.conv4_1, "conv_ds_7", stride =2)#Build depthwise separable Convolution layer and load weights
        self._debug(self.conv4_1)
        self._debug(self.conv4_2)
        # Layer 5
        self.conv5_1 = self.depthwise_separable_conv2d(self.conv4_2, "conv_ds_8", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv5_2 = self.depthwise_separable_conv2d(self.conv5_1, "conv_ds_9", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv5_3 = self.depthwise_separable_conv2d(self.conv5_2, "conv_ds_10", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv5_4 = self.depthwise_separable_conv2d(self.conv5_3, "conv_ds_11", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv5_5 = self.depthwise_separable_conv2d(self.conv5_4, "conv_ds_12", stride =1)#Build depthwise separable Convolution layer and load weights
        self.conv5_6 = self.depthwise_separable_conv2d(self.conv5_5, "conv_ds_13", stride =2)#Build depthwise separable Convolution layer and load weights
        self._debug(self.conv5_1)
        self._debug(self.conv5_2)
        self._debug(self.conv5_3)
        self._debug(self.conv5_4)
        self._debug(self.conv5_5)
        self._debug(self.conv5_6)
        # Layer 6
        self.conv6_1 = self.depthwise_separable_conv2d(self.conv5_6, "conv_ds_14", stride =1)#Build depthwise separable Convolution layer and load weights
        self._debug(self.conv6_1)
        
        ###### DECODER ##### 
        deconv_shape1 = self.conv5_1.get_shape()#self.pool4.get_shape()  # Set the output shape for the the transpose convolution output take only the depth since the transpose convolution will have to have the same depth for output
        W_t1 = utils.weight_variable([4, 4, deconv_shape1[3].value,self.conv6_1.get_shape()[3].value],name="W_t1")  # Deconvolution/transpose in size 4X4 note that the output shape is of  depth NUM_OF_CLASSES this is not necessary in will need to be fixed if you only have 2 catagories
        b_t1 = utils.bias_variable([deconv_shape1[3].value], name="b_t1")
        self.conv_t1 = utils.conv2d_transpose_strided(self.conv6_1, W_t1, b_t1, output_shape=tf.shape(self.conv5_1), name = "conv_t1")#tf.shape(self.pool4))  # Use strided convolution to double layer size (depth is the depth of pool4 for the later element wise addition
        self.fuse_1 = tf.add(self.conv_t1, self.conv5_1, name="fuse_1")  # Add element wise the pool layer from the decoder
        self._debug(self.conv_t1)
        
        deconv_shape2 = self.conv4_1.get_shape()#self.pool3.get_shape()
        W_t2 = utils.weight_variable([4, 4, deconv_shape2[3].value, self.conv_t1.get_shape()[3].value], name="W_t2")
        b_t2 = utils.bias_variable([deconv_shape2[3].value], name="b_t2")
        self.conv_t2 = utils.conv2d_transpose_strided(self.fuse_1, W_t2, b_t2, output_shape=tf.shape(self.conv4_1), name = "conv_t2")#tf.shape(self.pool3))
        self.fuse_2 = tf.add(self.conv_t2, self.conv4_1, name="fuse_2")
        self._debug(self.conv_t2)
        
        deconv_shape3 = self.conv3_1.get_shape()#self.pool3.get_shape()
        W_t3 = utils.weight_variable([4, 4, deconv_shape3[3].value, self.conv_t2.get_shape()[3].value], name="W_t3")
        b_t3 = utils.bias_variable([deconv_shape3[3].value], name="b_t3")
        self.conv_t3 = utils.conv2d_transpose_strided(self.fuse_2, W_t3, b_t3, output_shape=tf.shape(self.conv3_1), name = "conv_t3")#tf.shape(self.pool3))
        self.fuse_3 = tf.add(self.conv_t3, self.conv3_1, name="fuse_3")
        self._debug(self.conv_t3)
        
        deconv_shape4 = self.conv2_1.get_shape()#self.pool3.get_shape()
        W_t4 = utils.weight_variable([4, 4, deconv_shape4[3].value, self.conv_t3.get_shape()[3].value], name="W_t4")
        b_t4 = utils.bias_variable([deconv_shape4[3].value], name="b_t4")
        self.conv_t4 = utils.conv2d_transpose_strided(self.fuse_3, W_t4, b_t4, output_shape=tf.shape(self.conv2_1), name = "conv_t4")#tf.shape(self.pool3))
        self.fuse_4 = tf.add(self.conv_t4, self.conv2_1, name="fuse_4")
        self._debug(self.conv_t4)
        
        deconv_shape5 = self.conv1_1.get_shape()#self.pool3.get_shape()
        W_t5 = utils.weight_variable([4, 4, deconv_shape5[3].value, self.conv_t4.get_shape()[3].value], name="W_t5")
        b_t5 = utils.bias_variable([deconv_shape5[3].value], name="b_t5")
        self.conv_t5 = utils.conv2d_transpose_strided(self.fuse_4, W_t5, b_t5, output_shape=tf.shape(self.conv1_1), name = "conv_t5", stride=1)#tf.shape(self.pool3))
        self.fuse_5 = tf.add(self.conv_t5, self.conv1_1, name="fuse_5")
        self._debug(self.conv_t5)
        self._debug(self.fuse_5)

        shape = tf.shape(rgb)
        W_t6 = utils.weight_variable([16, 16, NUM_CLASSES, deconv_shape5[3].value], name="W_t6")
        b_t6 = utils.bias_variable([NUM_CLASSES], name="b_t6")
        self.Prob = utils.conv2d_transpose_strided(self.fuse_5, W_t6, b_t6, output_shape=[shape[0], shape[1], shape[2], NUM_CLASSES], name = "Prob")
        self._debug(self.Prob)
        #--------------------Transform  probability vectors to label maps-----------------------------------------------------------------
        self.Pred = tf.argmax(self.Prob, dimension=3, name="Pred")
        self._debug(self.Pred)
        print("FCN model built")
    
    @staticmethod
    def _debug(operation):
        print("Layer_name: " + operation.op.name + " -Output_Shape: " + str(operation.shape.as_list()))

#    def conv2d_transpose(self,bottom,name, output_shape=None,outputTensor=None, stride = 1, kernel_size = 3, use_batchnorm = True):
#        with tf.variable_scope(name):
#            stride = [1, stride, stride, 1]
#            if output_shape is None: # then outputTensor must not be none
#                output_shape = outputTensor.get_shape().as_list()
#                output_shape_with_Nones = tf.shape(outputTensor)
#            elif outputTensor is None:
#                output_shape_with_Nones=tf.stack(output_shape)
#            kernel_shape = [kernel_size, kernel_size, output_shape[-1], bottom.shape[-1]]
#            w = self.get_deconv_filter(kernel_shape, name)
#            deconv = tf.nn.conv2d_transpose(bottom, w, output_shape_with_Nones, strides=stride, padding='SAME')
#            bias = tf.get_variable('layer_biases', [output_shape[-1]], initializer=tf.constant_initializer(0.0))
#            out = tf.nn.bias_add(deconv, bias)
#            if use_batchnorm:
#                out = tf.layers.batch_normalization(out, training=True)
#                    
#        return out
#        
#    def get_deconv_filter(self,f_shape, name):
#        """
#        The initializer for the bilinear convolution transpose filters
#        :param f_shape: The shape of the filter used in convolution transpose.
#        :return weights: The initialized weights.
#        """
#        width = f_shape[0]
#        height = f_shape[0]
#        f = math.ceil(width / 2.0)
#        c = (2 * f - 1 - f % 2) / (2.0 * f)
#        bilinear = np.zeros([f_shape[0], f_shape[1]])
#        for x in range(width):
#            for y in range(height):
#                value = (1 - abs(x / f - c)) * (1 - abs(y / f - c))
#                bilinear[x, y] = value
#        weights = np.zeros(f_shape)
#        for i in range(f_shape[2]):
#            weights[:, :, i, i] = bilinear
#    
#        init = tf.constant_initializer(value=weights, dtype=tf.float32)
#        return tf.get_variable('deconv_' + name, weights.shape, tf.float32, initializer=init)
#
#        
######################################################################################################################################################
#    def max_pool(self, bottom, name):
#        return tf.nn.max_pool(bottom, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding='SAME', name=name)
#    
#    def conv2d(self,bottom,name, num_filters,stride = 1, kernel_size=3, use_batchnorm=True):
#        with tf.variable_scope(name):
#            stride = [1, stride, stride, 1]
#            kernel_shape = [kernel_size, kernel_size, bottom.shape[-1], num_filters]
#    
#            with tf.name_scope('layer_weights'):
#                w=tf.get_variable('conv_' + name, kernel_shape, tf.float32, initializer=tf.contrib.layers.xavier_initializer())
#            with tf.name_scope('layer_biases'):
#                bias = tf.get_variable('biases', [num_filters], initializer=tf.constant_initializer(0.0))
#            with tf.name_scope('layer_conv2d'):
#                conv = tf.nn.conv2d(bottom, w, stride, padding='SAME')
#                out = tf.nn.bias_add(conv, bias)
#            if use_batchnorm:
#                out = tf.layers.batch_normalization(out, training=True)
#        return out
#        
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
            #print("DEPTHWISE: " + str(conv_depthwise.get_shape().as_list()))
            if use_batchnorm:
                conv_depthwise=self.batch_norm_layer(conv_depthwise,name+"-depthwise", activation=tf.nn.relu6)
            # pointwise
            filt_pointwise = self.get_conv_filter(name+"-pointwise-weights")
            conv_pointwise = tf.nn.conv2d(conv_depthwise, filt_pointwise, [1, 1, 1, 1], padding='SAME')
            #print("POINTWISE: " + str(conv_pointwise.get_shape().as_list()))
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
    

