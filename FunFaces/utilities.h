#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

class Utilities
{
public:
    Utilities();
    static QString getDataPath();
    static QString newPhotoName();
    static QString getPhotoPath(QString name, QString postfix);
};

#endif // UTILITIES_H
