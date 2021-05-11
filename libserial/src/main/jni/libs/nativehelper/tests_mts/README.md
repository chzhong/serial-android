# libnativehelper MTS tests

These tests cover the API surface of libnativehelper that is applicable once
the runtime is initialized.

These tests do not cover the API surface relating to the binding of the ART
runtime (DalvikVM), that preclude the initialization of the runtime, nor do
they cover JNI_CreateJavaVM(). These APIs have been invoked before the test
harness runs these tests.

API surface not directly covered here are:

```
    JNI_GetCreatedJavaVMs

    JniInvocationCreate
    JniInvocationDestroy
    JniInvocationInit
    JniInvocationGetLibrary

    jniUninitializeConstants
```

`JniInvocationInit()` is responsible for binding the ART runtime and
specifically the following methods:

```
    JNI_CreateJavaVM
    JNI_GetCreatedJavaVMs
    JNI_GetDefaultJavaVMInitArgs
```

These tests do check that `JNI_GetCreatedJavaVMs()` and
`JNI_GetDefaultJavaVMInitArgs()` behave as expected and are thus asserted to
be correctly bound. `JNI_CreateJavaVM()` cannot be called in these tests
because Android only supports a single runtime per process.

`JniInvocationInit()` uses `JniInvocationGetLibrary()` to determine which
runtime to load (release, debug, or custom). The code responsible for that
decision is tested comprehensively in `libnativehelper_gtests`.

`jniUninitializeConstants` is only intended to be called when the runtime is
shutting down and unloading the managed core libraries.

## Potential Issues

The test harness depends on libnativehelper_compat_libc++ and the tests
depend on libnativehelper. The former library is a subset of libnativehelper.
There are potential ODR problems if the two libraries having overlapping
global state. It would be better to have two separate test suites for these
two libraries.
