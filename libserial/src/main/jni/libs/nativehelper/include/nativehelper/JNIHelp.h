/*
 * Copyright (C) 2007 The Android Open Source Project
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

/*
 * JNI helper functions.
 *
 * This file may be included by C or C++ code, which is trouble because jni.h
 * uses different typedefs for JNIEnv in each language.
 */
#pragma once

#include <sys/cdefs.h>

#include <errno.h>
#include <unistd.h>

#include <jni.h>

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

__BEGIN_DECLS


/*
 * Register one or more native methods with a particular class.  "className" looks like
 * "java/lang/String". Aborts on failure, returns 0 on success.
 */
int jniRegisterNativeMethods(C_JNIEnv* env,
                             const char* className,
                             const JNINativeMethod* gMethods,
                             int numMethods);

/*
 * Throw an exception with the specified class and an optional message.
 *
 * The "className" argument will be passed directly to FindClass, which
 * takes strings with slashes (e.g. "java/lang/Object").
 *
 * If an exception is currently pending, we log a warning message and
 * clear it.
 *
 * Returns 0 on success, nonzero if something failed (e.g. the exception
 * class couldn't be found, so *an* exception will still be pending).
 *
 * Currently aborts the VM if it can't throw the exception.
 */
int jniThrowException(C_JNIEnv* env, const char* className, const char* msg);

/*
 * Throw an exception with the specified class and formatted error message.
 *
 * The "className" argument will be passed directly to FindClass, which
 * takes strings with slashes (e.g. "java/lang/Object").
 *
 * If an exception is currently pending, we log a warning message and
 * clear it.
 *
 * Returns 0 on success, nonzero if something failed (e.g. the exception
 * class couldn't be found, so *an* exception will still be pending).
 *
 * Currently aborts the VM if it can't throw the exception.
 */
int jniThrowExceptionFmt(C_JNIEnv* env, const char* className, const char* fmt, va_list args);

/*
 * Throw a java.lang.NullPointerException, with an optional message.
 */
int jniThrowNullPointerException(C_JNIEnv* env, const char* msg);

/*
 * Throw a java.lang.RuntimeException, with an optional message.
 */
int jniThrowRuntimeException(C_JNIEnv* env, const char* msg);

/*
 * Throw a java.io.IOException, generating the message from the given errno value.
 */
int jniThrowIOException(C_JNIEnv* env, int errno_value);

/*
 * Throw an android.system.ErrnoException, with the given function name and errno value.
 */
int jniThrowErrnoException(C_JNIEnv* env, const char* functionName, int errno_value);

/*
 * Returns a Java String object created from UTF-16 data either from jchar or,
 * if called from C++11, char16_t (a bitwise identical distinct type).
 */
jstring jniCreateString(C_JNIEnv* env, const jchar* unicodeChars, jsize len);

/*
 * Log a message and an exception.
 * If exception is NULL, logs the current exception in the JNI environment.
 */
void jniLogException(C_JNIEnv* env, int priority, const char* tag, jthrowable exception);

__END_DECLS

/*
 * For C++ code, we provide inlines that map to the C functions.  g++ always
 * inlines these, even on non-optimized builds.
 */
#if defined(__cplusplus)

inline int jniRegisterNativeMethods(JNIEnv* env, const char* className, const JNINativeMethod* gMethods, int numMethods) {
    return jniRegisterNativeMethods(&env->functions, className, gMethods, numMethods);
}

inline int jniThrowException(JNIEnv* env, const char* className, const char* msg) {
    return jniThrowException(&env->functions, className, msg);
}

/*
 * Equivalent to jniThrowException but with a printf-like format string and
 * variable-length argument list. This is only available in C++.
 */
inline int jniThrowExceptionFmt(JNIEnv* env, const char* className, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    return jniThrowExceptionFmt(&env->functions, className, fmt, args);
    va_end(args);
}

inline int jniThrowNullPointerException(JNIEnv* env, const char* msg) {
    return jniThrowNullPointerException(&env->functions, msg);
}

inline int jniThrowRuntimeException(JNIEnv* env, const char* msg) {
    return jniThrowRuntimeException(&env->functions, msg);
}

inline int jniThrowIOException(JNIEnv* env, int errnum) {
    return jniThrowIOException(&env->functions, errnum);
}

inline int jniThrowErrnoException(JNIEnv* env, const char* functionName, int errnum) {
    return jniThrowErrnoException(&env->functions, functionName, errnum);
}

inline jstring jniCreateString(JNIEnv* env, const jchar* unicodeChars, jsize len) {
    return jniCreateString(&env->functions, unicodeChars, len);
}

inline jstring jniCreateString(JNIEnv* env, const char16_t* unicodeChars, jsize len) {
    return jniCreateString(&env->functions, reinterpret_cast<const jchar*>(unicodeChars), len);
}

inline void jniLogException(JNIEnv* env, int priority, const char* tag, jthrowable exception = NULL) {
    jniLogException(&env->functions, priority, tag, exception);
}

#endif  // defined(__cplusplus)

