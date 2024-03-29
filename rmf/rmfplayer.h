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
#include <mutex>
#include "mediaplayer.h"
#include "rdkmediaplayerimpl.h"
#ifdef USE_EXTERNAL_CAS
#include "CASDataListener.h"
#include "CASHelper.h"
#include "casservice.h"

using namespace anycas;

class CASManagement
{
    virtual void open(const std::string& openData) = 0;
    virtual void registerCASData() = 0;
    virtual void sendCASData(const std::string& data) = 0;
    virtual void destroy(const std::string& casOcdmId) = 0;
};
#endif

class RDKMediaPlayer;

#ifdef USE_EXTERNAL_CAS
class RMFPlayer : public RDKMediaPlayerImpl , public MediaPlayerClient, public CASManagement, public ICasSectionFilter, public ICasPipeline
#else
class RMFPlayer : public RDKMediaPlayerImpl , public MediaPlayerClient
#endif
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
    bool isManagementSession() const;

#ifdef USE_EXTERNAL_CAS
    //CAS Management Related Methods
    void open(const std::string& openData);
    void registerCASData();
    void sendCASData(const std::string& data);
    void destroy(const std::string& casOcdmId);

    //ICasPipeline interface
    uint32_t setKeySlot(uint16_t pid, std::vector<uint8_t> data);
    uint32_t deleteKeySlot(uint16_t pid);
    void unmuteAudio();

    //ICasSectionFilter interface
    ICasFilterStatus create(uint16_t pid, ICasFilterParam &param, ICasHandle **pHandle);
    ICasFilterStatus setState(bool isRunning, ICasHandle* handle);
    ICasFilterStatus start(ICasHandle* handle);
    ICasFilterStatus destroy(ICasHandle* handle);

    uint32_t getPATBuffer(std::vector<uint8_t>& patBuf);
    uint32_t getPMTBuffer(std::vector<uint8_t>& pmtBuf);
    uint32_t getCATBuffer(std::vector<uint8_t>& catBuf);
    bool getAudioPidFromPMT(uint32_t *pid, const std::string& audioLang);
#endif

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
    void psiReady();
    void sectionDataReceived();
    void languageChange();
    void psiUpdateReceived(uint8_t psiStatus);
    void pmtUpdate();
    int getCurrentAudioPid();

private:
    void doTimeUpdate(bool forced = false);
    void startPlaybackProgressMonitor();
    void stopPlaybackProgressMonitor();
    bool refreshCachedCurrentTime(bool forced = false);
    bool endedPlayback() const;
    bool potentiallyPlaying() const;
    bool couldPlayIfEnoughData() const;
    static bool setContentType(const std::string &url, std::string& contentType);
    int get_section_length(vector<uint8_t>sectionDataBuffer);
    vector<uint8_t> get_multiple_section_data(vector<uint8_t>&sectionDataBuffer, int sectionSize);

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
#ifdef USE_EXTERNAL_CAS
    int m_lastAudioPid;
    int m_lastVideoPid;
#endif
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
#ifdef USE_EXTERNAL_CAS
    CASService *m_casService;
    mutable std::mutex cas_mutex;
    bool m_casPending;
#endif
};
#endif
