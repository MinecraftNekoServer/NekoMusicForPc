/**
 * @file logindialog.cpp
 * @brief 登录/注册对话框实现
 */

#include "logindialog.h"
#include "forgotpassworddialog.h"
#include "core/apiclient.h"
#include "core/usermanager.h"
#include "core/i18n.h"
#include "theme/theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QTimer>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , m_api(new ApiClient(this))
{
    setupUi();

    setModal(true);
    setMinimumSize(400, 380);
    setMaximumSize(400, 480);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 阴影效果
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 80));
    setGraphicsEffect(shadow);
}

LoginDialog::~LoginDialog() = default;

void LoginDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // 标题
    auto *titleLabel = new QLabel(I18n::instance().tr("login"), this);
    titleLabel->setStyleSheet(
        "QLabel { color: " + QString(Theme::kLavender) + "; font-size: 22px; "
        "font-weight: bold; padding: 8px 0; }"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 消息提示
    m_msgLabel = new QLabel(this);
    m_msgLabel->setAlignment(Qt::AlignCenter);
    m_msgLabel->setStyleSheet(
        "QLabel { color: " + QString(Theme::kSakura) + "; font-size: 13px; "
        "min-height: 20px; }"
    );
    mainLayout->addWidget(m_msgLabel);

    // 表单区域
    auto *formWidget = new QWidget(this);
    auto *formLayout = new QVBoxLayout(formWidget);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(12);

    // 登录表单
    auto *loginWidget = new QWidget(this);
    auto *loginLayout = new QVBoxLayout(loginWidget);
    loginLayout->setContentsMargins(0, 0, 0, 0);
    loginLayout->setSpacing(12);

    m_loginUserEdit = new QLineEdit(loginWidget);
    m_loginUserEdit->setPlaceholderText(I18n::instance().tr("username"));
    m_loginUserEdit->setObjectName("dialogEdit");
    m_loginUserEdit->setFixedHeight(40);
    loginLayout->addWidget(m_loginUserEdit);

    m_loginPassEdit = new QLineEdit(loginWidget);
    m_loginPassEdit->setPlaceholderText(I18n::instance().tr("password"));
    m_loginPassEdit->setObjectName("dialogEdit");
    m_loginPassEdit->setFixedHeight(40);
    m_loginPassEdit->setEchoMode(QLineEdit::Password);
    loginLayout->addWidget(m_loginPassEdit);

    // 注册表单
    auto *regWidget = new QWidget(this);
    auto *regLayout = new QVBoxLayout(regWidget);
    regLayout->setContentsMargins(0, 0, 0, 0);
    regLayout->setSpacing(12);

    m_regUserEdit = new QLineEdit(regWidget);
    m_regUserEdit->setPlaceholderText(I18n::instance().tr("username"));
    m_regUserEdit->setObjectName("dialogEdit");
    m_regUserEdit->setFixedHeight(40);
    regLayout->addWidget(m_regUserEdit);

    m_regPassEdit = new QLineEdit(regWidget);
    m_regPassEdit->setPlaceholderText(I18n::instance().tr("password"));
    m_regPassEdit->setObjectName("dialogEdit");
    m_regPassEdit->setFixedHeight(40);
    m_regPassEdit->setEchoMode(QLineEdit::Password);
    regLayout->addWidget(m_regPassEdit);

    // 邮箱 + 验证码行
    auto *emailRow = new QWidget(regWidget);
    auto *emailRowLayout = new QHBoxLayout(emailRow);
    emailRowLayout->setContentsMargins(0, 0, 0, 0);
    emailRowLayout->setSpacing(8);

    m_regEmailEdit = new QLineEdit(emailRow);
    m_regEmailEdit->setPlaceholderText(I18n::instance().tr("email"));
    m_regEmailEdit->setObjectName("dialogEdit");
    m_regEmailEdit->setFixedHeight(40);
    emailRowLayout->addWidget(m_regEmailEdit);

    m_sendCodeBtn = new QPushButton(I18n::instance().tr("sendCode"), emailRow);
    m_sendCodeBtn->setObjectName("dialogBtn");
    m_sendCodeBtn->setFixedHeight(40);
    m_sendCodeBtn->setFixedWidth(100);
    connect(m_sendCodeBtn, &QPushButton::clicked, this, &LoginDialog::doSendVerificationCode);
    emailRowLayout->addWidget(m_sendCodeBtn);

    regLayout->addWidget(emailRow);

    m_regCodeEdit = new QLineEdit(regWidget);
    m_regCodeEdit->setPlaceholderText(I18n::instance().tr("verificationCode"));
    m_regCodeEdit->setObjectName("dialogEdit");
    m_regCodeEdit->setFixedHeight(40);
    regLayout->addWidget(m_regCodeEdit);

    // Stacked 切换
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(loginWidget);
    m_stack->addWidget(regWidget);
    m_stack->setCurrentIndex(0);
    formLayout->addWidget(m_stack);

    mainLayout->addWidget(formWidget);

    // 按钮区
    m_submitBtn = new QPushButton(I18n::instance().tr("login"), this);
    m_submitBtn->setObjectName("dialogBtn");
    m_submitBtn->setFixedHeight(44);
    connect(m_submitBtn, &QPushButton::clicked, this, &LoginDialog::doLogin);
    mainLayout->addWidget(m_submitBtn);

    m_switchBtn = new QPushButton(I18n::instance().tr("register"), this);
    m_switchBtn->setObjectName("dialogLinkBtn");
    m_switchBtn->setFixedHeight(36);
    m_switchBtn->setCursor(Qt::PointingHandCursor);
    connect(m_switchBtn, &QPushButton::clicked, this, &LoginDialog::switchMode);
    mainLayout->addWidget(m_switchBtn);

    // 忘记密码链接(只在登录模式显示)
    m_forgotBtn = new QPushButton(I18n::instance().tr("forgotPassword"), this);
    m_forgotBtn->setObjectName("dialogLinkBtn");
    m_forgotBtn->setFixedHeight(36);
    m_forgotBtn->setCursor(Qt::PointingHandCursor);
    connect(m_forgotBtn, &QPushButton::clicked, this, &LoginDialog::showForgotPassword);
    mainLayout->addWidget(m_forgotBtn);

    // 关闭按钮
    auto *closeBtn = new QPushButton(QStringLiteral("×"), this);
    closeBtn->setObjectName("dialogCloseBtn");
    closeBtn->setFixedSize(32, 32);
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto *closeLayout = new QHBoxLayout();
    closeLayout->addStretch();
    closeLayout->addWidget(closeBtn);
    closeLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(closeLayout);
}

