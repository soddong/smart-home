package com.soddong.myhome

import android.content.Intent
import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.button.MaterialButton

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // 에어컨 버튼 클릭 이벤트
        findViewById<MaterialButton>(R.id.airconButton).setOnClickListener {
            startActivity(Intent(this, AirconActivity::class.java))
        }

        // TV 버튼 클릭 이벤트
        findViewById<MaterialButton>(R.id.tvButton).setOnClickListener {
            // TODO: TV 액티비티 구현
        }

        // 조명 버튼 클릭 이벤트
        findViewById<MaterialButton>(R.id.lightButton).setOnClickListener {
            // TODO: 조명 액티비티 구현
        }

        // 스피커 버튼 클릭 이벤트
        findViewById<MaterialButton>(R.id.speakerButton).setOnClickListener {
            // TODO: 스피커 액티비티 구현
        }
    }
} 