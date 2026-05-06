#pragma once

/**
 * @file settingspage.h
 * @brief 设置页面
 */

#include <QWidget>
#include "theme/thememanager.h"

class QComboBox;
class QLabel;
class QPushButton;

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

signals:
    void languageChanged(int language);
    void checkForUpdatesRequested();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    void retranslate();

    QComboBox *m_langCombo = nullptr;
    QComboBox *m_themeCombo = nullptr;
    QLabel *m_langLabel = nullptr;
    QLabel *m_versionLabel = nullptr;
    QLabel *m_systemLabel = nullptr;
    QPushButton *m_checkUpdateBtn = nullptr;
};
