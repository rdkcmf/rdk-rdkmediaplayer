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
#ifndef RDKMEDIAPLAYERIMPL
#define RDKMEDIAPLAYERIMPL

#include <string>
#include <intrect.h>
#include "rtevents.h"

class RDKMediaPlayer;

struct ProgressData 
{ 
    double position, duration, speed, start, end;
    ProgressData() : position(0), duration(0), speed(0), start(-1), end(-1) {}
};

enum TuneState {
    TuneNone,
    TuneStart,
    TuneStop
};

class RDKMediaPlayerImpl {
public:
    RDKMediaPlayerImpl(RDKMediaPlayer* parent);
    virtual ~RDKMediaPlayerImpl();
    virtual bool doCanPlayURL(const std::string& url) = 0;
    virtual void doInit() = 0;
    virtual void doLoad(const std::string& url) = 0;
    virtual void doSetVideoRectangle(const IntRect& rect) = 0;
    virtual void doSetAudioLanguage(std::string& lang) = 0;
    virtual void doPlay() = 0;
    virtual void doPause() = 0;
    virtual void doSetPosition(float position) = 0;
    virtual void doSeekToLive() = 0;
    virtual void doStop() = 0;
    virtual void doChangeSpeed(float speed, int32_t overshootTime) = 0;
    virtual void doSetSpeed(float speed) = 0;
    virtual void doSetBlocked(bool blocked) = 0;
    virtual void doSetEISSFilterStatus(bool status) = 0;
    virtual void doSetVolume(float volume) = 0;
    virtual void doSetIsInProgressRecording(bool isInProgressRecording) = 0;
    virtual void doSetZoom(int zoom) = 0;
    virtual void doSetNetworkBufferSize(int32_t networkBufferSize) = 0;
    virtual void doSetVideoBufferLength(float videoBufferLength) = 0;
    virtual void getProgressData(ProgressData* progressData) = 0;
    RDKMediaPlayer* getParent()
    {
        return m_parent;
    }
    TuneState getTuneState()
    {
        return m_tuneState;
    }
    void setTuneState(TuneState state)
    {
        m_tuneState = state;
    }
protected:
private:
    RDKMediaPlayer* m_parent;
    TuneState m_tuneState;
};

//Events

struct OnMediaOpenedEvent: public Event
{
    OnMediaOpenedEvent(const char* mediaType, int duration, int width, int height, const char* availableSpeeds, const char* availableAudioLanguages, const char* availableClosedCaptionsLanguages) : Event("onMediaOpened")
    {
        m_object.set("mediaType", mediaType);
        m_object.set("duration", duration);
        m_object.set("width", width);
        m_object.set("height", height);
        m_object.set("availableSpeeds", availableSpeeds);
        m_object.set("availableAudioLanguages", availableAudioLanguages);
        m_object.set("availableClosedCaptionsLanguages", availableClosedCaptionsLanguages);
        //TODO handle the following 2
        m_object.set("customProperties", "");
        m_object.set("mediaSegments", "");
    }
};

struct OnStatusEvent: public Event
{
    // Special case to ensure that caller receives the actual time
    struct rtOnStatus: public rtMapObject
    {
        rtOnStatus(RDKMediaPlayerImpl* player) : m_player(player)
        {
            rtValue nameVal = "onStatus";
            rtValue val = -1.0f;
            Set("name", &nameVal);
            Set("position", &val);
            Set("duration", &val);
            //TODO handle the following 8
            Set("width", &val);
            Set("height", &val);
            Set("bufferPercentage", &val);
            Set("connectionURL", &val);
            Set("dynamicProps", &val);
            Set("isBuffering", &val);
            Set("isLive", &val);
            Set("netStreamInfo", &val);
        }
        rtError Get(const char* name, rtValue* value) const override
        {
            if (!value)
                return RT_FAIL;
            ProgressData pd;
            m_player->getProgressData(&pd);
            if (!strcmp(name, "position")) { *value = pd.position; return RT_OK; }
            if (!strcmp(name, "duration")) { *value = pd.duration; return RT_OK; }
            return rtMapObject::Get(name, value);
        }
        RDKMediaPlayerImpl* m_player;
    };
    OnStatusEvent(RDKMediaPlayerImpl* player) : Event(new rtOnStatus(player)) { }
};

struct OnProgressEvent: public Event
{
    // Special case to ensure that caller receives the actual time
    struct rtOnProgress: public rtMapObject
    {
        rtOnProgress(RDKMediaPlayerImpl* player) : m_player(player)
        {
            rtValue nameVal = "onProgress";
            rtValue val = -1.0f;
            Set("name", &nameVal);
            Set("position", &val);
            Set("duration", &val);
            Set("speed", &val);
            Set("start", &val);
            Set("end", &val);
        }
        rtError Get(const char* name, rtValue* value) const override
        {
            if (!value)
                return RT_FAIL;
            ProgressData pd;
            m_player->getProgressData(&pd);
            if (!strcmp(name, "position")) { *value = pd.position; return RT_OK; }
            if (!strcmp(name, "duration")) { *value = pd.duration; return RT_OK; }
            if (!strcmp(name, "speed")) { *value = pd.speed; return RT_OK; }
            if (!strcmp(name, "start")) { *value = pd.start; return RT_OK; }
            if (!strcmp(name, "end")) { *value = pd.end; return RT_OK; }
            return rtMapObject::Get(name, value);
        }
        RDKMediaPlayerImpl* m_player;
    };
    OnProgressEvent(RDKMediaPlayerImpl* player) : Event(new rtOnProgress(player)) { }
};

