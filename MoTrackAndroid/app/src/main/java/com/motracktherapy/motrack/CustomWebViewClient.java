package com.motracktherapy.motrack;

//This class is use to do the web view

import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import java.util.List;

/**
 * Created by adamp on 1/16/2018.
 * Taken from ryerrabelli
 */

public class CustomWebViewClient extends WebViewClient {

    private Context mContext;

    public CustomWebViewClient(Context context) {
        this.mContext = context;
    }

    @Override
    public WebResourceResponse shouldInterceptRequest(WebView view, String resourceUrl) {
        //String resourceUrl = request.getUrl().toString();
        final Uri request = Uri.parse(resourceUrl);
        System.out.println("Webview Interface: Received URL request for " + resourceUrl);
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
        //This function will be called whenever the webview tries to go a new location (it doesn't activate when appui.html is first loaded)
        final String host = uri.getHost();
        final String scheme = uri.getScheme();
        System.out.println("Webview Interface: host: " + host);
        System.out.println("Webview Interface: scheme: " + scheme);

        //the intent will let the user open the link in a different app (will offer different apps based off the scheme, for example tel will give the phone app)
        //put a true in the beginning because I wanted a catchall for any future schemes we use and I didn't find an example when this
        //method is called when didn't want the user to open it in another app
        if (true || scheme.equalsIgnoreCase("http") ||
                scheme.equalsIgnoreCase("https") ||
                scheme.equalsIgnoreCase("tel") ||
                scheme.equalsIgnoreCase("mailto")) {
            Intent intent = new Intent(Intent.ACTION_VIEW, uri);
            mContext.startActivity(intent);
            return true;
        }

        //returning false will open the link within the webview
        //return false;
        return true;
    }
}
