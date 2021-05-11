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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#pragma clang diagnostic ignored "-Wdeprecated"
#include <gtest/gtest.h>
#pragma clang diagnostic pop
#include <sstream>

#define PARSE_FAILURES_NONFATAL  // return empty optionals wherever possible instead of asserting.
#include "nativehelper/jni_macros.h"

// Provide static storage to these values so they can be used in a runtime context.
// This has to be defined local to the test translation unit to avoid ODR violations prior to C++17.
#define STORAGE_FN_FOR_JNI_TRAITS(jtype, ...)                                  \
constexpr char nativehelper::detail::jni_type_trait<jtype>::type_descriptor[]; \
constexpr char nativehelper::detail::jni_type_trait<jtype>::type_name[];

DEFINE_JNI_TYPE_TRAIT(STORAGE_FN_FOR_JNI_TRAITS)

template <typename T>
std::string stringify_helper(const T& val) {
  std::stringstream ss;
  ss << val;
  return ss.str();
}

#define EXPECT_STRINGIFY_EQ(x, y) EXPECT_EQ(stringify_helper(x), stringify_helper(y))

TEST(JniSafeRegisterNativeMethods, StringParsing) {
  using namespace nativehelper::detail;  // NOLINT

  // Super basic bring-up tests for core functionality.

  {
    constexpr ConstexprStringView v_str = "V";
    EXPECT_EQ(1u, v_str.size());
    EXPECT_EQ(false, v_str.empty());

    std::stringstream ss;
    ss << v_str;
    EXPECT_EQ("V", ss.str());
  }

  {
    auto parse = ParseSingleTypeDescriptor("", /*allow_void*/true);
    EXPECT_EQ("", parse->token);
    EXPECT_EQ("", parse->remainder);
  }

  {
    auto parse = ParseSingleTypeDescriptor("V", /*allow_void*/true);
    EXPECT_EQ("V", parse->token);
    EXPECT_EQ("", parse->remainder);
  }

  {
    auto parse = ParseSingleTypeDescriptor("[I");
    EXPECT_EQ("[I", parse->token);
    EXPECT_EQ("", parse->remainder);
  }

  {
    auto parse = ParseSingleTypeDescriptor("LObject;");
    EXPECT_EQ("LObject;", parse->token);
    EXPECT_EQ("", parse->remainder);
  }

  {
    auto parse = ParseSingleTypeDescriptor("LBadObject);");
    EXPECT_FALSE(parse.has_value());
  }

  {
    auto parse = ParseSingleTypeDescriptor("LBadObject(;");
    EXPECT_FALSE(parse.has_value());
  }

  {
    auto parse = ParseSingleTypeDescriptor("LBadObject[;");
    EXPECT_FALSE(parse.has_value());
  }

  // Stringify is used for convenience to make writing out tests easier.
  // Transforms as "(XYZ)W" -> "args={X,Y,Z}, ret=W"

#define PARSE_SIGNATURE_AS_LIST(str) (ParseSignatureAsList<sizeof(str)>(str))

  {
    constexpr auto jni_descriptor = PARSE_SIGNATURE_AS_LIST("()V");
    EXPECT_STRINGIFY_EQ("args={}, ret=V", jni_descriptor);
  }

  {
    constexpr auto
        jni_descriptor = PARSE_SIGNATURE_AS_LIST("()Ljava/lang/Object;");
    EXPECT_STRINGIFY_EQ("args={}, ret=Ljava/lang/Object;", jni_descriptor);
  }

  {
    constexpr auto jni_descriptor = PARSE_SIGNATURE_AS_LIST("()[I");
    EXPECT_STRINGIFY_EQ("args={}, ret=[I", jni_descriptor);
  }

#define EXPECT_OK_SIGNATURE_PARSE(signature, args, ret) \
  do { \
    constexpr auto jni_descriptor = PARSE_SIGNATURE_AS_LIST(signature); \
    EXPECT_EQ(true, jni_descriptor.has_value());                        \
    EXPECT_STRINGIFY_EQ("args={" args "}, ret=" ret, jni_descriptor);   \
  } while (0)

  // Exhaustive tests for successful parsing.

  EXPECT_OK_SIGNATURE_PARSE("()V", /*args*/"", /*ret*/"V");
  EXPECT_OK_SIGNATURE_PARSE("()Z", /*args*/"", /*ret*/"Z");
  EXPECT_OK_SIGNATURE_PARSE("()B", /*args*/"", /*ret*/"B");
  EXPECT_OK_SIGNATURE_PARSE("()C", /*args*/"", /*ret*/"C");
  EXPECT_OK_SIGNATURE_PARSE("()S", /*args*/"", /*ret*/"S");
  EXPECT_OK_SIGNATURE_PARSE("()I", /*args*/"", /*ret*/"I");
  EXPECT_OK_SIGNATURE_PARSE("()F", /*args*/"", /*ret*/"F");
  EXPECT_OK_SIGNATURE_PARSE("()J", /*args*/"", /*ret*/"J");
  EXPECT_OK_SIGNATURE_PARSE("()D", /*args*/"", /*ret*/"D");
  EXPECT_OK_SIGNATURE_PARSE("()Ljava/lang/Object;", /*args*/"", /*ret*/"Ljava/lang/Object;");
  EXPECT_OK_SIGNATURE_PARSE("()[Ljava/lang/Object;", /*args*/"", /*ret*/"[Ljava/lang/Object;");
  EXPECT_OK_SIGNATURE_PARSE("()[I", /*args*/"", /*ret*/"[I");
  EXPECT_OK_SIGNATURE_PARSE("()[[I", /*args*/"", /*ret*/"[[I");
  EXPECT_OK_SIGNATURE_PARSE("()[[[I", /*args*/"", /*ret*/"[[[I");


  EXPECT_OK_SIGNATURE_PARSE("(Z)V", /*args*/"Z", /*ret*/"V");
  EXPECT_OK_SIGNATURE_PARSE("(B)V", /*args*/"B", /*ret*/"V");
  EXPECT_OK_SIGNATURE_PARSE("(C)D", /*args*/"C", /*ret*/"D");
  EXPECT_OK_SIGNATURE_PARSE("(S)V", /*args*/"S", /*ret*/"V");
  EXPECT_OK_SIGNATURE_PARSE("(I)V", /*args*/"I", /*ret*/"V");
  EXPECT_OK_SIGNATURE_PARSE("(F)V", /*args*/"F", /*ret*/"V");
  EXPECT_OK_SIGNATURE_PARSE("(J)F", /*args*/"J", /*ret*/"F");
  EXPECT_OK_SIGNATURE_PARSE("(D)V", /*args*/"D", /*ret*/"V");
  EXPECT_OK_SIGNATURE_PARSE("(Ljava/lang/Object;)V", "Ljava/lang/Object;", "V");
  EXPECT_OK_SIGNATURE_PARSE("([Ljava/lang/Object;)V",
                            "[Ljava/lang/Object;",
                            "V");
  EXPECT_OK_SIGNATURE_PARSE("([I)V", /*ret*/"[I", "V");
  EXPECT_OK_SIGNATURE_PARSE("([[I)V", /*ret*/"[[I", "V");
  EXPECT_OK_SIGNATURE_PARSE("([[[I)V", /*ret*/"[[[I", "V");

  EXPECT_OK_SIGNATURE_PARSE("(ZIJ)V", /*args*/"Z,I,J", /*ret*/"V");
  EXPECT_OK_SIGNATURE_PARSE("(B[IJ)V", /*args*/"B,[I,J", /*ret*/"V");
  EXPECT_OK_SIGNATURE_PARSE("(Ljava/lang/Object;B)D",
                            /*args*/"Ljava/lang/Object;,B",
                            /*ret*/"D");
  EXPECT_OK_SIGNATURE_PARSE("(Ljava/lang/Object;Ljava/lang/String;IF)D",
                            /*args*/"Ljava/lang/Object;,Ljava/lang/String;,I,F",
                            /*ret*/"D");
  EXPECT_OK_SIGNATURE_PARSE("([[[Ljava/lang/Object;Ljava/lang/String;IF)D",
                            /*args*/"[[[Ljava/lang/Object;,Ljava/lang/String;,I,F",
                            /*ret*/"D");

  /*
   * Test Failures in Parsing
   */

#define EXPECT_FAILED_SIGNATURE_PARSE(jni_descriptor) \
  EXPECT_STRINGIFY_EQ(ConstexprOptional<JniSignatureDescriptor<sizeof(jni_descriptor)>>{},\
                      ParseSignatureAsList<sizeof(jni_descriptor)>(jni_descriptor))

  // For the failures to work we must turn off 'PARSE_FAILURES_FATAL'.
  // Otherwise they immediately cause a crash, which is actually the desired behavior
  // when this is used by the end-user in REGISTER_NATIVE_METHOD.
  {
    EXPECT_FAILED_SIGNATURE_PARSE("");
    EXPECT_FAILED_SIGNATURE_PARSE("A");
    EXPECT_FAILED_SIGNATURE_PARSE(")");
    EXPECT_FAILED_SIGNATURE_PARSE("V");
    EXPECT_FAILED_SIGNATURE_PARSE("(");
    EXPECT_FAILED_SIGNATURE_PARSE("(A");
    EXPECT_FAILED_SIGNATURE_PARSE("()");
    EXPECT_FAILED_SIGNATURE_PARSE("()A");
    EXPECT_FAILED_SIGNATURE_PARSE("()VV");
    EXPECT_FAILED_SIGNATURE_PARSE("()L");
    EXPECT_FAILED_SIGNATURE_PARSE("()L;");
    EXPECT_FAILED_SIGNATURE_PARSE("()BAD;");
    EXPECT_FAILED_SIGNATURE_PARSE("()Ljava/lang/Object");
    EXPECT_FAILED_SIGNATURE_PARSE("()Ljava/lang/Object;X");

    EXPECT_FAILED_SIGNATURE_PARSE("(V)V");
    EXPECT_FAILED_SIGNATURE_PARSE("(ILcat)V");
    EXPECT_FAILED_SIGNATURE_PARSE("([dog)V");
    EXPECT_FAILED_SIGNATURE_PARSE("(IV)V");
    EXPECT_FAILED_SIGNATURE_PARSE("([V)V");
    EXPECT_FAILED_SIGNATURE_PARSE("([[V)V");
    EXPECT_FAILED_SIGNATURE_PARSE("()v");
    EXPECT_FAILED_SIGNATURE_PARSE("()i");
    EXPECT_FAILED_SIGNATURE_PARSE("()f");
  }

}

