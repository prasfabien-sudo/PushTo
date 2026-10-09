plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "fr.dobsonpushto.mobile"
    compileSdk = 35

    buildFeatures {
        buildConfig = true
    }

    defaultConfig {
        applicationId = "fr.dobsonpushto.mobile"
        minSdk = 23
        targetSdk = 35
        versionCode = 7
        versionName = "0.7.0"
    }

    dependencies {
        implementation("androidx.core:core:1.16.0")
        testImplementation("junit:junit:4.13.2")
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = "17"
    }
}
