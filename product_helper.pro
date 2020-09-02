QT += quick multimedia sql svg

CONFIG += c++17

include("QZXing/QZXing.pri")
include("openssl.pri")


localData.files = localDB.db
localData.path = /assets/databases
INSTALLS += localData

INCLUDEPATH += $$PWD/glm

android:contains(QT_ARCH, arm) {
    DEFINES += SLOW_ARM
    INCLUDEPATH += $$PWD/opencv-armeabi-v7a/include

    ANDROID_EXTRA_LIBS += \
            $$PWD/opencv-armeabi-v7a/lib/libopencv_world.so

    LIBS += -L$$PWD/opencv-armeabi-v7a/lib \
            -lopencv_world
}

android:contains(QT_ARCH, arm64) {
    DEFINES += FAST_ARM
    INCLUDEPATH += $$PWD/opencv-arm64-v8a/include

    ANDROID_EXTRA_LIBS += \
            $$PWD/opencv-arm64-v8a/lib/libopencv_world.so \
            $$PWD/opencv-arm64-v8a/lib/libtbb.so

    LIBS += -L$$PWD/opencv-arm64-v8a/lib \
            -ltbb \
            -lopencv_world
}

DEFINES +=  QTAT_APP_PERMISSIONS     \
            QTAT_APK_EXPANSION_FILES \
            QTAT_APP_PERMISSIONS     \
            QTAT_APK_INFO            \
            QTAT_SCREEN              \
            QTAT_SYSTEM              \
            QTAT_BATTERY_STATE       \
            QTAT_SIGNAL_STRENGTH     \
            QTAT_IMAGES              \
            QTAT_NOTIFICATION        \
            QTAT_GOOGLE_ACCOUNT      \
            QTAT_SHARING

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

include($$PWD/QtAndroidTools/QtAndroidTools.pri)


SOURCES += \
        ARVideoFilterRunnable.cpp \
        OBJModelLoader.cpp \
        json_to_hypertext.cpp \
        main.cpp

RESOURCES += qml.qrc


QML_IMPORT_PATH =


QML_DESIGNER_IMPORT_PATH =

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ARVideoFilterRunnable.h \
    BarcodeFinder.h \
    CategorySQLQueryModel.h \
    HistorySQLQueryModel.h \
    OBJModelLoader.h \
    ProductSearchModel.h \
    QueryManager.h \
    TextureBuffer.h \
    json_to_hypertext.h

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml
