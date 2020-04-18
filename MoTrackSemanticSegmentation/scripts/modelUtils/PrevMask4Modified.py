import tensorflow as tf

# ========================Class for building the FCN neural network==================================================================================
class BUILD_NET:

    @staticmethod
    def _debug(operation):
        print("Layer_name: " + operation.op.name + " -Output_Shape: " + str(operation.shape.as_list()))

    ########################################Build Net#####################################################################################################################
    def build(self, rgbm, NUM_CLASSES, keep_prob,
              is_training=True):  # Build the fully convolutional neural network (FCN) and load weight for decoder based on trained VGG16 network
        """
        load variable from npy to build the VGG

        :param rgb: rgb image [batch, height, width, 3] values 0-255
        """
        self.SumWeights = tf.constant(0.0,
                                      name="SumFiltersWeights")  # Sum of weights of all filters for weight decay loss

        print("build model started")
        print("RGBM: " + str(rgbm.get_shape().as_list()))
        # -----------------------------Build network encoder based on MobileNetV1 network and load the trained VGG16 weights-----------------------------------------

        # Layer 1
        self.conv1_1 = tf.layers.batch_normalization(tf.layers.conv2d(inputs = rgbm, filters = 8, strides=1, padding='same', kernel_size = 3))  # Build Convolution layer and load weights
        self._debug(self.conv1_1)
        self.conv2_1 = tf.layers.conv2d(self.conv1_1, 32, strides=1, padding='same', kernel_size=3)
        self._debug(self.conv2_1)
        self.conv3_1 = tf.layers.conv2d(self.conv2_1, 8, strides=1, padding='same', kernel_size=3)
        self._debug(self.conv3_1)
        self.Prob = tf.layers.conv2d(inputs=self.conv3_1, filters=2, kernel_size=3, strides=1, padding='same')
        self._debug(self.Prob)
        # --------------------Transform  probability vectors to label maps-----------------------------------------------------------------
        if (is_training):
            self.Pred = tf.argmax(self.Prob, dimension=3, name="Pred")

        print("FCN model built")

    @staticmethod
    def _debug(operation):
        print("Layer_name: " + operation.op.name + " -Output_Shape: " + str(operation.shape.as_list()))