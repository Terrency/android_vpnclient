package com.lantern.launcher.vpnservice;

import android.content.Intent;
import android.net.VpnService;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;

import java.io.IOException;

import cn.limc.androidcharts.R;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private Button start;
    private Button end;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
        initData();
    }

    private void initData() {
    }

    private void initView() {
        start = (Button) findViewById(R.id.start);
        end = (Button) findViewById(R.id.end);
        start.setOnClickListener(this);
        end.setOnClickListener(this);
        Runtime runtime = Runtime.getRuntime();
        try {
            Process proc = runtime.exec("su");
        } catch (IOException e) {
            e.printStackTrace();
        }
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                CommandUtils.execute("ping -c 1 baidu.com");
            }
        });
        thread.start();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_OK) {
            Intent intent = new Intent(this, MyVpnService.class);
            startService(intent);
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()){
            case R.id.start:
                Intent intent = VpnService.prepare(this);
                if (intent != null) {
                    startActivityForResult(intent, 0);
                } else {
                    onActivityResult(0, RESULT_OK, null);
                }
                break;
            case R.id.end:
                startVPN("abcdefg");
                break;
        }
    }
    void startVPN(String name) {
        Intent i=new Intent("doenter.onevpn.ACTION_CONNECT");
        i.putExtra("name",name);
        i.putExtra("force", true);
        i.putExtra("force_same", false);
        startActivity(i);
    }

    void restartVPN(String name) {
        Intent i=new Intent("doenter.onevpn.ACTION_CONNECT");
        i.putExtra("name",name);
        i.putExtra("force", true);
        i.putExtra("force_same", true);
        startActivity(i);
    }

    void stopVPN() {
        Intent i=new Intent("doenter.onevpn.ACTION_DISCONNECT");
        // Stops any VPN regardless of name
        startActivity(i);
    }
}
