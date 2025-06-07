package com.example.mqttclient;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;

import androidx.appcompat.app.AppCompatActivity;

import java.util.UUID;

public class MainActivity extends AppCompatActivity {
    MqttHelper mqttHelper;
    private final String serverUri = "soddong.duckdns.org:8883";
    private final String topic = "Starry/Pub";

    @SuppressLint("ResourceType")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mqttHelper = new MqttHelper(getApplicationContext(), serverUri, UUID.randomUUID().toString(), topic);

        EditText editText = findViewById(R.id.messageInput);

        Button sendBtn = findViewById(R.id.sendButton);
        sendBtn.setOnClickListener(v -> {
            String msg = String.valueOf(editText.getText());
            mqttHelper.publish(topic, msg);
        });
    }
}

