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
#include "aampplayer.h"
#include "rdkmediaplayer.h"
#include <glib.h>
#include <limits>
#include <cmath>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "libIBus.h"
#include "intrect.h"
#include "logger.h"
#include "main_aamp.h"
#ifndef DISABLE_CLOSEDCAPTIONS
#include "host.hpp"
#include "videoOutputPort.hpp"
#include "manager.hpp"
#endif
extern "C"
{
    void setComcastSessionToken(const char* token);
	void loadAVEJavaScriptBindings(void* context);
}

void* GetPlatformCallbackManager()
{
    return 0;
}

void ClearClosedCaptionSurface()
{
}


void HideClosedCaptionSurface()
{
}

void ShowClosedCaptionSurface()
{
}

void* CreateSurface()
{
     return 0;
}

void DestroySurface(void* surface)
{
}

void GetSurfaceScale(double *pScaleX, double *pScaleY)
{
}

void SetSurfaceSize(void* surface, int width, int height)
{
}

void SetSurfacePos(void* surface, int x, int y)
{
}

#define CALL_ON_MAIN_THREAD(body) \
    do { \
        g_timeout_add_full( \
            G_PRIORITY_HIGH, \
            0, \
            [](gpointer data) -> gboolean \
            { \
                AAMPListener &self = *static_cast<AAMPListener*>(data); \
                body \
                return G_SOURCE_REMOVE; \
            }, \
            this, \
            0); \
    } while(0)

