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

#include <libnativehelper_test.h>

#include <nativetesthelper_jni/utils.h>

void LibnativehelperTest::SetUp() {
    int result = GetJavaVM()->GetEnv(reinterpret_cast<void**>(&mEnv), JNI_VERSION_1_6);
    EXPECT_EQ(JNI_OK, result);
    EXPECT_NE(nullptr, mEnv);
}

void LibnativehelperTest::TearDown() {
    mEnv = nullptr;
}
