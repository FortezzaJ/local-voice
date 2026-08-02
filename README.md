# local-voice

Android app scaffold for fully offline local voice AI chat (mid-range Android target).

Features in this scaffold:
- Kotlin + Jetpack Compose UI
- Audio recording + VAD placeholder
- JNI/NDK integration stubs for whisper.cpp (STT), llama.cpp (LLM), and offline TTS
- CMake + native stubs in C++ that you can replace with real whisper/llama integration
- Instructions for downloading models to app storage (do not bundle models in APK)

See app/README section for build & model instructions.

---

TODO: Native integration (whisper.cpp, llama.cpp, TTS)

The repository contains JNI/NDK scaffolding. Next steps are to implement native shims that load ggml/whisper and ggml/llama models and expose a small C API used by the JNI bridge.

Below are example shim implementations and supporting files you can add. These are TODO templates — replace the pseudocode with the exact API calls of the Android ports you choose.

Files to add (templates)

1) app/src/main/cpp/whisper_shim.cpp

```cpp
// whisper_shim.cpp

#include <mutex>
#include <string>
#include <cstring>
#include <cstdlib>
#include <android/log.h>

#define LOG_TAG "WhisperShim"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static std::mutex g_whisper_mutex;
struct whisper_ctx_t; // replace with actual type from the port
static whisper_ctx_t* g_ctx = nullptr;

extern "C" {

bool whisper_init_from_file(const char* model_path) {
    std::lock_guard<std::mutex> lock(g_whisper_mutex);
    if (g_ctx) {
        ALOGI("Whisper already initialized");
        return true;
    }

    ALOGI("whisper_init_from_file: %s", model_path);

    // TODO: Replace this block with the exact initialization calls of your port.
    // Example pseudocode (replace with real API):
    // g_ctx = whisper_load_model_from_file(model_path);
    // if (!g_ctx) return false;

    ALOGI("TODO: Implement actual whisper model load");
    return false;
}

const char* whisper_transcribe_file(const char* wav_path) {
    std::lock_guard<std::mutex> lock(g_whisper_mutex);
    if (!g_ctx) {
        return strdup("[error: whisper model not initialized]");
    }

    ALOGI("whisper_transcribe_file: %s", wav_path);

    // TODO: Use your port's API to run transcription and return a strdup'd C string.
    // Example pseudocode:
    // std::string out;
    // bool ok = whisper_transcribe_to_string(g_ctx, wav_path, out);
    // if (!ok) return strdup("[error: transcription failed]");
    // return strdup(out.c_str());

    const char* sample = "hello from whisper shim (stub)";
    return strdup(sample);
}

void whisper_free_string(const char* p) {
    if (p) free((void*)p);
}

void whisper_free_model() {
    std::lock_guard<std::mutex> lock(g_whisper_mutex);
    if (g_ctx) {
        // TODO: free underlying context properly
        // whisper_free(g_ctx);
        g_ctx = nullptr;
    }
}

} // extern "C"
```

2) app/src/main/cpp/llama_shim.cpp

```cpp
// llama_shim.cpp

#include <mutex>
#include <string>
#include <cstring>
#include <cstdlib>
#include <android/log.h>

#define LOG_TAG "LlamaShim"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static std::mutex g_llama_mutex;
struct llama_ctx_t; // replace with actual type from the port
static llama_ctx_t* g_llama = nullptr;

extern "C" {

bool llama_init_from_file(const char* model_path) {
    std::lock_guard<std::mutex> lock(g_llama_mutex);
    if (g_llama) {
        ALOGI("Llama already initialized");
        return true;
    }

    ALOGI("llama_init_from_file: %s", model_path);

    // TODO: Replace this block with the actual model loading calls for your port.
    // Example pseudocode:
    // g_llama = llama_model_load(model_path);
    // return g_llama != nullptr;

    ALOGI("TODO: Implement actual llama model load");
    return false;
}

const char* llama_generate(const char* prompt, int max_tokens) {
    std::lock_guard<std::mutex> lock(g_llama_mutex);
    if (!g_llama) {
        return strdup("[error: llama model not initialized]");
    }

    ALOGI("llama_generate prompt: %s", prompt);

    // TODO: call the actual llama generation API and return strdup'd string
    std::string out = std::string("[LLM stub reply] I heard: ") + prompt;
    return strdup(out.c_str());
}

void llama_free_string(const char* p) {
    if (p) free((void*)p);
}

void llama_free_model() {
    std::lock_guard<std::mutex> lock(g_llama_mutex);
    if (g_llama) {
        // TODO: free model resources
        // llama_free(g_llama);
        g_llama = nullptr;
    }
}

} // extern "C"
```

3) app/src/main/cpp/CMakeLists.txt (updates)