class AAMPListener : public AAMPEventListener
{
public:
    AAMPListener(AAMPPlayer* player, PlayerInstanceAAMP* aamp) : m_player(player), m_aamp(aamp)
    {
        reset();
    }
    void reset()
    {
        m_eventTuned = m_eventPlaying = m_didProgress = m_tuned = false;
        m_lastPosition = std::numeric_limits<double>::max();
        m_duration = 0;
        m_tuneStartTime = g_get_monotonic_time();
    }
    //first progress event indicates that we tuned, so don't fire any progress events before that
    void checkIsTuned()
    {
        if(!m_tuned && (m_eventTuned && m_eventPlaying && m_didProgress))
        {
            LOG_WARNING("AAMP TUNETIME %.2fs\n", (float)(g_get_monotonic_time() - m_tuneStartTime)/1000000.0f);
            m_tuned = true;
            AAMPEvent progress;
            progress.data.progress.positionMiliseconds = 0;
            progress.data.progress.durationMiliseconds = m_duration;
            progress.data.progress.playbackSpeed = 1;
            progress.data.progress.startMiliseconds = 0;
            progress.data.progress.endMiliseconds = m_duration;
            m_player->onProgress(progress);
        }
    }
    void Event(const AAMPEvent & e)
    {
        switch (e.type)
        {
        case AAMP_EVENT_TUNED:
            LOG_INFO("event tuned");
            m_eventTuned = true;
            checkIsTuned();
            break;
        case AAMP_EVENT_TUNE_FAILED:
            LOG_INFO("tune failed. code:%d desc:%s failure:%d", e.data.mediaError.code, e.data.mediaError.description, e.data.mediaError.failure);
            m_player->getParent()->getEventEmitter().send(OnErrorEvent(e.data.mediaError.code, e.data.mediaError.description));
            break;
        case AAMP_EVENT_SPEED_CHANGED:
            m_player->getParent()->getEventEmitter().send(OnSpeedChangeEvent(e.data.speedChanged.rate));
            break;
        case AAMP_EVENT_EOS:
            m_player->getParent()->getEventEmitter().send(OnCompleteEvent());
            m_player->getParent()->stop();//automatically call stop on player
            break;
        case AAMP_EVENT_PLAYLIST_INDEXED:
            break;
        case AAMP_EVENT_PROGRESS:
            //LOG_WARNING("progress %g", e.data.progress.positionMiliseconds);
            if(m_tuned)//don't send any progress event until tuned (first progress from checkIsTuned signals tune complete)
            {
                m_player->onProgress(e);
            }
            else if(m_eventTuned)//monitor for change in progress position (ignoring any progress events coming before the tuned event)
            {
                //if(m_lastPosition != std::numeric_limits<double>::max())
                {
                    //if(e.data.progress.positionMiliseconds > m_lastPosition) 
                    {
                        LOG_INFO("event progress");
                        m_didProgress = true;
                        checkIsTuned();
                    }
                }
                //m_lastPosition = e.data.progress.positionMiliseconds;
            }
            break;
        case AAMP_EVENT_CC_HANDLE_RECEIVED:
            m_player->getParent()->onVidHandleReceived(e.data.ccHandle.handle);
            break;
        case AAMP_EVENT_JS_EVENT:
            break;
        case AAMP_EVENT_VIDEO_METADATA:
        {
            std::string lang;
            for(int i = 0; i < e.data.metadata.languageCount; ++i)
            {
                if(i > 0)
                    lang.append(",");
                lang.append(e.data.metadata.languages[i]);
            }
            m_player->getParent()->updateVideoMetadata(lang, "-64,-32,-16,-4,-1,0,1,4,16,32,64", e.data.metadata.durationMiliseconds, e.data.metadata.width, e.data.metadata.height);
            m_duration = e.data.metadata.durationMiliseconds;
            break;
        }
        case AAMP_EVENT_ENTERING_LIVE:
            break;
        case AAMP_EVENT_BITRATE_CHANGED:
            m_player->getParent()->getEventEmitter().send(OnBitrateChanged(e.data.bitrateChanged.bitrate, e.data.bitrateChanged.description));
            break;
        case AAMP_EVENT_TIMED_METADATA:
            break;
        case AAMP_EVENT_STATUS_CHANGED:
            switch(e.data.stateChanged.state)
            {
	            case eSTATE_IDLE: break;
	            case eSTATE_INITIALIZING: break;
	            case eSTATE_INITIALIZED: break;
	            case eSTATE_PREPARING: break;
	            case eSTATE_PREPARED: break;
	            case eSTATE_BUFFERING: break;
	            case eSTATE_PAUSED: 
                    m_player->getParent()->getEventEmitter().send(OnPausedEvent());
                    break;
	            case eSTATE_SEEKING: break;
	            case eSTATE_PLAYING: 
                    if(!m_tuned)
                    {
                        LOG_INFO("event playing");
                        m_eventPlaying = true;
                        checkIsTuned();
                    }
                    m_player->getParent()->getEventEmitter().send(OnPlayingEvent());
                    break;
	            case eSTATE_STOPPING: break;
	            case eSTATE_STOPPED: break;
	            case eSTATE_COMPLETE: break;
	            case eSTATE_ERROR: break;
	            case eSTATE_RELEASED: break;
            }
            break;
        default:
            break;
        }
    }
private:
    AAMPPlayer* m_player;
    PlayerInstanceAAMP* m_aamp;
    bool m_eventTuned, m_eventPlaying, m_didProgress, m_tuned;
    int m_duration;
    double m_lastPosition;
    gint64 m_tuneStartTime;
};

bool AAMPPlayer::canPlayURL(const std::string& url)
{
    std::string contentType;
    return AAMPPlayer::setContentType(url, contentType);
}

AAMPPlayer::AAMPPlayer(RDKMediaPlayer* parent) : 
    RDKMediaPlayerImpl(parent),
    m_aampInstance(0),
    m_aampListener(0)
{
    LOG_INFO("AAMPPlayer ctor");
}

AAMPPlayer::~AAMPPlayer()
{
    if(m_aampListener)
        delete m_aampListener;
    if(m_aampInstance)
        delete m_aampInstance;
}

// RDKMediaPlayerImpl impl start
bool AAMPPlayer::doCanPlayURL(const std::string& url)
{
    return AAMPPlayer::canPlayURL(url);
}

void AAMPPlayer::doInit()
{
  LOG_INFO("AAMPPlayer started");
  m_aampInstance = new PlayerInstanceAAMP();
  m_aampListener = new AAMPListener(this,  m_aampInstance);
  m_aampInstance->RegisterEvents(m_aampListener);
  IARM_Bus_Init("AAMPPlayer");
  IARM_Bus_Connect();
#ifndef DISABLE_CLOSEDCAPTIONS
  device::Manager::Initialize();
#endif
}

