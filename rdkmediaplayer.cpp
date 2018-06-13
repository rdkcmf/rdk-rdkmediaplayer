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
#include "rdkmediaplayer.h"
#include <glib.h>
#include <string>
#include <limits>
#include <unistd.h>
#include "logger.h"
#include "rmfplayer.h"
#include "aampplayer.h"

struct ReleaseOnScopeEnd {
    rtObject& obj;
    ReleaseOnScopeEnd(rtObject& o) : obj(o)
    {
    }
    ~ReleaseOnScopeEnd()
    {
        obj.Release();
    }
};

#define CALL_ON_MAIN_THREAD(body) \
    do { \
        this->AddRef(); \
        g_timeout_add_full( \
            G_PRIORITY_HIGH, \
            0, \
            [](gpointer data) -> gboolean \
            { \
                RDKMediaPlayer &self = *static_cast<RDKMediaPlayer*>(data); \
                ReleaseOnScopeEnd releaseOnScopeEnd(self); \
                body \
                return G_SOURCE_REMOVE; \
            }, \
            this, \
            0); \
    } while(0)

void stringToBool(rtString const& s, bool& b)
{
    if(s.compare("true") == 0)
        b = true;
    else if(s.compare("false") == 0)
        b = false;
    //else b remains unchanged
    //LOG_INFO("%s in=%s out=%d\n", __PRETTY_FUNCTION__, s.cString(), (int)b);
}

void boolToString(bool b, rtString& s)
{
    s = b ? rtString("true") : rtString("false");
}

rtDefineObject    (RDKMediaPlayer, rtObject);
rtDefineProperty  (RDKMediaPlayer, url);
rtDefineProperty  (RDKMediaPlayer, audioLanguage);
rtDefineProperty  (RDKMediaPlayer, secondaryAudioLanguage);
rtDefineProperty  (RDKMediaPlayer, position);
rtDefineProperty  (RDKMediaPlayer, speed);
rtDefineProperty  (RDKMediaPlayer, volume);
rtDefineProperty  (RDKMediaPlayer, blocked);
rtDefineProperty  (RDKMediaPlayer, isInProgressRecording);;
rtDefineProperty  (RDKMediaPlayer, zoom);
rtDefineProperty  (RDKMediaPlayer, networkBufferSize);
rtDefineProperty  (RDKMediaPlayer, videoBufferLength);
rtDefineProperty  (RDKMediaPlayer, eissFilterStatus);
rtDefineProperty  (RDKMediaPlayer, loadStartTime);
rtDefineProperty  (RDKMediaPlayer, closedCaptionsEnabled);
rtDefineProperty  (RDKMediaPlayer, closedCaptionsOptions);
rtDefineProperty  (RDKMediaPlayer, closedCaptionsTrack);
rtDefineProperty  (RDKMediaPlayer, contentOptions);
rtDefineProperty  (RDKMediaPlayer, autoPlay);
rtDefineProperty  (RDKMediaPlayer, duration);
rtDefineProperty  (RDKMediaPlayer, availableAudioLanguages);
rtDefineProperty  (RDKMediaPlayer, availableClosedCaptionsLanguages);
rtDefineProperty  (RDKMediaPlayer, availableSpeeds);
rtDefineProperty  (RDKMediaPlayer, tsbEnabled);
rtDefineMethod    (RDKMediaPlayer, setVideoRectangle);
rtDefineMethod    (RDKMediaPlayer, play);
rtDefineMethod    (RDKMediaPlayer, pause);
rtDefineMethod    (RDKMediaPlayer, seekToLive);
rtDefineMethod    (RDKMediaPlayer, stop);
rtDefineMethod    (RDKMediaPlayer, changeSpeed);
rtDefineMethod    (RDKMediaPlayer, setPositionRelative);
rtDefineMethod    (RDKMediaPlayer, requestStatus);
rtDefineMethod    (RDKMediaPlayer, setAdditionalAuth);
rtDefineMethod    (RDKMediaPlayer, setListener);
rtDefineMethod    (RDKMediaPlayer, delListener);
rtDefineMethod    (RDKMediaPlayer, destroy);

