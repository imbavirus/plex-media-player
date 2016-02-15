//
// Created by Tobias Hieta on 01/09/15.
//

#ifndef KONVERGO_PATHS_H
#define KONVERGO_PATHS_H

#include <QString>
#include <QUrl>
#include <QVariant>

namespace Paths
{
  QString resourceDir(const QString& file = QString());
  QString dataDir(const QString& file = QString());
  QString cacheDir(const QString& file = QString());
  QString logDir(const QString& file = QString());
  QString socketName(const QString& serverName);
};

#endif //KONVERGO_PATHS_H
