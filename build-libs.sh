#!/bin/bash

ARGS=$(getopt -o "dca" -l "debug,clean,archive" -n "build-libs.sh" -- "$@");

eval set -- "$ARGS";

cd libserial/src/main

JNI_ARGS="NDK_LIBS_OUT=jniLibs"
BUILD_TASK="assembleRelease"

EXPORT_AAR=0

while true; do
  case "$1" in
    -d|--debug)
      shift
      JNI_ARGS="$JNI_ARGS NDK_DEBUG=1"
      BUILD_TASK="assembleDebug"
      break;
      ;;
    -c|--clean)
      shift
      JNI_ARGS="$JNI_ARGS clean"
      break;
      ;;
    -a|--archive)
      shift
      EXPORT_AAR=1
      break;
      ;;
    --)
      shift
      break;
      ;;
  esac
done

# Remove '--'
shift

ndk-build $JNI_ARGS

cd ../../..

if [ "$EXPORT_AAR" = "1" ]; then
  cd libserial
  ../gradlew $BUILD_TASK
  cd ..
fi
