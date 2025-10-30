#include "alarmwindow.h"

#include <QAbstractSpinBox>
#include <QAction>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAudio>
#endif
#include <QAudioOutput>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QFont>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLocale>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QMenu>
#include <QRandomGenerator>
#include <QPushButton>
#include <QStringList>
#include <QTimeEdit>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QDir>
#include <QSystemTrayIcon>
#include <QtGlobal>
#include <QApplication>
#include <QCoreApplication>

#include <algorithm>
#include <cmath>
#include <random>

namespace {

struct LanguageStrings {
    QLocale::Language language;
    QHash<QString, QString> values;
};

const LanguageStrings englishStrings = {
    QLocale::English,
    {
        {QStringLiteral("app_title"), QStringLiteral("Achim Alarm")},
        {QStringLiteral("app_heading"), QStringLiteral("Achim Alarm")},
        {QStringLiteral("date_label"), QStringLiteral("Alarm date")},
        {QStringLiteral("repeat_group"), QStringLiteral("Weekly repeat")},
        {QStringLiteral("repeat_hint"), QStringLiteral("The alarm repeats on selected weekdays. Leave empty for a one-time alarm on the chosen date.")},
        {QStringLiteral("pick_sound_button"), QStringLiteral("Choose alarm sound")},
        {QStringLiteral("no_sound_selected"), QStringLiteral("No file selected")},
        {QStringLiteral("wake_checkbox"), QStringLiteral("Attempt wake from sleep")},
        {QStringLiteral("set_button"), QStringLiteral("Set alarm")},
        {QStringLiteral("cancel_button"), QStringLiteral("Cancel")},
        {QStringLiteral("status_playing"), QStringLiteral("Alarm is ringing.")},
        {QStringLiteral("status_time_remaining"), QStringLiteral("%1 remaining until the alarm.")},
        {QStringLiteral("status_scheduled_time"), QStringLiteral("Scheduled time: %1")},
        {QStringLiteral("status_wake_active"), QStringLiteral("Wake from sleep request is active.")},
        {QStringLiteral("status_none"), QStringLiteral("No alarm scheduled.")},
        {QStringLiteral("sound_required_title"), QStringLiteral("Sound required")},
        {QStringLiteral("sound_required_body"), QStringLiteral("Please choose an alarm sound first.")},
        {QStringLiteral("sound_missing_title"), QStringLiteral("Sound missing")},
        {QStringLiteral("sound_missing_body"), QStringLiteral("The selected sound file could not be found. Please choose a new file.")},
        {QStringLiteral("repeat_failure_title"), QStringLiteral("Repeat scheduling failed")},
        {QStringLiteral("repeat_failure_body"), QStringLiteral("Could not compute the next alarm time for the chosen weekdays.")},
        {QStringLiteral("time_error_title"), QStringLiteral("Time error")},
        {QStringLiteral("time_error_body"), QStringLiteral("Please check the alarm time.")},
        {QStringLiteral("next_repeat_failure_title"), QStringLiteral("Next repeat scheduling failed")},
        {QStringLiteral("wake_failure_title"), QStringLiteral("Wake from sleep failed")},
        {QStringLiteral("wake_android_note"), QStringLiteral("A background service is required on Android to wake the device. This version works only while the app stays open.")},
        {QStringLiteral("wake_platform_note"), QStringLiteral("This platform does not support waking from suspend.")},
        {QStringLiteral("wake_permission_error"), QStringLiteral("No write permission for %1. Run as administrator or adjust udev rules." )},
        {QStringLiteral("wake_clear_failed"), QStringLiteral("Failed to clear the RTC alarm: %1")},
        {QStringLiteral("wake_set_failed"), QStringLiteral("Failed to schedule the RTC alarm: %1")},
        {QStringLiteral("playback_error_title"), QStringLiteral("Playback error")},
        {QStringLiteral("playback_error_body"), QStringLiteral("Could not play the alarm sound: %1")},
        {QStringLiteral("audio_filter"), QStringLiteral("Audio files (*.mp3 *.wav *.ogg);;All files (*.*)")},
        {QStringLiteral("tray_show"), QStringLiteral("Show window")},
        {QStringLiteral("tray_cancel"), QStringLiteral("Cancel alarm")},
        {QStringLiteral("tray_quit"), QStringLiteral("Quit app")},
        {QStringLiteral("tray_message_title"), QStringLiteral("Running in background")},
        {QStringLiteral("tray_message_body"), QStringLiteral("The alarm keeps working even when the window is closed. Use the tray icon to reopen it.")},
        {QStringLiteral("repeat_none"), QStringLiteral("No repeat")},
        {QStringLiteral("repeat_prefix"), QStringLiteral("Repeats every week on %1")},
        {QStringLiteral("list_separator"), QStringLiteral(", ")},
        {QStringLiteral("question_punctuality"), QStringLiteral("How punctual are you usually?")},
        {QStringLiteral("punctuality_carefree"), QStringLiteral("I'm a bit carefree")},
        {QStringLiteral("punctuality_meticulous"), QStringLiteral("I'm very punctual")},
        {QStringLiteral("adjust_label"), QStringLiteral("Arrival preference")},
        {QStringLiteral("adjust_neutral"), QStringLiteral("Keep it exact")},
        {QStringLiteral("adjust_early"), QStringLiteral("A little earlier please")},
        {QStringLiteral("adjust_late"), QStringLiteral("A little later please")},
        {QStringLiteral("recommendation_title"), QStringLiteral("Suggested alternatives")},
        {QStringLiteral("recommendation_line"), QStringLiteral("• %1: %2 (%3)")},
        {QStringLiteral("recommendation_unavailable"), QStringLiteral("Set a valid time to see suggestions." )},
        {QStringLiteral("recommendation_earlier_big"), QStringLiteral("Earlier, bigger shift")},
        {QStringLiteral("recommendation_earlier_small"), QStringLiteral("Earlier, smaller shift")},
        {QStringLiteral("recommendation_later_big"), QStringLiteral("Later, bigger shift")},
        {QStringLiteral("recommendation_later_small"), QStringLiteral("Later, smaller shift")},
        {QStringLiteral("recommendation_offset_earlier"), QStringLiteral("%1 min earlier")},
        {QStringLiteral("recommendation_offset_later"), QStringLiteral("%1 min later")},
        {QStringLiteral("time_hms_format"), QStringLiteral("%1h %2m %3s")},
        {QStringLiteral("schedule_time_format"), QStringLiteral("yyyy-MM-dd hh:mm")}
    }
};

const LanguageStrings koreanStrings = {
    QLocale::Korean,
    {
        {QStringLiteral("app_title"), QStringLiteral("Achim Alarm")},
        {QStringLiteral("app_heading"), QStringLiteral("아침: 즐거운 아침 되세요!")},
        {QStringLiteral("date_label"), QStringLiteral("알람 날짜")},
        {QStringLiteral("repeat_group"), QStringLiteral("요일 반복")},
        {QStringLiteral("repeat_hint"), QStringLiteral("선택된 요일에 매주 알람이 반복됩니다. 선택하지 않으면 지정한 날짜의 1회성 알람으로 동작합니다.")},
        {QStringLiteral("pick_sound_button"), QStringLiteral("알람 소리 선택")},
        {QStringLiteral("no_sound_selected"), QStringLiteral("선택된 파일 없음")},
        {QStringLiteral("wake_checkbox"), QStringLiteral("절전모드에서도 깨우기 시도")},
        {QStringLiteral("set_button"), QStringLiteral("알람 설정")},
        {QStringLiteral("cancel_button"), QStringLiteral("해제")},
        {QStringLiteral("status_playing"), QStringLiteral("알람이 울리는 중입니다." )},
        {QStringLiteral("status_time_remaining"), QStringLiteral("알람까지 %1 남았습니다.")},
        {QStringLiteral("status_scheduled_time"), QStringLiteral("예정 시각: %1")},
        {QStringLiteral("status_wake_active"), QStringLiteral("절전모드 해제 요청이 설정되었습니다.")},
        {QStringLiteral("status_none"), QStringLiteral("알람이 설정되지 않았습니다.")},
        {QStringLiteral("sound_required_title"), QStringLiteral("알람 소리 필요")},
        {QStringLiteral("sound_required_body"), QStringLiteral("알람 소리를 먼저 선택해주세요.")},
        {QStringLiteral("sound_missing_title"), QStringLiteral("소리를 찾을 수 없음")},
        {QStringLiteral("sound_missing_body"), QStringLiteral("선택한 소리 파일을 찾을 수 없습니다. 새 파일을 선택해주세요.")},
        {QStringLiteral("repeat_failure_title"), QStringLiteral("반복 예약 실패")},
        {QStringLiteral("repeat_failure_body"), QStringLiteral("선택한 요일로 다음 알람 시간을 계산할 수 없습니다.")},
        {QStringLiteral("time_error_title"), QStringLiteral("시간 오류")},
        {QStringLiteral("time_error_body"), QStringLiteral("알람 시간을 다시 확인해주세요.")},
        {QStringLiteral("next_repeat_failure_title"), QStringLiteral("다음 반복 예약 실패")},
        {QStringLiteral("wake_failure_title"), QStringLiteral("절전 깨우기 실패")},
        {QStringLiteral("wake_android_note"), QStringLiteral("Android에서는 시스템 절전 해제를 위해 별도의 백그라운드 서비스가 필요합니다. 현재 버전은 앱이 실행 중일 때만 동작합니다.")},
        {QStringLiteral("wake_platform_note"), QStringLiteral("이 플랫폼에서는 시스템 절전 해제를 지원하지 않습니다.")},
        {QStringLiteral("wake_permission_error"), QStringLiteral("%1에 기록 권한이 없습니다. 관리자 권한으로 실행하거나 udev 규칙을 추가해주세요.")},
        {QStringLiteral("wake_clear_failed"), QStringLiteral("RTC 알람 초기화에 실패했습니다: %1")},
        {QStringLiteral("wake_set_failed"), QStringLiteral("RTC 알람 예약에 실패했습니다: %1")},
        {QStringLiteral("playback_error_title"), QStringLiteral("재생 오류")},
        {QStringLiteral("playback_error_body"), QStringLiteral("알람 소리를 재생할 수 없습니다: %1")},
        {QStringLiteral("audio_filter"), QStringLiteral("오디오 파일 (*.mp3 *.wav *.ogg);;모든 파일 (*.*)")},
        {QStringLiteral("tray_show"), QStringLiteral("창 열기")},
        {QStringLiteral("tray_cancel"), QStringLiteral("알람 해제")},
        {QStringLiteral("tray_quit"), QStringLiteral("앱 종료")},
        {QStringLiteral("tray_message_title"), QStringLiteral("백그라운드 실행")},
        {QStringLiteral("tray_message_body"), QStringLiteral("창을 닫아도 알람은 계속 작동합니다. 트레이 아이콘을 통해 다시 열 수 있습니다.")},
        {QStringLiteral("repeat_none"), QStringLiteral("반복 없음")},
        {QStringLiteral("repeat_prefix"), QStringLiteral("매주 %1 반복")},
        {QStringLiteral("list_separator"), QStringLiteral(", ")},
        {QStringLiteral("question_punctuality"), QStringLiteral("평소 시간 약속 성향이 어떠신가요?")},
        {QStringLiteral("punctuality_carefree"), QStringLiteral("덤벙대요")},
        {QStringLiteral("punctuality_meticulous"), QStringLiteral("꼼꼼해요")},
        {QStringLiteral("adjust_label"), QStringLiteral("도착 선호")},
        {QStringLiteral("adjust_neutral"), QStringLiteral("정시에 맞춰주세요")},
        {QStringLiteral("adjust_early"), QStringLiteral("조금 더 일찍 부탁해요")},
        {QStringLiteral("adjust_late"), QStringLiteral("조금 더 늦게 부탁해요")},
        {QStringLiteral("recommendation_title"), QStringLiteral("추천 대안 시간")},
        {QStringLiteral("recommendation_line"), QStringLiteral("• %1: %2 (%3)")},
        {QStringLiteral("recommendation_unavailable"), QStringLiteral("추천을 보려면 유효한 시간을 설정해주세요.")},
        {QStringLiteral("recommendation_earlier_big"), QStringLiteral("더 빠른 큰 조정")},
        {QStringLiteral("recommendation_earlier_small"), QStringLiteral("더 빠른 작은 조정")},
        {QStringLiteral("recommendation_later_big"), QStringLiteral("더 늦은 큰 조정")},
        {QStringLiteral("recommendation_later_small"), QStringLiteral("더 늦은 작은 조정")},
        {QStringLiteral("recommendation_offset_earlier"), QStringLiteral("%1분 더 일찍")},
        {QStringLiteral("recommendation_offset_later"), QStringLiteral("%1분 더 늦게")},
        {QStringLiteral("time_hms_format"), QStringLiteral("%1시간 %2분 %3초")},
        {QStringLiteral("schedule_time_format"), QStringLiteral("yyyy-MM-dd hh:mm")}
    }
};

const LanguageStrings japaneseStrings = {
    QLocale::Japanese,
    {
        {QStringLiteral("app_title"), QStringLiteral("Achim Alarm")},
        {QStringLiteral("app_heading"), QStringLiteral("アーチムアラーム")},
        {QStringLiteral("date_label"), QStringLiteral("アラーム日付")},
        {QStringLiteral("repeat_group"), QStringLiteral("曜日の繰り返し")},
        {QStringLiteral("repeat_hint"), QStringLiteral("選択した曜日に毎週アラームが繰り返されます。選択しない場合は指定日のみ鳴ります。")},
        {QStringLiteral("pick_sound_button"), QStringLiteral("アラーム音を選択")},
        {QStringLiteral("no_sound_selected"), QStringLiteral("ファイルが選択されていません")},
        {QStringLiteral("wake_checkbox"), QStringLiteral("スリープからの復帰を試みる")},
        {QStringLiteral("set_button"), QStringLiteral("アラームを設定")},
        {QStringLiteral("cancel_button"), QStringLiteral("解除")},
        {QStringLiteral("status_playing"), QStringLiteral("アラームが鳴っています。")},
        {QStringLiteral("status_time_remaining"), QStringLiteral("アラームまで残り %1 です。")},
        {QStringLiteral("status_scheduled_time"), QStringLiteral("予定時刻: %1")},
        {QStringLiteral("status_wake_active"), QStringLiteral("スリープ復帰要求が有効です。")},
        {QStringLiteral("status_none"), QStringLiteral("アラームは設定されていません。")},
        {QStringLiteral("sound_required_title"), QStringLiteral("サウンドが必要です")},
        {QStringLiteral("sound_required_body"), QStringLiteral("まずアラーム音を選択してください。")},
        {QStringLiteral("sound_missing_title"), QStringLiteral("サウンドが見つかりません")},
        {QStringLiteral("sound_missing_body"), QStringLiteral("選択したサウンドファイルが見つかりません。別のファイルを選択してください。")},
        {QStringLiteral("repeat_failure_title"), QStringLiteral("繰り返しの予約に失敗しました")},
        {QStringLiteral("repeat_failure_body"), QStringLiteral("選択した曜日で次のアラーム時刻を計算できませんでした。")},
        {QStringLiteral("time_error_title"), QStringLiteral("時刻エラー")},
        {QStringLiteral("time_error_body"), QStringLiteral("アラーム時刻を確認してください。")},
        {QStringLiteral("next_repeat_failure_title"), QStringLiteral("次の繰り返し予約に失敗しました")},
        {QStringLiteral("wake_failure_title"), QStringLiteral("スリープ復帰に失敗しました")},
        {QStringLiteral("wake_android_note"), QStringLiteral("Androidでスリープから復帰するにはバックグラウンドサービスが必要です。このバージョンはアプリが開いている場合のみ動作します。")},
        {QStringLiteral("wake_platform_note"), QStringLiteral("このプラットフォームはスリープ復帰をサポートしていません。")},
        {QStringLiteral("wake_permission_error"), QStringLiteral("%1 への書き込み権限がありません。管理者として実行するか、udev ルールを調整してください。")},
        {QStringLiteral("wake_clear_failed"), QStringLiteral("RTC アラームのクリアに失敗しました: %1")},
        {QStringLiteral("wake_set_failed"), QStringLiteral("RTC アラームの予約に失敗しました: %1")},
        {QStringLiteral("playback_error_title"), QStringLiteral("再生エラー")},
        {QStringLiteral("playback_error_body"), QStringLiteral("アラーム音を再生できません: %1")},
        {QStringLiteral("audio_filter"), QStringLiteral("オーディオファイル (*.mp3 *.wav *.ogg);;すべてのファイル (*.*)")},
        {QStringLiteral("tray_show"), QStringLiteral("ウィンドウを表示")},
        {QStringLiteral("tray_cancel"), QStringLiteral("アラームを解除")},
        {QStringLiteral("tray_quit"), QStringLiteral("アプリを終了")},
        {QStringLiteral("tray_message_title"), QStringLiteral("バックグラウンドで動作中")},
        {QStringLiteral("tray_message_body"), QStringLiteral("ウィンドウを閉じてもアラームは動作します。トレイアイコンから再表示できます。")},
        {QStringLiteral("repeat_none"), QStringLiteral("繰り返しなし")},
        {QStringLiteral("repeat_prefix"), QStringLiteral("毎週 %1 に繰り返します")},
        {QStringLiteral("list_separator"), QStringLiteral("、")},
        {QStringLiteral("question_punctuality"), QStringLiteral("普段の時間厳守の傾向は？")},
        {QStringLiteral("punctuality_carefree"), QStringLiteral("せっかちです")},
        {QStringLiteral("punctuality_meticulous"), QStringLiteral("きちんとしています")},
        {QStringLiteral("adjust_label"), QStringLiteral("到着の好み")},
        {QStringLiteral("adjust_neutral"), QStringLiteral("ぴったりにしてください")},
        {QStringLiteral("adjust_early"), QStringLiteral("少し早めでお願いします")},
        {QStringLiteral("adjust_late"), QStringLiteral("少し遅めでお願いします")},
        {QStringLiteral("recommendation_title"), QStringLiteral("おすすめの代替時間")},
        {QStringLiteral("recommendation_line"), QStringLiteral("• %1: %2 (%3)")},
        {QStringLiteral("recommendation_unavailable"), QStringLiteral("有効な時刻を設定すると提案が表示されます。")},
        {QStringLiteral("recommendation_earlier_big"), QStringLiteral("早め・大きな調整")},
        {QStringLiteral("recommendation_earlier_small"), QStringLiteral("早め・小さな調整")},
        {QStringLiteral("recommendation_later_big"), QStringLiteral("遅め・大きな調整")},
        {QStringLiteral("recommendation_later_small"), QStringLiteral("遅め・小さな調整")},
        {QStringLiteral("recommendation_offset_earlier"), QStringLiteral("%1 分早く")},
        {QStringLiteral("recommendation_offset_later"), QStringLiteral("%1 分遅く")},
        {QStringLiteral("time_hms_format"), QStringLiteral("%1時間 %2分 %3秒")},
        {QStringLiteral("schedule_time_format"), QStringLiteral("yyyy-MM-dd hh:mm")}
    }
};

const LanguageStrings chineseStrings = {
    QLocale::Chinese,
    {
        {QStringLiteral("app_title"), QStringLiteral("Achim Alarm")},
        {QStringLiteral("app_heading"), QStringLiteral("阿晨闹钟")},
        {QStringLiteral("date_label"), QStringLiteral("闹钟日期")},
        {QStringLiteral("repeat_group"), QStringLiteral("按星期重复")},
        {QStringLiteral("repeat_hint"), QStringLiteral("闹钟会在选定的星期几重复。如不选择，则只在指定日期响一次。")},
        {QStringLiteral("pick_sound_button"), QStringLiteral("选择闹铃声音")},
        {QStringLiteral("no_sound_selected"), QStringLiteral("未选择文件")},
        {QStringLiteral("wake_checkbox"), QStringLiteral("尝试从休眠唤醒")},
        {QStringLiteral("set_button"), QStringLiteral("设置闹钟")},
        {QStringLiteral("cancel_button"), QStringLiteral("取消")},
        {QStringLiteral("status_playing"), QStringLiteral("闹钟正在响铃。")},
        {QStringLiteral("status_time_remaining"), QStringLiteral("距离闹钟还有 %1。")},
        {QStringLiteral("status_scheduled_time"), QStringLiteral("预定时间：%1")},
        {QStringLiteral("status_wake_active"), QStringLiteral("已启用休眠唤醒请求。")},
        {QStringLiteral("status_none"), QStringLiteral("尚未设置闹钟。")},
        {QStringLiteral("sound_required_title"), QStringLiteral("需要选择声音")},
        {QStringLiteral("sound_required_body"), QStringLiteral("请先选择闹铃声音。")},
        {QStringLiteral("sound_missing_title"), QStringLiteral("无法找到铃声")},
        {QStringLiteral("sound_missing_body"), QStringLiteral("所选铃声文件不存在，请重新选择。")},
        {QStringLiteral("repeat_failure_title"), QStringLiteral("重复计划失败")},
        {QStringLiteral("repeat_failure_body"), QStringLiteral("无法根据所选星期计算下一次闹钟时间。")},
        {QStringLiteral("time_error_title"), QStringLiteral("时间错误")},
        {QStringLiteral("time_error_body"), QStringLiteral("请检查闹钟时间。")},
        {QStringLiteral("next_repeat_failure_title"), QStringLiteral("下一次重复计划失败")},
        {QStringLiteral("wake_failure_title"), QStringLiteral("休眠唤醒失败")},
        {QStringLiteral("wake_android_note"), QStringLiteral("在 Android 上需要后台服务才能从休眠唤醒。本版本仅在应用保持打开时可用。")},
        {QStringLiteral("wake_platform_note"), QStringLiteral("此平台不支持从休眠唤醒。")},
        {QStringLiteral("wake_permission_error"), QStringLiteral("没有 %1 的写权限。请以管理员身份运行或调整 udev 规则。")},
        {QStringLiteral("wake_clear_failed"), QStringLiteral("清除 RTC 闹钟失败：%1")},
        {QStringLiteral("wake_set_failed"), QStringLiteral("设置 RTC 闹钟失败：%1")},
        {QStringLiteral("playback_error_title"), QStringLiteral("播放错误")},
        {QStringLiteral("playback_error_body"), QStringLiteral("无法播放铃声：%1")},
        {QStringLiteral("audio_filter"), QStringLiteral("音频文件 (*.mp3 *.wav *.ogg);;所有文件 (*.*)")},
        {QStringLiteral("tray_show"), QStringLiteral("显示窗口")},
        {QStringLiteral("tray_cancel"), QStringLiteral("取消闹钟")},
        {QStringLiteral("tray_quit"), QStringLiteral("退出应用")},
        {QStringLiteral("tray_message_title"), QStringLiteral("后台运行中")},
        {QStringLiteral("tray_message_body"), QStringLiteral("即使关闭窗口，闹钟仍会运行。使用托盘图标重新打开。")},
        {QStringLiteral("repeat_none"), QStringLiteral("不重复")},
        {QStringLiteral("repeat_prefix"), QStringLiteral("每周的 %1 重复")},
        {QStringLiteral("list_separator"), QStringLiteral("，")},
        {QStringLiteral("question_punctuality"), QStringLiteral("您平时守时的倾向如何？")},
        {QStringLiteral("punctuality_carefree"), QStringLiteral("我有点毛躁")},
        {QStringLiteral("punctuality_meticulous"), QStringLiteral("我很细心")},
        {QStringLiteral("adjust_label"), QStringLiteral("到达偏好")},
        {QStringLiteral("adjust_neutral"), QStringLiteral("保持准时")},
        {QStringLiteral("adjust_early"), QStringLiteral("请稍微提前一点")},
        {QStringLiteral("adjust_late"), QStringLiteral("请稍微晚一点")},
        {QStringLiteral("recommendation_title"), QStringLiteral("推荐的备选时间")},
        {QStringLiteral("recommendation_line"), QStringLiteral("• %1: %2 (%3)")},
        {QStringLiteral("recommendation_unavailable"), QStringLiteral("请设置有效时间以查看建议。")},
        {QStringLiteral("recommendation_earlier_big"), QStringLiteral("提前较大的调整")},
        {QStringLiteral("recommendation_earlier_small"), QStringLiteral("提前较小的调整")},
        {QStringLiteral("recommendation_later_big"), QStringLiteral("推迟较大的调整")},
        {QStringLiteral("recommendation_later_small"), QStringLiteral("推迟较小的调整")},
        {QStringLiteral("recommendation_offset_earlier"), QStringLiteral("提前 %1 分钟")},
        {QStringLiteral("recommendation_offset_later"), QStringLiteral("推迟 %1 分钟")},
        {QStringLiteral("time_hms_format"), QStringLiteral("%1小时 %2分 %3秒")},
        {QStringLiteral("schedule_time_format"), QStringLiteral("yyyy-MM-dd hh:mm")}
    }
};

const LanguageStrings mongolianStrings = {
    QLocale::Mongolian,
    {
        {QStringLiteral("app_title"), QStringLiteral("Achim Alarm")},
        {QStringLiteral("app_heading"), QStringLiteral("Ачимийсэрүүлэг")},
        {QStringLiteral("date_label"), QStringLiteral("Сэрүүлгийн огноо")},
        {QStringLiteral("repeat_group"), QStringLiteral("Долоо хоног бүр давтах")},
        {QStringLiteral("repeat_hint"), QStringLiteral("Сонгосон гариг бүрт сэрүүлэг давтагдана. Хоосон бол зөвхөн сонгосон өдөр нэг удаа дуугарна.")},
        {QStringLiteral("pick_sound_button"), QStringLiteral("Сэрүүлгийн дуу сонгох")},
        {QStringLiteral("no_sound_selected"), QStringLiteral("Файл сонгогдоогүй")},
        {QStringLiteral("wake_checkbox"), QStringLiteral("Унтралтаас сэрээхийг оролдох")},
        {QStringLiteral("set_button"), QStringLiteral("Сэрүүлэг тохируулах")},
        {QStringLiteral("cancel_button"), QStringLiteral("Цуцлах")},
        {QStringLiteral("status_playing"), QStringLiteral("Сэрүүлэг дуугарч байна.")},
        {QStringLiteral("status_time_remaining"), QStringLiteral("Сэрүүлэг хүртэл %1 үлдлээ.")},
        {QStringLiteral("status_scheduled_time"), QStringLiteral("Товлосон цаг: %1")},
        {QStringLiteral("status_wake_active"), QStringLiteral("Унтралтаас сэргээх хүсэлт идэвхтэй." )},
        {QStringLiteral("status_none"), QStringLiteral("Сэрүүлэг тохируулаагүй байна.")},
        {QStringLiteral("sound_required_title"), QStringLiteral("Дуу шаардлагатай")},
        {QStringLiteral("sound_required_body"), QStringLiteral("Эхлээд сэрүүлгийн дуу сонгоно уу.")},
        {QStringLiteral("sound_missing_title"), QStringLiteral("Дуу олдсонгүй")},
        {QStringLiteral("sound_missing_body"), QStringLiteral("Сонгосон дохионы файлыг олдсонгүй. Шинэ файлыг сонгоно уу.")},
        {QStringLiteral("repeat_failure_title"), QStringLiteral("Давталтыг тооцоолоход алдаа гарлаа")},
        {QStringLiteral("repeat_failure_body"), QStringLiteral("Сонгосон өдрүүдээр дараагийн сэрүүлгийн цагийг тооцоолж чадсангүй.")},
        {QStringLiteral("time_error_title"), QStringLiteral("Цагийн алдаа")},
        {QStringLiteral("time_error_body"), QStringLiteral("Сэрүүлгийн цагийг шалгана уу.")},
        {QStringLiteral("next_repeat_failure_title"), QStringLiteral("Дараагийн давталтыг тохируулахад алдаа гарлаа")},
        {QStringLiteral("wake_failure_title"), QStringLiteral("Унтралтаас сэрээгүй")},
        {QStringLiteral("wake_android_note"), QStringLiteral("Android дээр унтралтаас сэрээхийн тулд дэвсгэр үйлчилгээ хэрэгтэй. Энэ хувилбар нь апп нээлттэй үед л ажиллана.")},
        {QStringLiteral("wake_platform_note"), QStringLiteral("Энэ платформ унтралтаас сэрээхийг дэмждэггүй.")},
        {QStringLiteral("wake_permission_error"), QStringLiteral("%1 дээр бичих эрх байхгүй. Администратор эрхээр ажиллуулах эсвэл udev дүрмийг засна уу.")},
        {QStringLiteral("wake_clear_failed"), QStringLiteral("RTC сэрүүлгийг арилгаж чадсангүй: %1")},
        {QStringLiteral("wake_set_failed"), QStringLiteral("RTC сэрүүлгийг тохируулахад алдаа гарлаа: %1")},
        {QStringLiteral("playback_error_title"), QStringLiteral("Тоглуулалтын алдаа")},
        {QStringLiteral("playback_error_body"), QStringLiteral("Сэрүүлгийн дууг тоглуулж чадсангүй: %1")},
        {QStringLiteral("audio_filter"), QStringLiteral("Аудио файлууд (*.mp3 *.wav *.ogg);;Бүх файлууд (*.*)")},
        {QStringLiteral("tray_show"), QStringLiteral("Цонхыг харуулах")},
        {QStringLiteral("tray_cancel"), QStringLiteral("Сэрүүлгийг цуцлах")},
        {QStringLiteral("tray_quit"), QStringLiteral("Аппаас гарах")},
        {QStringLiteral("tray_message_title"), QStringLiteral("Арын горимд ажиллаж байна")},
        {QStringLiteral("tray_message_body"), QStringLiteral("Цонхыг хаасан ч сэрүүлэг ажиллана. Трей дүрсээр дахин нээнэ үү.")},
        {QStringLiteral("repeat_none"), QStringLiteral("Давталтгүй")},
        {QStringLiteral("repeat_prefix"), QStringLiteral("Долоо хоног бүр %1-д давтана")},
        {QStringLiteral("list_separator"), QStringLiteral(", ")},
        {QStringLiteral("question_punctuality"), QStringLiteral("Та ихэвчлэн цаг барьдаг уу?")},
        {QStringLiteral("punctuality_carefree"), QStringLiteral("Яаруу сандруу")},
        {QStringLiteral("punctuality_meticulous"), QStringLiteral("Нямбай")},
        {QStringLiteral("adjust_label"), QStringLiteral("Ирэх цагийн сонголт")},
        {QStringLiteral("adjust_neutral"), QStringLiteral("Яг тохируулна уу")},
        {QStringLiteral("adjust_early"), QStringLiteral("Жаахан эрт болгож өгөөч")},
        {QStringLiteral("adjust_late"), QStringLiteral("Жаахан орой болгож өгөөч")},
        {QStringLiteral("recommendation_title"), QStringLiteral("Санал болгож буй өөр цагууд")},
        {QStringLiteral("recommendation_line"), QStringLiteral("• %1: %2 (%3)")},
        {QStringLiteral("recommendation_unavailable"), QStringLiteral("Зөвлөмж харахын тулд зөв цаг тохируулна уу.")},
        {QStringLiteral("recommendation_earlier_big"), QStringLiteral("Илүү эрт – их өөрчлөлт")},
        {QStringLiteral("recommendation_earlier_small"), QStringLiteral("Илүү эрт – бага өөрчлөлт")},
        {QStringLiteral("recommendation_later_big"), QStringLiteral("Илүү орой – их өөрчлөлт")},
        {QStringLiteral("recommendation_later_small"), QStringLiteral("Илүү орой – бага өөрчлөлт")},
        {QStringLiteral("recommendation_offset_earlier"), QStringLiteral("%1 минут эрт")},
        {QStringLiteral("recommendation_offset_later"), QStringLiteral("%1 минут орой")},
        {QStringLiteral("time_hms_format"), QStringLiteral("%1ц %2мин %3сек")},
        {QStringLiteral("schedule_time_format"), QStringLiteral("yyyy-MM-dd hh:mm")}
    }
};

const LanguageStrings vietnameseStrings = {
    QLocale::Vietnamese,
    {
        {QStringLiteral("app_title"), QStringLiteral("Achim Alarm")},
        {QStringLiteral("app_heading"), QStringLiteral("Báo thức Achim")},
        {QStringLiteral("date_label"), QStringLiteral("Ngày báo thức")},
        {QStringLiteral("repeat_group"), QStringLiteral("Lặp theo tuần")},
        {QStringLiteral("repeat_hint"), QStringLiteral("Báo thức sẽ lặp vào các ngày trong tuần được chọn. Để trống để báo một lần vào ngày đã chọn.")},
        {QStringLiteral("pick_sound_button"), QStringLiteral("Chọn âm báo")},
        {QStringLiteral("no_sound_selected"), QStringLiteral("Chưa chọn tệp")},
        {QStringLiteral("wake_checkbox"), QStringLiteral("Thử đánh thức khi ngủ")},
        {QStringLiteral("set_button"), QStringLiteral("Đặt báo thức")},
        {QStringLiteral("cancel_button"), QStringLiteral("Huỷ")},
        {QStringLiteral("status_playing"), QStringLiteral("Báo thức đang reo.")},
        {QStringLiteral("status_time_remaining"), QStringLiteral("Còn %1 trước khi báo thức." )},
        {QStringLiteral("status_scheduled_time"), QStringLiteral("Thời gian dự kiến: %1")},
        {QStringLiteral("status_wake_active"), QStringLiteral("Yêu cầu đánh thức đang bật.")},
        {QStringLiteral("status_none"), QStringLiteral("Chưa có báo thức nào.")},
        {QStringLiteral("sound_required_title"), QStringLiteral("Cần âm báo")},
        {QStringLiteral("sound_required_body"), QStringLiteral("Vui lòng chọn âm báo trước.")},
        {QStringLiteral("sound_missing_title"), QStringLiteral("Không tìm thấy âm thanh")},
        {QStringLiteral("sound_missing_body"), QStringLiteral("Không tìm thấy tập tin âm thanh đã chọn. Vui lòng chọn tập tin khác.")},
        {QStringLiteral("repeat_failure_title"), QStringLiteral("Không thể đặt lặp")},
        {QStringLiteral("repeat_failure_body"), QStringLiteral("Không thể tính thời gian báo tiếp theo cho các ngày đã chọn.")},
        {QStringLiteral("time_error_title"), QStringLiteral("Lỗi thời gian")},
        {QStringLiteral("time_error_body"), QStringLiteral("Vui lòng kiểm tra lại thời gian báo.")},
        {QStringLiteral("next_repeat_failure_title"), QStringLiteral("Lỗi đặt lặp tiếp theo")},
        {QStringLiteral("wake_failure_title"), QStringLiteral("Đánh thức khi ngủ thất bại")},
        {QStringLiteral("wake_android_note"), QStringLiteral("Android cần dịch vụ nền để đánh thức thiết bị. Phiên bản này chỉ hoạt động khi ứng dụng đang mở.")},
        {QStringLiteral("wake_platform_note"), QStringLiteral("Nền tảng này không hỗ trợ đánh thức khỏi chế độ ngủ.")},
        {QStringLiteral("wake_permission_error"), QStringLiteral("Không có quyền ghi vào %1. Hãy chạy với quyền quản trị hoặc điều chỉnh quy tắc udev.")},
        {QStringLiteral("wake_clear_failed"), QStringLiteral("Không xoá được báo RTC: %1")},
        {QStringLiteral("wake_set_failed"), QStringLiteral("Không đặt được báo RTC: %1")},
        {QStringLiteral("playback_error_title"), QStringLiteral("Lỗi phát")},
        {QStringLiteral("playback_error_body"), QStringLiteral("Không thể phát âm thanh báo thức: %1")},
        {QStringLiteral("audio_filter"), QStringLiteral("Tệp âm thanh (*.mp3 *.wav *.ogg);;Tất cả tệp (*.*)")},
        {QStringLiteral("tray_show"), QStringLiteral("Hiển thị cửa sổ")},
        {QStringLiteral("tray_cancel"), QStringLiteral("Huỷ báo thức")},
        {QStringLiteral("tray_quit"), QStringLiteral("Thoát ứng dụng")},
        {QStringLiteral("tray_message_title"), QStringLiteral("Đang chạy nền")},
        {QStringLiteral("tray_message_body"), QStringLiteral("Ngay cả khi đóng cửa sổ, báo thức vẫn hoạt động. Dùng biểu tượng khay để mở lại.")},
        {QStringLiteral("repeat_none"), QStringLiteral("Không lặp")},
        {QStringLiteral("repeat_prefix"), QStringLiteral("Lặp mỗi tuần vào %1")},
        {QStringLiteral("list_separator"), QStringLiteral(", ")},
        {QStringLiteral("question_punctuality"), QStringLiteral("Bạn thường đúng giờ như thế nào?")},
        {QStringLiteral("punctuality_carefree"), QStringLiteral("Tôi hơi hấp tấp")},
        {QStringLiteral("punctuality_meticulous"), QStringLiteral("Tôi rất cẩn thận")},
        {QStringLiteral("adjust_label"), QStringLiteral("Ưu tiên thời điểm đến")},
        {QStringLiteral("adjust_neutral"), QStringLiteral("Giữ đúng giờ")},
        {QStringLiteral("adjust_early"), QStringLiteral("Cho sớm hơn một chút")},
        {QStringLiteral("adjust_late"), QStringLiteral("Cho muộn hơn một chút")},
        {QStringLiteral("recommendation_title"), QStringLiteral("Giờ gợi ý thay thế")},
        {QStringLiteral("recommendation_line"), QStringLiteral("• %1: %2 (%3)")},
        {QStringLiteral("recommendation_unavailable"), QStringLiteral("Hãy đặt thời gian hợp lệ để xem gợi ý.")},
        {QStringLiteral("recommendation_earlier_big"), QStringLiteral("Sớm hơn - thay đổi lớn")},
        {QStringLiteral("recommendation_earlier_small"), QStringLiteral("Sớm hơn - thay đổi nhỏ")},
        {QStringLiteral("recommendation_later_big"), QStringLiteral("Muộn hơn - thay đổi lớn")},
        {QStringLiteral("recommendation_later_small"), QStringLiteral("Muộn hơn - thay đổi nhỏ")},
        {QStringLiteral("recommendation_offset_earlier"), QStringLiteral("Sớm hơn %1 phút")},
        {QStringLiteral("recommendation_offset_later"), QStringLiteral("Muộn hơn %1 phút")},
        {QStringLiteral("time_hms_format"), QStringLiteral("%1 giờ %2 phút %3 giây")},
        {QStringLiteral("schedule_time_format"), QStringLiteral("yyyy-MM-dd hh:mm")}
    }
};

const LanguageStrings russianStrings = {
    QLocale::Russian,
    {
        {QStringLiteral("app_title"), QStringLiteral("Achim Alarm")},
        {QStringLiteral("app_heading"), QStringLiteral("Будильник Ачим")},
        {QStringLiteral("date_label"), QStringLiteral("Дата будильника")},
        {QStringLiteral("repeat_group"), QStringLiteral("Повтор по дням недели")},
        {QStringLiteral("repeat_hint"), QStringLiteral("Будильник повторяется в выбранные дни недели. Если ничего не выбрано, он сработает один раз в указанную дату.")},
        {QStringLiteral("pick_sound_button"), QStringLiteral("Выбрать звук будильника")},
        {QStringLiteral("no_sound_selected"), QStringLiteral("Файл не выбран")},
        {QStringLiteral("wake_checkbox"), QStringLiteral("Пробовать выводить из сна")},
        {QStringLiteral("set_button"), QStringLiteral("Установить будильник")},
        {QStringLiteral("cancel_button"), QStringLiteral("Отмена")},
        {QStringLiteral("status_playing"), QStringLiteral("Будильник звонит.")},
        {QStringLiteral("status_time_remaining"), QStringLiteral("До будильника осталось %1.")},
        {QStringLiteral("status_scheduled_time"), QStringLiteral("Запланированное время: %1")},
        {QStringLiteral("status_wake_active"), QStringLiteral("Запрос на вывод из сна активен.")},
        {QStringLiteral("status_none"), QStringLiteral("Будильник не установлен.")},
        {QStringLiteral("sound_required_title"), QStringLiteral("Требуется звук")},
        {QStringLiteral("sound_required_body"), QStringLiteral("Сначала выберите звук будильника.")},
        {QStringLiteral("sound_missing_title"), QStringLiteral("Звук не найден")},
        {QStringLiteral("sound_missing_body"), QStringLiteral("Выбранный звуковой файл не найден. Выберите новый файл.")},
        {QStringLiteral("repeat_failure_title"), QStringLiteral("Не удалось настроить повтор")},
        {QStringLiteral("repeat_failure_body"), QStringLiteral("Не удалось вычислить время следующего срабатывания для выбранных дней.")},
        {QStringLiteral("time_error_title"), QStringLiteral("Ошибка времени")},
        {QStringLiteral("time_error_body"), QStringLiteral("Проверьте время будильника.")},
        {QStringLiteral("next_repeat_failure_title"), QStringLiteral("Не удалось запланировать следующий повтор")},
        {QStringLiteral("wake_failure_title"), QStringLiteral("Не удалось вывести из сна")},
        {QStringLiteral("wake_android_note"), QStringLiteral("Для вывода из сна на Android требуется фоновый сервис. Эта версия работает только пока приложение открыто.")},
        {QStringLiteral("wake_platform_note"), QStringLiteral("Эта платформа не поддерживает вывод из сна.")},
        {QStringLiteral("wake_permission_error"), QStringLiteral("Нет прав записи в %1. Запустите от имени администратора или настройте правила udev.")},
        {QStringLiteral("wake_clear_failed"), QStringLiteral("Не удалось очистить будильник RTC: %1")},
        {QStringLiteral("wake_set_failed"), QStringLiteral("Не удалось запланировать будильник RTC: %1")},
        {QStringLiteral("playback_error_title"), QStringLiteral("Ошибка воспроизведения")},
        {QStringLiteral("playback_error_body"), QStringLiteral("Не удалось воспроизвести звук будильника: %1")},
        {QStringLiteral("audio_filter"), QStringLiteral("Аудиофайлы (*.mp3 *.wav *.ogg);;Все файлы (*.*)")},
        {QStringLiteral("tray_show"), QStringLiteral("Показать окно")},
        {QStringLiteral("tray_cancel"), QStringLiteral("Отменить будильник")},
        {QStringLiteral("tray_quit"), QStringLiteral("Выйти из приложения")},
        {QStringLiteral("tray_message_title"), QStringLiteral("Работает в фоне")},
        {QStringLiteral("tray_message_body"), QStringLiteral("Будильник продолжает работать после закрытия окна. Используйте значок в трее, чтобы открыть его снова.")},
        {QStringLiteral("repeat_none"), QStringLiteral("Без повтора")},
        {QStringLiteral("repeat_prefix"), QStringLiteral("Повтор каждую неделю: %1")},
        {QStringLiteral("list_separator"), QStringLiteral(", ")},
        {QStringLiteral("question_punctuality"), QStringLiteral("Как вы обычно относитесь к пунктуальности?")},
        {QStringLiteral("punctuality_carefree"), QStringLiteral("Я немного торопыжка")},
        {QStringLiteral("punctuality_meticulous"), QStringLiteral("Я очень аккуратен")},
        {QStringLiteral("adjust_label"), QStringLiteral("Предпочтение прибытия")},
        {QStringLiteral("adjust_neutral"), QStringLiteral("Пусть будет точно")},
        {QStringLiteral("adjust_early"), QStringLiteral("Чуть пораньше, пожалуйста")},
        {QStringLiteral("adjust_late"), QStringLiteral("Чуть попозже, пожалуйста")},
        {QStringLiteral("recommendation_title"), QStringLiteral("Предложенные альтернативы")},
        {QStringLiteral("recommendation_line"), QStringLiteral("• %1: %2 (%3)")},
        {QStringLiteral("recommendation_unavailable"), QStringLiteral("Установите корректное время, чтобы увидеть предложения.")},
        {QStringLiteral("recommendation_earlier_big"), QStringLiteral("Раньше — большое смещение")},
        {QStringLiteral("recommendation_earlier_small"), QStringLiteral("Раньше — маленькое смещение")},
        {QStringLiteral("recommendation_later_big"), QStringLiteral("Позже — большое смещение")},
        {QStringLiteral("recommendation_later_small"), QStringLiteral("Позже — маленькое смещение")},
        {QStringLiteral("recommendation_offset_earlier"), QStringLiteral("На %1 мин раньше")},
        {QStringLiteral("recommendation_offset_later"), QStringLiteral("На %1 мин позже")},
        {QStringLiteral("time_hms_format"), QStringLiteral("%1 ч %2 мин %3 с")},
        {QStringLiteral("schedule_time_format"), QStringLiteral("yyyy-MM-dd hh:mm")}
    }
};

const QVector<LanguageStrings> allStrings = {
    englishStrings,
    koreanStrings,
    japaneseStrings,
    chineseStrings,
    mongolianStrings,
    vietnameseStrings,
    russianStrings
};

QString localizedText(const QString &key)
{
    const QLocale locale = QLocale::system();
    const auto uiLanguages = locale.uiLanguages();
    auto lookup = [&](QLocale::Language lang) -> QString {
        for (const auto &entry : allStrings) {
            if (entry.language == lang) {
                return entry.values.value(key);
            }
        }
        return QString();
    };

    for (const QString &languageTag : uiLanguages) {
        const QLocale uiLocale(languageTag);
        const QString text = lookup(uiLocale.language());
        if (!text.isEmpty()) {
            return text;
        }
    }

    QString text = lookup(locale.language());
    if (!text.isEmpty()) {
        return text;
    }

    text = lookup(QLocale::English);
    if (!text.isEmpty()) {
        return text;
    }

    return englishStrings.values.value(key, key);
}

QIcon systemClockIcon()
{
    const auto tryThemeIcons = [](const QStringList &names) -> QIcon {
        for (const QString &name : names) {
            if (name.isEmpty()) {
                continue;
            }
            const QIcon icon = QIcon::fromTheme(name);
            if (!icon.isNull()) {
                return icon;
            }
        }
        return {};
    };

    QIcon icon;

#if defined(Q_OS_WINDOWS)
    icon = tryThemeIcons(QStringList{
        QStringLiteral("clock"),
        QStringLiteral("AlarmClock"),
        QStringLiteral("timedate"),
        QStringLiteral("date-and-time")
    });
    if (icon.isNull()) {
        QFileIconProvider provider;
        const auto iconFromPath = [&](const QString &path) -> QIcon {
            const QFileInfo info(path);
            if (info.exists()) {
                return provider.icon(info);
            }
            return {};
        };

        const QString windowsDir = qEnvironmentVariable("WINDIR");
        if (!windowsDir.isEmpty()) {
            icon = iconFromPath(QDir(windowsDir).filePath(QStringLiteral("System32/timedate.cpl")));
        }
        if (icon.isNull()) {
            icon = iconFromPath(QStringLiteral("C:/Windows/System32/timedate.cpl"));
        }
    }
#elif defined(Q_OS_MACOS)
    icon = tryThemeIcons(QStringList{
        QStringLiteral("clock"),
        QStringLiteral("Clock"),
        QStringLiteral("com.apple.clockapp"),
        QStringLiteral("com.apple.menuitem.clock")
    });
    if (icon.isNull()) {
        QFileIconProvider provider;
        const auto iconFromPath = [&](const QString &path) -> QIcon {
            const QFileInfo info(path);
            if (info.exists()) {
                return provider.icon(info);
            }
            return {};
        };

        icon = iconFromPath(QStringLiteral("/System/Applications/Clock.app"));
        if (icon.isNull()) {
            icon = iconFromPath(QStringLiteral("/Applications/Clock.app"));
        }
    }
#elif defined(Q_OS_IOS)
    icon = tryThemeIcons(QStringList{
        QStringLiteral("clock"),
        QStringLiteral("Clock")
    });
#elif defined(Q_OS_ANDROID)
    icon = tryThemeIcons(QStringList{
        QStringLiteral("clock"),
        QStringLiteral("alarm"),
        QStringLiteral("ic_menu_recent_history"),
        QStringLiteral("ic_menu_history")
    });
#else
    icon = tryThemeIcons(QStringList{
        QStringLiteral("clock"),
        QStringLiteral("alarm"),
        QStringLiteral("alarm-clock"),
        QStringLiteral("preferences-system-time"),
        QStringLiteral("org.gnome.clocks"),
        QStringLiteral("gnome-clocks"),
        QStringLiteral("appointment-new")
    });
#endif

    if (icon.isNull()) {
        icon = tryThemeIcons(QStringList{
            QStringLiteral("clock"),
            QStringLiteral("alarm"),
            QStringLiteral("preferences-system-time"),
            QStringLiteral("appointment-new"),
            QStringLiteral("time")
        });
    }

    if (icon.isNull()) {
        icon = QIcon(QStringLiteral(":/tray/logo.png"));
    }

    return icon;
}

} // namespace

