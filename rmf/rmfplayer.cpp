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
#include "rmfplayer.h"
#include "rdkmediaplayer.h"
#include <glib.h>
#include <limits>
#include <cmath>
#include <unistd.h>
#include "logger.h"
#include "mediaplayerdlna.h"
#include "mediaplayergeneric.h"

#define TO_MS(x) (uint32_t)( (x) / 1000)
#define TUNE_LOG_MAX_SIZE 100

namespace
{
    const guint kProgressMonitorTimeoutMs = 250;
    const guint kRefreshCachedTimeThrottleMs = 100;
}

bool RMFPlayer::canPlayURL(const std::string& url)
{
    std::string contentType;
    return RMFPlayer::setContentType(url, contentType);
}

RMFPlayer::RMFPlayer(RDKMediaPlayer* parent) : 
    RDKMediaPlayerImpl(parent),
    m_mediaPlayer(new MediaPlayerDLNA(this)),
    m_isLoaded(false),
    m_isPaused(true),
    m_currentTime(0.f),
    m_lastReportedCurrentTime(0.f),
    m_duration(0.f),
    m_lastReportedDuration(0.f),
    m_lastReportedPlaybackRate(0.f),
    m_playerState(MediaPlayer::RMF_PLAYER_EMPTY),
    m_videoState(MediaPlayer::RMF_VIDEO_BUFFER_HAVENOTHING),
    m_videoStateMaximum(MediaPlayer::RMF_VIDEO_BUFFER_HAVENOTHING),
    m_playbackProgressMonitorTag(0),
    m_lastRefreshTime(0)
{
}

RMFPlayer::~RMFPlayer()
{
}

// RDKMediaPlayerImpl impl start
bool RMFPlayer::doCanPlayURL(const std::string& url)
{
    return RMFPlayer::canPlayURL(url);
}

void RMFPlayer::doInit()
{
}

void RMFPlayer::doLoad(const std::string& url)
{
    m_playerState = MediaPlayer::RMF_PLAYER_LOADING;
    m_videoState = MediaPlayer::RMF_VIDEO_BUFFER_HAVENOTHING;
    m_videoStateMaximum = MediaPlayer::RMF_VIDEO_BUFFER_HAVENOTHING;
    m_lastReportedCurrentTime = 0.f;
    m_lastReportedDuration = 0.f;
    m_lastReportedPlaybackRate = 0.f;
    m_isPaused = true;
    m_setURLTime = getParent()->getSetURLTime();
    m_loadStartTime = getParent()->getLoadStartTime();

    if (MediaPlayerDLNA::supportsUrl(url))
        m_mediaPlayer.reset(new MediaPlayerDLNA(this));
    else
        m_mediaPlayer.reset(new MediaPlayerGeneric(this));

    setContentType(url, m_contentType);

    LOG_INFO("load(%s)", url.c_str());

    m_isLoaded = m_mediaPlayer->rmf_load(url);

    m_loadedTime = g_get_monotonic_time();

    m_loadCompleteTime = g_get_monotonic_time();
}

void RMFPlayer::doSetVideoRectangle(const IntRect& rect)
{
    if (rect.width() > 0 && rect.height() > 0)
    {
        LOG_INFO("set video rectangle: %dx%d %dx%d", rect.x(), rect.y(), rect.width(), rect.height());
        m_mediaPlayer->rmf_setVideoRectangle(rect.x(), rect.y(), rect.width(), rect.height());
    }
}

void RMFPlayer::doSetAudioLanguage(std::string& lang)
{
  LOG_INFO("set lang: %s", lang.c_str());
  m_mediaPlayer->rmf_setAudioLanguage(lang);
}

