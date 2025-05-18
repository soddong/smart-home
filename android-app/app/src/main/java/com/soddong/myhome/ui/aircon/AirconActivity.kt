package com.soddong.myhome.ui.aircon

import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.soddong.myhome.R
import com.soddong.myhome.databinding.ActivityAirconBinding
import com.soddong.myhome.data.mqtt.MQTTClient
import org.json.JSONObject

class AirconActivity : AppCompatActivity() {
    private lateinit var binding: ActivityAirconBinding
    private val mqttClient = MQTTClient.getInstance()
    private var isPowerOn = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityAirconBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupInitialState()
        setupConnectionStatus()
        setupUI()
        setupMQTT()
    }

    private fun setupInitialState() {
        binding.temperatureText.text = "온도: --°C"
        binding.humidityText.text = "습도: --%"
        binding.powerStatus.text = "전원: 꺼짐"
        binding.connectionStatus.text = "연결 상태: 연결 안됨"
        enableButtons(false)
    }

    private fun setupConnectionStatus() {
        mqttClient.setConnectionCallback { isConnected ->
            runOnUiThread {
                binding.connectionStatus.text = "연결 상태: ${if (isConnected) "연결됨" else "연결 안됨"}"
                enableButtons(isConnected)
                if (!isConnected) {
                    setupInitialState()
                }
            }
        }
    }

    private fun setupUI() {
        binding.powerButton.setOnClickListener {
            isPowerOn = !isPowerOn
            val message = JSONObject().apply {
                put("power", if (isPowerOn) "on" else "off")
            }.toString()
            mqttClient.publish("aircon/control", message)
        }

        binding.tempUpButton.setOnClickListener {
            val message = JSONObject().apply {
                put("temperature", "up")
            }.toString()
            mqttClient.publish("aircon/control", message)
        }

        binding.tempDownButton.setOnClickListener {
            val message = JSONObject().apply {
                put("temperature", "down")
            }.toString()
            mqttClient.publish("aircon/control", message)
        }
    }

    private fun setupMQTT() {
        mqttClient.subscribe("aircon/status") { topic, message ->
            try {
                val json = JSONObject(message)
                updateStatus(json)
            } catch (e: Exception) {
                runOnUiThread {
                    Toast.makeText(this, "메시지 파싱 오류", Toast.LENGTH_SHORT).show()
                }
            }
        }
    }

    private fun updateStatus(json: JSONObject) {
        runOnUiThread {
            try {
                val temperature = json.getDouble("temperature")
                val humidity = json.getDouble("humidity")
                val power = json.getString("power")

                binding.temperatureText.text = "온도: ${temperature}°C"
                binding.humidityText.text = "습도: ${humidity}%"
                binding.powerStatus.text = "전원: ${if (power == "on") "켜짐" else "꺼짐"}"
                isPowerOn = power == "on"
            } catch (e: Exception) {
                Toast.makeText(this, "상태 업데이트 오류", Toast.LENGTH_SHORT).show()
            }
        }
    }

    private fun enableButtons(enable: Boolean) {
        binding.powerButton.isEnabled = enable
        binding.tempUpButton.isEnabled = enable
        binding.tempDownButton.isEnabled = enable
    }

    override fun onDestroy() {
        super.onDestroy()
        mqttClient.unsubscribe("aircon/status")
    }
} 