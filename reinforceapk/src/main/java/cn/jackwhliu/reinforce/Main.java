package cn.jackwhliu.reinforce;

import java.io.File;
import java.io.IOException;

import cn.jackwhliu.reinforce.bean.ManifestInfo;
import cn.jackwhliu.reinforce.shell.ShellPack;
import cn.jackwhliu.reinforce.util.Apktool;
import cn.jackwhliu.reinforce.util.FileUtils;
import cn.jackwhliu.reinforce.util.Utils;

public class Main {

    public static void main(String... args) throws Exception {
        System.out.println("unpack host apk");
        Apktool.unpackApk(Utils.HOST_APK_NAME);
        System.out.println("read host apk manifest info");
        ManifestInfo hostInfo = ManifestInfo.loadManifest(new File(Utils.HOST_APK_UNPACK_PATH));
        System.out.println(hostInfo.toString());
        System.out.println("unpack shell apk");
        ShellPack shellPack = new ShellPack();
        shellPack.unpackShellApk(Utils.SHELL_APK_NAME);
        System.out.println("change shell manifest info");
        shellPack.rewriteManifest(hostInfo, Utils.SHELL_APK_UNPACK_PATH);
        System.out.println("change shell launcher icon");
        shellPack.copyLauncherIcon(hostInfo.iconResName);
        System.out.println("change shell app name");
        shellPack.changeAppName(hostInfo.appResName);
        System.out.println("change shell app version name and version code");
        shellPack.changeVersion(hostInfo.versionCode, hostInfo.versionName);
        System.out.println("encrypt host apk and copy to shell apk assets directory");
        encryptHostApk();
        System.out.println("repack shell apk");
        shellPack.repackShell();
        System.out.println("sign shell apk");
        shellPack.signShellApk(hostInfo.packageName);
    }

    /**
     * 加密源程序apk文件。
     *
     * @throws IOException
     */
    private static void encryptHostApk() throws IOException {
        File apkFile = new File(Utils.HOST_APK_NAME);
        byte[] apkFileBytes = encrypt(FileUtils.readFileBytes(apkFile));
        FileUtils.writeBytes2File(Utils.HOST_APK_ENCRYPT_PATH, apkFileBytes);
    }

    /**
     * 加密二进制数据。
     *
     * @param data 加密前的数据
     * @return 加密后的数据
     */
    private static byte[] encrypt(byte[] data) {
        for (int i = 0; i < data.length; i++) {
            data[i] = (byte) (0xa1 ^ data[i]);
        }
        return data;
    }
}