AlarmWindow::AlarmWindow(QWidget *parent)
    : QWidget(parent),
      m_statusLabel(nullptr),
      m_soundLabel(nullptr),
      m_timeEdit(nullptr),
      m_dateEdit(nullptr),
      m_wakeCheck(nullptr),
      m_setButton(nullptr),
      m_cancelButton(nullptr),
      m_pickSoundButton(nullptr),
      m_punctualityCombo(nullptr),
      m_adjustCombo(nullptr),
      m_recommendationLabel(nullptr),
      m_countdownTimer(new QTimer(this)),
      m_alarmTimer(new QTimer(this)),
      m_player(new QMediaPlayer(this)),
      m_audioOutput(new QAudioOutput(this)),
      m_trayIcon(nullptr),
      m_trayMenu(nullptr),
      m_showAction(nullptr),
      m_quitAction(nullptr),
      m_wakeRequestActive(false),
      m_trayMessageShown(false),
      m_loopPlayback(false)
{
    buildUi();

    m_audioOutput->setVolume(0.8f);
    m_player->setAudioOutput(m_audioOutput);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_player->setAudioRole(QAudio::AlarmRole);
#endif

    m_alarmTimer->setSingleShot(true);
    connect(m_alarmTimer, &QTimer::timeout, this, &AlarmWindow::handleAlarmTriggered);

    m_countdownTimer->setInterval(1000);
    connect(m_countdownTimer, &QTimer::timeout, this, &AlarmWindow::updateCountdown);

    connect(m_pickSoundButton, &QPushButton::clicked, this, &AlarmWindow::pickSound);
    connect(m_setButton, &QPushButton::clicked, this, &AlarmWindow::scheduleAlarm);
    connect(m_cancelButton, &QPushButton::clicked, this, &AlarmWindow::cancelAlarm);

    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (m_loopPlayback && status == QMediaPlayer::EndOfMedia) {
            m_player->setPosition(0);
            m_player->play();
        }
    });
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState) {
        refreshStatusLabel();
    });
    connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &errorString) {
        if (!errorString.isEmpty()) {
            showPlaybackError(errorString);
        }
    });

    if (m_timeEdit) {
        connect(m_timeEdit, &QTimeEdit::timeChanged, this, &AlarmWindow::updateRecommendations);
    }
    if (m_punctualityCombo) {
        connect(m_punctualityCombo, &QComboBox::currentIndexChanged, this, &AlarmWindow::updateRecommendations);
    }
    if (m_adjustCombo) {
        connect(m_adjustCombo, &QComboBox::currentIndexChanged, this, &AlarmWindow::updateRecommendations);
    }

    setupSystemTray();

    refreshStatusLabel();
    updateRecommendations();
}