#define EXPECT_IS_VALID_JNI_ARGUMENT_TYPE(expected, expr) \
 { constexpr bool is_valid = (expr); \
   EXPECT_EQ(expected, is_valid) << #expr; \
 }

// Basic smoke tests for parameter validity.
// See below for more exhaustive tests.
TEST(JniSafeRegisterNativeMethods, ParameterTypes) {
  using namespace nativehelper::detail;  // NOLINT
  EXPECT_TRUE(IsJniParameterCountValid(kCriticalNative, 0u));
  EXPECT_TRUE(IsJniParameterCountValid(kCriticalNative, 1u));
  EXPECT_TRUE(IsJniParameterCountValid(kCriticalNative, 2u));
  EXPECT_TRUE(IsJniParameterCountValid(kCriticalNative, 3u));
  EXPECT_TRUE(IsJniParameterCountValid(kCriticalNative, 4u));

  EXPECT_FALSE(IsJniParameterCountValid(kNormalNative, 0u));
  EXPECT_FALSE(IsJniParameterCountValid(kNormalNative, 1u));
  EXPECT_TRUE(IsJniParameterCountValid(kNormalNative, 2u));
  EXPECT_TRUE(IsJniParameterCountValid(kNormalNative, 3u));
  EXPECT_TRUE(IsJniParameterCountValid(kNormalNative, 4u));

  EXPECT_TRUE((IsValidJniParameter<void>(kNormalNative, kReturnPosition)));
  EXPECT_IS_VALID_JNI_ARGUMENT_TYPE(true,(is_valid_jni_argument_type<kNormalNative, /*pos*/0u, JNIEnv*>::value));
  EXPECT_IS_VALID_JNI_ARGUMENT_TYPE(true,(is_valid_jni_argument_type<kNormalNative, /*pos*/1u, jobject>::value));
  EXPECT_IS_VALID_JNI_ARGUMENT_TYPE(true,(is_valid_jni_argument_type<kNormalNative, /*pos*/1u, jclass>::value));
  EXPECT_IS_VALID_JNI_ARGUMENT_TYPE(false,(is_valid_jni_argument_type<kNormalNative, /*pos*/1u, jstring>::value));
}

struct TestReturnAnything {
  template <typename T>
  operator T() const {  // NOLINT
    return T{};
  }
};

namespace test_jni {
  void empty_fn() {}
}
struct TestJni {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

  // Always bad.
  static void bad_cptr(const char* ptr) {}
  static void* bad_ret_ptr() { return nullptr; }
  static JNIEnv* bad_ret_env() { return nullptr; }
  static void bad_wrongplace_env(jobject, JNIEnv*) {}
  static void bad_wrongplace_env2(jobject, jobject, JNIEnv*) {}
  static void v_e(JNIEnv*) {}
  static void v_ei(JNIEnv*, jint l) {}
  static void v_el(JNIEnv*, jlong l) {}
  static void v_et(JNIEnv*, jstring) {}
  static jobject o_none() { return nullptr; }
  static void bad_noref_jint_norm(JNIEnv*, jclass, jint&) {}
  static void bad_noref_jint_crit(jint&) {}

  // Good depending on the context:

  // CriticalNative
  static void empty_fn() {}
  static jint int_fn() { return 0; }

  static void v_() {}
  // Note: volatile,const don't participate in the function signature
  // but we still have these here to clarify that it is indeed allowed.
  static void v_vol_i(volatile jint) {}
  static void v_const_i(const jint) {}
  static void v_i(jint) {}
  static void v_l(jlong) {}
  static void v_lib(jlong, jint, jboolean) {}
  static jshort s_lib(jlong, jint, jboolean) { return 0; }

