# Achim Alarm

간단한 Qt 기반 알람 애플리케이션입니다. MP3 등 로컬 오디오 파일을 알람으로 설정할 수 있고, 가능하다면 시스템 절전 모드에서도 깨우도록 RTC 알람을 요청합니다.

## 빌드 방법

이 프로젝트는 Qt 6 (Widgets, Multimedia 모듈)을 사용합니다.

```bash
cmake -S . -B build
cmake --build build
```

## 실행

빌드 후 생성된 실행 파일은 `build/achim_alarm` 입니다.

```bash
./build/achim_alarm
```

절전 모드에서 깨우기 기능은 `/sys/class/rtc/rtc0/wakealarm`에 기록할 수 있는 권한이 있어야 하며, 일부 시스템에서는 루트 권한이 필요할 수 있습니다.
