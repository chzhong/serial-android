# Serial Port communication library for Android (with Java API)

This is a serial port library for Android, with both JNI API and Java API.

The Java API is bridged to JNI implementation, and not all of the API is strictly tested. Use it at your own risk.

This library is based on wjwwood's cross-platform serial library  https://github.com/wjwwood/serial.


### Documentation

* Native API: See original C++ document at http://wjwwood.github.com/serial/
* Java API: See javadoc in source files.

### Dependencies

* NDK r13
* Android Studio 2.2.1

### Install

Get the code:

    git clone https://github.com/chzhong/serial-android.git

Build Native Code:

Android Studio NDK support is still in beta, so please build the so files using ndk-build:

    cd android/library/src/main/jni
    ./build-jni.sh

Any change to native code it build with Android Studio are welcome.

NOTE: There might be some issue while building with different NDK revisions. 

Use:

* Use it directly in your native code.
* Import it as an Android Library project.

### License

The MIT License

Copyright (c) 2012 William Woodall, John Harrison

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

### Authors

William Woodall <wjwwood@gmail.com>
John Harrison <ash.gti@gmail.com>

Cheng Zhong <hust.zcheng@gmail.com>

### Contact

William Woodall <william@osrfoundation.org>

Cheng Zhong <hust.zcheng@gmail.com>
