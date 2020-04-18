package com.motracktherapy.motrack;

import android.Manifest;
import android.app.Activity;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.NotificationCompat;
import android.support.v4.app.NotificationManagerCompat;
import android.support.v4.content.ContextCompat;
import android.text.TextUtils;
import android.util.Log;
import android.util.Size;
import android.webkit.JavascriptInterface;
import android.webkit.WebView;
import android.widget.Toast;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
//import com.google.firebase.auth.*;

import java.util.HashMap;
import java.util.Iterator;
import java.util.concurrent.Executor;

import com.crashlytics.android.Crashlytics;
import com.google.firebase.analytics.FirebaseAnalytics;
import com.google.firebase.iid.FirebaseInstanceId;


import com.google.firebase.iid.InstanceIdResult;
import com.google.firebase.messaging.FirebaseMessaging;
//*/

import org.json.JSONException;
import org.json.JSONObject;

import io.fabric.sdk.android.Fabric;


/**
 * Created by ryerrabelli on 9/27/18.
 */

public class WebAppInterface {

    public String TAG = "WepAppInterface";
    public static FirebaseAnalytics mAnalytics;

    MainActivity mContext; //of class "Context". Forced it under the subclass Activity to work with Firebase authentication code
    int minCVActivityRequestCode;
    static String username = null;

    /** Instantiate the interface and set the context */
    WebAppInterface(MainActivity my_context, int my_minCVActivityRequestCode) {
        mContext = my_context;
        minCVActivityRequestCode = my_minCVActivityRequestCode;

        //sets up crashlytics
        Fabric.with(my_context, new Crashlytics());

        //set up firebase analytics
        mAnalytics = FirebaseAnalytics.getInstance(mContext);
    }

    //private FirebaseAuth getAuth() {return mContext.getFirebaseAuth();}
    private WebView getWebView() {
        return mContext.getWebView();
    }

    @JavascriptInterface
    public void setUserID(String userID) {
        LOGD("setUserID:" + userID);
        //Note in swift and js, it is setUserID, but in Java, it is setUserId
        mAnalytics.setUserId(userID);

        //"If you ever need to clear a user identifier after you set it, reset the value to a blank string."
        //if (userID == null) userID = ""; //removed this line because I don't know how to do it in swift, and I want to keep Android/iOS the same
        Crashlytics.setUserIdentifier(userID);

    }

    @JavascriptInterface
    public void logEvent(String name, String jsonParams) {
        LOGD("logEvent:" + name);
        mAnalytics.logEvent(name, bundleFromJson(jsonParams));
    }

    @JavascriptInterface
    public void setUserProperty(String name, String value) {
        LOGD("setUserProperty:" + name);
        mAnalytics.setUserProperty(name, value);
    }

    @JavascriptInterface
    public void setCurrentScreen(String toPageId) {
        mAnalytics.setCurrentScreen(mContext, toPageId, "webview" /* null would be a class override */);
    }

    private void LOGD(String message) {
        // Only log on debug builds, for privacy
        if (BuildConfig.DEBUG) {
            Log.d(TAG, message);
        }
    }

    private Bundle bundleFromJson(String json) {
        // [START_EXCLUDE]
        if (TextUtils.isEmpty(json)) {
            return new Bundle();
        }

        Bundle result = new Bundle();
        try {
            JSONObject jsonObject = new JSONObject(json);
            Iterator<String> keys = jsonObject.keys();

            while (keys.hasNext()) {
                String key = keys.next();
                Object value = jsonObject.get(key);

                if (value instanceof String) {
                    result.putString(key, (String) value);
                } else if (value instanceof Integer) {
                    result.putInt(key, (Integer) value);
                } else if (value instanceof Double) {
                    result.putDouble(key, (Double) value);
                } else {
                    Log.w(TAG, "Value for key " + key + " not one of [String, Integer, Double]");
                }
            }
        } catch (JSONException e) {
            Log.w(TAG, "Failed to parse JSON, returning empty Bundle.", e);
            return new Bundle();
        }

        return result;
        // [END_EXCLUDE]
    }



