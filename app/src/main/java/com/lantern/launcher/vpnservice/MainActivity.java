package com.lantern.launcher.vpnservice;

import android.content.Context;
import android.net.DhcpInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private Button switch_button;
    private WifiManager myWifiManager;
    private DhcpInfo myDhcpInfo;
    private EditText serverIp;
    private EditText password;
    private EditText mtu;
    private boolean vpnStatus = false;
    private TextView logText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
    }

    private void initView() {
        serverIp = (EditText) findViewById(R.id.serverIp);
        password = (EditText) findViewById(R.id.serverpwd);
        mtu = (EditText) findViewById(R.id.mtu);
        switch_button = (Button) findViewById(R.id.switch_button);
        logText = (TextView) findViewById(R.id.log);
        switch_button.setOnClickListener(this);
        String sip = (String) SpUtils.get(this, "serverIp", "");
        String pwd = (String) SpUtils.get(this, "serverPwd", "");
        if(!TextUtils.isEmpty(sip))
            serverIp.setText(sip);
        if(!TextUtils.isEmpty(pwd))
            password.setText(pwd);
        myWifiManager = (WifiManager) getSystemService(WIFI_SERVICE);
        myDhcpInfo = myWifiManager.getDhcpInfo();
        Log.e("Error", "My Gateway is :" + intToIp(myDhcpInfo.gateway));
        stopVPN();
        //复制文件到项目目录中，并授予权限
        Log.e("ERROR", "当前CPU架构:"+ Build.CPU_ABI);
        copyFile(Build.CPU_ABI +"/hans", "/data/data/com.lantern.launcher.vpnservice/hans", MainActivity.this);
        boolean b = execCommand(new String[]{"chmod 777 /data/data/com.lantern.launcher.vpnservice/hans"});
        if(b){
            Log.e("ERROR", "initial success");
        }else {
            Log.e("ERROR", "initial failed");
        }
        execCommand(new String[]{"ps |grep com.android.musicfx"});
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()){
            case R.id.switch_button:
                if(!vpnStatus){
                    //启动
                    stopVPN();
                    changeButton();
                    startVPN();
                } else {
                    //停止
                    changeButton();
                    stopVPN();
                }
                break;
        }
    }

    private void changeButton() {
        if(!vpnStatus){
            switch_button.setText("停止");
            switch_button.setBackgroundColor(getResources().getColor(R.color.red));
            SpUtils.put(this, "serverIp", serverIp.getText().toString());
            SpUtils.put(this, "serverPwd", password.getText().toString());
        } else {
            switch_button.setText("启动");
            switch_button.setBackgroundColor(getResources().getColor(R.color.green));
        }
    }

    private void startVPN() {
        String startServer = "/data/data/com.lantern.launcher.vpnservice/hans -c ";
        if(TextUtils.isEmpty(serverIp.getText().toString())){
            startServer += "182.254.245.209";
        } else {
            startServer += serverIp.getText().toString();
        }
        if(TextUtils.isEmpty(password.getText().toString())){
            startServer += " -p 123456";
        }else {
            startServer += " -p " + password.getText().toString();
        }
        if(!TextUtils.isEmpty(mtu.getText().toString())){
            startServer += " -m " + mtu.getText().toString();
        }
        LogE("execute command: " + startServer);
        boolean b = execCommand(new String[]{startServer});
        if(b){
            LogE("Run Successful");
        }else {
            LogE("Run failed");
        }
        if (isProcessAlive("hans")){
            vpnStatus = true;
        } else {
            vpnStatus = false;
        }
    }

    private void LogE(String s) {
        Log.e("ERROR", s);
        String te = logText.getText().toString();
        logText.setText(s + "\n" + te);
    }

    private void stopVPN() {
        LogE("execute command: ps |grep hans|grep -v grep|awk '{print $2}' |xargs kill \n" +
                "ip rule del prio 100");
        boolean b = execCommand(new String[]{"ps |grep hans|grep -v grep|awk '{print $2}' |xargs kill",
                                    "ip rule del prio 100"});
        if(b){
            LogE("Killed successful");
        }else {
            LogE("Killed failed");
        }
        vpnStatus = false;
    }

    private static void copyFile(String assetPath, String localPath, Context context) {
        try {
            InputStream in = context.getAssets().open(assetPath);
            FileOutputStream out = new FileOutputStream(localPath);
            int read;
            byte[] buffer = new byte[4096];
            while ((read = in.read(buffer)) > 0) {
                out.write(buffer, 0, read);
            }
            out.close();
            in.close();

        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
    public static Boolean execCommand(String[] command)
    {
        try {
            Runtime rt = Runtime.getRuntime();
            Process process = rt.exec("su");
            DataOutputStream os = new DataOutputStream(process.getOutputStream());
            for(int i=0; i< command.length; i++){
                os.writeBytes(command[i] + "\n");
                os.flush();
            }
            os.writeBytes("exit\n");
            os.flush();


            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            int read;
            char[] buffer = new char[4096];
            StringBuffer sb = new StringBuffer();
            while ((read = reader.read(buffer)) > 0) {
                sb.append(buffer, 0, read);
            }
            reader.close();
            String nativeOutput = sb.toString();
            Log.e("Error", "output information:" + nativeOutput);
            process.waitFor();
        } catch (IOException e) {
            return false;
        } catch (InterruptedException e) {
            return false;
        }
        return true;
    }

     public static boolean isProcessAlive(String processName){
         Runtime rt = Runtime.getRuntime();
         try {
             Process process = rt.exec("ps |grep " + processName);
             BufferedReader br = new BufferedReader(new InputStreamReader(process.getInputStream()));
             String output = br.readLine();
             if(!TextUtils.isEmpty(output)){
                 return true;
             } else {
                 return false;
             }
         } catch (IOException e) {
             e.printStackTrace();
             return false;
         }
     }

    private String intToIp(int paramInt) {
        return (paramInt & 0xFF) + "." + (0xFF & paramInt >> 8) + "." + (0xFF & paramInt >> 16) + "."
                + (0xFF & paramInt >> 24);
    }
    @Override
    protected void onDestroy() {
        stopVPN();
        super.onDestroy();
    }
}
