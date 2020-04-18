import json
import cv2
import numpy as np
Pred_Dir="../Output_Prediction/" # Library where the output prediction will be written

################################################################################################################################################################################
def getCoreStrFromFileName(file_name):
    #assumes that the format is IMG_0003_H_RAW.jpg
    if file_name.upper().startswith("IMG_") and file_name.upper().endswith("_RAW.JPG"):
        #return file_name[4:-8]
        return file_name[len("IMG_"):-len("_RAW.jpg")]
    else:
        return file_name

def main(argv=None):
    with open(Pred_Dir+"/OverLay/data_unanalyzed_ML.json", 'r') as f:
        mystr = f.read()
        #must do json.loads(mystr.strip()) instead of json.load(f) which goes directly from the file because it apparently
        #throws an error if it isn't strip (I don't know why)
        data = json.loads(mystr.strip())
        sorted_images = sorted(data["images"].items(), key=lambda kvpair: kvpair[1]["analysis"]["loss"])
        print([getCoreStrFromFileName(i[0]) for i in sorted_images])

        windowName = "MoTrack Image Analysis Viewer"
        scale = 0.5
        cv2.namedWindow(windowName, cv2.WINDOW_NORMAL)
        cv2.resizeWindow(windowName, (int(1125*scale), int(768*scale*1.5)) )
        keepRunning = True
        letterInd = 0
        goingBackwards = False
        feedbackText = ""

        while keepRunning:
            imageFileName = ""
            if letterInd < 0:
                letterInd = len(sorted_images)
                continue
            elif letterInd > len(sorted_images):
                letterInd = 0
                continue
            elif letterInd == len(sorted_images):
                img = np.zeros((480, 360, 4), np.uint8)
                cv2.putText(img, "You've finished going", (0, int(img.shape[0]*0.45) ), cv2.FONT_HERSHEY_COMPLEX, 0.8, (255, 255, 255, 255), 2)
                cv2.putText(img, "thru all the images",   (0, int(img.shape[0]*0.55) ), cv2.FONT_HERSHEY_COMPLEX, 0.8, (255, 255, 255, 255), 2)


            else:
                imageFileName = sorted_images[letterInd][0]
                img = cv2.imread(Pred_Dir+"/OverLay/"+sorted_images[letterInd][0], cv2.IMREAD_UNCHANGED)


            if img is None:
                img = np.zeros((360, 480, 4), np.uint8)
            elif len(img.shape)<3 or img.shape[2]<4:
                img = cv2.cvtColor(img, cv2.COLOR_BGR2BGRA)

            imgTextArea = np.ones((100, img.shape[1], 4), np.uint8)*230 #light grey
            #color in bgra
            cv2.putText(imgTextArea, str(letterInd+1)+"/"+str(len(sorted_images)), (0, int(imgTextArea.shape[0]*0.4) ), cv2.FONT_HERSHEY_COMPLEX, 1.0, (127, 127, 0, 255), 4)
            cv2.putText(imgTextArea, getCoreStrFromFileName(imageFileName), (0, int(imgTextArea.shape[0]*0.9) ), cv2.FONT_HERSHEY_COMPLEX, 1.0, (127, 127, 0, 255), 4)

            updateAllImage = False
            while not updateAllImage:
                imgError = np.ones((100, img.shape[1], 4), np.uint8)*230 #light grey
                cv2.putText(imgError, feedbackText, (0, int(imgTextArea.shape[0]*0.7) ), cv2.FONT_HERSHEY_COMPLEX, 1.5, (0, 0, 255, 255), 3)
                cv2.imshow(windowName, np.vstack((imgTextArea, img, imgError)) )
                feedbackText = ""

                updateImage = False
                while not updateImage and not updateAllImage:
                    k = cv2.waitKey(0) & 0xff
                    if k == 27 or k == ord('q'): # wait for ESC key or q key to exit
                        keepRunning = False
                        updateAllImage = True
                    elif k == ord('j'): # represents left in IJKL keyboard
                        letterInd -= 1
                        goingBackwards = True
                        updateAllImage = True
                    elif k == ord('l'): # represents right in IJKL keyboard
                        letterInd += 1
                        goingBackwards = False
                        updateAllImage = True
                    else:
                        feedbackText = "Invalid key: " + chr(k)
                        updateImage = True

main()#Run script
print("Finished")