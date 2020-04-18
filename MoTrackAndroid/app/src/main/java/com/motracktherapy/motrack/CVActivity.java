package com.motracktherapy.motrack;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Point;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.util.Size;
import android.util.SparseIntArray;
import 	android.util.Range;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;
import android.content.pm.ActivityInfo;
import android.media.AudioManager;

import com.crashlytics.android.Crashlytics;

import org.opencv.android.Utils;
import org.opencv.core.Mat;
import org.opencv.dnn.Dnn;
import org.opencv.dnn.Net;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

public class CVActivity extends AppCompatActivity {

    public String TAG = "CVActivity";
    // c++ methods
    public native boolean inputImage(int width, int height, ByteBuffer srcBuffer, Surface previewSurface, boolean jTesting,  boolean jDoInteractiveTutorial);//, ByteBuffer pixels, int w, int h);
    public static native boolean createAssetAudioPlayer(AssetManager assetManager, String filename,String filenameWindshield, String filenameDoorknock, String filenameSqueeze, String filenameSkip);
    public static native boolean createAssetAudioPlayerBalloonGame(AssetManager assetManager, String filenameBalloon);
    public static native boolean createAssetAudioPlayerCatapultGame(AssetManager assetManager, String filenameCatapultBreak, String filenameCatapultThrow) ;
    public static native boolean createAssetAudioPlayerCliffGame(AssetManager assetManager, String filenameCollapse);
    public static native boolean createAssetAudioPlayerKetchupGame(AssetManager assetManager, String filenameNomNomNom);

    public static native void createEngine();
    public native void resetCalibrationBackground();
    public native void shutdownAudio();
    public native void setLemonBottle(long imageAddr, boolean includes_alpha);
    public native void setCupImage(long imageAddr, boolean includes_alpha);
    public native void setBottleGameScenery(long imageAddr, boolean includes_alpha);
    public native void setCurveArrows(long imageAddrLeft, long imageAddrRight, boolean includes_alpha);
    public native void setSizeRefImg(long imageAddr, boolean includes_alpha);

    public native void setBalloon(long imageAddr, boolean includes_alpha);
    public native void setPoppedBalloon(long imageAddr, boolean includes_alpha);
    public native void setHeliumTank(long imageAddr, boolean includes_alpha);
    public native void setBalloonGameScenery(long imageAddr, boolean includes_alpha);

    public native void setCarFrame(long imageAddr, boolean includes_alpha);
    public native void setWindshieldGameScenery(long imageAddr, boolean includes_alpha);
    public native void setWindshieldWiperBottom(long lemonImageAddr, boolean includes_alpha);
    public native void setWindshieldWiperTop(long lemonImageAddr, boolean includes_alpha);
    public native void setWindshieldGameSteeringWheel(long lemonImageAddr, boolean includes_alpha);

    public native void setKnockGameScenery(long imageAddr, boolean includes_alpha);

    public native void setCliffGameScenery(long imageAddr, boolean includes_alpha);
    public native void setCliffGameCliff(long imageAddr, boolean includes_alpha);
    public native void setCliffGameGoldNugget(long imageAddr, boolean includes_alpha);

    public native void setDialFrame(long imageAddr, boolean includes_alpha);
    public native void setOvenBackground(long imageAddr, boolean includes_alpha);
    public native void setDialPointer(long imageAddr, boolean includes_alpha);

    public native void setFieldScenery(long imageAddr, boolean includes_alpha);
    public native void setBullseye(long imageAddr, boolean includes_alpha);
    public native void setCatapultBeam(long imageAddr, boolean includes_alpha);
    public native void setCatapultMount(long imageAddr, boolean includes_alpha);
    public native void setCatapultMountCracked(long imageAddr, boolean includes_alpha);
    public native void setCatapultMountBrokenFront(long imageAddr, boolean includes_alpha);
    public native void setCatapultMountBrokenBack(long imageAddr, boolean includes_alpha);

    public native void setPeacockGameScenery(long imageAddr, boolean includes_alpha);
    public native void setPeacockGamePeacockBody(long imageAddr, boolean includes_alpha);
    public native void setPeacockGameFeather(long imageAddr, boolean includes_alpha);

    public native void setKetchupGameScenery(long imageAddr);
    public native void setKetchupGamePlateWithFries1(long imageAddr);
    public native void setKetchupGamePlateWithFries2(long imageAddr);
    public native void setKetchupGameKetchupBottle(long imageAddr);
    public native void setKetchupGameKetchupBottleCap(long imageAddr);

    public native void setKetchupGameSingleFryWithoutKetchup(long imageAddr);
    public native void setKetchupGameSingleFryWithKetchup(long imageAddr);

    public native void setAlligatorGameAlligatorBody(long imageAddr);
    public native void setAlligatorGameAlligatorLowerMouth(long imageAddr);
    public native void setAlligatorGameAlligatorUpperMouth(long imageAddr);

    public native void setCaterpillarGameScenery(long imageAddr, boolean includes_alpha);

    public native void setMLNet(long addr);


    public native void redoImage(int width, int height, Surface previewSurface);
    public native void displayCamera(int width, int height, ByteBuffer srcBuffer, Surface previewSurface);
    public native void setLearning(boolean Jlearn);
    public native void setShowingBackground(int Jshow);
    public native void initialize(int nrtbc_each, int hand, String json);
    public native int  doSkip();
    public static native void setDisplayExtraInfo(int displayExtraInfoAmt);
    public static native void setGameSoundsEnabled(int playGameSounds);


    public native int getCurrentMotion();
    public native int getSkips();
    public native int getNumCompleted(int motion);
    public native double getGameScore();
    public native int getNumRepsCompletedTot();
    public native String getDescription();
    public native int getMotionTitle();
    public native int getDoneState();
    public native boolean getDoneInteractiveTutorial();
    public native String getCrashErrorDescription();
    public native String getCrashErrorDescriptionCoded();

    public static native int getOrientationForExerciseGame(int exercise_game);
    public static native void setExerciseGameAndOrientation(int exercise_game, int orientation);

    //variables
    public static boolean useMaxResolution = false;
    private boolean jlearn;
    private boolean prevCalibResult = false; // used to store the previous calibration result
    public static int useBackCamera = 1; // 1 for back camera, 0 for front camera
    private int jshowBackground;
    private TextureView mTextureView; //Represents the texture view, where the camera preview will be displayed
    private TextView titleTextView;
    private TextView descriptionTextView;
    private TextView feedbackTextView;
    private TextView scoreTextView;
    private static final int REQUEST_CAMERA_PERMISSION_RESULT = 0;
    private CameraDevice mCameraDevice; //Represents the camera hardware
    private HandlerThread mBackgroundHandlerThread; //(see background thread methods at bottom)
    private Handler mBackgroundHandler;
    private String mCameraId;
    private Size mPreviewSize;
    private CaptureRequest.Builder mCaptureRequestBuilder;
    private CaptureRequest mCaptureRequest;
    private CameraCaptureSession mCaptureSession;
    private ImageReader mImageReader;
    private Surface mPreviewSurface;
    private static SparseIntArray ORIENTATIONS = new SparseIntArray();
    static {
        ORIENTATIONS.append(Surface.ROTATION_0, 0);
        ORIENTATIONS.append(Surface.ROTATION_90, 90);
        ORIENTATIONS.append(Surface.ROTATION_180, 180);
        ORIENTATIONS.append(Surface.ROTATION_270, 270);
    }

    private static int exerciseGameNum = -1; // the exercise game ID
    private int nrtbcEach = 5; // the exercise game ID
    private int whichHandIntended = 5;
    private String jsonDataStr = "";
    private final boolean isTesting = false; // true if testing, false is actually using/demoing
    public static int orientationGame = -5;  //0 is portrait, 1 is landscape (with left side of phone on bottom)

