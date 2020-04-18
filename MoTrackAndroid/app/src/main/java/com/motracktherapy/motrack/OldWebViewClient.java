package com.motracktherapy.motrack;

//This class used to do the web view

/***
 * WARNING: THIS FILE IS NO LONGER USED
 * ***/

import android.annotation.TargetApi;
import android.content.Intent;
import android.content.Context;
import android.net.Uri;
import android.os.Build;
import android.util.Log;
import android.webkit.*;
import java.util.List;
import static android.support.v4.content.ContextCompat.startActivity;

/**
 * Created by adamp on 1/16/2018.
 * Taken from ryerrabelli
 */

public class OldWebViewClient extends WebViewClient {

    private Context context;

    public OldWebViewClient(Context context) {
        this.context = context;
    }

    @Override
    public WebResourceResponse shouldInterceptRequest(WebView view, String resourceUrl) {
        //String resourceUrl = request.getUrl().toString();
        final Uri request = Uri.parse(resourceUrl);
        return super.shouldInterceptRequest(view,resourceUrl);
    }

    /// ********** CHOOSE EXTERNAL OR INTERNAL OPENING OF WEBPAGE **********
    //This method is for newer android os versions See https://stackoverflow.com/questions/
    // 36484074/is-shouldoverrideurlloading-really-deprecated-what-can-i-use-instead
    @SuppressWarnings("deprecation")
    @Override
    public boolean shouldOverrideUrlLoading(WebView view, String url) {
        final Uri uri = Uri.parse(url);
        return handleUri(view, uri);
    }

    //This method is for newer android os versions See https://stackoverflow.com/questions/
    // 36484074/is-shouldoverrideurlloading-really-deprecated-what-can-i-use-instead
    @TargetApi(Build.VERSION_CODES.N)
    @Override
    public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
        final Uri uri = request.getUrl();
        return handleUri(view, uri);
    }

    //Default implementation (if shouldOverrideUrlLoading wasn't overriden) is always returns false
    private boolean handleUri(WebView view, final Uri uri) {
        final String host = uri.getHost();
        final String scheme = uri.getScheme();
        // You can use `host` or `scheme` or any part of the `uri` to decide.
        if (uri.getHost().endsWith("motracktherapy.com")) {

            //Makes a list of each extension of the url
            List<String> pathSegments = uri.getPathSegments();

            //check if motracktherapy.com/phonedev/main-menu/tutorial/, opens CVActivity if it is
            if (pathSegments.size() > 2
                    && pathSegments.get(0).equalsIgnoreCase("phonedev")
                    && pathSegments.get(1).equalsIgnoreCase("main-menu")
                    && pathSegments.get(2).equalsIgnoreCase("tutorial")) {
                // Returning true means that you need to handle what to do with the url
                // in this case, open the cv app
                Intent i = new Intent(context, CVActivity.class);
                context.startActivity(i);
                return false;
            }

            if (pathSegments.size() > 3
                    && pathSegments.get(0).equalsIgnoreCase("phonedev")
                    && pathSegments.get(1).equalsIgnoreCase("main-menu")
                    && pathSegments.get(2).equalsIgnoreCase("exercise-page")
                    && pathSegments.get(3).equalsIgnoreCase("learn-your-exercises")) {
                // Returning true means that you need to handle what to do with the url
                // in this case, open the cv app
                Intent i = new Intent(context, CVActivity.class);
                context.startActivity(i);
                return false;
            }

            // Returning false means that you are going to load this url in the webView itself
            return false;
        } else {
            /*
            // Returning true means that you need to handle what to do with the url
            // in this case, open web page in an actual browser (will ask user which browser if multiple installed)
            final Intent intent = new Intent(Intent.ACTION_VIEW, uri);
            view.getContext().startActivity(intent);
            return true;
            */

            Intent i = new Intent(context, CVActivity.class);
            context.startActivity(i);
            return false;
        }
    }

}
