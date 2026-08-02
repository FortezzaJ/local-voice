package com.jfortezza.localvoice

object NativeBridge {
    init {
        System.loadLibrary("localvoice")
    }

    external fun initWhisper(modelPath: String): Boolean
    external fun transcribeFile(wavPath: String): String?

    external fun initLlama(modelPath: String): Boolean
    external fun generate(prompt: String, maxTokens: Int = 128): String?

    external fun ttsSpeak(text: String)
}
