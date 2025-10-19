#include "alarmwindow.h"

#include <QAbstractSpinBox>
#include <QAudioOutput>
#include <QCheckBox>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QPushButton>
#include <QTimeEdit>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

AlarmWindow::AlarmWindow(QWidget *parent)
    : QWidget(parent),
      m_statusLabel(nullptr),
      m_soundLabel(nullptr),
      m_timeEdit(nullptr),
      m_wakeCheck(nullptr),
      m_setButton(nullptr),
      m_cancelButton(nullptr),
      m_pickSoundButton(nullptr),
      m_countdownTimer(new QTimer(this)),
      m_alarmTimer(new QTimer(this)),
      m_player(new QMediaPlayer(this)),
      m_audioOutput(new QAudioOutput(this)),
      m_wakeRequestActive(false)
{
    buildUi();

    m_audioOutput->setVolume(0.8f);
    m_player->setAudioOutput(m_audioOutput);

    m_alarmTimer->setSingleShot(true);
    connect(m_alarmTimer, &QTimer::timeout, this, &AlarmWindow::handleAlarmTriggered);

    m_countdownTimer->setInterval(1000);
    connect(m_countdownTimer, &QTimer::timeout, this, &AlarmWindow::updateCountdown);

    connect(m_pickSoundButton, &QPushButton::clicked, this, &AlarmWindow::pickSound);
    connect(m_setButton, &QPushButton::clicked, this, &AlarmWindow::scheduleAlarm);
    connect(m_cancelButton, &QPushButton::clicked, this, &AlarmWindow::cancelAlarm);

    refreshStatusLabel();
}

void AlarmWindow::buildUi()
{
    setWindowTitle(tr("Achim Alarm"));
    setMinimumWidth(320);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    auto *title = new QLabel(tr("간단한 알람"), this);
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    m_timeEdit = new QTimeEdit(QTime::currentTime(), this);
    m_timeEdit->setDisplayFormat("hh:mm");
    m_timeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_timeEdit->setAlignment(Qt::AlignCenter);
    m_timeEdit->setFixedHeight(48);
    QFont timeFont = m_timeEdit->font();
    timeFont.setPointSize(24);
    m_timeEdit->setFont(timeFont);
    layout->addWidget(m_timeEdit, 0, Qt::AlignCenter);

    m_pickSoundButton = new QPushButton(tr("알람 소리 선택"), this);
    layout->addWidget(m_pickSoundButton);

    m_soundLabel = new QLabel(tr("선택된 파일 없음"), this);
    m_soundLabel->setWordWrap(true);
    m_soundLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_soundLabel);

    m_wakeCheck = new QCheckBox(tr("절전모드에서도 깨우기 시도"), this);
    layout->addWidget(m_wakeCheck, 0, Qt::AlignCenter);

    auto *buttonLayout = new QHBoxLayout();
    m_setButton = new QPushButton(tr("알람 설정"), this);
    m_cancelButton = new QPushButton(tr("해제"), this);
    buttonLayout->addWidget(m_setButton);
    buttonLayout->addWidget(m_cancelButton);
    layout->addLayout(buttonLayout);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    layout->addStretch();
}

void AlarmWindow::pickSound()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("알람 소리 선택"),
        QString(),
        tr("오디오 파일 (*.mp3 *.wav *.ogg);;모든 파일 (*.*)"));

    if (!fileName.isEmpty()) {
        m_soundPath = fileName;
        m_soundLabel->setText(QFileInfo(fileName).fileName());
    }

}

void AlarmWindow::scheduleAlarm()
{
    if (m_soundPath.isEmpty()) {
        QMessageBox::information(this, tr("알람 소리 필요"), tr("알람 소리를 먼저 선택해주세요."));
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();
    QDateTime target(now.date(), m_timeEdit->time());
    if (target <= now) {
        target = target.addDays(1);
    }

    const qint64 msecsUntil = now.msecsTo(target);
    if (msecsUntil <= 0) {
        QMessageBox::warning(this, tr("시간 오류"), tr("알람 시간을 다시 확인해주세요."));
        return;
    }

    m_alarmDateTime = target;
    m_alarmTimer->start(msecsUntil);
    if (!m_countdownTimer->isActive()) {
        m_countdownTimer->start();
    }

    clearWakeRequest();
    if (m_wakeCheck->isChecked()) {
        if (scheduleWakeFromSuspend(target)) {
            m_wakeRequestActive = true;
        } else {
            QMessageBox::warning(this, tr("절전 깨우기 실패"),
                                 tr("시스템 절전 예약에 실패했습니다. 관리자 권한이 필요할 수 있습니다."));
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
    m_wakeRequestActive = false;
    refreshStatusLabel();

    if (!m_soundPath.isEmpty()) {
        m_player->setSource(QUrl::fromLocalFile(m_soundPath));
        m_player->play();
    }
}

bool AlarmWindow::scheduleWakeFromSuspend(const QDateTime &alarmTime)
{
    QFile clearFile(QStringLiteral("/sys/class/rtc/rtc0/wakealarm"));
    if (!clearFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    clearFile.write("0\n");
    clearFile.close();

    QFile setFile(QStringLiteral("/sys/class/rtc/rtc0/wakealarm"));
    if (!setFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    const QByteArray seconds = QByteArray::number(alarmTime.toSecsSinceEpoch());
    if (setFile.write(seconds + "\n") == -1) {
        return false;
    }
    setFile.close();
    return true;
}

void AlarmWindow::clearWakeRequest()
{
    if (!m_wakeRequestActive) {
        return;
    }

    QFile clearFile(QStringLiteral("/sys/class/rtc/rtc0/wakealarm"));
    if (clearFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        clearFile.write("0\n");
    }
    m_wakeRequestActive = false;
}

void AlarmWindow::stopPlayback()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->stop();
    }
}

void AlarmWindow::refreshStatusLabel()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_statusLabel->setText(tr("알람이 울리는 중입니다."));
        return;
    }

    if (m_alarmTimer->isActive()) {
        const QDateTime now = QDateTime::currentDateTime();
        qint64 seconds = now.secsTo(m_alarmDateTime);
        if (seconds < 0) {
            seconds = 0;
        }
        const QTime remaining = QTime(0, 0).addSecs(static_cast<int>(seconds));

        QString message = tr("알람까지 %1 남았습니다.")
                               .arg(remaining.toString("hh시간 mm분 ss초"));
        message += QLatin1Char('\n');
        message += tr("예정 시각: %1")
                        .arg(m_alarmDateTime.toString("yyyy-MM-dd hh:mm"));
        if (m_wakeRequestActive) {
            message += QLatin1Char('\n');
            message += tr("절전모드 해제 요청이 설정되었습니다.");
        }
        m_statusLabel->setText(message);
    } else {
        m_statusLabel->setText(tr("알람이 설정되지 않았습니다."));
    }
}
