import numpy as np
import cv2
import json


def makeStrFromInt(num: int , length: int, leadChar: str):
    if not isinstance(leadChar, str): leadChar = str(leadChar)
    if leadChar == "0": #simple way that doesn't line up with the c++ way
        return ("{:0"+str(length)+"d}").format(num)
    '''c++ way
    std::string makeStrFromInt(int num, int length, char leadChar) {
        if (num == 0) return std::string(length-1,leadChar) + "0";
        else return std::string( std::max(0,length-1-int(log10(num)+0.00001)) , leadChar) + std::to_string(num);
    }'''
    if num == 0:
        return leadChar*length #repeats the character leadChar by leadChar times
    else:
        import math
        return leadChar*max(0, length-1-int(math.log10(num)+0.0001 ) ) + str(num)


genFilePath = "../MoTrack Therapy Mac/MoTrack Therapy Mac/photos/photo_analysis/"
setNameLen = 4


def view():
    setNums = np.arange(1, 42)
    setNumInd = 0

    segmentationAlgs = [1684, 1684, 1684]
    segmentationSuffixes = ["", "_betterGraphics0_05", "_betterGraphics0_10"]
    segmentationInd = 0
    keepRunning = True

    letters = "ABCDEFGH"
    letterInd = 0

    global threshIoU, threshAccr, threshParamDifAbs, needToBeAboveThresh, updateImage
    threshIoU = 0
    threshAccr = 0
    threshParamDifAbs = 2
    needToBeAboveThresh = 0
    def updateThreshIoU(pos):
        global threshIoU, updateImage
        threshIoU = pos/100.0
        updateImage = True
    def updateThreshAccr(pos):
        global threshAccr, updateImage
        threshAccr = pos/100.0
        updateImage = True
    def updateThreshParamDif(pos):
        global threshParamDifAbs, updateImage
        threshParamDifAbs = pos/100.0
        updateImage = True
    def updateNeedToBeAboveThresh(pos):
        global needToBeAboveThresh, updateImage
        needToBeAboveThresh = pos
        updateImage = True

    windowName = "MoTrack Image Analysis Viewer"
    scale = 0.5
    cv2.namedWindow(windowName, cv2.WINDOW_NORMAL)
    cv2.createTrackbar('IoU',  windowName, threshIoU, 100, updateThreshIoU)
    cv2.createTrackbar('Accr', windowName, threshAccr, 100, updateThreshAccr)
    cv2.createTrackbar('ParamDif', windowName, threshParamDifAbs, 200, updateThreshParamDif)
    cv2.createTrackbar('Above', windowName, needToBeAboveThresh, 1, updateNeedToBeAboveThresh)

    cv2.resizeWindow(windowName, (int(1125*scale), int(768*scale*1.5)) )

    goingBackwards = False
    feedbackText = ""

    while keepRunning:
        command = ""
        shift = False

        if letterInd >= len(letters):
            setNumInd += 1
            letterInd = 0
        elif letterInd < 0:
            setNumInd -= 1
            letterInd = len(letters)-1


        setNumInd %= len(setNums) #setNumInd = setNumInd % len(setNums)
        segmentationInd %= len(segmentationAlgs) #segmentationInd = segmentationInd % len(segmentationAlgs)

        segmentationAlg = segmentationAlgs[segmentationInd]
        segmentationSuffix = segmentationSuffixes[segmentationInd]
        segmentationDesc = str(segmentationAlg)+segmentationSuffix

        setNum = setNums[setNumInd]
        setNumStr = makeStrFromInt(setNum,setNameLen,"0")
        letter = letters[letterInd]

        with open(genFilePath+segmentationDesc+"/data_unanalyzed_"+segmentationDesc+".json") as f:
            data = json.load(f)
            try:
                thisData = data["sets"][setNumStr]["IMG_"+setNumStr+"_"+letter]["analysis"]
                sign = -1 if needToBeAboveThresh == 1 else +1
                if thisData["IoU"]*sign < threshIoU*sign:
                    if goingBackwards:
                        letterInd -= 1
                    else:
                        letterInd += 1
                    continue
                elif thisData["accuracy"]*sign < threshAccr*sign:
                    if goingBackwards:
                        letterInd -= 1
                    else:
                        letterInd += 1
                    continue
                elif thisData["paramDiffAbs"] is not None and thisData["paramDiffAbs"]*sign*-1 < threshParamDifAbs*sign*-1:
                    # paramDiffAbs goes in the other direction because smaller paramDiffsAbs is better
                    # paramDiffAbs can be none (null, nan, depending on the language, json string vs cpp, etc) if comparing NOT_VALID and NO_HAND, for example
                    if goingBackwards:
                        letterInd -= 1
                    else:
                        letterInd += 1
                    continue
            except KeyError as e:
                print("KeyError:" + str(e))
                feedbackText = "Unable to find full info, wrong key"
            except TypeError as e:
                print("KeyError:" + str(e))
                feedbackText = "Unable to find full info, wrong type"

        imageFileName = "IMG_"+setNumStr+"_"+letter+"_COMP"+segmentationDesc
        imagePath = segmentationDesc +"/"+imageFileName+".png"
        print("imagePath = '"+imagePath+"'")
        img = cv2.imread(genFilePath+imagePath, cv2.IMREAD_UNCHANGED)

        if img is None:
            letterInd += 1
            img = np.zeros((768, 1125, 4), np.uint8)

        imgTextArea = np.ones((100, img.shape[1], 4), np.uint8)*230 #light grey
        #color in bgra
        cv2.putText(imgTextArea, imageFileName, (0, int(imgTextArea.shape[0]*0.7) ), cv2.FONT_HERSHEY_COMPLEX, 1.5, (127, 127, 0, 255), 4)

        updateAllImage = False
        while not updateAllImage:
            imgError = np.ones((100, img.shape[1], 4), np.uint8)*230 #light grey
            cv2.putText(imgError, feedbackText, (0, int(imgTextArea.shape[0]*0.7) ), cv2.FONT_HERSHEY_COMPLEX, 1.5, (0, 0, 255, 255), 4)
            cv2.imshow(windowName, np.vstack((imgTextArea, img, imgError)) )
            feedbackText = ""

            updateImage = False
            while not updateImage and not updateAllImage:
                thisShifted = shift
                shift = False

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
                elif k == ord('i'): # represents up in IJKL keyboard
                    segmentationInd -= 1
                    updateAllImage = True
                elif k == ord('k'): # represents down in IJKL keyboard
                    segmentationInd += 1
                    updateAllImage = True
                elif k == 9: # tab
                    setNumInd += 1
                    letterInd = 0
                    goingBackwards = False
                    updateAllImage = True
                elif k == ord('`'): # takes the place of shift tab (inverse tab)
                    setNumInd -= 1
                    letterInd = 0
                    goingBackwards = True
                    updateAllImage = True
                elif chr(k) in letters.lower():
                    command = command + chr(k).upper() #chr is the inverse of ord
                    feedbackText = "command: " + command
                    updateImage = True
                elif k in [ord('0'),ord('1'),ord('2'),ord('3'),ord('4'),ord('5'),ord('6'),ord('7'),ord('8'),ord('9')]: # number
                    #k is 48 through 57 inclusive
                    command = command + chr(k) #chr is the inverse of ord
                    feedbackText = "command: " + command
                    updateImage = True
                elif k == 8: # backspace
                    if len(command) > 0:
                        command = command[:-1]
                    feedbackText = "command: " + command
                    updateImage = True
                elif k == 13: # enter
                    commandNum = setNumStr
                    commandLetter = letter
                    if command.isnumeric():
                        commandNum = command
                        commandLetter = "A"
                    elif len(command)==1 and command.isalpha():
                        commandLetter = command
                    elif len(command)>=2 and command[:-1].isnumeric():
                        commandNum = command[:-1]
                        commandLetter = command[-1]
                    else:
                        feedbackText = "Invalid command: '" + command + "'"

                    if len(commandNum)==0 or len(commandLetter)==0:
                        feedbackText = "Invalid command: '" + command + "'"
                    else:
                        attemptSetNum = int(commandNum)
                        attemptSetNumInds = np.where(setNums == attemptSetNum)[0]
                        if len(attemptSetNumInds) > 0:
                            setNumInd = attemptSetNumInds[0]
                            letterInd = letters.find(commandLetter) #if can't be found, will be -1
                            if letterInd == -1:
                                letterInd = 0
                    updateAllImage = True
                else:
                    feedbackText = "Invalid key: " + chr(k)
                    updateImage = True

    cv2.destroyWindow(windowName)


