plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.obsifox.generals"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.obsifox.generals"
        minSdk = 24
        targetSdk = 34
        versionCode = 5
        versionName = "0.1.3.1-alpha"
        resourceConfigurations += listOf("fa", "en")

        ndk {
            // بخش ۳ Master Prompt: هدف اصلی arm64-v8a
            abiFilters += listOf("arm64-v8a")
        }
        externalNativeBuild {
            cmake {
                cppFlags += listOf("-std=c++17", "-fexceptions", "-frtti")
                arguments += listOf("-DANDROID_STL=c++_shared", "-DGENERALS_BUILD_PLATFORM_ANDROID=ON")
                // فقط لایبرری خودمان ساخته شود؛ ابزارهای دسکتاپ آپ‌استریم
                // (worldbuilder/wdump/...) نباید در بیلد اندروید کامپایل شوند.
                // لینک موتور کامل (win32shim + engine) فقط وقتی صریحاً خواسته
                // شده باشد: GEN_PORT_ENGINE=ON. پیش‌فرض = پوسته‌ی پلتفرم
                // (مسیر سبز و اثبات‌شده‌ی v0.1.2) تا انتشار APK همیشه سبز بماند؛
                // لینک موتور به‌صورت جداگانه ادامه پیدا می‌کند (job پایدار
                // engine-link در CI).
                val engineRoot = System.getenv("GENERALS_ENGINE_ROOT")
                val portEngine = (System.getenv("GEN_PORT_ENGINE") ?: "OFF")
                    .equals("ON", ignoreCase = true)
                val linkEngine = engineRoot != null && portEngine
                arguments += if (linkEngine) {
                    listOf("-DGENERALS_ENGINE_ROOT=$engineRoot")
                } else emptyList()
                // نام هدف هم باید با مود هم‌خوان باشد: در حالت پوسته‌ی پلتفرم
                // فقط هدف legacy «generals» وجود دارد؛ در حالت موتور کامل،
                // «generals_android» (لایه‌ی port، OUTPUT_NAME=generals).
                targets += if (linkEngine) listOf("generals_android")
                           else listOf("generals")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            // آپ‌استریم پین‌شده cmake_minimum_required(3.28) دارد؛ SDK با 3.31.1
            // به‌صورت خودکار روی CI نصب می‌شود.
            version = "3.31.1"
        }
    }

    signingConfigs {
        // فقط وقتی CI/کاربر کی‌استور داده باشد فعال می‌شود
        val ksPath: String? = System.getenv("OBSI_KEYSTORE_PATH")
        if (ksPath != null) {
            create("release") {
                storeFile = file(ksPath)
                storePassword = System.getenv("OBSI_KEYSTORE_PASS")
                keyAlias = System.getenv("OBSI_KEY_ALIAS")
                keyPassword = System.getenv("OBSI_KEY_PASS")
            }
        }
    }

    buildTypes {
        debug {
            applicationIdSuffix = ".debug"
            versionNameSuffix = "-debug"
        }
        release {
            isMinifyEnabled = false
            isShrinkResources = false
            val ksPath: String? = System.getenv("OBSI_KEYSTORE_PATH")
            if (ksPath != null) {
                signingConfig = signingConfigs.getByName("release")
            }
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }
    packaging {
        resources.excludes += "/META-INF/{AL2.0,LGPL2.1}"
    }
}

dependencies {
    implementation("androidx.core:core-ktx:1.13.1")
    implementation("androidx.appcompat:appcompat:1.7.0")
    // SAF document tree traversal (game files importer — Master Prompt §13)
    implementation("androidx.documentfile:documentfile:1.0.1")
    // activity-result API (wizard folder picker)
    implementation("androidx.activity:activity-ktx:1.9.3")
}
