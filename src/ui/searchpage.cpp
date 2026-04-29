/**
 * @file searchpage.cpp
 * @brief 搜索页面实现
 */

#include "searchpage.h"
#include "core/apiclient.h"
#include "core/i18n.h"
#include "core/covercache.h"
#include "theme/theme.h"
#include "ui/svgicon.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QMouseEvent>
#include <QMenu>

// ─── 音乐结果卡片（复用 MusicItemCard 样式）──────────────
class SearchResultCard : public QWidget
{
public:
    explicit SearchResultCard(const MusicInfo &info, QWidget *parent = nullptr)
        : QWidget(parent), m_musicId(info.id), m_info(info)
    {
        setFixedHeight(70);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, false);

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(12, 8, 12, 8);
        lay->setSpacing(14);

        // 封面
        m_coverLbl = new QLabel(this);
        m_coverLbl->setFixedSize(54, 54);
        m_coverLbl->setScaledContents(false);
        loadCover();
        lay->addWidget(m_coverLbl);

        // 信息
        auto *infoV = new QWidget(this);
        auto *infoLay = new QVBoxLayout(infoV);
        infoLay->setContentsMargins(0, 0, 0, 0);
        infoLay->setSpacing(4);

        m_titleLbl = new QLabel(info.title, infoV);
        m_titleLbl->setStyleSheet("QLabel { font-size: 14px; font-weight: 600; color: " + QString(Theme::kTextMain) + "; }");
        infoLay->addWidget(m_titleLbl);

        m_artistLbl = new QLabel(info.artist, infoV);
        m_artistLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextSub) + "; }");
        infoLay->addWidget(m_artistLbl);

        infoLay->addStretch();
        lay->addWidget(infoV, 1);

        // 时长
        if (info.duration > 0) {
            int mins = info.duration / 60;
            int secs = info.duration % 60;
            auto *timeLbl = new QLabel(
                QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')), this);
            timeLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextMuted) + "; }");
            lay->addWidget(timeLbl);
        }
    }

    int musicId() const { return m_musicId; }
    const MusicInfo& info() const { return m_info; }

    std::function<void(int)> onClicked;
    std::function<void()> onAddToPlaylist;

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QWidget::paintEvent(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(rect().adjusted(2, 2, -2, -2), 8, 8);
        p.fillPath(path, QColor(45, 38, 65, 100));
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && onClicked) {
            onClicked(m_musicId);
        }
        QWidget::mousePressEvent(e);
    }

    void contextMenuEvent(QContextMenuEvent *event) override
    {
        QMenu menu(this);
        menu.setStyleSheet(
            "QMenu { background-color: rgba(40, 40, 50, 0.95); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 8px; padding: 4px; }"
            "QMenu::item { color: #e0e0e0; padding: 8px 24px; border-radius: 4px; }"
            "QMenu::item:selected { background-color: rgba(255, 255, 255, 0.1); }"
        );

        QAction *addAction = menu.addAction(I18n::instance().tr("addToPlaylist"));
        QAction *selected = menu.exec(event->globalPos());
        if (selected == addAction && onAddToPlaylist) {
            onAddToPlaylist();
        }
    }

private:
    void loadCover()
    {
        if (m_info.coverUrl.isEmpty()) {
            setPlaceholder();
            return;
        }
        QString musicId = QString::number(m_musicId);
        QPixmap cached = CoverCache::instance()->get(musicId);
        if (!cached.isNull()) {
            disconnect(m_coverConn);
            m_coverConn = {};
            applyPixmap(cached);
            return;
        }
        disconnect(m_coverConn);
        m_coverConn = connect(CoverCache::instance(), &CoverCache::coverLoaded, this,
                [this, musicId](const QString &id, const QPixmap &pix) {
            if (id == musicId) applyPixmap(pix);
        });
        CoverCache::instance()->fetchCover(musicId, m_info.coverUrl);
    }

    void setPlaceholder()
    {
        QPixmap pix(54, 54);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        QPainterPath path;
        path.addRoundedRect(0, 0, 54, 54, 6, 6);
        p.fillPath(path, QColor(128, 128, 128, 40));
        p.setClipPath(path);
        auto icon = Icons::render(Icons::kMusic, 24, QColor(255, 255, 255, 100));
        p.drawPixmap(15, 15, icon);
        m_coverLbl->setPixmap(pix);
    }

    void applyPixmap(const QPixmap &pix)
    {
        disconnect(m_coverConn);
        m_coverConn = {};
        int s = qMin(pix.width(), pix.height());
        QPixmap scaled = pix.copy((pix.width()-s)/2, (pix.height()-s)/2, s, s)
            .scaled(54, 54, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
        m_coverLbl->setPixmap(scaled);
    }

    int m_musicId;
    MusicInfo m_info;
    QLabel *m_coverLbl = nullptr;
    QLabel *m_titleLbl = nullptr;
    QLabel *m_artistLbl = nullptr;
    QMetaObject::Connection m_coverConn;
};

