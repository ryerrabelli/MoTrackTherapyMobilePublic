//
//  CVViewController.swift
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 12/26/18.
//  Recreated by Rahul Yerrabelli on 2/13/19.
//  Copyright © 2018/2019 MoTrack Therapy. All rights reserved.
//

// To learn how to create another view controller in storyboard, and make the associated cocoa touch class, I used the first couple of points in this article: https://www.huffingtonpost.com/dulio-denis/ios-quick-read-implementi_4_b_6923788.html
//Note: the above link is no longer live (as of May 15, 2019). You can use the Wayback Machine to get an archive or you can use the below link which is very similar (much of the text is copied, but some things are different; written by the same author and published only a couple days apart) updated URL: https://cocoaallocinit.com/2015/03/22/implementing-multiple-view-controllers-in-swift/. Title "iOS Quick Read: Implementing Multiple View Controllers in Swift" publsihed by Dulio Denis in late March 2014

//To learn how to open up this second view controller from the first: https://stackoverflow.com/questions/46209316/navigate-to-new-view-controller-after-animation-is-completed-in-swift-3-0


import UIKit
import AVFoundation
//import CoreMotion //for gyroscope/accelerometer/magnetometer
import Firebase
import Crashlytics
import Photos


let openCVWrapper = OpenCVWrapper()

class CVViewController: UIViewController, FrameExtractorDelegate, AVAudioPlayerDelegate {
    
    var frameExtractor: FrameExtractor!
    
    public var exerciseGame:Int = -1;
    public var nrtbcEach:Int = 5;
    public var whichHandIntended:Int = 5;
    public var jsonDataStr:String = "";
    //allow photo saving (allowPhotoSaving) can't be part of jsonDataStr because it is needed in Swift
    public var allowPhotoSaving:Bool = false;
    private var coreButtonsVisible:Bool = true;


    public final var isTesting:Bool = false; //if false, automatic background subtraction is enabled
    public var orientationGame = -1; //0 is portrait, 1 is landscape (with left side of phone on bottom)
    public var playingSoundEnabled:Int = 1;
    private var savingToPhotoIsAuthorized:Bool = false;
    private var saveNextFrameAsPhoto:Bool = false;
    

    //var audioPlayer = AVAudioPlayer()
    var audioPlayer:AVAudioPlayer!
    //var motionManager:CMMotionManager!
    
    @IBOutlet var tapGesture: UITapGestureRecognizer!
    
    @IBAction func tapImage(_ sender: UITapGestureRecognizer) {
        if sender.numberOfTouches > 1 {
            print( "multiple touches, numberOfTouches=" + String(sender.numberOfTouches)  )
        }
        for i in 0..<sender.numberOfTouches {
            let loc = sender.location(ofTouch: i, in: self.view)
            let returnWidth  = imageView.frame.width;
            let returnHeight = imageView.frame.height;
            OpenCVWrapper.registerTapOf(x: Double(loc.x), andY: Double(loc.y), andDisplayWidth: Int(returnWidth), andDisplayHeight: Int(returnHeight) );
        }
    }
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        Analytics.setScreenName("game_num\(exerciseGame)", screenClass: "nativecv")

        
        //In order for an image to be loadable, you have to click Assets.xcassets folder in the project manager in XCode, and then create a new image set
        let yourImage: UIImage = UIImage(named: "Image")!
        imageView.image = yourImage
        
        
        saveNextFrameAsPhoto = false;
        
        
        //INITIALIZE C++ WITH THE NUMBER OF REPS AND THE HAND
        OpenCVWrapper.initializeObjc(withReps: nrtbcEach, andHand: whichHandIntended, andJSONDataStr: jsonDataStr);
        
        
        //SET UP IMAGES (ON ANDROID, WOULD SET UP AUDIO, BUT NO NEED TO SET UP AUDIO ON iOS)
        OpenCVWrapper.setupImages(for: Int32(exerciseGame));
        
        
        //SET UP ORIENTATION
        let continuousCamera = OpenCVWrapper.shouldCameraBeInitializedAsContinuousObjC()
        frameExtractor = FrameExtractor(orientation: orientationGame, continuousCamera: continuousCamera)
        frameExtractor.delegate = self
        
        
        //SET UP DEVICE-SPECIFIC USER INTERFACE
        if (!isTesting) { //if isTesting is false i.e. automatic background subtraction is turned on
            flipLearningButton.isHidden = true;
            showBackgroundButton.isHidden = true;
        }
        