void LoginDialog::switchMode()
{
    m_isLoginMode = !m_isLoginMode;
    m_msgLabel->clear();

    if (m_isLoginMode) {
        m_stack->setCurrentIndex(0);
        m_submitBtn->setText(I18n::instance().tr("login"));
        m_submitBtn->disconnect();
        connect(m_submitBtn, &QPushButton::clicked, this, &LoginDialog::doLogin);
        m_switchBtn->setText(I18n::instance().tr("register"));
        m_forgotBtn->show();  // 登录模式显示忘记密码
    } else {
        m_stack->setCurrentIndex(1);
        m_submitBtn->setText(I18n::instance().tr("register"));
        m_submitBtn->disconnect();
        connect(m_submitBtn, &QPushButton::clicked, this, &LoginDialog::doRegister);
        m_switchBtn->setText(I18n::instance().tr("login"));
        m_forgotBtn->hide();  // 注册模式隐藏忘记密码
    }
}

void LoginDialog::doLogin()
{
    QString username = m_loginUserEdit->text().trimmed();
    QString password = m_loginPassEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        m_msgLabel->setText(I18n::instance().tr("fillUsernameAndPassword"));
        m_msgLabel->setStyleSheet(
            "QLabel { color: " + QString(Theme::kSakura) + "; font-size: 13px; min-height: 20px; }"
        );
        return;
    }

    m_msgLabel->setText("");
    m_submitBtn->setEnabled(false);
    m_submitBtn->setText("...");

    m_api->login(username, password, [this](bool success, const QString &message,
                                             const QString &token, const QVariantMap &user) {
        QTimer::singleShot(0, this, [this, success, message, token, user]() {
            onLoginResult(success, message, token, user);
        });
    });
}

