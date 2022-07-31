#include "utilities.h"

Utilities::Utilities()
{

}

QString Utilities::getDataPath()
{
    QString user_pictures_path = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).constFirst();
    QDir pictures_dir(user_pictures_path);
    pictures_dir.mkpath("Fun_Faces_Photos");
    return pictures_dir.absoluteFilePath("Fun_Faces_Photos");
}

QString Utilities::newPhotoName()
{
    QDateTime time = QDateTime::currentDateTime();
    return time.toString("yyyy-MM-dd+HH:mm:ss");
}

QString Utilities::getPhotoPath(QString name, QString postfix)
{
    return QString("%1/%2.%3").arg(Utilities::getDataPath(), name, postfix);
}