        //would do "exerciseGame == DEMO_PORTRAIT_GAME || exerciseGame == DEMO_LANDSCAPE_GAME", but those DEMO_ values aren't able to be accessed in swift
        if (exerciseGame < 0 || exerciseGame >= 10) {
            showBtnsButton.alpha = 0.0;
        }
        setButtonOpacities();
        
        //Photos
        if (allowPhotoSaving) {
            photoButton.isEnabled = true;
            photoButton.alpha = 1.0;
            let photos = PHPhotoLibrary.authorizationStatus()
            if photos == .notDetermined {
                PHPhotoLibrary.requestAuthorization({status in
                    if status == .authorized {
                        self.savingToPhotoIsAuthorized = true;
                        
                    } else {
                        self.savingToPhotoIsAuthorized = false;
                    }
                })
            }
        } else {
            photoButton.isEnabled = false;
            photoButton.alpha = 0.0;
        }

        
        //****
        //NOTE: The buttons are made round in storyboard using layer.cornerRadius
        //Link: https://stackoverflow.com/questions/38874517/how-to-make-a-simple-rounded-button-in-storyboard
        //****
        
        
        //motionManager =  CMMotionManager()
        /*if motionManager.isMagnetometerAvailable {
            motionManager.magnetometerUpdateInterval = 0.1
            motionManager.startMagnetometerUpdates(to: OperationQueue.main) { (data, error) in
                //y is about 0 when vertical up, about 90 when vertical down, and around 30-60 when flat (it varies significantly though)
                print(data)
            }
        } else {
            print("Magnetometer not available");
        }
        */
        /*if motionManager.isMagnetometerAvailable {
         motionManager.magnetometerUpdateInterval = 0.1
         motionManager.startMagnetometerUpdates()
         } else {
         print("Magnetometer not available");
         }*/
        /*if motionManager.isDeviceMotionAvailable {
            self.motionManager.deviceMotionUpdateInterval = 1.0 / 60.0
            self.motionManager.showsDeviceMovementDisplay = true
            self.motionManager.startDeviceMotionUpdates(using: .xMagneticNorthZVertical)
        }*/
    }
    
    func audioPlayerDidFinishPlaying(_ player: AVAudioPlayer, successfully flag: Bool) {
        print("The song ended")
    }
    
    //This doesn't actually appear to get called for some reason
    override func didReceiveMemoryWarning() {
        print("MEMORY WARNING 2")
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        //This will prevent the phone from dimming out at first and eventually going to sleep because of perceived inactivity (i.e. no touches to the screen). This has to be undone when the app leaves the CV part is over so that the phone's normal settings are put back during the html part of the app (i.e. go to sleep if the user doesn't interact/touch the app)
        UIApplication.shared.isIdleTimerDisabled = true
        
        
        //Below code will allow app rotation to landscapeRight and then force rotation to landscapeRight. Undone in viewWillDisappear
        //help source: https://stackoverflow.com/questions/20987249/how-do-i-programmatically-set-device-orientation-in-ios-7
        //help source: https://stackoverflow.com/questions/24928057/only-one-view-landscape-mode
        if (orientationGame == 0) { //portrait
            let appDelegate = UIApplication.shared.delegate as! AppDelegate
            appDelegate.shouldRotateLandscape = false
            let value = UIInterfaceOrientation.portrait.rawValue
            UIDevice.current.setValue(value, forKey: "orientation")
        } else if (orientationGame == 1) { //landscape (with left side of phone on bottom)
            let appDelegate = UIApplication.shared.delegate as! AppDelegate
            appDelegate.shouldRotateLandscape = true
            let value = UIInterfaceOrientation.landscapeRight.rawValue
            UIDevice.current.setValue(value, forKey: "orientation")
        } else {
            print("Orientation_game is not one of the allowed options. Couldn't rotate screen.")
        }
        
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        
        frameExtractor.close()
        
        //Undoes what happens in viewWillAppear to bring back the normal phone settings of going to sleep after a certain point in time.
        UIApplication.shared.isIdleTimerDisabled = false
        
        /*if (self.isMovingFromParent) {
         UIDevice.current.setValue(Int(UIInterfaceOrientation.portrait.rawValue), forKey: "orientation")
         }*/
        
        //Puts rotation back to normal, undoing what is done in viewWillAppear
        let appDelegate = UIApplication.shared.delegate as! AppDelegate
        appDelegate.shouldRotateLandscape = false
        let value = UIInterfaceOrientation.portrait.rawValue
        UIDevice.current.setValue(value, forKey: "orientation")
    }
    
    @IBOutlet weak var learningButton: UIBarButtonItem! //used to be learningButton, but now is resetButton
    @IBOutlet weak var flipLearningButton: UIButton!
    @IBOutlet weak var showBackgroundButton: UIButton!
    @IBOutlet weak var quitButton: UIBarButtonItem!
    
    @IBOutlet weak var showBtnsButton: UIButton!
    @IBOutlet weak var photoButton: UIButton!
    @IBOutlet weak var  backButton: UIButton!
    @IBOutlet weak var resetButton: UIButton!
    @IBOutlet weak var  skipButton: UIButton!

    @IBAction func doPhoto(_ sender: UIButton) {
        if (allowPhotoSaving) {
            saveNextFrameAsPhoto = true;
            OpenCVWrapper.playAudioFileClickObjC();
        }
    }
    @IBAction func doFlipButtonOpacities(_ sender: UIButton) {
        coreButtonsVisible = !coreButtonsVisible;
        
        setButtonOpacities();
    }
    func setButtonOpacities() {
        if (coreButtonsVisible) {
            skipButton.alpha = 0.7;
            resetButton.alpha = 0.7;
            backButton.alpha = 0.7;
            
            showBackgroundButton.alpha = 0;
            flipLearningButton.alpha = 0;
            
        } else {
            skipButton.alpha = 0.03;
            resetButton.alpha = 0.03;
            backButton.alpha = 0.03;
            
            showBackgroundButton.alpha = 0;
            flipLearningButton.alpha = 0;
            
        }
    }
    
    @IBAction func doReset(_ sender: UIButton) {
        frameExtractor.allowExposureAdjustment();
        OpenCVWrapper.resetCalibrationBackgroundObjC();
    }
    @IBAction func doSkip(_ sender: UIButton) {
        OpenCVWrapper.doSkipObjC();
    }
    func quitWithError(error: String) {
        weak var pvc = self.presentingViewController
        //multiply nrtbcEach by 2 to get total rep count, not per each of the two exercises in a given game
        self.dismiss(animated: true, completion: {
            (pvc as! WebViewController).updateAfterGameFinishedWithError(exerciseGame: self.exerciseGame,
                                                                         gameScore: OpenCVWrapper.getGameScoreObjC(),
                                                                         repsCompletedTot: Int(OpenCVWrapper.getNumRepsCompletedTotObjC()),
                                                                         nrtbcEach: self.nrtbcEach,
                                                                         errorStr: error);
        })
    }
    @IBAction func doQuit(_ sender: UIButton) {
        if sender != nil { //this will enver actually be called, but left for alignment with the other quit method
            OpenCVWrapper.playAudioFileClickObjC();
        }
        //Source: https://stackoverflow.com/questions/43566414/present-a-controller-dismiss-it-and-present-a-different-one-in-swift
        weak var pvc = self.presentingViewController
        //multiply nrtbcEach by 2 to get total rep count, not per each of the two exercises in a given game
        self.dismiss(animated: true, completion: {
            (pvc as! WebViewController).updateAfterGameFinished(exerciseGame: self.exerciseGame,
                                                             gameScore: OpenCVWrapper.getGameScoreObjC(),
                                                             repsCompletedTot: Int(OpenCVWrapper.getNumRepsCompletedTotObjC()),
                                                             nrtbcEach: self.nrtbcEach);
        })
    }
    
    @IBAction func quit(_ sender: UIBarButtonItem?) {
        //frameExtractor.delegate = nil;
        //frameExtractor.close()
        
        //self.dismiss(animated: true, completion: nil)
        if sender != nil {
            OpenCVWrapper.playAudioFileClickObjC();
        }
        //Source: https://stackoverflow.com/questions/43566414/present-a-controller-dismiss-it-and-present-a-different-one-in-swift
        weak var pvc = self.presentingViewController
        //multiply nrtbcEach by 2 to get total rep count, not per each of the two exercises in a given game
        self.dismiss(animated: true, completion: {
            (pvc as! WebViewController).updateAfterGameFinished(exerciseGame: self.exerciseGame,
                                                             gameScore: OpenCVWrapper.getGameScoreObjC(),
                                                             repsCompletedTot: Int(OpenCVWrapper.getNumRepsCompletedTotObjC()),
                                                             nrtbcEach: self.nrtbcEach);
        })
        
    }
    
    //used to be learning button functionality, but now reset button
    @IBAction func changeLearning(_ sender: UIBarButtonItem) {
        /*if (OpenCVWrapper.flipLearning()) {
         learningButton.title = "LRN";
         } else {
         learningButton.title = "SHOW";
         }*/
        //openCVWrapper.isThisWorking()
        
        frameExtractor.allowExposureAdjustment();
        OpenCVWrapper.resetCalibrationBackgroundObjC();
    }
    @IBAction func skip(_ sender: UIBarButtonItem) {
        //print("playing?: " + String(audioPlayer.isPlaying))
        
        OpenCVWrapper.doSkipObjC();
    }
    @IBAction func flipLearning(_ sender: UIButton) {
        /* //Disable the flip learning button
         if (OpenCVWrapper.flipLearning()) {
         flipLearningButton.setTitle("LRN", for: .normal);
         } else {
         flipLearningButton.setTitle("SHOW", for: .normal);
         }*/
    }
    
    @IBAction func changeShowBackground(_ sender: UIButton) {
        /* //Disable the show background button
         let currentShowBackground:Int32 = OpenCVWrapper.incrementShowBackground();
         showBackgroundButton.setTitle("Background: " + String(currentShowBackground), for: .normal);
         */
    }
    
    /*
     // MARK: - Navigation
     
     // In a storyboard-based application, you will often want to do a little preparation before navigation
     override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
     // Get the new view controller using segue.destination.
     // Pass the selected object to the new view controller.
     }
     */
    
    
    @IBOutlet weak var imageView: UIImageView!
    
    //Tutorials that helped:
    //https://www.hackingwithswift.com/read/13/5/saving-to-the-ios-photo-library
    //https://www.hackingwithswift.com/example-code/media/uiimagewritetosavedphotosalbum-how-to-write-to-the-ios-photo-album
    //https://developer.apple.com/documentation/uikit/1619125-uiimagewritetosavedphotosalbum
    @objc func image(_ image: UIImage, didFinishSavingWithError error: NSError?, contextInfo: UnsafeRawPointer) {
        if let error = error {
            // we got back an error!
            /*
            let ac = UIAlertController(title: "Save error", message: error.localizedDescription, preferredStyle: .alert)
            ac.addAction(UIAlertAction(title: "OK", style: .default))
            present(ac, animated: true) */
            OpenCVWrapper.playAudioFileDoorKnockbjC();
            
        } else {
            /*
            let ac = UIAlertController(title: "Saved!", message: "Your altered image has been saved to your photos.", preferredStyle: .alert)
            ac.addAction(UIAlertAction(title: "OK", style: .default))
            present(ac, animated: true)*/
            OpenCVWrapper.playAudioFileCameraObjC();
        }
    }
    
    func captured(inputImage: UIImage) {
        //Check if there is a sound needed to be played, and play it if necessary.
        //every time getSoundToPlayIfAnyObjC is called, the sound needed to play is set to null in c++ so it will won't return a sound next time
        let soundNameNS = OpenCVWrapper.getSoundToPlayIfAnyObjC() //soundNameNS is originally an NSString*
        if (playingSoundEnabled > 0) { //playingSoundEnabled isn't really changeable/functional yet
            playSoundIfAny(soundName: soundNameNS as String) //as String converts from NSString to swift string
        }
        
        let doneState = OpenCVWrapper.getDoneStateObjC()
        if (doneState > 0) {
            //need to replace with doQuit(.)
            quit(nil);
            
        } else if (doneState < 0) {
            //errorDescript is in regular text, errorDescriptCoded is modified to remove certain information and make it safe for users/competitors to see
            let errorDescript      = OpenCVWrapper.getCrashErrorDescriptionObjC();
            let errorDescriptCoded = OpenCVWrapper.getCrashErrorDescriptionCodedObjC();
            /*let userInfo: [String : Any] = [
                    NSLocalizedDescriptionKey :  NSLocalizedString("Unauthorized", value: "Test 1", comment: "test comment 1") ,
                    NSLocalizedFailureReasonErrorKey : NSLocalizedString("Unauthorized", value: "Test 2", comment: "test comment 2")
            ]*/
            let userInfo: [String : Any] = [
                "whichHandIntended"   :  whichHandIntended,
                "exerciseGame"        :  exerciseGame,
                "orientationGame"     :  orientationGame,
                "nrtbcEach"           :  self.nrtbcEach,
                "repsCompletedTot"    :  Int(OpenCVWrapper.getNumRepsCompletedTotObjC()),
                "gameScore"           :  OpenCVWrapper.getGameScoreObjC(),
                "codedDescription"    :  errorDescriptCoded
            ]
            let cppError = NSError(domain: errorDescript, code: -1001, userInfo: userInfo);
            Crashlytics.sharedInstance().recordError(cppError)
            
            quitWithError(error: errorDescriptCoded);
            
        } else {
            let returnWidth  = imageView.frame.width;
            let returnHeight = imageView.frame.height;
            let doInteractiveTutorial = false;
            let processedImage = OpenCVWrapper.processImage(withOpenCV: inputImage, andIsTesting: isTesting, andReturnWidth: Int(returnWidth), andReturnHeight: Int(returnHeight), andDoInteractiveTutorial: doInteractiveTutorial)
            imageView.image = processedImage
            
            if (saveNextFrameAsPhoto && allowPhotoSaving) {
                //UIImageWriteToSavedPhotosAlbum(inputImage, nil, nil, nil);
                print("Will save frame as photo");
                UIImageWriteToSavedPhotosAlbum(inputImage,     self, #selector(image(_:didFinishSavingWithError:contextInfo:)), nil)
                UIImageWriteToSavedPhotosAlbum(processedImage, self, #selector(image(_:didFinishSavingWithError:contextInfo:)), nil)
                saveNextFrameAsPhoto = false;
            }

            
            /*let x = motionManager.magnetometerData?.magneticField.x;
            let y = motionManager.magnetometerData?.magneticField.y;
            let z = motionManager.magnetometerData?.magneticField.z
            //print("Mag field" +  + "," + motionManager.magnetometerData?.magneticField.y + "," + motionManager.magnetometerData?.magneticField.z)
            print(x)*/
            
            //print (motionManager.magnetometerData);
            
            /*if let data = self.motionManager.deviceMotion {
                // Get the attitude relative to the magnetic north reference frame.
                let x = data.attitude.pitch
                let y = data.attitude.roll
                let z = data.attitude.yaw
                print("\(x), \(y), \(z)")
                // Use the motion data in your app.
            }*/
        }
        
    }
    
    func playSoundIfAny(soundName: String) {
        if (soundName != "" && soundName != "_") {
            //Source: https://stackoverflow.com/questions/47619607/how-to-play-a-sound-on-ios-11-with-swift-4-and-where-i-place-the-mp3-file
            if let audioPlayer = audioPlayer, audioPlayer.isPlaying { audioPlayer.stop() }
            //can either input like forResource: "mymusic.mp3" and withExtension: "" or forResource: "mymusic" and withExtension: "mp3". Did the former so the c++ code can also determine file extension since the skip sound is a different file extension (.wav) than the rest.
            guard let soundURL = Bundle.main.url(forResource: soundName, withExtension: "", subdirectory: "assets/Dropbox/ui_media/sounds/") else { return }
            do {
                try AVAudioSession.sharedInstance().setCategory(AVAudioSession.Category.playback, mode: AVAudioSession.Mode.default)
                try AVAudioSession.sharedInstance().setActive(true)
                audioPlayer = try AVAudioPlayer(contentsOf: soundURL)
                audioPlayer?.play()
            } catch let error {
                //Print.detailed(error.localizedDescription)
            }
        }
    }
    
}