void AlarmWindow::buildUi()
{
    setWindowTitle(localizedText(QStringLiteral("app_title")));
    setMinimumWidth(320);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    auto *title = new QLabel(localizedText(QStringLiteral("app_heading")), this);
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    const QTime currentTime = QTime::currentTime();

    m_timeEdit = new QTimeEdit(currentTime, this);
    m_timeEdit->setDisplayFormat("hh:mm");
    m_timeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_timeEdit->setAlignment(Qt::AlignCenter);
    m_timeEdit->setFixedHeight(48);
    QFont timeFont = m_timeEdit->font();
    timeFont.setPointSize(24);
    m_timeEdit->setFont(timeFont);
    layout->addWidget(m_timeEdit, 0, Qt::AlignCenter);

    auto *dateRow = new QHBoxLayout();
    auto *dateLabel = new QLabel(localizedText(QStringLiteral("date_label")), this);
    dateRow->addWidget(dateLabel);
    m_dateEdit = new QDateEdit(QDate::currentDate(), this);
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    m_dateEdit->setCalendarPopup(true);
    dateRow->addWidget(m_dateEdit, 1);
    layout->addLayout(dateRow);

    auto *repeatGroup = new QGroupBox(localizedText(QStringLiteral("repeat_group")), this);
    auto *repeatLayout = new QGridLayout(repeatGroup);
    repeatLayout->setSpacing(6);
    const QLocale locale = QLocale::system();
    for (int day = Qt::Monday; day <= Qt::Sunday; ++day) {
        auto *check = new QCheckBox(locale.dayName(day, QLocale::ShortFormat), repeatGroup);
        check->setProperty("weekday", day);
        const int index = day - Qt::Monday;
        const int row = index / 4;
        const int column = index % 4;
        repeatLayout->addWidget(check, row, column);
        m_repeatChecks.append(check);
    }
    auto *repeatHint = new QLabel(localizedText(QStringLiteral("repeat_hint")), repeatGroup);
    repeatHint->setWordWrap(true);
    repeatLayout->addWidget(repeatHint, 2, 0, 1, 4);
    layout->addWidget(repeatGroup);

    m_pickSoundButton = new QPushButton(localizedText(QStringLiteral("pick_sound_button")), this);
    layout->addWidget(m_pickSoundButton);

    m_soundLabel = new QLabel(localizedText(QStringLiteral("no_sound_selected")), this);
    m_soundLabel->setWordWrap(true);
    m_soundLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_soundLabel);

    m_wakeCheck = new QCheckBox(localizedText(QStringLiteral("wake_checkbox")), this);
    layout->addWidget(m_wakeCheck, 0, Qt::AlignCenter);

    auto *punctualityLabel = new QLabel(localizedText(QStringLiteral("question_punctuality")), this);
    punctualityLabel->setWordWrap(true);
    layout->addWidget(punctualityLabel);

    m_punctualityCombo = new QComboBox(this);
    m_punctualityCombo->addItem(localizedText(QStringLiteral("punctuality_carefree")));
    m_punctualityCombo->addItem(localizedText(QStringLiteral("punctuality_meticulous")));
    layout->addWidget(m_punctualityCombo);

    auto *adjustLabel = new QLabel(localizedText(QStringLiteral("adjust_label")), this);
    adjustLabel->setWordWrap(true);
    layout->addWidget(adjustLabel);

    m_adjustCombo = new QComboBox(this);
    m_adjustCombo->addItem(localizedText(QStringLiteral("adjust_neutral")));
    m_adjustCombo->addItem(localizedText(QStringLiteral("adjust_early")));
    m_adjustCombo->addItem(localizedText(QStringLiteral("adjust_late")));
    layout->addWidget(m_adjustCombo);

    auto *buttonLayout = new QHBoxLayout();
    m_setButton = new QPushButton(localizedText(QStringLiteral("set_button")), this);
    m_cancelButton = new QPushButton(localizedText(QStringLiteral("cancel_button")), this);
    buttonLayout->addWidget(m_setButton);
    buttonLayout->addWidget(m_cancelButton);
    layout->addLayout(buttonLayout);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    m_recommendationLabel = new QLabel(this);
    m_recommendationLabel->setWordWrap(true);
    m_recommendationLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(m_recommendationLabel);

    layout->addStretch();
}

