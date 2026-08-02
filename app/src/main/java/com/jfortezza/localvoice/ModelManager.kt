package com.jfortezza.localvoice

import android.content.Context
import java.io.File

object ModelManager {
    fun modelExists(context: Context, relativePath: String): Boolean {
        val f = File(context.filesDir, relativePath)
        return f.exists()
    }

    fun modelPath(context: Context, relativePath: String): String {
        val f = File(context.filesDir, relativePath)
        return f.absolutePath
    }

    // Placeholder downloader - implement your preferred downloading method (WorkManager/OkHttp)
    fun ensureModelDownloaded(context: Context, relativePath: String, onComplete: (Boolean) -> Unit) {
        // TODO: implement background download and checksum verification.
        onComplete(Boolean(modelExists(context, relativePath)))
    }
}
