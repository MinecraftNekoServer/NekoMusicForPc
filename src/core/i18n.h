#pragma once

/**
 * @file i18n.h
 * @brief 简易国际化 — 支持中文/英文
 */

#include <QString>

class I18n
{
public:
    enum Language
    {
        NyaCN,
        ZhCN,
        EnUS
    };

    static I18n &instance();

    void setLanguage(Language lang);
    Language language() const { return m_lang; }

    // 快捷方法
    QString tr(const QString &key) const;
    QString settingsTitle() const;
    QString version() const;
    QString system() const;
    QString languageLabel() const;
    QString languageNya() const;
    QString languageChinese() const;
    QString languageEnglish() const;
    QString languageZhCN() const;
    QString languageEnUS() const;

private:
    I18n() = default;
    Language m_lang = ZhCN;
};
