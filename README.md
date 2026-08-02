# local-voice

Android app scaffold for fully offline local voice AI chat (mid-range Android target).

Features in this scaffold:
- Kotlin + Jetpack Compose UI
- Audio recording + VAD placeholder
- JNI/NDK integration stubs for whisper.cpp (STT), llama.cpp (LLM), and offline TTS
- CMake + native stubs in C++ that you can replace with real whisper/llama integration
- Instructions for downloading models to app storage (do not bundle models in APK)

See app/README section for build & model instructions.
