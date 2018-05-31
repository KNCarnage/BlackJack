rem call ndk-build NDK_MODULE_PATH="../../../../"
call gradlew assembleRelease
call zipalign -v -p 4 build/outputs/apk/release/proj.android-release-unsigned.apk build/outputs/apk/release/proj.android-release-unsigned-aligned.apk
call apksigner sign --ks my-release-key.jks --out build/outputs/apk/release/proj.android-release.apk build/outputs/apk/release/proj.android-release-unsigned-aligned.apk
call adb install -r build/outputs/apk/release/proj.android-release-unsigned
call adb shell am start -n org.oxygine.FunesSol/org.oxygine.FunesSol.MainActivity