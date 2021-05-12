/*
 * Copyright (C) 2014 The Android Open Source Project
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

#include "../JniInvocation-priv.h"

#include <gtest/gtest.h>
#include <jni.h>


static const char* kDefaultJniInvocationLibrary = "libart.so";
static const char* kTestNonNull = "libartd.so";
static const char* kTestNonNull2 = "libartd2.so";

TEST(JNIInvocation, Debuggable) {
    const char* result = JniInvocationGetLibraryWith(nullptr, true, kTestNonNull2);
    EXPECT_STREQ(result, kTestNonNull2);

    result = JniInvocationGetLibraryWith(kTestNonNull, true, kTestNonNull2);
    EXPECT_STREQ(result, kTestNonNull);

    result = JniInvocationGetLibraryWith(kTestNonNull, true, nullptr);
    EXPECT_STREQ(result, kTestNonNull);

    result = JniInvocationGetLibraryWith(nullptr, true, nullptr);
    EXPECT_STREQ(result, kDefaultJniInvocationLibrary);
}

TEST(JNIInvocation, NonDebuggable) {
    const char* result = JniInvocationGetLibraryWith(nullptr, false, kTestNonNull2);
    EXPECT_STREQ(result, kDefaultJniInvocationLibrary);

    result = JniInvocationGetLibraryWith(kTestNonNull, false, kTestNonNull2);
    EXPECT_STREQ(result, kDefaultJniInvocationLibrary);

    result = JniInvocationGetLibraryWith(kTestNonNull, false, nullptr);
    EXPECT_STREQ(result, kDefaultJniInvocationLibrary);

    result = JniInvocationGetLibraryWith(nullptr, false, nullptr);
    EXPECT_STREQ(result, kDefaultJniInvocationLibrary);
}

TEST(JNIInvocation, GetDefaultJavaVMInitArgsBeforeInit) {
    EXPECT_DEATH(JNI_GetDefaultJavaVMInitArgs(nullptr), "Runtime library not loaded.");
}

TEST(JNIInvocation, CreateJavaVMBeforeInit) {
    JavaVM *vm;
    JNIEnv *env;
    EXPECT_DEATH(JNI_CreateJavaVM(&vm, &env, nullptr), "Runtime library not loaded.");
}

TEST(JNIInvocation, GetCreatedJavaVMsBeforeInit) {
    jsize vm_count;
    JavaVM *vm;
    int status = JNI_GetCreatedJavaVMs(&vm, 1, &vm_count);
    EXPECT_EQ(status, JNI_OK);
    EXPECT_EQ(vm_count, 0);
}
