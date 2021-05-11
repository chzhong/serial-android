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

#include "libnativehelper_test.h"

TEST_F(LibnativehelperTest, GetCreatedJavaVMs) {
    JavaVM* createdVMs[2] = { nullptr, nullptr };
    jsize count;
    ASSERT_NE(nullptr, mEnv);
    ASSERT_EQ(JNI_OK, JNI_GetCreatedJavaVMs(&createdVMs[0], 2, &count));
    ASSERT_EQ(1, count);
    ASSERT_NE(nullptr, createdVMs[0]);
    ASSERT_EQ(nullptr, createdVMs[1]);

    JavaVM* currentVM;
    ASSERT_EQ(JNI_OK, mEnv->GetJavaVM(&currentVM));
    ASSERT_EQ(createdVMs[0], currentVM);
}

TEST_F(LibnativehelperTest, GetDefaultJavaVMInitArgs) {
    JavaVMOption options[1];
    JavaVMInitArgs initArgs;
    initArgs.version = JNI_VERSION_1_6;
    initArgs.nOptions = 0;
    initArgs.options = options;
    initArgs.ignoreUnrecognized = JNI_TRUE;
    // ART does not support JNI_GetDefaultJavaVMInitArgs(), should this change it'll need a test.
    ASSERT_EQ(JNI_ERR, JNI_GetDefaultJavaVMInitArgs(&initArgs));
}