struct OnClosedEvent: public Event { OnClosedEvent() : Event("onClosed") { } };
struct OnPlayerInitializedEvent: public Event { OnPlayerInitializedEvent() : Event("onPlayerInitialized") { } };
struct OnBufferingEvent: public Event { OnBufferingEvent() : Event("onBuffering") { } };
struct OnPlayingEvent: public Event { OnPlayingEvent() : Event("onPlaying") { } };
struct OnPausedEvent: public Event { OnPausedEvent() : Event("onPaused") { } };
struct OnCompleteEvent: public Event { OnCompleteEvent() : Event("onComplete") { } };
struct OnIndividualizingEvent: public Event { OnIndividualizingEvent() : Event("onIndividualizing") { } };
struct OnAcquiringLicenseEvent: public Event { OnAcquiringLicenseEvent() : Event("onAcquiringLicense") { } };

struct OnWarningEvent: public Event
{
    OnWarningEvent(uint32_t code, const std::string& description) : Event("onWarning")
    {
        m_object.set("code", code);
        m_object.set("description", rtString(description.c_str()));
    }
};

struct OnErrorEvent: public Event
{
    OnErrorEvent(uint32_t code, const std::string& description) : Event("onError")
    {
        m_object.set("code", code);
        m_object.set("description", rtString(description.c_str()));
    }
};

struct OnSpeedChangeEvent: public Event
{
    OnSpeedChangeEvent(double speed) : Event("onSpeedChange")
    {
        m_object.set("speed", speed);
    }
};

struct OnDRMMetadata: public Event
{
    OnDRMMetadata(rtObjectRef const& props) : Event("onDRMMetadata")
    {
        m_object.set("props", props);
    }
};

struct OnSegmentStartedEvent: public Event
{
    OnSegmentStartedEvent(const std::string& segmentType, double duration, const std::string& segmentId, rtObjectRef const& segment) : Event("onSegmentStarted")
    {
        m_object.set("segmentType", segmentType.c_str());
        m_object.set("duration", duration);
        m_object.set("segmentId", segmentId.c_str());
        m_object.set("segment", segment);
    }
};

struct OnSegmentCompletedEvent: public Event
{
    OnSegmentCompletedEvent(const std::string& segmentType, double duration, const std::string& segmentId, rtObjectRef const& segment) : Event("onSegmentCompleted")
    {
        m_object.set("segmentType", segmentType.c_str());
        m_object.set("duration", duration);
        m_object.set("segmentId", segmentId.c_str());
        m_object.set("segment", segment);
    }
};

struct OnSegmentWatchedEvent: public Event
{
    OnSegmentWatchedEvent(const std::string& segmentType, double duration, const std::string& segmentId, rtObjectRef const& segment) : Event("onSegmentWatched")
    {
        m_object.set("segmentType", segmentType.c_str());
        m_object.set("duration", duration);
        m_object.set("segmentId", segmentId.c_str());
        m_object.set("segment", segment);
    }
};

struct OnBufferWarningEvent: public Event
{
    OnBufferWarningEvent(const std::string& warningType, double bufferSize, double bufferFillSize) : Event("onBufferWarning")
    {
        m_object.set("warningType", warningType.c_str());
        m_object.set("bufferSize", bufferSize);
        m_object.set("bufferFillSize", bufferFillSize);
    }
};

struct OnPlaybackSpeedsChangedEvent: public Event
{
    OnPlaybackSpeedsChangedEvent(const std::string& availableSpeeds) : Event("onPlaybackSpeedsChanged")
    {
        m_object.set("availableSpeeds", availableSpeeds.c_str());
    }
};

struct OnAdditionalAuthRequiredEvent: public Event
{
    OnAdditionalAuthRequiredEvent(const std::string& locator, const std::string& eventId) : Event("onAdditionalAuthRequired")
    {
        m_object.set("locator", locator.c_str());
        m_object.set("eventId", eventId.c_str());
    }
};

/////////////////////
// OnEISSDataReceivedEvent and OnMetricLogEvent aren't a part of RDK player spec, but i'm leaving them for now because rmdmediaplayer.cpp depends on them.
//You could probably parse the EISS data in side here and just sent OnMetricLogs for any errors
struct OnEISSDataReceivedEvent: public Event
{
    OnEISSDataReceivedEvent(const std::string& eissData) : Event("onEISSDataReceived")
    {
        m_object.set("rmfEISSDataBuffer", rtString(eissData.c_str()));
    }
};


struct OnMetricLogEvent: public Event
{
    OnMetricLogEvent(const char* message) : Event("onMetricLog")
    {
        m_object.set("message", message);
    }
};

struct OnBitrateChanged: public Event
{
    OnBitrateChanged(int bitrate, const char* reason) : Event("onBitrateChanged")
    {
        m_object.set("bitrate", bitrate);
        m_object.set("reason", reason);
    }
};

#endif
