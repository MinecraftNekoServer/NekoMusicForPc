/**
 * @file musiclistpage.cpp
 * @brief 音乐列表页面实现
 */

#include "musiclistpage.h"
#include "core/apiclient.h"
#include "core/i18n.h"
#include "core/covercache.h"
#include "theme/theme.h"
#include "ui/svgicon.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QMouseEvent>

// ─── 音乐列表项卡片 ──────────────────────────────────────
class MusicItemCard : public QWidget
{
public:
    explicit MusicItemCard(const MusicListPage::MusicInfo &info, QWidget *parent = nullptr)
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

        m_artistLbl = new QLabel(info.artist + " - " + info.album, infoV);
        m_artistLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextSub) + "; }");
        infoLay->addWidget(m_artistLbl);

        infoLay->addStretch();
        lay->addWidget(infoV, 1);

        // 时长
        int mins = info.duration / 60;
        int secs = info.duration % 60;
        auto *timeLbl = new QLabel(
            QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')), this);
        timeLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextMuted) + "; }");
        lay->addWidget(timeLbl);
    }

    int musicId() const { return m_musicId; }

    std::function<void(int)> onClicked;

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
            applyPixmap(cached);
            return;
        }
        connect(CoverCache::instance(), &CoverCache::coverLoaded, this,
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
        int s = qMin(pix.width(), pix.height());
        QPixmap scaled = pix.copy((pix.width()-s)/2, (pix.height()-s)/2, s, s)
            .scaled(54, 54, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_coverLbl->setPixmap(scaled);
    }

    int m_musicId;
    MusicListPage::MusicInfo m_info;
    QLabel *m_coverLbl = nullptr;
    QLabel *m_titleLbl = nullptr;
    QLabel *m_artistLbl = nullptr;
};

// ─── MusicListPage ──────────────────────────────────────

MusicListPage::MusicListPage(Type type, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
    , m_api(new ApiClient(this))
    , m_dataFetched(false)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();

    // 不自动加载,等用户第一次导航到该页面时才加载
}

void MusicListPage::setupUi()
{
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // 顶部栏：返回按钮 + 标题
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
    connect(backBtn, &QPushButton::clicked, this, &MusicListPage::backRequested);
    headerLay->addWidget(backBtn);

    m_titleLabel = new QLabel(
        m_type == Hot ? I18n::instance().tr("hotMusic") : I18n::instance().tr("latestMusic"),
        header);
    m_titleLabel->setStyleSheet(
        "QLabel { font-size: 18px; font-weight: 700; color: " + QString(Theme::kLavender) + "; }"
    );
    headerLay->addWidget(m_titleLabel);

    headerLay->addStretch();
    mainLay->addWidget(header);

    // 滚动列表区
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setObjectName("hpScroll");

    m_listContainer = new QWidget(m_scroll);
    m_listContainer->setObjectName("hpContainer");
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(16, 16, 16, 16);
    m_listLayout->setSpacing(8);

    m_loadingLabel = new QLabel(I18n::instance().tr("loading"), m_listContainer);
    m_loadingLabel->setObjectName("hpLoading");
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setStyleSheet(
        "QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 40px; }"
    );
    m_listLayout->addWidget(m_loadingLabel);
    m_listLayout->addStretch();

    m_scroll->setWidget(m_listContainer);
    mainLay->addWidget(m_scroll, 1);
}

void MusicListPage::refresh()
{
    // 如果已经加载过数据,不重复加载
    if (m_dataFetched) return;

    m_loaded = false;
    m_musicList.clear();

    // 清空列表,但保留loading label和stretch
    QLayoutItem *item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        QWidget *w = item->widget();
        // 保留loading label,其他都删除
        if (w && w != m_loadingLabel) {
            delete w;
        }
        delete item;
    }

    // 确保loading label在布局中
    m_loadingLabel->show();
    fetchData();
    m_dataFetched = true;
}

void MusicListPage::fetchData()
{
    if (m_type == Hot) {
        m_api->fetchRanking([this](bool success, const QList<QVariantMap> &results) {
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
                        m_musicList.append(info);
                    }
                }
                m_loaded = true;
                buildList();
            });
        });
    } else {
        m_api->fetchLatest(300, [this](bool success, const QList<QVariantMap> &results) {
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
                        m_musicList.append(info);
                    }
                }
                m_loaded = true;
                buildList();
            });
        });
    }
}

void MusicListPage::buildList()
{
    // 移除loading和stretch
    QLayoutItem *item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (m_musicList.isEmpty()) {
        auto *emptyLbl = new QLabel(I18n::instance().tr("noData"), m_listContainer);
        emptyLbl->setAlignment(Qt::AlignCenter);
        emptyLbl->setStyleSheet(
            "QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 40px; }"
        );
        m_listLayout->addWidget(emptyLbl);
        m_listLayout->addStretch();
        return;
    }

    for (const auto &info : m_musicList) {
        auto *card = new MusicItemCard(info, m_listContainer);
        card->onClicked = [this](int musicId) {
            onMusicItemClicked(musicId);
        };
        m_listLayout->addWidget(card);
    }

    m_listLayout->addStretch();
}

void MusicListPage::onMusicItemClicked(int musicId)
{
    emit playMusic(musicId);
}

void MusicListPage::retranslate()
{
    if (m_titleLabel) {
        m_titleLabel->setText(
            m_type == Hot ? I18n::instance().tr("hotMusic") : I18n::instance().tr("latestMusic")
        );
    }
    if (m_loadingLabel && !m_loaded) {
        m_loadingLabel->setText(I18n::instance().tr("loading"));
    }
}

void MusicListPage::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    // 透明背景，由父窗口渐变透出
}
