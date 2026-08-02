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

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Example: ensure models exist and init
        val whisperRel = "models/whisper-small.bin"
        val llamaRel = "models/llama-7b-q4.bin"

        ModelManager.ensureModelDownloaded(this, whisperRel) { ok ->
            if (ok) {
                val path = ModelManager.modelPath(this, whisperRel)
                NativeBridge.initWhisper(path)
            }
        }
        ModelManager.ensureModelDownloaded(this, llamaRel) { ok ->
            if (ok) {
                val path = ModelManager.modelPath(this, llamaRel)
                NativeBridge.initLlama(path)
            }
        }

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
                        // In a real app you'd record audio to a file and pass the file path here.
                        // For this scaffold we expect an existing WAV file in app storage for demonstration.
                        val sampleWav = "${filesDir.absolutePath}/sample_input.wav"
                        val t = NativeBridge.transcribeFile(sampleWav)
                        transcript = t ?: ""
                        val llm = NativeBridge.generate(transcript ?: "", 128)
                        response = llm ?: ""
                        NativeBridge.ttsSpeak(response ?: "")
                    } else {
                        // stop recording flow
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
