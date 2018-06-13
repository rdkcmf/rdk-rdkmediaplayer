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
#include "closedcaptions.h"
#include <stdio.h>
#include "ccDataReader.h"
#include "cc_util.h"
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
#define private public
#endif
#include "vlGfxScreen.h"
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
#undef private
#endif
#if USE_SYSRES_MLT
#include "rpl_new.h"
#endif
#include <unistd.h>
#include "logger.h"

#if 0
static void cc_msg_callback(CC_MSG_type_t decoderMsg)
{
    if (CC_MSG_RENDER_START == decoderMsg)
    {
        LOG_WARNING("CC IS BEING DISPLAYED");
    }
    return;
}
#endif

bool ClosedCaptions::m_wasLoadedOnce = false;

#if 0
void ClosedCaptions::shutdown()
{
    struct MyvlGfxScreen: public vlGfxScreen
    {
        static void shutdown()
        {
            IDirectFB* dfb = get_dfb();
            if(dfb)
                dfb->Release(dfb);
        }
    };
    
    if(m_wasLoadedOnce)
    {
        m_wasLoadedOnce = false;
        MyvlGfxScreen::shutdown();
    }
}
#endif

ClosedCaptions::ClosedCaptions()
{
    m_CCVisibilityStatus = 0;
}


ClosedCaptions::~ClosedCaptions()
{
}


bool ClosedCaptions::setEnabled(bool bNewState)
{
    bool ret = true;
    LOG_INFO("setEnabled enabled=%d", (int)bNewState);

    if(bNewState == m_isCCEnabled)
    {
        /* Trying to set the same state as previous */
        return ret;
    }

    m_isCCEnabled = bNewState;

    /* Decoder handle will be Set to a value only when it is playing mDVR content */ 
    if (m_viddecHandle)
    {
        if ((m_isCCEnabled) && (!m_isParentalBlocked) && (!m_isCCStopedAtTrickMode))
        {
            ret = ccStart();
            if (m_ccOptions.size() > 0)
            {
                //setAttribute(m_ccOptions);
            }
        }
        else
        {
            ret = ccStop();
        }
    }
    if(0 == access("/tmp/force_cc_enable_disable_failure", F_OK))
    {
        LOG_WARNING("Forcing setEnabled Failure.");
        ret = false;
    }
    return ret;
}


void ClosedCaptions::setVisible(bool is_visible)
{
    LOG_INFO("Entered %s \n", __FUNCTION__);
    if(is_visible)
    {
        ccShow();
//        Platform::Screen::showCCPlane(true);
    }
    else
    {
        ccHide();
//        Platform::Screen::showCCPlane(false);
    }
}


bool ClosedCaptions::start(void* pViddecHandle)
{
    LOG_INFO("start: viddecHandle=%p", pViddecHandle);
    //CCREADER_RegisterForLog(cclogcallback); //enable this only if you platform has corresponding modification in closed caption component.
    if (NULL != pViddecHandle)
    {
        if (!m_isCCRendering)
        {
            ccInit();
            int status = media_closeCaptionStart (pViddecHandle);
            if (status < 0)
            {
                LOG_ERROR("start media_closeCaptionStart failed %d", status);
                return false;
            }
            m_isCCReaderStarted = true;

            m_viddecHandle = pViddecHandle;

            m_wasLoadedOnce = true;


            if ((m_isCCEnabled) && (!m_isParentalBlocked) && (!m_isCCStopedAtTrickMode))
            {
#if 0
                /** For testing, set the attributres locally and see whether it reflects or not */
                QVariantHash options;

                options["textForegroundColor"] = "BLUE";
                options["textItalicized"] = "false";
                options["windowBorderEdgeStyle"] = "NONE";
                options["textEdgeColor"] = "RED";
                options["textEdgeStyle"] = "Uniform";
                options["windowBorderEdgeColor"] = "BLACK";
                options["textBackgroundOpacity"] = "SOLID";
                options["textUnderline"] = "false";
                options["windowFillColor"] = "white";
                options["textSize"] = "Medium";
                options["textBackgroundColor"] = "Green";
                options["fontStyle"] = "DEFAULT";
                options["textForegroundOpacity"] = "SOLID";
                options["windowFillOpacity"] = "TRANSPARENT";

                setAttribute (options);
#endif
                ccStart();
                if (m_ccOptions.size() > 0)
                {
//                    setAttribute(m_ccOptions);
                }

                LOG_INFO("ClosedCaptions::start - Started CC data fetching");
            }
            else
            {
                ccStop();
                LOG_INFO("ClosedCaptions::start - CC is not enabled");
            }
        }
        else
        {
            LOG_INFO("ClosedCaptions::start - Already started");
            return false;
        }
    }
#ifdef USE_CLOSED_CAPTIONING_CHIPSET_I
    else if ((pViddecHandle == NULL) && (m_viddecHandle != NULL))
    {
	        // special intel use case to stop decoder when viddec goes to zero 
            int status = media_closeCaptionStop();
            if (status < 0)
            {
                LOG_ERROR("ClosedCaptions::start media_closeCaptionStop failed %d", status);
            }
            else
            {
                LOG_INFO("stop succeeded");
            }
            m_viddecHandle = NULL;
    }
#endif
    else
    {
        LOG_ERROR("ClosedCaptions::start - Invalid video decoder handle passed");
        return false;
    }
    return true;
}

