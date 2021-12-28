LOCAL_PATH := $(call my-dir)

OASIS_LIB_PATH := $(LOCAL_PATH)/oasis

include $(CLEAR_VARS)
LOCAL_MODULE := oasis

LOCAL_SRC_FILES := $(OASIS_LIB_PATH)/liboasis_lite2D_DEFAULT_android_reg.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := JniAlgorithm
LOCAL_SRC_FILES := Algorithm.cpp

LOCAL_C_INCLUDES := $(OASIS_LIB_PATH)/include
LOCAL_CPP_INCLUDES := $(OASIS_LIB_PATH)/include

LOCAL_STATIC_LIBRARIES := oasis

LOCAL_CFLAGS := -O2 -fvisibility=hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math  -Os -s -frtti
LOCAL_CPPFLAGS := -O2 -fvisibility=hidden -fvisibility-inlines-hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math  -Os -s -frtti
LOCAL_LDFLAGS += -Wl,--gc-sections

#LOCAL_CFLAGS += -fopenmp
#LOCAL_CPPFLAGS += -fopenmp
#LOCAL_LDFLAGS += -fopenmp

LOCAL_LDLIBS := -lz -llog -ljnigraphics

include $(BUILD_SHARED_LIBRARY)