void AlarmWindow::setupSystemTray()
{
#if QT_CONFIG(systemtrayicon)
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    if (!m_trayIcon) {
        m_trayIcon = new QSystemTrayIcon(this);
        QIcon trayIcon = systemClockIcon();
        if (trayIcon.isNull()) {
            trayIcon = windowIcon();
        }
        if (!trayIcon.isNull()) {
            m_trayIcon->setIcon(trayIcon);
        }

        m_trayMenu = new QMenu(this);
        m_showAction = m_trayMenu->addAction(localizedText(QStringLiteral("tray_show")), this, &AlarmWindow::showFromTray);
        m_trayMenu->addAction(localizedText(QStringLiteral("tray_cancel")), this, &AlarmWindow::cancelAlarm);
        m_quitAction = m_trayMenu->addAction(localizedText(QStringLiteral("tray_quit")), this, &AlarmWindow::quitFromTray);
        m_trayIcon->setContextMenu(m_trayMenu);

        connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
                showFromTray();
            }
        });

        m_trayIcon->show();
    }
#endif
}

void AlarmWindow::loadRepeatSelectionFromUi()
{
    m_repeatDays.clear();
    for (QCheckBox *check : m_repeatChecks) {
        if (!check) {
            continue;
        }
        if (check->isChecked()) {
            const int day = check->property("weekday").toInt();
            if (day >= Qt::Monday && day <= Qt::Sunday) {
                m_repeatDays.insert(day);
            }
        }
    }
}