void AAMPPlayer::doLoad(const std::string& url)
{
    //setSessionToken();
    if(m_aampListener)
    {
        m_aampListener->reset();
    }
    if(m_aampInstance)
    {
        m_aampInstance->Tune(url.c_str());
    }
}

void AAMPPlayer::doSetVideoRectangle(const IntRect& rect)
{
    if (rect.width() > 0 && rect.height() > 0)
    {
        LOG_INFO("set video rectangle: %dx%d %dx%d", rect.x(), rect.y(), rect.width(), rect.height());
        m_aampInstance->SetVideoRectangle(rect.x(), rect.y(), rect.width(), rect.height());
    }
}

void AAMPPlayer::doSetAudioLanguage(std::string& lang)
{
  LOG_INFO("set lang: %s", lang.c_str());
  m_aampInstance->SetLanguage(lang.c_str());
}

void AAMPPlayer::doPlay()
{
    LOG_INFO("AAMPPlayer::doPlay()");
	if(m_aampInstance)
		m_aampInstance->SetRate(1);
}

void AAMPPlayer::doPause()
{
    LOG_INFO("AAMPPlayer::doPause()");
	if(m_aampInstance)
		m_aampInstance->SetRate(0);
}

void AAMPPlayer::doSetPosition(float position)
{
    if (!std::isnan(position))
    {
        LOG_INFO("set currentTime: %f", position);
        if(m_aampInstance)
            m_aampInstance->Seek(position/1000);
    }
}

void AAMPPlayer::doSeekToLive()
{
	LOG_INFO("seekToLive()");
	if(m_aampInstance)
		m_aampInstance->SeekToLive();
}


void AAMPPlayer::doStop()
{
    LOG_INFO("stop()");
	if(m_aampInstance)
    {
		m_aampInstance->Stop();    
        getParent()->getEventEmitter().send(OnClosedEvent());//TODO OnClosedEvent needs to be tied into aamps AAMP_EVENT_STATUS_CHANGED state correctly (e.g. eSTATE_RELEASED once aamp sends it)
    }
}

void AAMPPlayer::doChangeSpeed(float speed, int32_t overshootTime)
{
    LOG_INFO("doChangeSpeed(%f, %d)", speed, overshootTime);
    if(m_aampInstance)
    	m_aampInstance->SetRate(speed, overshootTime);
}

void AAMPPlayer::doSetSpeed(float speed)
{
    LOG_INFO("doSetSpeed(%f)", speed);
    if(m_aampInstance)
    	m_aampInstance->SetRate(speed);
}

void AAMPPlayer::doSetBlocked(bool blocked)
{
    LOG_INFO("doSetBlocked()");
    if(m_aampInstance)
        m_aampInstance->SetAudioVolume(blocked ? 0 : 100);
}

void AAMPPlayer::doSetVolume(float volume)
{
    LOG_INFO("set volume: %f", volume);
    if(m_aampInstance)
        m_aampInstance->SetAudioVolume(volume);
}

void AAMPPlayer::doSetZoom(int zoom)
{
    LOG_INFO("doSetVideoZoom()");
    if(m_aampInstance)
        m_aampInstance->SetVideoZoom((VideoZoomMode)zoom);
}

void AAMPPlayer::doSetNetworkBufferSize(int32_t networkBufferSize)
{
    LOG_INFO("doSetNetworkBufferSize()");
}

void AAMPPlayer::doSetVideoBufferLength(float videoBufferLength)
{
    LOG_INFO("doSetVideoBufferLength()");
}

void AAMPPlayer::doSetEISSFilterStatus(bool status)
{
    LOG_INFO("doSetEISSFilterStatus()");
}

void AAMPPlayer::doSetIsInProgressRecording(bool isInProgressRecording)
{
    LOG_INFO("doSetIsInProgressRecording()");
}

void AAMPPlayer::getProgressData(ProgressData* progressData)
{
    *progressData = m_progressData;
}

