#include <jni.h>
#include <string>
#include <android/log.h>

#define LOG_TAG "LocalVoiceNative"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT jboolean JNICALL
Java_com_jfortezza_localvoice_MainActivity_initNative(JNIEnv* env, jobject /* this */) {
    ALOGI("initNative called - initialize models here (whisper/llama)");
    // Initialize whisper/llama runtime here (load models from app storage)
    return JNI_TRUE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_jfortezza_localvoice_MainActivity_startSTT(JNIEnv* env, jobject /* this */) {
    ALOGI("startSTT called - run STT and return a transcript (stub)");
    std::string transcript = "hello from stub stt";
    return env->NewStringUTF(transcript.c_str());
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_jfortezza_localvoice_MainActivity_stopSTT(JNIEnv* env, jobject /* this */) {
    ALOGI("stopSTT called");
    return JNI_TRUE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_jfortezza_localvoice_MainActivity_sendToLLM(JNIEnv* env, jobject /* this */, jstring input) {
    const char* in = env->GetStringUTFChars(input, JNI_FALSE);
    ALOGI("sendToLLM called with: %s", in);
    // TODO: call llama.cpp runtime with `in` and return generated text
    std::string out = std::string("[LLM reply stub] I heard: ") + in;
    env->ReleaseStringUTFChars(input, in);
    return env->NewStringUTF(out.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_jfortezza_localvoice_MainActivity_ttsSpeak(JNIEnv* env, jobject /* this */, jstring text) {
    const char* t = env->GetStringUTFChars(text, JNI_FALSE);
    ALOGI("ttsSpeak called with: %s", t);
    // Stub: integrate offline TTS (e.g., Flite, Coqui TTS) here via native or call Android TTS from Java
    env->ReleaseStringUTFChars(text, t);
}
