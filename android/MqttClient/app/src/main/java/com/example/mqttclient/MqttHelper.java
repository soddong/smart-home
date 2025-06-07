package com.example.mqttclient;

import android.content.Context;
import android.util.Log;

import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.io.InputStream;
import java.security.KeyStore;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManagerFactory;

public class MqttHelper {
    private MqttAndroidClient client;
    private Context context;

    public MqttHelper(Context context, String serverUri, String clientId, String topic) {
        this.context = context;
        client = new MqttAndroidClient(context, serverUri, clientId);

        try {
            MqttConnectOptions options = new MqttConnectOptions();
            options.setCleanSession(true);
            options.setUserName("soddong");
            options.setPassword("earcuteb".toCharArray());

            try {
                options.setSocketFactory(getSocketFactory(this.context.getResources().openRawResource(R.raw.ca)));
            } catch (Exception e) {
                Log.e("MQTT", "Error setting SSL Socket Factory", e);
            }

            client.connect(options, null, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    subscribeToTopic(topic);
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.d("MQTT", "Connection failed: " + exception.toString());
                }
            });

            client.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    Log.d("MQTT", "Connection lost");
                }

                @Override
                public void messageArrived(String topic, MqttMessage message) {
                    Log.d("MQTT", "Message arrived: " + message.toString());
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    Log.d("MQTT", "Delivery complete");
                }
            });

        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    private void subscribeToTopic(String topic) {
        try {
            client.subscribe(topic, 1);
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public void publish(String topic, String msg) {
        try {
            client.publish(topic, new MqttMessage(msg.getBytes()));
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    // SSLSocketFactory를 생성하는 헬퍼 메소드
    private static SSLSocketFactory getSocketFactory(InputStream caCertInputStream) throws Exception {
        // CA 인증서 로드
        CertificateFactory cf = CertificateFactory.getInstance("X.509");
        X509Certificate caCert = (X509Certificate) cf.generateCertificate(caCertInputStream);
        caCertInputStream.close();

        // TrustStore 생성 및 CA 인증서 추가
        KeyStore trustStore = KeyStore.getInstance(KeyStore.getDefaultType());
        trustStore.load(null, null); // 빈 키스토어 초기화
        trustStore.setCertificateEntry("ca", caCert); // "ca"는 별칭

        // TrustManagerFactory 초기화
        TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        tmf.init(trustStore);

        // SSLContext 초기화
        SSLContext sslContext = SSLContext.getInstance("TLS"); // 또는 "SSL"
        sslContext.init(null, tmf.getTrustManagers(), null); // 클라이언트 인증서가 없으므로 첫 번째 인자는 null

        return sslContext.getSocketFactory();
    }
}