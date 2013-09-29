
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

BUILD_IPC-MODEMCTRL := true
DEBUG := true

LOCAL_MODULE := libmocha-ipc
LOCAL_MODULE_TAGS := optional debug

mocha-ipc_files := \
	mocha-ipc/ipc.c \
	mocha-ipc/ipc_dispatch.c \
	mocha-ipc/misc.c \
	mocha-ipc/util.c \
	mocha-ipc/fm.c \
	mocha-ipc/lbs.c \
	mocha-ipc/proto.c \
	mocha-ipc/sim.c \
	mocha-ipc/drv.c \
	mocha-ipc/tm.c \
	mocha-ipc/syssec.c \
	mocha-ipc/sound.c \
	mocha-ipc/tapi.c \
	mocha-ipc/tapi_call.c \
	mocha-ipc/tapi_nettext.c \
	mocha-ipc/tapi_network.c \
	mocha-ipc/tapi_ss.c \
	mocha-ipc/tapi_at.c \
	mocha-ipc/tapi_dmh.c \
	mocha-ipc/tapi_config.c \
	mocha-ipc/bt.c \
	mocha-ipc/device/$(TARGET_DEVICE)/$(TARGET_DEVICE)_ipc.c


ifeq ($(TARGET_DEVICE),jet)
	LOCAL_CFLAGS += -DDEVICE_JET
endif

ifeq ($(TARGET_DEVICE),wave)
	LOCAL_CFLAGS += -DDEVICE_WAVE
endif

ifeq ($(DEBUG),true)
	LOCAL_CFLAGS += -DDEBUG
	LOCAL_CFLAGS += -DDEBUG_INFO
	LOCAL_CFLAGS += -DLOG_STDOUT
endif

LOCAL_SRC_FILES := $(mocha-ipc_files) $(device_files)

LOCAL_SHARED_LIBRARIES := libutils

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
	$(LOCAL_PATH)/mocha-ipc

include $(BUILD_STATIC_LIBRARY)

ifeq ($(BUILD_IPC-MODEMCTRL),true)

include $(CLEAR_VARS)

LOCAL_MODULE := ipc-modemctrl
LOCAL_MODULE_TAGS := optional debug

modemctrl_files := tools/modemctrl.c

ifeq ($(TARGET_DEVICE),jet)
	LOCAL_CFLAGS += -DDEVICE_JET
endif
ifeq ($(TARGET_DEVICE),wave)
	LOCAL_CFLAGS += -DDEVICE_WAVE
endif

DEBUG := true

ifeq ($(DEBUG),true)
	LOCAL_CFLAGS += -DDEBUG
	LOCAL_CFLAGS += -DDEBUG_INFO
	LOCAL_CFLAGS += -DLOG_STDOUT
endif

LOCAL_SRC_FILES := $(modemctrl_files)

LOCAL_STATIC_LIBRARIES := libmocha-ipc
#LOCAL_SHARED_LIBRARIES := libutils

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	liblog \
	libutils

ifeq ($(TARGET_ARCH),arm)
LOCAL_SHARED_LIBRARIES += libdl
endif # arm

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

include $(BUILD_EXECUTABLE)

endif

include $(CLEAR_VARS)

DEBUG := true

ifeq ($(DEBUG),true)
	LOCAL_CFLAGS += -DDEBUG
	LOCAL_CFLAGS += -DDEBUG_INFO
	LOCAL_CFLAGS += -DLOG_STDOUT
endif

mocha-ril_files := \
	mocha-ril/mocha-ril.c \
	mocha-ril/client.c \
	mocha-ril/ipc.c \
	mocha-ril/srs.c \
	mocha-ril/pwr.c \
	mocha-ril/call.c \
	mocha-ril/misc.c \
	mocha-ril/network.c \
	mocha-ril/sim.c \
	mocha-ril/sms.c \
	mocha-ril/ss.c \
	mocha-ril/snd.c \
	mocha-ril/gprs.c \
	mocha-ril/util.c

LOCAL_SHARED_LIBRARIES := \
	libcutils libutils libril

LOCAL_STATIC_LIBRARIES := libmocha-ipc

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

ifeq ($(TARGET_DEVICE),jet)
	LOCAL_CFLAGS += -DDEVICE_JET
endif
ifeq ($(TARGET_DEVICE),wave)
	LOCAL_CFLAGS += -DDEVICE_WAVE
endif

LOCAL_C_INCLUDES := external/bada-modemril/libmocha-ipc/include
LOCAL_C_INCLUDES += hardware/ril/libmocha-ipc/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mocha-ipc
LOCAL_MODULE_TAGS := optional debug

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := $(mocha-ipc_files) $(mocha-ril_files)

ifeq (foo,foo)
	# build shared library
	LOCAL_SHARED_LIBRARIES += \
		libcutils libnetutils libutils liblog
	LOCAL_LDLIBS += -lpthread
	LOCAL_CFLAGS += -DRIL_SHLIB
	LOCAL_MODULE := libmocha-ril
	include $(BUILD_SHARED_LIBRARY)
else
	# build executable
	LOCAL_SHARED_LIBRARIES += \
		liblog \
		libril
	LOCAL_MODULE := mocha-ril
	include $(BUILD_EXECUTABLE)
endif

include $(CLEAR_VARS)

LOCAL_SRC_FILES := srs-client/srs-client.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/srs-client/include \

LOCAL_SHARED_LIBRARIES := liblog libcutils

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libsrs-client

include $(BUILD_SHARED_LIBRARY)
