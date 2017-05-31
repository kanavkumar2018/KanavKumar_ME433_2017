package com.example.kanavkumar.hw14_android;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import android.view.View;
import android.widget.Button;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;


import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;



public class MainActivity extends AppCompatActivity {

    SeekBar myControl;
    TextView myTextView;
    Button button;
    TextView myTextView2;
    ScrollView myScrollView;
    TextView myTextView3;

    private void setMyControlListener() {
        myControl.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            int progressChanged = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                myTextView.setText("The value is: "+ progress);

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }

        });
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        myControl = (SeekBar) findViewById(R.id.seek1);

        myTextView = (TextView) findViewById(R.id.textView01);
        myTextView.setText("Hello World!");
        setMyControlListener();

        myTextView2 = (TextView) findViewById(R.id.textView02);
        myScrollView = (ScrollView) findViewById(R.id.ScrollView01);
        myTextView3 = (TextView) findViewById(R.id.textView03);
        button = (Button) findViewById(R.id.button1);

        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                myTextView2.setText("value on click is "+myControl.getProgress());
            }
        });




    }



}
