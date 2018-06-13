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
#ifndef RDKMEDIAPLAYER_H
#define RDKMEDIAPLAYER_H

#include <rtRemote.h>
#include <rtError.h>
#include <string>
#include <map>
#include "rtevents.h"
#include "intrect.h"
#include "rdkmediaplayerimpl.h"
#ifndef DISABLE_CLOSEDCAPTIONS
#include "closedcaptions.h"
#endif

class RDKMediaPlayer : public rtObject {

public:
    rtDeclareObject(RDKMediaPlayer, rtObject);
    rtProperty(url, currentURL, setCurrentURL, rtString);
    rtProperty(audioLanguage, audioLanguage, setAudioLanguage, rtString);
    rtProperty(secondaryAudioLanguage, secondaryAudioLanguage, setSecondaryAudioLanguage, rtString);
    rtProperty(position, position, setPosition, float);
    rtProperty(speed, speed, setSpeed, float);
    rtProperty(volume, volume, setVolume, float);
    rtProperty(blocked, blocked, setBlocked, rtString);
    rtProperty(zoom, zoom, setZoom, rtString);
    rtProperty(autoPlay, autoPlay, setAutoPlay, rtString);
    rtProperty(closedCaptionsEnabled, closedCaptionsEnabled, setClosedCaptionsEnabled, rtString);
    rtProperty(closedCaptionsOptions, closedCaptionsOptions, setClosedCaptionsOptions, rtObjectRef);
    rtProperty(closedCaptionsTrack, closedCaptionsTrack, setClosedCaptionsTrack, rtString);
    rtProperty(contentOptions, contentOptions, setContentOptions, rtObjectRef);
    rtProperty(tsbEnabled, tsbEnabled, setTsbEnabled, rtString);
    rtReadOnlyProperty(duration, duration, float);
    rtReadOnlyProperty(availableAudioLanguages, availableAudioLanguages, rtString);
    rtReadOnlyProperty(availableClosedCaptionsLanguages, availableClosedCaptionsLanguages, rtString);
    rtReadOnlyProperty(availableSpeeds, availableSpeeds, rtString);
    rtProperty(isInProgressRecording, isInProgressRecording, setIsInProgressRecording, rtString);
    rtProperty(networkBufferSize, networkBufferSize, setNetworkBufferSize, int32_t);
    rtProperty(videoBufferLength, videoBufferLength, setVideoBufferLength, float);
    rtProperty(eissFilterStatus, eissFilterStatus, setEissFilterStatus, rtString);
    rtProperty(loadStartTime, loadStartTime, setLoadStartTime, int64_t);
    rtMethod4ArgAndNoReturn("setVideoRectangle", setVideoRectangle, int32_t, int32_t, int32_t, int32_t);
    rtMethodNoArgAndNoReturn("play", play);
    rtMethodNoArgAndNoReturn("pause", pause);
    rtMethodNoArgAndNoReturn("seekToLive", seekToLive);
    rtMethodNoArgAndNoReturn("stop", stop);
    rtMethod2ArgAndNoReturn("setSpeed", changeSpeed, float, int32_t);
    rtMethod1ArgAndNoReturn("setPositionRelative", setPositionRelative, float);
    rtMethod1ArgAndNoReturn("setAdditionalAuth", setAdditionalAuth, rtObjectRef);
    rtMethodNoArgAndNoReturn("requestStatus", requestStatus);
    rtMethod2ArgAndNoReturn("on", setListener, rtString, rtFunctionRef);
    rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);
    rtMethodNoArgAndNoReturn("destroy", destroy);

    RDKMediaPlayer();
    virtual ~RDKMediaPlayer();

    virtual void onInit();

    //rtRemote properties
    rtError currentURL(rtString& s) const;
    rtError setCurrentURL(rtString const& s);
    rtError audioLanguage(rtString& s) const;
    rtError setAudioLanguage(rtString const& s);
    rtError secondaryAudioLanguage(rtString& s) const;
    rtError setSecondaryAudioLanguage(rtString const& s);
    rtError position(float& t) const;
    rtError setPosition(float const& t);
    rtError speed(float& t) const;
    rtError setSpeed(float const& t);
    rtError volume(float& t) const;
    rtError setVolume(float const& t);
    rtError blocked(rtString& t) const;
    rtError setBlocked(rtString const& t);
    rtError autoPlay(rtString& t) const;
    rtError setAutoPlay(rtString const& t);
    rtError isInProgressRecording(rtString& t) const;
    rtError setIsInProgressRecording(rtString const& t);
    rtError zoom(rtString& t) const;
    rtError setZoom(rtString const& t);
    rtError networkBufferSize(int32_t& t) const;
    rtError setNetworkBufferSize(int32_t const& t);
    rtError videoBufferLength(float& t) const;
    rtError setVideoBufferLength(float const& t);
    rtError eissFilterStatus(rtString& t) const;
    rtError setEissFilterStatus(rtString const& t);
    rtError loadStartTime(int64_t& t) const;
    rtError setLoadStartTime(int64_t const& t);
    rtError closedCaptionsEnabled(rtString& t) const;
    rtError setClosedCaptionsEnabled(rtString const& t);
    rtError closedCaptionsOptions(rtObjectRef& t) const;
    rtError setClosedCaptionsOptions(rtObjectRef const& t);
    rtError closedCaptionsTrack(rtString& t) const;
    rtError setClosedCaptionsTrack(rtString const& t);
    rtError contentOptions(rtObjectRef& t) const;
    rtError setContentOptions(rtObjectRef const& t);
    rtError duration(float& t) const;
    rtError availableAudioLanguages(rtString& t) const;
    rtError availableClosedCaptionsLanguages(rtString& t) const;
    rtError availableSpeeds(rtString& t) const;
    rtError tsbEnabled(rtString& t) const;
    rtError setTsbEnabled(rtString const& t);

    //rtRemote methods
    rtError setVideoRectangle(int32_t x, int32_t y, int32_t w, int32_t h);
    rtError play();
    rtError pause();
    rtError seekToLive();
    rtError stop();
    rtError changeSpeed(float speed, int32_t overshootTime);
    rtError setPositionRelative(float seconds);
    rtError requestStatus();
    rtError setAdditionalAuth(rtObjectRef const& t);
    rtError setListener(rtString eventName, const rtFunctionRef& f);
    rtError delListener(rtString  eventName, const rtFunctionRef& f);
    rtError destroy();

    bool getIsBlocked() const;
    const gint64& getSetURLTime() const;
    const gint64& getLoadStartTime() const;
    const gint64& getPlayStartTime() const;
    EventEmitter& getEventEmitter();
    void onVidHandleReceived(uint32_t handle);


    //status updates from impls
    void updateVideoMetadata(const std::string& languages, const std::string& speeds, int duration, int width, int height);