bool ClosedCaptions::stop()
{
    LOG_INFO("stop");

    if (ccStop())
    {
        if (m_viddecHandle)
        {
            int status = media_closeCaptionStop();
            if (status < 0)
            {            
                LOG_ERROR("ClosedCaptions::stop media_closeCaptionStop failed %d", status);
                return false;  
            }
            else
            {
                LOG_INFO("ClosedCaptions::stop succeeded");
            }

            m_viddecHandle = NULL;
        }
    }
    /* Reset the flags */
    m_isParentalBlocked = false;
    m_isCCStopedAtTrickMode = false;
    m_isCCReaderStarted = false;
    return true;
}


bool ClosedCaptions::ccStart()
{
    if (m_isCCReaderStarted && !m_isCCRendering)
    {
        int ret = ccSetCCState(CCStatus_ON, 1);
        m_isCCRendering = true;
        LOG_WARNING("ClosedCaptions::ccStart started");
        if(0 == access("/tmp/force_cc_enable_disable_failure", F_OK))
        {
            LOG_WARNING("Forcing ccStart failure");
            return false;
        }
        if(0 != ret)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}

bool ClosedCaptions::ccStop()
{
    if (m_isCCRendering)
    {
        ccSetCCState(CCStatus_OFF, 1);
        m_isCCRendering = false;
        LOG_INFO("ClosedCaptions::ccStart stopped");
    }
    return true;
}

void ClosedCaptions::ccInit (void)
{
    static bool bInitStatus = false;
    
    if (!bInitStatus)
    {
        vlGfxInit(0);
        int status = vlMpeosCCManagerInit();
        if (0 != status)
        {
            LOG_ERROR("ClosedCaptions::ccInit vlMpeosCCManagerInit failed %d", status);
            return;
        }
       
        bInitStatus = true;
        LOG_INFO("ClosedCaptions::ccInit succeeded");
        /*Need to check the CC visibility status to check for Events generated before CC Initialization*/
        if (0 != m_CCVisibilityStatus)
        {
            setVisible(false);
        }
        //ccRegisterMsgCallback(cc_msg_callback);
    }
    else
    {
        LOG_INFO("ClosedCaptions::ccInit - Already inited.. nothing to be done");
    }

    return;
}

std::string ClosedCaptions::getAvailableTracks()
{
    LOG_INFO("getAvailableTracks");
    //TODO - replace with real API because some stream might have no CC tracks or different CC tracks
    return "1,2,3,4,5,6";
}

#if 0
void ClosedCaptions::setEvent(bool status, moduleEvents eventType)
{
    LOG_INFO("Entered %s \n", __FUNCTION__);

    if (true == status)
    {
        m_CCVisibilityStatus |= eventType;
    }
    else
    {
        if (m_CCVisibilityStatus & eventType)
        {
           m_CCVisibilityStatus ^= eventType;
        }        
    }
    if (0 == m_CCVisibilityStatus)
    {
        setVisible(true);
    }
    else
    {
        setVisible(false);
    }

}

QVariantList ClosedCaptions::getSupportedOptions()
{
    LOG_INFO("getSupportedOptions");
    //from Parker-Comcast-SP-Tru2way-VSP-W01-110715 doc
    //note: textEdgeColor/textEdgeStyle are not in Parker-Comcast-SP-Tru2way-VSP-W01-110715 doc but support by api
    //note: added fontSize because I'm not sure if its suppose to be textSize or fontSize - TODO - determine which one to use
    QVariantList list;
    list    << "textSize" 
            << "fontStyle" 
            << "textForegroundColor" 
            << "textForegroundOpacity" 
            << "textBackgroundColor" 
            << "textBackgroundOpacity" 
            << "textItalicized" 
            << "textUnderline" 
            << "windowFillColor" 
            << "windowFillOpacity" 
            << "windowBorderEdgeColor" 
            << "windowBorderEdgeStyle"
            << "textEdgeColor"
            << "textEdgeStyle"
            << "fontSize";
    return list;
}

QVariantList ClosedCaptions::getServiceDetails()
{
    QVariantList availableCCServices;
    QVariantHash ccServiceDetails;

    LOG_INFO("getServiceDetails");
    dumpCaptionDescriptor();

    //TODO - needs to add mutex protection for m_cationServices
    int size = m_cationServices.length();
    closedCaptionsService serviceData;

    for(int i = 0; i < size;i++)
    {
       serviceData = m_cationServices.at(i);

       ccServiceDetails.insert("ccType",serviceData.ccType);
       ccServiceDetails.insert("track",serviceData.track);
       ccServiceDetails.insert("language",serviceData.language);
       ccServiceDetails.insert("hasEasyReader",serviceData.hasEasyReader);
       ccServiceDetails.insert("isWideAspectRatio",serviceData.isWideAspectRatio);
       availableCCServices.append(ccServiceDetails);
    }
    return availableCCServices;
}

bool ClosedCaptions::setTrack(const QString& track)
{
    int iTrack = track.toInt();
    gsw_CcAnalogServices analogChannel = GSW_CC_ANALOG_SERVICE_CC1;
    LOG_WARNING("setTrack track=%s index=%d", track.toAscii().constData(), iTrack);

    /*In case of invalid track value set the track to primary channel*/
    if(iTrack < 1 || iTrack > 6)
    {
        iTrack = 1;
    }
    int status = ccSetDigitalChannel((unsigned int)iTrack);
    if(0 != status)
    {
        LOG_ERROR("setTrack ccSetDigitalChannel failed %d", status);
        return false;
    }
    switch (iTrack)
    {
        case 1:
            analogChannel = GSW_CC_ANALOG_SERVICE_CC1;
        break;
        case 2:
            analogChannel = GSW_CC_ANALOG_SERVICE_CC2;
        break;
        case 3:
            analogChannel = GSW_CC_ANALOG_SERVICE_CC3;
        break;
        case 4:
            analogChannel = GSW_CC_ANALOG_SERVICE_CC4;
        break;
        default:
            analogChannel = GSW_CC_ANALOG_SERVICE_NONE;
        break;
    }
    status = ccSetAnalogChannel(analogChannel);
    if(0 != status)
    {
        LOG_ERROR("setTrack ccSetAnalogChannel failed %d", status);
        return false;
    }
    else
    {
        if(0 == access("/tmp/force_cc_set_track_failure", F_OK))
        {
            LOG_WARNING("Forcing CC setTrack failure");
            return false;
        }
        return true;
    }
}

int ClosedCaptions::getColor (gsw_CcAttribType attributeIndex, gsw_CcType ccType, QString inputStr, gsw_CcColor *pGetColor)
{
    static gsw_CcColor* pCCcolorCaps[GSW_CC_COLOR_MAX];
    static bool bFlagForMalloc = false;
    unsigned int ccSizeOfCabs = 0;
    QByteArray inputBytes = inputStr.toAscii();
    const char* pColorTextIn = inputBytes.constData();
    LOG_INFO("%s: %s", __FUNCTION__, pColorTextIn ? pColorTextIn : "NULL");
    static const int MAX_COLOR_BUFFER_LEN = ((GSW_MAX_CC_COLOR_NAME_LENGTH > 8 ? GSW_MAX_CC_COLOR_NAME_LENGTH : 8) + 1);
    char pColorText[MAX_COLOR_BUFFER_LEN];

    if ((pColorTextIn) && (pGetColor))
    {

        if (!bFlagForMalloc)
        {
            bFlagForMalloc = true;
            for (int iLoop = 0; iLoop < GSW_CC_COLOR_MAX; iLoop++)
            {
                pCCcolorCaps[iLoop] = (gsw_CcColor*) malloc(sizeof(gsw_CcColor));
                memset (pCCcolorCaps[iLoop], 0, sizeof(gsw_CcColor));
            }
        }

        //if HEX format, convert to color
        if (0 == strncasecmp  ("0x000000", pColorTextIn, MAX_COLOR_BUFFER_LEN))
            strncpy(pColorText,"BLACK", sizeof(pColorText)); /* RDKSEC-585 unsafe function replaced*/
        else
        if (0 == strncasecmp  ("0xFFFFFF", pColorTextIn, MAX_COLOR_BUFFER_LEN))
            strncpy(pColorText,"WHITE", sizeof(pColorText));
        else
        if (0 == strncasecmp  ("0xFF0000", pColorTextIn, MAX_COLOR_BUFFER_LEN))
            strncpy(pColorText,"RED", sizeof(pColorText));
        else
        if (0 == strncasecmp  ("0x00FF00", pColorTextIn, MAX_COLOR_BUFFER_LEN))
            strncpy(pColorText,"GREEN", sizeof(pColorText));
        else
        if (0 == strncasecmp  ("0x0000FF", pColorTextIn, MAX_COLOR_BUFFER_LEN))
            strncpy(pColorText,"BLUE", sizeof(pColorText));
        else
        if (0 == strncasecmp  ("0xFFFF00", pColorTextIn, MAX_COLOR_BUFFER_LEN))
            strncpy(pColorText,"YELLOW", sizeof(pColorText));
        else
        if (0 == strncasecmp  ("0xFF00FF", pColorTextIn, MAX_COLOR_BUFFER_LEN))
            strncpy(pColorText,"MAGENTA", sizeof(pColorText));
        else
        if (0 == strncasecmp  ("0x00FFFF", pColorTextIn, MAX_COLOR_BUFFER_LEN))
            strncpy(pColorText,"CYAN", sizeof(pColorText));
        else
        if (0 == strncasecmp  ("0xFF000000", pColorTextIn, MAX_COLOR_BUFFER_LEN))
            strncpy(pColorText,"AUTO", sizeof(pColorText));
        else
            strncpy(pColorText, pColorTextIn, MAX_COLOR_BUFFER_LEN);//could be a string color or could just be wrong

        ccGetCapability(attributeIndex, ccType, (void**) &pCCcolorCaps, &ccSizeOfCabs);
        bool found = false;
        for (unsigned int i = 0; i < ccSizeOfCabs; i++)
        {
            //LOG_INFO("%s color caps: %s", __FUNCTION__, pCCcolorCaps[i]->name);
            if (0 == strncasecmp  (pCCcolorCaps[i]->name, pColorText, GSW_MAX_CC_COLOR_NAME_LENGTH))
            {
                LOG_INFO("%s found match %s", __FUNCTION__, pCCcolorCaps[i]->name);
                memcpy (pGetColor, pCCcolorCaps[i], sizeof (gsw_CcColor));
                found = true;
                break;
            }
        }
        if(!found)
            LOG_WARNING("%s: Unsupported Color type %s", __FUNCTION__, pColorText);
    }
    else
    {
        LOG_WARNING("%s: Null input", __FUNCTION__);
        return -1;
    }
    return 0;
}

int ClosedCaptions::getOpacity (QString inputStr, gsw_CcOpacity *pGetOpacity)
{
    QByteArray inputBytes = inputStr.toAscii();
    const char* pGivenOpacity = inputBytes.constData();
    LOG_INFO("%s: %s", __FUNCTION__, pGivenOpacity ? pGivenOpacity : "NULL");
    if ((pGivenOpacity) && (pGetOpacity))
    {
        if (0 == strncasecmp (pGivenOpacity, "solid", strlen ("solid")))
            *pGetOpacity = GSW_CC_OPACITY_SOLID;
        else if (0 == strncasecmp (pGivenOpacity, "flash", strlen ("flash")))
            *pGetOpacity = GSW_CC_OPACITY_FLASHING;
        else if (0 == strncasecmp (pGivenOpacity, "translucent", strlen ("translucent")))
            *pGetOpacity = GSW_CC_OPACITY_TRANSLUCENT;
        else if (0 == strncasecmp (pGivenOpacity, "transparent", strlen ("transparent")))
            *pGetOpacity = GSW_CC_OPACITY_TRANSPARENT;
        else if (0 == strncasecmp (pGivenOpacity, "auto", strlen ("auto")))
            *pGetOpacity = GSW_CC_OPACITY_EMBEDDED;
        else
        {
            LOG_WARNING("%s: Unsupported Opacity type %s", __FUNCTION__, pGivenOpacity);
        }
    }
    else
    {
        LOG_WARNING("%s: Null input", __FUNCTION__);
        return -1;
    }
    return 0;
}

int ClosedCaptions::getFontSize (QString inputStr, gsw_CcFontSize *pGetFontSize)
{
    QByteArray inputBytes = inputStr.toAscii();
    const char* pGivenFontSize = inputBytes.constData();
    LOG_INFO("%s: %s", __FUNCTION__, pGivenFontSize ? pGivenFontSize : "NULL");
    if ((pGivenFontSize) && (pGetFontSize))
    {
        if (0 == strncasecmp (pGivenFontSize, "small", strlen ("small")))
            *pGetFontSize = GSW_CC_FONT_SIZE_SMALL;
        else if ((0 == strncasecmp (pGivenFontSize, "standard", strlen ("standard"))) || (0 == strncasecmp (pGivenFontSize, "medium", strlen ("medium"))))
            *pGetFontSize = GSW_CC_FONT_SIZE_STANDARD;
        else if (0 == strncasecmp (pGivenFontSize, "large", strlen ("large")))
            *pGetFontSize = GSW_CC_FONT_SIZE_LARGE;
        else if (0 == strncasecmp (pGivenFontSize, "auto", strlen ("auto")))
            *pGetFontSize = GSW_CC_FONT_SIZE_EMBEDDED;
        else
        {
            LOG_WARNING("%s: Unsupported Font Size type %s", __FUNCTION__, pGivenFontSize);
        }
    }
    else
    {
        LOG_WARNING("%s: Null input", __FUNCTION__);
        return -1;
    }
    return 0;
}

int ClosedCaptions::getFontStyle (QString inputStr, gsw_CcFontStyle *pGetFontStyle)
{
    QByteArray inputBytes = inputStr.toAscii();
    const char* pGivenFontStyle = inputBytes.constData();
    LOG_INFO("%s: %s", __FUNCTION__, pGivenFontStyle ? pGivenFontStyle : "NULL");
    if ((pGivenFontStyle) && (pGetFontStyle))
    {
        if (0 == strncasecmp (pGivenFontStyle, "default", strlen ("default")))
            memcpy (pGetFontStyle, GSW_CC_FONT_STYLE_DEFAULT, sizeof(gsw_CcFontStyle));
        else if ((0 == strncasecmp (pGivenFontStyle, "monospaced_serif", strlen ("monospaced_serif"))) || (0 == strncasecmp (pGivenFontStyle, "Monospaced serif", strlen ("Monospaced serif"))))
            memcpy (pGetFontStyle, GSW_CC_FONT_STYLE_MONOSPACED_SERIF, sizeof(gsw_CcFontStyle));
        else if ((0 == strncasecmp (pGivenFontStyle, "proportional_serif", strlen ("proportional_serif"))) || (0 == strncasecmp (pGivenFontStyle, "Proportional serif", strlen ("Proportional serif"))))
            memcpy (pGetFontStyle, GSW_CC_FONT_STYLE_PROPORTIONAL_SERIF, sizeof(gsw_CcFontStyle));
        else if ((0 == strncasecmp (pGivenFontStyle, "monospaced_sanserif", strlen ("monospaced_sanserif"))) || (0 == strncasecmp (pGivenFontStyle, "Monospaced sans serif", strlen ("Monospaced sans serif"))))
            memcpy (pGetFontStyle, GSW_CC_FONT_STYLE_MONOSPACED_SANSSERIF, sizeof(gsw_CcFontStyle));
        else if ((0 == strncasecmp (pGivenFontStyle, "proportional_sanserif", strlen ("proportional_sanserif"))) || (0 == strncasecmp (pGivenFontStyle, "Proportional sans serif", strlen ("Proportional sans serif"))))
            memcpy (pGetFontStyle, GSW_CC_FONT_STYLE_PROPORTIONAL_SANSSERIF, sizeof(gsw_CcFontStyle));
        else if (0 == strncasecmp (pGivenFontStyle, "casual", strlen ("casual")))
            memcpy (pGetFontStyle, GSW_CC_FONT_STYLE_CASUAL, sizeof(gsw_CcFontStyle));
        else if (0 == strncasecmp (pGivenFontStyle, "cursive", strlen ("cursive")))
            memcpy (pGetFontStyle, GSW_CC_FONT_STYLE_CURSIVE, sizeof(gsw_CcFontStyle));
        else if ((0 == strncasecmp (pGivenFontStyle, "smallcaps", strlen ("smallcaps"))) || (0 == strncasecmp (pGivenFontStyle, "small capital", strlen ("small capital"))))
            memcpy (pGetFontStyle, GSW_CC_FONT_STYLE_SMALL_CAPITALS, sizeof(gsw_CcFontStyle));
        else if (0 == strncasecmp (pGivenFontStyle, "auto", strlen ("auto")))
            memcpy (pGetFontStyle, GSW_CC_FONT_STYLE_EMBEDDED, sizeof(gsw_CcFontStyle));
        else
            LOG_WARNING("%s: Unsupported Font style type %s", __FUNCTION__, pGivenFontStyle);
    }
    else
    {
        LOG_WARNING("%s: Null input", __FUNCTION__);
        return -1;
    }
    return 0;
}

int ClosedCaptions::getEdgeType (QString inputStr, gsw_CcEdgeType *pGetEdgeType)
{
    QByteArray inputBytes = inputStr.toAscii();
    const char* pGivenEdgeType = inputBytes.constData();
    LOG_INFO("%s: %s", __FUNCTION__, pGivenEdgeType ? pGivenEdgeType : "NULL");
    if ((pGivenEdgeType) && (pGetEdgeType))
    {
        if (0 == strncasecmp (pGivenEdgeType, "none", strlen ("none")))
            *pGetEdgeType = GSW_CC_EDGE_TYPE_NONE;
        else if (0 == strncasecmp (pGivenEdgeType, "raised", strlen ("raised")))
            *pGetEdgeType = GSW_CC_EDGE_TYPE_RAISED;
        else if (0 == strncasecmp (pGivenEdgeType, "depressed", strlen ("depressed")))
            *pGetEdgeType = GSW_CC_EDGE_TYPE_DEPRESSED;
        else if (0 == strncasecmp (pGivenEdgeType, "uniform", strlen ("uniform")))
            *pGetEdgeType = GSW_CC_EDGE_TYPE_UNIFORM;
        else if ((0 == strncasecmp (pGivenEdgeType, "drop_shadow_left", strlen ("drop_shadow_left"))) || (0 == strncasecmp (pGivenEdgeType, "Left drop shadow", strlen ("Left drop shadow"))))
            *pGetEdgeType = GSW_CC_EDGE_TYPE_SHADOW_LEFT;
        else if ((0 == strncasecmp (pGivenEdgeType, "drop_shadow_right", strlen ("drop_shadow_right"))) || (0 == strncasecmp (pGivenEdgeType, "Right drop shadow", strlen ("Right drop shadow"))))
            *pGetEdgeType = GSW_CC_EDGE_TYPE_SHADOW_RIGHT;
        else if (0 == strncasecmp (pGivenEdgeType, "auto", strlen ("auto")))
            *pGetEdgeType = GSW_CC_EDGE_TYPE_EMBEDDED;
        else
            LOG_WARNING("%s: Unsupported Edge type %s", __FUNCTION__, pGivenEdgeType);
    }
    else
    {
        LOG_WARNING("%s: Null input", __FUNCTION__);
        return -1;
    }
    return 0;
}

int ClosedCaptions::getTextStyle (QString inputStr, gsw_CcTextStyle *pGetTextStyle)
{
    QByteArray inputBytes = inputStr.toAscii();
    const char* pGivenTextStyle = inputBytes.constData();
    LOG_INFO("%s: %s", __FUNCTION__, pGivenTextStyle ? pGivenTextStyle : "NULL");
    if ((pGivenTextStyle) && (pGetTextStyle))
    {
        if (0 == strncasecmp (pGivenTextStyle, "false", strlen ("false")))
            *pGetTextStyle = GSW_CC_TEXT_STYLE_FALSE;
        else if (0 == strncasecmp (pGivenTextStyle, "true", strlen ("true")))
            *pGetTextStyle = GSW_CC_TEXT_STYLE_TRUE;
        else if (0 == strncasecmp (pGivenTextStyle, "auto", strlen ("auto")))
            *pGetTextStyle = GSW_CC_TEXT_STYLE_EMBEDDED_TEXT;
        else
        {
            LOG_WARNING("%s: Unsupported Text style  %s", __FUNCTION__, pGivenTextStyle);
        }
    }
    else
    {
        LOG_WARNING("%s: Null input", __FUNCTION__);
        return -1;
    }
    return 0;
}

void  ClosedCaptions::setAttribute(QVariantHash options)
{
    LOG_WARNING("setAttribute");

    if (m_ccOptions != options)
    {
        m_ccOptions = options;
    }

    gsw_CcAttributes attribute;

    /** Get the current attributes */
    ccGetAttributes (&attribute, GSW_CC_TYPE_DIGITAL);

    /* Font FG Color */
    {
        /* Get the color */
        getColor (GSW_CC_ATTRIB_FONT_COLOR, GSW_CC_TYPE_DIGITAL, options["textForegroundColor"].toString(), &(attribute.charFgColor));

        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_COLOR, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_COLOR, GSW_CC_TYPE_ANALOG);
    }

    /* Font BG Color */
    {
        /* Get the color */
        getColor (GSW_CC_ATTRIB_BACKGROUND_COLOR, GSW_CC_TYPE_DIGITAL, options["textBackgroundColor"].toString(), &(attribute.charBgColor));

        ccSetAttributes(&attribute, GSW_CC_ATTRIB_BACKGROUND_COLOR, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_BACKGROUND_COLOR, GSW_CC_TYPE_ANALOG);
    }

    /* Window border color */
    {
        /* Get the color */
        getColor (GSW_CC_ATTRIB_BORDER_COLOR, GSW_CC_TYPE_DIGITAL, options["windowBorderEdgeColor"].toString(), &(attribute.borderColor));

        ccSetAttributes(&attribute, GSW_CC_ATTRIB_BORDER_COLOR, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_BORDER_COLOR, GSW_CC_TYPE_ANALOG);
    }

    /* Window COLOR */
    {
        /* Get the color */
        getColor (GSW_CC_ATTRIB_WIN_COLOR, GSW_CC_TYPE_DIGITAL, options["windowFillColor"].toString(), &(attribute.winColor));

        ccSetAttributes(&attribute, GSW_CC_ATTRIB_WIN_COLOR, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_WIN_COLOR, GSW_CC_TYPE_ANALOG);
    }

    /* Test Edge color */
    {
        /* Get the color */
        getColor (GSW_CC_ATTRIB_EDGE_COLOR, GSW_CC_TYPE_DIGITAL, options["textEdgeColor"].toString(), &(attribute.edgeColor));

        ccSetAttributes(&attribute, GSW_CC_ATTRIB_EDGE_COLOR, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_EDGE_COLOR, GSW_CC_TYPE_ANALOG);
    }

    /* FONT FG OPACITY */
    {
        getOpacity (options["textForegroundOpacity"].toString(), &(attribute.charFgOpacity));
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_OPACITY, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_OPACITY, GSW_CC_TYPE_ANALOG);
    }

    /* Font BACKGROUND OPACITY */
    {
        getOpacity (options["textBackgroundOpacity"].toString(), &(attribute.charBgOpacity));
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_BACKGROUND_OPACITY, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_BACKGROUND_OPACITY, GSW_CC_TYPE_ANALOG);
    }

    /* Window OPACITY */
    {
        getOpacity (options["windowFillOpacity"].toString(), &(attribute.winOpacity));
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_WIN_OPACITY, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_WIN_OPACITY, GSW_CC_TYPE_ANALOG);
    }

    /* Text EDGE TYPE */
    {
        getEdgeType (options["textEdgeStyle"].toString(), &(attribute.edgeType));
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_EDGE_TYPE, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_EDGE_TYPE, GSW_CC_TYPE_ANALOG);
    }

    /* Window border TYPE */
    {
        getEdgeType (options["windowBorderEdgeStyle"].toString(), (gsw_CcEdgeType*) &(attribute.borderType));
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_BORDER_TYPE, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_BORDER_TYPE, GSW_CC_TYPE_ANALOG);
    }

    /* FONT STYLE */
    {
        getFontStyle (options["fontStyle"].toString(), &(attribute.fontStyle));
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_STYLE, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_STYLE, GSW_CC_TYPE_ANALOG);
    }

    /* FONT SIZE */
    {
        getFontSize (options["textSize"].toString(), &(attribute.fontSize));
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_SIZE, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_SIZE, GSW_CC_TYPE_ANALOG);
    }

    /* UnHandled part of settings */
    {
//        LOG_INFO("textItalicized = %s", options["textItalicized"].toString().toAscii().constData());
//        LOG_INFO("textUnderline= %s", options["textUnderline"].toString().toAscii().constData());
    }

    /* FONT ITALIC */
    {
        getTextStyle(options["textItalicized"].toString(), &(attribute.fontItalic));
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_ITALIC, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_ITALIC, GSW_CC_TYPE_ANALOG);
    }

    /* FONT UNDERLINE */
    {
        getTextStyle(options["textUnderline"].toString(), &(attribute.fontUnderline));
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_UNDERLINE, GSW_CC_TYPE_DIGITAL);
        ccSetAttributes(&attribute, GSW_CC_ATTRIB_FONT_UNDERLINE, GSW_CC_TYPE_ANALOG);
    }

    return;
}



