# libjgame on Android

This folder is an Android project (Gradle) that builds a game made with
libjgame into an APK. It is the same for every game: what changes from one to
the next is in two files of the game's own, so a game needs no Android project
of its own and nothing here is copied.

The APK holds SDL2 (`libSDL2.so`, built from SDL's sources), the game with
libjgame (`libmain.so`) and SDL's Java side, whose `SDLActivity` starts the
game's `main()`.

## What the game needs

1. **`main()` in a file that includes `<jgame/Main.h>`**: on Android SDL calls
   it `SDL_main()` and runs it from `libmain.so`.

2. **A `CMakeLists.txt` at its top** that, on Android, makes a shared library
   called `main` instead of the executable, linked with `jgame`:

   ```cmake
   cmake_minimum_required(VERSION 3.16)
   project(mygame LANGUAGES CXX)
   add_subdirectory(libjgame)
   if(ANDROID)
       set(TARGET main)
       add_library(main SHARED ${SOURCES})
   else()
       set(TARGET mygame)
       add_executable(mygame ${SOURCES})
   endif()
   target_link_libraries(${TARGET} PRIVATE jgame)
   ```

   (SDL2 is already a target when the Android build gets there: libjgame's
   CMake uses it.)

3. **A `jgame-android.properties` at its top**, next to the `CMakeLists.txt`:

   ```properties
   # Required
   applicationId=com.example.mygame
   appName=My Game
   # Optional (the defaults are shown)
   versionCode=1
   versionName=1.0
   # landscape, sensorLandscape, portrait, sensorPortrait, fullSensor...
   orientation=sensorLandscape
   minSdk=21
   abis=armeabi-v7a arm64-v8a x86_64
   # Definitions for the game's CMake
   cmakeArguments=
   # Folders of the game, from its top: files shipped in the APK (read them
   # with SDL_RWFromFile(): there is no folder of them on the phone), and
   # Android resources (an icon: mipmap-*/ic_launcher.png)
   assets=
   res=
   ```

## Building

You need the Android SDK with the NDK (Android Studio installs both; on its
own: the command line tools and `sdkmanager "platforms;android-35"
"build-tools;35.0.0" "ndk;27.2.12479018" "cmake;3.22.1"`), Java 17 and
`ANDROID_HOME` set (or a `local.properties` here with `sdk.dir=...`).

```sh
cd libjgame/android
./get-sdl.sh                 # SDL2's sources, once (into SDL/)
./gradlew assembleDebug      # app/build/outputs/apk/debug/app-debug.apk
./gradlew installDebug       # onto the phone plugged in (USB debugging on)
```

By default the game is the folder two levels up (`../..`): the right place
when libjgame is a submodule at the top of the game's repository. Otherwise
give it: `./gradlew assembleDebug -Pjgame.game=/path/to/mygame`.

Android Studio can open this folder as it is.

A release APK (`./gradlew assembleRelease`) is signed with the debug key,
which is enough to try it on a phone; to publish it, give it a key of your own
(a `signingConfigs` block in `app/build.gradle`).

## What libjgame does on Android

- **Touch screen**: `TouchControls` draws a stick and buttons over the game
  and presses keys with them (see `TouchControls.h`); a game has to call
  `TouchControls::Draw()` each frame and choose a layout
  (`UseDefaultLayout()` gives the buttons of a game controller).
- **Back key**: it is Escape (`Keyboard::KEY_ESCAPE`) and doesn't close the
  program.
- **The background**: `Events::WentToBackground()` says when to pause and save
  (Android may end a program in the background without warning).
- **Files**: `Platform::UserDataDirectory()` is the folder the program can
  write in (there is no home folder, and the working directory is `/`).
- **The screen**: the window is always the whole screen
  (`GraphicsEngine::CanToggleFullscreen()` is false), and it turns only between
  the orientations that suit the video mode.
- **Game controllers** work as on a computer (`Gamepad.h`).

Files read with `FileStream` (plain `fopen()`) must be in a folder of the
phone, such as `Platform::UserDataDirectory()`: the APK's assets are not one.
A game whose data are files can embed them in the program instead.
