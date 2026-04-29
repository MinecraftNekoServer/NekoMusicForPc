/**
 * @file desktoplrc.cpp
 * @brief 桌面歌词显示实现
 */

#include "desktoplrc.h"
#include <QFile>
#include <QTextStream>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QRegularExpression>
#include <QStringConverter>

DesktopLrc::DesktopLrc(QWidget *parent)
    : QWidget(parent)
{
    // 初始化桌面歌词窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // 设置默认大小
    resize(600, 80);
    
    // 设置默认位置（屏幕底部居中）
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move((screenGeometry.width() - width()) / 2, 
         screenGeometry.height() - height() - 50);
    
    // 初始化显示设置
    m_font = QFont("Microsoft YaHei", 20, QFont::Bold);
    m_textColor = Qt::white;
    m_backgroundColor = QColor(0, 0, 0, 180);
    
    // 创建更新定时器
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &DesktopLrc::updateLyricDisplay);
    m_updateTimer->start(100); // 每100ms更新一次
}

DesktopLrc::~DesktopLrc()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
        delete m_updateTimer;
    }
}

void DesktopLrc::setLyrics(const QString &lyrics)
{
    m_currentLyrics = lyrics;
    update();
}

void DesktopLrc::loadLyricsFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_currentLyrics = tr("无法加载歌词文件");
        return;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    QString lyricsText = stream.readAll();
    file.close();
    
    parseLyrics(lyricsText);
}

void DesktopLrc::parseLyrics(const QString &lyricsText)
{
    m_lyricsMap.clear();
    
    QStringList lines = lyricsText.split('\n');
    QRegularExpression timeRegex("\\[(\\d+):(\\d+)\\.(\\d+)\\]");
    
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty()) continue;
        
        // 匹配时间标签
        QRegularExpressionMatchIterator matches = timeRegex.globalMatch(line);
        QString lyricText = line;
        
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            lyricText = lyricText.remove(match.captured(0));
        }
        
        lyricText = lyricText.trimmed();
        if (lyricText.isEmpty()) continue;
        
        // 为每个时间标签添加歌词
        matches = timeRegex.globalMatch(line);
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            int minutes = match.captured(1).toInt();
            int seconds = match.captured(2).toInt();
            int milliseconds = match.captured(3).toInt();
            
            qint64 timeMs = minutes * 60000 + seconds * 1000 + milliseconds;
            m_lyricsMap[timeMs] = lyricText;
        }
    }
    
    if (m_lyricsMap.isEmpty()) {
        m_currentLyrics = tr("无可用歌词");
    }
}

void DesktopLrc::updatePosition(qint64 position)
{
    m_currentPosition = position;
}

void DesktopLrc::setCurrentSong(const QString &title, const QString &artist)
{
    m_currentSongTitle = title;
    m_currentSongArtist = artist;
    m_currentLyrics = QString("%1 - %2").arg(title).arg(artist);
    update();
}

QString DesktopLrc::getLyricAtTime(qint64 timeMs) const
{
    if (m_lyricsMap.isEmpty()) {
        return m_currentLyrics;
    }
    
    // 找到当前时间对应的歌词
    QString lyric;
    qint64 lastTime = 0;
    
    for (auto it = m_lyricsMap.constBegin(); it != m_lyricsMap.constEnd(); ++it) {
        if (it.key() <= timeMs) {
            lyric = it.value();
            lastTime = it.key();
        } else {
            break;
        }
    }
    
    return lyric.isEmpty() ? m_currentLyrics : lyric;
}

void DesktopLrc::updateLyricDisplay()
{
    if (!m_isVisible) return;
    
    QString lyric = getLyricAtTime(m_currentPosition);
    if (lyric != m_currentLyrics) {
        m_currentLyrics = lyric;
        update();
    }
}

void DesktopLrc::showWindow()
{
    m_isVisible = true;
    show();
    raise();
}

void DesktopLrc::hideWindow()
{
    m_isVisible = false;
    hide();
}

void DesktopLrc::toggleWindow()
{
    if (m_isVisible) {
        hideWindow();
    } else {
        showWindow();
    }
}

void DesktopLrc::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制半透明背景
    painter.fillRect(rect(), m_backgroundColor);
    
    // 绘制歌词
    painter.setPen(m_textColor);
    painter.setFont(m_font);
    painter.drawText(rect(), Qt::AlignCenter, m_currentLyrics);
    
    // 绘制歌曲信息（如果歌词为空）
    if (m_currentLyrics.isEmpty() || m_currentLyrics == tr("无可用歌词")) {
        QString songInfo = QString("%1 - %2").arg(m_currentSongTitle).arg(m_currentSongArtist);
        painter.setFont(QFont("Microsoft YaHei", 14, QFont::Normal));
        painter.drawText(rect(), Qt::AlignCenter | Qt::AlignBottom, songInfo);
    }
}

void DesktopLrc::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void DesktopLrc::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void DesktopLrc::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}

void DesktopLrc::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    
    QAction *showAction = menu.addAction(tr("显示桌面歌词"));
    showAction->setCheckable(true);
    showAction->setChecked(m_isVisible);
    connect(showAction, &QAction::triggered, this, &DesktopLrc::toggleWindow);
    
    menu.addSeparator();
    
    QAction *fontAction = menu.addAction(tr("字体设置"));
    QAction *colorAction = menu.addAction(tr("颜色设置"));
    
    menu.addSeparator();
    
    QAction *closeAction = menu.addAction(tr("关闭"));
    
    QAction *selectedAction = menu.exec(event->globalPos());
    
    if (selectedAction == closeAction) {
        hideWindow();
    }
}