QDateTime AlarmWindow::computeNextRepeatingOccurrence(const QDateTime &from) const
{
    if (m_repeatDays.isEmpty() || !m_timeEdit) {
        return QDateTime();
    }

    QDate baseDate;
    if (m_dateEdit && m_dateEdit->date().isValid()) {
        baseDate = m_dateEdit->date();
    }
    if (!baseDate.isValid()) {
        baseDate = from.date();
    }
    if (baseDate < from.date()) {
        baseDate = from.date();
    }

    const QTime alarmTime = m_timeEdit->time();
    QDate candidateDate = baseDate;

    for (int offset = 0; offset < 14; ++offset) {
        const QDate date = candidateDate.addDays(offset);
        if (!m_repeatDays.contains(date.dayOfWeek())) {
            continue;
        }

        QDateTime candidate(date, alarmTime);
        if (candidate <= from) {
            continue;
        }

        return candidate;
    }

    return QDateTime();
}

QString AlarmWindow::repeatSummary() const
{
    if (m_repeatDays.isEmpty()) {
        return localizedText(QStringLiteral("repeat_none"));
    }

    QList<int> days = m_repeatDays.values();
    std::sort(days.begin(), days.end());

    const QLocale locale = QLocale::system();
    QStringList parts;
    parts.reserve(days.size());
    for (int day : days) {
        parts.append(locale.dayName(day, QLocale::ShortFormat));
    }

    return localizedText(QStringLiteral("repeat_prefix")).arg(parts.join(localizedText(QStringLiteral("list_separator"))));
}

