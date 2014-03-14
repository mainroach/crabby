LOCAL_PATH := $(call my-dir)  #should be this value
include $(CLEAR_VARS)

#We are required to have an ANDROID.MK and an APPLICATION.MK in this jni folder.
#Typically, it's suggested that you put all your .c/.cpp files in this folder as well,
# and then the "LOCAL_SRC_FILES" variable is used to list each source file specifically.
#Since we want our code to be available to move between platforms, we do not want to put it all here
#Instead, we want to have it up, in a higher directory, and have this build point to that.
#But since we are required to have an ANDROID.MK in each sub-directory, we need to place one there
# and then reference it, from this make file. Hence the include statement below.


#Define what our activity name is
LOCAL_MODULE    := native-activity

#These are the libs we want to load
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2

#defined, since we want to be a native activity
LOCAL_STATIC_LIBRARIES := android_native_app_glue

FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

#include statement for shared libs
include $(BUILD_SHARED_LIBRARY)

#call the compile process
$(call import-module,android/native_app_glue)