// ─── 歌手结果卡片 ─────────────────────────────────────────
class ArtistResultCard : public QWidget
{
public:
    explicit ArtistResultCard(const QVariantMap &info, QWidget *parent = nullptr)
        : QWidget(parent), m_artistInfo(info)
    {
        setFixedHeight(60);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, false);

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(12, 8, 12, 8);
        lay->setSpacing(14);

        // 歌手头像
        m_avatarLbl = new QLabel(this);
        m_avatarLbl->setFixedSize(44, 44);
        m_avatarLbl->setScaledContents(false);
        loadAvatar();
        lay->addWidget(m_avatarLbl);

        // 信息
        auto *nameLbl = new QLabel(info.value("name").toString(), this);
        nameLbl->setStyleSheet("QLabel { font-size: 14px; font-weight: 600; color: " + QString(Theme::kTextMain) + "; }");
        lay->addWidget(nameLbl, 1);

        // 音乐数量
        int musicCount = info.value("musicCount").toInt();
        auto *countLbl = new QLabel(
            QString("%1 %2").arg(musicCount).arg(I18n::instance().tr("musicCount")), this);
        countLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextMuted) + "; }");
        lay->addWidget(countLbl);
    }

    std::function<void()> onClicked;

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QWidget::paintEvent(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(rect().adjusted(2, 2, -2, -2), 8, 8);
        p.fillPath(path, QColor(45, 38, 65, 100));
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && onClicked) {
            onClicked();
        }
        QWidget::mousePressEvent(e);
    }

private:
    void loadAvatar()
    {
        // TODO: 如果 API 提供歌手头像 URL，可在此异步加载
        QPixmap pix(44, 44);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        QPainterPath path;
        path.addEllipse(0, 0, 44, 44);
        p.fillPath(path, QColor(128, 128, 128, 40));
        p.setClipPath(path);
        auto icon = Icons::render(Icons::kMusic, 24, QColor(255, 255, 255, 100));
        p.drawPixmap(10, 10, icon);
        m_avatarLbl->setPixmap(pix);
    }

    QVariantMap m_artistInfo;
    QLabel *m_avatarLbl = nullptr;
    QMetaObject::Connection m_coverConn;
};

// ─── SearchPage ──────────────────────────────────────────

SearchPage::SearchPage(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent), m_apiClient(apiClient)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();
}

