#!/bin/zsh

function build_for_platform(){
    _PLATFORM=$1
    _TARGET=$2
    echo "Building for $_PLATFORM"
    ./Configure $_TARGET "-fembed-bitcode" \
        no-shared no-dso no-hw no-engine \
        --prefix=/usr/local/openssl-$_PLATFORM
    make clean
    make
    if [ $? -ne 0 ]; then
        >&2 echo "There was an error building for $_PLATFORM. Code $?" 
        exit 1
    else
        mv libcrypto.a libcrypto_$_PLATFORM.a
        mv libssl.a libssl_$_PLATFORM.a
    fi
}

build_for_platform MacOSX darwin64-x86_64-cc
build_for_platform iPhoneOS ios64-xcrun arm64
build_for_platform iPhoneSimulator iossimulator-xcrun

xcodebuild -create-xcframework \
    -library libssl_MacOSX.a -headers include \
    -library libssl_iPhoneOS.a -headers include \
    -library libssl_iPhoneSimulator.a -headers include \
    -output libssl.xcframework
rm -rf libssl.xcframework/ios-arm64/Headers/crypto
rm -rf libssl.xcframework/macos-x86_64/Headers/crypto
rm -rf libssl.xcframework/ios-x86_64-simulator/Headers/crypto
rm -rf libssl.xcframework/ios-arm64/Headers/internal
rm -rf libssl.xcframework/macos-x86_64/Headers/internal
rm -rf libssl.xcframework/ios-x86_64-simulator/Headers/internal

xcodebuild -create-xcframework \
    -library libcrypto_MacOSX.a -headers include \
    -library libcrypto_iPhoneOS.a -headers include \
    -library libcrypto_iPhoneSimulator.a -headers include \
    -output libcrypto.xcframework

rm -rf libcrypto.xcframework/ios-arm64/Headers/openssl
rm -rf libcrypto.xcframework/macos-x86_64/Headers/openssl
rm -rf libcrypto.xcframework/ios-x86_64-simulator/Headers/openssl
rm -rf libcrypto.xcframework/ios-arm64/Headers/internal
rm -rf libcrypto.xcframework/macos-x86_64/Headers/internal
rm -rf libcrypto.xcframework/ios-x86_64-simulator/Headers/internal