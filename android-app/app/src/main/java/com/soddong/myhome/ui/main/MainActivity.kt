package com.soddong.myhome.ui.main

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.soddong.myhome.databinding.ActivityMainBinding
import com.soddong.myhome.ui.aircon.AirconActivity
import com.soddong.myhome.data.mqtt.MQTTClient

class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding
    private lateinit var mqttClient: MQTTClient

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        mqttClient = MQTTClient.getInstance()
        setupUI()
    }

    private fun setupUI() {
        binding.airconButton.setOnClickListener {
            startActivity(Intent(this, AirconActivity::class.java))
        }

        // TODO: Add other device button click listeners
        binding.tvButton.setOnClickListener {
            // TODO: Implement TV control
        }

        binding.lightButton.setOnClickListener {
            // TODO: Implement light control
        }

        binding.speakerButton.setOnClickListener {
            // TODO: Implement speaker control
        }
    }
}