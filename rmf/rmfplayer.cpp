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
#ifdef USE_EXTERNAL_CAS
#include <jansson.h>
#endif

#define TO_MS(x) (uint32_t)( (x) / 1000)
#define TUNE_LOG_MAX_SIZE 100
#define HEADER_SIZE 3

namespace
{
    const guint kProgressMonitorTimeoutMs = 250;
    const guint kRefreshCachedTimeThrottleMs = 100;
}

#ifdef USE_EXTERNAL_CAS
bool CASService::initialize(bool management, const std::string& casOcdmId, PSIInfo *psiInfo)
{
    LOG_INFO("CASService initialize");
    casOcdmId_ = casOcdmId;
    casManager_ = CASManager::createInstance(casSFInterface_, casPipelineInterface_);

    if(!casManager_)
    {
        LOG_INFO("Failed to create casManager");
        return false;
    }

    bManagementSession_ = management;
    if(env_.getUsageManagement() == CAS_USAGE_MANAGEMENT_FULL) {
        bManagementSession_ = true;
    }
    else {
        bManagementSession_ = management;
    }
    LOG_INFO("Management = %d", bManagementSession_);

    if(bManagementSession_ && (psiInfo == NULL)) {
        casHelper_ = casManager_->createHelper(casOcdmId, env_);
    }
    else {
        casHelper_ = casManager_->createHelper(psiInfo->pat, psiInfo->pmt, psiInfo->cat, env_);
    }
    if(!casHelper_)
    {
        LOG_INFO("Failed to create Helper");
        return false;
    }
    return true;
}

bool CASService::startStopDecrypt(const std::vector<uint16_t>& startPids, const std::vector<uint16_t>& stopPids)
{
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
    if(!casHelper_)
    {
        LOG_INFO("casHelper_ Not available");
        return false;
    }

    casHelper_->registerStatusInformant(this);

    CASHelper::CASStartStatus status = casHelper_->startStopDecrypt(startPids, stopPids);
    bool ret = false;
    switch(status)
    {
        case CASHelper::CASStartStatus::OK:
            LOG_INFO("Everything was created and CAS Started, there is no need to wait for status");
            ret = true;
            break;
        case CASHelper::CASStartStatus::WAIT:
            LOG_INFO("verything was created but the CAS has not started yet...");
            break;
        case CASHelper::CASStartStatus::ERROR:
            LOG_INFO("CAS Start is Failed...");
            break;
        default:
            LOG_INFO("Invalid CAS start status");
            break;
    }
    return ret;
}

void CASService::updatePSI(const std::vector<uint8_t>& pat, const std::vector<uint8_t>& pmt, const std::vector<uint8_t>& cat)
{
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
        if(casHelper_)
        {
            casHelper_->updatePSI(pat, pmt, cat);
        }
        else
        {
            LOG_ERROR("[%s:%d] Failed to send CASService updatePSI \n", __FUNCTION__, __LINE__);
        }
}

void CASService::informStatus(const CASStatus& status) {
    LOG_INFO("[%s:%d] Inform Status back to Source = %d\n", __FUNCTION__, __LINE__, status);
    if(status == CASStatusInform::CASStatus_OK)
    {
       casPipelineInterface_->unmuteAudio();
    }
}

void CASService::casPublicData(const std::vector<uint8_t>& data)
{
    LOG_INFO("casPublicData - Received 0x%x",this);
    std::string casData;
    casData.assign(data.begin(), data.end());
    rtValue  sessionId((std::to_string((uint32_t)mpImpl_)).c_str());
    LOG_INFO("casPublicData - %s\n", casData.c_str());
    emit_.send(OnCASDataEvent(casData, sessionId));
}

void CASService::processSectionData(const uint32_t& filterId, const std::vector<uint8_t>& data)
{
    LOG_INFO("processSectionData - CAS Service");
    if(casManager_)
    {
        casManager_->processData(filterId, data);
    }
    else
    {
    LOG_ERROR("processSectionData - CASManager NULL");
    }
}

