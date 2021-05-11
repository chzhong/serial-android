/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iterator>

#include <dlfcn.h>
#include <jni.h>

#include <android/log.h>
#include <nativehelper/jni_macros.h>
#include <nativehelper/scoped_local_ref.h>
#include <nativehelper/scoped_string_chars.h>
#include <nativehelper/scoped_utf_chars.h>
#include <nativehelper/JNIPlatformHelp.h>

#include "libnativehelper_test.h"

namespace {

void AssertionError(JNIEnv* env, const char* format, ...) {
    // There is some awkwardness with java/lang/AssertionError. It cannot be directly thrown
    // by jniThrowException or the JNI ThrowNew methods because AssertionError(String) is not
    // public so we instead use the AssertionError(String, Throwable) constructor.
    va_list args;
    va_start(args, format);
    char messageBuffer[256];
    vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
    va_end(args);

    ScopedLocalRef<jclass> jlaeClass(env, env->FindClass("java/lang/AssertionError"));
    jmethodID init =
        env->GetMethodID(jlaeClass.get(), "<init>", "(Ljava/lang/String;Ljava/lang/Throwable;)V");
    ScopedLocalRef<jstring> message(env, env->NewStringUTF(messageBuffer));
    jobject jlae = env->NewObject(jlaeClass.get(), init, message.get(), nullptr);
    env->Throw(reinterpret_cast<jthrowable>(jlae));
}

static void throwException(JNIEnv* env, jclass /*clazz*/, jstring className, jstring message) {
    ScopedUtfChars c(env, className);
    ScopedUtfChars m(env, message);
    jniThrowException(env, c.c_str(), m.c_str());
}

static void throwExceptionWithIntFormat(JNIEnv* env,
                                        jclass /*clazz*/,
                                        jstring className,
                                        jstring format,
                                        jint value) {
    ScopedUtfChars c(env, className);
    ScopedUtfChars f(env, format);
    jniThrowExceptionFmt(env, c.c_str(), f.c_str(), value);
}

static void throwNullPointerException(JNIEnv* env,
                                      jclass /*clazz*/,
                                      jstring message) {
    ScopedUtfChars m(env, message);
    jniThrowNullPointerException(env, m.c_str());
}

static void throwRuntimeException(JNIEnv* env, jclass /*clazz*/, jstring message) {
    ScopedUtfChars m(env, message);
    jniThrowRuntimeException(env, m.c_str());
}

static void throwIOException(JNIEnv* env, jclass /*clazz*/, jint cause) {
    jniThrowIOException(env, cause);
}

static void throwErrnoException(JNIEnv* env, jclass /*clazz*/, jstring functionName, jint cause) {
    ScopedUtfChars m(env, functionName);
    jniThrowErrnoException(env, m.c_str(), cause);
}

static void logException(JNIEnv* env, jclass /*clazz*/, jthrowable throwable) {
    jniLogException(env, ANDROID_LOG_VERBOSE, __FILE__, throwable);
}

static jobject fileDescriptorCreate(JNIEnv* env, jclass /*clazz*/, jint unix_fd) {
    return jniCreateFileDescriptor(env, unix_fd);
}

static jint fileDescriptorGetFD(JNIEnv* env, jclass /*clazz*/, jobject jiofd) {
    return jniGetFDFromFileDescriptor(env, jiofd);
}

static void fileDescriptorSetFD(JNIEnv* env, jclass /*clazz*/, jobject jiofd, jint unix_fd) {
    jniSetFileDescriptorOfFD(env, jiofd, unix_fd);
}

static jobject allocateDirectNonHeapBuffer(JNIEnv* env, jclass /*clazz*/, jint length) {
    static uint8_t raw_memory[4096];
    if (length < 0 || length > sizeof(raw_memory)) {
        return nullptr;
    }
    return env->NewDirectByteBuffer(&raw_memory, length);
}

static void
assertBufferBaseArrayOffsetBytes(JNIEnv* env, jclass /*clazz*/, jobject jnb, jint offset) {
    int actualOffset = jniGetNioBufferBaseArrayOffset(env, jnb);
    if (actualOffset != offset) {
        AssertionError(env,
                       "Buffer offset incorrect (expected %d, actual %d)",
                       offset,
                       actualOffset);
    }
}

static void assertBufferPosition(JNIEnv* env, jclass /*clazz*/, jobject jnb, jint position) {
    int actualPosition, actualLimit, actualElementSizeShift;
    jniGetNioBufferFields(env, jnb, &actualPosition, &actualLimit, &actualElementSizeShift);
    if (actualPosition != position) {
        AssertionError(env,
                       "Buffer position incorrect (expected %d, actual %d)",
                       position,
                       actualPosition);
    }
}

static void assertBufferLimit(JNIEnv* env, jclass /*clazz*/, jobject jnb, jint limit) {
    int actualPosition, actualLimit, actualElementSizeShift;
    jniGetNioBufferFields(env, jnb, &actualPosition, &actualLimit, &actualElementSizeShift);
    if (actualLimit != limit) {
        AssertionError(env,
                       "Buffer limit incorrect (expected %d, actual %d)",
                       limit,
                       actualLimit);
    }
}

static void assertBufferElementSizeShift(JNIEnv* env, jclass /*clazz*/, jobject jnb, jint ess) {
    int actualPosition, actualLimit, actualElementSizeShift;
    jniGetNioBufferFields(env, jnb, &actualPosition, &actualLimit, &actualElementSizeShift);
    if (actualElementSizeShift != ess) {
        AssertionError(env,
                       "Buffer element size shift incorrect (expected %d, actual %d)",
                       ess,
                       actualElementSizeShift);
    }
}

static jlong getBufferBaseAddress(JNIEnv* env, jclass /*clazz*/, jobject jnb) {
    int actualPosition, actualLimit, actualElementSizeShift;
    return jniGetNioBufferFields(env, jnb, &actualPosition, &actualLimit, &actualElementSizeShift);
}

static jlong getDirectBufferAddress(JNIEnv* env, jclass /*clazz*/, jobject jnb) {
    void* directBufferAddress = env->GetDirectBufferAddress(jnb);
    return reinterpret_cast<jlong>(directBufferAddress);
}

static void assertBufferPointer(JNIEnv* env, jclass /*clazz*/, jobject jnb, jlong pointer) {
    jlong actualPointer = jniGetNioBufferPointer(env, jnb);
    if (actualPointer != pointer) {
        AssertionError(env,
                      "Buffer pointer incorrect (expected %p, actual %p)",
                       reinterpret_cast<void*>(pointer),
                       reinterpret_cast<void*>(actualPointer));
    }
}

static jstring createString(JNIEnv* env, jclass /*clazz*/, jstring value) {
    ScopedStringChars ssc(env, value);
    return jniCreateString(env, ssc.get(), ssc.size());
}

}  // namespace

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    static const JNINativeMethod methods[] = {
        MAKE_JNI_NATIVE_METHOD("throwException",
                               "(Ljava/lang/String;Ljava/lang/String;)V",
                               throwException),
        MAKE_JNI_NATIVE_METHOD("throwExceptionWithIntFormat",
                               "(Ljava/lang/String;Ljava/lang/String;I)V",
                               throwExceptionWithIntFormat),
        MAKE_JNI_NATIVE_METHOD("throwNullPointerException",
                               "(Ljava/lang/String;)V",
                               throwNullPointerException),
        MAKE_JNI_NATIVE_METHOD("throwRuntimeException",
                               "(Ljava/lang/String;)V",
                               throwRuntimeException),
        MAKE_JNI_NATIVE_METHOD("throwIOException",
                               "(I)V",
                               throwIOException),
        MAKE_JNI_NATIVE_METHOD("throwErrnoException",
                               "(Ljava/lang/String;I)V",
                               throwErrnoException),
        MAKE_JNI_NATIVE_METHOD("logException",
                               "(Ljava/lang/Throwable;)V",
                               logException),
        MAKE_JNI_NATIVE_METHOD("fileDescriptorCreate",
                               "(I)Ljava/io/FileDescriptor;",
                               fileDescriptorCreate),
        MAKE_JNI_NATIVE_METHOD("fileDescriptorGetFD",
                               "(Ljava/io/FileDescriptor;)I",
                               fileDescriptorGetFD),
        MAKE_JNI_NATIVE_METHOD("fileDescriptorSetFD",
                               "(Ljava/io/FileDescriptor;I)V",
                               fileDescriptorSetFD),
        MAKE_JNI_NATIVE_METHOD("allocateDirectNonHeapBuffer",
                               "(I)Ljava/nio/ByteBuffer;",
                               allocateDirectNonHeapBuffer),
        MAKE_JNI_NATIVE_METHOD("assertBufferBaseArrayOffsetBytes",
                               "(Ljava/nio/Buffer;I)V",
                               assertBufferBaseArrayOffsetBytes),
        MAKE_JNI_NATIVE_METHOD("assertBufferPosition",
                               "(Ljava/nio/Buffer;I)V",
                               assertBufferPosition),
        MAKE_JNI_NATIVE_METHOD("assertBufferLimit",
                               "(Ljava/nio/Buffer;I)V",
                               assertBufferLimit),
        MAKE_JNI_NATIVE_METHOD("assertBufferElementSizeShift",
                               "(Ljava/nio/Buffer;I)V",
                               assertBufferElementSizeShift),
        MAKE_JNI_NATIVE_METHOD("getBufferBaseAddress",
                               "(Ljava/nio/Buffer;)J",
                               getBufferBaseAddress),
        MAKE_JNI_NATIVE_METHOD("getDirectBufferAddress",
                               "(Ljava/nio/Buffer;)J",
                               getDirectBufferAddress),
        MAKE_JNI_NATIVE_METHOD("assertBufferPointer",
                               "(Ljava/nio/Buffer;J)V",
                               assertBufferPointer),
        MAKE_JNI_NATIVE_METHOD("createString",
                               "(Ljava/lang/String;)Ljava/lang/String;",
                               createString),
    };
    int rc = jniRegisterNativeMethods(env,
                                      "com/android/art/libnativehelper/JniHelpTest",
                                      methods,
                                      std::size(methods));
    if (rc != JNI_OK) return rc;

    return JNI_VERSION_1_6;
}
