#include "alarmwindow.h"

#include <QAbstractSpinBox>
#include <QAudioOutput>
#include <QAction>
#include <QAudio>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDateEdit>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
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
#include <QPushButton>
#include <QTimeEdit>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QSystemTrayIcon>
#include <QtGlobal>
#include <QApplication>
#include <QCoreApplication>

#include <algorithm>

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
      m_countdownTimer(new QTimer(this)),
      m_alarmTimer(new QTimer(this)),
      m_player(new QMediaPlayer(this)),
      m_audioOutput(new QAudioOutput(this)),
      m_trayIcon(nullptr),
      m_trayMenu(nullptr),
      m_showAction(nullptr),
      m_quitAction(nullptr),
      m_wakeRequestActive(false),
      m_trayMessageShown(false)
{
    buildUi();

    m_audioOutput->setVolume(0.8f);
    m_player->setAudioRole(QAudio::MusicRole);
    m_player->setAudioOutput(m_audioOutput);

    m_alarmTimer->setSingleShot(true);
    connect(m_alarmTimer, &QTimer::timeout, this, &AlarmWindow::handleAlarmTriggered);

    m_countdownTimer->setInterval(1000);
    connect(m_countdownTimer, &QTimer::timeout, this, &AlarmWindow::updateCountdown);

    connect(m_pickSoundButton, &QPushButton::clicked, this, &AlarmWindow::pickSound);
    connect(m_setButton, &QPushButton::clicked, this, &AlarmWindow::scheduleAlarm);
    connect(m_cancelButton, &QPushButton::clicked, this, &AlarmWindow::cancelAlarm);

    setupSystemTray();

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
    auto *dateLabel = new QLabel(tr("알람 날짜"), this);
    dateRow->addWidget(dateLabel);
    m_dateEdit = new QDateEdit(QDate::currentDate(), this);
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    m_dateEdit->setCalendarPopup(true);
    dateRow->addWidget(m_dateEdit, 1);
    layout->addLayout(dateRow);

    auto *repeatGroup = new QGroupBox(tr("요일 반복"), this);
    auto *repeatLayout = new QGridLayout(repeatGroup);
    repeatLayout->setSpacing(6);
    const QLocale locale;
    for (int day = Qt::Monday; day <= Qt::Sunday; ++day) {
        auto *check = new QCheckBox(locale.dayName(day, QLocale::ShortFormat), repeatGroup);
        check->setProperty("weekday", day);
        const int index = day - Qt::Monday;
        const int row = index / 4;
        const int column = index % 4;
        repeatLayout->addWidget(check, row, column);
        m_repeatChecks.append(check);
    }
    auto *repeatHint = new QLabel(tr("선택된 요일에 매주 알람이 반복됩니다. 선택하지 않으면 지정한 날짜의 1회성 알람으로 동작합니다."), repeatGroup);
    repeatHint->setWordWrap(true);
    repeatLayout->addWidget(repeatHint, 2, 0, 1, 4);
    layout->addWidget(repeatGroup);

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

void AlarmWindow::setupSystemTray()
{
#if QT_CONFIG(systemtrayicon)
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    if (!m_trayIcon) {
        m_trayIcon = new QSystemTrayIcon(this);
        QIcon trayIcon = QIcon::fromTheme(QStringLiteral("alarm"));
        if (trayIcon.isNull()) {
            trayIcon = windowIcon();
        }
        if (!trayIcon.isNull()) {
            m_trayIcon->setIcon(trayIcon);
        }

        m_trayMenu = new QMenu(this);
        m_showAction = m_trayMenu->addAction(tr("창 열기"), this, &AlarmWindow::showFromTray);
        m_trayMenu->addAction(tr("알람 해제"), this, &AlarmWindow::cancelAlarm);
        m_quitAction = m_trayMenu->addAction(tr("앱 종료"), this, &AlarmWindow::quitFromTray);
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
        return tr("반복 없음");
    }

    QList<int> days = m_repeatDays.values();
    std::sort(days.begin(), days.end());

    const QLocale locale;
    QStringList parts;
    parts.reserve(days.size());
    for (int day : days) {
        parts.append(locale.dayName(day, QLocale::ShortFormat));
    }

    return tr("매주 %1 반복").arg(parts.join(tr(", ")));
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

    loadRepeatSelectionFromUi();

    const QDateTime now = QDateTime::currentDateTime();
    QDateTime target;

    if (!m_repeatDays.isEmpty()) {
        target = computeNextRepeatingOccurrence(now);
        if (!target.isValid()) {
            QMessageBox::warning(this, tr("반복 예약 실패"), tr("선택한 요일로 다음 알람 시간을 계산할 수 없습니다."));
            return;
        }
    } else {
        target = QDateTime(m_dateEdit->date(), m_timeEdit->time());
        if (!target.isValid()) {
            QMessageBox::warning(this, tr("시간 오류"), tr("알람 시간을 다시 확인해주세요."));
            return;
        }
        while (target <= now) {
            target = target.addDays(1);
        }
    }

    const qint64 msecsUntil = now.msecsTo(target);
    if (msecsUntil <= 0) {
        QMessageBox::warning(this, tr("시간 오류"), tr("알람 시간을 다시 확인해주세요."));
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
            QMessageBox::warning(this, tr("절전 깨우기 실패"), errorMessage);
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
        m_player->setSource(QUrl::fromLocalFile(m_soundPath));
        m_player->play();
    }

    if (!m_repeatDays.isEmpty()) {
        const QDateTime next = computeNextRepeatingOccurrence(m_alarmDateTime.addSecs(1));
        if (next.isValid()) {
            m_alarmDateTime = next;
            const qint64 msecsUntil = QDateTime::currentDateTime().msecsTo(next);
            if (msecsUntil > 0) {
                m_alarmTimer->start(msecsUntil);
                if (!m_countdownTimer->isActive()) {
                    m_countdownTimer->start();
                }
                if (m_wakeCheck->isChecked()) {
                    QString errorMessage;
                    if (scheduleWakeFromSuspend(next, &errorMessage)) {
                        m_wakeRequestActive = true;
                    } else {
                        QMessageBox::warning(this, tr("다음 반복 예약 실패"), errorMessage);
                    }
                }
            }
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
                *errorMessage = tr("%1에 기록 권한이 없습니다. 관리자 권한으로 실행하거나 udev 규칙을 추가해주세요.").arg(wakePath);
            } else {
                *errorMessage = tr("RTC 알람 초기화에 실패했습니다: %1").arg(clearFile.errorString());
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
                *errorMessage = tr("%1에 기록 권한이 없습니다. 관리자 권한으로 실행하거나 udev 규칙을 추가해주세요.").arg(wakePath);
            } else {
                *errorMessage = tr("RTC 알람 예약에 실패했습니다: %1").arg(setFile.errorString());
            }
        }
        return false;
    }

    const QByteArray seconds = QByteArray::number(alarmTime.toSecsSinceEpoch());
    if (setFile.write(seconds + "\n") == -1) {
        if (errorMessage) {
            *errorMessage = tr("RTC 알람 예약에 실패했습니다: %1").arg(setFile.errorString());
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
        *errorMessage = tr("Android에서는 시스템 절전 해제를 위해 별도의 백그라운드 서비스가 필요합니다. 현재 버전은 앱이 실행 중일 때만 동작합니다.");
    }
    return false;
#else
    Q_UNUSED(alarmTime);
    if (errorMessage) {
        *errorMessage = tr("이 플랫폼에서는 시스템 절전 해제를 지원하지 않습니다.");
    }
    return false;
#endif
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
        if (!m_repeatDays.isEmpty()) {
            message += QLatin1Char('\n');
            message += repeatSummary();
        }
        if (m_wakeRequestActive) {
            message += QLatin1Char('\n');
            message += tr("절전모드 해제 요청이 설정되었습니다.");
        }
        m_statusLabel->setText(message);
    } else {
        m_statusLabel->setText(tr("알람이 설정되지 않았습니다."));
    }
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
            m_trayIcon->showMessage(tr("백그라운드 실행"),
                                    tr("창을 닫아도 알람은 계속 작동합니다. 트레이 아이콘을 통해 다시 열 수 있습니다."),
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