void RMFPlayer::doPlay()
{
    m_playStartTime = getParent()->getPlayStartTime();
    if (!m_mediaPlayer->rmf_canItPlay())
    {
        LOG_INFO("cannot play, skip play()");
        return;
    }
    if (m_isPaused || m_mediaPlayer->rmf_isPaused())
    {
        LOG_INFO("play()");
        m_mediaPlayer->rmf_play();
        if (m_videoState >= MediaPlayer::RMF_VIDEO_BUFFER_HAVEFUTUREDATA)
        {
            m_isPaused = false;
            getParent()->getEventEmitter().send(OnPlayingEvent());
        }
    }
    else
    {
        LOG_INFO("not paused, skip play()");
    }
    m_playEndTime = g_get_monotonic_time();
    startPlaybackProgressMonitor();
}

void RMFPlayer::doPause()
{
    if (!m_mediaPlayer->rmf_isPaused())
    {
        LOG_INFO("pause()");
        m_isPaused = true;
        m_mediaPlayer->rmf_pause();
        doTimeUpdate(true);
        getParent()->getEventEmitter().send(OnPausedEvent());
    }
}

void RMFPlayer::doSetPosition(float position)
{
    if (!std::isnan(position))
    {
        LOG_INFO("set currentTime: %f", position);
        m_mediaPlayer->rmf_seek(position/1000);
    }
}


void RMFPlayer::doSeekToLive()
{
    if (m_mediaPlayer->rmf_isItInProgressRecording())
    {
        if (m_mediaPlayer->rmf_getRate() != 1.f)
        {
            LOG_INFO("set rate: 1.0");
            m_mediaPlayer->rmf_setRate(1.f);
        }
        LOG_INFO("jumpToLive: true");
        m_mediaPlayer->rmf_seekToLivePosition();
    }
}


void RMFPlayer::doStop()
{
    LOG_INFO("stop()");
    m_isLoaded = false;
    stopPlaybackProgressMonitor();
    m_mediaPlayer->rmf_stop();
    getParent()->getEventEmitter().send(OnClosedEvent());
}

void RMFPlayer::doChangeSpeed(float speed, int32_t overshootTime)
{
    m_currentTime = 0.f;
    LOG_INFO("doChangeSpeed(%f, %d)", speed, overshootTime);
    m_mediaPlayer->rmf_changeSpeed(speed, overshootTime);
}

void RMFPlayer::doSetSpeed(float speed)
{
    if (m_mediaPlayer->rmf_getRate() != speed)
    {
        LOG_INFO("doSetSpeed(%f)", speed);
        m_mediaPlayer->rmf_setRate(speed);
    }
}

void RMFPlayer::doSetBlocked(bool blocked)
{
    if (m_mediaPlayer->rmf_isMuted() != blocked)
    {
        LOG_INFO("set mute: %s", blocked ? "true" : "false");
        m_mediaPlayer->rmf_setMute(blocked);
    }
}

void RMFPlayer::doSetEISSFilterStatus(bool status)
{
    LOG_INFO("set EISS Filter Status: %s", status? "true" : "false");
    m_mediaPlayer->rmf_setEissFilterStatus(status);
}

void RMFPlayer::doSetVolume(float volume)
{
    LOG_INFO("set volume: %f", volume);
    if(!getParent()->getIsBlocked() && volume > 0)
        m_mediaPlayer->rmf_setMute(false);
    m_mediaPlayer->rmf_setVolume(volume);
}

void RMFPlayer::doSetIsInProgressRecording(bool isInProgressRecording)
{
    LOG_TRACE("set isInProgressRecording: %s", isInProgressRecording ? "true" : "false");
    m_mediaPlayer->rmf_setInProgressRecording(isInProgressRecording);
}

void RMFPlayer::doSetZoom(int zoom)
{
    LOG_INFO("set videoZoom: %d", zoom);
    m_mediaPlayer->rmf_setVideoZoom(zoom);
}

void RMFPlayer::doSetNetworkBufferSize(int32_t networkBufferSize)
{
    LOG_INFO("set networkBufferSize: %d", networkBufferSize);
    m_mediaPlayer->rmf_setNetworkBufferSize(networkBufferSize);
}