  // Normal or FastNative.
  static void v_eo(JNIEnv*, jobject) {}
  static void v_eoo(JNIEnv*, jobject, jobject) {}
  static void v_ek(JNIEnv*, jclass) {}
  static void v_eolib(JNIEnv*, jobject, jlong, jint, jboolean) {}
  static jshort s_eolAibA(JNIEnv*, jobject, jlongArray, jint, jbooleanArray) { return 0; }

#define DEC_TEST_FN_IMPL(name, ret_t, ...) \
  static ret_t name (__VA_ARGS__) { return TestReturnAnything{}; }

#define DEC_TEST_FN(name, correct, ret_t, ...) \
  DEC_TEST_FN_IMPL(normal_ ## name, ret_t, JNIEnv*, jobject, __VA_ARGS__) \
  DEC_TEST_FN_IMPL(normal2_ ## name, ret_t, JNIEnv*, jclass, __VA_ARGS__) \
  DEC_TEST_FN_IMPL(critical_ ## name, ret_t, __VA_ARGS__)

#define DEC_TEST_FN0(name, correct, ret_t) \
  DEC_TEST_FN_IMPL(normal_ ## name, ret_t, JNIEnv*, jobject) \
  DEC_TEST_FN_IMPL(normal2_ ## name, ret_t, JNIEnv*, jclass) \
  DEC_TEST_FN_IMPL(critical_ ## name, ret_t)

#define JNI_TEST_FN(FN, FN0) \
  FN0(a0,CRITICAL,void) \
  FN0(a ,CRITICAL,jboolean) \
  FN0(a1,CRITICAL,jbyte) \
  FN0(g, CRITICAL,jchar) \
  FN0(c, CRITICAL,jshort) \
  FN0(b, CRITICAL,jint) \
  FN0(f, CRITICAL,jlong) \
  FN0(d, CRITICAL,jfloat) \
  FN0(e, CRITICAL,jdouble) \
  FN0(f2,NORMAL  ,jobject) \
  FN0(f3,NORMAL  ,jclass) \
  FN0(fr,NORMAL  ,jstring) \
  FN0(fa,NORMAL  ,jarray) \
  FN0(fb,NORMAL  ,jobjectArray) \
  FN0(fc,NORMAL  ,jbooleanArray) \
  FN0(fd,NORMAL  ,jcharArray) \
  FN0(fe,NORMAL  ,jshortArray) \
  FN0(ff,NORMAL  ,jintArray) \
  FN0(fg,NORMAL  ,jlongArray) \
  FN0(fk,NORMAL  ,jfloatArray) \
  FN0(fi,NORMAL  ,jdoubleArray) \
  FN0(fl,NORMAL  ,jthrowable) \
  FN(aa, CRITICAL,jboolean,jboolean) \
  FN(ax, CRITICAL,jbyte,jbyte) \
  FN(ag, CRITICAL,jchar,jchar) \
  FN(ac, CRITICAL,jshort,jshort) \
  FN(ac2,CRITICAL,jshort,jshort,jchar) \
  FN(ab, CRITICAL,jint,jint) \
  FN(af, CRITICAL,jlong,jlong) \
  FN(ad, CRITICAL,jfloat,jfloat) \
  FN(ae, CRITICAL,jdouble,jdouble) \
  FN(af2,NORMAL  ,jobject,jobject) \
  FN(af3,NORMAL  ,jclass,jclass) \
  FN(afr,NORMAL  ,jstring,jstring) \
  FN(afa,NORMAL  ,jarray,jarray) \
  FN(afb,NORMAL  ,jobjectArray,jobjectArray) \
  FN(afc,NORMAL  ,jbooleanArray,jbooleanArray) \
  FN(afd,NORMAL  ,jcharArray,jcharArray) \
  FN(afe,NORMAL  ,jshortArray,jshortArray) \
  FN(aff,NORMAL  ,jintArray,jintArray) \
  FN(afg,NORMAL  ,jlongArray,jlongArray) \
  FN(afk,NORMAL  ,jfloatArray,jfloatArray) \
  FN(afi,NORMAL  ,jdoubleArray,jdoubleArray) \
  FN(agi,NORMAL  ,jdoubleArray,jdoubleArray,jobject) \
  FN(afl,NORMAL  ,jthrowable,jthrowable) \
  \
  FN0(z0,ILLEGAL ,JNIEnv*) \
  FN(z1, ILLEGAL ,void, JNIEnv*) \
  FN(z2, ILLEGAL ,JNIEnv*, JNIEnv*) \
  FN(z3, ILLEGAL ,void, void*) \
  FN0(z4,ILLEGAL ,void*) \

#define JNI_TEST_FN_BOTH(x) JNI_TEST_FN(x,x)

// we generate a return statement because some functions are non-void.
// disable the useless warning about returning from a non-void function.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type"
  JNI_TEST_FN(DEC_TEST_FN, DEC_TEST_FN0);
#pragma clang diagnostic pop

  // TODO: probably should be an x-macro table
  // and that way we can add critical/normal to it as well
  // and also the type descriptor, and reuse this for multiple tests.

#pragma clang diagnostic pop
};
// Note: Using function-local structs does not work.
// Template parameters must have linkage, which function-local structs lack.

TEST(JniSafeRegisterNativeMethods, FunctionTypes) {
  using namespace nativehelper::detail;  // NOLINT
  // The exact error messages are not tested but they would be seen in the compiler
  // stack trace when used from a constexpr context.

#define IS_VALID_JNI_FUNCTION_TYPE(native_kind, func) \
    (IsValidJniFunctionType<native_kind, decltype(func), (func)>())
#define IS_VALID_NORMAL_JNI_FUNCTION_TYPE(func) IS_VALID_JNI_FUNCTION_TYPE(kNormalNative, func)
#define IS_VALID_CRITICAL_JNI_FUNCTION_TYPE(func) IS_VALID_JNI_FUNCTION_TYPE(kCriticalNative, func)

#define EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(func)                      \
    do {                                                            \
       EXPECT_FALSE(IS_VALID_CRITICAL_JNI_FUNCTION_TYPE(func));    \
       EXPECT_FALSE(IS_VALID_NORMAL_JNI_FUNCTION_TYPE(func));      \
    } while (false)

#define EXPECT_NORMAL_JNI_FUNCTION_TYPE(func)                       \
    do {                                                            \
       EXPECT_FALSE(IS_VALID_CRITICAL_JNI_FUNCTION_TYPE(func));    \
       EXPECT_TRUE(IS_VALID_NORMAL_JNI_FUNCTION_TYPE(func));       \
    } while (false)

#define EXPECT_CRITICAL_JNI_FUNCTION_TYPE(func)                    \
  do {                                                             \
     EXPECT_TRUE(IS_VALID_CRITICAL_JNI_FUNCTION_TYPE(func));      \
     EXPECT_FALSE(IS_VALID_NORMAL_JNI_FUNCTION_TYPE(func));       \
  } while (false)

  {
    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::bad_cptr);
    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::bad_ret_ptr);
    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::bad_ret_env);
    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::bad_wrongplace_env);
    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::bad_wrongplace_env2);

    EXPECT_CRITICAL_JNI_FUNCTION_TYPE(TestJni::empty_fn);
    EXPECT_CRITICAL_JNI_FUNCTION_TYPE(test_jni::empty_fn);
    EXPECT_CRITICAL_JNI_FUNCTION_TYPE(TestJni::int_fn);

    EXPECT_CRITICAL_JNI_FUNCTION_TYPE(TestJni::v_);
    EXPECT_CRITICAL_JNI_FUNCTION_TYPE(TestJni::v_vol_i);
    EXPECT_CRITICAL_JNI_FUNCTION_TYPE(TestJni::v_const_i);
    EXPECT_CRITICAL_JNI_FUNCTION_TYPE(TestJni::v_i);
    EXPECT_CRITICAL_JNI_FUNCTION_TYPE(TestJni::v_l);

    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::v_e);
    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::v_ei);
    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::v_el);
    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::v_et);

    EXPECT_NORMAL_JNI_FUNCTION_TYPE(TestJni::v_eo);
    EXPECT_NORMAL_JNI_FUNCTION_TYPE(TestJni::v_ek);

    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::o_none);
    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::bad_noref_jint_norm);
    EXPECT_ILLEGAL_JNI_FUNCTION_TYPE(TestJni::bad_noref_jint_crit);
  }

  enum class TestJniKind {
    ILLEGAL,
    NORMAL,
    CRITICAL
  };

  // ILLEGAL signatures are always illegal.
  bool kExpected_ILLEGAL_against_NORMAL = false;
  bool kExpected_ILLEGAL_against_CRITICAL = false;
  // NORMAL signatures are only legal for Normal JNI.
  bool kExpected_NORMAL_against_NORMAL = true;
  bool kExpected_NORMAL_against_CRITICAL = false;
  // CRITICAL signatures are legal for both Normal+Critical JNI.
  bool kExpected_CRITICAL_against_CRITICAL = true;
  bool kExpected_CRITICAL_against_NORMAL = true;
  // Note that we munge normal and critical type signatures separately
  // and that a normal_ prefixed is always a bad critical signature,
  // and a critical_ prefixed signature is always a bad normal signature.
  // See JNI_TEST_FN_MAKE_TEST for the implementation of this logic.