// RDKMediaPlayerImpl impl end

void AAMPPlayer::onProgress(const AAMPEvent & e)
{
    m_progressData.position = e.data.progress.positionMiliseconds;
    m_progressData.duration = e.data.progress.durationMiliseconds;
    m_progressData.speed = e.data.progress.playbackSpeed;
    m_progressData.start = e.data.progress.startMiliseconds;
    m_progressData.end = e.data.progress.endMiliseconds;
    getParent()->getEventEmitter().send(OnProgressEvent(this));
}

bool AAMPPlayer::setContentType(const std::string &url, std::string& contentStr)
{
    LOG_INFO("enter %s", url.c_str());
    int contentId = 0;
    
    if(url.find(".m3u8") != std::string::npos)
    {
        if(url.find("cdvr") != std::string::npos)
        {
            contentId = 1;
            contentStr = "cdvr";
        }
        else if(url.find("col") != std::string::npos)
        {
            contentId = 2;
            contentStr = "vod";//and help
        }
        else if(url.find("linear") != std::string::npos)
        {
            contentId = 3;
            contentStr = "ip-linear";
        }
        else if(url.find("ivod") != std::string::npos)
        {
            contentId = 4;
            contentStr = "ivod";
        }
        else if(url.find("ip-eas") != std::string::npos)
        {
            contentId = 5;
            contentStr = "ip-eas";
        }
        else if(url.find("xfinityhome") != std::string::npos)
        {
            contentId = 6;
            contentStr = "camera";
        }
        else if(url.find(".mpd"))
        {
            contentId = 7;
            contentStr = "helio";
        }
    }

    LOG_INFO("returning %d", contentId);

    if(contentId)
    {
        return true;
    }
    else
    {
        contentStr = "unknown";
        return false;
    }
}

/*
void setSessionToken()
{
	char *contents = NULL;
	FILE *fp = fopen("/opt/AAMPPlayer_session_token.txt", "r");
	if (fp != NULL)
	{
		if (fseek(fp, 0L, SEEK_END) == 0)
        {
		    long bufsize = ftell(fp);
		    if (bufsize == -1){}
		    contents = new char[bufsize + 1];
		    if (fseek(fp, 0L, SEEK_SET) != 0)
            {}
		    size_t newLen = fread(contents, sizeof(char), bufsize, fp);
		    if ( ferror( fp ) != 0 )
            {
		    }
            else
            {
		        contents[newLen-2] = '\0';
		    }
		}
		fclose(fp);

		if(contents)
		{
			char* token = strstr(contents, "token");
			if(token)
			{
				token += 8;
				char* tokenString = new char[100000];
                int t = time(NULL);
				srand(t);
                short r = rand() & 0xffff;
			
				char temp[100];
				char msgid[20];
				sprintf(temp, "%08X%04X", t, r);
				int i2 = 0;
				for(int i = 0 ; i < 12; ++i)
				{
					if(i==3 || i == 7 || i == 11)
					{
						msgid[i2++] = '-';
					}
					msgid[i2++] = temp[i];
				}
				msgid[15] = 0;

                char bytes[20];
                for (int i=0; i<20; i++)
                    bytes[i] = rand() & 0xff;
        //        QString nonce = b.toBase64();
				sprintf(tokenString, 
				"{\"message:id\":\"15E-6D8-269-5D7\",\"message:type\":\"clientAccess\",\"message:nonce\":\"dmYcSpnudef2U00wYvUxblWsMIk=\",\"client:accessToken\":\"%s\",\"client:mediaUsage\":\"stream\"}", token);
				LOG_INFO("sending session token");
				setComcastSessionToken(tokenString);
				delete tokenString;
			}
			else
			{
				LOG_INFO("no token /opt/AAMPPlayer_session_token.txt");
			}
			delete contents;
		}
		else
		{
			LOG_INFO("empty /opt/AAMPPlayer_session_token.txt");
		}

	}
	else
	{
		LOG_INFO("can't open /opt/AAMPPlayer_session_token.txt");
	}

}
*/