void RMFPlayer::doSetVideoBufferLength(float videoBufferLength)
{
    LOG_TRACE("set videoBufferLength: %f", videoBufferLength);
    m_mediaPlayer->rmf_setVideoBufferLength(videoBufferLength);
}

void RMFPlayer::getProgressData(ProgressData* progressData)
{
    progressData->position = m_lastReportedCurrentTime;
    progressData->duration = m_lastReportedDuration;
    progressData->speed = m_lastReportedPlaybackRate;
    progressData->start = -1;
    progressData->end = -1;
}

// RDKMediaPlayerImpl impl end

// MediaPlayerClient impl start
void RMFPlayer::mediaPlayerEngineUpdated()
{
}

void RMFPlayer::mediaPlaybackCompleted()
{
    getParent()->getEventEmitter().send(OnCompleteEvent());
    getParent()->stop();//automatically call stop on player
}

void RMFPlayer::mediaFrameReceived()
{
    std::string tuneTimeLog;
    if (m_setURLTime)
    {
        m_firstFrameTime = g_get_monotonic_time();

        if (m_loadStartTime == 0 || m_loadStartTime > m_setURLTime)
            m_loadStartTime = m_setURLTime;

        char buffer[TUNE_LOG_MAX_SIZE];

        int len = snprintf(buffer, TUNE_LOG_MAX_SIZE, "QAM_TUNE_TIME:TYPE-%s,%u,%u,%u,%u,%u",
                           m_contentType.c_str(),
                           TO_MS(m_loadedTime - m_loadStartTime),        // time taken by load
                           TO_MS(m_loadCompleteTime - m_loadStartTime),  // time of onLoaded event
                           TO_MS(m_playEndTime - m_playStartTime),       // time taken by play
                           TO_MS(m_firstFrameTime - m_playStartTime),    // time to render first frame
                           TO_MS(m_firstFrameTime - m_loadStartTime));   // tune time

        if (len > 0)
        {
            tuneTimeLog = std::string(buffer, len);
        }

        m_setURLTime = m_loadStartTime = 0;
    }

    const std::string& lang = m_mediaPlayer->rmf_getAudioLanguages();
    //const std::string& ccDescriptor = m_mediaPlayer->rmf_getCaptionDescriptor();
    std::string speeds = "-60.0, -30.0, -15.0, -4.0, 1.0, 0.5, 4.0, 15.0, 30.0, 60.0";
    getParent()->updateVideoMetadata(lang, speeds, m_duration, 1280, 720);//TODO fix 1280x720
    if (!tuneTimeLog.empty())
    {
        getParent()->getEventEmitter().send(OnMetricLogEvent(tuneTimeLog.c_str()));
    }
    getParent()->getEventEmitter().send(OnProgressEvent(this));//fire the first progress event to signal we are tuned
}

void RMFPlayer::mediaWarningReceived()
{
    if (!m_isLoaded)
        return;
    uint32_t code = m_mediaPlayer->rmf_getMediaWarnData();
    const std::string& description = m_mediaPlayer->rmf_getMediaWarnDescription();
    getParent()->getEventEmitter().send(OnWarningEvent(code, description));
}

void RMFPlayer::eissDataReceived()
{
    if (!m_isLoaded)
        return;
    const std::string& eissData = m_mediaPlayer->rmf_getEISSDataBuffer();
    getParent()->getEventEmitter().send(OnEISSDataReceivedEvent(eissData));
}

void RMFPlayer::volumeChanged(float volume)
{
}

