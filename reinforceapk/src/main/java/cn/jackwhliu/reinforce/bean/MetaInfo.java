package cn.jackwhliu.reinforce.bean;

import java.util.Collection;
import java.util.Map;

public class MetaInfo {
    public String version;
    public String apkFileName;
    public boolean isFrameworkApk;
    public UsesFramework usesFramework;
    public Map<String, String> sdkInfo;
    public PackageInfo packageInfo;
    public VersionInfo versionInfo;
    public boolean compressionType;
    public boolean sharedLibrary;
    public Map<String, String> unknownFiles;
    public Collection<String> doNotCompress;
}
