//
//  FrameExtractor.swift
//  MoTrack Therapy
//
//  Created by Rahul Yerrabelli on 12/26/18.
//  Recreated by Rahul Yerrabelli on 2/13/19.
//  Copyright © 2018/2019 MoTrack Therapy. All rights reserved.
//

//In order to figure out how to get the images from the camera per frame, I used tutorial here: https://medium.com/ios-os-x-development/ios-camera-frames-extraction-d2c0f80ed05a. However, that tutorial was made for swift 3. To make it compatible with swift 4, I had to:  [1] change the captureOutput optional delegate method because the syntax changed and so the swift 3 version is ignored (and no warning is given because it is an optional method). The comment in the tutorial by Timur Nasyrov (responding to Reza Shirazian's comment) fixed this. [2] This wasn't necessary since it still worked but AVCaptureDevice.devices() was deprecated and needed to be replaced by AVCaptureDevice.default(.). I found out how to do this from born_stubborn's answer in https://stackoverflow.com/questions/39894630/how-to-get-front-camera-back-camera-and-audio-with-avcapturedevicediscoverysess. It looks like the better resource might have actually been using AVCaptureDevice.DiscoverySession as described in https://developer.apple.com/documentation/avfoundation/cameras_and_media_capture/choosing_a_capture_device

import UIKit
import AVFoundation

protocol FrameExtractorDelegate: class {
    func captured(inputImage: UIImage)
}

class FrameExtractor: NSObject, AVCaptureVideoDataOutputSampleBufferDelegate {
    
    private let position = AVCaptureDevice.Position.front
    
    //https://developer.apple.com/documentation/avfoundation/avcapturesession/preset
    //on Rahul's iPhone 6, it appears .low means 192 by 144 (4:3), .medium means 480 by 360 (4:3), .high means 1280 by  720 (16:9), .photo means 1280 by  960 (4:3)
    //on Anita's iPhone X, it appears .low means 192 by 144 (4:3), .medium means 480 by 360 (4:3), .high means 1920 by 1080 (16:9), .photo means 1504 by 1128 (4:3)
    private let quality = AVCaptureSession.Preset.medium
    
    private var permissionGranted = false
    private let sessionQueue = DispatchQueue(label: "session queue")
    private let captureSession = AVCaptureSession()
    private let context = CIContext()
    
    private var orientation:Int = -5
    private var continuousCamera:Bool = false
    
    weak var delegate: FrameExtractorDelegate?
    
    // override
    init(orientation:Int, continuousCamera:Bool) {
        super.init()
        self.orientation = orientation
        self.continuousCamera = continuousCamera
        checkPermission()
        sessionQueue.async { [unowned self] in
            self.configureSession()
            self.captureSession.startRunning()
        }
    }
    
    // MARK: AVSession configuration
    private func checkPermission() {
        switch AVCaptureDevice.authorizationStatus(for: AVMediaType.video) {
        case .authorized:
            permissionGranted = true
        case .notDetermined:
            requestPermission()
        default:
            permissionGranted = false
        }
    }
    
    private func requestPermission() {
        sessionQueue.suspend()
        AVCaptureDevice.requestAccess(for: AVMediaType.video) { [unowned self] granted in
            self.permissionGranted = granted
            self.sessionQueue.resume()
        }
    }
    public func close() {
        //https://developer.apple.com/documentation/avfoundation/avcapturesession
        self.captureSession.stopRunning()
    }
    
