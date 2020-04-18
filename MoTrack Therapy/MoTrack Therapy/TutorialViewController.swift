//
//  TutorialViewController.swift
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 5/15/19.
//  Copyright © 2019 MoTrack Therapy. All rights reserved.
//

import UIKit
import AVFoundation
//import CoreMotion //for gyroscope/accelerometer/magnetometer
import Firebase

class TutorialViewController: UIViewController, FrameExtractorDelegate, AVAudioPlayerDelegate {
    @IBOutlet weak var imageView: UIImageView!
    
    public var playingSoundEnabled:Int = 1;
    public var orientationGame = -1; //0 is portrait, 1 is landscape (with left side of phone on bottom)
    
    var audioPlayer:AVAudioPlayer!

    func captured(inputImage: UIImage) {
        //Check if there is a sound needed to be played, and play it if necessary.
        //every time getSoundToPlayIfAnyObjC is called, the sound needed to play is set to null in c++ so it will won't return a sound next time
        let soundNameNS = OpenCVWrapper.getSoundToPlayIfAnyObjC(); //soundNameNS is originally an NSString*
        if (playingSoundEnabled > 0) { //playingSoundEnabled isn't really changeable/functional yet
            playSoundIfAny(soundName: soundNameNS as String) //as String converts from NSString to swift string
        }
        
        
        if (OpenCVWrapper.getLightsTutorialDoneStateObjC()) {
            nextButton.isEnabled = true
        }
        
        let returnWidth  = imageView.frame.width;
        let returnHeight = imageView.frame.height;
        let processedImage = OpenCVWrapper.doLightsTutorialObjC(inputImage, andReturnWidth: Int(returnWidth), andReturnHeight: Int(returnHeight))
        imageView.image = processedImage
        //imageView.image = image
        
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
    
    var frameExtractor: FrameExtractor!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        Analytics.setScreenName("tutorial", screenClass: "nativecv")
        
        nextButton.isEnabled = false
        
        //In order for an image to be loadable, you have to click Assets.xcassets folder in the project manager in XCode, and then create a new image set
        let yourImage: UIImage = UIImage(named: "Image")!
        imageView.image = yourImage
        
        OpenCVWrapper.initializeLightsTutorialObjC(jsonDataStr)

        
        frameExtractor = FrameExtractor(orientation: orientationGame, continuousCamera: false)
        frameExtractor.delegate = self
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
    
    public var jsonDataStr:String = "";
    
    
    @IBOutlet weak var backButton: UIButton!
    @IBOutlet weak var skipButton: UIButton!
    @IBOutlet weak var nextButton: UIButton!

    
    @IBAction func doBack(_ sender: UIButton) {
        weak var pvc = self.presentingViewController
        self.dismiss(animated: true, completion: {
            (pvc as! WebViewController).navigateTo(toPage: "#interactive-tutorial-4");
        })
    }
    @IBAction func doSkip(_ sender: UIButton) {
        
    }
    @IBAction func doNext(_ sender: UIButton) {
        weak var pvc = self.presentingViewController
        self.dismiss(animated: true, completion: {
            (pvc as! WebViewController).navigateTo(toPage: "#interactive-tutorial-6");
        })
    }
    

}
