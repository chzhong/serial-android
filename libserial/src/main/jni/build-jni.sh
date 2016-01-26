#!/bin/bash

ARGS=$(getopt -o "dc" -l "debug,clean" -n "build-jni.sh" -- "$@");

eval set -- "$ARGS";

JNI_ARGS="NDK_LIBS_OUT=../jniLibs"

while true; do
  case "$1" in
    -d|--debug)
      shift
      JNI_ARGS="$JNI_ARGS NDK_DEBUG=1"
      break;
      ;;
    -c|--clean)
      shift
      JNI_ARGS="$JNI_ARGS clean"
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
