//
//  WebViewController.swift
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 12/25/18.
//  Recreated by Rahul Yerrabelli on 2/13/19.
//  Copyright © 2018/2019 MoTrack Therapy. All rights reserved.
//

import UIKit
import WebKit
import Firebase
import Crashlytics



// For getting the webview, I used: https://developer.apple.com/documentation/webkit/wkwebview
// For use of a local file, I used: https://stackoverflow.com/questions/49638653/load-local-web-files-resources-in-wkwebview
//For accessing swift code (like to start the games) from javascript, I used: https://medium.com/@hoishing/using-javascript-with-wkwebview-64f94153ad0

class WebViewController: UIViewController, WKUIDelegate, WKNavigationDelegate, WKScriptMessageHandler {
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) {
        //IMPORTANT NOTE: If you add another function here, you must also add it to viewDidLoad so the following function is called webView.configuration.userContentController.add(.)
        if message.name == "jsHandler" {
            print(message.body)
            print("hello")
            
        } else if message.name == "setDisplayExtraInfo" {
            let bd = message.body
            let displayExtraInfoNum = (bd as! NSNumber).intValue
            OpenCVWrapper.setDisplayExtraInfoObjC(Int32(displayExtraInfoNum));
            
        } else if message.name == "setGameSoundsEnabled" {
            let bd = message.body
            let playGameSounds = (bd as! NSNumber).intValue
            OpenCVWrapper.setGameSoundsEnabledObjC(Int32(playGameSounds));
            
        } else if message.name == "startGame" {
            /*
            //downcast message.body to type Int (could by type string or others, but I made sure the javascript only sends an int)
            let bd = message.body
            let combinedNum = (bd as! NSNumber).intValue //last two digits are nrtbc_each, two digits before that are which_hand_intended, and anything before that are exerciseGame. EX 1500110
            let nrtbcEach = combinedNum % 100
            let whichHandIntended = ((combinedNum-nrtbcEach)/100) % 100;
            let exerciseGame = (combinedNum-whichHandIntended*100-nrtbcEach)/10000
            
            print("exerciseGame will become " + String(exerciseGame))
            Analytics.logEvent("start_game", parameters: [
                AnalyticsParameterQuantity: nrtbcEach,
                AnalyticsParameterItemID: exerciseGame,
                "hand": whichHandIntended,
                ])
            
            let orientationGame = Int( OpenCVWrapper.getOrientationObjC(Int32(exerciseGame)) );
            OpenCVWrapper.setExerciseGame(Int32(exerciseGame), andOrientationGame: Int32(orientationGame))
            
            let secondViewController = self.storyboard?.instantiateViewController(withIdentifier: "CVView")
            //force downcast to CVViewController
            (secondViewController as! CVViewController).nrtbcEach = nrtbcEach;
            (secondViewController as! CVViewController).exerciseGame = exerciseGame;
            (secondViewController as! CVViewController).whichHandIntended = whichHandIntended;
            (secondViewController as! CVViewController).orientationGame = orientationGame;
            //self.show(secondViewController!, sender: nil)
            self.present(secondViewController as! CVViewController, animated: true, completion: nil)
            */
            
            
            
            guard let body = message.body as? [String: Any] else { return }
            guard let command = body["command"] as? String else { return }
            guard let nrtbcEach = body["nrtbcEach"] as? Int else { return }
            guard let whichHandIntended = body["whichHandIntended"] as? Int else { return }
            guard let exerciseGame = body["exerciseGame"] as? Int else { return }
            guard let jsonDataStr = body["jsonDataStr"] as? String else { return }
            //allow photo saving (allowPhotoSaving) can't be part of jsonDataStr because it is needed in Swift
            guard let allowPhotoSaving = body["allowPhotoSaving"] as? Bool else { return }
            
            
            print("exerciseGame will become " + String(exerciseGame))
            Analytics.logEvent("start_game", parameters: [
                AnalyticsParameterQuantity: nrtbcEach,
                AnalyticsParameterItemID: exerciseGame,
                "hand": whichHandIntended,
                ])
            
            let orientationGame = Int( OpenCVWrapper.getOrientationObjC(Int32(exerciseGame)) );
            OpenCVWrapper.setExerciseGame(Int32(exerciseGame), andOrientationGame: Int32(orientationGame))
            
            let secondViewController = self.storyboard?.instantiateViewController(withIdentifier: "CVView")
            //force downcast to CVViewController
            (secondViewController as! CVViewController).nrtbcEach = nrtbcEach;
            (secondViewController as! CVViewController).exerciseGame = exerciseGame;
            (secondViewController as! CVViewController).whichHandIntended = whichHandIntended;
            (secondViewController as! CVViewController).orientationGame = orientationGame;
            (secondViewController as! CVViewController).jsonDataStr = jsonDataStr;
            (secondViewController as! CVViewController).allowPhotoSaving = allowPhotoSaving;
            //self.show(secondViewController!, sender: nil)
            self.present(secondViewController as! CVViewController, animated: true, completion: nil)
            
            
        } else if message.name == "MoTrackTherapy" {
            guard let body = message.body as? [String: Any] else { return }
            guard let command = body["command"] as? String else { return }
            
            if command == "displayBackground" {
                
            } else if command == "startLightTutorial" {
                let secondViewController = self.storyboard?.instantiateViewController(withIdentifier: "TutorialView")
                //self.show(secondViewController!, sender: nil)
                self.present(secondViewController as! TutorialViewController, animated: true, completion: nil)
                
            } else if command == "setGameResolution" {
                guard let name = body["resolutionStr"] as? String else { return }
                
            } else if command == "getAndSaveFirebaseRegistrationToken" {
                InstanceID.instanceID().instanceID { (result, error) in
                    if let error = error {
                        print("Error fetching remote instance ID: \(error)")
                    } else if let result = result {
                        print("Remote instance ID token: \(result.token)")
                        self.saveFirebaseRegistrationToken(token:result.token);
                    }
                }
                
            } else if command == "subscribeToTopic" {
                guard let topic = body["topic"] as? String else { return }
                Messaging.messaging().subscribe(toTopic: topic) { error in
                    print("Subscribed to '"+topic+"' topic")
                }
            } else if command == "unsubscribeFromTopic" {
                guard let topic = body["topic"] as? String else { return }
                Messaging.messaging().unsubscribe(fromTopic: topic) { error in
                    print("Unsubscribed to '"+topic+"' topic")
                }
                
            } else if command == "crashAppPurposefully" {
                Crashlytics.sharedInstance().crash();
                
            } else if command == "getDeviceType" {
                //return UIDevice.current.model;
                
            } else if command == "print" {
                guard let info = body["info"] else {
                    print("Couldn't print out requested info");
                    return;
                }
                print(info)
            }
            
        } else if message.name == "setCurrentScreen" {
            Analytics.setScreenName(message.body as? String, screenClass: "webview")

        } else if message.name == "firebase" {
            //"String: Any" means dictionary type with input String and output Any
            guard let body = message.body as? [String: Any] else { return }
            guard let command = body["command"] as? String else { return }
            
            if command == "setUserID" {
                guard let userID = body["userID"] as? String else { return }
                //Note in swift and js, it is setUserID, but in Java, it is setUserId
                Analytics.setUserID(userID)
                
                //"If you ever need to clear a user identifier after you set it, reset the value to a blank string."
                //Not converting null to "" because I don't know how to do it in swift, and I want to keep Android/iOS the same
                Crashlytics.sharedInstance().setUserIdentifier(userID)
                
            } else if command == "setUserProperty" {
                guard let name = body["name"] as? String else { return }
                guard let value = body["value"] as? String else { return }
                Analytics.setUserProperty(value, forName: name)
            } else if command == "logEvent" {
                guard let name = body["name"] as? String else { return }
                guard let params = body["parameters"] as? [String: NSObject] else { return }
                Analytics.logEvent(name, parameters: params)
            }
            
        }
    }
    
    func saveFirebaseRegistrationToken(token:String) {
        //self.instanceIDTokenMessage.text  = "Remote InstanceID token: \(result.token)"
        let scriptString = "saveFirebaseRegistrationToken(`\(token)`, true);";
        webView?.evaluateJavaScript(scriptString, completionHandler:nil)
    }
    
    func updateAfterGameFinished(exerciseGame:Int, gameScore:Double, repsCompletedTot:Int, nrtbcEach:Int) {
        let delegate = UIApplication.shared.delegate as! AppDelegate
        saveAPNToken(apnToken: delegate.apnToken)
        
        //Note: if I ever want to put JSON into the javascript line, there are easier ways of doing it than just putting it directly as a string
        //below syntax of strings and numbers is called string interpolation
        let scriptString = "updateAfterGameFinished(\(exerciseGame), \(gameScore), \(repsCompletedTot), \(nrtbcEach));";
        webView?.evaluateJavaScript(scriptString, completionHandler:nil)
    }
    func updateAfterGameFinishedWithError(exerciseGame:Int, gameScore:Double, repsCompletedTot:Int, nrtbcEach:Int, errorStr:String) {
        //Note: if I ever want to put JSON into the javascript line, there are easier ways of doing it than just putting it directly as a string
        //below syntax of strings and numbers is called string interpolation
        let scriptString = "updateAfterGameFinishedWithError(\(exerciseGame), \(gameScore), \(repsCompletedTot), \(nrtbcEach), `\(errorStr)`);";
        webView?.evaluateJavaScript(scriptString, completionHandler:nil)
    }
    
    
    func navigateTo(toPage:String) {
        let scriptString = "navigateTo('"+toPage+"')";
        webView?.evaluateJavaScript(scriptString, completionHandler:nil)
    }
    
    func saveAPNToken(apnToken:String?) {
        //Note: if I ever want to put JSON into the javascript line, there are easier ways of doing it than just putting it directly as a string
        //below syntax of strings and numbers is called string interpolation
        var scriptString:String
        if apnToken == nil {
            scriptString = "saveAPNToken( '' );";
        } else {
            scriptString = "saveAPNToken( '\(apnToken as! String)' );";
        }
        
        webView?.evaluateJavaScript(scriptString, completionHandler:nil)
    }
    
    func saveFirebaseRegistrationToken(apnToken:String?) {
        var scriptString:String
        if apnToken == nil {
            scriptString = "saveFirebaseRegistrationToken( '' );";
        } else {
            scriptString = "saveFirebaseRegistrationToken( '\(apnToken as! String)' );";
        }
        
        webView?.evaluateJavaScript(scriptString, completionHandler:nil)
    }
    
    var webView: WKWebView!
    var webViewHasBeenSetUp = false;
    var useLocalFile = true;
    
    override func loadView() {
        if (!webViewHasBeenSetUp) {
            let webConfiguration = WKWebViewConfiguration()
            //Source: https://stackoverflow.com/questions/43885705/how-to-play-video-inline-with-wkwebview, https://stackoverflow.com/a/39051722
            webConfiguration.allowsInlineMediaPlayback = true //this and playsinline/webkit-playsinline in html makes videos not play in fullscreen
            webView = WKWebView(frame: .zero, configuration: webConfiguration)
            webView.uiDelegate = self
            view = webView
        }
        
    }
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        if (!webViewHasBeenSetUp) {
            webView.navigationDelegate = self
            webView.configuration.userContentController.add(self, name: "jsHandler")
            webView.configuration.userContentController.add(self, name: "startGame")
            webView.configuration.userContentController.add(self, name: "setDisplayExtraInfo")
            webView.configuration.userContentController.add(self, name: "setGameSoundsEnabled")
            webView.configuration.userContentController.add(self, name: "setCurrentScreen")
            self.webView.configuration.userContentController.add(self, name: "firebase")
            webView.configuration.userContentController.add(self, name: "MoTrackTherapy")

            
            var myURL:URL;
            if (useLocalFile) {
                //use below line instead for an app signup version
                //myURL = Bundle.main.url(forResource: "appsignup", withExtension: "html", subdirectory: "assets")!
                myURL = Bundle.main.url(forResource: "appui", withExtension: "html", subdirectory: "assets")!
                webView.loadFileURL(myURL, allowingReadAccessTo: myURL)
            } else {
                //myURL = URL(string: "https://motracktherapy.com")! //http://localhost:5000/appui.html
                //myURL = URL(string: "http://localhost")!
                myURL = URL(string: "https://motracktherapymobileapp.firebaseapp.com/appui.html")!
                
            }
            let myRequest = URLRequest(url: myURL)
            webView.load(myRequest)
            webViewHasBeenSetUp = true
        }
    }
    
    //This doesn't actually appear to get called for some reason
    override func didReceiveMemoryWarning() {
        print("MEMORY WARNING 1")
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated
    }
    
    //What to do after webpage has loaded (need to do some of this stuff here instead of after webView.load, otherwise it won't work)
    func webView(_ myWebView: WKWebView, didFinish navigation: WKNavigation!) {
        //Source: https://stackoverflow.com/questions/45448443/swift-3-check-if-wkwebview-has-loaded-page
        let appVersionStr: String = Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as! String
        let buildNumberStr: String = Bundle.main.object(forInfoDictionaryKey: "CFBundleVersion") as! String
        
        var systemInfo = utsname()
        uname(&systemInfo)
        let machineMirror = Mirror(reflecting: systemInfo.machine)
        var deviceDetailedStr =  machineMirror.children.reduce("") { identifier, element in
            guard let value = element.value as? Int8, value != 0 else { return identifier }
            return identifier + String(UnicodeScalar(UInt8(value)))
        }
        print(deviceDetailedStr)
        
        /*
         print(UIDevice.current.localizedModel)
         print(UIDevice.current.name)
         print(UIDevice.current.systemName)
         print(UIDevice.current.systemVersion)
         var systemInfo = utsname()
         uname(&systemInfo)
         let machineMirror = Mirror(reflecting: systemInfo.machine)
         var deviceDetailedStr =  machineMirror.children.reduce("") { identifier, element in
         guard let value = element.value as? Int8, value != 0 else { return identifier }
         return identifier + String(UnicodeScalar(UInt8(value)))
         }
         print(deviceDetailedStr)
         
         Below is what I found to be the output of the above commented out code on various devices.
         
         --Rahul's iPad, tested Oct 1, 2019--
         iPad //possible values are like iPad, iPod touch, iPhone, iPhone Simulator, etc, not specific devices
         iPad
         Megan’s iPad
         iOS
         12.1.4
         iPad4,7
         
         --Rahul's iPhone 6+, tested Oct 1, 2019--
         iPhone
         iPhone
         iPhone (2)
         iOS
         11.4.1
         iPhone7,1
         
         --MoTrack iPhone 8, tested Oct 1, 2019--
         iPhone
         iPhone
         iPhone
         iOS
         12.4
         iPhone10,4
         */
        
        
        //Don't have quotes around buildNumberStr so it converts to a number in scriptString
        //let scriptString = "setAppVersions('1', 1);"
        //let scriptString = "setAppVersions('\(appVersionStr)', \(buildNumberStr));"
        let deviceDetailsStr = """
        {
        UIDevice_model         : '\(UIDevice.current.model)',
        UIDevice_name          : '\(UIDevice.current.name)',
        UIDevice_systemName    : '\(UIDevice.current.systemName)',
        UIDevice_systemVersion : '\(UIDevice.current.systemVersion)',
        machineModel           : '\(deviceDetailedStr)'
        }
        """;
        let scriptString = "setAppVersions('\(appVersionStr)', \(buildNumberStr), \(deviceDetailsStr));"
        print(scriptString)
        myWebView.evaluateJavaScript(scriptString, completionHandler:nil)
    }
    
    
    //Useful link: https://stackoverflow.com/questions/36231061/wkwebview-open-links-from-certain-domain-in-safari
    func webView(_ webView: WKWebView, decidePolicyFor navigationAction: WKNavigationAction, decisionHandler: @escaping (WKNavigationActionPolicy) -> Void) {
        if navigationAction.navigationType == .linkActivated  {
            
            if let url = navigationAction.request.url {
                print(url)
                if let host = url.host { //telephone and email links don't get past this
                    if !host.hasPrefix("motracktherapy.com") && !host.hasPrefix("www.motracktherapy.com") { //don't allow links that aren't motracktherapy.com to be displayed within the webview
                        if UIApplication.shared.canOpenURL(url) {
                            UIApplication.shared.open(url)
                            print("Redirected to browser. No need to open it locally")
                            decisionHandler(.cancel)
                        } else {
                            print("url link type 1, can't open it")
                            decisionHandler(.cancel)
                        }
                    } else {
                        //print("url link type 2, allow since MoTrack link")
                        //decisionHandler(.allow)
                        //UIApplication.shared.open(url)
                        //decisionHandler(.cancel)
                        print("motrack link, force override open externally");
                        UIApplication.shared.open(url)
                        decisionHandler(.cancel)
                    }
                } else {
                    //can't check if MoTrack link. Telephones and email links get here. Don't open locally
                    print("url link type 3")
                    UIApplication.shared.open(url)
                    decisionHandler(.cancel)
                }
            } else {
                print("url link type 4")
                decisionHandler(.allow) //idk what to do here
            }
        } else {
            print("not a user click")
            decisionHandler(.allow) //idk what to do here
        }
    }
    
}