RDKMediaPlayer::RDKMediaPlayer() :
    m_pImpl(0),
    m_speed(1),
    m_vidDecoderHandle(0),
    m_closedCaptionsEnabled(false),
    m_autoPlay(false)
{
    m_closedCaptionsOptions["textSize"] = "";
    m_closedCaptionsOptions["fontStyle"] = "";
    m_closedCaptionsOptions["textForegroundColor"] = "";
    m_closedCaptionsOptions["textForegroundOpacity"] = "";
    m_closedCaptionsOptions["textBackgroundColor"] = "";
    m_closedCaptionsOptions["textBackgroundOpacity"] = "";
    m_closedCaptionsOptions["textItalicized"] = "";
    m_closedCaptionsOptions["textUnderline"] = "";
    m_closedCaptionsOptions["windowFillColor"] = "";
    m_closedCaptionsOptions["windowFillOpacity"] = "";
    m_closedCaptionsOptions["windowBorderEdgeColor"] = "";
    m_closedCaptionsOptions["windowBorderEdgeStyle"] = "";
    m_closedCaptionsOptions["textEdgeColor"] = "";
    m_closedCaptionsOptions["textEdgeStyle"] = "";
    m_closedCaptionsOptions["fontSize"] = "";
    m_audioLanguage = "en";
    m_loadStartTime = g_get_monotonic_time();//can be set more exactly via loadStartTime property
}

RDKMediaPlayer::~RDKMediaPlayer()
{
    LOG_WARNING("Destroying RDKMediaPlayer (should almost NEVER happen!)");
}

void RDKMediaPlayer::onInit()
{
    LOG_INFO("%s\n", __PRETTY_FUNCTION__);
}

// rtRemote properties
rtError RDKMediaPlayer::currentURL(rtString& s) const
{
    s = m_currentURL.c_str();
    return RT_OK;
}
rtError RDKMediaPlayer::setCurrentURL(rtString const& s)
{
    m_setURLTime = g_get_monotonic_time();
    m_currentURL = s.cString();
    LOG_INFO("%s %s\n", __PRETTY_FUNCTION__, m_currentURL.c_str());
    m_vidDecoderHandle = 0;
    m_duration = 0;
    updateClosedCaptionsState();//m_vidDecoderHandle = 0, so CC will be disabled

    if(m_pImpl)
    {
        m_pImpl->doStop();
        usleep(100000);//TODO REMOVE
        m_pImpl = 0;
    }

    for(std::vector<RDKMediaPlayerImpl*>::iterator it = m_playerCache.begin(); it != m_playerCache.end(); ++it)
    {
        if( (*it)->doCanPlayURL(m_currentURL) )
        {
            LOG_INFO("RDKMediaPlayer::setCurrentURL: Reusing cached player");
            m_pImpl = *it;
        }
    }

    if(!m_pImpl)
    {
        if(AAMPPlayer::canPlayURL(m_currentURL))
        {
            LOG_INFO("RDKMediaPlayer::setCurrentURL:Creating AAMPPlayer");
            m_pImpl = new AAMPPlayer(this);
        }
        else 
        if(RMFPlayer::canPlayURL(m_currentURL))
        {
            LOG_INFO("RDKMediaPlayer::setCurrentURL:Creating RMFPlayer");
            m_pImpl = new RMFPlayer(this);
        }
        else
        {
            LOG_WARNING("RDKMediaPlayer::setCurrentURL:Unsupported media type!");
            return RT_FAIL;
        }

        m_pImpl->doInit();
        m_playerCache.push_back(m_pImpl);
        getEventEmitter().send(OnPlayerInitializedEvent());
    }

    CALL_ON_MAIN_THREAD (
        bool isCurrentURLEmpty = false;
        {
            isCurrentURLEmpty = self.m_currentURL.size() == 0;
        }
        LOG_INFO("currentURL='%s'", self.m_currentURL.c_str());
        if (isCurrentURLEmpty)
        {
            self.m_pImpl->doStop();
            self.m_isLoaded = false;
        }
        else
        {
            self.m_pImpl->doLoad(self.m_currentURL);
            self.m_isLoaded = true;
            if(self.m_autoPlay)
                self.play();
            self.m_pImpl->doSetVideoRectangle(self.m_videoRect);
        }
    );
    return RT_OK;
}