void AlarmWindow::pickSound()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        localizedText(QStringLiteral("pick_sound_button")),
        QString(),
        localizedText(QStringLiteral("audio_filter")));

    if (!fileName.isEmpty()) {
        m_soundPath = fileName;
        m_soundLabel->setText(QFileInfo(fileName).fileName());
    }

}

void AlarmWindow::scheduleAlarm()
{
    if (m_soundPath.isEmpty()) {
        QMessageBox::information(this,
                                 localizedText(QStringLiteral("sound_required_title")),
                                 localizedText(QStringLiteral("sound_required_body")));
        return;
    }

    if (!ensureSoundFileAccessible()) {
        return;
    }

    loadRepeatSelectionFromUi();

    const QDateTime now = QDateTime::currentDateTime();
    QDateTime target;

    if (!m_repeatDays.isEmpty()) {
        target = computeNextRepeatingOccurrence(now);
        if (!target.isValid()) {
            QMessageBox::warning(this,
                                 localizedText(QStringLiteral("repeat_failure_title")),
                                 localizedText(QStringLiteral("repeat_failure_body")));
            return;
        }
    } else {
        target = QDateTime(m_dateEdit->date(), m_timeEdit->time());
        if (!target.isValid()) {
            QMessageBox::warning(this,
                                 localizedText(QStringLiteral("time_error_title")),
                                 localizedText(QStringLiteral("time_error_body")));
            return;
        }
        while (target <= now) {
            target = target.addDays(1);
        }
    }

    const int offsetMinutes = preferenceOffsetMinutes();
    if (offsetMinutes != 0) {
        target = target.addSecs(static_cast<qint64>(offsetMinutes) * 60);
        while (target <= now) {
            target = target.addDays(1);
        }
    }

    const qint64 msecsUntil = now.msecsTo(target);
    if (msecsUntil <= 0) {
        QMessageBox::warning(this,
                             localizedText(QStringLiteral("time_error_title")),
                             localizedText(QStringLiteral("time_error_body")));
        return;
    }

    stopPlayback();
    m_alarmDateTime = target;
    m_alarmTimer->start(msecsUntil);
    if (!m_countdownTimer->isActive()) {
        m_countdownTimer->start();
    }

    clearWakeRequest();
    if (m_wakeCheck->isChecked()) {
        QString errorMessage;
        if (scheduleWakeFromSuspend(target, &errorMessage)) {
            m_wakeRequestActive = true;
        } else {
            QMessageBox::warning(this,
                                 localizedText(QStringLiteral("wake_failure_title")),
                                 errorMessage);
        }
    }

    refreshStatusLabel();
}

