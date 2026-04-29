#pragma once

/**
 * @file settingspage.h
 * @brief 设置页面
 */

#include <QWidget>

class QComboBox;
class QLabel;
class QPushButton;
class QCheckBox;

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

signals:
    void languageChanged(int language);
    void checkForUpdatesRequested();
    void desktopLyricsChanged(bool enabled);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    void retranslate();

    QComboBox *m_langCombo = nullptr;
    QLabel *m_langLabel = nullptr;
    QLabel *m_versionLabel = nullptr;
    QLabel *m_systemLabel = nullptr;
    QPushButton *m_checkUpdateBtn = nullptr;
    QCheckBox *m_desktopLyricsCheck = nullptr;
};
