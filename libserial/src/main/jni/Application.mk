# NDK_TOOLCHAIN_VERSION := clang

ifeq ($(NDK_DEBUG), 1)
APP_OPTIM := debug
else
APP_OPTIM := release
endif

APP_PLATFORM := android-28
APP_ABI := arm64-v8a armeabi-v7a x86
APP_STL := c++_static
#APP_CFLAGS := -std=gnu11
APP_CPPFLAGS := -fexceptions