    static AssetManager assetManagerAudio;
    public static void setExerciseGame(int exercise_game) {
        orientationGame = getOrientationForExerciseGame(exercise_game); //0 is portrait, 1 is landscape
        exerciseGameNum = exercise_game;
        setExerciseGameAndOrientation(exercise_game, orientationGame);
    }

    // Listeners and Callbacks
    private TextureView.SurfaceTextureListener mSurfaceTextureListener = new TextureView.SurfaceTextureListener() {
        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
            openCamera(width, height);
            /*setupCamera(width, height); //setup the camera
            configureTransform(width, height);
            configureTransform(width, height);
            connectCamera(); //connect the camera*/
        }
        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {
            configureTransform(width, height);
        }
        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
            return false;
        }
        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) { }
    };
    private CameraDevice.StateCallback mCameraDeviceStateCallabck = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(@NonNull CameraDevice camera) { //when camera is open, set mCameraDecvice to it
            mCameraDevice = camera;
            startPreview();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) { //when camera is disconnected, close it
            camera.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) { //when there is an error, close the camera
            camera.close();
            mCameraDevice = null;
        }
    };

    boolean paused = false;
    Image lastImage = null;
    ByteBuffer displayedBuffer = null;
    int displayedWidth;
    int displayedHeight;
    CVActivity thisActivity = this;

    private Intent resultIntent;



    private ImageReader.OnImageAvailableListener mImageAvailable = new ImageReader.OnImageAvailableListener() {

        @Override
        public void onImageAvailable(ImageReader reader) {
        if (lastImage != null) lastImage.close();
        //get the image
        lastImage = reader.acquireLatestImage();

        int doneState = getDoneState();
        if (doneState > 0) {
            resultIntent.putExtra("CVACTIVITY_GAMESCORE", getGameScore());
            resultIntent.putExtra("CVACTIVITY_NRC_TOT", getNumRepsCompletedTot());
            thisActivity.setResult(RESULT_OK, resultIntent);
            thisActivity.finish();

        } else if (doneState < 0) { //this means an error happened
            String errorDescript = getCrashErrorDescription();
            String errorDescriptCoded = getCrashErrorDescriptionCoded();

            int nrcTot = getNumRepsCompletedTot();
            double gameScore = getGameScore();

            Crashlytics.setInt(     "whichHandIntended",    whichHandIntended);
            Crashlytics.setInt(     "exerciseGame",         exerciseGameNum);
            Crashlytics.setInt(     "orientationGame",      orientationGame);
            Crashlytics.setInt(     "nrtbcEach",            nrtbcEach);
            Crashlytics.setInt(     "repsCompletedTot",     nrcTot);
            Crashlytics.setDouble(  "gameScore",            gameScore);
            Crashlytics.setString(  "codedDescription",     errorDescriptCoded);

            //Crashlytics.logException(new Exception(errorDescript));
            Crashlytics.getInstance().core.logException(new Exception(errorDescript));

            resultIntent.putExtra("CVACTIVITY_GAMESCORE", gameScore);
            resultIntent.putExtra("CVACTIVITY_ERROR", errorDescriptCoded);
            resultIntent.putExtra("CVACTIVITY_NRC_TOT", nrcTot);
            thisActivity.setResult(RESULT_CANCELED, resultIntent);
            thisActivity.finish();

        } else {
            if (lastImage != null && !paused) {
                if (lastImage.getFormat() != ImageFormat.YUV_420_888) {
                    throw new IllegalArgumentException("src must have format YUV_420_888.");
                }
                Image.Plane[] planes = lastImage.getPlanes();
                // Spec guarantees that planes[0] is luma and has pixel stride of 1.
                // It also guarantees that planes[1] and planes[2] have the same row and
                // pixel stride.
                if (planes[1].getPixelStride() != 1 && planes[1].getPixelStride() != 2) {
                    throw new IllegalArgumentException(
                            "src chroma plane must have a pixel stride of 1 or 2: got "
                                    + planes[1].getPixelStride());
                }
                // when this isn't equal to planes[N].getRowWidth(), it causes the green bar issue
                displayedWidth = lastImage.getWidth();
                displayedHeight = lastImage.getHeight();
                //displayedBuffer = planes[0].getBuffer();
                //System.out.println("displayedbuffer changed");

                ByteBuffer Y = planes[0].getBuffer();
                ByteBuffer U = planes[1].getBuffer();
                ByteBuffer V = planes[2].getBuffer();

                displayedBuffer = ByteBuffer.allocateDirect(Y.limit()+U.limit()+V.limit()).put(Y).put(U).put(V);
                displayedBuffer.flip();
                //System.out.println("displayedbuffer changed");

            /*File f = new File("/storage/emulated/0/Download/gingi.jpg");
            if(f.exists()) {
                System.out.println("file exists");
            }*/
                //Bitmap bMap=BitmapFactory.decodeResource(getResources(),R.drawable.lemon0);
                //byte[] bytes = new byte[displayedBuffer.capacity()];
                //displayedBuffer.get(bytes);
                //Bitmap bMap2 = BitmapFactory.decodeByteArray(bytes, 0, bytes.length, null);
                //Bitmap bMap=BitmapFactory.decodeFile("/storage/emulated/0/Download/gingi.jpg");

                //int bytes = bMap.getByteCount();
                //or we can calculate bytes this way. Use a different value than 4 if you don't use 32bit images.
                //int bytes = b.getWidth()*b.getHeight()*4;
                //int[] pixels = new int[bMap.getWidth() * bMap.getHeight() * 4]; // assume RGBA, do once only
                //bMap.getPixels(pixels, 0, bMap.getWidth(), 0, 0, bMap.getWidth(), bMap.getHeight());
                //ByteBuffer lemonBuffer = ByteBuffer.allocate(bytes); //Create a new buffer
                //bMap.copyPixelsToBuffer(lemonBuffer); //Move the byte data to the buffer
                //displayedWidth = bMap.getWidth();
                //displayedHeight = bMap.getHeight();
                //Mat src = new Mat();//330, 600, CvType.CV_8UC3, new Scalar(4));
                //Utils.bitmapToMat(bMap, src);

                //int rows = src.rows();
                //int cols = src.cols();
                //int channels = src.channels();
                //Calculate function execution time
                //double start = System.currentTimeMillis();
                //double start1 = Debug.threadCpuTimeNanos();
                boolean doInteractiveTutorial = false;//&!getDoneInteractiveTutorial();
                boolean calibResult = inputImage(displayedWidth, displayedHeight, displayedBuffer, mPreviewSurface, isTesting, doInteractiveTutorial);//, lemonBuffer, bMap.getHeight(), bMap.getWidth());
            /*double end = System.currentTimeMillis();
            double end1 = Debug.threadCpuTimeNanos();
            double t = (end - start);

            //Calculate the amount of time that the current thread has spent executing code or waiting for certain types of I/O.

            double t1 = (end1 - start1)/1000000;

            double fps = 1000/t;
            double fps1 = 1/t1;

            //CPU Usage
            double cpu = (t1/t) * 100;*/
                if (calibResult != prevCalibResult) { // different result in calibration,
                /*if (calibResult) {
                    restartPreview(true); // control lighting
                } else {
                    restartPreview(false);
                }*/
                    restartPreview(calibResult);
                    prevCalibResult = calibResult;
                }
                //displayCamera(image.getWidth(), image.getHeight(), planes[0].getBuffer(),
                //        mPreviewSurface);


                //image.close();
            }
        }

        }
    };


    private static ArrayList<Integer> exerciseGameNumsLoaded_left = new ArrayList<>();
    private static ArrayList<Integer> exerciseGameNumsLoaded_right = new ArrayList<>();
    // "On" methods.  Note: no main method.  These are automatically called when something happens


    /*
     * Runs when the activity is created
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        WebAppInterface.mAnalytics.setCurrentScreen(this, "game_num"+exerciseGameNum, "nativecv" /* null would be a class override */);

        resultIntent = new Intent();

        Intent startingIntent = getIntent();
        nrtbcEach = startingIntent.getIntExtra("NRTBC_EACH", -1);
        resultIntent.putExtra("NRTBC_EACH", nrtbcEach); //make it available again in the final screen
        whichHandIntended = startingIntent.getIntExtra("WHICH_HAND_INTENDED", -1);
        jsonDataStr = startingIntent.getStringExtra("JSON_DATA");
        if (jsonDataStr == null) jsonDataStr = "";
        if (nrtbcEach < 0) {
            String error = "Reps of " + nrtbcEach + " is invalid";
            Toast.makeText(getApplicationContext(), error, Toast.LENGTH_LONG).show();
            resultIntent.putExtra("CVACTIVITY_GAMESCORE", 0);
            resultIntent.putExtra("CVACTIVITY_ERROR", "Java: "+error);
            resultIntent.putExtra("CVACTIVITY_NRC_TOT", 0 );
            thisActivity.setResult(RESULT_CANCELED, resultIntent);
            thisActivity.finish();
        }

        setContentView(R.layout.activity_cv);


        //INITIALIZE C++ WITH THE NUMBER OF REPS AND THE HAND
        initialize(nrtbcEach, whichHandIntended, jsonDataStr);


        //SET UP IMAGES (ON ANDROID, WOULD SET UP AUDIO, BUT NO NEED TO SET UP AUDIO ON iOS)
        setUpImagesAndAudio(whichHandIntended);


        //SET UP ORIENTATION
        //Below lines will specify the orientation of the page (this is the programmatic way, as opposed to specifying it in AndroidManifest.xml
        if (orientationGame == 0) {
            setRequestedOrientation (ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        } else if (orientationGame == 1) {
            setRequestedOrientation (ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        } else {
            setRequestedOrientation (ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        }


        //SET UP DEVICE-SPECIFIC USER INTERFACE
        setUpAndroidUserInterface();

    }


    final int   FIST_SQUEEZE_BOTTLE_GAME = 100,
                FIST_PUMP_BALLOON_GAME = 102,
                FIST_PADDLE_BALL_GAME = 104,
                FIST_PLAIN_GAME = 105,
                HOOK_FIST_CATERPILLAR_GAME = 110,
                HOOK_FIST_PLAIN_GAME = 115,
                DEVIATION_WINDSHIELD_WIPER_GAME = 120,
                DEVIATION_CATAPULT_LAUNCH_GAME = 122,
                DEVIATION_PLAIN_GAME = 125,
                FLEXION_DOOR_KNOCK_GAME = 130,
                FLEXION_CLIFF_MINE_GAME = 132,
                FLEXION_PLAIN_GAME = 135,
                PRONATION_TURN_DIAL_GAME = 150,
                PRONATION_KETCHUP_SHAKE_GAME = 152,
                PRONATION_PLAIN_GAME = 155,
                FABDUCTION_PEACOCK_GAME = 160,
                EXERCISE_GAME_ABDUCTION_PLAIN = 165,
                TOPPOSITION_ALLIGATOR_GAME = 170,
                EXERCISE_GAME_OPPOSITION_PLAIN = 175;


    private void setUpHandSizeRefImages(int exercise_game) throws IOException {
        Mat sizeRefImg = null;
        if (exercise_game == DEVIATION_PLAIN_GAME || exercise_game == DEVIATION_WINDSHIELD_WIPER_GAME || exercise_game == DEVIATION_CATAPULT_LAUNCH_GAME ||
                exercise_game == PRONATION_PLAIN_GAME || exercise_game == PRONATION_TURN_DIAL_GAME || exercise_game == PRONATION_KETCHUP_SHAKE_GAME) {
            sizeRefImg = Utils.loadResource(getApplicationContext(), R.drawable.sizeref_openpalm);

        } else if (exercise_game == FIST_PLAIN_GAME || exercise_game == FIST_SQUEEZE_BOTTLE_GAME || exercise_game == FIST_PUMP_BALLOON_GAME || exercise_game == FIST_PADDLE_BALL_GAME) { //squeeze bottle and pump balloon
            sizeRefImg = Utils.loadResource(getApplicationContext(), R.drawable.sizeref_closedfist);

        } else if (exercise_game == HOOK_FIST_PLAIN_GAME || exercise_game == HOOK_FIST_CATERPILLAR_GAME) {
            sizeRefImg = Utils.loadResource(getApplicationContext(), R.drawable.sizeref_hookfist);

        } else if (exercise_game == FLEXION_PLAIN_GAME || exercise_game == FLEXION_DOOR_KNOCK_GAME || exercise_game == FLEXION_CLIFF_MINE_GAME) { //door knock and cliff mine
            sizeRefImg = Utils.loadResource(getApplicationContext(), R.drawable.sizeref_flexionextension);

        } else if (exercise_game == EXERCISE_GAME_ABDUCTION_PLAIN || exercise_game == FABDUCTION_PEACOCK_GAME) {
            sizeRefImg = Utils.loadResource(getApplicationContext(), R.drawable.sizeref_openpalm);

        } else if (exercise_game == EXERCISE_GAME_OPPOSITION_PLAIN ||exercise_game == TOPPOSITION_ALLIGATOR_GAME) {
            sizeRefImg = Utils.loadResource(getApplicationContext(), R.drawable.sizeref_thumb);

        }

        if (sizeRefImg != null) setSizeRefImg(sizeRefImg.getNativeObjAddr(), true);
    }


    // Modified this from the getPath(.) function in https://docs.opencv.org/4.1.1/d0/d6c/tutorial_dnn_android.html
    // Upload file to storage and return a path.
    private String getPath(String subfolderWithinAssets, String file, Context context) {
        if (!subfolderWithinAssets.endsWith("/")) subfolderWithinAssets = subfolderWithinAssets + "/";

        AssetManager assetManager = context.getAssets();
        BufferedInputStream inputStream = null;
        try {
            // Read data from assets.
            inputStream = new BufferedInputStream(assetManager.open(subfolderWithinAssets+file));
            byte[] data = new byte[inputStream.available()];
            inputStream.read(data);
            inputStream.close();

            // Create copy file in storage.
            File outFile = new File(context.getFilesDir(), file); //testing on Android, it appeared to go a location like /data/user/0/com.motracktherapy.motrack/files/tensorflowModel.pb
            FileOutputStream os = new FileOutputStream(outFile);
            os.write(data);
            os.close();


            Log.i(TAG, "Saved to " + outFile.getAbsolutePath());
            // Return a path to file which may be read in common way.
            return outFile.getAbsolutePath();
        } catch (IOException ex) {
            Log.i(TAG, "Failed to upload a file");
        }
        return "";
    }

    private void setUpImagesAndAudio(int hand) {
        // set up audio
        assetManagerAudio = getAssets();
        createEngine();

        String audPath = "Dropbox/ui_media/sounds/";

        //Do this until we get some bugs fixed including
        //1) if you go from right hand to left hand to right hand, it will use the left hand image since the second right hand doesn't trigger reseting of the images
        //2) the bullseye hits remain across different play sessions in catapult game
        //3) cliff cracks remain across different play sessions
        // Presumably other bugs like 2) and 3) are present. These only happen in Android since iOS
        // doesn't keep a list and just resets each image every time.
        exerciseGameNumsLoaded_right.clear();
        exerciseGameNumsLoaded_left.clear();


        if ((hand==0 && !exerciseGameNumsLoaded_right.contains(exerciseGameNum)) ||
                (hand==1 && !exerciseGameNumsLoaded_left.contains(exerciseGameNum))) {
            //if this exerciseGame's images haven't been loaded before, then load them now.

            if (exerciseGameNum == DEVIATION_PLAIN_GAME || exerciseGameNum == DEVIATION_WINDSHIELD_WIPER_GAME
                    || exerciseGameNum == FLEXION_PLAIN_GAME || exerciseGameNum == FLEXION_DOOR_KNOCK_GAME) {
                // get feedback arrows for radial and ulnar deviation
                Mat left_curve_arrow = null;
                Mat right_curve_arrow = null;
                try {
                    left_curve_arrow = Utils.loadResource(getApplicationContext(), R.drawable.left_curve_arrow);
                    right_curve_arrow = Utils.loadResource(getApplicationContext(), R.drawable.right_curve_arrow);
                    setCurveArrows(left_curve_arrow.getNativeObjAddr(), right_curve_arrow.getNativeObjAddr(), true);

                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            //Dont do "else if" between the last if and the next if because the games from the above if repeat below

            if (exerciseGameNum == FIST_SQUEEZE_BOTTLE_GAME) {
                // get lemon bottle image
                Mat sizeRefImg = null;
                Mat src_bottle = null;
                Mat src_cup = null;
                Mat src_scenery = null;
                try {
                    src_bottle = Utils.loadResource(getApplicationContext(), R.drawable.squeezegame_lemonbottle);
                    src_cup = Utils.loadResource(getApplicationContext(), R.drawable.squeezegame_cup);
                    setLemonBottle(src_bottle.getNativeObjAddr(), true);
                    setCupImage(src_cup.getNativeObjAddr(), true);

                    src_scenery = Utils.loadResource(getApplicationContext(), R.drawable.squeezegame_scenery);
                    setBottleGameScenery(src_scenery.getNativeObjAddr(), true);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }

            } else if (exerciseGameNum == FIST_PUMP_BALLOON_GAME) { //pump balloon fist game
                Mat src_balloon = null;
                Mat src_poppedballoon = null;
                Mat src_heliumtank = null;
                Mat sizeRefImg = null;
                Mat src_scenery = null;
                try {
                    src_balloon = Utils.loadResource(getApplicationContext(), R.drawable.balloongame_balloon);
                    setBalloon(src_balloon.getNativeObjAddr(), true);

                    src_heliumtank = Utils.loadResource(getApplicationContext(), R.drawable.balloongame_heliumtank);
                    setHeliumTank(src_heliumtank.getNativeObjAddr(), true);

                    src_poppedballoon = Utils.loadResource(getApplicationContext(), R.drawable.balloongame_poppedballoon);
                    setPoppedBalloon(src_poppedballoon.getNativeObjAddr(), true);

                    src_scenery = Utils.loadResource(getApplicationContext(), R.drawable.balloongame_scenery);
                    setBalloonGameScenery(src_scenery.getNativeObjAddr(), true);
                } catch (Exception e) {
                    Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT).show();
                    e.printStackTrace();
                }

                // load audio
                createAssetAudioPlayerBalloonGame(assetManagerAudio, audPath+"balloon_pop_sound.mp3");

            } else if (exerciseGameNum == FIST_PADDLE_BALL_GAME) { //paddle ball fist game


            } else if (exerciseGameNum == DEVIATION_PLAIN_GAME) {
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }

            } else if (exerciseGameNum == DEVIATION_WINDSHIELD_WIPER_GAME) { //Windshield Game
                try {
                    Mat carFrame = Utils.loadResource(getApplicationContext(), R.drawable.windshieldgame_carframeandwheel);
                    setCarFrame(carFrame.getNativeObjAddr(), true);

                    Mat windshieldGameScenery = Utils.loadResource(getApplicationContext(), R.drawable.windshieldgame_scenery);
                    setWindshieldGameScenery(windshieldGameScenery.getNativeObjAddr(), true);

                    Mat windshieldWiperBottom = Utils.loadResource(getApplicationContext(), R.drawable.windshieldgame_windshieldbottom);
                    setWindshieldWiperBottom(windshieldWiperBottom.getNativeObjAddr(), true);

                    Mat windshieldWiperTop = Utils.loadResource(getApplicationContext(), R.drawable.windshieldgame_windshieldtop);
                    setWindshieldWiperTop(windshieldWiperTop.getNativeObjAddr(), true);
                    //Mat windshieldGameSteeringWheel = Utils.loadResource(getApplicationContext(), R.drawable.windshieldgame_steeringwheel);
                    //setWindshieldGameSteeringWheel(windshieldGameSteeringWheel.getNativeObjAddr(), true);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }

            } else if (exerciseGameNum == DEVIATION_CATAPULT_LAUNCH_GAME) { //Catapult Game
                try {
                    Mat fieldScenery = Utils.loadResource(getApplicationContext(), R.drawable.catapultgame_scenery);
                    setFieldScenery(fieldScenery.getNativeObjAddr(), true);

                    Mat bullseye = Utils.loadResource(getApplicationContext(), R.drawable.catapultgame_bullseye);
                    setBullseye(bullseye.getNativeObjAddr(), true);

                    Mat catapultMount = Utils.loadResource(getApplicationContext(), R.drawable.catapultgame_catapultmount);
                    setCatapultMount(catapultMount.getNativeObjAddr(), true);

                    Mat catapultBeam = Utils.loadResource(getApplicationContext(), R.drawable.catapultgame_catapultbeam);
                    setCatapultBeam(catapultBeam.getNativeObjAddr(), true);

                    Mat catapultMountCracked = Utils.loadResource(getApplicationContext(), R.drawable.catapultgame_catapultmountcracked);
                    setCatapultMountCracked(catapultMountCracked.getNativeObjAddr(), true);

                    Mat catapultMountBrokenFront = Utils.loadResource(getApplicationContext(), R.drawable.catapultgame_catapultmountbrokenfront);
                    setCatapultMountBrokenFront(catapultMountBrokenFront.getNativeObjAddr(), true);

                    Mat catapultMountBrokenBack = Utils.loadResource(getApplicationContext(), R.drawable.catapultgame_catapultmountbrokenback);
                    setCatapultMountBrokenBack(catapultMountBrokenBack.getNativeObjAddr(), true);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }

                // load audio
                createAssetAudioPlayerCatapultGame(assetManagerAudio, audPath+"catapult_break.mp3",audPath+"catapult_throw.mp3");

            } else if (exerciseGameNum == FLEXION_DOOR_KNOCK_GAME) {
                //Door knock game
                try {
                    Mat knockGameScenery = Utils.loadResource(getApplicationContext(), R.drawable.knockgame_scenery);
                    setKnockGameScenery(knockGameScenery.getNativeObjAddr(), true);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }

            } else if (exerciseGameNum == FLEXION_CLIFF_MINE_GAME) {
                //Cliff mine game
                try {
                    Mat cliffGameScenery = Utils.loadResource(getApplicationContext(), R.drawable.cliffgame_scenery);
                    setCliffGameScenery(cliffGameScenery.getNativeObjAddr(), true);

                    Mat cliffGameCliff = Utils.loadResource(getApplicationContext(), R.drawable.cliffgame_cliff);
                    setCliffGameCliff(cliffGameCliff.getNativeObjAddr(), true);

                    Mat cliffGameGoldNugget = Utils.loadResource(getApplicationContext(), R.drawable.cliffgame_goldnugget);
                    setCliffGameGoldNugget(cliffGameGoldNugget.getNativeObjAddr(), true);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }

                // load audio
                createAssetAudioPlayerCliffGame(assetManagerAudio, audPath+"cliff_collapse.mp3");
            } else if (exerciseGameNum == FLEXION_PLAIN_GAME) {
                //exerciseGameNum of 135 is for flexion extension plain

            } else if (exerciseGameNum == PRONATION_TURN_DIAL_GAME) {
                //dial game
                try {
                    Mat dialFrame = Utils.loadResource(getApplicationContext(), R.drawable.dialgame_dialframe);
                    setDialFrame(dialFrame.getNativeObjAddr(), true);

                    Mat ovenBackground = Utils.loadResource(getApplicationContext(), R.drawable.dialgame_ovenbackground);
                    setOvenBackground(ovenBackground.getNativeObjAddr(), true);

                    Mat dialPointer = Utils.loadResource(getApplicationContext(), R.drawable.dialgame_dialpointer);
                    setDialPointer(dialPointer.getNativeObjAddr(), true);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }

            } else if (exerciseGameNum == PRONATION_KETCHUP_SHAKE_GAME) {
                try {
                    Mat dialFrame = Utils.loadResource(getApplicationContext(), R.drawable.dialgame_dialframe);
                    setDialFrame(dialFrame.getNativeObjAddr(), true);

                    Mat ovenBackground = Utils.loadResource(getApplicationContext(), R.drawable.dialgame_ovenbackground);
                    setOvenBackground(ovenBackground.getNativeObjAddr(), true);

                    Mat dialPointer = Utils.loadResource(getApplicationContext(), R.drawable.dialgame_dialpointer);
                    setDialPointer(dialPointer.getNativeObjAddr(), true);


                    Mat mat1 = Utils.loadResource(getApplicationContext(), R.drawable.ketchupgame_scenery);
                    setKetchupGameScenery(mat1.getNativeObjAddr());

                    Mat mat2 = Utils.loadResource(getApplicationContext(), R.drawable.ketchupgame_platewithfries1);
                    setKetchupGamePlateWithFries1(mat2.getNativeObjAddr());

                    Mat mat3 = Utils.loadResource(getApplicationContext(), R.drawable.ketchupgame_platewithfries2);
                    setKetchupGamePlateWithFries2(mat3.getNativeObjAddr());

                    Mat mat4 = Utils.loadResource(getApplicationContext(), R.drawable.ketchupgame_bottle);
                    setKetchupGameKetchupBottle(mat4.getNativeObjAddr());

                    Mat mat5 = Utils.loadResource(getApplicationContext(), R.drawable.ketchupgame_bottlecap);
                    setKetchupGameKetchupBottleCap(mat5.getNativeObjAddr());

                    Mat mat6 = Utils.loadResource(getApplicationContext(), R.drawable.ketchupgame_singlefrywithoutketchup);
                    setKetchupGameSingleFryWithoutKetchup(mat6.getNativeObjAddr());

                    Mat mat7 = Utils.loadResource(getApplicationContext(), R.drawable.ketchupgame_singlefrywithketchup);
                    setKetchupGameSingleFryWithKetchup(mat7.getNativeObjAddr());

                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }

                //load audio
                createAssetAudioPlayerKetchupGame(assetManagerAudio, audPath+"nomnomnom.mp3");

            } else if (exerciseGameNum == EXERCISE_GAME_ABDUCTION_PLAIN) {
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }

            } else if (exerciseGameNum == FABDUCTION_PEACOCK_GAME) {
                try {
                    Mat scenery = Utils.loadResource(getApplicationContext(), R.drawable.peacockgame_scenery);
                    setPeacockGameScenery(scenery.getNativeObjAddr(), true);

                    Mat peacockBody = Utils.loadResource(getApplicationContext(), R.drawable.peacockgame_peacockbody);
                    setPeacockGamePeacockBody(peacockBody.getNativeObjAddr(), true);

                    Mat feather = Utils.loadResource(getApplicationContext(), R.drawable.peacockgame_feather);
                    setPeacockGameFeather(feather.getNativeObjAddr(), true);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }

            } else if (exerciseGameNum == EXERCISE_GAME_OPPOSITION_PLAIN) {
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }

            } else if (exerciseGameNum == TOPPOSITION_ALLIGATOR_GAME) {
                try {
                    Mat alligatorBody = Utils.loadResource(getApplicationContext(), R.drawable.alligatorgame_alligatorbody);
                    setAlligatorGameAlligatorBody(alligatorBody.getNativeObjAddr());

                    Mat mouthUpper = Utils.loadResource(getApplicationContext(), R.drawable.alligatorgame_alligatormouthupper);
                    long addrUpper = mouthUpper.getNativeObjAddr();
                    setAlligatorGameAlligatorUpperMouth(addrUpper);

                    Mat mouthLower = Utils.loadResource(getApplicationContext(), R.drawable.alligatorgame_alligatormouthlower);
                    long addrLower = mouthLower.getNativeObjAddr();
                    setAlligatorGameAlligatorLowerMouth(addrLower);

                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }
            } else if (exerciseGameNum == HOOK_FIST_CATERPILLAR_GAME) {
                try {
                    Mat scenery = Utils.loadResource(getApplicationContext(), R.drawable.caterpillargame_scenery);
                    setCaterpillarGameScenery(scenery.getNativeObjAddr(), true);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (hand==0) {
                    exerciseGameNumsLoaded_right.add(exerciseGameNum);
                } else if (hand == 1) {
                    exerciseGameNumsLoaded_left.add(exerciseGameNum);
                }
            }
        }


        try {
            setUpHandSizeRefImages(exerciseGameNum);
        } catch (IOException e) {
            e.printStackTrace();
        }

        //click.wav is the file for the skip button sound (same file/sound reused from html ui)
        createAssetAudioPlayer(assetManagerAudio, audPath+"correct.mp3",audPath+"windshield_wiper.mp3",
                audPath+"door_knock.mp3",audPath+"squeeze.mp3", audPath+"click.wav");


        //https://docs.opencv.org/3.4.3/d0/d6c/tutorial_dnn_android.html
        //String dnnModelPrefix = "/storage/emulated/0/Download/"; //Doesn't work (even file is in internal storage on Ben's phone)
        //String dnnModelPrefix = "/Internal Storage/Download/";
        String dnnModelPrefix =  "dnnmodels/"; //this.getFilesDir() +
        String model  = "tensorflowModel.pb";
        String config = "tensorflowModel.pbtxt.txt";
        model  = getPath(dnnModelPrefix,  model, this);
        config = getPath(dnnModelPrefix, config, this);
        Net net = Dnn.readNetFromTensorflow(model);

        setMLNet( net.getNativeObjAddr() );
        int myint=2+2;
    }


    // returns the volume level for the game
    public int checkVolumeLevel() {
        AudioManager am = (AudioManager) getSystemService(AUDIO_SERVICE);
        int volume_level= am.getStreamVolume(AudioManager.STREAM_MUSIC);
        // STREAM_MUSIC gets the media volume. different STREAM_<type> gets different volume
        // works for both bluetooth connected speaker and just in general
        // on the note 8, volume_level goes from 0 to 15 (with 0 being quiet)
        // just as an addition, im not sure if there is another way to mute the media volume without setting this to 0
        return volume_level;
    }

    private void setUpAndroidUserInterface() {
        //texture view where images are displayed
        mTextureView = (TextureView) findViewById(R.id.textureView);

        //Text views
        titleTextView = (TextView) findViewById(R.id.textView2);
        titleTextView.setText(Integer.toString(getMotionTitle()));

        descriptionTextView = (TextView) findViewById(R.id.textView3);
        descriptionTextView.setText(getDescription());

        feedbackTextView = (TextView) findViewById(R.id.textView4);
        feedbackTextView.setText("Feedback: not implemented yet");

        scoreTextView = (TextView) findViewById(R.id.textView5);
        scoreTextView.setText("Score " + getGameScore());

        //skip button
        final Button skipButton = findViewById(R.id.skipButton);
        skipButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            int skipResult = doSkip();
            if (skipResult == 1) {
                descriptionTextView.setText(getDescription());
                titleTextView.setText(Integer.toString(getMotionTitle()));
            }
            scoreTextView.setText("Score " + getGameScore());
            }
        });

        if (!isTesting) { // remove the testing buttons
            //reset button
            final Button resetbutton = findViewById(R.id.resetButton);
            resetbutton.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    resetCalibrationBackground();
                }
            });
            if (exerciseGameNum == 0) {
                skipButton.setBackgroundColor(Color.TRANSPARENT);
                skipButton.setTextColor(Color.TRANSPARENT);
                resetbutton.setBackgroundColor(Color.TRANSPARENT);
                resetbutton.setTextColor(Color.TRANSPARENT);
            }

            final ToggleButton b = findViewById(R.id.toggleButton);
            b.setVisibility(View.GONE);
            final Button button = findViewById(R.id.backgroundbutton);
            button.setVisibility(View.GONE);
            final Button button2 = findViewById(R.id.button2);
            button2.setVisibility(View.GONE);

            titleTextView.setVisibility(View.GONE);
            descriptionTextView.setVisibility(View.GONE);
            feedbackTextView.setVisibility(View.GONE);
            scoreTextView.setVisibility(View.GONE);
            TextView backgroundText = (TextView) findViewById(R.id.textView6);
            backgroundText.setVisibility(View.GONE);
            TextView learnText = (TextView) findViewById(R.id.textView7);
            learnText.setVisibility(View.GONE);

        } else {
            final Button resetButton = findViewById(R.id.resetButton);
            resetButton.setVisibility(View.GONE);

            //toggle learning button
            jlearn = true;
            final ToggleButton learningRateButton = findViewById(R.id.toggleButton);
            learningRateButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if (jlearn) {
                        jlearn = false;
                    } else {
                        jlearn = true;
                    }
                    setLearning(jlearn);
                }
            });


            //toggle learning button
            final String[] background_options = new String[]{"Yes", "Half", "No"};
            jshowBackground = 0;
            final Button showBackgroundButton = findViewById(R.id.backgroundbutton);
            showBackgroundButton.setText(background_options[jshowBackground]);
            showBackgroundButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    jshowBackground = -1;
                    for (int option_i = 0; option_i < background_options.length; option_i++) {
                        if (showBackgroundButton.getText().toString().equalsIgnoreCase(background_options[option_i])) {
                            jshowBackground = (option_i + 1) % background_options.length;
                            showBackgroundButton.setText(background_options[jshowBackground]);
                            setShowingBackground(jshowBackground);
                            if (paused) {
                                //inputImage(displayedWidth, displayedHeight, displayedBuffer, mPreviewSurface);
                                //redoImage(displayedWidth, displayedHeight, mPreviewSurface);
                            }

                            break;
                        }
                    }
                }
            });

            //pause button
            final Button closeCameraButton = findViewById(R.id.button2); //closeCamera

            //skipButton.setVisibility(View.INVISIBLE);
            //closeCameraButton.setVisibility(View.INVISIBLE);
            //feedbackTextView.setVisibility(View.INVISIBLE);

            closeCameraButton.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                if (paused) {
                    paused = false;
                    closeCameraButton.setText("Pause");
                    Toast.makeText(getApplicationContext(), "Played", Toast.LENGTH_SHORT).show();
                } else {
                    paused = true;
                    closeCameraButton.setText("Play");
                    Toast.makeText(getApplicationContext(), "Paused", Toast.LENGTH_SHORT).show();
                }
                }
            });
        }
    }

    /*
     * Runs when the activity resumes after being paused
     */
    @Override
    protected void onResume() {
        super.onResume();

        startBackgroundThread(); //starts our background thread

        if (mTextureView.isAvailable()) {
            openCamera(mTextureView.getWidth(), mTextureView.getHeight());
        } else {
            mTextureView.setSurfaceTextureListener(mSurfaceTextureListener); //calls onSurfaceTextureAvailable when it's ready?
        }
    }

    /*
     * Runs when the app requests certain permissions (like the camera permission
     */
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if(requestCode == REQUEST_CAMERA_PERMISSION_RESULT) {
            if(grantResults[0] != PackageManager.PERMISSION_GRANTED) { //if something goes wrong, request rejected
                Toast.makeText(getApplicationContext(),
                        "Application will not run without camera services", Toast.LENGTH_SHORT).show();
            }
        }
    }

    /*
     * Runs when the activity is paused
     */
    @Override
    protected void onPause() {
        closeCamera(); //closes camera when activity is paused
        stopBackgroundThread(); //stops the background thread

        super.onPause();
    }

    /*
     * Runs when the activity is stopped
     */
    @Override
    protected void onStop() {

        shutdownAudio();

        super.onStop();
    }

    /*
     * Causes the app to be full screen
     */
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        View decorView = getWindow().getDecorView();
        if(hasFocus) {
            decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION // hide nav bar
            | View.SYSTEM_UI_FLAG_FULLSCREEN // hide status bar
            | View.SYSTEM_UI_FLAG_IMMERSIVE);
        }
    }

    @Override
    public void onBackPressed() {
        resultIntent.putExtra("CVACTIVITY_GAMESCORE", getGameScore());
        resultIntent.putExtra("CVACTIVITY_NRC_TOT", getNumRepsCompletedTot());
        thisActivity.setResult(RESULT_CANCELED, resultIntent);
        super.onBackPressed();
    }


    /*
     * returns the [width,height] of the desired image size on the phone
     * if multiple image sizes are available with same aspect ratio, take smallest one
     */
    /*private Integer[] getPhoneDims(CameraCharacteristics cameraCharacteristics) {
        StreamConfigurationMap streamConfigurationMap = cameraCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        Size[] sizes = streamConfigurationMap.getOutputSizes(SurfaceTexture.class);
        Size[] sizes1 = streamConfigurationMap.getOutputSizes(ImageFormat.JPEG);

        DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        int DSI_height = displayMetrics.heightPixels; //2076
        int DSI_width  = displayMetrics.widthPixels; //1080
        double aspect_ratio = (double)(DSI_height)/DSI_width;
        Integer[] bestDims = {DSI_width, DSI_height};
        boolean set = false;
        double minDiff = 100000;
        Integer[] secondBestDims = {DSI_width, DSI_height};
        for (Size s: sizes1) {
            double sRatio = (double)(s.getHeight())/s.getWidth();
            if (sRatio==aspect_ratio) {
                if (s.getHeight() < bestDims[1]) {
                    bestDims[0] = s.getWidth();
                    set = true;
                    bestDims[1] = s.getHeight();
                    //return bestDims;
                }
            } else { // set 2nd best
                double diff = Math.abs(aspect_ratio - sRatio);
                if (diff < minDiff) {
                    minDiff = diff;
                    secondBestDims[0] = s.getWidth();
                    secondBestDims[1] = s.getHeight();
                } else if (diff == minDiff && s.getHeight() < secondBestDims[1]) {
                    secondBestDims[0] = s.getWidth();
                    secondBestDims[1] = s.getHeight();
                }
            }
        }
        if (set) {
            return bestDims;
        } else { //return 2nd best option
            return secondBestDims;
        }
        // just return the default width and size, should never reach this point
        //Integer[] result = ;
        //return bestDims;
    }

    private Integer[] widthHeight;*/
    /*
     * Sets up the camera to be used
     */
    private void setupCamera(int width, int height) {
        CameraManager cameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE); //manages the device's cameras
        try { //needs try catch in case something goes wrong with the camera
            for(String cameraId : cameraManager.getCameraIdList()) { //searching for the correct camera
                CameraCharacteristics cameraCharacteristics
                        = cameraManager.getCameraCharacteristics(cameraId);
                //widthHeight = getPhoneDims(cameraCharacteristics);
                Integer facing = cameraCharacteristics.get(CameraCharacteristics.LENS_FACING);
                if( facing != null && facing == CameraCharacteristics.LENS_FACING_BACK) {
                    if (useBackCamera == 1 && facing == CameraCharacteristics.LENS_FACING_BACK) { // LENS_FACING_BACK is 1
                        continue; //if it is the back facing camera skip it, we want the front facing camera
                    }
                    if (useBackCamera == 0 && facing == CameraCharacteristics.LENS_FACING_FRONT) { // LENS_FACING_FRONT is 0
                        continue; //if it is the front facing camera skip it, we want the back facing camera
                    }
                }

                StreamConfigurationMap map = cameraCharacteristics.get(
                        CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                if (map == null) {
                    continue;
                }

                // Find out if we need to swap dimension to get the preview size relative to sensor
                // coordinate.
                int displayRotation = getWindowManager().getDefaultDisplay().getRotation();
                //noinspection ConstantConditions
                mSensorOrientation = cameraCharacteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
                boolean swappedDimensions = false;
                switch (displayRotation) {
                    case Surface.ROTATION_0:
                    case Surface.ROTATION_180:
                        if (mSensorOrientation == 90 || mSensorOrientation == 270) {
                            swappedDimensions = true;
                        }
                        break;
                    case Surface.ROTATION_90:
                    case Surface.ROTATION_270:
                        if (mSensorOrientation == 0 || mSensorOrientation == 180) {
                            swappedDimensions = true;
                        }
                        break;
                    default:
                        Log.e(TAG, "Display rotation is invalid: " + displayRotation);
                }

                Point displaySize = new Point();
                getWindowManager().getDefaultDisplay().getSize(displaySize);
                int rotatedPreviewWidth = width;
                int rotatedPreviewHeight = height;
                int maxPreviewWidth = displaySize.x;
                int maxPreviewHeight = displaySize.y;

                //Size largest = new Size(352,288);
                //Size largest = new Size(maxPreviewWidth,maxPreviewHeight);
                //May wanna take a look here
                if (swappedDimensions) {
                    rotatedPreviewWidth = height;
                    rotatedPreviewHeight = width;
                    maxPreviewWidth = displaySize.y;
                    maxPreviewHeight = displaySize.x;
                }

                if (maxPreviewWidth > MAX_PREVIEW_WIDTH) {
                    maxPreviewWidth = MAX_PREVIEW_WIDTH;
                }

                if (maxPreviewHeight > MAX_PREVIEW_HEIGHT) {
                    maxPreviewHeight = MAX_PREVIEW_HEIGHT;
                }

                mPreviewSize = chooseOptimalSize(map.getOutputSizes(SurfaceTexture.class),
                        rotatedPreviewWidth, rotatedPreviewHeight, maxPreviewWidth,
                        maxPreviewHeight, useMaxResolution);//, largest);
                // We fit the aspect ratio of TextureView to the size of preview we picked.
                int orientation = getResources().getConfiguration().orientation;
            /*if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
                mTextureView.setAspectRatio(
                        mPreviewSize.getWidth(), mPreviewSize.getHeight());
            } else {
                mTextureView.setAspectRatio(
                        mPreviewSize.getHeight(), mPreviewSize.getWidth());
            }*/

                // Check if the flash is supported.
                Boolean available = cameraCharacteristics.get(CameraCharacteristics.FLASH_INFO_AVAILABLE);
                mFlashSupported = available == null ? false : available;

                mCameraId = cameraId;



                //mImageReader = ImageReader.newInstance(largest.getWidth(), largest.getHeight(), ImageFormat.YUV_420_888, 2);
                mImageReader = ImageReader.newInstance(mPreviewSize.getWidth(), mPreviewSize.getHeight(), ImageFormat.YUV_420_888, 2);

                mImageReader.setOnImageAvailableListener(mImageAvailable,mBackgroundHandler);

                return;
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }


    private Range<Integer> getRange() {
        CameraManager cameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        CameraCharacteristics chars = null;
        try {
            chars = cameraManager.getCameraCharacteristics(mCameraId);
            Range<Integer>[] ranges = chars.get(CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);
            Range<Integer> result = null;
            for (Range<Integer> range : ranges) {
                int upper = range.getUpper();
                // 10 - min range upper for my needs
                if (upper >= 10) {
                    if (result == null || upper < result.getUpper().intValue()) {
                        result = range;
                    }
                }
            }
            if (result == null) {
                result = ranges[0];
            }
            return result;
        } catch (CameraAccessException e) {
            e.printStackTrace();
            return null;
        }
    }

    private void restartPreview(boolean controlLight) {
        try {
            if (controlLight) {
                // Below three lines causes the lighting of the camera to not change
                //mCaptureRequestBuilder.set(CaptureRequest.CONTROL_MODE,CaptureRequest.CONTROL_MODE_OFF);
                /*mCaptureRequestBuilder.set(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_ON);
                mCaptureRequestBuilder.set(CaptureRequest.CONTROL_AE_LOCK, true);
                mCaptureRequestBuilder.set(CaptureRequest.CONTROL_AE_EXPOSURE_COMPENSATION, 6);*/
                mCaptureRequestBuilder.set(CaptureRequest.CONTROL_AE_LOCK,true);
                mCaptureRequestBuilder.set(CaptureRequest.CONTROL_AWB_LOCK,true);

                //mCaptureRequestBuilder.set(CaptureRequest.CONTROL_AE_LOCK, true);
               /* mCaptureRequestBuilder.set(CaptureRequest.CONTROL_MODE, CaptureRequest.CONTROL_MODE_OFF);
                mCaptureRequestBuilder.set(CaptureRequest.CONTROL_VIDEO_STABILIZATION_MODE, CaptureRequest.CONTROL_VIDEO_STABILIZATION_MODE_OFF);
                mCaptureRequestBuilder.set(CaptureRequest.LENS_OPTICAL_STABILIZATION_MODE, CaptureRequest.LENS_OPTICAL_STABILIZATION_MODE_OFF);
                mCaptureRequestBuilder.set(CaptureRequest.SENSOR_EXPOSURE_TIME, 500000000L);
                mCaptureRequestBuilder.set(CaptureRequest.SENSOR_SENSITIVITY, 400);*/

            } else {
                mCaptureRequestBuilder.set(CaptureRequest.CONTROL_AE_LOCK,false);
                mCaptureRequestBuilder.set(CaptureRequest.CONTROL_AWB_LOCK, false);
            }
            mCaptureRequest = mCaptureRequestBuilder.build();
            // Set new repeating request with our changed one
            mCaptureSession.setRepeatingRequest(mCaptureRequest, null, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /*
     * Starts camera preview on our texture view
     */
    private void startPreview() {
        try {
            //get texture view surface and set it's dimensions
            SurfaceTexture surfaceTexture = mTextureView.getSurfaceTexture();
            assert surfaceTexture != null;

            surfaceTexture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());

            //create preview surface from texture view
            mPreviewSurface = new Surface(surfaceTexture);

            mCaptureRequestBuilder
                    = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);

            List<Surface> surfaces = new ArrayList<>();

            //create image reader surface
            Surface readerSurface = mImageReader.getSurface();
            surfaces.add(readerSurface);
            mCaptureRequestBuilder.addTarget(readerSurface);

            mCameraDevice.createCaptureSession(surfaces, new CameraCaptureSession.StateCallback() {

                @Override
                public void onConfigured(CameraCaptureSession session) {
                    mCaptureSession = session;
                    try {
                        // Auto focus should be continuous for camera preview.
                        mCaptureRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_AUTO);
                        //mCaptureRequestBuilder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE,getRange());
                        //Exposure Control (set it manually)
                        //mCaptureRequestBuilder.set(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_MODE_OFF);
                        //mCaptureRequestBuilder.set(CaptureRequest.SENSOR_EXPOSURE_TIME, 10000000L);
                        //mCaptureRequestBuilder.set(CaptureRequest.SENSOR_SENSITIVITY, 400);


                        mCaptureRequest = mCaptureRequestBuilder.build();
                        session.setRepeatingRequest(mCaptureRequestBuilder.build(),
                                null, mBackgroundHandler); // You may be able to replace null with stuff to do processing
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void onConfigureFailed(CameraCaptureSession session){
                    Toast.makeText(getApplicationContext(),
                            "Unable to setup camera preview", Toast.LENGTH_SHORT).show();
                }
            }, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /*
     * Opens the camera specified
     */
    private void openCamera(int width, int height) {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
            if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                if (shouldShowRequestPermissionRationale(Manifest.permission.CAMERA)) { //if they've said no before, give them this message
                    Toast.makeText(this,
                            "This app requires access to camera", Toast.LENGTH_SHORT).show();
                }
                requestPermissions(new String[]{Manifest.permission.CAMERA}, REQUEST_CAMERA_PERMISSION_RESULT);
                return;
            }
        }
        setupCamera(width, height);
        configureTransform(width, height);
        Activity activity = this;
        CameraManager manager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        try {
            /*if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }*/
            manager.openCamera(mCameraId, mCameraDeviceStateCallabck, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } /*catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera opening.", e);
        }*/
    }

    /*
     * closes the camera
     */
    private void closeCamera() {
        if(mCameraDevice != null) {
            mCameraDevice.close();
            mCameraDevice = null;
        }
    }

    /*
     * Background thread pushes all our time consuming tasks off the UI thread to prevent framerate loss
     */
    private void startBackgroundThread() {
        mBackgroundHandlerThread = new HandlerThread(("CVActivity"));
        mBackgroundHandlerThread.start();
        mBackgroundHandler = new Handler(mBackgroundHandlerThread.getLooper());
    }

    /*
     * Background thread pushes all our time consuming tasks off the UI thread to prevent framerate loss
     */
    private void stopBackgroundThread() {
        mBackgroundHandlerThread.quitSafely();
        try {
            mBackgroundHandlerThread.join(); //stops anything from interrupting this
            mBackgroundHandlerThread = null;
            mBackgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }




    //Delete Later:  Trying to make Ben's camera work
    /**
     * Compares two {@code Size}s based on their areas.
     */
    static class CompareSizesByArea implements Comparator<Size> {

        @Override
        public int compare(Size lhs, Size rhs) {
            // We cast here to ensure the multiplications won't overflow
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() -
                    (long) rhs.getWidth() * rhs.getHeight());
        }

    }
    /**
     * Orientation of the camera sensor
     */
    private int mSensorOrientation;


    public static Size chooseOptimalSize(Size[] choices, int maxWidth, int maxHeight, boolean n_useMaxResolution) {
        return chooseOptimalSize(choices, -1, -1, maxWidth, maxHeight, n_useMaxResolution);
    }
    /**
     * Given {@code choices} of {@code Size}s supported by a camera, choose the smallest one that
     * is at least as large as the respective texture view size, and that is at most as large as the
     * respective max size, and whose aspect ratio matches with the specified value. If such size
     * doesn't exist, choose the largest one that is at most as large as the respective max size,
     * and whose aspect ratio matches with the specified value.
     *
     * NOTE: This function (or the associated function without some arguments) is also called by web app interface.
     *
     * @param choices           The list of sizes that the camera supports for the intended output
     *                          class
     * @param textureViewWidth  The width of the texture view relative to sensor coordinate
     * @param textureViewHeight The height of the texture view relative to sensor coordinate
     * @param maxWidth          The maximum width that can be chosen
     * @param maxHeight         The maximum height that can be chosen
     * @return The optimal {@code Size}, or an arbitrary one if none were big enough
     */
    public static Size chooseOptimalSize(Size[] choices, int textureViewWidth,
                                          int textureViewHeight, int maxWidth, int maxHeight, boolean n_useMaxResolution) { //}, Size aspectRatio) {
        //Now that we have a separation between the analysis and the displaying, we don't need to make the resolution as small as possible
        if (n_useMaxResolution) return new Size(maxWidth,maxHeight);

        /*Size biggestSize = new Size(0,0);
        for (Size option : choices) {
            if (option.getHeight()>=biggestSize.getHeight() && option.getWidth()>=biggestSize.getWidth()) {
                biggestSize = option;
            }
        }*/

        Size smallestSize = new Size(maxWidth,maxHeight);
        boolean sizeChanged = false;
        for (Size option : choices) {
            if (option.getHeight()<=smallestSize.getHeight() && option.getWidth()<=smallestSize.getWidth()) {
                //don't let the size below 400 in the smallest dimension, unless we have no option
                if (Math.max(option.getWidth(),option.getHeight()) > 400 ) {
                    smallestSize = option;
                }
            }
        }
        if (true) return smallestSize;


        // Collect the supported resolutions that are at least as big as the preview Surface
        //List<Size> bigEnough = new ArrayList<>();
        // Collect the supported resolutions that are smaller than the preview Surface
        //List<Size> notBigEnough = new ArrayList<>();
        //int w = maxWidth;//aspectRatio.getWidth();
        //int h = maxHeight;//aspectRatio.getHeight();
        Size bestDims = new Size(maxWidth,maxHeight);
        boolean set = false;
        double minDiff = 100000;
        Size secondBestDims = new Size(maxWidth,maxHeight);
        for (Size option : choices) {
            if (option.getWidth() <= maxWidth && option.getHeight() <= maxHeight &&
                    option.getHeight() == option.getWidth() * maxHeight / maxWidth) {
                /*if (option.getWidth() >= textureViewWidth &&
                        option.getHeight() >= textureViewHeight) {
                    bigEnough.add(option);
                } else {
                    notBigEnough.add(option);
                }*/
                if (option.getWidth() < bestDims.getWidth()) {
                    bestDims = option;
                    set = true;
                }
            } else if (!set) { // set the 2nd best option only if set isn't true
                double diff = Math.abs((double)(maxHeight)/maxWidth - (double)(option.getHeight())/option.getWidth() );
                if (diff < minDiff) {
                    minDiff = diff;
                    secondBestDims = option;
                } else if (diff == minDiff && option.getHeight() < secondBestDims.getHeight()) {
                    secondBestDims = option;
                }
            }
        }

        if (set) {
            return bestDims;
        } else { //return 2nd best option
            return secondBestDims;
        }
        // Pick the smallest of those big enough. If there is no one big enough, pick the
        // largest of those not big enough.
        /*if (bigEnough.size() > 0) {
            return Collections.min(bigEnough, new CompareSizesByArea());
        } else if (notBigEnough.size() > 0) {
            return Collections.max(notBigEnough, new CompareSizesByArea());
        } else {
            Log.e("TESTING", "Couldn't find any suitable preview size");
            return choices[0];
        }*/
    }
    /**
     * Max preview width that is guaranteed by Camera2 API
     */
    protected static final int MAX_PREVIEW_WIDTH = 1920;
    /**
     * Max preview height that is guaranteed by Camera2 API
     */
    protected static final int MAX_PREVIEW_HEIGHT = 1080;
    /**
     * Whether the current camera device supports Flash or not.
     */
    private boolean mFlashSupported;
    //Helps compare camera resolution sizes to texture view size
    private static class CompareSizeByArea implements Comparator<Size> {

        @Override
        public int compare(Size lhs, Size rhs) {
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() /
                    (long) rhs.getWidth() * rhs.getHeight());
        }
    }
    /**
     * Configures the necessary {@link android.graphics.Matrix} transformation to `mTextureView`.
     * This method should be called after the camera preview size is determined in
     * setUpCameraOutputs and also the size of `mTextureView` is fixed.
     *
     * @param viewWidth  The width of `mTextureView`
     * @param viewHeight The height of `mTextureView`
     */
    private void configureTransform(int viewWidth, int viewHeight) {
        if (null == mTextureView || null == mPreviewSize) {
            return;
        }
        int rotation = getWindowManager().getDefaultDisplay().getRotation();
        Matrix matrix = new Matrix();
        RectF viewRect = new RectF(0, 0, viewWidth, viewHeight);
        RectF bufferRect = new RectF(0, 0, mPreviewSize.getHeight(), mPreviewSize.getWidth());
        float centerX = viewRect.centerX();
        float centerY = viewRect.centerY();
        if (Surface.ROTATION_90 == rotation || Surface.ROTATION_270 == rotation) {
            bufferRect.offset(centerX - bufferRect.centerX(), centerY - bufferRect.centerY());
            matrix.setRectToRect(viewRect, bufferRect, Matrix.ScaleToFit.FILL);
            float scale = Math.max(
                    (float) viewHeight / mPreviewSize.getHeight(),
                    (float) viewWidth / mPreviewSize.getWidth());
            matrix.postScale(scale, scale, centerX, centerY);
            matrix.postRotate(90 * (rotation - 2), centerX, centerY);
        } else if (Surface.ROTATION_180 == rotation) {
            matrix.postRotate(180, centerX, centerY);
        }
        mTextureView.setTransform(matrix);
    }

}