#undef EXPECTED_FOR
#define EXPECTED_FOR(jni_kind, context) \
  (kExpected_ ## jni_kind ## _against_ ## context)

  {
#define JNI_TEST_FN_MAKE_TEST(name, jni_kind, ...) \
     do {                                                            \
       EXPECT_EQ(EXPECTED_FOR(jni_kind, NORMAL),                     \
                 IS_VALID_NORMAL_JNI_FUNCTION_TYPE(TestJni::normal_ ## name));  \
       EXPECT_FALSE(IS_VALID_CRITICAL_JNI_FUNCTION_TYPE(TestJni::normal_ ## name)); \
       EXPECT_EQ(EXPECTED_FOR(jni_kind, NORMAL),                     \
                 IS_VALID_NORMAL_JNI_FUNCTION_TYPE(TestJni::normal2_ ## name)); \
       EXPECT_FALSE(IS_VALID_CRITICAL_JNI_FUNCTION_TYPE(TestJni::normal2_ ## name)); \
       EXPECT_EQ(EXPECTED_FOR(jni_kind, CRITICAL),                   \
                 IS_VALID_CRITICAL_JNI_FUNCTION_TYPE(TestJni::critical_ ## name)); \
       EXPECT_FALSE(IS_VALID_NORMAL_JNI_FUNCTION_TYPE(TestJni::critical_ ## name)); \
    } while (false);

    JNI_TEST_FN_BOTH(JNI_TEST_FN_MAKE_TEST);
  }
}

#define EXPECT_CONSTEXPR_EQ(lhs, rhs) \
 { constexpr auto lhs_val = (lhs);    \
   constexpr auto rhs_val = (rhs);    \
   EXPECT_EQ(lhs_val, rhs_val) << "LHS: " << #lhs << ", RHS: " << #rhs; \
 }

TEST(JniSafeRegisterNativeMethods, FunctionTypeDescriptorConversion) {
  using namespace nativehelper::detail;  // NOLINT
  {
    constexpr auto cvrt = MaybeMakeReifiedJniSignature<kCriticalNative,
                                                       decltype(TestJni::v_i),
                                                       TestJni::v_i>();
    ASSERT_TRUE(cvrt.has_value());
    EXPECT_CONSTEXPR_EQ(2u, cvrt->max_size);
    EXPECT_CONSTEXPR_EQ(1u, cvrt->args.size());
    EXPECT_STRINGIFY_EQ("args={jint}, ret=void", cvrt.value());
  }

  {
    constexpr auto cvrt = MaybeMakeReifiedJniSignature<kNormalNative,
                                                       decltype(TestJni::v_i),
                                                       TestJni::v_i>();
    EXPECT_FALSE(cvrt.has_value());
  }

  {
    constexpr auto cvrt = MaybeMakeReifiedJniSignature<kNormalNative,
                                                       decltype(TestJni::normal_agi),
                                                       TestJni::normal_agi>();
    ASSERT_TRUE(cvrt.has_value());
    EXPECT_EQ(2u, cvrt->args.size());
    EXPECT_STRINGIFY_EQ("args={jdoubleArray,jobject}, ret=jdoubleArray", cvrt.value());
  }

  {
    constexpr auto cvrt = MaybeMakeReifiedJniSignature<kCriticalNative,
                                                       decltype(TestJni::critical_ac2),
                                                       TestJni::critical_ac2>();
    ASSERT_TRUE(cvrt.has_value());
    EXPECT_EQ(2u, cvrt->args.size());
    EXPECT_STRINGIFY_EQ("args={jshort,jchar}, ret=jshort", cvrt.value());
  }

  // TODO: use JNI_TEST_FN to generate these tests automatically.
}

struct test_function_traits {
  static int int_returning_function() { return 0; }
};

template <typename T>
struct apply_return_type {
  constexpr int operator()() const {
    return sizeof(T) == sizeof(int);
  }
};

#define FN_ARGS_PAIR(fn) decltype(fn), (fn)

TEST(JniSafeRegisterNativeMethods, FunctionTraits) {
  using namespace nativehelper::detail;  // NOLINT
  using traits_for_int_ret =
      FunctionTypeMetafunction<FN_ARGS_PAIR(test_function_traits::int_returning_function)>;
  int applied = traits_for_int_ret::map_return<apply_return_type>();
  EXPECT_EQ(1, applied);

  auto arr = traits_for_int_ret::map_args<apply_return_type>();
  EXPECT_EQ(0u, arr.size());
}

struct IntHolder {
  int value;
};

constexpr int GetTestValue(const IntHolder& i) {
  return i.value;
}

constexpr int GetTestValue(int i) {
  return i;
}

template <typename T, size_t kMaxSize>
constexpr size_t SumUpVector(const nativehelper::detail::ConstexprVector<T, kMaxSize>& vec) {
  size_t s = 0;
  for (const T& elem : vec) {
    s += static_cast<size_t>(GetTestValue(elem));
  }
  return s;
}

template <typename T>
constexpr auto make_test_int_vector() {
  using namespace nativehelper::detail;  // NOLINT
  ConstexprVector<T, 5> vec_int;
  vec_int.push_back(T{1});
  vec_int.push_back(T{2});
  vec_int.push_back(T{3});
  vec_int.push_back(T{4});
  vec_int.push_back(T{5});
  return vec_int;
}

TEST(JniSafeRegisterNativeMethods, ConstexprOptional) {
  using namespace nativehelper::detail;  // NOLINT

  ConstexprOptional<int> int_opt;
  EXPECT_FALSE(int_opt.has_value());

  int_opt = ConstexprOptional<int>(12345);
  EXPECT_EQ(12345, int_opt.value());
  EXPECT_EQ(12345, *int_opt);
}

TEST(JniSafeRegisterNativeMethods, ConstexprVector) {
  using namespace nativehelper::detail;  // NOLINT
  {
    constexpr ConstexprVector<IntHolder, 5> vec_int = make_test_int_vector<IntHolder>();
    constexpr size_t the_sum = SumUpVector(vec_int);
    EXPECT_EQ(15u, the_sum);
  }

  {
    constexpr ConstexprVector<int, 5> vec_int = make_test_int_vector<int>();
    constexpr size_t the_sum = SumUpVector(vec_int);
    EXPECT_EQ(15u, the_sum);
  }
}

// Need this intermediate function to make a JniDescriptorNode from a string literal.
// C++ doesn't do implicit conversion through two+ type constructors.
constexpr nativehelper::detail::JniDescriptorNode MakeNode(
    nativehelper::detail::ConstexprStringView str) {
  return nativehelper::detail::JniDescriptorNode{str};
}

#define EXPECT_EQUALISH_JNI_DESCRIPTORS_IMPL(user_desc, derived, cond)                      \
  do {                                                                                      \
    constexpr bool res =                                                                    \
        CompareJniDescriptorNodeErased(MakeNode(user_desc),                                 \
                                       ReifiedJniTypeTrait::Reify<derived>());              \
    (void)res;                                                                              \
    EXPECT_ ## cond(CompareJniDescriptorNodeErased(MakeNode(user_desc),                     \
                                                   ReifiedJniTypeTrait::Reify<derived>())); \
  } while (0);

#define EXPECT_EQUALISH_JNI_DESCRIPTORS(user_desc, derived_desc) \
  EXPECT_EQUALISH_JNI_DESCRIPTORS_IMPL(user_desc, derived_desc, TRUE)

#define EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS(user_desc, derived_desc) \
  EXPECT_EQUALISH_JNI_DESCRIPTORS_IMPL(user_desc, derived_desc, FALSE)

TEST(JniSafeRegisterNativeMethods, CompareJniDescriptorNodeErased) {
  using namespace nativehelper::detail;  // NOLINT
  EXPECT_EQUALISH_JNI_DESCRIPTORS("V", void);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("V", jint);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("Z", jboolean);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("Z", void);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("Z", jobject);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("J", jlong);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("J", jobject);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("J", jthrowable);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("J", jint);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/String;", jstring);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Class;", jclass);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Object;", jobject);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Integer;", jobject);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("[Z", jthrowable);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("[Z", jobjectArray);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Integer;", jintArray);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Integer;", jarray);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Integer;", jarray);

  // Stricter checks.
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Object;", jobjectArray);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/String;", jobject);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Class;", jobject);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("[Z", jobject);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("[Ljava/lang/Object;", jobject);
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Object;", jarray);

  // Permissive checks that are weaker than normal.
  EXPECT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Exception;", jobject);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Error;", jobject);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("[Z", jarray);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("[I", jarray);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("[[Z", jarray);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("[[Ljava/lang/Object;", jarray);

  // jthrowable-related checks.
  EXPECT_NOT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Throwable;", jobject);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Throwable;", jthrowable);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Exception;", jthrowable);
  EXPECT_EQUALISH_JNI_DESCRIPTORS("Ljava/lang/Error;", jthrowable);
}

#define EXPECT_SIMILAR_TYPE_DESCRIPTOR_MATCH(type_desc, type)                             \
  do {                                                                                    \
    constexpr auto res = ReifiedJniTypeTrait::MostSimilarTypeDescriptor(type_desc);       \
    EXPECT_TRUE((ReifiedJniTypeTrait::MostSimilarTypeDescriptor(type_desc)).has_value()); \
    if (res.has_value()) EXPECT_EQ(ReifiedJniTypeTrait::Reify<type>(), res.value());      \
  } while (false)

#define EXPECT_SIMILAR_TYPE_DESCRIPTOR_NO_MATCH(type_desc)                \
  do {                                                                    \
    auto res = ReifiedJniTypeTrait::MostSimilarTypeDescriptor(type_desc); \
    EXPECT_FALSE(res.has_value());                                        \
  } while (false)

#define JNI_TYPE_TRAIT_MUST_BE_SAME_FN(type_name, type_desc, ...)              \
  /* skip jarray because it aliases Ljava/lang/Object; */                      \
  do {                                                                         \
    constexpr auto str_type_name = ConstexprStringView(#type_name);            \
    if (str_type_name != "jarray" && str_type_name != "JNIEnv*") {             \
      EXPECT_SIMILAR_TYPE_DESCRIPTOR_MATCH(type_desc, type_name);              \
    }                                                                          \
  } while(false);

TEST(JniSafeRegisterNativeMethods, MostSimilarTypeDescriptor) {
  using namespace nativehelper::detail;  // NOLINT
  EXPECT_SIMILAR_TYPE_DESCRIPTOR_MATCH("Z", jboolean);
  EXPECT_SIMILAR_TYPE_DESCRIPTOR_MATCH("[[I", jobjectArray);
  EXPECT_SIMILAR_TYPE_DESCRIPTOR_MATCH("[[Z", jobjectArray);
  EXPECT_SIMILAR_TYPE_DESCRIPTOR_MATCH("[Ljava/lang/String;", jobjectArray);
  EXPECT_SIMILAR_TYPE_DESCRIPTOR_MATCH("[Ljava/lang/Integer;", jobjectArray);
  EXPECT_SIMILAR_TYPE_DESCRIPTOR_NO_MATCH("illegal");
  EXPECT_SIMILAR_TYPE_DESCRIPTOR_NO_MATCH("?");
  EXPECT_SIMILAR_TYPE_DESCRIPTOR_NO_MATCH("");

  DEFINE_JNI_TYPE_TRAIT(JNI_TYPE_TRAIT_MUST_BE_SAME_FN);
}

#define ENFORCE_CONSTEXPR(expr) \
  static_assert(__builtin_constant_p(expr), "Expression must be constexpr")

#define EXPECT_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION_IMPL(cond, native_kind, func, desc) \
  do {                                                                        \
    ENFORCE_CONSTEXPR((MatchJniDescriptorWithFunctionType<                    \
        native_kind,                                                          \
        decltype(func),                                                       \
        func,                                                                 \
        sizeof(desc)>(desc)));                                                \
    EXPECT_ ## cond((MatchJniDescriptorWithFunctionType<                      \
        native_kind,                                                          \
        decltype(func),                                                       \
        func,                                                                 \
        sizeof(desc)>(desc)));                                                \
  } while(0)

#define EXPECT_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(native_kind, func, desc) \
    EXPECT_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION_IMPL(TRUE, native_kind, func, desc)

#define EXPECT_NO_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(native_kind, func, desc) \
    EXPECT_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION_IMPL(FALSE, native_kind, func, desc)

TEST(JniSafeRegisterNativeMethods, MatchJniDescriptorWithFunctionType) {
  using namespace nativehelper::detail;  // NOLINT
  // Bad C++ signature.
  EXPECT_NO_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kCriticalNative, TestJni::bad_cptr, "()V");
  EXPECT_NO_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kNormalNative, TestJni::bad_cptr, "()V");

  // JNI type descriptor is not legal (by itself).
  EXPECT_NO_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kCriticalNative, TestJni::v_, "BAD");
  EXPECT_NO_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kNormalNative, TestJni::v_eo, "BAD");

  // Number of parameters in signature vs C++ function does not match.
  EXPECT_NO_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kCriticalNative, TestJni::v_i, "()V");
  EXPECT_NO_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kNormalNative, TestJni::v_eoo, "()V");

  // Return types don't match.
  EXPECT_NO_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kCriticalNative, TestJni::v_, "()Z");
  EXPECT_NO_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kFastNative, TestJni::v_eo, "()Z");

  // Argument types don't match.
  EXPECT_NO_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kCriticalNative, TestJni::v_i, "(Z)V");
  EXPECT_NO_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kNormalNative,
                                                  TestJni::v_eoo,
                                                  "(Ljava/lang/Class;)V");

  // OK.
  EXPECT_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kCriticalNative, TestJni::v_i, "(I)V");
  EXPECT_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kNormalNative,
                                               TestJni::v_eoo,
                                               "(Ljava/lang/Object;)V");

  EXPECT_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kCriticalNative, TestJni::v_lib, "(JIZ)V");
  EXPECT_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kNormalNative, TestJni::v_eolib, "(JIZ)V");
  EXPECT_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kCriticalNative, TestJni::s_lib, "(JIZ)S");
  EXPECT_MATCH_JNI_DESCRIPTOR_AGAINST_FUNCTION(kNormalNative, TestJni::s_eolAibA, "([JI[Z)S");
}