bool CASService::isManagementSession() const
{
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
    return bManagementSession_;
}

ICasFilterStatus RMFPlayer::create(uint16_t pid, ICasFilterParam &param, ICasHandle **pHandle) {
    LOG_INFO("create - CAS Upstream to Create Filter");
    uint32_t filterId = 0;

    ICasFilterParam *filterParam = (ICasFilterParam *)malloc(sizeof(ICasFilterParam));
    filterParam->pos_size = param.pos_size;
    filterParam->neg_size = param.neg_size;
    for (int i = 0; i < FILTER_SIZE; i++)
    {
        filterParam->pos_mask[i] = param.pos_mask[i];
        filterParam->pos_value[i] = param.pos_value[i];
        filterParam->neg_mask[i] = param.neg_mask[i];
        filterParam->neg_value[i] = param.neg_value[i];
    }
    filterParam->disableCRC = param.disableCRC;
    filterParam->noPaddingBytes = param.noPaddingBytes;
    filterParam->mode = param.mode;
    LOG_INFO("create : param - pos_size = %d, neg_size = %d\n", param.pos_size, param.neg_size);
    m_mediaPlayer->setFilter(pid, (char *)filterParam, &filterId);
    (*pHandle)->filterId = filterId;
    if(filterParam)
    {
        free(filterParam);
        filterParam = NULL;
    }
    return ICasFilterStatus_NoError;
}

ICasFilterStatus RMFPlayer::setState(bool isRunning, ICasHandle* handle) {
    LOG_INFO("setState - CAS Upstream to Create Filter");
    uint32_t filterId = handle->filterId;
    //pause filter
    //stop cancel
    if(!isRunning)
    {
       m_mediaPlayer->pauseFilter(filterId);
    }

    return ICasFilterStatus_NoError;
}

ICasFilterStatus RMFPlayer::start(ICasHandle* handle) {
    LOG_INFO("start - CAS Upstream to Create Filter");
    uint32_t filterId = handle->filterId;
    //resume filter
    m_mediaPlayer->resumeFilter(filterId);
    return ICasFilterStatus_NoError;
}

ICasFilterStatus RMFPlayer::destroy(ICasHandle* handle) {
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
    LOG_INFO("destroy - CAS Upstream to Create Filter");
    uint32_t filterId = handle->filterId;
    m_mediaPlayer->releaseFilter(filterId);
    return ICasFilterStatus_NoError;
}

void RMFPlayer::unmuteAudio()
{
#ifdef USE_EXTERNAL_CAS
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
    if(m_mediaPlayer->getAudioMute())
    {
        LOG_INFO("setAudioMute - false\n");
        m_mediaPlayer->rmf_setAudioMute(false);
    }
#endif
}