```cmake
cmake_minimum_required(VERSION 3.10)
project(localvoice-native)

set(CMAKE_CXX_STANDARD 17)

# Build the JNI bridge and shims into localvoice.so
add_library(localvoice SHARED 
    native-bridge.cpp
    native-lib.cpp
    whisper_shim.cpp
    llama_shim.cpp
)

find_library(log-lib log)
find_library(dl-lib dl)

target_include_directories(localvoice PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(localvoice ${log-lib} ${dl-lib})

# If you want to link prebuilt libwhisper.so / libllama.so from jniLibs at configure time,
# this will try to link them if they exist for the current ANDROID_ABI.
if(DEFINED ANDROID_ABI)
    set(PREBUILT_DIR ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})
    if(EXISTS ${PREBUILT_DIR}/libwhisper.so)
        message(STATUS "Found prebuilt libwhisper.so for ${ANDROID_ABI}")
        target_link_libraries(localvoice ${PREBUILT_DIR}/libwhisper.so)
    endif()
    if(EXISTS ${PREBUILT_DIR}/libllama.so)
        message(STATUS "Found prebuilt libllama.so for ${ANDROID_ABI}")
        target_link_libraries(localvoice ${PREBUILT_DIR}/libllama.so)
    endif()
endif()

# Optional: add third_party/whisper or third_party/llama if you include sources in-tree
if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/whisper/CMakeLists.txt)
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/whisper)
    target_link_libraries(localvoice whisper)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/llama/CMakeLists.txt)
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/llama)
    target_link_libraries(localvoice llama)
endif()
```

4) scripts/build_native.sh (helper)

```bash
#!/usr/bin/env bash
set -euo pipefail

# scripts/build_native.sh
# Usage: scripts/build_native.sh <ANDROID_NDK_PATH> <ANDROID_ABI> <BUILD_DIR>

NDK_PATH=${1:-$ANDROID_NDK_ROOT}
ABI=${2:-arm64-v8a}
BUILD_DIR=${3:-build-${ABI}}
API_LEVEL=${4:-24}

if [ -z "$NDK_PATH" ]; then
  echo "ERROR: ANDROID_NDK_ROOT or first arg must point to your NDK"
  exit 1
fi

mkdir -p $BUILD_DIR
pushd $BUILD_DIR
cmake -DCMAKE_TOOLCHAIN_FILE=${NDK_PATH}/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=${ABI} \
  -DANDROID_NATIVE_API_LEVEL=${API_LEVEL} \
  -DANDROID_NDK=${NDK_PATH} \
  -DCMAKE_BUILD_TYPE=Release \
  ..

cmake --build . -- -j$(nproc)
popd

echo "Build finished. If you built libwhisper/libllama, copy resulting .so files into app/src/main/jniLibs/<abi>/" 
```

5) .github/workflows/android-native-build.yml (CI)

```yaml
name: Build native libs (NDK)

on:
  workflow_dispatch:
  push:
    branches:
      - init-scaffold

jobs:
  build-native:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        abi: [arm64-v8a, armeabi-v7a]
    env:
      ANDROID_NDK_VERSION: 25.2.9519653
    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Setup JDK
        uses: actions/setup-java@v4
        with:
          distribution: temurin
          java-version: '17'

      - name: Install Android SDK and NDK
        uses: android-actions/setup-ndk@v2
        with:
          ndk-version: ${{ env.ANDROID_NDK_VERSION }}

      - name: Show NDK info
        run: |
          echo "NDK_HOME=${NDK_HOME:-$ANDROID_NDK_HOME}"
          ls -la ${NDK_HOME:-$ANDROID_NDK_HOME}

      - name: Make build script executable
        run: chmod +x scripts/build_native.sh

      - name: Build native for ABI
        run: |
          ./scripts/build_native.sh ${NDK_HOME:-$ANDROID_NDK_HOME} ${{ matrix.abi }} build-${{ matrix.abi }}

      - name: Upload built libs
        if: success()
        uses: actions/upload-artifact@v4
        with:
          name: native-libs-${{ matrix.abi }}
          path: |
            build-${{ matrix.abi }}/lib*/*.so
```

Notes and next steps

- These templates are intentionally conservative. Replace the TODO/pseudocode blocks with the exact API calls and types used by your chosen Android ports of whisper.cpp and llama.cpp (function names and types vary by port).
- Recommended flow:
  1. Convert models to ggml quantized formats on desktop and host them for the app to download, or sideload them into device under filesDir/models/.
  2. Prefer tiny/small whisper models and quantized 7B llama models (q4/q5 quantizations) for mid-range devices.
  3. Test with sample WAV (16-bit PCM, mono) saved to filesDir/sample_input.wav.
  4. Use the included CI workflow as a starting point to cross-compile native .so artifacts for arm64-v8a and armeabi-v7a; artifacts will be uploaded for manual download.

If you want, I can commit these template files into branch init-scaffold for you (they are prepared). Otherwise, you can copy the templates into the repo and implement the TODOs locally.
