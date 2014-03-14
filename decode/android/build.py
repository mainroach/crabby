'''
Copyright 2013 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.'''

import os
import os.path
import fnmatch
import shutil
import sys

'''
This is a helper file to do repeated android builds of native code.
'''
#======================================
def giveFiles(mypath):
    matches = []
    for root, dirnames, filenames in os.walk(mypath):
        for filename in filenames:
            matches.append(os.path.join(root, filename))
    return matches

#======================================
def giveFilesWithExtention(mypath,ext):
    matches = []
    for root, dirnames, filenames in os.walk(mypath):
        for filename in fnmatch.filter(filenames, '*.' + ext):
            matches.append(os.path.join(root, filename))
    return matches

#======================================
def deleteFolder(mypath):
    import os
    import shutil

    for root, dirs, files in os.walk(mypath):
        for f in files:
            #os.unlink(os.path.join(root, f))
            os.remove(os.path.join(root, f))
        for d in dirs:
            shutil.rmtree(os.path.join(root, d))
#======================================
def installToAllDevices(apkPath):

    cmd = "adb devices > devices.txt"
    os.system(cmd)

    devList = open("devices.txt").read()
    lines = devList.split("\n")
    numDevices = len(lines)-1
    if(numDevices==0):
        print "No devices attached"
        return
    elif(numDevices==1):
        cmd = "adb -d install -r " + apkPath
        os.system(cmd)
        return
    else:
        targetDevice = ""
        for i in range(1,len(lines)):
            ln = lines[i]
            if ln=="":break
            devID = ln.split("\t")[0]
            print "Installing to " + devID + "......"
            cmd = "adb -s " + devID + " install -r " + apkPath
            os.system(cmd )
            targetDevice=devID

        return targetDevice





    
    

if __name__ == "__main__":

    os.system("cls")
    print "#======================================"

    projectName = "NativeActivity"
    ndkSOName = "native-activity"
    apkName = "./bin/" + projectName + "-debug.apk"

    doFastBuild = False
    startApp = False
    startLogcat = True

    if not doFastBuild:
        
        deleteFolder("./bin")
        deleteFolder("./gen")
        deleteFolder("./libs/armeabi")
        deleteFolder("./libs/armeabi-v7a")
        deleteFolder("./obj")
  

    if not os.path.exists("build.xml"):
        os.system("android update project -p . --target android-19")
    


    
    #call the compile
    #print "done writing mk, press any key to continue"
    #raw_input()
    cmd = "ndk-build"
    os.system(cmd )

    if not os.path.exists("libs/armeabi/lib" + ndkSOName + ".so"):
        print "<<<<<<<<<<<<<<<<<<< Didn't find built .SO file >>>>>>>>>>>>>>>>>>>>>>>>>>>"
        sys.exit(1)
    

    #build debug
    #print "done ndk-build, press any key to continue"
    #raw_input()
    if os.path.exists(apkName):
        os.remove(apkName); #remove the existing apk to ensure we're not screwing things up
    cmd = "ant debug"
    os.system(cmd )

    
    if not os.path.exists(apkName):
        print "<<<<<<<<<<<<<<<<<<< Didn't find built .APK file >>>>>>>>>>>>>>>>>>>>>>>>>>>"
        sys.exit(1)
        
    
    targetDeviceID = installToAllDevices(apkName)
    #cmd = "adb -d install -r ./bin/" + projectName + "-debug.apk"
    #os.system(cmd )
    

    #clear the logcat buffer
    cmd = "adb logcat -c"
    os.system(cmd )


    if startApp:
        cmd = "adb shell am start -n com.example.native_activity/android.app." + projectName
        os.system(cmd )
    elif startLogcat:
        cmd = "adb logcat"
        os.system(cmd )



