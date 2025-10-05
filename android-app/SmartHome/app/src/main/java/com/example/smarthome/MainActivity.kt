package com.example.smarthome

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import com.example.smarthome.ui.theme.SmartHomeTheme

import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import org.eclipse.paho.client.mqttv3.*
import java.io.InputStream
import java.security.KeyStore
import java.security.cert.CertificateFactory
import java.security.cert.X509Certificate
import javax.net.ssl.SSLContext
import javax.net.ssl.TrustManagerFactory
import javax.net.ssl.SSLSocketFactory

class MainActivity : ComponentActivity() {

    private val mqttBrokerUri = Secret.MQTT_BROKER_URI
    private val mqttUsername = Secret.MQTT_USERNAME
    private val mqttPassword = Secret.MQTT_PASSWORD
    private val mqttClientId = Secret.getMqttClientId()

    // UI
    private lateinit var tvStatus: TextView
    private lateinit var tvMessages: TextView
    private lateinit var etTopic: EditText
    private lateinit var etMessage: EditText
    private lateinit var btnConnect: Button
    private lateinit var btnSubscribe: Button
    private lateinit var btnPublish: Button

    // MQTT Client
    private var mqttClient: MqttClient? = null
    private var isConnected = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        tvStatus = findViewById(R.id.tvStatus)
        tvMessages = findViewById(R.id.tvMessages)
        etTopic = findViewById(R.id.etTopic)
        etMessage = findViewById(R.id.etMessage)
        btnConnect = findViewById(R.id.btnConnect)
        btnSubscribe = findViewById(R.id.btnSubscribe)
        btnPublish = findViewById(R.id.btnPublish)

        btnConnect.setOnClickListener { connectMqtt() }
        btnSubscribe.setOnClickListener { subscribeTopic() }
        btnPublish.setOnClickListener { publishMessage() }
    }

    private fun connectMqtt() {
        Thread {
            try {
                if (mqttClient?.isConnected == true) {
                    runOnUiThread { tvStatus.text = "이미 연결됨" }
                    return@Thread
                }

                // CleanSession true로 새로 연결
                mqttClient = MqttClient(mqttBrokerUri, mqttClientId, null)
                val options = MqttConnectOptions().apply {
                    userName = mqttUsername
                    password = mqttPassword.toCharArray()
                    socketFactory = getSocketFactoryFromPem()
                    isCleanSession = true
                }
                mqttClient?.setCallback(object : MqttCallback {
                    override fun connectionLost(cause: Throwable?) {
                        runOnUiThread {
                            tvStatus.text = "연결 끊김: ${cause?.localizedMessage}"
                            isConnected = false
                        }
                    }

                    override fun messageArrived(topic: String?, message: MqttMessage?) {
                        runOnUiThread {
                            tvMessages.append("\n[수신] $topic : ${message.toString()}")
                        }
                    }

                    override fun deliveryComplete(token: IMqttDeliveryToken?) {}
                })

                mqttClient?.connect(options)
                isConnected = true
                runOnUiThread { tvStatus.text = "연결 성공" }
            } catch (e: Exception) {
                e.printStackTrace()
                runOnUiThread { tvStatus.text = "연결 실패: ${e.localizedMessage}" }
            }
        }.start()
    }

    private fun subscribeTopic() {
        if (!isConnected) return
        val topic = etTopic.text.toString()
        Thread {
            try {
                mqttClient?.subscribe(topic, 1)
                runOnUiThread { tvStatus.text = "구독 성공: $topic" }
            } catch (e: Exception) {
                e.printStackTrace()
                runOnUiThread { tvStatus.text = "구독 실패: ${e.localizedMessage}" }
            }
        }.start()
    }

    private fun publishMessage() {
        if (!isConnected) return
        val topic = etTopic.text.toString()
        val msg = etMessage.text.toString()
        Thread {
            try {
                mqttClient?.publish(topic, msg.toByteArray(), 1, false)
                runOnUiThread { tvMessages.append("\n[발행] $topic : $msg") }
            } catch (e: Exception) {
                e.printStackTrace()
                runOnUiThread { tvStatus.text = "발행 실패: ${e.localizedMessage}" }
            }
        }.start()
    }

    // Let's Encrypt 루트 인증서 적용
    private fun getSocketFactoryFromPem(): SSLSocketFactory? {
        return try {
            val cf = CertificateFactory.getInstance("X.509")
            val caInput: InputStream = resources.openRawResource(R.raw.isrgrootx1)
            val ca = cf.generateCertificate(caInput) as X509Certificate
            caInput.close()

            val keyStore = KeyStore.getInstance(KeyStore.getDefaultType())
            keyStore.load(null, null)
            keyStore.setCertificateEntry("ca", ca)

            val tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm())
            tmf.init(keyStore)

            val context = SSLContext.getInstance("TLS")
            context.init(null, tmf.trustManagers, null)
            context.socketFactory
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }
}

@Composable
fun Greeting(name: String, modifier: Modifier = Modifier) {
    Text(
        text = "Hello $name!",
        modifier = modifier
    )
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    SmartHomeTheme {
        Greeting("Android")
    }
}