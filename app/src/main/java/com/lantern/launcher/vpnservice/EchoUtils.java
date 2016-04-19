package com.lantern.launcher.vpnservice;

/**
 * Created by Wang on 4/19/2016.
 */
public class EchoUtils {
    static{
        System.loadLibrary("hans_java");
    }
    public static native void connectToServer(String serverIp, String mtu);
}