private:
    void updateClosedCaptionsState();

    std::vector<RDKMediaPlayerImpl*> m_playerCache;
    RDKMediaPlayerImpl* m_pImpl;
#ifndef DISABLE_CLOSEDCAPTIONS
    ClosedCaptions m_closedCaptions;
#endif
    EventEmitter m_eventEmitter;

    std::string m_currentURL;
    std::string m_audioLanguage;
    std::string m_secondaryAudioLanguage;
    IntRect m_videoRect;
    float m_seekTime;
    float m_speed;
    float m_volume;
    float m_videoBufferLength;
    bool m_isLoaded;
    bool m_isBlocked;
    bool m_eissFilterStatus;
    bool m_isInProgressRecording;
    bool m_isPaused;
    bool m_tsbEnabled;
    int32_t m_zoom;
    int32_t m_networkBufferSize;
    int32_t m_overshootTime;
    gint64 m_loadStartTime { 0 };
    gint64 m_setURLTime { 0 };
    gint64 m_playStartTime { 0 };
    bool m_closedCaptionsEnabled;
    std::map<std::string, std::string> m_closedCaptionsOptions;
    std::string m_closedCaptionsTrack;
    uint32_t m_vidDecoderHandle;
    rtObjectRef m_contentOptions;
    bool m_autoPlay;

    //status info
    bool m_duration;
    std::string m_availableAudioLanguages;
    std::string m_availableClosedCaptionsLanguages;
    std::string m_availableSpeeds;
};
#endif  // _RT_AAMP_PLAYER_H_