void SearchPage::setupUi()
{
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // 顶部栏：返回按钮 + 搜索框
    auto *header = new QWidget(this);
    header->setFixedHeight(56);
    header->setStyleSheet("QWidget { background: " + QString(Theme::kGlassSidebar) + "; }");
    auto *headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(20, 0, 20, 0);
    headerLay->setSpacing(12);

    auto *backBtn = new QPushButton(QStringLiteral("←"), header);
    backBtn->setFixedSize(36, 36);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton { background: rgba(245,240,255,15); border: none; border-radius: 18px; "
        "color: " + QString(Theme::kTextMain) + "; font-size: 18px; }"
        "QPushButton:hover { background: rgba(196,167,231,40); color: " + QString(Theme::kLavender) + "; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &SearchPage::backRequested);
    headerLay->addWidget(backBtn);

    // 搜索框
    m_searchEdit = new QLineEdit(header);
    m_searchEdit->setPlaceholderText(I18n::instance().tr("searchPlaceholder"));
    m_searchEdit->setFixedHeight(36);
    m_searchEdit->setMinimumWidth(300);
    m_searchEdit->setStyleSheet(
        "QLineEdit { background: rgba(245,240,255,15); border: 1px solid rgba(196,167,231,30); "
        "border-radius: 18px; padding: 0 16px; color: " + QString(Theme::kTextMain) + "; font-size: 13px; }"
        "QLineEdit:focus { border: 1px solid " + QString(Theme::kMint) + "; }"
    );
    connect(m_searchEdit, &QLineEdit::returnPressed, this, [this]() {
        QString q = m_searchEdit->text().trimmed();
        if (!q.isEmpty()) search(q);
    });
    headerLay->addWidget(m_searchEdit);

    // 搜索按钮
    m_searchBtn = new QPushButton(I18n::instance().tr("search"), header);
    m_searchBtn->setFixedHeight(36);
    m_searchBtn->setCursor(Qt::PointingHandCursor);
    m_searchBtn->setStyleSheet(
        "QPushButton { background: " + QString(Theme::kMint) + "; border: none; border-radius: 18px; "
        "color: " + QString(Theme::kBgMid) + "; font-size: 13px; font-weight: 600; padding: 0 20px; }"
        "QPushButton:hover { background: " + QString(Theme::kMintLt) + "; }"
    );
    connect(m_searchBtn, &QPushButton::clicked, this, [this]() {
        QString q = m_searchEdit->text().trimmed();
        if (!q.isEmpty()) search(q);
    });
    headerLay->addWidget(m_searchBtn);

    // 标签切换
    m_musicTab = new QPushButton(I18n::instance().tr("searchMusic"), header);
    m_musicTab->setFixedHeight(36);
    m_musicTab->setCursor(Qt::PointingHandCursor);
    m_musicTab->setCheckable(true);
    m_musicTab->setChecked(true);
    m_musicTab->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: " + QString(Theme::kTextSub) + "; "
        "font-size: 13px; font-weight: 600; padding: 0 16px; }"
        "QPushButton:checked { color: " + QString(Theme::kMint) + "; border-bottom: 2px solid " + QString(Theme::kMint) + "; }"
        "QPushButton:hover { color: " + QString(Theme::kTextMain) + "; }"
    );
    connect(m_musicTab, &QPushButton::clicked, this, &SearchPage::showMusicPage);
    headerLay->addWidget(m_musicTab);

    m_artistTab = new QPushButton(I18n::instance().tr("artist"), header);
    m_artistTab->setFixedHeight(36);
    m_artistTab->setCursor(Qt::PointingHandCursor);
    m_artistTab->setCheckable(true);
    m_artistTab->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: " + QString(Theme::kTextSub) + "; "
        "font-size: 13px; font-weight: 600; padding: 0 16px; }"
        "QPushButton:checked { color: " + QString(Theme::kMint) + "; border-bottom: 2px solid " + QString(Theme::kMint) + "; }"
        "QPushButton:hover { color: " + QString(Theme::kTextMain) + "; }"
    );
    connect(m_artistTab, &QPushButton::clicked, this, &SearchPage::showArtistPage);
    headerLay->addWidget(m_artistTab);

    headerLay->addStretch();
    mainLay->addWidget(header);

    // 状态标签
    m_statusLabel = new QLabel(I18n::instance().tr("inputKeywordToSearch"), this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 40px; }"
    );
    mainLay->addWidget(m_statusLabel);

    // 滚动列表区
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setVisible(false);

    m_container = new QWidget(m_scroll);
    m_listLayout = new QVBoxLayout(m_container);
    m_listLayout->setContentsMargins(16, 16, 16, 16);
    m_listLayout->setSpacing(8);

    m_scroll->setWidget(m_container);
    mainLay->addWidget(m_scroll, 1);
}

