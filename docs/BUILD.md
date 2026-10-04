# راهنمای بیلد (BUILD)

<div dir="rtl">

## پیش‌نیازها

| ابزار | نسخه آزمایش‌شده | یادداشت |
|---|---|---|
| JDK | **17** (Temurin) | اجباری برای AGP 8.x |
| Gradle | **8.9** | یا wrapper در CI |
| Android SDK | Platform 34 + Build-Tools 34 | `sdkmanager` |
| Android NDK | r26/r27 (پیش‌فرض AGP) | برای `libgenerals.so` |
| CMake | 3.22.1 | از SDK Manager |
| فضا | ~8GB | SDK+NDK+کش |

## بیلد سریع (خط فرمان)

```bash
git clone https://github.com/obsifox/GeneralsAndroid-Source.git
cd GeneralsAndroid-Source/source

# debug APK
gradle :app:assembleDebug
# خروجی: app/build/outputs/apk/debug/app-debug.apk

# release (امضاشده فقط اگر متغیرهای محیطی کی‌استور ست باشند)
export OBSI_KEYSTORE_PATH=/path/keystore.jks
export OBSI_KEYSTORE_PASS=****  ; export OBSI_KEY_ALIAS=generals ; export OBSI_KEY_PASS=****
gradle :app:assembleRelease
```

نکته: **سورس و بیلد جدا هستند** — خروجی‌ها هرگز به گیت کامیت نمی‌شوند؛ ریلیزها فقط در مخزن [GeneralsAndroid](https://github.com/obsifox/GeneralsAndroid) منتشر می‌شوند.

## بیلد با Android Studio

1. `File → Open` → پوشه `source/`
2. صبر برای sync (اولین بار NDK را دانلود می‌کند)
3. `Build → Build Bundle(s)/APK(s) → Build APK(s)`

## CI (خودکار)

هر push به `main`:
1. JDK 17 + Gradle 8.9 نصب می‌شود
2. `assembleDebug` + `assembleRelease` (ARM64-v8a)
3. APKها به‌عنوان artifact ذخیره می‌شوند
4. روی تگ `v*` → **GitHub Release** با SHA256 در همین مخزن + اطلاع در مخزن دانلود

## بیلد بومی (فقط فاز پلتفرم)

```bash
cd source/app/src/main/cpp
cmake -B ../../../../../build/native \
      -DANDROID_ABI=arm64-v8a \
      -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_PLATFORM=android-24
cmake --build ../../../../../build/native
```

## فچ کردن سورس موتور (برای فازهای بعدی)

```bash
./engine/fetch_upstream.sh          # پین‌شده از engine/UPSTREAM.md
```

</div>