    @JavascriptInterface
    public int getVersionCode() {
        return BuildConfig.VERSION_CODE;
    }
    @JavascriptInterface
    public String getVersionName() {
        return BuildConfig.VERSION_NAME;
    }
    @JavascriptInterface
    public String getDeviceInfo(String property) {
        switch(property.toUpperCase()) {
            case "BOARD":           return Build.BOARD;
            case "BOOTLOADER":      return Build.BOOTLOADER;
            case "BRAND":           return Build.BRAND;
            case "DEVICE":          return Build.DEVICE;
            case "DISPLAY":         return Build.DISPLAY;
            case "HARDWARE":        return Build.HARDWARE;
            case "MANUFACTURER":    return Build.MANUFACTURER;
            case "PRODUCT":         return Build.PRODUCT;
            case "FINGERPRINT":     return Build.FINGERPRINT;
            case "HOST":            return Build.HOST;
            case "ID":              return Build.ID;
            case "MODEL":           return Build.MODEL;
            case "TAGS":            return Build.TAGS;
            case "TYPE":            return Build.TYPE;
            case "USER":            return Build.USER;
            default:                return "";
        }
    }

    @JavascriptInterface
    public HashMap<String, String> getDeviceInfo() {
        HashMap<String, String>  toReturn = new HashMap<>();
        toReturn.put("MANUFACTURER", Build.MANUFACTURER);
        toReturn.put("c", "cccc");
        return toReturn;
    }


    /** Show a toast from the web page */
    @JavascriptInterface
    public void showToast(String toast) {
        Toast.makeText(mContext, toast, Toast.LENGTH_SHORT).show();
    }