void LoginDialog::doRegister()
{
    QString username = m_regUserEdit->text().trimmed();
    QString password = m_regPassEdit->text();
    QString email = m_regEmailEdit->text().trimmed();
    QString code = m_regCodeEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty() || email.isEmpty() || code.isEmpty()) {
        m_msgLabel->setText(I18n::instance().tr("fillAllFields"));
        m_msgLabel->setStyleSheet(
            "QLabel { color: " + QString(Theme::kSakura) + "; font-size: 13px; min-height: 20px; }"
        );
        return;
    }

    m_msgLabel->setText("");
    m_submitBtn->setEnabled(false);
    m_submitBtn->setText("...");

    m_api->registerUser(username, password, email, code,
                        [this](bool success, const QString &message,
                               const QString &token, const QVariantMap &user) {
        QTimer::singleShot(0, this, [this, success, message, token, user]() {
            onLoginResult(success, message, token, user);
        });
    });
}

void LoginDialog::doSendVerificationCode()
{
    QString email = m_regEmailEdit->text().trimmed();
    if (email.isEmpty()) {
        m_msgLabel->setText(I18n::instance().tr("pleaseEnterEmail"));
        return;
    }

    m_sendCodeBtn->setEnabled(false);
    m_api->sendVerificationCode(email, [this](bool success, const QString &message) {
        QTimer::singleShot(0, this, [this, success, message]() {
            if (success) {
                m_msgLabel->setText(message);
                m_msgLabel->setStyleSheet(
                    "QLabel { color: " + QString(Theme::kMint) + "; font-size: 13px; min-height: 20px; }"
                );
                // 60秒倒计时
                m_countdown = 60;
                auto *timer = new QTimer(this);
                connect(timer, &QTimer::timeout, this, [this, timer]() {
                    m_countdown--;
                    m_sendCodeBtn->setText(QString("%1s").arg(m_countdown));
                    if (m_countdown <= 0) {
                        timer->stop();
                        timer->deleteLater();
                        m_sendCodeBtn->setEnabled(true);
                        m_sendCodeBtn->setText(I18n::instance().tr("sendCode"));
                    }
                });
                timer->start(1000);
            } else {
                m_msgLabel->setText(message);
                m_msgLabel->setStyleSheet(
                    "QLabel { color: " + QString(Theme::kSakura) + "; font-size: 13px; min-height: 20px; }"
                );
                m_sendCodeBtn->setEnabled(true);
            }
        });
    });
}

void LoginDialog::onLoginResult(bool success, const QString &message,
                                 const QString &token, const QVariantMap &user)
{
    m_submitBtn->setEnabled(true);
    if (m_isLoginMode) {
        m_submitBtn->setText(I18n::instance().tr("login"));
    } else {
        m_submitBtn->setText(I18n::instance().tr("register"));
    }

    if (success) {
        UserManager::instance().setLoginInfo(token, user);
        accept();
    } else {
        m_msgLabel->setText(message);
        m_msgLabel->setStyleSheet(
            "QLabel { color: " + QString(Theme::kSakura) + "; font-size: 13px; min-height: 20px; }"
        );
    }
}

void LoginDialog::showForgotPassword()
{
    // 显示忘记密码对话框
    ForgotPasswordDialog dlg(this);
    dlg.exec();
}

void LoginDialog::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 圆角背景
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(2, 2, -2, -2), 16, 16);
    p.fillPath(path, QColor(36, 31, 49, 245));

    // 边框
    p.setPen(QPen(QColor(196, 167, 231, 60), 1));
    p.drawPath(path);
}