void RMFPlayer::playerStateChanged()
{
    MediaPlayer::RMFPlayerState oldState = m_playerState;
    MediaPlayer::RMFPlayerState newState = m_mediaPlayer->rmf_playerState();
    if (newState == oldState)
        return;

    LOG_INFO("oldState=%s newState=%s", StateString(oldState), StateString(newState));

    if (newState == MediaPlayer::RMF_PLAYER_FORMATERROR ||
        newState == MediaPlayer::RMF_PLAYER_NETWORKERROR ||
        newState == MediaPlayer::RMF_PLAYER_DECODEERROR)
    {
        uint32_t code = static_cast<uint32_t>(m_mediaPlayer->rmf_playerState());
        const std::string& description = m_mediaPlayer->rmf_getMediaErrorMessage();
        getParent()->getEventEmitter().send(OnErrorEvent(code, description));
        return;
    }
    m_playerState = newState;
}

void RMFPlayer::videoStateChanged()
{
    MediaPlayer::RMFVideoBufferState oldState = m_videoState;
    MediaPlayer::RMFVideoBufferState newState = m_mediaPlayer->rmf_videoState();
    if (newState == oldState)
        return;

    LOG_INFO("oldState=%s newState=%s", StateString(oldState), StateString(newState));

    m_videoState = newState;
    if (oldState > m_videoStateMaximum)
        m_videoStateMaximum = oldState;

    bool isPotentiallyPlaying = potentiallyPlaying();

    bool isTimeUpdated = false;
    isTimeUpdated  = (isPotentiallyPlaying && newState < MediaPlayer::RMF_VIDEO_BUFFER_HAVEFUTUREDATA);
    isTimeUpdated |= (newState >= MediaPlayer::RMF_VIDEO_BUFFER_HAVEMETADATA    && oldState < MediaPlayer::RMF_VIDEO_BUFFER_HAVEMETADATA);
    isTimeUpdated |= (newState >= MediaPlayer::RMF_VIDEO_BUFFER_HAVECURRENTDATA && oldState < MediaPlayer::RMF_VIDEO_BUFFER_HAVEMETADATA);

    if (isTimeUpdated)
        timeChanged();

    bool isPlaying = false;
    if (isPotentiallyPlaying)
    {
        isPlaying  = (newState >= MediaPlayer::RMF_VIDEO_BUFFER_HAVEFUTUREDATA && oldState <= MediaPlayer::RMF_VIDEO_BUFFER_HAVECURRENTDATA);
    }

    if (isPlaying)
    {
        m_isPaused = false;
        getParent()->getEventEmitter().send(OnPlayingEvent());
    }
    else if (!m_isPaused &&
             m_videoStateMaximum >= MediaPlayer::RMF_VIDEO_BUFFER_HAVEFUTUREDATA &&
             m_videoState > MediaPlayer::RMF_VIDEO_BUFFER_HAVENOTHING &&
             m_videoState < MediaPlayer::RMF_VIDEO_BUFFER_HAVEFUTUREDATA)
    {
        m_isPaused = true;
        getParent()->getEventEmitter().send(OnPausedEvent());
    }
}

void RMFPlayer::durationChanged()
{
    doTimeUpdate();
}

void RMFPlayer::timeChanged()
{
    doTimeUpdate();
}

void RMFPlayer::rateChanged()
{
    float rate = m_mediaPlayer->rmf_getRate();
    if (rate != m_lastReportedPlaybackRate)
    {
        m_lastReportedPlaybackRate = rate;
        LOG_INFO("newRate=%f", rate);
        doTimeUpdate(true);
        getParent()->getEventEmitter().send(OnSpeedChangeEvent(rate));
    }
}

void RMFPlayer::videoDecoderHandleReceived()
{
    getParent()->onVidHandleReceived(m_mediaPlayer->rmf_getCCDecoderHandle());
}
//MediaPlayerClient impl end

void RMFPlayer::doTimeUpdate(bool forced)
{
    if (!m_isLoaded)
        return;

    refreshCachedCurrentTime(forced);

    if (m_duration != m_lastReportedDuration)
    {
        m_lastReportedDuration = m_duration;
    }
    if (m_currentTime != m_lastReportedCurrentTime)
    {
        m_lastReportedCurrentTime = m_currentTime;
        getParent()->getEventEmitter().send(OnProgressEvent(this));
    }
}

