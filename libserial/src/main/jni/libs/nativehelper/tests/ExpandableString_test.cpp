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

#include <array>
#include <string>

#include <gtest/gtest.h>
#include <string.h>

#include "../ExpandableString.h"


TEST(ExpandableString, InitializeAppendRelease) {
    const char* kAhoy = "Ahoy!";
    struct ExpandableString s;
    ExpandableStringInitialize(&s);
    EXPECT_TRUE(s.data == NULL);
    EXPECT_EQ(s.dataSize, 0u);
    EXPECT_TRUE(ExpandableStringAppend(&s, kAhoy));
    EXPECT_TRUE(s.data != NULL);
    EXPECT_GE(s.dataSize, strlen(kAhoy));
    ExpandableStringRelease(&s);
    EXPECT_TRUE(s.data == NULL);
    EXPECT_GE(s.dataSize, 0u);
}

TEST(ExpandableString, InitializeWriteRelease) {
    const char* kAhoy = "Ahoy!";
    const char* kMercy = "Mercy, Mercy, Mercy!";

    struct ExpandableString s;
    ExpandableStringInitialize(&s);
    EXPECT_TRUE(s.data == NULL);
    EXPECT_EQ(s.dataSize, 0u);
    EXPECT_TRUE(ExpandableStringAssign(&s, kAhoy));
    EXPECT_TRUE(s.data != NULL);
    EXPECT_GE(s.dataSize, strlen(kAhoy));
    EXPECT_TRUE(ExpandableStringAssign(&s, kMercy));
    EXPECT_TRUE(s.data != NULL);
    EXPECT_GE(s.dataSize, strlen(kMercy));
    EXPECT_TRUE(ExpandableStringAssign(&s, kAhoy));
    EXPECT_TRUE(s.data != NULL);
    EXPECT_GE(s.dataSize, strlen(kAhoy));
    ExpandableStringRelease(&s);
    EXPECT_TRUE(s.data == NULL);
    EXPECT_GE(s.dataSize, 0u);
}

class ExpandableStringTestFixture : public :: testing::TestWithParam<size_t> {
    protected:
        struct ExpandableString expandableString;
};

TEST_P(ExpandableStringTestFixture, AppendTest) {
   size_t step = GetParam();

   std::array<std::string, 3> inputs = {
      std::string(step, 'a'),
      std::string(step, 'b'),
      std::string(step, 'c'),
   };

   for (size_t offset = 0; offset < step; ++offset) {
      ExpandableStringInitialize(&expandableString);

      std::string pad(step - 1u, '_');
      EXPECT_TRUE(ExpandableStringAppend(&expandableString, pad.c_str()));

      for (size_t i = 0; i < 4096u; ++i) {
         const std::string& appendee = inputs[i % inputs.size()];
         EXPECT_TRUE(ExpandableStringAppend(&expandableString, appendee.c_str()));
         size_t requiredSize = pad.size() + i * step + 1u;
         EXPECT_GE(expandableString.dataSize, requiredSize);
      }

      size_t position = 0u;
      for (char c : pad) {
         EXPECT_EQ(c, expandableString.data[position]);
         position++;
      }
      for (size_t i = 0; i < 4096; ++i) {
         const std::string& expected = inputs[i % inputs.size()];
         EXPECT_EQ(0, strncmp(expected.c_str(), expandableString.data + position, expected.size()));
         position += expected.size();
      }

      ExpandableStringRelease(&expandableString);
   }
}

INSTANTIATE_TEST_CASE_P(
         AppendTest,
         ExpandableStringTestFixture,
         ::testing::Values(
                 1, 2, 3, 4, 5, 11, 17
         ));