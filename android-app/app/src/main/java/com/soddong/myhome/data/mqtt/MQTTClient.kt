package com.soddong.myhome.data.mqtt

import android.content.Context
import org.eclipse.paho.client.mqttv3.*
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence

class MQTTClient private constructor() {
    private var mqttClient: MqttClient? = null
    private val BROKER_URL = "tcp://192.168.0.10:1883"
    private val CLIENT_ID = "AndroidClient"
    private var isConnected = false
    private var connectionCallback: ((Boolean) -> Unit)? = null

    companion object {
        @Volatile
        private var instance: MQTTClient? = null

        fun getInstance(): MQTTClient {
            return instance ?: synchronized(this) {
                instance ?: MQTTClient().also { instance = it }
            }
        }
    }

    fun setConnectionCallback(callback: (Boolean) -> Unit) {
        connectionCallback = callback
        callback(isConnected)
    }

    fun connect(onConnected: () -> Unit, onError: (String) -> Unit) {
        try {
            if (mqttClient == null) {
                mqttClient = MqttClient(BROKER_URL, CLIENT_ID, MemoryPersistence())
            }
            
            val options = MqttConnectOptions().apply {
                isCleanSession = true
                connectionTimeout = 10
                keepAliveInterval = 60
            }

            mqttClient?.setCallback(object : MqttCallback {
                override fun connectionLost(cause: Throwable?) {
                    isConnected = false
                    connectionCallback?.invoke(false)
                }

                override fun messageArrived(topic: String?, message: MqttMessage?) {
                    // 메시지 수신 처리
                }

                override fun deliveryComplete(token: IMqttDeliveryToken?) {
                    // 메시지 전송 완료 처리
                }
            })

            mqttClient?.connect(options)
            isConnected = true
            connectionCallback?.invoke(true)
            onConnected()
        } catch (e: MqttException) {
            isConnected = false
            connectionCallback?.invoke(false)
            onError(e.message ?: "Unknown error")
        }
    }

    fun disconnect() {
        try {
            mqttClient?.disconnect()
            isConnected = false
            connectionCallback?.invoke(false)
        } catch (e: MqttException) {
            e.printStackTrace()
        }
    }

    fun publish(topic: String, message: String) {
        if (!isConnected) return
        try {
            mqttClient?.publish(topic, MqttMessage(message.toByteArray()))
        } catch (e: MqttException) {
            e.printStackTrace()
        }
    }

    fun subscribe(topic: String, callback: (String, String) -> Unit) {
        if (!isConnected) return
        try {
            mqttClient?.subscribe(topic) { _, message ->
                callback(topic, String(message.payload))
            }
        } catch (e: MqttException) {
            e.printStackTrace()
        }
    }

    fun unsubscribe(topic: String) {
        if (!isConnected) return
        try {
            mqttClient?.unsubscribe(topic)
        } catch (e: MqttException) {
            e.printStackTrace()
        }
    }

    fun isConnected(): Boolean = isConnected
} 