rtError RDKMediaPlayer::audioLanguage(rtString& s) const
{
    s = m_audioLanguage.c_str();
    return RT_OK;
}

rtError RDKMediaPlayer::setAudioLanguage(rtString const& s)
{
    m_audioLanguage = s.cString();
    LOG_INFO("%s %s\n", __PRETTY_FUNCTION__, m_audioLanguage.c_str());
    if(m_pImpl)
    {
        CALL_ON_MAIN_THREAD(self.m_pImpl->doSetAudioLanguage(self.m_audioLanguage););
    }
    return RT_OK;
}

rtError RDKMediaPlayer::secondaryAudioLanguage(rtString& s) const
{
    s = m_secondaryAudioLanguage.c_str();
    return RT_OK;
}
rtError RDKMediaPlayer::setSecondaryAudioLanguage(rtString const& s)
{
    m_secondaryAudioLanguage = s.cString();
    LOG_INFO("%s %s\n", __PRETTY_FUNCTION__, m_secondaryAudioLanguage.c_str());
    return RT_OK;
}

rtError RDKMediaPlayer::position(float& t) const
{
    if(m_pImpl)
    {
        ProgressData pd;
        m_pImpl->getProgressData(&pd);
        t = pd.position;
    }
    else
        t = 0;
    return RT_OK;
}

rtError RDKMediaPlayer::setPosition(float const& t)
{
    LOG_INFO("%s %f\n", __PRETTY_FUNCTION__, t);
    m_seekTime = t;
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSetPosition(self.m_seekTime););
    return RT_OK;
}

rtError RDKMediaPlayer::speed(float& t) const
{
    if(m_pImpl)
    {
        ProgressData pd;
        m_pImpl->getProgressData(&pd);
        t = pd.speed;
    }
    else
        t = 0;
    return RT_OK;
}

rtError RDKMediaPlayer::setSpeed(float const& t)
{
    LOG_INFO("%s %f\n", __PRETTY_FUNCTION__, t);
    m_speed = t;
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSetSpeed(self.m_speed););
    updateClosedCaptionsState();
    return RT_OK;
}

rtError RDKMediaPlayer::volume(float& t) const
{
    t = m_volume;
    return RT_OK;
}
rtError RDKMediaPlayer::setVolume(float const& t)
{
    LOG_INFO("%s %f\n", __PRETTY_FUNCTION__, t);
    m_volume = t;
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSetVolume(self.m_volume););
    return RT_OK;
}

rtError RDKMediaPlayer::blocked(rtString& t) const
{
    boolToString(m_isBlocked, t);
    return RT_OK;
}

rtError RDKMediaPlayer::setBlocked(rtString const& t)
{
    stringToBool(t, m_isBlocked);
    LOG_INFO("%s %d\n", __PRETTY_FUNCTION__, (int)m_isBlocked);
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSetBlocked(self.m_isBlocked););
    return RT_OK;
}

rtError RDKMediaPlayer::isInProgressRecording(rtString& t) const
{
    boolToString(m_isInProgressRecording, t);
    return RT_OK;
}
rtError RDKMediaPlayer::setIsInProgressRecording(rtString const& t)
{
    stringToBool(t, m_isInProgressRecording);
    LOG_INFO("%s %d\n", __PRETTY_FUNCTION__, (int)m_isInProgressRecording);
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSetIsInProgressRecording(self.m_isInProgressRecording););
    return RT_OK;
}

rtError RDKMediaPlayer::zoom(rtString& t) const
{
    t = m_zoom ? rtString("NONE") : rtString("FULL");
    return RT_OK;
}