void AlarmWindow::cancelAlarm()
{
    if (m_alarmTimer->isActive()) {
        m_alarmTimer->stop();
    }
    if (m_countdownTimer->isActive()) {
        m_countdownTimer->stop();
    }

    m_alarmDateTime = QDateTime();
    clearWakeRequest();
    stopPlayback();
    refreshStatusLabel();
}

void AlarmWindow::updateCountdown()
{
    refreshStatusLabel();
}

void AlarmWindow::handleAlarmTriggered()
{
    m_countdownTimer->stop();
    clearWakeRequest();
    m_wakeRequestActive = false;
    refreshStatusLabel();

    if (!m_soundPath.isEmpty()) {
        startPlayback();
    }

    if (!m_repeatDays.isEmpty()) {
        const int offsetMinutes = preferenceOffsetMinutes();
        QDateTime from = m_alarmDateTime;
        if (offsetMinutes != 0) {
            from = from.addSecs(static_cast<qint64>(-offsetMinutes) * 60);
        }

        QDateTime candidateBase = computeNextRepeatingOccurrence(from.addSecs(1));
        bool scheduled = false;
        for (int attempt = 0; attempt < 14 && candidateBase.isValid(); ++attempt) {
            QDateTime adjusted = candidateBase;
            if (offsetMinutes != 0) {
                adjusted = adjusted.addSecs(static_cast<qint64>(offsetMinutes) * 60);
            }

            const qint64 msecsUntil = QDateTime::currentDateTime().msecsTo(adjusted);
            if (msecsUntil > 0) {
                m_alarmDateTime = adjusted;
                m_alarmTimer->start(msecsUntil);
                if (!m_countdownTimer->isActive()) {
                    m_countdownTimer->start();
                }
                if (m_wakeCheck->isChecked()) {
                    QString errorMessage;
                    if (scheduleWakeFromSuspend(adjusted, &errorMessage)) {
                        m_wakeRequestActive = true;
                    } else {
                        QMessageBox::warning(this,
                                             localizedText(QStringLiteral("next_repeat_failure_title")),
                                             errorMessage);
                    }
                }
                scheduled = true;
                break;
            }

            from = candidateBase;
            candidateBase = computeNextRepeatingOccurrence(from.addSecs(1));
        }
        if (!scheduled) {
            m_alarmDateTime = QDateTime();
        }
        refreshStatusLabel();
    }
}

void AlarmWindow::clearWakeRequest()
{
    if (!m_wakeRequestActive) {
        return;
    }

#if defined(Q_OS_LINUX)
    QFile clearFile(QStringLiteral("/sys/class/rtc/rtc0/wakealarm"));
    if (clearFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        clearFile.write("0\n");
    }
#endif
    m_wakeRequestActive = false;
}

