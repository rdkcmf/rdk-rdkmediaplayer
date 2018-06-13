/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#ifndef RMFPLAYER_H
#define RMFPLAYER_H

#include <glib.h>
#include <memory>
#include "mediaplayer.h"
#include "rdkmediaplayerimpl.h"

class RDKMediaPlayer;

class RMFPlayer : public RDKMediaPlayerImpl , public MediaPlayerClient
{
public:
    RMFPlayer(RDKMediaPlayer* parent);
    ~RMFPlayer();
    static bool canPlayURL(const std::string& url);

    //RDKMediaPlayerImpl interface
    bool doCanPlayURL(const std::string& url);
    void doInit();
    void doLoad(const std::string& url);
    void doSetVideoRectangle(const IntRect& rect);
    void doSetAudioLanguage(std::string& lang);
    void doPlay();
    void doPause();
    void doSetPosition(float position);
    void doSeekToLive();
    void doStop();
    void doChangeSpeed(float speed, int32_t overshootTime);
    void doSetSpeed(float speed);
    void doSetBlocked(bool blocked);
    void doSetEISSFilterStatus(bool status);
    void doSetVolume(float volume);
    void doSetIsInProgressRecording(bool isInProgressRecording);
    void doSetZoom(int zoom);
    void doSetNetworkBufferSize(int32_t networkBufferSize);
    void doSetVideoBufferLength(float videoBufferLength);
    void getProgressData(ProgressData* progressData);
    
    //MediaPlayerClient interface
    void mediaPlayerEngineUpdated();
    void mediaPlaybackCompleted();
    void mediaFrameReceived();
    void mediaWarningReceived();
    void volumeChanged(float volume);
    void playerStateChanged();
    void videoStateChanged();
    void durationChanged();
    void timeChanged();
    void rateChanged();
    void videoDecoderHandleReceived();
    void eissDataReceived();

private:
    void doTimeUpdate(bool forced = false);
    void startPlaybackProgressMonitor();
    void stopPlaybackProgressMonitor();
    bool refreshCachedCurrentTime(bool forced = false);
    bool endedPlayback() const;
    bool potentiallyPlaying() const;
    bool couldPlayIfEnoughData() const;
    static bool setContentType(const std::string &url, std::string& contentType);

    std::unique_ptr<MediaPlayer> m_mediaPlayer;
    MediaPlayer::RMFPlayerState m_playerState;
    MediaPlayer::RMFVideoBufferState m_videoState;
    MediaPlayer::RMFVideoBufferState m_videoStateMaximum;
    bool m_isLoaded;
    bool m_isPaused;
    ProgressData m_progressData;
    float m_currentTime;
    float m_duration;
    float m_lastReportedCurrentTime;
    float m_lastReportedDuration;
    float m_lastReportedPlaybackRate;
    guint m_playbackProgressMonitorTag;
    gint64 m_lastRefreshTime;
    std::string m_contentType;
    gint64 m_loadStartTime { 0 };
    gint64 m_setURLTime { 0 };
    gint64 m_loadedTime { 0 };
    gint64 m_loadCompleteTime { 0 };
    gint64 m_playStartTime { 0 };
    gint64 m_playEndTime { 0 };
    gint64 m_firstFrameTime { 0 };
};
#endif