void RMFPlayer::startPlaybackProgressMonitor()
{
    if (m_playbackProgressMonitorTag != 0)
        return;
    static const auto triggerTimeUpdateFunc = [](gpointer data) -> gboolean {
        RMFPlayer &self = *static_cast<RMFPlayer*>(data);
        self.doTimeUpdate();
        return G_SOURCE_CONTINUE;
    };
    LOG_INFO("WMR startPlaybackProgressMonitor starting timer");
    m_playbackProgressMonitorTag = g_timeout_add(kProgressMonitorTimeoutMs, triggerTimeUpdateFunc, this);
}

void RMFPlayer::stopPlaybackProgressMonitor()
{
    LOG_INFO("WMR stopPlaybackProgressMonitor");
    if (m_playbackProgressMonitorTag)
        g_source_remove(m_playbackProgressMonitorTag);
    m_playbackProgressMonitorTag = 0;
    m_lastRefreshTime = 0;
}

bool RMFPlayer::refreshCachedCurrentTime(bool forced)
{
    if (!m_isLoaded)
        return false;

    gint64 now = g_get_monotonic_time();
    if (!forced && m_currentTime > 0.f && (now - m_lastRefreshTime) < kRefreshCachedTimeThrottleMs * 1000)
        return false;

    m_duration = m_mediaPlayer->rmf_getDuration();
    m_currentTime = m_mediaPlayer->rmf_getCurrentTime();
    m_lastRefreshTime = now;

    return true;
}

bool RMFPlayer::endedPlayback() const
{
    float dur = m_mediaPlayer->rmf_getDuration();
    if (!std::isnan(dur))
        return false;
    if (m_videoState < MediaPlayer::RMF_VIDEO_BUFFER_HAVEMETADATA)
        return false;
    int rate = m_mediaPlayer->rmf_getRate();
    float now = m_mediaPlayer->rmf_getCurrentTime();
    if (rate > 0)
        return dur > 0 && now >= dur;
    if (rate < 0)
        return now <= 0;
    return false;
}

bool RMFPlayer::couldPlayIfEnoughData() const
{
    if (m_videoState >= MediaPlayer::RMF_VIDEO_BUFFER_HAVEMETADATA && m_playerState >= MediaPlayer::RMF_PLAYER_FORMATERROR)
        return false;
    else
        return !m_mediaPlayer->rmf_isPaused() && !endedPlayback();
}

bool RMFPlayer::potentiallyPlaying() const
{
    bool pausedToBuffer = (m_videoStateMaximum >= MediaPlayer::RMF_VIDEO_BUFFER_HAVEFUTUREDATA) && (m_videoState < MediaPlayer::RMF_VIDEO_BUFFER_HAVEFUTUREDATA);
    return (pausedToBuffer || m_videoState >= MediaPlayer::RMF_VIDEO_BUFFER_HAVEFUTUREDATA) && couldPlayIfEnoughData();
}

bool RMFPlayer::setContentType(const std::string &url, std::string& contentType)
{
    const char* dvrStrIdentifier  = "recordingId=";
    const char* liveStrIdentifier = "live=ocap://";
    const char* vodStrIdentifier  = "live=vod://";

    if (url.find(dvrStrIdentifier) != std::string::npos)
    {
        if (url.find("169.") != std::string::npos)
        {
            contentType = "mDVR";
        }
        else
        {
            contentType = "DVR";
        }
    }
    else if (url.find(liveStrIdentifier) != std::string::npos)
    {
        contentType = "LIVE";
    }
    else if (url.find(vodStrIdentifier) != std::string::npos)
    {
        contentType = "VOD";
    } 
    else
    {
        contentType = "Unsupported";
        return false;
    }
    return true;
}

