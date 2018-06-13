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
#ifndef AAMPMEDIAPLAYER_H
#define AAMPMEDIAPLAYER_H

#include "rdkmediaplayerimpl.h"

class RDKMediaPlayer;
class AAMPListener;
class AAMPEvent;
class PlayerInstanceAAMP;

class AAMPPlayer : public RDKMediaPlayerImpl
{
public:
    AAMPPlayer(RDKMediaPlayer* parent);
    ~AAMPPlayer();
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
private:
    static bool setContentType(const std::string &uri, std::string& contentType);
    void onProgress(const AAMPEvent& progressEvent);
    PlayerInstanceAAMP* m_aampInstance;
    AAMPListener* m_aampListener;
    ProgressData m_progressData;
    friend class AAMPListener;
};

#endif