# Actual code to run
segmentationAlg = 1684
segmentationSuffix = "_noDilation"
#segmentationDesc = str(segmentationAlg)+segmentationSuffix
segmentationDescs = ["1684",  "1684_noDilation", "1684_betterGraphics0_05"]
with open(genFilePath+segmentationDescs[0]+"/data_unanalyzed_"+segmentationDescs[0]+".json") as f0, \
     open(genFilePath+segmentationDescs[1]+"/data_unanalyzed_"+segmentationDescs[1]+".json") as f1, \
     open(genFilePath+segmentationDescs[2]+"/data_unanalyzed_"+segmentationDescs[2]+".json") as f2:
    fs = [f0,f1,f2]
    data = [ json.load(f) for f in fs ]

    increaseCt = 0
    decreaseCt = 0
    sameCt = 0
    sumdiff = 0
    for setKey in data[0]["sets"]:
        for imgKey in data[0]["sets"][setKey]:
            if imgKey.startswith("IMG_"): #setnum will also be a key
                #condition = data[0]["sets"][setKey][imgKey]["analysis"]
                values = [ data[i]["sets"][setKey][imgKey]["analysis"]["IoU"] for i in range(len(segmentationDescs))]

                diff = values[2]-values[0]
                sumdiff += diff
                if abs(diff) >= 0.005:
                    if diff < 0:
                        decreaseCt += 1
                    else:
                        increaseCt += 1
                    #print(imgKey + ": true,  diff=" + "{:+.3f}".format(values[2]-values[0]) + "\t\t" + str(["{:+.3f}".format(x) for x in values]) )
                    print(imgKey + ":  true,\t diff = " + "{:+.3f}".format(diff) + "\t\t [" + ', '.join("{:+.3f}".format(e) for e in values)  + "]" )
                else:
                    sameCt += 1
                    #print(imgKey + ": false, diff=" + "{:.3f}".format(delta) + " " + str(values))
                    #print(imgKey + ": false,\t diff = " + "{:+.3f}".format(sumdiff) + "\t\t [" + ', '.join("{:+.3f}".format(e) for e in values)  + "]" )


    print("increaseCt: " + str(increaseCt) + ",\t decreaseCt: " + str(decreaseCt) + ",\t sameCt: " + str(sameCt) + ",\t increaseCt %: " + "{:.3f}".format(increaseCt/(increaseCt+decreaseCt+sameCt) ) + ",\t decreaseCt %: " + "{:.3f}".format(decreaseCt/(increaseCt+decreaseCt+sameCt) ) )
    print("avgDiff: "+str(sumdiff/(increaseCt+decreaseCt+sameCt)))

view()
