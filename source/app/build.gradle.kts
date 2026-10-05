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
        versionCode = 3
        versionName = "0.1.2-alpha"
        resourceConfigurations += listOf("fa", "en")

        ndk {
            // بخش ۳ Master Prompt: هدف اصلی arm64-v8a
            abiFilters += listOf("arm64-v8a")
        }
        externalNativeBuild {
            cmake {
                cppFlags += listOf("-std=c++17", "-fexceptions", "-frtti")
                arguments += listOf("-DANDROID_STL=c++_shared", "-DGENERALS_BUILD_PLATFORM_ANDROID=ON")
                arguments += System.getenv("GENERALS_ENGINE_ROOT")?.let {
                    listOf("-DGENERALS_ENGINE_ROOT=$it")
                } ?: emptyList()
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
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
