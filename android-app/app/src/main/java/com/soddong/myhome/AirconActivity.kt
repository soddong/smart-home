package com.soddong.myhome

import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.button.MaterialButton

class AirconActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_aircon)

        // 온도 조절 버튼 클릭 이벤트
        findViewById<MaterialButton>(R.id.tempUpButton).setOnClickListener {
            // TODO: 온도 올리기 구현
        }

        findViewById<MaterialButton>(R.id.tempDownButton).setOnClickListener {
            // TODO: 온도 내리기 구현
        }

        // 전원 버튼 클릭 이벤트
        findViewById<MaterialButton>(R.id.powerButton).setOnClickListener {
            // TODO: 전원 켜기/끄기 구현
        }
    }
} 