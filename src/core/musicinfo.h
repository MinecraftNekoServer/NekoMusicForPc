#ifndef MUSICINFO_H
#define MUSICINFO_H

#include <QString>

struct MusicInfo {
    int id = 0;
    QString title;
    QString artist;
    QString album;
    int duration = 0;
    QString coverUrl;
};

#endif // MUSICINFO_H
