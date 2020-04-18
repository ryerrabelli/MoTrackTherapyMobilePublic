package com.motracktherapy.motrack;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.support.annotation.NonNull;
import android.support.v4.app.NotificationCompat;
import 	android.app.PendingIntent;
import 	android.support.v4.app.NotificationManagerCompat;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import 	java.util.Calendar;
import android.app.AlarmManager;
//import com.google.firebase.auth.FirebaseAuth;

import com.google.firebase.iid.*;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
//import com.google.firebase.iid.FirebaseInstanceId;
//import com.google.firebase.iid.InstanceIdResult;
//import com.google.firebase.messaging.FirebaseMessaging;
//import com.google.firebase.quickstart.fcm.R;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;


public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    private WebView mWebView;
    boolean answerCorrect = false;
    private int picturesCompleted = 0;
    private String masterKeyString = ("abrahamlincoln");
    private boolean[] tilePicked = new boolean[9];
    private int turnsLeft = 20;
    List<String> masterKeyArray = new ArrayList<String>();
    private int numberOfNames = masterKeyArray.size();
    private boolean[] namePicked = new boolean[masterKeyArray.size()];
    private WebAppInterface mWebAppInterface = null;




    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("opencv_java3");
        System.loadLibrary("motion_info");
        System.loadLibrary("bilinear_approx");
        System.loadLibrary("fist_analysis");
        System.loadLibrary("calibration");
        System.loadLibrary("audio_player");
    }

    //private FirebaseAuth mAuth;
    //public FirebaseAuth getFirebaseAuth() { return mAuth; }
    public WebView getWebView() {
        return mWebView;
    }

    int minCVActivityRequestCode = 1;

    /**
     * This is run when the app starts
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //mAuth = FirebaseAuth.getInstance();

        mWebView = findViewById(R.id.activity_main_webview);

        // Enable Javascript
        WebSettings webSettings = mWebView.getSettings();
        webSettings.setJavaScriptEnabled(true);

        mWebAppInterface = new WebAppInterface(this,minCVActivityRequestCode);
        mWebView.addJavascriptInterface(mWebAppInterface, "Android");
        //required for autologin persistence between each times (i.e. html app will autologin the
        // user if the user logged in before even if the last log in was before the app was quit)
        mWebView.getSettings().setDomStorageEnabled(true);
        mWebView.setWebViewClient(new CustomWebViewClient(this));
        mWebView.loadUrl("file:///android_asset/appui.html");

        Button devMode = findViewById(R.id.button3);
        devMode.setEnabled(false);
        devMode.setVisibility(View.INVISIBLE);
        Button button5 = findViewById(R.id.button5);
        button5.setEnabled(false);
        button5.setVisibility(View.INVISIBLE);
        EditText textbox = findViewById(R.id.userAnswer);
        textbox.setEnabled(false);
        textbox.setVisibility(View.INVISIBLE);

   }

    //Two functions below are called on button clicks
    public void openTile(View view)
    {
        //fillMasterKeyArray();
        //loadMasterKey();
        revealPicture();
    }

    public void checkAnswer(View view)
    {
        EditText userInputBox = findViewById(R.id.userAnswer);
        userInput(userInputBox.getText().toString());
    }

    /**
     * WebView has a method canGoBack which tells you if there is anything on the page stack that
     * can be popped. Here, detect a back button press and determine if you should step back through
     * the WebView's history or allow the platform to determine the correct behaviour.
     */
    @Override
    public void onBackPressed() {
        if (mWebView.canGoBack()) {
            mWebView.goBack();
        } else {
            super.onBackPressed();
        }
    }

    /**
     * following method inputs a string which is given by the user,
     * and outputs the same string condensed into a one word, all lowercase, string, without any punctation.
     * <p>
     * It does this by scanning each character of the input string with a for loop,
     * and placing each letter the loop encounters into a character array which is then condensed back into a single string,
     * ready to be compared to the "master key" string
     */
    public static String removeAndLowerCase(String inputString) {
        inputString.trim();
        String changedString = inputString.toLowerCase();
        char[] stringSorter = new char[inputString.length()];
        int stringLengthWithoutInvalid = 0;

        for (int i = 0; i < changedString.length(); i++) {
            if (Character.isLetter(changedString.charAt(i))) {
                stringSorter[stringLengthWithoutInvalid] = changedString.charAt(i);
                stringLengthWithoutInvalid++;
            }
        }
        String finalString = String.copyValueOf(stringSorter, 0, stringLengthWithoutInvalid);
        return finalString;
    }
    public void revealPicture() {
        turnsLeft--;
        if (turnsLeft == -1)
        {
            endGame();
        }

        else
        {
            int tile = getRandom(9, tilePicked) + 1;
            if (tile < 10)
            {
                mWebView.loadUrl("http://www.motracktherapy.com/abe" + tile + "/");///game-testing");
            }
        }
    }


    /**
     * the following method is in charge of checking the input given by the user through a textbox,
     * as well as deciding whether or not this text is correct or not
     * <p>
     * It does this by running the removeAndLowerCase method created earlier on the users input string,
     * then running it through a simple If statement to check the characters with the masterKeyString which corresponds with the photo
     * on the users screen.
     */
    public void userInput(String userInput) {
        String inputString;
        inputString = removeAndLowerCase(userInput);
        if (inputString.equals(masterKeyString)) {
            answerCorrect = true;
            picturesCompleted++;
            mWebView.loadUrl("http://www.motracktherapy.com/abe9/");
            //loadMasterKey(); // will load new image with new associated master key
        } else {
            answerCorrect = false;
            mWebView.loadUrl("http://www.motracktherapy.com/abe0/"); //TODO add real penalty to missing a picture
            Log.v("debug", masterKeyString);
        }
    }
    public void endGame() {
        //TODO find ways of displaying score or ask Anita to create another webpage to redirect to

        mWebView.loadUrl("http://www.motracktherapy.com/abe0/");///game-testing");
        Button devMode = findViewById(R.id.button3);
        devMode.setEnabled(false);
        devMode.setVisibility(View.INVISIBLE);

        Button button5 = findViewById(R.id.button5);
        button5.setVisibility(View.INVISIBLE);
    }

    public void loadMasterKey() {
        //masterKeyString = masterKeyArray.get(getRandom(numberOfNames, namePicked)); //Will crash program if masterKeyArray is not filled (null entity)
        Log.d("debug", masterKeyString);
        mWebView.loadUrl("http://www.motracktherapy.com/abe9/"); // TODO will fix so this redirects to the proper image
    }


    public void fillMasterKeyArray()
    {
            //Thread t1 = new Thread(new MainActivity());
            //t1.start();
            //InputStreamReader inputStreamReader = new InputStreamReader(is);
            //BufferedReader br = new BufferedReader(inputStreamReader);

            //if (br.ready())
            {
                //while ((tempString = br.readLine()) != null)
                {
                    //masterKeyArray.add(tempString);
                    //Log.d("debug", tempString);
                }
                //br.close();
            }

            //else
            {
                //Log.d("debug", "Buffered reader error, not ready");
                //mWebView.loadUrl("http://www.motracktherapy.com");
            }



    }

   // public void run()
    {
        /*
        try {
            String tempString;

            URL url = new URL("http://www.motracktherapy.com/names.txt"); //TODO PROBLEM HERE URL connection issues
            HttpURLConnection uc = (HttpURLConnection) url.openConnection();//might cause problems as the address is an http, not https, unsure for now
            InputStream is = uc.getInputStream();
            Scanner scanner = new Scanner(is);

            while ((tempString = scanner.next()) != null) {
                masterKeyArray.add(tempString);
                Log.d("debug", tempString);
            }
            scanner.close();

        } catch (MalformedURLException e) {
            Log.d("debug", "MalformedURLException occurred");
        } catch (IOException e) {
            Log.d("debug", "IOException occurred");
        }
        */
    }

    public int getRandom (int numberOfEntities, boolean[] entityPicked)
    {
        int result = numberOfEntities;
        boolean moreThanOneTileOpen = false;

        for (int i = 0; i < numberOfEntities; i++) {
            if (entityPicked[i] == false) {
                if (result != 9) {
                    moreThanOneTileOpen = true;
                    break;
                }
                result = i;
            }
        }

        if (moreThanOneTileOpen == false || result == numberOfEntities) {
            return result;
        }

        Random rand = new Random();
        while (true)
        {
            result = rand.nextInt(8);
            if (!entityPicked[result])
            {
                entityPicked[result] = true;
                return result;
            }
        }

    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode >= minCVActivityRequestCode) {
            mWebAppInterface.onCVActivityResult(requestCode, resultCode, data);
        } else {
            // leave this here for any startActivityForResult not called by WebAppInterface
            // at this point, there are none (and I don't really anticipate why they would be here
            // in the near future, so it should not get here. Thus, throw an exception.
            throw new RuntimeException("Error: Invalid requestCode of " + requestCode);
        }
    }



    /* old way

    public void sendNotification(View view) {

        //Get an instance of NotificationManager//

        NotificationCompat.Builder mBuilder = new NotificationCompat.Builder(this, CHANNEL_ID)
                .setSmallIcon(R.drawable.singleletterlogo_384)
                .setContentTitle("My notification")
                .setContentText("Much longer text that cannot fit one line...")
                .setStyle(new NotificationCompat.BigTextStyle()
                        .bigText("Much longer text that cannot fit one line..."))
                .setPriority(NotificationCompat.PRIORITY_DEFAULT);


        // Gets an instance of the NotificationManager service//

        NotificationManager mNotificationManager =

                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

        // When you issue multiple notifications about the same type of event,
        // it’s best practice for your app to try to update an existing notification
        // with this new information, rather than immediately creating a new notification.
        // If you want to update this notification at a later date, you need to assign it an ID.
        // You can then use this ID whenever you issue a subsequent notification.
        // If the previous notification is still visible, the system will update this existing notification,
        // rather than create a new one. In this example, the notification’s ID is 001//

        NotificationManager.notify().

                mNotificationManager.notify(001, mBuilder.build());
    }*/


}
