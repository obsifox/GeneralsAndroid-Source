<div dir="rtl">

# جنرالز اندروید — پورت بومی فارسی ⚙️🇮🇷

**پورت بومی (Native) بازی Command & Conquer: Generals و Zero Hour برای اندروید، با رابط فارسی کامل (RTL)، کنترل‌های لمسی مخصوص موبایل و مالتی‌پلیر محلی از طریق هات‌اسپات.**

> ⚠️ این یک پورت **غیررسمی و اجتماعی** است و به EA وابسته نیست.
> این برنامه **هیچ فایل بازی را همراه خودش ندارد** — شما باید فایل‌های بازیِ خودتان (نسخه‌ی قانونی) را وارد کنید.
> موتور بازی کد بازشدگی EA است که تحت **GPLv3** منتشر شده و در مخزن [TheSuperHackers/GeneralsGameCode](https://github.com/TheSuperHackers/GeneralsGameCode) نگهداری می‌شود.

| مخزن | آدرس | توضیح |
|---|---|---|
| 📦 دانلود APK | [obsifox/GeneralsAndroid](https://github.com/obsifox/GeneralsAndroid) | ریلیزها و بیلدها (جدا از سورس) |
| 📜 این مخزن (سورس) | [obsifox/GeneralsAndroid-Source](https://github.com/obsifox/GeneralsAndroid-Source) | فقط سورس؛ بیلد خودکار با GitHub Actions |

## وضعیت پروژه (شفاف و بدون اغراق)

<!-- STATUS:BEGIN -->
| بخش | وضعیت |
|---|---|
| لایه پلتفرم اندروید (JNI/NDK، ARM64) | ✅ پیاده‌سازی اولیه |
| رابط کاربری فارسی RTL (لانچر، ویزارد، تنظیمات) | ✅ قابل استفاده |
| واردکردن و اعتبارسنجی فایل‌های بازی (SAF) | ✅ قابل استفاده |
| مالتی‌پلیر هات‌اسپات (کشف UDP + لابی TCP + چت) | ✅ لابی واقعی کار می‌کند |
| اتصال لابی به موتور بازی | 🔜 فاز بعدی |
| رندرر موبایل (GLES3/Vulkan جایگزین D3D8) | 🔧 در حال طراحی — به [KNOWN-BLOCKERS](docs/KNOWN-BLOCKERS.md) |
| اجرای کامل کمپین/اسکرمیش | ❌ هنوز نه |
<!-- STATUS:END -->

مستندات کامل در پوشه [`docs/`](docs/) — شروع از [IMPLEMENTATION_PLAN.md](docs/IMPLEMENTATION_PLAN.md).

## ساخت سریع

```bash
# در CI (GitHub Actions) به صورت خودکار انجام می‌شود
cd source && gradle :app:assembleDebug
```

راهنمای کامل: [`docs/BUILD.md`](docs/BUILD.md)

## لایسنس

- کد این مخزن و موتور آپ‌استریم: **GPL-3.0-or-later** (به‌همراه شرایط تکمیلی EA — متن کامل: [engine/UPSTREAM-LICENSE.md](engine/UPSTREAM-LICENSE.md))
- فونت پیشنهادی رابط: Vazirmatn — **OFL**
- هیچ asset متعلق به EA (مپ، تکسچر، موزیک، ویدیو، فونت) در این مخزن یا APK قرار ندارد. **هرگز نخواهد داشت.**

</div>

---

<div dir="ltr">

# Generals Android — Native Persian Port

Native Android port of **Command & Conquer: Generals / Zero Hour** with full Persian (RTL) UI, mobile-first touch controls, and **hotspot LAN multiplayer**.

Unofficial community port — not affiliated with EA. Bring your own legally obtained game files. Engine code: EA's GPLv3 release, maintained by TheSuperHackers.

- APK downloads → [`obsifox/GeneralsAndroid`](https://github.com/obsifox/GeneralsAndroid)
- Docs (mostly Persian) → [`docs/`](docs/)
- License: GPL-3.0-or-later (see [engine/UPSTREAM-LICENSE.md](engine/UPSTREAM-LICENSE.md)). No EA assets included.

</div>
