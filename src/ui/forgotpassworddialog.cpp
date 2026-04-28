/**
 * @file forgotpassworddialog.cpp
 * @brief 忘记密码对话框实现
 */

#include "forgotpassworddialog.h"
#include "core/apiclient.h"
#include "core/i18n.h"
#include "theme/theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QTimer>

ForgotPasswordDialog::ForgotPasswordDialog(QWidget *parent)
    : QDialog(parent)
    , m_api(new ApiClient(this))
{
    setupUi();

    setModal(true);
    setMinimumSize(400, 350);
    setMaximumSize(400, 420);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 阴影效果
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 80));
    setGraphicsEffect(shadow);
}

ForgotPasswordDialog::~ForgotPasswordDialog() = default;

void ForgotPasswordDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // 标题
    auto *titleLabel = new QLabel(QStringLiteral("找回密码"), this);
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

    // Stacked 两步流程
    m_stack = new QStackedWidget(this);

    // 步骤1: 输入邮箱
    auto *step1Widget = new QWidget(this);
    auto *step1Layout = new QVBoxLayout(step1Widget);
    step1Layout->setContentsMargins(0, 0, 0, 0);
    step1Layout->setSpacing(12);

    auto *emailLabel = new QLabel(QStringLiteral("请输入注册时的邮箱地址"), step1Widget);
    emailLabel->setStyleSheet("QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 13px; }");
    step1Layout->addWidget(emailLabel);

    m_emailEdit = new QLineEdit(step1Widget);
    m_emailEdit->setPlaceholderText(I18n::instance().tr("email"));
    m_emailEdit->setObjectName("dialogEdit");
    m_emailEdit->setFixedHeight(40);
    step1Layout->addWidget(m_emailEdit);

    m_sendCodeBtn = new QPushButton(I18n::instance().tr("sendCode"), step1Widget);
    m_sendCodeBtn->setObjectName("dialogBtn");
    m_sendCodeBtn->setFixedHeight(44);
    connect(m_sendCodeBtn, &QPushButton::clicked, this, &ForgotPasswordDialog::doSendResetCode);
    step1Layout->addWidget(m_sendCodeBtn);

    step1Layout->addStretch();
    m_stack->addWidget(step1Widget);

    // 步骤2: 输入验证码和新密码
    auto *step2Widget = new QWidget(this);
    auto *step2Layout = new QVBoxLayout(step2Widget);
    step2Layout->setContentsMargins(0, 0, 0, 0);
    step2Layout->setSpacing(12);

    auto *codeLabel = new QLabel(QStringLiteral("请输入邮箱中的验证码"), step2Widget);
    codeLabel->setStyleSheet("QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 13px; }");
    step2Layout->addWidget(codeLabel);

    m_codeEdit = new QLineEdit(step2Widget);
    m_codeEdit->setPlaceholderText(I18n::instance().tr("verificationCode"));
    m_codeEdit->setObjectName("dialogEdit");
    m_codeEdit->setFixedHeight(40);
    step2Layout->addWidget(m_codeEdit);

    m_newPassEdit = new QLineEdit(step2Widget);
    m_newPassEdit->setPlaceholderText(QStringLiteral("新密码(6-30位)"));
    m_newPassEdit->setObjectName("dialogEdit");
    m_newPassEdit->setFixedHeight(40);
    m_newPassEdit->setEchoMode(QLineEdit::Password);
    step2Layout->addWidget(m_newPassEdit);

    m_confirmPassEdit = new QLineEdit(step2Widget);
    m_confirmPassEdit->setPlaceholderText(QStringLiteral("确认新密码"));
    m_confirmPassEdit->setObjectName("dialogEdit");
    m_confirmPassEdit->setFixedHeight(40);
    m_confirmPassEdit->setEchoMode(QLineEdit::Password);
    step2Layout->addWidget(m_confirmPassEdit);

    m_submitBtn = new QPushButton(QStringLiteral("重置密码"), step2Widget);
    m_submitBtn->setObjectName("dialogBtn");
    m_submitBtn->setFixedHeight(44);
    connect(m_submitBtn, &QPushButton::clicked, this, &ForgotPasswordDialog::doResetPassword);
    step2Layout->addWidget(m_submitBtn);

    step2Layout->addStretch();
    m_stack->addWidget(step2Widget);

    m_stack->setCurrentIndex(0);
    mainLayout->addWidget(m_stack);

    // 返回按钮
    auto *backBtn = new QPushButton(QStringLiteral("返回登录"), this);
    backBtn->setObjectName("dialogLinkBtn");
    backBtn->setFixedHeight(36);
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked, this, &QDialog::reject);
    mainLayout->addWidget(backBtn);

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

