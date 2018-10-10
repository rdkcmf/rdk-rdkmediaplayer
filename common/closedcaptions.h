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
#ifndef CLOSEDCAPTIONS_H
#define CLOSEDCAPTIONS_H

#include "vlCCConstants.h"
#include <map>
#include <vector>

class ClosedCaptions
{
public:
    ClosedCaptions();
    ~ClosedCaptions();
    bool start(void* pVidDecoderHandle);
    bool stop();
    void setVisible(bool is_visible);
    bool setEnabled(bool bNewState);
    bool isEnabled() { return m_isCCEnabled; }
    std::string getAvailableTracks();
#if 0
    void setAttribute(std::map<std::string, std::string> options);
    bool ccEASStarted ();
    bool ccEASStopped ();
    bool ccParentalLockStart();
    bool ccParentalLockStop();
    void ccGfxPreResolution(unsigned id);
    void ccGfxSetResolution(unsigned id, int width, int height);
    void ccGfxPostResolution(int id, int width, int height);
    bool stopCCatEAS () { return ccStop(); }
    bool stopCCatTrickMode ();
    bool startCCatNormalMode ();
    std::map<std::string, std::string> getSupportedOptions();
    bool setTrack(const std::string& track);
#endif
private:
    void ccInit (void);
    bool ccStart (void);
    bool ccStop (void);
/*
    int getColor (gsw_CcAttribType attributeIndex, gsw_CcType ccType, QString inputStr, gsw_CcColor *pGetColor);
    int getOpacity (QString inputStr, gsw_CcOpacity *pGetOpacity);
    int getFontSize (QString inputStr, gsw_CcFontSize *pGetFontSize);
    int getFontStyle (QString inputStr, gsw_CcFontStyle *pGetFontStyle);
    int getEdgeType (QString inputStr, gsw_CcEdgeType *pGetEdgeType);
    int getTextStyle (QString inputStr, gsw_CcTextStyle *pGetTextStyle);
*/
    std::map<std::string, std::string> m_ccOptions;
    void* m_viddecHandle;
    bool m_isCCEnabled;
    bool m_isCCRendering;
    bool m_isCCReaderStarted;
    bool m_isParentalBlocked;
    bool m_isCCStopedAtTrickMode;
    int m_CCVisibilityStatus;
    static bool m_wasLoadedOnce;

};

#endif
