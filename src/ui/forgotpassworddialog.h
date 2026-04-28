#pragma once

/**
 * @file forgotpassworddialog.h
 * @brief 忘记密码对话框
 *
 * 两步流程:
 * 1. 验证邮箱并发送验证码
 * 2. 输入验证码和新密码,重置密码
 */

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;
class QStackedWidget;
class ApiClient;

class ForgotPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForgotPasswordDialog(QWidget *parent = nullptr);
    ~ForgotPasswordDialog() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    void doSendResetCode();
    void doResetPassword();
    void updateStep();

    QStackedWidget *m_stack = nullptr;
    QLineEdit *m_emailEdit = nullptr;
    QLineEdit *m_codeEdit = nullptr;
    QLineEdit *m_newPassEdit = nullptr;
    QLineEdit *m_confirmPassEdit = nullptr;
    QPushButton *m_submitBtn = nullptr;
    QPushButton *m_sendCodeBtn = nullptr;
    QLabel *m_msgLabel = nullptr;
    ApiClient *m_api = nullptr;

    int m_countdown = 0;
};
