# Upstream Engine — پین و قواعد

| مورد | مقدار |
|---|---|
| مخزن | https://github.com/TheSuperHackers/GeneralsGameCode |
| برنچ | `main` |
| پین (commit) | `f8ba7eb44b688db14e2ff24d7172474f70be1587` |
| تاریخ پین | 2026-10-04 |
| لایسنس | GPL-3.0-or-later + شرایط EA (→ `UPSTREAM-LICENSE.md`) |

## قاعده

- سورس موتور **vendor نمی‌شود**؛ با اسکریپت زیر روی همین پین فچ می‌شود (در `engine/upstream/` که gitignore است).
- ارتقای پین = کامیت جدا فقط شامل تغییر همین فایل + اجرای مجدد CI.
- پچ‌های لازم ما روی موتور، در `engine/patches/*.patch` نگهداری و upstream-first پیشنهاد می‌شوند.

## استفاده

```bash
./fetch_upstream.sh                      # فچ روی پین
./fetch_upstream.sh <other-commit-sha>   # فچ روی پین دلخواه (آزمایش)
```