    private func configureSession() {
        guard permissionGranted else { return }
        captureSession.sessionPreset = quality
        guard let captureDevice = selectCaptureDevice() else { return }
        guard let captureDeviceInput = try? AVCaptureDeviceInput(device: captureDevice) else { return }
        guard captureSession.canAddInput(captureDeviceInput) else { return }
        captureSession.addInput(captureDeviceInput)
        
        //change the resolution
        //guard captureSession.canSetSessionPreset(.photo) else { return }
        //self.captureSession.sessionPreset = .photo
        
        let videoOutput = AVCaptureVideoDataOutput()
        videoOutput.setSampleBufferDelegate(self, queue: DispatchQueue(label: "Buffer"))
        guard captureSession.canAddOutput(videoOutput) else { return }
        captureSession.addOutput(videoOutput)
        guard let connection = videoOutput.connection(with: AVFoundation.AVMediaType.video) else { return }
        guard connection.isVideoOrientationSupported else { return }
        guard connection.isVideoMirroringSupported else { return }
        if (orientation == 0) {
            connection.videoOrientation = .portrait
        } else if (orientation == 1) {
            //landscape right means rotate the phone counterclockwise from portrait position (assuming screen is face up)
            connection.videoOrientation = AVCaptureVideoOrientation.landscapeRight
        } else {
            //default
            connection.videoOrientation = .portrait
        }
        connection.isVideoMirrored = position == .front
        
        
        allowExposureAdjustment();
    }
    
    //Called upon hitting reset, as well as during the initial setup
    public func allowExposureAdjustment() {
        guard let captureDevice = selectCaptureDevice() else { return }
        do {
            try captureDevice.lockForConfiguration()
            
            if continuousCamera {
                captureDevice.exposureMode = AVCaptureDevice.ExposureMode.continuousAutoExposure;
                if captureDevice.isWhiteBalanceModeSupported(AVCaptureDevice.WhiteBalanceMode.continuousAutoWhiteBalance) {
                    captureDevice.whiteBalanceMode = AVCaptureDevice.WhiteBalanceMode.continuousAutoWhiteBalance;
                }
                
            } else {
                /*if captureDevice.isFocusPointOfInterestSupported {
                 captureDevice.focusPointOfInterest = focusPoint
                 captureDevice.focusMode = AVCaptureDevice.FocusMode.autoFocus
                 }
                 if captureDevice.isExposurePointOfInterestSupported {
                 captureDevice.exposurePointOfInterest = focusPoint
                 captureDevice.exposureMode = AVCaptureDevice.ExposureMode.autoExpose
                 }*/
                
                captureDevice.exposureMode = AVCaptureDevice.ExposureMode.autoExpose;
                if captureDevice.isWhiteBalanceModeSupported(AVCaptureDevice.WhiteBalanceMode.autoWhiteBalance) {
                    captureDevice.whiteBalanceMode = AVCaptureDevice.WhiteBalanceMode.autoWhiteBalance;
                }
            }
            
            captureDevice.unlockForConfiguration()
            
        } catch {
            // Handle errors here
            print("There was an error focusing the device's camera")
        }
    }
    
    private func selectCaptureDevice() -> AVCaptureDevice? {
        let b = AVCaptureDevice.default(.builtInWideAngleCamera, for: AVMediaType.video, position: .front);
        return b
    }
    
    // MARK: Sample buffer to UIImage conversion
    private func imageFromSampleBuffer(sampleBuffer: CMSampleBuffer) -> UIImage? {
        guard let imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer) else { return nil }
        let ciImage = CIImage(cvPixelBuffer: imageBuffer)
        guard let cgImage = context.createCGImage(ciImage, from: ciImage.extent) else { return nil }
        return UIImage(cgImage: cgImage)
    }
    
    
    private var counter1:Int = 0;
    private var counter2:Int = 0;
    
    // Below function is called every frame (it is an optional delegate function of AVCaptureVideoDataOutputSampleBufferDelegate)
    func captureOutput(_ output: AVCaptureOutput, didOutput sampleBuffer: CMSampleBuffer, from connection: AVCaptureConnection) {
        
        counter1 = counter1 + 1
        //print("new frame " + String(self.counter1))
        
        //The code inside the DispatchQueue.main.async sometimes skips a frame if the algorithm/cpp code is too slow
        DispatchQueue.main.async { [unowned self] in
            //Below line was originally outside the DispatchQueue.main.async
            //However, moving it here made the app no longer crash if the algorithm/cpp code was too slow (now the frame speed just decreases), Sept 3rd, 2019
            guard let uiImage = self.imageFromSampleBuffer(sampleBuffer: sampleBuffer) else { return }

            self.counter2 = self.counter2 + 1
            //print("frame " + String(self.counter1) + ", " + String(self.counter2))
            
            self.delegate?.captured(inputImage: uiImage)
        }
    }
    
}