TEST(JniSafeRegisterNativeMethods, Infer) {
  using namespace nativehelper::detail;  // NOLINT
  {
    using Infer_v_eolib_t = InferJniDescriptor<kNormalNative,
                                               decltype(TestJni::v_eolib),
                                               TestJni::v_eolib>;
    EXPECT_CONSTEXPR_EQ(6u, Infer_v_eolib_t::kMaxStringSize);
    std::string x = Infer_v_eolib_t::GetStringAtRuntime();
    EXPECT_STRINGIFY_EQ("(JIZ)V", x.c_str());
  }

  {
    using Infer_v_eolib_t = InferJniDescriptor<kNormalNative,
                                               decltype(TestJni::s_eolAibA),
                                               TestJni::s_eolAibA>;
    EXPECT_STRINGIFY_EQ("args={[J,I,[Z}, ret=S", Infer_v_eolib_t::FromFunctionType().value());
    EXPECT_CONSTEXPR_EQ(8u, Infer_v_eolib_t::kMaxStringSize);
    std::string x = Infer_v_eolib_t::GetStringAtRuntime();
    EXPECT_STRINGIFY_EQ("([JI[Z)S", x.c_str());
  }
}

// Test the macro definition only. See other tests above for signature-match testing.
TEST(JniSafeRegisterNativeMethods, MakeCheckedJniNativeMethod) {
  // Ensure the temporary variables don't conflict with other local vars of same name.
  JNINativeMethod tmp_native_method;  // shadow test.
  (void) tmp_native_method;
  bool is_signature_valid = true;  // shadow test.
  (void) is_signature_valid;

  // Ensure it works with critical.
  {
    JNINativeMethod m =
        MAKE_CHECKED_JNI_NATIVE_METHOD(kCriticalNative,
                                       "v_lib",
                                       "(JIZ)V",
                                       TestJni::v_lib);
    (void)m;
  }

  // Ensure it works with normal.
  {
    JNINativeMethod m =
        MAKE_CHECKED_JNI_NATIVE_METHOD(kNormalNative,
                                       "v_eolib",
                                       "(JIZ)V",
                                       TestJni::v_eolib);
    (void)m;
  }

  // Make sure macros properly expand inside of an array.
  {
    JNINativeMethod m_array[] = {
        MAKE_CHECKED_JNI_NATIVE_METHOD(kCriticalNative,
                                       "v_lib",
                                       "(JIZ)V",
                                       TestJni::v_lib),
        MAKE_CHECKED_JNI_NATIVE_METHOD(kNormalNative,
                                       "v_eolib",
                                       "(JIZ)V",
                                       TestJni::v_eolib),
    };
    (void)m_array;
  }
  {
    JNINativeMethod m_array_direct[] {
        MAKE_CHECKED_JNI_NATIVE_METHOD(kCriticalNative,
                                       "v_lib",
                                       "(JIZ)V",
                                       TestJni::v_lib),
        MAKE_CHECKED_JNI_NATIVE_METHOD(kNormalNative,
                                       "v_eolib",
                                       "(JIZ)V",
                                       TestJni::v_eolib),
    };
    (void)m_array_direct;
  }

}

