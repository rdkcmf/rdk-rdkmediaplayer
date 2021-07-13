/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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

#ifndef _CAS_SERVICE_H_
#define _CAS_SERVICE_H_

#include <memory>
#include "mediaplayer.h"
#include "rdkmediaplayerimpl.h"
#include "CASDataListener.h"
#include "CASManager.h"
#include "CASHelper.h"
#include "rmfbase.h"
#include "ICasSectionFilter.h"

using namespace anycas;
class MediaPlayerSink;
class RMFPlayer;

class PSIInfo
{
public:
    std::vector<uint8_t> pat;
    std::vector<uint8_t> pmt;
    std::vector<uint8_t> cat;
};

class CASService : public CASDataListener, CASStatusInform
{
public:
    CASService(const EventEmitter& emit, const CASEnvironment& env, ICasSectionFilter* casSFInterface, ICasPipeline *casPipelineInterface)
                :emit_(emit),
                 env_(env),
                 casSFInterface_(casSFInterface),
                 casPipelineInterface_(casPipelineInterface),
                 bManagementSession_(false) { }
    ~CASService() = default;

    /**
     *  @brief This API to initialize the CAS Serivce includes casManager and casHelper
     *
     *  @param[in] management   flag to represents whether CAS created for Management
     *  @param[in] casOcdmId    CAS openCDM Id
     *  @param[in] psiInfo      PSI info, more of PAT, PMT, CAT
     *
     *  @return Returns status of the operation true or false
     *  @retval true on success, false upon any failure.
     */
    bool initialize(bool management, const std::string& casOcdmId, PSIInfo *psiInfo);

    /**
     *  @brief This API to start/stop the decryption of the audio/video content
     *
     *  @param[in] startPids   Pids List to be started to play (Audio Video Pids)
     *  @param[in] stopPids    Pids List to be stopped which is currently playing (empty for the Channel tune, e.g. used upon Audio Language change)
     *
     *  @return Returns status of the operation true or false
     *  @retval true on success, false upon any failure.
     */
    bool startStopDecrypt(const std::vector<uint16_t>& startPids, const std::vector<uint16_t>& stopPids);

    /**
     *  @brief This API is updatePSI when PAT/PMT/CAT updates.
     *
     *  @param[in] pat   updated PAT Buffer
     *  @param[in] pmt   updated PMT Buffer
     *  @param[in] cat   updated CAT Buffer
     */
    void updatePSI(const std::vector<uint8_t>& pat, const std::vector<uint8_t>& pmt, const std::vector<uint8_t>& cat);

    /**
     *  @brief This API is invoked by CAS plugin when it requires to sends the CAS Status
     *
     *  @param[in] status   CAS Status which contains the status, errorno and message
     */
    virtual void informStatus(const CASStatus& status) override;

    /**
     *  @brief This API is invoked by CAS plugin when it requires to push data
     *
     *  @param[in] data   data to be send back as response asynchronously
     */
    void casPublicData(const std::vector<uint8_t>& data);

    /**
     *  @brief This API is invoked by CAS plugin when it requires to push data
     *
     *  @param[in] filterId     filter id for the corressponding section data received
     *  @param[in] data         section data to be processed
     */
    void processSectionData(const uint32_t& filterId, const std::vector<uint8_t>& data);

    /**
     *  @brief This API is to get CASHelper
     *
     *  @return Returns the CAS Helper object
     */
    std::shared_ptr<CASHelper> getCasHelper() { return casHelper_; }

    /**
     *  @brief This API is to check whether current one is for Managemnet or Tuning Session
     *
     *  @return Returns true or false whether it is a session for tuning or management
     *  @retval true if a management session, false otherwise
     */
    bool isManagementSession() const;

private:
    std::shared_ptr<CASManager> casManager_;
    std::shared_ptr<CASHelper> casHelper_;
    EventEmitter emit_;
    PSIInfo psiInfo;
    bool bManagementSession_;
    std::string casOcdmId_;
    CASEnvironment env_;

    ICasSectionFilter* casSFInterface_;
    ICasPipeline* casPipelineInterface_;

};

#endif  // _CAS_SERVICE_H_
