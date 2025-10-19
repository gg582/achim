#ifndef ALARMWINDOW_H
#define ALARMWINDOW_H

#include <QWidget>
#include <QDateTime>

class QLabel;
class QPushButton;
class QTimeEdit;
class QCheckBox;
class QTimer;
class QMediaPlayer;
class QAudioOutput;

class AlarmWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AlarmWindow(QWidget *parent = nullptr);

private slots:
    void pickSound();
    void scheduleAlarm();
    void cancelAlarm();
    void updateCountdown();
    void handleAlarmTriggered();

private:
    void buildUi();
    bool scheduleWakeFromSuspend(const QDateTime &alarmTime);
    void clearWakeRequest();
    void stopPlayback();
    void refreshStatusLabel();

    QLabel *m_statusLabel;
    QLabel *m_soundLabel;
    QTimeEdit *m_timeEdit;
    QCheckBox *m_wakeCheck;
    QPushButton *m_setButton;
    QPushButton *m_cancelButton;
    QPushButton *m_pickSoundButton;

    QTimer *m_countdownTimer;
    QTimer *m_alarmTimer;

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;

    QDateTime m_alarmDateTime;
    QString m_soundPath;
    bool m_wakeRequestActive;
};

#endif // ALARMWINDOW_H