void SearchPage::search(const QString &query)
{
    m_query = query;
    m_page = 1;
    m_musicResults.clear();
    m_artistResults.clear();

    m_statusLabel->setText(I18n::instance().tr("loading"));
    m_statusLabel->show();
    m_scroll->setVisible(false);

    fetchMusicResults();
    fetchArtistResults();
}

void SearchPage::fetchMusicResults()
{
    if (!m_apiClient) return;
    m_apiClient->searchMusic(m_query, m_page, kPageSize, [this](bool success, int total, int page, int pageSize, const QList<QVariantMap> &results) {
        QTimer::singleShot(0, this, [this, success, results]() {
            if (success) {
                for (const auto &item : results) {
                    MusicInfo info;
                    info.id = item.value("id").toInt();
                    info.title = item.value("title").toString();
                    info.artist = item.value("artist").toString();
                    info.album = item.value("album").toString();
                    info.duration = item.value("duration").toInt();
                    info.coverUrl = QString::fromUtf8("%1/api/music/cover/%2")
                                        .arg(Theme::kApiBase).arg(info.id);
                    m_musicResults.append(info);
                }
            }
            if (m_showingMusic) buildMusicList();
        });
    });
}

void SearchPage::fetchArtistResults()
{
    if (!m_apiClient) return;
    m_apiClient->searchArtists(m_query, [this](bool success, const QVariantMap &result) {
        QTimer::singleShot(0, this, [this, success, result]() {
            if (success) {
                m_artistResults.clear();
                // API 返回单个歌手信息，包装成列表
                m_artistResults.append(result);
            }
            if (!m_showingMusic) buildArtistList();
        });
    });
}

void SearchPage::buildMusicList()
{
    // 清除现有
    QLayoutItem *item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (m_musicResults.isEmpty()) {
        auto *emptyLbl = new QLabel(I18n::instance().tr("noResults"), m_container);
        emptyLbl->setAlignment(Qt::AlignCenter);
        emptyLbl->setStyleSheet(
            "QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 40px; }"
        );
        m_listLayout->addWidget(emptyLbl);
    } else {
        for (const auto &info : m_musicResults) {
            auto *card = new SearchResultCard(info, m_container);
            card->onClicked = [this, info](int) {
                emit playMusic(info);
            };
            card->onAddToPlaylist = [this, info]() {
                // TODO: 弹出添加到歌单对话框
            };
            m_listLayout->addWidget(card);
        }
    }
    m_listLayout->addStretch();

    m_statusLabel->hide();
    m_scroll->setVisible(true);
}

void SearchPage::buildArtistList()
{
    QLayoutItem *item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (m_artistResults.isEmpty()) {
        auto *emptyLbl = new QLabel(I18n::instance().tr("noResults"), m_container);
        emptyLbl->setAlignment(Qt::AlignCenter);
        emptyLbl->setStyleSheet(
            "QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 40px; }"
        );
        m_listLayout->addWidget(emptyLbl);
    } else {
        for (const auto &info : m_artistResults) {
            auto *card = new ArtistResultCard(info, m_container);
            card->onClicked = [this]() {
                // TODO: 导航到歌手详情页
            };
            m_listLayout->addWidget(card);
        }
    }
    m_listLayout->addStretch();

    m_statusLabel->hide();
    m_scroll->setVisible(true);
}

void SearchPage::showMusicPage()
{
    m_showingMusic = true;
    m_musicTab->setChecked(true);
    m_artistTab->setChecked(false);
    if (!m_musicResults.isEmpty()) {
        buildMusicList();
    }
}

void SearchPage::showArtistPage()
{
    m_showingMusic = false;
    m_artistTab->setChecked(true);
    m_musicTab->setChecked(false);
    if (!m_artistResults.isEmpty()) {
        buildArtistList();
    }
}

void SearchPage::retranslate()
{
    m_searchEdit->setPlaceholderText(I18n::instance().tr("searchPlaceholder"));
    m_searchBtn->setText(I18n::instance().tr("search"));
    m_musicTab->setText(I18n::instance().tr("searchMusic"));
    m_artistTab->setText(I18n::instance().tr("artist"));
}

void SearchPage::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    // 透明背景，由父窗口渐变透出
}
