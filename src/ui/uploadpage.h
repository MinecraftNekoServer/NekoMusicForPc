#pragma once

/**
 * @file uploadpage.h
 * @brief 上传音乐页面
 */

#include <QWidget>

class QLabel;
class QLineEdit;
class QTextEdit;
class QComboBox;
class QPushButton;
class ApiClient;

class UploadPage : public QWidget
{
    Q_OBJECT

public:
    explicit UploadPage(QWidget *parent = nullptr);

signals:
    void backRequested();

public slots:
    void retranslate();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    void selectMusicFile();
    void selectCoverFile();
    void selectLyricsFile();
    void doUpload();
    void onUploadResult(bool success, const QString &message);
    void updateCoverPreview();
    void updateLyricsPreview();

    ApiClient *m_api = nullptr;

    // File paths
    QString m_musicFilePath;
    QString m_coverFilePath;
    QString m_lyricsFilePath;

    // UI elements
    QLabel *m_titleLabel = nullptr;
    QLabel *m_descLabel = nullptr;
    QPushButton *m_backBtn = nullptr;

    // Basic info
    QLineEdit *m_titleEdit = nullptr;
    QLineEdit *m_artistEdit = nullptr;
    QComboBox *m_langCombo = nullptr;
    QLineEdit *m_albumEdit = nullptr;
    QLineEdit *m_tagsEdit = nullptr;
    QLineEdit *m_durationEdit = nullptr;

    // File selection
    QPushButton *m_musicFileBtn = nullptr;
    QLabel *m_musicFileLabel = nullptr;
    QPushButton *m_coverFileBtn = nullptr;
    QLabel *m_coverPreview = nullptr;
    QPushButton *m_lyricsFileBtn = nullptr;
    QTextEdit *m_lyricsPreview = nullptr;

    // Upload
    QPushButton *m_uploadBtn = nullptr;
};