rtError RDKMediaPlayer::setZoom(rtString const& t)
{
    if(t.compare("FULL")==0)
        m_zoom = 0;
    else
        m_zoom = 1;
    LOG_INFO("%s %s=%d\n", __PRETTY_FUNCTION__, t.cString(), m_zoom);
    if(m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSetZoom(self.m_zoom););
    return RT_OK;
}

rtError RDKMediaPlayer::networkBufferSize(int32_t& t) const
{
    t = m_networkBufferSize;
    return RT_OK;
}

rtError RDKMediaPlayer::setNetworkBufferSize(int32_t const& t)
{
    LOG_INFO("%s %d\n", __PRETTY_FUNCTION__, t);
    m_networkBufferSize = t;
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSetNetworkBufferSize(self.m_networkBufferSize););
    return RT_OK;
}

rtError RDKMediaPlayer::videoBufferLength(float& t) const
{
    t = m_videoBufferLength;
    return RT_OK;
}
rtError RDKMediaPlayer::setVideoBufferLength(float const& t)
{
    LOG_INFO("%s %d\n", __PRETTY_FUNCTION__, t);
    m_videoBufferLength = t;
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSetVideoBufferLength(self.m_videoBufferLength););
    return RT_OK;
}

rtError RDKMediaPlayer::eissFilterStatus(rtString& t) const
{
    boolToString(m_eissFilterStatus, t);
    return RT_OK;
}

rtError RDKMediaPlayer::setEissFilterStatus(rtString const& t)
{
    stringToBool(t, m_eissFilterStatus);
    LOG_INFO("%s %d\n", __PRETTY_FUNCTION__, (int)m_eissFilterStatus);
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSetEISSFilterStatus(self.m_eissFilterStatus););
    return RT_OK;
}

rtError RDKMediaPlayer::loadStartTime(int64_t& t) const
{
    t = m_loadStartTime;
    return RT_OK;
}

rtError RDKMediaPlayer::setLoadStartTime(int64_t const& t)
{
    LOG_INFO("%s %lld\n", __PRETTY_FUNCTION__, t);
    m_loadStartTime = t;
    return RT_OK;
}

rtError RDKMediaPlayer::closedCaptionsEnabled(rtString& t) const
{
    boolToString(m_closedCaptionsEnabled, t);
    return RT_OK;
}

rtError RDKMediaPlayer::setClosedCaptionsEnabled(rtString const& t)
{
    stringToBool(t, m_closedCaptionsEnabled);
    LOG_INFO("%s %d\n", __PRETTY_FUNCTION__, (int)m_closedCaptionsEnabled);
    updateClosedCaptionsState();
    return RT_OK;
}

rtError RDKMediaPlayer::closedCaptionsOptions(rtObjectRef& t) const
{
    for(std::map<std::string, std::string>::const_iterator it = m_closedCaptionsOptions.begin(); 
        it != m_closedCaptionsOptions.end(); 
        it++)
    {
        t.set(it->first.c_str(), rtValue(it->second.c_str()));
    }
    return RT_OK;
}

rtError RDKMediaPlayer::setClosedCaptionsOptions(rtObjectRef const& t)
{
    for(std::map<std::string, std::string>::iterator it = m_closedCaptionsOptions.begin(); 
        it != m_closedCaptionsOptions.end(); 
        it++)
    {
        rtValue v;
        if(t.get(it->first.c_str(), v) == RT_OK)
            it->second = v.convert<rtString>().cString();
    }

    for(std::map<std::string, std::string>::iterator it = m_closedCaptionsOptions.begin(); 
        it != m_closedCaptionsOptions.end(); 
        it++)
    {
        LOG_INFO("%s %s:%d\n", __PRETTY_FUNCTION__, it->first.c_str(), it->second.c_str());
    }
    return RT_OK;
}

rtError RDKMediaPlayer::closedCaptionsTrack(rtString& t) const
{
    t = m_closedCaptionsTrack.c_str();
    return RT_OK;
}

