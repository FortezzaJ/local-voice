package com.jfortezza.localvoice

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.material.Button
import androidx.compose.material.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.core.content.ContextCompat

class MainActivity : ComponentActivity() {

    // Native functions (stubs): implement in C++ and wire to whisper.cpp / llama.cpp
    external fun initNative(): Boolean
    external fun startSTT(): String?
    external fun stopSTT(): Boolean
    external fun sendToLLM(input: String): String?
    external fun ttsSpeak(text: String)

    companion object {
        init {
            System.loadLibrary("localvoice")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // init native libs
        initNative()

        val requestPermissionLauncher = registerForActivityResult(
            ActivityResultContracts.RequestPermission()
        ) { isGranted: Boolean ->
            // handle permission result
        }

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            requestPermissionLauncher.launch(Manifest.permission.RECORD_AUDIO)
        }

        setContent {
            var recording by remember { mutableStateOf(false) }
            var transcript by remember { mutableStateOf("") }
            var response by remember { mutableStateOf("") }

            Column(
                modifier = Modifier.fillMaxSize().padding(24.dp),
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.Center
            ) {
                Text(text = "Local Voice AI Chat")
                Spacer(modifier = Modifier.height(12.dp))
                Button(onClick = {
                    recording = !recording
                    if (recording) {
                        // start STT in native
                        val t = startSTT()
                        transcript = t ?: ""
                        // send to LLM
                        val llm = sendToLLM(transcript)
                        response = llm ?: ""
                        // speak response
                        ttsSpeak(response)
                    } else {
                        stopSTT()
                    }
                }) {
                    Text(if (recording) "Stop" else "Record & Send")
                }

                Spacer(modifier = Modifier.height(16.dp))
                Text(text = "Transcript: $transcript")
                Spacer(modifier = Modifier.height(8.dp))
                Text(text = "LLM Response: $response")
            }
        }
    }
}