void ForgotPasswordDialog::doSendResetCode()
{
    QString email = m_emailEdit->text().trimmed();
    if (email.isEmpty()) {
        m_msgLabel->setText(I18n::instance().tr("pleaseEnterEmail"));
        m_msgLabel->setStyleSheet(
            "QLabel { color: " + QString(Theme::kSakura) + "; font-size: 13px; min-height: 20px; }"
        );
        return;
    }

    m_msgLabel->clear();
    m_sendCodeBtn->setEnabled(false);

    m_api->sendResetCode(email, [this](bool success, const QString &message) {
        QTimer::singleShot(0, this, [this, success, message]() {
            if (success) {
                m_msgLabel->setText(message);
                m_msgLabel->setStyleSheet(
                    "QLabel { color: " + QString(Theme::kMint) + "; font-size: 13px; min-height: 20px; }"
                );
                // 进入第二步
                m_stack->setCurrentIndex(1);
                // 60秒倒计时
                m_countdown = 60;
                auto *timer = new QTimer(this);
                connect(timer, &QTimer::timeout, this, [this, timer]() {
                    m_countdown--;
                    if (m_countdown > 0) {
                        m_sendCodeBtn->setText(QString("%1s").arg(m_countdown));
                    } else {
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

void ForgotPasswordDialog::doResetPassword()
{
    QString email = m_emailEdit->text().trimmed();
    QString code = m_codeEdit->text().trimmed();
    QString newPass = m_newPassEdit->text();
    QString confirmPass = m_confirmPassEdit->text();

    if (email.isEmpty() || code.isEmpty() || newPass.isEmpty() || confirmPass.isEmpty()) {
        m_msgLabel->setText(I18n::instance().tr("fillAllFields"));
        m_msgLabel->setStyleSheet(
            "QLabel { color: " + QString(Theme::kSakura) + "; font-size: 13px; min-height: 20px; }"
        );
        return;
    }

    if (newPass.length() < 6 || newPass.length() > 30) {
        m_msgLabel->setText(QStringLiteral("密码长度必须在6-30位之间"));
        m_msgLabel->setStyleSheet(
            "QLabel { color: " + QString(Theme::kSakura) + "; font-size: 13px; min-height: 20px; }"
        );
        return;
    }

    if (newPass != confirmPass) {
        m_msgLabel->setText(QStringLiteral("两次输入的密码不一致"));
        m_msgLabel->setStyleSheet(
            "QLabel { color: " + QString(Theme::kSakura) + "; font-size: 13px; min-height: 20px; }"
        );
        return;
    }

    m_msgLabel->clear();
    m_submitBtn->setEnabled(false);
    m_submitBtn->setText("...");

    m_api->resetPassword(email, code, newPass, [this](bool success, const QString &message) {
        QTimer::singleShot(0, this, [this, success, message]() {
            m_submitBtn->setEnabled(true);
            m_submitBtn->setText(QStringLiteral("重置密码"));

            if (success) {
                m_msgLabel->setText(QStringLiteral("密码重置成功,请登录"));
                m_msgLabel->setStyleSheet(
                    "QLabel { color: " + QString(Theme::kMint) + "; font-size: 13px; min-height: 20px; }"
                );
                // 延迟关闭,让用户看到成功消息
                QTimer::singleShot(1500, this, &QDialog::accept);
            } else {
                m_msgLabel->setText(message);
                m_msgLabel->setStyleSheet(
                    "QLabel { color: " + QString(Theme::kSakura) + "; font-size: 13px; min-height: 20px; }"
                );
            }
        });
    });
}

void ForgotPasswordDialog::paintEvent(QPaintEvent *event)
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
