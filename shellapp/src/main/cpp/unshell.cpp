#include <string>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "unzip.h"
#include <sys/system_properties.h>
#include "unshell.h"

#ifdef __cplusplus
extern "C" {
#endif

void Java_cn_jackwhliu_apploader_App_onAppCreate(JNIEnv *env, jobject obj, jobject app, jobject hostApp,
                                            jstring applicationName) {
    if (hostApp == NULL) {
        if (applicationName == NULL) {
            return;
        } else {
            hostApp = createHostApplication(env, applicationName, env->NewStringUTF(apkFileName));
        }
    }
    // ActivityThread
    jclass clsActThread = env->FindClass("android/app/ActivityThread");
    // currentActivityThread()
    jmethodID curThreadMthId = env->GetStaticMethodID(clsActThread, "currentActivityThread",
                                                      "()Landroid/app/ActivityThread;");
    // ActivityThread.currentActivityThread()
    jobject actThread = env->CallStaticObjectMethod(clsActThread, curThreadMthId);
    // ActivityThread类中的Application mInitialApplication成员变量
    jfieldID initAppId = env->GetFieldID(clsActThread, "mInitialApplication",
                                         "Landroid/app/Application;");
    jobject mInitialApplication = env->GetObjectField(actThread, initAppId);
    // ActivityThread类中的ArrayList<Application> mAllApplications成员变量
    jfieldID allAppId = env->GetFieldID(clsActThread, "mAllApplications", "Ljava/util/ArrayList;");
    jobject mAllApplications = env->GetObjectField(actThread, allAppId);
    // ArrayList
    jclass clsArrList = env->FindClass("java/util/ArrayList");
    // ArrayList的remove()
    jmethodID removeMth = env->GetMethodID(clsArrList, "remove", "(Ljava/lang/Object;)Z");
    env->CallBooleanMethod(mAllApplications, removeMth, mInitialApplication);
    // Activity类中mInitialApplication = hostApp
    env->SetObjectField(actThread, initAppId, hostApp);
    // ActivityThread类中的内部类AppBindData mBoundApplication的成员变量
    jfieldID boundAppId = env->GetFieldID(clsActThread, "mBoundApplication",
                                          "Landroid/app/ActivityThread$AppBindData;");
    jobject mBoundApplication = env->GetObjectField(actThread, boundAppId);
    // ActivityThread.AppBindData
    jclass clsAppBindData = env->FindClass("android/app/ActivityThread$AppBindData");
    // AppBindData类中的LoadedApk成员变量
    jfieldID infoId = env->GetFieldID(clsAppBindData, "info", "Landroid/app/LoadedApk;");
    jobject info = env->GetObjectField(mBoundApplication, infoId);
    // LoadedApk
    jclass clsLoadedApk = env->FindClass("android/app/LoadedApk");
    // LoadedApk类中的Application mApplication成员变量
    jfieldID appId = env->GetFieldID(clsLoadedApk, "mApplication", "Landroid/app/Application;");
    // LoadedApk类中mApplication = hostApp
    env->SetObjectField(info, appId, hostApp);
    // Application
    jclass clsApplication = env->FindClass("android/app/Application");
    // Application#onCreate()
    jmethodID onCreate = env->GetMethodID(clsApplication, "onCreate", "()V");
    // hostApp.onCreate()
    env->CallVoidMethod(hostApp, onCreate);
    // 释放java层包名和native层包名对象
    env->ReleaseStringUTFChars(jPackageName, packageName);
    // 节省内存资源
    free(apkFilePath);
    free(apkLibPath);
    free(apkFileName);
}

jobject createHostApplication(JNIEnv *env, jstring appName, jstring apkFileName) {
    // ActivityThread
    jclass clsActThread = env->FindClass("android/app/ActivityThread");
    // ActivityThread的currentActivityThread的成员变量
    jmethodID curThreadMthId = env->GetStaticMethodID(clsActThread, "currentActivityThread",
                                                      "()Landroid/app/ActivityThread;");
    jobject actThread = env->CallStaticObjectMethod(clsActThread, curThreadMthId);
    // ActivityThread.AppBindData mBoundApplication
    jfieldID boundAppId = env->GetFieldID(clsActThread, "mBoundApplication",
                                          "Landroid/app/ActivityThread$AppBindData;");
    jobject mBoundApplication = env->GetObjectField(actThread, boundAppId);
    // ActivityThread.AppBindData
    jclass clsAppBindData = env->FindClass("android/app/ActivityThread$AppBindData");
    // ActivityThread.AppBindData的成员变量LoadedApk mBoundApplication
    jfieldID infoId = env->GetFieldID(clsAppBindData, "info", "Landroid/app/LoadedApk;");
    jobject info = env->GetObjectField(mBoundApplication, infoId);
    // LoadedApk
    jclass clsLoadedApk = env->FindClass("android/app/LoadedApk");
    // LoadedApk类的成员变量Application mApplication
    jfieldID appId = env->GetFieldID(clsLoadedApk, "mApplication", "Landroid/app/Application;");
    // LoadedApk的Application mApplication = null
    env->SetObjectField(info, appId, NULL);
    // LoadedApk类中的成员变量ApplicationInfo mApplicationInfo
    jfieldID appInfoId = env->GetFieldID(clsLoadedApk, "mApplicationInfo",
                                         "Landroid/content/pm/ApplicationInfo;");
    jobject mApplicationInfo = env->GetObjectField(info, appInfoId);
    // AppBindData类中的ApplicationInfo appInfo
    jfieldID appBindAppInfoId = env->GetFieldID(clsAppBindData, "appInfo",
                                                "Landroid/content/pm/ApplicationInfo;");
    jobject appInfo = env->GetObjectField(mBoundApplication, appBindAppInfoId);
    LOGE("createGameApplication app name: %s", Jstring2CStr(env, appName));
//    LOGE("createGameApplication app name: %s", env->GetStringUTFChars(appName, false));
    // ApplicationInfo
    jclass clsAppInfo = env->FindClass("android/content/pm/ApplicationInfo");
    // ApplicationInfo类中的String className
    jfieldID clsNameId = env->GetFieldID(clsAppInfo, "className", "Ljava/lang/String;");
    // mApplicationInfo对象的String className = appName
    env->SetObjectField(mApplicationInfo, clsNameId, appName);
    // appInfo对象的String className = appName
    env->SetObjectField(appInfo, clsNameId, appName);
    // ApplicationInfo类的String sourceDir
    jfieldID sourceDirId = env->GetFieldID(clsAppInfo, "sourceDir", "Ljava/lang/String;");
    // ApplicationInfo类中String sourceDir = apkFileName
    env->SetObjectField(mApplicationInfo, sourceDirId, apkFileName);
    // LoadedApk类中的String mResDir
    jfieldID resDirId = env->GetFieldID(clsLoadedApk, "mResDir", "Ljava/lang/String;");
    // LoadedApk类中的String mResDir = apkFileName
    env->SetObjectField(info, resDirId, apkFileName);
    // LoadedApk类中的Resources mResources
    jfieldID mResource = env->GetFieldID(clsLoadedApk, "mResources",
                                         "Landroid/content/res/Resources;");
    // LoadedApk类中的Resources mResources = null
    env->SetObjectField(info, mResource, NULL);
    // LoadedApk类中的makeApplication(boolean, Instrumentation)
    jmethodID makeAppMth = env->GetMethodID(clsLoadedApk, "makeApplication",
                                            "(ZLandroid/app/Instrumentation;)Landroid/app/Application;");
    // mBoundedApplication.makeApplication(false, null)
    jobject hostApp = env->CallObjectMethod(info, makeAppMth, false, NULL);
    LOGE("host application created, return app object");
    return hostApp;
}

char *Jstring2CStr(JNIEnv *env, jstring jstr) {
    char *rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("utf-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize len = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (len > 0) {
        rtn = (char *) malloc(len);
        memcpy(rtn, ba, len);
        rtn[len] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

int GetOSVersion() {
    char sdk_ver_str[10];
    __system_property_get("ro.build.version.sdk", sdk_ver_str);
    LOGD("sdk version: %s", sdk_ver_str);
    return atoi(sdk_ver_str);
}

jstring GetPackageName(JNIEnv *env, jobject app) {
    jclass applicationCls = env->FindClass("android/app/Application");
    jmethodID getPkgNameId = env->GetMethodID(applicationCls, "getPackageName",
                                              "()Ljava/lang/String;");
    jstring packageName = (jstring) env->CallObjectMethod(app, getPkgNameId);
    return packageName;
}

void InitPackageName(JNIEnv *env, jobject app) {
    jPackageName = GetPackageName(env, app);
    packageName = (char *) env->GetStringUTFChars(jPackageName, false);
//    packageName = Jstring2CStr(env, GetPackageName(env, app));
    LOGD("package name: %s", packageName);
}

char *GetDexFilePath(JNIEnv *env, jobject app) {
    const char *root = "/data/data/";
//    char* package = Jstring2CStr(env, GetPackageName(env, app));
    const char *path = "/files/dex/";
    int len = strlen(root) + strlen(packageName) + strlen(path) + 1;
    char *buf = (char *) malloc(len);
    buf[len] = 0;
    sprintf(buf, "%s%s%s", root, packageName, path);
    LOGD("dex file path: %s", buf);
//    free(package);
    return buf;
}

char *GetApkFilePath(JNIEnv *env, jobject app) {
    const char *root = "/data/data/";
//    char* package = Jstring2CStr(env, GetPackageName(env, app));
    const char *path = "/files/apkFile/";

    int rootLen = strlen(root);
    int packageLen = strlen(packageName);
    int pathLen = strlen(path);
    int len = rootLen + packageLen + pathLen;
    char *buf = (char *) malloc(len);
    sprintf(buf, "%s%s%s", root, packageName, path);
    buf[len] = 0;
    LOGD("apk file path: %s", buf);
//    free(package);
    return buf;
}

char *GetApkFileName(const char *apkFilePath) {
    const char *decryptFileName = "loader.apk";
    int len = strlen(apkFilePath) + strlen(decryptFileName);
    char *apkFileName = (char *) malloc(len + 1);
    sprintf(apkFileName, "%s%s", apkFilePath, decryptFileName);
    apkFileName[len] = 0;
    LOGD("apk file: %s", apkFileName);
    return apkFileName;
}

char *GetApkLibPath(JNIEnv *env, jobject app) {
    const char *root = "/data/data/";
//    char* package = Jstring2CStr(env, GetPackageName(env, app));
    const char *path = "/files/apkLib/";
    int len = strlen(root) + strlen(packageName) + strlen(path);
    char *buf = (char *) malloc(len + 1);
    sprintf(buf, "%s%s%s", root, packageName, path);
    buf[len] = 0;
    LOGD("apk lib path: %s", buf);
//    free(package);
    return buf;
}

jobject GetAssetsManager(JNIEnv *env, jobject app) {
    jclass myAppCls = env->FindClass("cn/jackwhliu/apploader/App");
    jmethodID methodGetAssets = env->GetMethodID(myAppCls, "getAssets",
                                                 "()Landroid/content/res/AssetManager;");
    return env->CallObjectMethod(app, methodGetAssets);
}

int CreateDir(const char *pDir) {
    int i = 0;
    int iRet;
    int iLen;
    char *pszDir;
    if (NULL == pDir) {
        return 0;
    }
    iRet = access(pDir, 0);
    if (iRet == 0) {
        LOGD("%s is exists", pDir);
        return 0;
    }
    LOGD("pDir len: %d", strlen(pDir));
    pszDir = strdup(pDir);
    iLen = strlen(pszDir);
    LOGD("pszDir: %s", pszDir);
    for (; i < iLen; i++) {
        if (pszDir[i] == '\\' || pszDir[i] == '/') {
            pszDir[i] = '\0';
            LOGD("pszDir: %s", pszDir);
            iRet = access(pszDir, 0);
            if (iRet != 0) {
                iRet = mkdir(pszDir, 0777);
                LOGD("mkdir: %s, ret: %d", pszDir, iRet);
            }
            pszDir[i] = '/';
        }
    }
    if (pszDir[iLen - 1] != '/') {
        iRet = mkdir(pszDir, 0777);
        LOGD("mkdir: %s, ret: %d", pszDir, iRet);
    }
    free(pszDir);
    return iRet;
}

char *GetLibFileName(const char *libFullName) {
    int len = strlen(libFullName);
    int i = len - 1;
    for (; i >= 0; i--) {
        if (libFullName[i] == '/') {
            int size = len - i;
            char *fileName = (char *) malloc(size + 1);
            char *nameStart = (char *) &libFullName[i + 1];
            memcpy(fileName, nameStart, size);
            LOGD("lib file name: %s", fileName);
            fileName[size] = 0;
            return fileName;
        }
    }
    return NULL;
}

void setFileTime(const char *filename, uLong dosdate, tm_unz tmu_date) {
    struct tm newdate;
    newdate.tm_sec = tmu_date.tm_sec;
    newdate.tm_min = tmu_date.tm_min;
    newdate.tm_hour = tmu_date.tm_hour;
    newdate.tm_mday = tmu_date.tm_mday;
    newdate.tm_mon = tmu_date.tm_mon;
    if (tmu_date.tm_year > 1900) {
        newdate.tm_year = tmu_date.tm_year - 1900;
    } else {
        newdate.tm_year = tmu_date.tm_year;
    }
    newdate.tm_isdst = -1;
    struct utimbuf ut;
    ut.actime = ut.modtime = mktime(&newdate);
    utime(filename, &ut);
}

int ExtractFileInZip(unzFile uf, const char *destFile, unz_file_info64 file_info) {
    LOGD("ExtractFileInZip, dest file name: %s", destFile);
    uint size_buf = 2048;
    void *buf = (void *) malloc(size_buf);
    if (buf == NULL) return UNZ_INTERNALERROR;
    int status = unzOpenCurrentFile(uf);
    FILE *fout = NULL;
    if (status == UNZ_OK) {
        fout = fopen(destFile, "a+");
    }
    // Read from the zip, unzip to buffer, and write to disk
    if (fout != NULL) {
        do {
            status = unzReadCurrentFile(uf, buf, size_buf);
//            LOGD("read zip file status: %d", status);
            if (status <= 0) break;
            if (fwrite(buf, status, 1, fout) != 1) {
                status = UNZ_ERRNO;
                break;
            }
        } while (status > 0);
        LOGD("file len: %d", ftell(fout));
        if (fout) fclose(fout);
        // Set the time of the file that has been unzipped
        if (status == 0) {
            setFileTime(destFile, file_info.dosDate, file_info.tmu_date);
        }
    }
    unzCloseCurrentFile(uf);
    free(buf);
    return status;
}

void CopyApkLib(JNIEnv *env, const char *apkFileName, const char *apkLibPath) {
    int ret = CreateDir(apkLibPath);
    if (ret != 0) {
        LOGD("create apk lib path failed");
        return;
    }
    LOGD("CopyApkLib, apk file: %s", apkFileName);
    LOGD("CopyApkLib, is apk file exists: %d", access(apkFileName, 0));
    unzFile uf = unzOpen64(apkFileName);
    if (uf == NULL) {
        LOGD("open zip file failed.");
        return;
    }
    unz_file_info64 file_info = {0};
    int status;
    char filename_in_zip[MAX_FILENAME_LEN] = {0};
    while (true) {
        status = unzGetCurrentFileInfo64(uf, &file_info, filename_in_zip, sizeof(filename_in_zip),
                                         NULL, 0, NULL, 0);
        if (status != UNZ_OK) {
            LOGD("unzip file failed.");
            unzCloseCurrentFile(uf);
            unzClose(uf);
            return;
        }
        char fileDir[5] = {0};
        fileDir[4] = 0;
        memcpy(fileDir, filename_in_zip, 4);
        int res = strcmp(fileDir, "lib/");
        if (res == 0) {
            char *fileName = GetLibFileName(filename_in_zip);
            int len = strlen(fileName) + strlen(apkLibPath);
            char *libFileFullPath = (char *) malloc(len + 1);
            strcpy(libFileFullPath, apkLibPath);
            strcat(libFileFullPath, fileName);
            libFileFullPath[len] = 0;
            LOGD("extract lib file full path: %s", libFileFullPath);
            if (access(libFileFullPath, 0) == 0) {
                FILE *file = fopen(libFileFullPath, "a+");
                long size = ftell(file);
                LOGD("lib file is exists file size: %d", size);
                if (size == file_info.uncompressed_size) {
                    LOGD("lib file is exists and file len is same, do not extract");
                    fclose(file);
                    free(libFileFullPath);
                    free(fileName);
                    unzCloseCurrentFile(uf);
                    int ret = unzGoToNextFile(uf);
                    if (ret != UNZ_OK) {
                        LOGD("go to file end.");
                        unzClose(uf);
                        return;
                    }
                    continue;
                } else {
                    fclose(file);
                    remove(libFileFullPath);
                }
            }
            ExtractFileInZip(uf, libFileFullPath, file_info);
            free(fileName);
            free(libFileFullPath);
        }
        unzCloseCurrentFile(uf);
        int ret = unzGoToNextFile(uf);
        if (ret != UNZ_OK) {
            LOGD("go to file end.");
            unzClose(uf);
            return;
        }
    }
}

bool CopyApkFile(JNIEnv *env, const char *apkFileName, const char *apkFilePath, const char *apkLibPath,
            jobject app) {
    AAssetManager *mgr = AAssetManager_fromJava(env, GetAssetsManager(env, app));
    const char *mfile = "Ldal.bin"; //源程序加密后的二进制文件
    AAsset *asset = AAssetManager_open(mgr, mfile, AASSET_MODE_UNKNOWN);
    if (asset == NULL) {
        LOGD("%s", "asset is NULL");
        return NULL;
    }
    int ret = CreateDir(apkFilePath);
    if (ret != 0) {
        LOGD("create apk file path failed");
        AAsset_close(asset);
        return false;
    }
//    const char* decryptFileName = "loader.so";
//    int len = strlen(apkFilePath) + strlen(decryptFileName) + 1;
//    char *apkFileName = (char *) malloc(len);
//    apkFileName[len] = 0;
//    sprintf(apkFileName, "%s%s", apkFilePath, decryptFileName);
//    LOGD("apk file: %s", apkFileName);

//    char *apkFileName = GetApkFileName(apkFilePath);
    off_t fileLen = AAsset_getLength(asset);
    LOGD("file length: %d", fileLen);
    FILE *file = fopen(apkFileName, "a+");
    if (file == NULL) {
//        free(apkFileName);
        AAsset_close(asset);
        LOGE("apk file is NULL, error: %s", strerror(errno));
        return NULL;
    }
    if (access(apkFileName, 0) == 0) {
        long size = ftell(file);
        LOGD("apk file size: %d", size);
        if (size == fileLen) {
            LOGE("apk file %s is exists", apkFileName);
            AAsset_close(asset);
            fclose(file);
            CopyApkLib(env, apkFileName, apkLibPath);
            return true;
        } else {
            fclose(file);
            remove(apkFileName);
            file = fopen(apkFileName, "w");
        }
    }
    char *buffer = (char *) malloc(2048);
    int numBytesRead = 0;
    while ((numBytesRead = AAsset_read(asset, buffer, 2048)) > 0) {
        for (int i = 0; i < numBytesRead; i++) {
            buffer[i] ^= 0xa1;
        }
        fwrite(buffer, numBytesRead, 1, file);
    }
    free(buffer);
    fclose(file);
    AAsset_close(asset);
    CopyApkLib(env, apkFileName, apkLibPath);
    return true;
}

char *GetSourceApkFilePath(JNIEnv *env, jobject app) {
    jclass clsApplication = env->FindClass("android/app/Application");
    jmethodID getApplicationInfo = env->GetMethodID(clsApplication, "getApplicationInfo",
                                                    "()Landroid/content/pm/ApplicationInfo;");
    jobject applicationInfo = env->CallObjectMethod(app, getApplicationInfo);
    jclass clsAppInfo = env->FindClass("android/content/pm/ApplicationInfo");
    jfieldID sourDirId = env->GetFieldID(clsAppInfo, "sourceDir", "Ljava/lang/String;");
    jstring sourceDir = (jstring) env->GetObjectField(applicationInfo, sourDirId);
//    return Jstring2CStr(env, sourceDir);
    return (char *) env->GetStringUTFChars(sourceDir, false);
}

void UNZipDexFile(JNIEnv *env, const char *dexFilePath, jobject app) {
    jclass clsApplication = env->FindClass("android/app/Application");
    jmethodID getApplicationInfo = env->GetMethodID(clsApplication, "getApplicationInfo",
                                                    "()Landroid/content/pm/ApplicationInfo;");
    jobject applicationInfo = env->CallObjectMethod(app, getApplicationInfo);
    jclass clsAppInfo = env->FindClass("android/content/pm/ApplicationInfo");
    jfieldID sourDirId = env->GetFieldID(clsAppInfo, "sourceDir", "Ljava/lang/String;");
    jstring sourceDir = (jstring) env->GetObjectField(applicationInfo, sourDirId);
    char *sourceApkFile = (char *) env->GetStringUTFChars(sourceDir, false);
    LOGD("source apk file: %s", sourceApkFile);
    unz_file_info64 file_info = {0};
    int status;
    char filename_in_zip[MAX_FILENAME_LEN] = {0};
    unzFile uf = unzOpen64(sourceApkFile);
    if (uf == NULL) {
        LOGD("open zip file failed.");
//        free(sourceApkFile);
        env->ReleaseStringUTFChars(sourceDir, sourceApkFile);
        return;
    }
    while (true) {
        status = unzGetCurrentFileInfo64(uf, &file_info, filename_in_zip, sizeof(filename_in_zip),
                                         NULL, 0, NULL, 0);
        if (status != UNZ_OK) {
            LOGD("unzip file failed.");
            unzClose(uf);
//            free(sourceApkFile);
            env->ReleaseStringUTFChars(sourceDir, sourceApkFile);
            return;
        }
        int res = strcmp(filename_in_zip, "classes.dex");
        LOGD("is dex file: %d, file: %s", res, filename_in_zip);
        if (res == 0) {
            FILE *dexFile = fopen(dexFilePath, "a+");
            if (dexFile == NULL) {
                LOGD("open dex file failed, file path: %s, err: %s", dexFilePath, strerror(errno));
                unzClose(uf);
//                free(sourceApkFile);
                env->ReleaseStringUTFChars(sourceDir, sourceApkFile);
                return;
            }
            if (access(dexFilePath, 0) == 0) {
                int dexFileLen = ftell(dexFile);
                if (dexFileLen == file_info.uncompressed_size) {
                    LOGD("dex file has uncompressed.");
                    fclose(dexFile);
                    break;
                }
                remove(dexFilePath);
            }
            fclose(dexFile);
            ExtractFileInZip(uf, dexFilePath, file_info);
            break;
        }
        int ret = unzGoToNextFile(uf);
        if (ret != UNZ_OK) {
            LOGD("go to file end.");
            unzClose(uf);
//            free(sourceApkFile);
            env->ReleaseStringUTFChars(sourceDir, sourceApkFile);
            return;
        }
    }
    unzClose(uf);
    env->ReleaseStringUTFChars(sourceDir, sourceApkFile);
//    free(sourceApkFile);
}

int GetApkFileLength(const char *dexFilePath) {
    FILE *file = fopen(dexFilePath, "rb");
    fseek(file, 0, SEEK_END);
    long fileLen = ftell(file);
    LOGD("dex file length: %d", fileLen);
    if (fileLen == 0) {
        fclose(file);
        return 0;
    }
    fseek(file, -4, SEEK_END);
    int index = ftell(file);
    LOGD("GetApkFileLength, dex read index: %d", index);
    char apkLenBytes[4] = {0};
    fread(apkLenBytes, sizeof(char), 4, file);
    fclose(file);
    int apkLen = apkLenBytes[0] & 0xff;
    for (int i = 1; i < 4; i++) {
        apkLen = (apkLen << 8) | (apkLenBytes[i] & 0xff);
    }
    LOGD("apk file len: %d", apkLen);
    return apkLen;
}

char *ssr(JNIEnv *env, const char *apkFilePath, const char *apkLibPath, jobject app) {
    char *dexFilePath = GetDexFilePath(env, app);
    LOGD("dex file: %s", dexFilePath);
    CreateDir(dexFilePath);
    int dexPathLen = strlen(dexFilePath) + strlen("classes.dex");
    char *dexFileName = (char *) malloc(dexPathLen + 1);
    dexFileName[dexPathLen] = 0;
    strcpy(dexFileName, dexFilePath);
    strcat(dexFileName, "classes.dex");
    int ret = CreateDir(apkFilePath);
    if (ret != 0) {
        LOGD("create apk file path failed");
        free(dexFilePath);
        free(dexFileName);
        return NULL;
    }

    char *apkFileName = GetApkFileName(apkFilePath);

    FILE *apkFile = fopen(apkFileName, "a+");
    if (apkFile == NULL) {
        free(dexFilePath);
        free(dexFileName);
        free(apkFileName);
        LOGD("apk file is NULL, error: %s", strerror(errno));
        return NULL;
    }

    UNZipDexFile(env, dexFileName, app);

    if (access(dexFileName, 0) == 0) {
        int apkFileLen = GetApkFileLength(dexFileName);
        if (apkFileLen == 0) {
            free(dexFilePath);
            free(dexFileName);
            free(apkFileName);
            return NULL;
        }
        if (access(apkFileName, 0) == 0) {
            long size = ftell(apkFile);
            LOGD("apk file size: %d", size);
            if (size == apkFileLen) {
                LOGD("apk file %s is exists", apkFileName);
                fclose(apkFile);
                free(dexFilePath);
                free(dexFileName);
                CopyApkLib(env, apkFileName, apkLibPath);
                return apkFileName;
            } else if (size > 0) {
                fclose(apkFile);
                remove(apkFileName);
                apkFile = fopen(apkFileName, "a+");
            }
        }
        FILE *dexFile = fopen(dexFileName, "rb");
        fseek(dexFile, -(apkFileLen + 4), SEEK_END);
        LOGD("read apk index: %d", ftell(dexFile));
        char *buffer = (char *) malloc(16384);
        int readSize = 0;
        while ((readSize = fread(buffer, sizeof(char), 16384, dexFile)) > 0) {
            if (readSize < 16384) {
                readSize -= 4;
            }

//            decrypt(buffer);
//            LOGD("read apk file size: %d", readSize);
            for (int i = 0; i < readSize; i++) {
                buffer[i] ^= 0xa1;
            }
            fwrite(buffer, readSize, 1, apkFile);
        }

        free(buffer);

        LOGD("apk file size: %d", ftell(apkFile));
        char bytes[4] = {0};
        fseek(apkFile, 0, SEEK_SET);
        fread(bytes, sizeof(char), 4, apkFile);
        fseek(apkFile, -4, SEEK_END);
        fread(bytes, sizeof(char), 4, apkFile);
        fclose(apkFile);

        LOGD("apk file is exists: %d", access(apkFileName, 0));
        CopyApkLib(env, apkFileName, apkLibPath);
    } else {
        fclose(apkFile);
    }

    free(dexFilePath);
    free(dexFileName);

    return apkFileName;
}

void LoadResource(JNIEnv *env, jstring apkFileName) {
    jclass clsAssetsMgr = env->FindClass("android/content/res/AssetManager");
    jmethodID initMethodId = env->GetMethodID(clsAssetsMgr, "<init>", "()V");
    jobject assetsMgr = env->NewObject(clsAssetsMgr, initMethodId);

    jmethodID addAssetsMthd = env->GetMethodID(clsAssetsMgr, "addAssetPath",
                                               "(Ljava/lang/String;)I");
    env->CallIntMethod(assetsMgr, addAssetsMthd, apkFileName);

    jclass myAppCls = env->FindClass("cn/jackwhliu/apploader/App");
    jfieldID fieldAssets = env->GetStaticFieldID(myAppCls, "mAssetManager",
                                                 "Landroid/content/res/AssetManager;");
    env->SetStaticObjectField(myAppCls, fieldAssets, assetsMgr);

//    InitResource(env, objLoadedApk);

//    jclass myAppCls = env->FindClass("com/jackwhliu/apploader/App");
//    jfieldID sAppId = env->GetStaticFieldID(myAppCls, "sApplication", "Landroid/app/Application;");
//    jobject application = env->GetStaticObjectField(myAppCls, sAppId);

//    jmethodID loadRes = env->GetMethodID(myAppCls, "loadResources", "(Ljava/lang/String;)V");
//    env->CallVoidMethod(application, loadRes, apkFileName);
}

jobject Java_cn_jackwhliu_apploader_App_onAppAttach(JNIEnv *env, jobject thiz, jobject app,
                                                    jstring appName) {
    InitPackageName(env, app);
    apkFilePath = GetApkFilePath(env, app);
    apkLibPath = GetApkLibPath(env, app);
    apkFileName = GetApkFileName(apkFilePath);

    LOGD("apkFilePath: %s", apkFilePath);
    LOGD("apkLibPath: %s", apkLibPath);
    LOGD("apkFileName: %s", apkFileName);

    bool ret = CopyApkFile(env, apkFileName, apkFilePath, apkLibPath, app);
    if (!ret) {
        return NULL;
    }

    jstring strApkFileName = env->NewStringUTF(apkFileName);
    jstring strApkFilePath = env->NewStringUTF(apkFilePath);
    jstring strApkLibPath = env->NewStringUTF(apkLibPath);

//    LoadResource(env, strApkFileName);

    jclass clsActThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(clsActThread, "currentActivityThread",
                                                             "()Landroid/app/ActivityThread;");
    jobject actThread = env->CallStaticObjectMethod(clsActThread, currentActivityThread);
    jfieldID mPackagesId;
    if (GetOSVersion() >= 19) {
        mPackagesId = env->GetFieldID(clsActThread, "mPackages", "Landroid/util/ArrayMap;");
    } else {
        mPackagesId = env->GetFieldID(clsActThread, "mPackages", "Ljava/util/HashMap;");
    }
    jobject mPackages = env->GetObjectField(actThread, mPackagesId);

    jclass mapCls = env->FindClass("java/util/Map");
    jmethodID getId = env->GetMethodID(mapCls, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
    jobject wr = env->CallObjectMethod(mPackages, getId, jPackageName);

    jclass wrclass = env->FindClass("java/lang/ref/WeakReference");
    jmethodID methodGet = env->GetMethodID(wrclass, "get", "()Ljava/lang/Object;");
    jobject objApkLoader = env->CallObjectMethod(wr, methodGet);

    jclass clsApkLoader = env->FindClass("android/app/LoadedApk");
    jfieldID fieldClassLoader = env->GetFieldID(clsApkLoader, "mClassLoader",
                                                "Ljava/lang/ClassLoader;");
    jobject classDexLoader = env->GetObjectField(objApkLoader, fieldClassLoader);

    jclass dexClassLoader = env->FindClass("dalvik/system/DexClassLoader");
    jmethodID initDexLoaderMethod = env->GetMethodID(dexClassLoader, "<init>",
                                                     "(Ljava/lang/String;Ljava/lang/String;"
                                                             "Ljava/lang/String;Ljava/lang/ClassLoader;)V");
    jobject dexLoader = env->NewObject(dexClassLoader, initDexLoaderMethod, strApkFileName,
                                       strApkFilePath, strApkLibPath, classDexLoader);
    env->SetObjectField(objApkLoader, fieldClassLoader, dexLoader);

    LoadResource(env, strApkFileName);

    jfieldID resDirId = env->GetFieldID(clsApkLoader, "mResDir", "Ljava/lang/String;");
    jstring resDir = (jstring) env->GetObjectField(objApkLoader, resDirId);
//    LOGD("res dir: %s", Jstring2CStr(env, resDir));
//    LOGD("res dir: %s", env->GetStringUTFChars(resDir, false));
    env->SetObjectField(objApkLoader, resDirId, strApkFileName);
//
//    jfieldID mResource = env->GetFieldID(clsApkLoader, "mResources", "Landroid/content/res/Resources;");
//    env->SetObjectField(objApkLoader, mResource, NULL);

//    LoadResource(env, strApkFileName);
//    jmethodID getResources = env->GetMethodID(clsApkLoader, "getResources", "(Landroid/app/ActivityThread;)Landroid/content/res/Resources;");
//    env->CallObjectMethod(objApkLoader, getResources, actThread);

//    jfieldID applicationInfoId = env->GetFieldID(clsApkLoader, "mApplicationInfo", "Landroid/content/pm/ApplicationInfo;");
//    jobject mApplicationInfo = env->GetObjectField(objApkLoader, applicationInfoId);

//    jclass clsAppInfo = env->FindClass("android/content/pm/ApplicationInfo");
//    jfieldID sourceDirId = env->GetFieldID(clsAppInfo, "sourceDir", "Ljava/lang/String;");
//    env->SetObjectField(mApplicationInfo, sourceDirId, strApkFileName);

    jobject hostApp = NULL;
    if (appName != NULL) {
//        LOGE("app name: %s", Jstring2CStr(env, appName));
//        LOGE("app name: %s", env->GetStringUTFChars(appName, false));
        hostApp = createHostApplication(env, appName, strApkFileName);
    }

//    if(apkFilePath != NULL) {
//        free(apkFilePath);
//        apkFilePath = NULL;
//    }
//    if(apkLibPath != NULL) {
//        free(apkLibPath);
//        apkLibPath = NULL;
//    }
//    if(apkFileName != NULL) {
//        free(apkFileName);
//        apkFileName = NULL;
//    }

    LOGE("onAppAttach finish, return host application object");

    return hostApp;
}

//JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
//    JNIEnv* env = NULL;
//
//    if(vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
//        return -1;
//    }
//
//    char *apkFilePath = GetApkFilePath(env);
//    char *apkLibPath = GetApkLibPath(env);
//    char *apkFileName = CopyApkFile(env, apkFilePath, apkLibPath);
//
//    jstring strApkFileName = env->NewStringUTF(apkFileName);
//    jstring strApkFilePath = env->NewStringUTF(apkFilePath);
//    jstring strApkLibPath = env->NewStringUTF(apkLibPath);
//
////    LoadResource(env, strApkFileName);
//
//    jclass clsActThread = env->FindClass("android/app/ActivityThread");
//    jmethodID currentActivityThread = env->GetStaticMethodID(clsActThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
//    jobject actThread = env->CallStaticObjectMethod(clsActThread, currentActivityThread);
//    jfieldID mPackagesId;
//    if(GetOSVersion() >= 19) {
//        mPackagesId = env->GetFieldID(clsActThread, "mPackages", "Landroid/util/ArrayMap;");
//    } else {
//        mPackagesId = env->GetFieldID(clsActThread, "mPackages", "Ljava/util/HashMap;");
//    }
//    jobject  mPackages = env->GetObjectField(actThread, mPackagesId);
//
//    jclass mapCls = env->FindClass("java/util/Map");
//    jmethodID getId = env->GetMethodID(mapCls, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
//    jobject  wr = env->CallObjectMethod(mPackages, getId, GetPackageName(env));
//
//    jclass wrclass = env->FindClass("java/lang/ref/WeakReference");
//    jmethodID methodGet = env->GetMethodID(wrclass, "get", "()Ljava/lang/Object;");
//    jobject objApkLoader = env->CallObjectMethod(wr, methodGet);
//
//    jclass clsApkLoader = env->FindClass("android/app/LoadedApk");
//    jfieldID fieldClassLoader = env->GetFieldID(clsApkLoader, "mClassLoader", "Ljava/lang/ClassLoader;");
//    jobject classDexLoader = env->GetObjectField(objApkLoader, fieldClassLoader);
//
//    jclass dexClassLoader = env->FindClass("dalvik/system/DexClassLoader");
//    jmethodID initDexLoaderMethod = env->GetMethodID(dexClassLoader, "<init>",
//                                                     "(Ljava/lang/String;Ljava/lang/String;"
//                                                             "Ljava/lang/String;Ljava/lang/ClassLoader;)V");
//    jobject dexLoader = env->NewObject(dexClassLoader, initDexLoaderMethod, strApkFileName, strApkFilePath, strApkLibPath, classDexLoader);
//    env->SetObjectField(objApkLoader, fieldClassLoader, dexLoader);
//
//    LoadResource(env, strApkFileName);
//
//    jfieldID resDirId = env->GetFieldID(clsApkLoader, "mResDir", "Ljava/lang/String;");
//    jstring resDir = (jstring) env->GetObjectField(objApkLoader, resDirId);
//    LOGD("res dir: %s", Jstring2CStr(env, resDir));
//    env->SetObjectField(objApkLoader, resDirId, strApkFileName);
////
////    jfieldID mResource = env->GetFieldID(clsApkLoader, "mResources", "Landroid/content/res/Resources;");
////    env->SetObjectField(objApkLoader, mResource, NULL);
//
////    LoadResource(env, strApkFileName);
////    jmethodID getResources = env->GetMethodID(clsApkLoader, "getResources", "(Landroid/app/ActivityThread;)Landroid/content/res/Resources;");
////    env->CallObjectMethod(objApkLoader, getResources, actThread);
//
////    jfieldID applicationInfoId = env->GetFieldID(clsApkLoader, "mApplicationInfo", "Landroid/content/pm/ApplicationInfo;");
////    jobject mApplicationInfo = env->GetObjectField(objApkLoader, applicationInfoId);
////
////    jclass clsAppInfo = env->FindClass("android/content/pm/ApplicationInfo");
////    jfieldID sourceDirId = env->GetFieldID(clsAppInfo, "sourceDir", "Ljava/lang/String;");
////    env->SetObjectField(mApplicationInfo, sourceDirId, strApkFileName);
//
//    free(apkFilePath);
//    free(apkLibPath);
//    free(apkFileName);
//
//
//    return JNI_VERSION_1_4;
//}

#ifdef __cplusplus
}
#endif