bool ClosedCaptions::ccEASStarted()
{
    LOG_INFO("ClosedCaptions::ccEASStarted - Entered");
    ccOnEasStart();
    return true;
}

bool ClosedCaptions::ccEASStopped()
{
    LOG_INFO("ClosedCaptions::ccEASStopped - Entered");
    ccOnEasStop();
    return true;
}


/**
 * @fn bool ClosedCaptions::ccParentalLockStart()
 * @brief This function disables the closed caption and stop rendering cc if the service is a parental lock.
 *
 * @return Returns the status of cc stop process.
 */
bool ClosedCaptions::ccParentalLockStart()
{
    LOG_INFO("ClosedCaptions::ccParentalLockStart - Entered");
    m_isParentalBlocked = true;
    return ccStop();
}

bool ClosedCaptions::ccParentalLockStop()
{
    LOG_INFO("ClosedCaptions::ccParentalLockStop - Entered");
    if ((m_isCCEnabled) && (m_isParentalBlocked) && (!m_isCCStopedAtTrickMode))
    {
        ccStart();
        if (m_ccOptions.size() > 0)
        {
            setAttribute(m_ccOptions);
        }
    }
    m_isParentalBlocked = false;
    return true;
}

bool ClosedCaptions::stopCCatTrickMode ()
{ 
    LOG_INFO("ClosedCaptions::stopCCatTrickMode - Entered");
    m_isCCStopedAtTrickMode = true;
    return ccStop();
}

bool ClosedCaptions::startCCatNormalMode ()
{ 
    LOG_INFO("ClosedCaptions::startCCatNormalMode - Entered");
    m_isCCStopedAtTrickMode = false;
    if ((m_isCCEnabled) && (!m_isParentalBlocked) && (!m_isCCStopedAtTrickMode))
    {
        return ccStart();
    }
    return true;
}

void ClosedCaptions::ccGfxPreResolution(unsigned id)
{
    LOG_INFO("ClosedCaptions::ccGfxPreResolution - Entered");
    vlGfxPreResolution(id);
    return;
}


void ClosedCaptions::ccGfxSetResolution(unsigned id, int width, int height)
{
    LOG_INFO("ClosedCaptions::ccGfxSetResolution - Entered");
    vlGfxSetResolution(id, width, height);
    return;
}


void ClosedCaptions::ccGfxPostResolution(int id, int width, int height)
{
    LOG_INFO("ClosedCaptions::ccGfxPostResolution - Entered");
    vlGfxPostResolution(id, width, height);
    return;
}

#endif