bool AlarmWindow::scheduleWakeFromSuspend(const QDateTime &alarmTime, QString *errorMessage)
{
#if defined(Q_OS_LINUX)
    const QString wakePath = QStringLiteral("/sys/class/rtc/rtc0/wakealarm");

    QFile clearFile(wakePath);
    if (!clearFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            QFileInfo info(wakePath);
            if (!info.isWritable()) {
                *errorMessage = localizedText(QStringLiteral("wake_permission_error")).arg(wakePath);
            } else {
                *errorMessage = localizedText(QStringLiteral("wake_clear_failed")).arg(clearFile.errorString());
            }
        }
        return false;
    }
    clearFile.write("0\n");
    clearFile.close();

    QFile setFile(wakePath);
    if (!setFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            QFileInfo info(wakePath);
            if (!info.isWritable()) {
                *errorMessage = localizedText(QStringLiteral("wake_permission_error")).arg(wakePath);
            } else {
                *errorMessage = localizedText(QStringLiteral("wake_set_failed")).arg(setFile.errorString());
            }
        }
        return false;
    }

    const QByteArray seconds = QByteArray::number(alarmTime.toSecsSinceEpoch());
    if (setFile.write(seconds + "\n") == -1) {
        if (errorMessage) {
            *errorMessage = localizedText(QStringLiteral("wake_set_failed")).arg(setFile.errorString());
        }
        return false;
    }
    setFile.close();
    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
#elif defined(Q_OS_ANDROID)
    Q_UNUSED(alarmTime);
    if (errorMessage) {
        *errorMessage = localizedText(QStringLiteral("wake_android_note"));
    }
    return false;
#else
    Q_UNUSED(alarmTime);
    if (errorMessage) {
        *errorMessage = localizedText(QStringLiteral("wake_platform_note"));
    }
    return false;
#endif
}

bool AlarmWindow::ensureSoundFileAccessible()
{
    if (m_soundPath.isEmpty()) {
        return false;
    }

    QFileInfo info(m_soundPath);
    if (!info.exists()) {
        QMessageBox::warning(this,
                             localizedText(QStringLiteral("sound_missing_title")),
                             localizedText(QStringLiteral("sound_missing_body")));
        m_soundPath.clear();
        if (m_soundLabel) {
            m_soundLabel->setText(localizedText(QStringLiteral("no_sound_selected")));
        }
        return false;
    }

    return true;
}

void AlarmWindow::startPlayback()
{
    if (!ensureSoundFileAccessible()) {
        return;
    }

    stopPlayback();

    m_player->setSource(QUrl::fromLocalFile(m_soundPath));
    m_loopPlayback = true;
    m_player->play();
    refreshStatusLabel();
}

void AlarmWindow::showPlaybackError(const QString &errorText)
{
    stopPlayback();
    QMessageBox::warning(this,
                         localizedText(QStringLiteral("playback_error_title")),
                         localizedText(QStringLiteral("playback_error_body")).arg(errorText));
    refreshStatusLabel();
}

void AlarmWindow::stopPlayback()
{
    const QMediaPlayer::PlaybackState state = m_player->playbackState();
    if (state != QMediaPlayer::StoppedState) {
        m_player->stop();
    }
    m_loopPlayback = false;
}

void AlarmWindow::refreshStatusLabel()
{
    const QMediaPlayer::PlaybackState playbackState = m_player->playbackState();
    if (playbackState == QMediaPlayer::PlayingState) {
        m_statusLabel->setText(localizedText(QStringLiteral("status_playing")));
        return;
    }

    if (m_alarmTimer->isActive()) {
        const QDateTime now = QDateTime::currentDateTime();
        qint64 seconds = now.secsTo(m_alarmDateTime);
        if (seconds < 0) {
            seconds = 0;
        }
        const qint64 hours = seconds / 3600;
        const qint64 minutes = (seconds % 3600) / 60;
        const qint64 secs = seconds % 60;

        QString durationText = localizedText(QStringLiteral("time_hms_format"))
                                   .arg(static_cast<qlonglong>(hours))
                                   .arg(static_cast<qlonglong>(minutes))
                                   .arg(static_cast<qlonglong>(secs));

        QString message = localizedText(QStringLiteral("status_time_remaining")).arg(durationText);
        message += QLatin1Char('\n');
        message += localizedText(QStringLiteral("status_scheduled_time"))
                        .arg(m_alarmDateTime.toString(localizedText(QStringLiteral("schedule_time_format"))));
        if (!m_repeatDays.isEmpty()) {
            message += QLatin1Char('\n');
            message += repeatSummary();
        }
        if (m_wakeRequestActive) {
            message += QLatin1Char('\n');
            message += localizedText(QStringLiteral("status_wake_active"));
        }
        m_statusLabel->setText(message);
    } else {
        m_statusLabel->setText(localizedText(QStringLiteral("status_none")));
    }
}

void AlarmWindow::updateRecommendations()
{
    if (!m_recommendationLabel || !m_timeEdit) {
        return;
    }

    const QTime baseTime = m_timeEdit->time();
    if (!baseTime.isValid()) {
        m_recommendationLabel->setText(localizedText(QStringLiteral("recommendation_unavailable")));
        return;
    }

    const int punctualityIndex = m_punctualityCombo ? m_punctualityCombo->currentIndex() : 0;
    double stddevMinutes = (punctualityIndex == 0) ? 18.0 : 10.0;
    if (stddevMinutes < 4.0) {
        stddevMinutes = 4.0;
    }

    auto preferenceMean = [&](bool wantEarlier) -> double {
        const int adjustIndex = m_adjustCombo ? m_adjustCombo->currentIndex() : 0;
        switch (adjustIndex) {
        case 1: // A little earlier
            return wantEarlier ? -6.0 : 3.0;
        case 2: // A little later
            return wantEarlier ? -3.0 : 6.0;
        default:
            return wantEarlier ? -2.0 : 2.0;
        }
    };

    auto sampleOffset = [&](bool wantEarlier, bool wantLarge) -> int {
        const double mean = preferenceMean(wantEarlier);
        const double minMagnitude = wantLarge ? stddevMinutes * 0.9 : stddevMinutes * 0.25;
        const double maxMagnitude = wantLarge ? stddevMinutes * 3.0 : stddevMinutes * 1.1;

        std::mt19937 generator(static_cast<std::mt19937::result_type>(QRandomGenerator::global()->generate64()));
        std::normal_distribution<double> dist(mean, stddevMinutes);

        for (int attempt = 0; attempt < 32; ++attempt) {
            double value = dist(generator);
            double magnitude = std::fabs(value);
            if (magnitude < minMagnitude || magnitude > maxMagnitude) {
                continue;
            }
            if (wantEarlier) {
                value = -std::fabs(value);
            } else {
                value = std::fabs(value);
            }
            int rounded = static_cast<int>(std::round(value));
            if (rounded == 0) {
                continue;
            }
            return rounded;
        }

        int fallback = wantLarge ? static_cast<int>(stddevMinutes * 2.0) : static_cast<int>(stddevMinutes * 0.6);
        if (fallback == 0) {
            fallback = 3;
        }
        return wantEarlier ? -fallback : fallback;
    };

    const int earlierLarge = sampleOffset(true, true);
    const int laterSmall = sampleOffset(false, false);

    const QTime earlierTime = baseTime.addSecs(static_cast<int>(earlierLarge) * 60);
    const QTime laterTime = baseTime.addSecs(static_cast<int>(laterSmall) * 60);

    const QLocale locale = QLocale::system();
    const QString earlierTimeText = locale.toString(earlierTime, QLocale::ShortFormat);
    const QString laterTimeText = locale.toString(laterTime, QLocale::ShortFormat);

    const QString earlierOffsetText = localizedText(earlierLarge < 0 ? QStringLiteral("recommendation_offset_earlier")
                                                                   : QStringLiteral("recommendation_offset_later"))
                                          .arg(std::abs(earlierLarge));
    const QString laterOffsetText = localizedText(laterSmall < 0 ? QStringLiteral("recommendation_offset_earlier")
                                                               : QStringLiteral("recommendation_offset_later"))
                                        .arg(std::abs(laterSmall));

    QStringList lines;
    lines << localizedText(QStringLiteral("recommendation_title"));
    lines << localizedText(QStringLiteral("recommendation_line"))
                    .arg(localizedText(QStringLiteral("recommendation_earlier_big")))
                    .arg(earlierTimeText)
                    .arg(earlierOffsetText);
    lines << localizedText(QStringLiteral("recommendation_line"))
                    .arg(localizedText(QStringLiteral("recommendation_later_small")))
                    .arg(laterTimeText)
                    .arg(laterOffsetText);

    m_recommendationLabel->setText(lines.join(QLatin1Char('\n')));
}

int AlarmWindow::preferenceOffsetMinutes() const
{
    const int adjustIndex = m_adjustCombo ? m_adjustCombo->currentIndex() : 0;
    if (adjustIndex == 0) {
        return 0;
    }

    const int punctualityIndex = m_punctualityCombo ? m_punctualityCombo->currentIndex() : 0;
    const int baseMagnitude = (punctualityIndex == 0) ? 8 : 4;

    if (adjustIndex == 1) {
        return -baseMagnitude;
    }
    if (adjustIndex == 2) {
        return baseMagnitude;
    }

    return 0;
}

void AlarmWindow::showFromTray()
{
    show();
    raise();
    activateWindow();
}

void AlarmWindow::quitFromTray()
{
    QCoreApplication::quit();
}

void AlarmWindow::closeEvent(QCloseEvent *event)
{
#if QT_CONFIG(systemtrayicon)
    if (m_trayIcon && m_trayIcon->isVisible()) {
        hide();
        if (!m_trayMessageShown) {
            m_trayIcon->showMessage(localizedText(QStringLiteral("tray_message_title")),
                                    localizedText(QStringLiteral("tray_message_body")),
                                    QSystemTrayIcon::Information,
                                    3000);
            m_trayMessageShown = true;
        }
        event->ignore();
        return;
    }
#endif
    QWidget::closeEvent(event);
}
