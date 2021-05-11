/*
 * Copyright (C) 2018 The Android Open Source Project
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

#pragma once

#include <memory>

#include <gtest/gtest.h>

#include <jni.h>
#include <nativehelper/JniInvocation.h>

namespace android {

// Example test setup following googletest docs:
//
//   template <typename Provider>
//   class TemplatedTest : public JNITestBase<Provider> {
//      ...
//   }
//
//   typedef ::testing::Types<MockJNIProvider> Providers;
//   TYPED_TEST_CASE(TemplatedTest, Providers);
//
//   TYPED_TEST() {
//     // Test code. Use "this->" to access TemplatedTest members.
//   }



// Provider is a concept that must follow this structure:
//
// class JNIProvider {
// public:
//    JNIProvider();
//
//    void SetUp();
//    JNIEnv* CreateJNIEnv();
//
//    void DestroyJNIEnv(JNIEnv* env);
//    void TearDown();
// }

template <typename Provider, typename Test = ::testing::Test>
class JNITestBase : public Test {
protected:
    JNITestBase() : provider_(), env_(nullptr), java_vm_(nullptr) {
    }

    void SetUp() override {
        Test::SetUp();
        provider_.SetUp();
        env_ = provider_.CreateJNIEnv();
        ASSERT_TRUE(env_ != nullptr);
    }

    void TearDown() override {
        provider_->DestroyJNIEnv(env_);
        provider_->TearDown();
        Test::TearDown();
    }

protected:
    Provider provider_;

    JNIEnv* env_;
    JavaVM* java_vm_;
};

// A mockable implementation of the Provider concept. It is the responsibility
// of the test to stub out any needed functions (all function pointers will be
// null initially).
//
// TODO: Consider googlemock.
class MockJNIProvider {
public:
    MockJNIProvider() {
    }

    void SetUp() {
        // Nothing to here.
    }

    // TODO: Spawn threads to allow more envs?
    JNIEnv* CreateJNIEnv() {
        return CreateMockedJNIEnv().release();
    }

    void DestroyJNIEnv(JNIEnv* env) {
        delete env->functions;
        delete env;
    }

    void TearDown() {
        // Nothing to do here.
    }

protected:
    std::unique_ptr<JNIEnv> CreateMockedJNIEnv() {
        JNINativeInterface* inf = new JNINativeInterface();
        memset(inf, 0, sizeof(JNINativeInterface));

        std::unique_ptr<JNIEnv> ret(new JNIEnv{0});
        ret->functions = inf;

        return ret;
    }
};

}  // namespace android

