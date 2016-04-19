package com.lantern.launcher.vpnservice;

import android.content.Intent;
import android.net.VpnService;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by Wang on 4/8/2016.
 */
public class MyVpnService extends VpnService{

    @Override
    public IBinder onBind(Intent intent) {
        Builder builder = new Builder();
        builder.setMtu(15000);
//        网卡的IP地址
        builder.addAddress("172.16.130.46", 24);
//        设置捕捉的数据包的路由
        builder.addRoute("0.0.0.0", 0);
//        端口的DNS服务器地址
        builder.addDnsServer("114.114.114.114");
//        域名补全
//        builder.addSearchDomain(...);
//        建立VPN的名称
        builder.setSession("icmp_vpn");
//        builder.setConfigureIntent(...);
        ParcelFileDescriptor inte = builder.establish();
        // Packets to be sent are queued in this input stream.
        FileInputStream in = new FileInputStream(inte.getFileDescriptor());

// Packets received need to be written to this output stream.
        FileOutputStream out = new FileOutputStream(inte.getFileDescriptor());

// Allocate the buffer for a single packet.
        ByteBuffer packet = ByteBuffer.allocate(32767);
// Read packets sending to this interface
        int length = 0;
        try {
            length = in.read(packet.array());
        } catch (IOException e) {
            e.printStackTrace();
        }
// Write response packets back
        try {
            out.write(packet.array(), 0, length);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return super.onBind(intent);
    }

    @Override
    public boolean protect(int socket) {
        return super.protect(socket);
    }
}
