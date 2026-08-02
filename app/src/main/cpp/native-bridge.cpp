#include <jni.h>
#include <string>
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "LocalVoiceBridge"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static void* whisperHandle = nullptr;
static void* llamaHandle = nullptr;

// We try to locate helper functions in the native libraries. These are optional shims
// that you should provide when building whisper/llama for Android. The wrapper will
// attempt to find a transcribe/generate helper symbol and call it.

typedef bool (*whisper_init_fn)(const char* model_path);
typedef const char* (*whisper_transcribe_file_fn)(const char* wav_path);

typedef bool (*llama_init_fn)(const char* model_path);
typedef const char* (*llama_generate_fn)(const char* prompt, int max_tokens);

static whisper_init_fn whisper_init_ptr = nullptr;
static whisper_transcribe_file_fn whisper_transcribe_file_ptr = nullptr;

static llama_init_fn llama_init_ptr = nullptr;
static llama_generate_fn llama_generate_ptr = nullptr;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_jfortezza_localvoice_NativeBridge_initWhisper(JNIEnv* env, jclass /*cls*/, jstring modelPath) {
    const char* model_path_c = env->GetStringUTFChars(modelPath, nullptr);
    ALOGI("initWhisper requested for %s", model_path_c);

    // Try to load libwhisper.so from app's lib path
    if (!whisperHandle) {
        whisperHandle = dlopen("/data/data/com.jfortezza.localvoice/lib/libwhisper.so", RTLD_NOW | RTLD_LOCAL);
        if (!whisperHandle) {
            // Try system library name
            whisperHandle = dlopen("libwhisper.so", RTLD_NOW | RTLD_LOCAL);
        }
        if (!whisperHandle) {
            ALOGE("Failed to dlopen libwhisper.so: %s", dlerror());
            env->ReleaseStringUTFChars(modelPath, model_path_c);
            return JNI_FALSE;
        }
    }

    // Resolve expected symbols (these are shims you should provide in the native lib)
    if (!whisper_init_ptr) {
        whisper_init_ptr = (whisper_init_fn)dlsym(whisperHandle, "whisper_init_from_file");
        if (!whisper_init_ptr) {
            // try alternative shim name
            whisper_init_ptr = (whisper_init_fn)dlsym(whisperHandle, "whisper_init");
        }
    }

    if (!whisper_transcribe_file_ptr) {
        whisper_transcribe_file_ptr = (whisper_transcribe_file_fn)dlsym(whisperHandle, "whisper_transcribe_file");
    }

    if (!whisper_init_ptr) {
        ALOGE("Unable to find whisper_init symbol in libwhisper.so");
        env->ReleaseStringUTFChars(modelPath, model_path_c);
        return JNI_FALSE;
    }

    bool ok = whisper_init_ptr(model_path_c);
    env->ReleaseStringUTFChars(modelPath, model_path_c);

    ALOGI("initWhisper -> %d", ok);
    return ok ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_jfortezza_localvoice_NativeBridge_transcribeFile(JNIEnv* env, jclass /*cls*/, jstring wavPath) {
    const char* wav_c = env->GetStringUTFChars(wavPath, nullptr);
    ALOGI("transcribeFile %s", wav_c);

    if (!whisper_transcribe_file_ptr) {
        ALOGE("whisper_transcribe_file symbol not found in libwhisper.so");
        env->ReleaseStringUTFChars(wavPath, wav_c);
        return env->NewStringUTF("[error: no transcribe symbol]");
    }

    const char* out = whisper_transcribe_file_ptr(wav_c);
    std::string result = out ? out : "";

    // Assume the native side allocated the returned string; if so, it should also
    // expose a free function. This is a contract you must implement in your native shim.

    env->ReleaseStringUTFChars(wavPath, wav_c);
    return env->NewStringUTF(result.c_str());
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_jfortezza_localvoice_NativeBridge_initLlama(JNIEnv* env, jclass /*cls*/, jstring modelPath) {
    const char* model_path_c = env->GetStringUTFChars(modelPath, nullptr);
    ALOGI("initLlama requested for %s", model_path_c);

    if (!llamaHandle) {
        llamaHandle = dlopen("/data/data/com.jfortezza.localvoice/lib/libllama.so", RTLD_NOW | RTLD_LOCAL);
        if (!llamaHandle) {
            llamaHandle = dlopen("libllama.so", RTLD_NOW | RTLD_LOCAL);
        }
        if (!llamaHandle) {
            ALOGE("Failed to dlopen libllama.so: %s", dlerror());
            env->ReleaseStringUTFChars(modelPath, model_path_c);
            return JNI_FALSE;
        }
    }

    if (!llama_init_ptr) {
        llama_init_ptr = (llama_init_fn)dlsym(llamaHandle, "llama_init_from_file");
        if (!llama_init_ptr) {
            llama_init_ptr = (llama_init_fn)dlsym(llamaHandle, "llama_init");
        }
    }

    if (!llama_generate_ptr) {
        llama_generate_ptr = (llama_generate_fn)dlsym(llamaHandle, "llama_generate");
    }

    if (!llama_init_ptr) {
        ALOGE("Unable to find llama_init symbol in libllama.so");
        env->ReleaseStringUTFChars(modelPath, model_path_c);
        return JNI_FALSE;
    }

    bool ok = llama_init_ptr(model_path_c);
    env->ReleaseStringUTFChars(modelPath, model_path_c);

    ALOGI("initLlama -> %d", ok);
    return ok ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_jfortezza_localvoice_NativeBridge_generate(JNIEnv* env, jclass /*cls*/, jstring prompt, jint maxTokens) {
    const char* prompt_c = env->GetStringUTFChars(prompt, nullptr);
    ALOGI("generate prompt: %s", prompt_c);

    if (!llama_generate_ptr) {
        ALOGE("llama_generate symbol not found in libllama.so");
        env->ReleaseStringUTFChars(prompt, prompt_c);
        return env->NewStringUTF("[error: no generate symbol]");
    }

    const char* out = llama_generate_ptr(prompt_c, (int)maxTokens);
    std::string result = out ? out : "";

    // As with whisper, the native llama shim should manage allocation and free of returned strings.

    env->ReleaseStringUTFChars(prompt, prompt_c);
    return env->NewStringUTF(result.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_jfortezza_localvoice_NativeBridge_ttsSpeak(JNIEnv* env, jclass /*cls*/, jstring text) {
    const char* t = env->GetStringUTFChars(text, nullptr);
    ALOGI("ttsSpeak (bridge) %s", t);
    // Option: call an integrated native TTS lib via dlopen or call back into Java
    // For now, we leave this as a no-op. Implement integration with Flite/Coqui here.
    env->ReleaseStringUTFChars(text, t);
}