    private String CHANNEL_ID = "MoTrackNotification";
    private int NOTIFICATION_ID = 1;//"MoTrackNotID";
    private NotificationCompat.Builder notificationBuilder;
    @JavascriptInterface
    public void createPushNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            CharSequence name = mContext.getString(R.string.channel_name);
            String description = mContext.getString(R.string.channel_description);
            int importance = NotificationManager.IMPORTANCE_DEFAULT;
            NotificationChannel channel = new NotificationChannel(CHANNEL_ID, name, importance);
            channel.setDescription(description);
            // Register the channel with the system; you can't change the importance
            // or other notification behaviors after this
            NotificationManager notificationManager = mContext.getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }
    }
    @JavascriptInterface
    public void sendPushNotification(String contentTitle, String contentText, String contentTextBigText) {
        // Create an explicit intent for an Activity in your app
        Intent intent = new Intent(mContext, MainActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        PendingIntent pendingIntent = PendingIntent.getActivity(mContext, 0, intent, 0);

        notificationBuilder = new NotificationCompat.Builder(mContext, CHANNEL_ID)
                .setSmallIcon(R.drawable.singleletterlogo_384)
                .setContentTitle(contentTitle)
                .setContentText(contentText)
                .setStyle(new NotificationCompat.BigTextStyle()
                        .bigText(contentTextBigText))
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                // Set the intent that will fire when the user taps the notification
                .setContentIntent(pendingIntent)
                .setAutoCancel(true);

        NotificationManagerCompat notificationManager = NotificationManagerCompat.from(mContext);

        // notificationId is a unique int for each notification that you must define
        notificationManager.notify(NOTIFICATION_ID, notificationBuilder.build());

    }

    private void initializeNotification() {
        notificationBuilder = new NotificationCompat.Builder(mContext, CHANNEL_ID)
                .setSmallIcon(R.drawable.singleletterlogo_384)
                .setContentTitle("My notification")
                .setContentText("Much longer text that cannot fit one line...")
                .setStyle(new NotificationCompat.BigTextStyle()
                        .bigText("Much longer text that cannot fit one line..."))
                .setPriority(NotificationCompat.PRIORITY_DEFAULT);
    }
    private void createNotificationChannel() {
    //Calendar calendar = Calendar.getInstance();
        //calendar.set(Calendar.HOUR_OF_DAY, 18);
        //calendar.set(Calendar.MINUTE, 30);
        //calendar.set(Calendar.SECOND, 0);
        //Intent intent1 = new Intent(MainActivity.this, AlarmReceiver.class);
        //PendingIntent pendingIntent = PendingIntent.getBroadcast(MainActivity.this, 0,intent1, PendingIntent.FLAG_UPDATE_CURRENT);
        //AlarmManager am = (AlarmManager) MainActivity.this.getSystemService(MainActivity.this.ALARM_SERVICE);
        //am.setRepeating(AlarmManager.RTC_WAKEUP, calendar.getTimeInMillis(), AlarmManager.INTERVAL_DAY, pendingIntent);

        // Create the NotificationChannel, but only on API 26+ because
        // the NotificationChannel class is new and not in the support library
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            CharSequence name = mContext.getString(R.string.channel_name);
            String description = mContext.getString(R.string.channel_description);
            int importance = NotificationManager.IMPORTANCE_DEFAULT;
            NotificationChannel channel = new NotificationChannel(CHANNEL_ID, name, importance);
            channel.setDescription(description);
            // Register the channel with the system; you can't change the importance
            // or other notification behaviors after this
            NotificationManager notificationManager = mContext.getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }

        // Create an explicit intent for an Activity in your app
        Intent intent = new Intent(mContext, MainActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        PendingIntent pendingIntent = PendingIntent.getActivity(mContext, 0, intent, 0);

        notificationBuilder = new NotificationCompat.Builder(mContext, CHANNEL_ID)
                .setSmallIcon(R.drawable.singleletterlogo_384)
                .setContentTitle("My notification")
                .setContentText("Much longer text that cannot fit one line...")
                .setStyle(new NotificationCompat.BigTextStyle()
                        .bigText("Much longer text that cannot fit one line..."))
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                // Set the intent that will fire when the user taps the notification
                .setContentIntent(pendingIntent)
                .setAutoCancel(true);

        NotificationManagerCompat notificationManager = NotificationManagerCompat.from(mContext);

        // notificationId is a unique int for each notification that you must define
        notificationManager.notify(NOTIFICATION_ID, notificationBuilder.build());
    }

    @JavascriptInterface
    public void crashAppPurposefully() {
        //Note: This line doesn't actually crash the app if called here (because of threading or
        // something?). To crash the app, call it in a different line, like the WebAppInterface constructor
        Crashlytics.getInstance().crash(); // Force a crash

    }

    @JavascriptInterface
    public void startGame(int exerciseGame) {
        int which_hand_intended = 0; // 0 for right, 1 for left
        int nrtbc_each = 5; // 5 each means 10 reps total, and max score of 10
        startGame(exerciseGame, which_hand_intended, nrtbc_each);
    }
    @JavascriptInterface
    public void startGame(int exerciseGame, int which_hand_intended, int nrtbc_each) {
        /*
        Bundle bundle = new Bundle();
        bundle.putInt(FirebaseAnalytics.Param.QUANTITY, nrtbc_each);
        bundle.putInt(FirebaseAnalytics.Param.ITEM_ID, exerciseGame);
        bundle.putInt("which_hand_intended", which_hand_intended);
        mAnalytics.logEvent("start_game", bundle);

        if (exerciseGame >= 0) { //avoid -1, which means that the exercise game couldn't be found
            CVActivity.setExerciseGame(exerciseGame);
            int requestCode = minCVActivityRequestCode+exerciseGame;
            Intent i = new Intent(mContext, CVActivity.class);
            i.putExtra("WHICH_HAND_INTENDED", which_hand_intended); // 0 for right, 1 for left
            i.putExtra("NRTBC_EACH", nrtbc_each); // 5 each means 10 reps total, and max score of 10.
            mContext.startActivityForResult(i, requestCode);
        } else {
            showToast("Error: That game does not exist.");
        }*/
        startGame(exerciseGame, which_hand_intended, nrtbc_each, "no_data");
    }

    @JavascriptInterface
    public void startGame(int exerciseGame, int which_hand_intended, int nrtbc_each, String json_data) {
        Bundle bundle = new Bundle();
        bundle.putInt(FirebaseAnalytics.Param.QUANTITY, nrtbc_each);
        bundle.putInt(FirebaseAnalytics.Param.ITEM_ID, exerciseGame);
        bundle.putInt("hand", which_hand_intended);
        bundle.putString("json_data",json_data);
        mAnalytics.logEvent("start_game", bundle);

        if (exerciseGame >= 0) { //avoid -1, which means that the exercise game couldn't be found
            CVActivity.setExerciseGame(exerciseGame);
            int requestCode = minCVActivityRequestCode+exerciseGame;
            Intent i = new Intent(mContext, CVActivity.class);
            i.putExtra("WHICH_HAND_INTENDED", which_hand_intended); // 0 for right, 1 for left
            i.putExtra("NRTBC_EACH", nrtbc_each); // 5 each means 10 reps total, and max score of 10.
            i.putExtra("JSON_DATA",json_data);
            mContext.startActivityForResult(i, requestCode);
        } else {
            showToast("Error: That game does not exist.");
        }
    }

    @JavascriptInterface
    public void setGameResolution(String resolutionStr) {
        CVActivity.useMaxResolution = resolutionStr.trim().equalsIgnoreCase("3");
    }


    @JavascriptInterface
    public void setDisplayExtraInfo(int displayExtraInfoAmt) {
        CVActivity.setDisplayExtraInfo(displayExtraInfoAmt);
    }

    @JavascriptInterface
    public void setGameSoundsEnabled(int playGameSounds) {
        CVActivity.setGameSoundsEnabled(playGameSounds);
    }


    @JavascriptInterface
    public int getVolumeLevel() {
        AudioManager am = (AudioManager) mContext.getSystemService(android.content.Context.AUDIO_SERVICE);
        int volume_level= am.getStreamVolume(AudioManager.STREAM_MUSIC);
        // STREAM_MUSIC gets the media volume. different STREAM_<type> gets different volume
        // works for both bluetooth connected speaker and just in general
        // on the note 8, volume_level goes from 0 to 15 (with 0 being quiet)
        // just as an addition, im not sure if there is another way to mute the media volume without setting this to 0
        return volume_level;
        //return 0;
    }
    @JavascriptInterface
    public double getVolumeLevelRatio() {
        AudioManager am = (AudioManager) mContext.getSystemService(android.content.Context.AUDIO_SERVICE);
        int volume_level = am.getStreamVolume(AudioManager.STREAM_MUSIC);
        int volume_level_max = am.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
        // STREAM_MUSIC gets the media volume. different STREAM_<type> gets different volume
        // works for both bluetooth connected speaker and just in general
        // on the note 8, volume_level goes from 0 to 15 (with 0 being quiet)
        // just as an addition, im not sure if there is another way to mute the media volume without setting this to 0
        return volume_level * 1.0/ volume_level_max;
    }


    @JavascriptInterface
    public String getCameraResolutionOptions() {
        if (ContextCompat.checkSelfPermission(mContext, Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) return null;
        CameraManager cameraManager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);

        try { //needs try catch in case something goes wrong with the camera
            for (String cameraId : cameraManager.getCameraIdList()) { //searching for the correct camera
                CameraCharacteristics cameraCharacteristics
                        = cameraManager.getCameraCharacteristics(cameraId);
                //widthHeight = getPhoneDims(cameraCharacteristics);
                Integer facing = cameraCharacteristics.get(CameraCharacteristics.LENS_FACING);
                if (facing != null && facing == CameraCharacteristics.LENS_FACING_BACK) {
                    if (CVActivity.useBackCamera == 1 && facing == CameraCharacteristics.LENS_FACING_BACK) { // LENS_FACING_BACK is 1
                        continue; //if it is the back facing camera skip it, we want the front facing camera
                    }
                    if (CVActivity.useBackCamera == 0 && facing == CameraCharacteristics.LENS_FACING_FRONT) { // LENS_FACING_FRONT is 0
                        continue; //if it is the front facing camera skip it, we want the back facing camera
                    }
                }
                StreamConfigurationMap map = cameraCharacteristics.get(
                        CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                if (map == null) {
                    continue;
                }
                //return null;
                Size[] choices = map.getOutputSizes(SurfaceTexture.class);

                Point displaySize = new Point();
                mContext.getWindowManager().getDefaultDisplay().getSize(displaySize);
                int maxPreviewWidth = displaySize.x;
                int maxPreviewHeight = displaySize.y;

                boolean swappedDimensions = true; //swapped (true) means portrait. unswapped (false) means landscape
                if (swappedDimensions) {
                    maxPreviewWidth = displaySize.y;
                    maxPreviewHeight = displaySize.x;
                }

                if (maxPreviewWidth > CVActivity.MAX_PREVIEW_WIDTH) {
                    maxPreviewWidth = CVActivity.MAX_PREVIEW_WIDTH;
                }
                if (maxPreviewHeight > CVActivity.MAX_PREVIEW_HEIGHT) {
                    maxPreviewHeight = CVActivity.MAX_PREVIEW_HEIGHT;
                }
                Size lowResChoiceNormDimensions = CVActivity.chooseOptimalSize(choices, maxPreviewWidth, maxPreviewHeight, false);
                Size highResChoiceNormDimensions = CVActivity.chooseOptimalSize(choices, maxPreviewWidth, maxPreviewHeight, true);


                swappedDimensions = true; //swapped (true) means portrait. unswapped (false) means landscape
                if (swappedDimensions) {
                    maxPreviewWidth = displaySize.y;
                    maxPreviewHeight = displaySize.x;
                }

                if (maxPreviewWidth > CVActivity.MAX_PREVIEW_WIDTH) {
                    maxPreviewWidth = CVActivity.MAX_PREVIEW_WIDTH;
                }
                if (maxPreviewHeight > CVActivity.MAX_PREVIEW_HEIGHT) {
                    maxPreviewHeight = CVActivity.MAX_PREVIEW_HEIGHT;
                }
                Size lowResChoiceSwapDimensions = CVActivity.chooseOptimalSize(choices, maxPreviewWidth, maxPreviewHeight, false);
                Size highResChoiceSwapDimensions = CVActivity.chooseOptimalSize(choices, maxPreviewWidth, maxPreviewHeight, true);


                /*
                String toReturn = "ht, wd\n";
                for (Size option : choices) {
                    toReturn += ""+option.getHeight() + ", " + option.getWidth() + "\n";
                }
                toReturn = "[" +toReturn.substring(1) + "]"; //do .substring(1) to ignore leading comma, which throws an error
                */
                String sizeOptionsJSON = "";
                for (Size option : choices) {
                    sizeOptionsJSON += ", " + sizeToJSON(option);
                }
                sizeOptionsJSON = "[" +sizeOptionsJSON.substring(1) + "]"; //do .substring(1) to ignore leading comma, which throws an error

                String toReturnJSON = "{\"sizeOptions\":"+sizeOptionsJSON+", \"defaultSizes\":{\"lowResolution\":{\"portrait\":"  + sizeToJSON(lowResChoiceNormDimensions)
                        + ", \"landscape\":" + sizeToJSON(lowResChoiceSwapDimensions)
                        + "}, \"highResolution\":{\"portrait\":"+sizeToJSON(highResChoiceNormDimensions)
                        + ", \"landscape\":"+sizeToJSON(highResChoiceSwapDimensions)
                        +"}}}";

                return toReturnJSON;

            }


        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        return null;
    }

    private String sizeToJSON(Size option) {
        return "{\"ht\":"+option.getHeight() + ", \"wd\":" + option.getWidth() + "}";
    }


    @JavascriptInterface
    public void subscribeToTopic(String topic) {
        FirebaseMessaging.getInstance().subscribeToTopic(topic)
            .addOnCompleteListener(new OnCompleteListener<Void>() {
                @Override
                public void onComplete(@NonNull Task<Void> task) {
                String msg = mContext.getString(R.string.msg_subscribed);
                if (!task.isSuccessful()) {
                    msg = mContext.getString(R.string.msg_subscribe_failed);
                }
                Log.d(TAG, msg);
                }
            });
    }

    @JavascriptInterface
    public void getAndSaveFirebaseRegistrationToken() {
        FirebaseInstanceId.getInstance().getInstanceId()
            .addOnCompleteListener(new OnCompleteListener<InstanceIdResult>() {
                @Override
                public void onComplete(@NonNull Task<InstanceIdResult> task) {
                if (!task.isSuccessful()) {
                    Log.w(TAG, "getInstanceId failed", task.getException());
                    return;
                }

                // Get new Instance ID token
                String token = task.getResult().getToken();
                saveFirebaseRegistrationToken(token);

                // Log and toast
                String msg = mContext.getString(R.string.msg_token_fmt, token);
                Log.d(TAG, msg);
                }
            });
    }

    private void saveFirebaseRegistrationToken(String token) {
        String run = "saveFirebaseRegistrationToken(`" + token + "`, true);";
        getWebView().loadUrl(run);
    }


    @JavascriptInterface
    public void unsubscribeFromTopic(String topic) {
        FirebaseMessaging.getInstance().unsubscribeFromTopic(topic)
            .addOnCompleteListener(new OnCompleteListener<Void>() {
                @Override
                public void onComplete(@NonNull Task<Void> task) {
                String msg = mContext.getString(R.string.msg_unsubscribed);
                if (!task.isSuccessful()) {
                    msg = mContext.getString(R.string.msg_unsubscribe_failed);
                }
                Log.d(TAG, msg);
                }
            });
    }
    //*/

    public void onCVActivityResult(int requestCode, int resultCode, Intent data) {
        int exerciseGame = requestCode - minCVActivityRequestCode;
        double game_score = -20;
        int nrtbc_each = -20;
        int nrc_tot = -20;

        String errorStr = null;
        if (data != null) {
            if(resultCode == Activity.RESULT_OK) {
                game_score = data.getDoubleExtra("CVACTIVITY_GAMESCORE", -10);
                nrtbc_each = data.getIntExtra("NRTBC_EACH", -5); //multiply by two because we need to factor in that there are two "exercises" in each game/exercise
                nrc_tot = data.getIntExtra("CVACTIVITY_NRC_TOT", -10);
            } else if(resultCode == Activity.RESULT_CANCELED)  { //this is what happens if you press the back button
                game_score = data.getDoubleExtra("CVACTIVITY_GAMESCORE", -10);
                nrtbc_each = data.getIntExtra("NRTBC_EACH", -5); //multiply by two because we need to factor in that there are two "exercises" in each game/exercise
                nrc_tot = data.getIntExtra("CVACTIVITY_NRC_TOT", -10);
            }
            //if CVACTIVITY_ERROR does not exist, then it will return null
            errorStr = data.getStringExtra("CVACTIVITY_ERROR");
        }

        if (errorStr == null) { //will happen if the data intent is null or CVACTIVITY_ERROR is not a valid key
            String run = "javascript:updateAfterGameFinished(" + exerciseGame + "," + ((int) game_score) + "," + nrc_tot + "," + nrtbc_each + ")";
            getWebView().loadUrl(run);

        } else {
            String errorStrReadable = errorStr.replace("\\","\\\\'").replace("'","\\'").trim();
            String run = "javascript:updateAfterGameFinishedWithError(" + exerciseGame + "," + ((int) game_score) + "," + nrc_tot + "," + nrtbc_each + ", '" + errorStrReadable + "')";
            getWebView().loadUrl(run);

        }



    }


    /*@JavascriptInterface
    public void setUsername(String new_username) {
        username = new_username;
    }

    @JavascriptInterface
    public void attemptLogin(String username, String password) {

        Task<AuthResult> tr = getAuth().signInWithEmailAndPassword(username, password);
        //The tutorial uses this specific addOnCompleteListener method with the Activity method. Googling says it is useful because then
        //"These listeners are removed during the onStop method of your Activity so that your listeners are not called when the Activity is no longer visible."
        tr.addOnCompleteListener(mContext, new OnCompleteListener<AuthResult>() {
                @Override
                public void onComplete(@NonNull Task<AuthResult> task) {
                    if (task.isSuccessful()) {
                        // Sign in success, update UI with the signed-in user's information
                        Log.d(TAG, "signInWithEmail:success");
                        getWebView().loadUrl("javascript:updateAfterSuccessfulLogin()");
                    } else {
                        // If sign in fails, display a message to the user.
                        Log.w(TAG, "signInWithEmail:failure", task.getException());
                        getWebView().loadUrl("javascript:updateAfterFailedLogin()");
                    }
                }
            });
    }

    @JavascriptInterface
    public void removeUsername() {
        username = null;
    }

    @JavascriptInterface
    public void logoutOfFirebase() {
        FirebaseAuth.getInstance().signOut();
    }

    @JavascriptInterface
    public String getUsername() {
        if (getAuth().getCurrentUser() == null) {
            return username == null ? "undef" : username;
        } else {
            return getAuth().getCurrentUser().getDisplayName();
        }
    }*/
}