uint32_t RMFPlayer::setKeySlot(uint16_t pid, std::vector<uint8_t> data){
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
    uint32_t ret = RMF_RESULT_SUCCESS;
    //For External CAS, the data sent is having a pointer address
    //This needs to be made it is more generic for different platform and CAS
    //E.g.: [232, 180, 153, 192], keyslot handle should be 0xE8B499C0
    char* keyHandle = (char*)((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
    RDK_LOG(RDK_LOG_INFO, "LOG.RDK.ANYCAS", "[%s:%d] keyHandle = 0x%x\n", __FUNCTION__, __LINE__, keyHandle);
    if(pid == m_mediaPlayer->rmf_getVideoPid())
    {
        LOG_INFO("setKeySlot - Setting Video Key Slot, pid = %d", pid);
        m_mediaPlayer->rmf_setVideoKeySlot(keyHandle);
    }
    else if (pid == m_mediaPlayer->rmf_getAudioPid())
    {
        LOG_INFO("setKeySlot - Setting Audio Key Slot, pid = %d", pid);
        m_mediaPlayer->rmf_setAudioKeySlot(keyHandle);
    }
    else
    {
        LOG_INFO("setKeySlot - Invalid Pid - Not either Audio/Video Pid currently playing");
        ret = RMF_RESULT_FAILURE;
    }
    return ret;
}

uint32_t RMFPlayer::deleteKeySlot(uint16_t pid){
    uint32_t ret = RMF_RESULT_SUCCESS;
    RDK_LOG(RDK_LOG_INFO, "LOG.RDK.ANYCAS", "[%s:%d] pid = 0x%x\n", __FUNCTION__, __LINE__, pid);

    if(pid == m_mediaPlayer->rmf_getVideoPid())
    {
        LOG_INFO("deleteKeySlot - deleting Video Key Slot, pid = %d", pid);
	m_mediaPlayer->rmf_deleteVideoKeySlot();
    }
    else if (pid == m_mediaPlayer->rmf_getAudioPid())
    {
        LOG_INFO("deleteKeySlot - deleting Audio Key Slot, pid = %d", pid);
        m_mediaPlayer->rmf_deleteAudioKeySlot();
    }
    else
    {
        LOG_INFO("deleteKeySlot - Invalid Pid - Not either Audio/Video Pid currently playing");
        ret = RMF_RESULT_FAILURE;
    }
    return ret;
}

uint32_t RMFPlayer::getPATBuffer(std::vector<uint8_t>& patBuf) {
    return m_mediaPlayer->getPATBuffer(patBuf);
}

uint32_t RMFPlayer::getPMTBuffer(std::vector<uint8_t>& pmtBuf) {
    return m_mediaPlayer->getPMTBuffer(pmtBuf);
}

uint32_t RMFPlayer::getCATBuffer(std::vector<uint8_t>& catBuf) {
    return m_mediaPlayer->getCATBuffer(catBuf);
}
#endif

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
#ifdef USE_EXTERNAL_CAS
    m_lastRefreshTime(0),
    m_casService(NULL),
    m_lastAudioPid(-1),
    m_lastVideoPid(-1),
#endif
    m_playbackProgressMonitorTag(0)
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
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
    m_playerState = MediaPlayer::RMF_PLAYER_LOADING;
    m_videoState = MediaPlayer::RMF_VIDEO_BUFFER_HAVENOTHING;
    m_videoStateMaximum = MediaPlayer::RMF_VIDEO_BUFFER_HAVENOTHING;
    m_lastReportedCurrentTime = 0.f;
    m_lastReportedDuration = 0.f;
    m_lastReportedPlaybackRate = 0.f;
    m_isPaused = true;
#ifdef USE_EXTERNAL_CAS
    m_lastAudioPid = -1;
    m_lastVideoPid = -1;
#endif
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

bool RMFPlayer::isManagementSession() const
{
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
#ifdef USE_EXTERNAL_CAS
    std::lock_guard<std::mutex> guard(cas_mutex);
    if(m_casService)
    {
        return m_casService->isManagementSession();
    }
    else
    {
        return false;
    }
#else
    return false;
#endif
}

#ifdef USE_EXTERNAL_CAS
void RMFPlayer::open(const std::string& openData)
{
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);

    json_error_t error;
    json_t *root = json_loads(openData.c_str(), 0, &error);
    if (!root) {
        LOG_INFO("json error on line %d: %s\n", error.line, error.text);
        return;
    }

    std::string mediaUrl = json_string_value(json_object_get(root, "mediaurl"));
    std::string mode = json_string_value(json_object_get(root, "mode"));
    std::string manage = json_string_value(json_object_get(root, "manage"));
    std::string initData = json_string_value(json_object_get(root, "casinitdata"));
    std::string casOcdmId = json_string_value(json_object_get(root, "casocdmid"));

    CASUsageMode mode_val;
    CASUsageManagement manage_val;
    if(mode == "MODE_NONE") { mode_val = CAS_USAGE_NULL; }
    else if(mode == "MODE_LIVE") { mode_val = CAS_USAGE_LIVE; }
    else if(mode == "MODE_RECORD") { mode_val = CAS_USAGE_RECORDING; }
    else if(mode == "MODE_PLAYBACK") { mode_val = CAS_USAGE_PLAYBACK; }

    if(manage == "MANAGE_NONE") { manage_val = CAS_USAGE_MANAGEMENT_NONE; }
    else if(manage == "MANAGE_FULL") { manage_val = CAS_USAGE_MANAGEMENT_FULL; }
    else if(manage == "MANAGE_NO_PSI") { manage_val = CAS_USAGE_MANAGEMENT_NO_PSI; }
    else if(manage == "MANAGE_NO_TUNER") { manage_val = CAS_USAGE_MANAGEMENT_NO_TUNE; }

    CASEnvironment env{ mediaUrl, mode_val, manage_val, initData};

    std::lock_guard<std::mutex> guard(cas_mutex);
    m_casService = new CASService(getParent()->getEventEmitter(), env, this, this, this);
    if(m_casService) {
        LOG_INFO("Successfully created m_casService");
    }
    else {
        LOG_INFO("Failed to create m_casService");
    }

    if(casOcdmId.empty())
    {
        LOG_INFO("Missing casocdmid which is mandatory to create management session.");
    }
    else if(manage == "MANAGE_NO_PSI" || manage == "MANAGE_NO_TUNER")
    {
        if(m_casService && m_casService->initialize(true, casOcdmId, NULL) == false)
        {
            delete m_casService;
            m_casService = NULL;
            LOG_INFO("Failed to create Management Session");
        }
    }
    else
    {
        LOG_INFO("Invalid Manage option: %s", manage.c_str());
    }

    json_decref(root);
}

void RMFPlayer::registerCASData()
{
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
    std::lock_guard<std::mutex> guard(cas_mutex);
    if(m_casService) {
        std::shared_ptr<CASHelper> casHelper = m_casService->getCasHelper();
        if(casHelper){
            casHelper->registerDataListener(m_casService);
        }
    }
    else{
        LOG_INFO("No Management Session - Invalid Operation");
    }
}

void RMFPlayer::sendCASData(const std::string& data)
{
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
    std::lock_guard<std::mutex> guard(cas_mutex);
    if(m_casService)
    {
        json_error_t error;
        json_t *root = json_loads(data.c_str(), 0, &error);

        if (root) {
            std::string strPayload = json_string_value(json_object_get(root, "payload"));

            std::vector<uint8_t> payload;
            payload.assign(strPayload.begin(), strPayload.end());

            std::shared_ptr<CASHelper> casHelper = m_casService->getCasHelper();
            if(casHelper){
                casHelper->sendData(payload);
            }
            json_decref(root);
        }
        else{
            LOG_INFO("json error on line %d: %s\n", error.line, error.text);
        }
    }
    else{
        LOG_INFO("No Management Session - Invalid Operation");
    }
}

void RMFPlayer::destroy(const std::string& casOcdmId)
{
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
    std::lock_guard<std::mutex> guard(cas_mutex);
    if(m_casService)
    {
        delete m_casService;
        m_casService = NULL;
    }
    else{
        LOG_INFO("No Management Session - Invalid Operation");
    }
}
#endif

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
  m_mediaPlayer->rmf_setAudioMute(true);
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
    LOG_INFO("[%s:%d] Enter\n", __FUNCTION__, __LINE__);
    LOG_INFO("stop()");
    m_isLoaded = false;
    stopPlaybackProgressMonitor();
#ifdef USE_EXTERNAL_CAS
    std::lock_guard<std::mutex> guard(cas_mutex);
    if(m_casService)
    {
        std::shared_ptr<CASHelper> casHelper = m_casService->getCasHelper();
        if(casHelper){
            casHelper->setState(CASHelper::CASHelperState::PAUSED);
        }
	   std::vector<uint16_t> startPids;
           std::vector<uint16_t> stopPids;
           uint16_t pid = m_mediaPlayer->rmf_getVideoPid();
           if (pid != -1)
           {
               LOG_INFO("VideoPid = %d\n", pid);
               stopPids.push_back(pid);
           }
           pid = m_mediaPlayer->rmf_getAudioPid();
           if (pid != -1)
           {
               LOG_INFO("AudioPid = %d\n", pid);
               stopPids.push_back(pid);
           }
           if (stopPids.size() > 0)
           {
               m_casService->startStopDecrypt(startPids, stopPids);
           }
    }
    if(m_casService)
    {
        delete m_casService;
        m_casService = NULL;
    }
#endif

    m_mediaPlayer->rmf_stop();
    getParent()->getEventEmitter().send(OnClosedEvent());
#ifdef USE_EXTERNAL_CAS
    if(m_casService)
    {
        delete m_casService;
        m_casService = NULL;
    }
#endif
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

        int len = sprintf_s(buffer, sizeof(buffer), "QAM_TUNE_TIME:TYPE-%s,%u,%u,%u,%u,%u",
                           m_contentType.c_str(),
                           TO_MS(m_loadedTime - m_loadStartTime),        // time taken by load
                           TO_MS(m_loadCompleteTime - m_loadStartTime),  // time of onLoaded event
                           TO_MS(m_playEndTime - m_playStartTime),       // time taken by play
                           TO_MS(m_firstFrameTime - m_playStartTime),    // time to render first frame
                           TO_MS(m_firstFrameTime - m_loadStartTime));   // tune time

        if (len > 0)
        {
            tuneTimeLog = std::string(buffer, len);
        } else {
           ERR_CHK(len);
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

void RMFPlayer::psiReady()
{
#ifdef USE_EXTERNAL_CAS
    LOG_INFO("[%s:%d] - Enter\n", __FUNCTION__, __LINE__);
    PSIInfo psiInfo;

    if( getPATBuffer(psiInfo.pat) == 0 ||
        getPMTBuffer(psiInfo.pmt) == 0 )
    {
        LOG_INFO("Failed to get PSI buffers (PAT or PMT or CAT)");
        return;
    }
    std::lock_guard<std::mutex> guard(cas_mutex);
    if( getCATBuffer(psiInfo.cat) == 0 )
    {
        LOG_INFO("Failed to get CAT buffer");
    }
    else
    {
        m_casPending = 1;
    }

    if(!m_casService || m_casService->initialize(false, "", &psiInfo) == false)
    {
        if(m_casService)
            delete m_casService;
        m_casService = NULL;
        LOG_INFO("Failed to create Tuning Session for CAS");
        return;
    }

    if(m_casService->isManagementSession())
    {
        LOG_INFO("Managemnet Session - No stopStartDecrypt");
        return;
    }

    int videoPid = -1, audioPid = -1;
    if(((videoPid = m_mediaPlayer->rmf_getVideoPid()) != -1) &&
       ((audioPid = m_mediaPlayer->rmf_getAudioPid()) != -1))
    {
        LOG_INFO("VideoPid = %d, AudioPid = %d", videoPid, audioPid);
        m_lastAudioPid = audioPid;
        m_lastVideoPid = videoPid;
        std::vector<uint16_t> startPids;
        std::vector<uint16_t> stopPids;
        startPids.push_back(videoPid);
        startPids.push_back(audioPid);

        m_casService->startStopDecrypt(startPids, stopPids);
    }
    else{
        LOG_INFO("Failed to get the AV Pids. VideoPid = %d, AudioPid = %d", videoPid, audioPid);
    }
#endif
}

void RMFPlayer::languageChange()
{
#ifdef USE_EXTERNAL_CAS
    LOG_INFO("Enter languageChange");
    std::lock_guard<std::mutex> guard(cas_mutex);
    if(m_casService)
    {
        int audioPid = -1;
        if((m_lastAudioPid != -1) && (audioPid = m_mediaPlayer->rmf_getAudioPid()) != -1)
        {
            if(m_lastAudioPid != audioPid)
            {
                 LOG_INFO("m_lastAudioPid = %d, AudioPid = %d", m_lastAudioPid, audioPid);
                std::vector<uint16_t> startPids;
                std::vector<uint16_t> stopPids;
                startPids.push_back(audioPid);
                stopPids.push_back(m_lastAudioPid);
                m_casService->startStopDecrypt(startPids, stopPids);
                m_lastAudioPid = audioPid;
            }
            else
            {
                LOG_INFO("No Change in Audio Pids m_lastAudioPid = %d, AudioPid = %d", m_lastAudioPid, audioPid);
            }
        }
        else
        {
            LOG_INFO("Failed to get the Audio Pids. m_lastAudioPid = %d, AudioPid = %d", m_lastAudioPid, audioPid);
        }
    }
    else
    {
        LOG_INFO("CAS Service not available");
    }
#endif
}

void RMFPlayer::psiUpdateReceived(uint8_t psiStatus)
{
#ifdef USE_EXTERNAL_CAS
    LOG_INFO("Enter psiUpdateReceived\n");
    std::lock_guard<std::mutex> guard(cas_mutex);
    if(!m_casService)
    {
        LOG_INFO("m_casService not available");
        return;
    }
    PSIInfo psiInfo;
    bool hasBuffer = false;

    if((psiStatus & PAT_UPDATE) != 0)
    {
        if( getPATBuffer(psiInfo.pat) != 0)
        {
            LOG_INFO("get PSI buffers (PAT) Success");
            hasBuffer = true;
        }
    }
    else if( ((psiStatus & CAT_UPDATE) != 0) || ((psiStatus & CAT_ACQUIRE) != 0 && m_casPending) )
    {
        if( getCATBuffer(psiInfo.cat) != 0)
        {
            LOG_INFO("get PSI buffers (CAT) Success");
            hasBuffer = true;
	    m_casPending = 0;
        }
    }
    else if((psiStatus & PMT_UPDATE) != 0)
    {
        if( getPMTBuffer(psiInfo.pmt) != 0)
        {
            LOG_INFO("get PSI buffers (PMT) Success");
            hasBuffer = true;
        }
    }

    if(hasBuffer)
    {
        m_casService->updatePSI(psiInfo.pat,psiInfo.pmt,psiInfo.cat);
    }
#endif
}

void RMFPlayer::pmtUpdate()
{
#ifdef USE_EXTERNAL_CAS
    LOG_INFO("Enter pmtUpdate\n");
    std::lock_guard<std::mutex> guard(cas_mutex);
    if(!m_casService)
    {
        LOG_INFO("m_casService not available");
        return;
    }

    int videoPid = -1, audioPid = -1;

    if(((videoPid = m_mediaPlayer->rmf_getVideoPid()) != -1) &&
       ((audioPid = m_mediaPlayer->rmf_getAudioPid()) != -1))
    {
        LOG_INFO("VideoPid = %d, AudioPid = %d", videoPid, audioPid);
        std::vector<uint16_t> startPids;
        std::vector<uint16_t> stopPids;

        if(videoPid != m_lastVideoPid)
        {
            startPids.push_back(videoPid);
            stopPids.push_back(m_lastVideoPid);
        }
        if(audioPid != m_lastAudioPid)
        {
            startPids.push_back(audioPid);
            stopPids.push_back(m_lastAudioPid);
        }
        m_lastVideoPid = videoPid;
        m_lastAudioPid = audioPid;

        if ((startPids.size() != 0) || (stopPids.size() != 0))
        {
            m_casService->startStopDecrypt(startPids, stopPids);
        }
        else
        {
            LOG_INFO("No change in PIDs, not call startStopDecrypt");
        }
    }
    else
    {
        LOG_ERROR("Failed to get the AV Pids. VideoPid = %d, AudioPid = %d", videoPid, audioPid);
    }
#endif
}

int RMFPlayer::getCurrentAudioPid()
{
#ifdef USE_EXTERNAL_CAS
    return m_lastAudioPid;
#endif
}

int RMFPlayer::get_section_length(vector<uint8_t>sectionDataBuffer)
{
#ifdef USE_EXTERNAL_CAS
    int nSect = (((sectionDataBuffer[1]<<8)|(sectionDataBuffer[2]<<0))&0xFFF);
    RDK_LOG(RDK_LOG_INFO, "LOG.RDK.INBSI", "section_length %d, sectionDataBuffer.size() - %d \n", nSect, sectionDataBuffer.size());
    return nSect;
#endif
}

vector<uint8_t> RMFPlayer::get_multiple_section_data(vector<uint8_t>&sectionDataBuffer, int sectionSize)
{
#ifdef USE_EXTERNAL_CAS
    std::vector<uint8_t> sectionDataParsed (sectionDataBuffer.begin(), sectionDataBuffer.begin() + sectionSize);
    sectionDataBuffer.erase(sectionDataBuffer.begin(), sectionDataBuffer.begin() + sectionSize);
    RDK_LOG(RDK_LOG_INFO, "LOG.RDK.INBSI", "get_multiple_section_data sectionDataBuffer.size() -  %d, parsed - %d\n", sectionDataBuffer.size(),sectionDataParsed.size());
    return sectionDataParsed;
#endif
}

void RMFPlayer::sectionDataReceived()
{
#ifdef USE_EXTERNAL_CAS
    LOG_INFO("sectionDataReceived");
    uint32_t filterId = 0;
    std::vector<uint8_t> sectionData;
    int sectionLength = 0;
    do {
        m_mediaPlayer->getSectionData(&filterId, sectionData);
        std::lock_guard<std::mutex> guard(cas_mutex);
	if(!filterId )
	{
        	RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.INBSI", "filterId not available");
		return;
	}
	if(!m_casService)
	{
		LOG_ERROR("m_casService not available\n");
                return;
	}
	sectionLength = get_section_length(sectionData);
        RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.INBSI", "sectionLength %d, sectionData.size() - %d\n", sectionLength,(int)sectionData.size());
	while((int)sectionData.size() > (sectionLength + HEADER_SIZE)){
	    std::vector<uint8_t> sectionDataParsed;
	    sectionDataParsed = get_multiple_section_data(sectionData, (sectionLength + HEADER_SIZE));
	    m_casService->processSectionData(filterId, sectionDataParsed);
	    sectionLength = get_section_length(sectionData);
	};

	LOG_INFO("sectionDataReceived for Filter id = %d\n", filterId);
	m_casService->processSectionData(filterId, sectionData);

    }while(filterId);
#endif
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
    const char* tuneStrIdentifier = "tune://";
#ifdef ENABLE_RDKMEDIAPLAYER_TS_QAM_TEST
    const char* tsStrIdentifier  = ".ts";
#endif

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
    else if ((url.find(liveStrIdentifier) != std::string::npos) || (url.find(tuneStrIdentifier) != std::string::npos))
    {
        contentType = "LIVE";
    }
    else if (url.find(vodStrIdentifier) != std::string::npos)
    {
        contentType = "VOD";
    } 
#ifdef ENABLE_RDKMEDIAPLAYER_TS_QAM_TEST
    else if (url.find(tsStrIdentifier) != std::string::npos)
    {
        contentType = "LIVE";
    }
#endif
    else
    {
        contentType = "Unsupported";
        return false;
    }
    return true;
}
