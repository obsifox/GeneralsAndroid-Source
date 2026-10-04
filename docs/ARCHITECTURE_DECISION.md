# تصمیم معماری — انتخاب آپ‌استریم (ARCHITECTURE DECISION)

> طبق بخش ۲ و ۴۵ سند Master Agent، قبل از هر کدی، اکوسیستم موجود بررسی شد.
> تاریخ ممیزی: ۲۰۲۶-۱۰-۰۴ | ممیز: عامل خودکار پورت

## ۱. نامزدهای بررسی‌شده

| مخزن | وضعیت | حجم | آخرین push | ارزیابی |
|---|---|---|---|---|
| **TheSuperHackers/GeneralsGameCode** | فعال | ~39MB (فقط کد) | 2026-10-04 (امروز) | ✅ انتخاب شد |
| electronicarts/CnC_Generals_Zero_Hour | **آرشیو شده** (2025-02-27) | ~9MB | — | مرجع تاریخی، نگهداری نمی‌شود |
| Fighter19/CnC_Generals_Zero_Hour | نیمه‌فعال (فوکوس Linux دسکتاپ) | ~16MB | 2026-08-18 | فورک تزئینی، جامعه کوچک‌تر |
| GeneralsX/GeneralsX | مرده (فقط پروفایل) | ~0MB | — | رد شد |

## ۲. دلیل انتخاب

1. **فعالیت مستمر:** چندین کامیت در هفته، CI کامل، تیم سازمان‌یافته — همان سازمانی که EA انتشار سورس را با آن هماهنگ کرد.
2. **مدرن‌سازی در جریان:** سیستم CMake واحد (به‌جای پروژه‌های VC6 تاریخی)، گزینه FFmpeg برای ویدیو، جداسازی `Core/` از `Generals/` که نشان‌دهنده حرکت به سمت لایه‌بندی است.
3. **عدم وجود assetهای EA در مخزن** — انطباق حقوقی ساده‌تر.
4. GPLv3 + شرایط تکمیلی EA، مشابه بقیه نامزدها.

**نقطه‌ی پین‌شده (pinned):**
`TheSuperHackers/GeneralsGameCode@f8ba7eb44b688db14e2ff24d7172474f70be1587` (2026-10-04)

## ۳. واقعیت فنی مهم (کشف ممیزی)

پیمایش کامل درخت سورس (۴۸۶۵ مسیر) نشان داد:

- ❌ **صفر** فایل SDL / OpenGL / GLES / X11 / Android در مخزن
- لایه دستگاه (`GameEngineDevice`) فقط شامل `Win32Device`، `W3DDevice` (رندرر Direct3D8-محور)، `MilesAudioDevice` (صدای Miles — **مالکیتی**) و `StdDevice` است.
- یعنی: **آپ‌استریم در حال حاضر فقط روی ویندوز اجرا می‌شود.**

## ۴. نتیجه — استراتژی پورت اندروید

موتور به‌صورت تاریخی از رابط‌های مجزای دستگاه استفاده می‌کند؛ پس نقشه راه:

```
ENGINE (GameEngine مشترک)
│
├── DeviceFactory / رابط‌های دستگاه
│     ├── Win32Device  ← آپ‌استریم
│     └── AndroidDevice ← ✍️ کد ما (source/app/src/main/cpp)
│           ├── پنجره/رویداد  : EGL + NativeActivity
│           ├── فایل‌سیستم    : نگاشت به پوشه داده واردشده با SAF
│           ├── ورودی        : touch → mouse/keyboard engine events
│           ├── صدا          : AAudio/OpenSL (جایگزین Miles مالکیتی)
│           └── شبکه         : سوکت BSD (Winsock→POSIX) — هات‌اسپات LAN
│
├── رندرر: W3DDevice(D3D8) → نیاز به بک‌اند GLES3/Vulkan جدید (بزرگ‌ترین کار)
└── ویدیو: گزینه FFmpeg آپ‌استریم → قابل استفاده روی اندروید
```

### قواعد انضباطی (بخش ۴ Master Prompt)

- کد اندروید فقط در `source/app/src/main/cpp` و ماژول‌های پلتفرمی؛ **هیچ `#ifdef ANDROID` در هسته موتور پخش نمی‌شود** — به‌جای آن پیاده‌سازی رابط‌های موجود (تزریق از طریق DeviceFactory).
- آپ‌استریم با **پین ثابت** fetch می‌شود (`engine/fetch_upstream.sh`)، نه vendor کورکورانه؛ تغییرات بالادستی ما به‌صورت پچ‌های جدا نگهداری و upstream-first پیشنهاد می‌شوند.
- سورس و بیلد کاملاً جدا: سورس این مخزن؛ خروجی‌ها در `build/` (gitignore) و ریلیز در مخزن `GeneralsAndroid`.

## ۵. گراف وابستگی‌ها (فاز موتور)

```
app (Kotlin UI, lobby, importer)
 └── libgenerals.so (CMake/NDK)
      ├── app/src/main/cpp/platform/*   ← کد ما (AndroidDevice + JNI)
      ├── engine/upstream/Generals/Code/GameEngine    (pinned, GPLv3)
      ├── engine/upstream/Generals/Code/GameEngineDevice (بک‌اند Android جدید)
      ├── engine/upstream/Generals/Code/Libraries       (زیرساخت)
      ├── engine/upstream/Core/*                        (هسته مشترک)
      └── zlib, stb (vcpkg) — روی اندروید: zlib داخلی NDK + stb vendored
```