static auto sTestCheckedAtFileScope =
        MAKE_CHECKED_JNI_NATIVE_METHOD(kCriticalNative,
                                       "v_lib",
                                       "(JIZ)V",
                                       TestJni::v_lib);

static auto sTestInferredAtFileScope =
        MAKE_INFERRED_JNI_NATIVE_METHOD(kCriticalNative,
                                       "v_lib",
                                       TestJni::v_lib);

TEST(JniSafeRegisterNativeMethods, TestInferredJniNativeMethod) {
  (void) sTestCheckedAtFileScope;
  (void) sTestInferredAtFileScope;

  // Ensure it works with critical.
  {
    JNINativeMethod m =
        MAKE_INFERRED_JNI_NATIVE_METHOD(kCriticalNative,
                                       "v_lib",
                                       TestJni::v_lib);
    (void)m;
  }

  // Ensure it works with normal.
  {
    JNINativeMethod m =
        MAKE_INFERRED_JNI_NATIVE_METHOD(kNormalNative,
                                       "v_eolib",
                                       TestJni::v_eolib);
    (void)m;
  }
}

static void TestJniMacros_v_lib(jlong, jint, jboolean) {}
static void TestJniMacros_v_lib_od(jlong, jint, jboolean) {}
static void TestJniMacros_v_eolib(JNIEnv*, jobject, jlong, jint, jboolean) {}
static void TestJniMacros_v_eolib_od(JNIEnv*, jobject, jlong, jint, jboolean) {}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

static jint android_os_Parcel_dataSize(jlong) { return 0; }
static jint android_os_Parcel_dataAvail(jlong) { return 0; }
static jint android_os_Parcel_dataPosition(jlong) { return 0; }
static jint android_os_Parcel_dataCapacity(jlong) { return 0; }
static jlong android_os_Parcel_setDataSize(JNIEnv*, jclass, jlong, jint) { return 0; }
static void android_os_Parcel_setDataPosition(jlong, jint) {}
static void android_os_Parcel_setDataCapacity(JNIEnv*, jclass, jlong, jint) {}
static jboolean android_os_Parcel_pushAllowFds(jlong, jboolean) { return true; }
static void android_os_Parcel_restoreAllowFds(jlong, jboolean) {}
static void android_os_Parcel_writeByteArray(JNIEnv*, jclass, jlong, jbyteArray, jint, jint) {}

static void android_os_Parcel_writeBlob(JNIEnv*, jclass, jlong, jbyteArray, jint, jint) {}
static void android_os_Parcel_writeInt(JNIEnv*, jclass, jlong, jint) {}
static void android_os_Parcel_writeLong(JNIEnv* env,
                                        jclass clazz,
                                        jlong nativePtr,
                                        jlong val) {}
static void android_os_Parcel_writeFloat(JNIEnv* env,
                                         jclass clazz,
                                         jlong nativePtr,
                                         jfloat val) {}
static void android_os_Parcel_writeDouble(JNIEnv* env,
                                          jclass clazz,
                                          jlong nativePtr,
                                          jdouble val) {}
static void android_os_Parcel_writeString(JNIEnv* env,
                                          jclass clazz,
                                          jlong nativePtr,
                                          jstring val) {}
static void android_os_Parcel_writeStrongBinder(JNIEnv* env,
                                                jclass clazz,
                                                jlong nativePtr,
                                                jobject object) {}
static jlong android_os_Parcel_writeFileDescriptor(JNIEnv* env,
                                                   jclass clazz,
                                                   jlong nativePtr,
                                                   jobject object) { return 0; }
static jbyteArray android_os_Parcel_createByteArray(JNIEnv* env,
                                                    jclass clazz,
                                                    jlong nativePtr) { return nullptr; }

static jboolean android_os_Parcel_readByteArray(JNIEnv* env,
                                                jclass clazz,
                                                jlong nativePtr,
                                                jbyteArray dest,
                                                jint destLen) { return false; }
static jbyteArray android_os_Parcel_readBlob(JNIEnv* env,
                                             jclass clazz,
                                             jlong nativePtr) { return nullptr; }

static jint android_os_Parcel_readInt(jlong nativePtr) { return 0; }

static jlong android_os_Parcel_readLong(jlong nativePtr) { return 0; }

static jfloat android_os_Parcel_readFloat(jlong nativePtr) { return 0.0f; }
static jdouble android_os_Parcel_readDouble(jlong nativePtr) { return 0.0; }

static jstring android_os_Parcel_readString(JNIEnv* env,
                                            jclass clazz,
                                            jlong nativePtr) { return nullptr; }

static jobject android_os_Parcel_readStrongBinder(JNIEnv* env,
                                                  jclass clazz,
                                                  jlong nativePtr) { return nullptr; }


static jobject android_os_Parcel_readFileDescriptor(JNIEnv* env,
                                                    jclass clazz,
                                                    jlong nativePtr) { return nullptr; }

static jobject android_os_Parcel_openFileDescriptor(JNIEnv* env,
                                                    jclass clazz,
                                                    jstring name,
                                                    jint mode) { return 0; }


static jobject android_os_Parcel_dupFileDescriptor(JNIEnv* env,
                                                   jclass clazz,
                                                   jobject orig) { return 0; }


static void android_os_Parcel_closeFileDescriptor(JNIEnv* env,
                                                  jclass clazz,
                                                  jobject object) {}


static void android_os_Parcel_clearFileDescriptor(JNIEnv* env,
                                                  jclass clazz,
                                                  jobject object) {}


