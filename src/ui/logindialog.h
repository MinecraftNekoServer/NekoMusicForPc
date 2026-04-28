#pragma once

/**
 * @file logindialog.h
 * @brief 登录/注册对话框
 *
 * 支持登录和注册两种模式,包含用户名、密码、邮箱(注册)、验证码(注册)。
 * 采用与主窗口一致的日系动漫风主题。
 */

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;
class QStackedWidget;
class ApiClient;

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    void switchMode();
    void doLogin();
    void doRegister();
    void doSendVerificationCode();
    void onLoginResult(bool success, const QString &message,
                       const QString &token, const QVariantMap &user);
    void showForgotPassword();

    QStackedWidget *m_stack = nullptr;
    QLineEdit *m_loginUserEdit = nullptr;
    QLineEdit *m_loginPassEdit = nullptr;
    QLineEdit *m_regUserEdit = nullptr;
    QLineEdit *m_regPassEdit = nullptr;
    QLineEdit *m_regEmailEdit = nullptr;
    QLineEdit *m_regCodeEdit = nullptr;
    QPushButton *m_submitBtn = nullptr;
    QPushButton *m_switchBtn = nullptr;
    QPushButton *m_sendCodeBtn = nullptr;
    QPushButton *m_forgotBtn = nullptr;
    QLabel *m_msgLabel = nullptr;
    ApiClient *m_api = nullptr;
    bool m_isLoginMode = true;
    int m_countdown = 0;
};