rtError RDKMediaPlayer::setClosedCaptionsTrack(rtString const& t)
{
    m_closedCaptionsTrack = t.cString();
    LOG_INFO("%s %s\n", __PRETTY_FUNCTION__, m_closedCaptionsTrack.c_str());
    return RT_OK;
}

rtError RDKMediaPlayer::contentOptions(rtObjectRef& t) const
{
    t = m_contentOptions;
    return RT_OK;
}

rtError RDKMediaPlayer::setContentOptions(rtObjectRef const& t)
{
    m_contentOptions = t;
    return RT_OK;
}

rtError RDKMediaPlayer::autoPlay(rtString& t) const
{
    boolToString(m_autoPlay, t);
    return RT_OK;
}

rtError RDKMediaPlayer::setAutoPlay(rtString const& t)
{
    LOG_INFO("%s %s\n", __PRETTY_FUNCTION__, t.cString());
    stringToBool(t, m_autoPlay);
    return RT_OK;
}

rtError RDKMediaPlayer::duration(float& t) const
{
    t = m_duration;
    return RT_OK;
}

rtError RDKMediaPlayer::availableAudioLanguages(rtString& t) const
{
    t = m_availableAudioLanguages.c_str();
    return RT_OK;
}

rtError RDKMediaPlayer::availableClosedCaptionsLanguages(rtString& t) const
{
    t = m_availableClosedCaptionsLanguages.c_str();
    return RT_OK;
}

rtError RDKMediaPlayer::availableSpeeds(rtString& t) const
{
    t = m_availableSpeeds.c_str();
    return RT_OK;
}

rtError RDKMediaPlayer::tsbEnabled(rtString& t) const
{
    boolToString(m_tsbEnabled, t);
    return RT_OK;
}

rtError RDKMediaPlayer::setTsbEnabled(rtString const& t)
{
    LOG_INFO("%s %s\n", __PRETTY_FUNCTION__, t.cString());
    stringToBool(t, m_tsbEnabled);
    return RT_OK;
}


// rtRemote methods
rtError RDKMediaPlayer::setVideoRectangle(int32_t x, int32_t y, int32_t w, int32_t h)
{
    LOG_INFO("%s %d %d %d %d\n", __PRETTY_FUNCTION__, x, y, w, h);
    m_videoRect = IntRect(x,y,w,h);
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSetVideoRectangle(self.m_videoRect););
    return RT_OK;
}

rtError RDKMediaPlayer::play()
{
    LOG_INFO("%s\n", __PRETTY_FUNCTION__);
    m_playStartTime = g_get_monotonic_time();
    m_speed = 1;
    m_overshootTime = 0;
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doPlay(););
    updateClosedCaptionsState();
    return RT_OK;
}

rtError RDKMediaPlayer::pause()
{
    LOG_INFO("%s\n", __PRETTY_FUNCTION__);
    m_speed = 0;
    m_overshootTime = 0;
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doPause(););
    return RT_OK;
}

rtError RDKMediaPlayer::seekToLive()
{
    LOG_INFO("%s\n", __PRETTY_FUNCTION__);
    m_speed = 1;
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSeekToLive(););
    return RT_OK;
}


rtError RDKMediaPlayer::stop()
{
    LOG_INFO("%s\n", __PRETTY_FUNCTION__);
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doStop(););
    m_isLoaded = false;
    return RT_OK;
}

rtError RDKMediaPlayer::changeSpeed(float speed, int32_t overshootTime)
{
    LOG_INFO("%s %f %d\n", __PRETTY_FUNCTION__, speed, overshootTime);
    m_speed = speed;
    m_overshootTime = overshootTime;
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doChangeSpeed(self.m_speed, self.m_overshootTime););
    return RT_OK;
}

rtError RDKMediaPlayer::setPositionRelative(float seconds)
{
    if(m_pImpl)
    {
        ProgressData pd;
        m_pImpl->getProgressData(&pd);
        m_seekTime = pd.position + seconds/1000.0;
    }
    else
        m_seekTime = 0 + seconds/1000.0;
    if(m_seekTime < 0)
        m_seekTime = 0;
    if(!m_pImpl)
        return RT_OK;
    CALL_ON_MAIN_THREAD(self.m_pImpl->doSetPosition(self.m_seekTime););
    return RT_OK;
}