static jlong android_os_Parcel_create(JNIEnv* env, jclass clazz) { return 0; }


static jlong android_os_Parcel_freeBuffer(JNIEnv* env,
                                          jclass clazz,
                                          jlong nativePtr) { return 0; }


static void  android_os_Parcel_destroy(JNIEnv* env, jclass clazz, jlong nativePtr) {}


static jbyteArray android_os_Parcel_marshall(JNIEnv* env,
                                             jclass clazz,
                                             jlong nativePtr) { return 0; }


static jlong android_os_Parcel_unmarshall(JNIEnv* env,
                                          jclass clazz,
                                          jlong nativePtr,
                                          jbyteArray data,
                                          jint offset,
                                          jint length) { return 0; }


static jint android_os_Parcel_compareData(JNIEnv* env,
                                          jclass clazz,
                                          jlong thisNativePtr,
                                          jlong otherNativePtr) { return 0; }


static jlong android_os_Parcel_appendFrom(JNIEnv* env,
                                          jclass clazz,
                                          jlong thisNativePtr,
                                          jlong otherNativePtr,
                                          jint offset,
                                          jint length) { return 0; }


static jboolean android_os_Parcel_hasFileDescriptors(jlong nativePtr) { return 0; }


static void android_os_Parcel_writeInterfaceToken(JNIEnv* env,
                                                  jclass clazz,
                                                  jlong nativePtr,
                                                  jstring name) {}


static void android_os_Parcel_enforceInterface(JNIEnv* env,
                                               jclass clazz,
                                               jlong nativePtr,
                                               jstring name) {}


static jlong android_os_Parcel_getGlobalAllocSize(JNIEnv* env, jclass clazz) { return 0; }


static jlong android_os_Parcel_getGlobalAllocCount(JNIEnv* env, jclass clazz) { return 0; }


static jlong android_os_Parcel_getBlobAshmemSize(jlong nativePtr) { return 0; }

#pragma clang diagnostic pop

TEST(JniSafeRegisterNativeMethods, ParcelExample) {
  // Test a wide range of automatic signature inferencing.
  // This is taken from real code in android_os_Parcel.cpp.

  const JNINativeMethod gParcelMethods[] = {
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeDataSize", android_os_Parcel_dataSize),
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeDataAvail", android_os_Parcel_dataAvail),
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeDataPosition", android_os_Parcel_dataPosition),
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeDataCapacity", android_os_Parcel_dataCapacity),
    // @FastNative
    MAKE_JNI_FAST_NATIVE_METHOD_AUTOSIG(
        "nativeSetDataSize",  android_os_Parcel_setDataSize),
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeSetDataPosition", android_os_Parcel_setDataPosition),
    // @FastNative
    MAKE_JNI_FAST_NATIVE_METHOD_AUTOSIG(
        "nativeSetDataCapacity", android_os_Parcel_setDataCapacity),
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativePushAllowFds", android_os_Parcel_pushAllowFds),
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeRestoreAllowFds", android_os_Parcel_restoreAllowFds),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeWriteByteArray", android_os_Parcel_writeByteArray),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeWriteBlob", android_os_Parcel_writeBlob),
    // @FastNative
    MAKE_JNI_FAST_NATIVE_METHOD_AUTOSIG(
        "nativeWriteInt", android_os_Parcel_writeInt),
    // @FastNative
    MAKE_JNI_FAST_NATIVE_METHOD_AUTOSIG(
        "nativeWriteLong", android_os_Parcel_writeLong),
    // @FastNative
    MAKE_JNI_FAST_NATIVE_METHOD_AUTOSIG(
        "nativeWriteFloat", android_os_Parcel_writeFloat),
    // @FastNative
    MAKE_JNI_FAST_NATIVE_METHOD_AUTOSIG(
        "nativeWriteDouble", android_os_Parcel_writeDouble),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeWriteString",  android_os_Parcel_writeString),
    MAKE_JNI_NATIVE_METHOD(
        "nativeWriteStrongBinder", "(JLandroid/os/IBinder;)V", android_os_Parcel_writeStrongBinder),
    MAKE_JNI_NATIVE_METHOD(
        "nativeWriteFileDescriptor", "(JLjava/io/FileDescriptor;)J", android_os_Parcel_writeFileDescriptor),

    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeCreateByteArray", android_os_Parcel_createByteArray),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeReadByteArray", android_os_Parcel_readByteArray),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeReadBlob", android_os_Parcel_readBlob),
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeReadInt", android_os_Parcel_readInt),
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeReadLong", android_os_Parcel_readLong),
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeReadFloat", android_os_Parcel_readFloat),
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeReadDouble", android_os_Parcel_readDouble),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeReadString", android_os_Parcel_readString),
    MAKE_JNI_NATIVE_METHOD(
        "nativeReadStrongBinder", "(J)Landroid/os/IBinder;", android_os_Parcel_readStrongBinder),
    MAKE_JNI_NATIVE_METHOD(
        "nativeReadFileDescriptor", "(J)Ljava/io/FileDescriptor;", android_os_Parcel_readFileDescriptor),
    MAKE_JNI_NATIVE_METHOD(
        "openFileDescriptor", "(Ljava/lang/String;I)Ljava/io/FileDescriptor;", android_os_Parcel_openFileDescriptor),
    MAKE_JNI_NATIVE_METHOD(
        "dupFileDescriptor", "(Ljava/io/FileDescriptor;)Ljava/io/FileDescriptor;", android_os_Parcel_dupFileDescriptor),
    MAKE_JNI_NATIVE_METHOD(
        "closeFileDescriptor", "(Ljava/io/FileDescriptor;)V", android_os_Parcel_closeFileDescriptor),
    MAKE_JNI_NATIVE_METHOD(
        "clearFileDescriptor", "(Ljava/io/FileDescriptor;)V", android_os_Parcel_clearFileDescriptor),

    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeCreate", android_os_Parcel_create),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeFreeBuffer", android_os_Parcel_freeBuffer),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeDestroy", android_os_Parcel_destroy),

    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeMarshall", android_os_Parcel_marshall),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeUnmarshall", android_os_Parcel_unmarshall),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeCompareData", android_os_Parcel_compareData),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeAppendFrom", android_os_Parcel_appendFrom),
    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeHasFileDescriptors", android_os_Parcel_hasFileDescriptors),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeWriteInterfaceToken", android_os_Parcel_writeInterfaceToken),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "nativeEnforceInterface", android_os_Parcel_enforceInterface),

    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "getGlobalAllocSize", android_os_Parcel_getGlobalAllocSize),
    MAKE_JNI_NATIVE_METHOD_AUTOSIG(
        "getGlobalAllocCount", android_os_Parcel_getGlobalAllocCount),

    // @CriticalNative
    MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG(
        "nativeGetBlobAshmemSize", android_os_Parcel_getBlobAshmemSize),
  };

  const JNINativeMethod gParcelMethodsExpected[] = {
    // @CriticalNative
    {"nativeDataSize",            "(J)I", (void*)android_os_Parcel_dataSize},
    // @CriticalNative
    {"nativeDataAvail",           "(J)I", (void*)android_os_Parcel_dataAvail},
    // @CriticalNative
    {"nativeDataPosition",        "(J)I", (void*)android_os_Parcel_dataPosition},
    // @CriticalNative
    {"nativeDataCapacity",        "(J)I", (void*)android_os_Parcel_dataCapacity},
    // @FastNative
    {"nativeSetDataSize",         "(JI)J", (void*)android_os_Parcel_setDataSize},
    // @CriticalNative
    {"nativeSetDataPosition",     "(JI)V", (void*)android_os_Parcel_setDataPosition},
    // @FastNative
    {"nativeSetDataCapacity",     "(JI)V", (void*)android_os_Parcel_setDataCapacity},

    // @CriticalNative
    {"nativePushAllowFds",        "(JZ)Z", (void*)android_os_Parcel_pushAllowFds},
    // @CriticalNative
    {"nativeRestoreAllowFds",     "(JZ)V", (void*)android_os_Parcel_restoreAllowFds},

    {"nativeWriteByteArray",      "(J[BII)V", (void*)android_os_Parcel_writeByteArray},
    {"nativeWriteBlob",           "(J[BII)V", (void*)android_os_Parcel_writeBlob},
    // @FastNative
    {"nativeWriteInt",            "(JI)V", (void*)android_os_Parcel_writeInt},
    // @FastNative
    {"nativeWriteLong",           "(JJ)V", (void*)android_os_Parcel_writeLong},
    // @FastNative
    {"nativeWriteFloat",          "(JF)V", (void*)android_os_Parcel_writeFloat},
    // @FastNative
    {"nativeWriteDouble",         "(JD)V", (void*)android_os_Parcel_writeDouble},
    {"nativeWriteString",         "(JLjava/lang/String;)V", (void*)android_os_Parcel_writeString},
    {"nativeWriteStrongBinder",   "(JLandroid/os/IBinder;)V", (void*)android_os_Parcel_writeStrongBinder},
    {"nativeWriteFileDescriptor", "(JLjava/io/FileDescriptor;)J", (void*)android_os_Parcel_writeFileDescriptor},

    {"nativeCreateByteArray",     "(J)[B", (void*)android_os_Parcel_createByteArray},
    {"nativeReadByteArray",       "(J[BI)Z", (void*)android_os_Parcel_readByteArray},
    {"nativeReadBlob",            "(J)[B", (void*)android_os_Parcel_readBlob},
    // @CriticalNative
    {"nativeReadInt",             "(J)I", (void*)android_os_Parcel_readInt},
    // @CriticalNative
    {"nativeReadLong",            "(J)J", (void*)android_os_Parcel_readLong},
    // @CriticalNative
    {"nativeReadFloat",           "(J)F", (void*)android_os_Parcel_readFloat},
    // @CriticalNative
    {"nativeReadDouble",          "(J)D", (void*)android_os_Parcel_readDouble},
    {"nativeReadString",          "(J)Ljava/lang/String;", (void*)android_os_Parcel_readString},
    {"nativeReadStrongBinder",    "(J)Landroid/os/IBinder;", (void*)android_os_Parcel_readStrongBinder},
    {"nativeReadFileDescriptor",  "(J)Ljava/io/FileDescriptor;", (void*)android_os_Parcel_readFileDescriptor},

    {"openFileDescriptor",        "(Ljava/lang/String;I)Ljava/io/FileDescriptor;", (void*)android_os_Parcel_openFileDescriptor},
    {"dupFileDescriptor",         "(Ljava/io/FileDescriptor;)Ljava/io/FileDescriptor;", (void*)android_os_Parcel_dupFileDescriptor},
    {"closeFileDescriptor",       "(Ljava/io/FileDescriptor;)V", (void*)android_os_Parcel_closeFileDescriptor},
    {"clearFileDescriptor",       "(Ljava/io/FileDescriptor;)V", (void*)android_os_Parcel_clearFileDescriptor},

    {"nativeCreate",              "()J", (void*)android_os_Parcel_create},
    {"nativeFreeBuffer",          "(J)J", (void*)android_os_Parcel_freeBuffer},
    {"nativeDestroy",             "(J)V", (void*)android_os_Parcel_destroy},

    {"nativeMarshall",            "(J)[B", (void*)android_os_Parcel_marshall},
    {"nativeUnmarshall",          "(J[BII)J", (void*)android_os_Parcel_unmarshall},
    {"nativeCompareData",         "(JJ)I", (void*)android_os_Parcel_compareData},
    {"nativeAppendFrom",          "(JJII)J", (void*)android_os_Parcel_appendFrom},
    // @CriticalNative
    {"nativeHasFileDescriptors",  "(J)Z", (void*)android_os_Parcel_hasFileDescriptors},
    {"nativeWriteInterfaceToken", "(JLjava/lang/String;)V", (void*)android_os_Parcel_writeInterfaceToken},
    {"nativeEnforceInterface",    "(JLjava/lang/String;)V", (void*)android_os_Parcel_enforceInterface},

    {"getGlobalAllocSize",        "()J", (void*)android_os_Parcel_getGlobalAllocSize},
    {"getGlobalAllocCount",       "()J", (void*)android_os_Parcel_getGlobalAllocCount},

    // @CriticalNative
    {"nativeGetBlobAshmemSize",       "(J)J", (void*)android_os_Parcel_getBlobAshmemSize},
  };

  ASSERT_EQ(sizeof(gParcelMethodsExpected)/sizeof(JNINativeMethod),
            sizeof(gParcelMethods)/sizeof(JNINativeMethod));


  for (size_t i = 0; i < sizeof(gParcelMethods) / sizeof(JNINativeMethod); ++i) {
    const JNINativeMethod& actual = gParcelMethods[i];
    const JNINativeMethod& expected = gParcelMethodsExpected[i];

    EXPECT_STREQ(expected.name, actual.name);
    EXPECT_STREQ(expected.signature, actual.signature) << expected.name;
    EXPECT_EQ(expected.fnPtr, actual.fnPtr) << expected.name;
  }
}

