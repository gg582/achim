# Achim Alarm

간단한 Qt 기반 알람 애플리케이션입니다. MP3 등 로컬 오디오 파일을 알람으로 설정할 수 있고, 가능하다면 시스템 절전 모드에서도 깨우도록 RTC 알람을 요청합니다. Qt Multimedia를 사용하여 알람 사운드를 반복 재생하므로 Windows, macOS, Linux 어디서든 동일한 경험을 제공합니다.

## 빌드 방법

이 프로젝트는 Qt 6 (Widgets, Multimedia 모듈)을 사용합니다.

```bash
cmake -S . -B build
cmake --build build
```

빌드 후 테스트 실행 파일은 `build/achim_alarm` (Windows에서는 `build/achim_alarm.exe`) 입니다.

## 실행

빌드 후 생성된 실행 파일은 `build/achim_alarm` 입니다.

```bash
./build/achim_alarm
```

절전 모드에서 깨우기 기능은 `/sys/class/rtc/rtc0/wakealarm`에 기록할 수 있는 권한이 있어야 하며, 일부 시스템에서는 루트 권한이 필요할 수 있습니다.

## 설치 스크립트

플랫폼별 의존성 배포 도구(Qt 제공)를 활용하는 간단한 스크립트를 제공합니다. 스크립트는 기본적으로 `dist/<platform>` 폴더에 실행 파일/앱 번들을 모아 둡니다.

### Linux

```bash
./install.sh
```

`install.sh`는 CMake를 이용해 프로젝트를 빌드하고 기본적으로 `/usr/local` 아래에 설치합니다. sudo 권한이 필요할 수 있습니다.

### Windows (PowerShell)

```powershell
pwsh -ExecutionPolicy Bypass -File scripts/install_windows.ps1
```

스크립트는 Release 빌드를 수행하고 `windeployqt`가 PATH에 있으면 필요한 Qt 런타임을 함께 배포합니다. 결과물은 `dist\windows`에 저장됩니다.

### macOS

```bash
./scripts/install_macos.sh
```

스크립트는 Release 빌드 후 `macdeployqt`가 발견되면 실행하여 `.app` 번들을 완성하고 `dist/macos`에 복사합니다.

### Android

```bash
./scripts/package_android.sh
```

`ANDROID_DEPLOY_JSON` 환경 변수를 사용하여 `androiddeployqt` 입력 JSON 경로를 지정하면 자동으로 APK 번들을 갱신합니다. 스크립트는 항상 `dist/android` 아래에 트레이 아이콘을 포함한 리소스를 준비합니다.

### iOS

```bash
./scripts/package_ios.sh
```

스크립트는 `dist/ios` 아래에 트레이 아이콘을 포함한 Asset Catalog(`TrayIcon.imageset`)를 생성합니다. Xcode 프로젝트에 추가하여 사용할 수 있습니다.