rtError RDKMediaPlayer::requestStatus()
{
    LOG_INFO("%s\n", __PRETTY_FUNCTION__);
    if(m_pImpl)
        getEventEmitter().send(OnStatusEvent(m_pImpl));
    return RT_OK;
}

rtError RDKMediaPlayer::setAdditionalAuth(rtObjectRef const& t)
{
    LOG_INFO("%s\n", __PRETTY_FUNCTION__);
    return RT_OK;
}

rtError RDKMediaPlayer::setListener(rtString eventName, const rtFunctionRef& f)
{
    return m_eventEmitter.setListener(eventName, f);
}

rtError RDKMediaPlayer::delListener(rtString  eventName, const rtFunctionRef& f)
{
    return m_eventEmitter.delListener(eventName, f);
}

rtError RDKMediaPlayer::destroy()
{
    LOG_INFO("%s\n", __PRETTY_FUNCTION__);
    exit(0);
}

bool RDKMediaPlayer::getIsBlocked() const
{
    return m_isBlocked;
}

const gint64& RDKMediaPlayer::getSetURLTime() const
{
    return m_setURLTime;
}

const gint64& RDKMediaPlayer::getLoadStartTime() const
{
    return m_loadStartTime;
}

const gint64& RDKMediaPlayer::getPlayStartTime() const
{
    return m_playStartTime;
}

EventEmitter& RDKMediaPlayer::getEventEmitter()
{
    return m_eventEmitter;
}

void RDKMediaPlayer::onVidHandleReceived(uint32_t handle)
{
    LOG_INFO("%s %u\n", __PRETTY_FUNCTION__, handle);
    m_vidDecoderHandle = handle;
    updateClosedCaptionsState();
}

void RDKMediaPlayer::updateVideoMetadata(const std::string& languages, const std::string& speeds, int duration, int width, int height)
{
    m_availableAudioLanguages = languages;
    m_availableSpeeds = speeds;
#ifndef DISABLE_CLOSEDCAPTIONS
    m_availableClosedCaptionsLanguages = m_closedCaptions.getAvailableTracks();
#endif
    std::string mediaType;//should be  "live", "liveTSB", or "recorded"
    getEventEmitter().send(OnMediaOpenedEvent(mediaType.c_str(), duration, width, height, m_availableSpeeds.c_str(), m_availableAudioLanguages.c_str(),  m_availableClosedCaptionsLanguages.c_str()));
}

void RDKMediaPlayer::updateClosedCaptionsState()
{
#ifndef DISABLE_CLOSEDCAPTIONS
    LOG_INFO("%s m_vidDecoderHandle=%u m_closedCaptionsEnabled=%d m_speed=%f\n", __PRETTY_FUNCTION__, (uint32_t)m_vidDecoderHandle, (int)m_closedCaptionsEnabled, m_speed);

    if(m_vidDecoderHandle && m_closedCaptionsEnabled && m_speed >= 0 && m_speed <= 1 )
    {
        if(!m_closedCaptions.isEnabled())
        {
            LOG_INFO("updateClosedCaptionsState cc start\n");
            if(!m_closedCaptions.setEnabled(true))
            {
                //m_pres->reportMediaWarning(XREProtocol::MEDIA_WARNING_CAPTION_STATE_CHANGE_FAILED, "Failed to set CC state");
            }
            m_closedCaptions.start((void*)m_vidDecoderHandle);
            m_closedCaptions.setVisible(true);
        }
    }
    else
    {
        if(m_closedCaptions.isEnabled())
        {
            LOG_INFO("updateClosedCaptionsState cc stop\n");
            m_closedCaptions.stop();
            if(!m_closedCaptions.setEnabled(false))
            {
                //m_pres->reportMediaWarning(XREProtocol::MEDIA_WARNING_CAPTION_STATE_CHANGE_FAILED, "Failed to set CC state");
            }
        }
    }
#endif
}

