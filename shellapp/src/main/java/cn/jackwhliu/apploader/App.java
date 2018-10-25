package cn.jackwhliu.apploader;

import android.app.Application;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.util.Log;

import java.lang.reflect.Method;

public class App extends Application {

    /**
     * 脱壳后源程序apk的Application。
     */
    private Object mLoadedApplication;

    /**
     * 资源管理器。
     */
    protected static AssetManager sAssetManager;

    /**
     * 资源。
     */
    protected Resources mResources;

    /**
     * 主题。
     */
    protected Resources.Theme mTheme;

    static {
        System.loadLibrary("jiagu");    //脱壳过程在这个动态库实现
    }

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        Log.d("AppLoader", "attachBaseContext");
        String applicationName = getReinforceApkApplicationClassName();
        mLoadedApplication = onAppAttach(this, applicationName);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d("AppLoader", "onApplicationCreate");
//        String appName = getReinforceApkAppName();
//        onAppCreate(this, obj, getReinforceApkAppName());
    }

    private String getReinforceApkApplicationClassName() {
        String applicationName = null;
        try {
            ApplicationInfo applicationInfo = getPackageManager().getApplicationInfo(
                    getPackageName(), PackageManager.GET_META_DATA);
            applicationName = applicationInfo.metaData.getString("application_name");
            applicationName = "null".equals(applicationName) ? null : applicationName;
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return applicationName;
    }

    private native Object onAppAttach(Application app, String application);

    private native void onAppCreate(Application app, Object obj, String application);

    protected void loadResources(String dexPath) {
        try {
            AssetManager am = AssetManager.class.newInstance();
            Method addAssetPath = am.getClass().getMethod("addAssetPath", String.class);
            addAssetPath.invoke(am, dexPath);
            sAssetManager = am;
        } catch (Exception e) {
            e.printStackTrace();
        }
        Resources superRes = super.getResources();
        superRes.getDisplayMetrics();
        superRes.getConfiguration();
        mResources = new Resources(sAssetManager, superRes.getDisplayMetrics(),superRes.getConfiguration());
        mTheme = mResources.newTheme();
        mTheme.setTo(super.getTheme());
    }

    @Override
    public AssetManager getAssets() {
        return sAssetManager == null ? super.getAssets() : sAssetManager;
    }

    @Override
    public Resources getResources() {
        if (sAssetManager != null && mResources == null) {
            Resources superRes = super.getResources();
            mResources = new Resources(sAssetManager, superRes.getDisplayMetrics(), superRes.getConfiguration());
        }
        return mResources == null ? super.getResources() : mResources;
    }

    @Override
    public Resources.Theme getTheme() {
        if (mResources != null && mTheme == null) {
            mTheme = mResources.newTheme();
            mTheme.setTo(super.getTheme());
        }
        return mTheme == null ? super.getTheme() : mTheme;
    }
}
