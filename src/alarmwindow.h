#ifndef ALARMWINDOW_H
#define ALARMWINDOW_H

#include <QDateTime>
#include <QSet>
#include <QVector>
#include <QWidget>

class QLabel;
class QPushButton;
class QTimeEdit;
class QCheckBox;
class QTimer;
class QMediaPlayer;
class QAudioOutput;
class QDateEdit;
class QSystemTrayIcon;
class QMenu;
class QAction;
class QCloseEvent;
class QComboBox;

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
    void showFromTray();
    void quitFromTray();

private:
    void buildUi();
    void setupSystemTray();
    void loadRepeatSelectionFromUi();
    QDateTime computeNextRepeatingOccurrence(const QDateTime &from) const;
    QString repeatSummary() const;
    bool scheduleWakeFromSuspend(const QDateTime &alarmTime, QString *errorMessage = nullptr);
    void clearWakeRequest();
    void stopPlayback();
    void refreshStatusLabel();
    void updateRecommendations();
    int preferenceOffsetMinutes() const;
    void closeEvent(QCloseEvent *event) override;

    QLabel *m_statusLabel;
    QLabel *m_soundLabel;
    QTimeEdit *m_timeEdit;
    QDateEdit *m_dateEdit;
    QCheckBox *m_wakeCheck;
    QPushButton *m_setButton;
    QPushButton *m_cancelButton;
    QPushButton *m_pickSoundButton;
    QComboBox *m_punctualityCombo;
    QComboBox *m_adjustCombo;
    QLabel *m_recommendationLabel;
    QVector<QCheckBox *> m_repeatChecks;

    QTimer *m_countdownTimer;
    QTimer *m_alarmTimer;

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_showAction;
    QAction *m_quitAction;

    QDateTime m_alarmDateTime;
    QString m_soundPath;
    bool m_wakeRequestActive;
    bool m_trayMessageShown;
    QSet<int> m_repeatDays;
};

#endif // ALARMWINDOW_H
