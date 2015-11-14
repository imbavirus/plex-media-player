#ifndef UPDATERCOMPONENT_H
#define UPDATERCOMPONENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCryptographicHash>
#include <QFile>
#include <QThread>
#include <time.h>

#include "ComponentManager.h"
#include "QsLog.h"

#include <time.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
class Update : public QObject
{
  Q_OBJECT
public:
  Update(const QString& url = "", const QString& localPath = "",
         const QString& hash = "", QObject* parent = NULL) : QObject(parent)
  {
    m_url = url;
    m_localPath = localPath;
    m_hash = hash;
    m_reply = NULL;
    m_openFile = new QFile(m_localPath);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  bool setReply(QNetworkReply* reply)
  {
    if (m_reply)
    {
      disconnect(m_reply, 0, 0, 0);
      m_reply->deleteLater();
      m_reply = NULL;
      m_openFile->close();
    }

    m_reply = reply;
    m_timeStarted = time(NULL);

    connect(m_reply, &QNetworkReply::readyRead, this, &Update::write);
    connect(m_reply, &QNetworkReply::finished, this, &Update::finished);

    if (m_openFile->open(QFile::WriteOnly))
      return true;

    m_reply->deleteLater();
    m_reply = NULL;

    return false;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  void write()
  {
    m_openFile->write(m_reply->read(m_reply->bytesAvailable()));
    QThread::yieldCurrentThread();
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  void finished()
  {
    m_openFile->close();
    m_reply->deleteLater();
    m_reply = NULL;

    QLOG_DEBUG() << "Update downloaded, took:" << time(NULL) - m_timeStarted << "seconds";

    emit fileDone(this);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  bool isReady()
  {
    if ((m_reply && m_reply->isRunning()) ||
        (m_openFile && m_openFile->isOpen()))
      return false;

    QFile file(m_localPath);
    if (file.exists())
    {
      QString fileHash = hashFile();
      if (!fileHash.isEmpty() && fileHash == m_hash)
        return true;
    }

    return false;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  QString hashFile()
  {
    QFile file(m_localPath);
    QCryptographicHash hash(QCryptographicHash::Sha1);

    if (file.open(QFile::ReadOnly))
    {
      while (!file.atEnd())
        hash.addData(file.read(8192));

      QByteArray binhash = hash.result();
      return binhash.toHex();
    }

    return "";
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  void abort()
  {
    if (m_reply)
      m_reply->abort();
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  QString m_url;
  QString m_localPath;
  QString m_hash;

  QNetworkReply* m_reply;
  QFile* m_openFile;
  time_t m_timeStarted;

signals:
  void fileDone(Update* update);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class UpdaterComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(UpdaterComponent);

public:
  virtual bool componentExport() { return true; }
  virtual const char* componentName() { return "updater"; }
  virtual bool componentInitialize() { return true; }

  Q_INVOKABLE void downloadUpdate(const QVariantMap &updateInfo);
  Q_INVOKABLE void doUpdate();


signals:
  void downloadError(const QString& error);
  void downloadComplete(const QString& version);
  void downloadProgress(qint64 bytesReceived, qint64 total);

private slots:
  void dlComplete(QNetworkReply *reply);
  bool fileComplete(Update *update);

private:
  explicit UpdaterComponent(QObject *parent = 0);

  bool isDownloading();
  void downloadFile(Update *update);

  QString m_version;

  Update* m_manifest;
  Update* m_file;
  bool m_hasManifest;

  QNetworkAccessManager m_netManager;
};

#endif // UPDATERCOMPONENT_H
