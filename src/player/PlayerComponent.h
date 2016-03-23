#ifndef PLAYERCOMPONENT_H
#define PLAYERCOMPONENT_H

#include <QObject>
#include <QtCore/qglobal.h>
#include <QVariant>
#include <QSet>
#include <QQuickWindow>
#include <QTimer>
#include <QTextStream>

#include "ComponentManager.h"

#include <mpv/client.h>
#include <mpv/qthelper.hpp>

void initD3DDevice(void);

///////////////////////////////////////////////////////////////////////////////////////////////////
class PlayerComponent : public ComponentBase
{
  Q_OBJECT
  DEFINE_SINGLETON(PlayerComponent);

public:
  const char* componentName() override { return "player"; }
  bool componentExport() override { return true; }
  bool componentInitialize() override;
  void componentPostInitialize() override;
  
  explicit PlayerComponent(QObject* parent = nullptr);
  ~PlayerComponent() override;

  // Deprecated. Corresponds to stop() + queueMedia().
  Q_INVOKABLE bool load(const QString& url, const QVariantMap& options, const QVariantMap& metadata, const QString& audioStream = QString(), const QString& subtitleStream = QString());

  // Append a media item to the internal playlist. If nothing is played yet, the
  // newly appended item will start playing immediately.
  // options:
  //  - startMilliseconds: start playback at this time (in ms)
  //  - autoplay: if false, start playback paused; if true, start normally
  Q_INVOKABLE void queueMedia(const QString& url, const QVariantMap& options, const QVariantMap &metadata, const QString& audioStream, const QString& subtitleStream);

  // This clears all items queued with queueMedia().
  // It explicitly excludes the currently playing item. The main use of this function
  // is updating the next item that should be played (for the purpose of gapless audio).
  // If you want to wipe everything, use stop().
  Q_INVOKABLE void clearQueue();

  Q_INVOKABLE virtual void seekTo(qint64 ms);

  // Stop playback and clear all queued items.
  Q_INVOKABLE virtual void stop();

  // A full reload of the stream is imminent (stop() + load())
  // Used ofr not resetting display mode with the next stop() call.
  Q_INVOKABLE virtual void streamSwitch();

  Q_INVOKABLE virtual void pause();
  Q_INVOKABLE virtual void play();
  
  // 0-100 volume 0=mute and 100=normal
  // Ignored if no audio output active (e.g. when no file is playing).
  Q_INVOKABLE virtual void setVolume(int volume);
  // Returns 0 if no audio output active.
  Q_INVOKABLE virtual int volume();

  // Ignored if no audio output active.
  Q_INVOKABLE virtual void setMuted(bool muted);
  // Returns 0 if no audio output active.
  Q_INVOKABLE virtual bool muted();

  // Returns a QVariant of the following format:
  // QVariantList                   (list of audio device entries)
  //    QVariantMap                 (an audio device entry)
  //      "name" -> QString         (symbolic name/ID of the device)
  //      "description" -> QString  (human readable description intended for display)
  //
  Q_INVOKABLE virtual QVariant getAudioDeviceList();
  // Uses the "name" from the device list.
  Q_INVOKABLE virtual void setAudioDevice(const QString& name);
  
  Q_INVOKABLE virtual void setAudioStream(const QString& audioStream);
  Q_INVOKABLE virtual void setSubtitleStream(const QString& subtitleStream);

  Q_INVOKABLE virtual void setAudioDelay(qint64 milliseconds);
  Q_INVOKABLE virtual void setSubtitleDelay(qint64 milliseconds);

  // If enabled, hide the web view (whether it's OSD or not), and show video
  // only. If no video is running, render a black background only.
  Q_INVOKABLE virtual void setVideoOnlyMode(bool enable);

  // Currently is meant to check for "vc1" and "mpeg2video". Will return whether
  // it can be natively decoded. Will return true for all other codecs,
  // including unknown codec names.
  Q_INVOKABLE virtual bool checkCodecSupport(const QString& codec);

  Q_INVOKABLE void userCommand(QString command);

  const mpv::qt::Handle getMpvHandle() const { return m_mpv; }

  virtual void setWindow(QQuickWindow* window);

  QString videoInformation() const;

  static QStringList AudioCodecsAll() { return { "ac3", "dts", "eac3", "dts-hd", "truehd" }; };
  static QStringList AudioCodecsSPDIF() { return { "ac3", "dts" }; };
  
public Q_SLOTS:
  void setAudioConfiguration();
  void updateAudioDeviceList();
  void updateSubtitleSettings();
  void updateVideoSettings();

private Q_SLOTS:
  void handleMpvEvents();
  void onRestoreDisplay();
  void onRefreshRateChange();
  void onReloadAudio();

Q_SIGNALS:
  void playing(const QString& url);
  void buffering(float);
  // playback has stopped due to a stop() or loadMedia() request
  void stopped(const QString& url);
  // playback has stopped because the current media was fully played
  void finished(const QString& url);
  // playback has stopped due to any reason - this always happens if the
  // playing() signal was emitted
  void playbackEnded(const QString& url);
  // emitted if playback has ended, and no more items are queued for playback
  void playbackAllDone();
  // emitted after playing(), and as soon as the the media is fully loaded, and
  // playback starts normally
  void playbackStarting();
  void paused(bool paused);
  // true if the video (or music) is actually
  // false if nothing is loaded, playback is paused, during seeking, or media is being loaded
  void playbackActive(bool active);
  void windowVisible(bool visible);
  // emitted as soon as the duration of the current file is known
  void updateDuration(qint64 milliseconds);

  // an error happened during playback - this implies abort of playback
  // the id is the (negative) error number, and the message parameter is a short
  // English description of the error (always the same for the same id, no
  // further information)
  void error(int id, const QString& message);

  // current position in ms should be triggered 2 times a second
  // when position updates
  void positionUpdate(quint64);

  void onMpvEvents();
  
private:
  // this is the function actually implemented in the backends. the variantmap contains
  // a few known keys:
  // * subtitleStreamIndex
  // * subtitleStreamIdentifier
  // * audioStreamIndex
  // * audioStreamIdentifier
  // * viewOffset
  //
  void loadWithOptions(const QVariantMap& options);
  void setRpiWindow(QQuickWindow* window);
  void setQtQuickWindow(QQuickWindow* window);
  void handleMpvEvent(mpv_event *event);
  // Potentially switch the display refresh rate, and return true if the refresh rate
  // was actually changed.
  bool switchDisplayFrameRate();
  void checkCurrentAudioDevice(const QSet<QString>& old_devs, const QSet<QString>& new_devs);
  void appendAudioFormat(QTextStream& info, const QString& property) const;
  void initializeCodecSupport();

  mpv::qt::Handle m_mpv;

  double m_lastPositionUpdate;
  qint64 m_playbackAudioDelay;
  QString m_currentUrl;
  bool m_playbackStartSent;
  QQuickWindow* m_window;
  float m_mediaFrameRate;
  QTimer m_restoreDisplayTimer;
  QTimer m_reloadAudioTimer;
  QSet<QString> m_audioDevices;
  bool m_streamSwitchImminent;
  QMap<QString, bool> m_codecSupport;
};

#endif // PLAYERCOMPONENT_H