TEST(JniSafeRegisterNativeMethods, JniMacros) {
  JNINativeMethod tmp_native_method;  // shadow variable check.
  (void)tmp_native_method;
  using Infer_t = int;  // shadow using check.
  Infer_t unused;
  (void)unused;

  MAKE_JNI_CRITICAL_NATIVE_METHOD("v_lib", "(JIZ)V", TestJniMacros_v_lib);
  MAKE_JNI_CRITICAL_NATIVE_METHOD_AUTOSIG("v_lib", TestJniMacros_v_lib);
  CRITICAL_NATIVE_METHOD(TestJniMacros, v_lib, "(JIZ)V");
  OVERLOADED_CRITICAL_NATIVE_METHOD(TestJniMacros, v_lib, "(JIZ)V", v_lib_od);
  CRITICAL_NATIVE_METHOD_AUTOSIG(TestJniMacros, v_lib);

  MAKE_JNI_FAST_NATIVE_METHOD("v_eolib", "(JIZ)V", TestJniMacros_v_eolib);
  MAKE_JNI_FAST_NATIVE_METHOD_AUTOSIG("v_eolib", TestJniMacros_v_eolib);
  FAST_NATIVE_METHOD(TestJniMacros, v_eolib, "(JIZ)V");
  OVERLOADED_FAST_NATIVE_METHOD(TestJniMacros, v_eolib, "(JIZ)V", v_eolib_od);
  FAST_NATIVE_METHOD_AUTOSIG(TestJniMacros, v_eolib);

  MAKE_JNI_NATIVE_METHOD("v_eolib", "(JIZ)V", TestJniMacros_v_eolib);
  MAKE_JNI_NATIVE_METHOD_AUTOSIG("v_eolib", TestJniMacros_v_eolib);
  NATIVE_METHOD(TestJniMacros, v_eolib, "(JIZ)V");
  OVERLOADED_NATIVE_METHOD(TestJniMacros, v_eolib, "(JIZ)V", v_eolib_od);
  NATIVE_METHOD_AUTOSIG(TestJniMacros, v_eolib);

  _NATIVEHELPER_JNI_MAKE_METHOD_OLD(kNormalNative, "v_eolib", "(JIZ)V", TestJniMacros_v_eolib);
  tmp_native_method =
      _NATIVEHELPER_JNI_MAKE_METHOD_OLD(kNormalNative, "v_eolib", "(JIZ)V", TestJniMacros_v_eolib);
}
