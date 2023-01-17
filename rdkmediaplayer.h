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

/**
 * @defgroup MEDIA_PLAYER rdkmediaplayer
 *
 * - The rdkmediaplayer supports both ip video (hls/dash) and qam.
 * - It's a wayland plugin using the rtRemote API.
 * - Internally uses the AAMP player for IP and RMF player for QAM.
 *
 * @defgroup MEDIA_PLAYER_API  Mediaplayer Data Types and API(s)
 * This file provides the data types and API(s) used by the mediaplayer.
 * @ingroup  MEDIA_PLAYER
 *
 */

/**
 * @addtogroup MEDIA_PLAYER_API
 * @{
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
    rtMethod1ArgAndReturn("open", open, rtString, rtString);
    rtMethod1ArgAndReturn("sendCASData", sendCASData, rtString, rtString);

    RDKMediaPlayer();
    virtual ~RDKMediaPlayer();

    virtual void onInit();

    //rtRemote properties

    /**
     *  @brief This API returns current playback URL.
     *
     *  @param[out] s  current video URL
     *
     *  @return Returns status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError currentURL(rtString& s) const;

    /**
     *  @brief This API performs following functionalities.
     *
     *  - Checks the URL is empty, if not load the URL
     *  - Updates closedcaption state.
     *  - Decides which player to choose AAMP,RMF player or there is need to use cache player.
     *  - Initiates the playback
     *
     *  @param[in] s  current video URL
     *
     *  @return Returns status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setCurrentURL(rtString const& s);

    /**
     *  @brief This API returns the audio language selected.
     *
     *  @param[out] s  Audio Language selected.
     *
     *  @return Returns status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError audioLanguage(rtString& s) const;

    /**
     *  @brief This API is to set the audio language to the player.
     *
     *  @param[in] s  Language to set.
     *
     *  @return Returns status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setAudioLanguage(rtString const& s);

    /**
     *  @brief Function returns the secondary audio language.
     *
     *  @param[out] s  Secondary language selected.
     *
     *  @return Returns status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError secondaryAudioLanguage(rtString& s) const;

    /**
     *  @brief This API is to set the secondary audio language.
     *
     *  @param[in] s  Secondary language to set.
     *
     *  @return Returns status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setSecondaryAudioLanguage(rtString const& s);

    /**
     *  @brief This API returns the current position details from the progress data.
     *
     *  @param[out] t  current position
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError position(float& t) const;

    /**
     *  @brief Moves the playhead to the absolute mediatime specified.
     *
     *  If the value passed is outside of the current valid range, value will be clamped to the valid range.
     *
     *  @param[in] t  position to set.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setPosition(float const& t);

    /**
     *  @brief This API is to return current playback rate.
     *
     *  @param[out] t  current playback rate.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError speed(float& t) const;

    /**
     *  @brief This API is to set the playback rate.
     *
     *  @param[in] t  playback rate.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setSpeed(float const& t);

    /**
     *  @brief This API returns the current volume.
     *
     *  @param[out] t  current volume
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError volume(float& t) const;

    /**
     *  @brief This method sets the volume level for the player instance and is persisted across different URLs.
     *
     *  Value from 0 to 100.
     *
     *  @param[in] t  volume to set.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setVolume(float const& t);

    /**
     *  @brief This API is to check video is blocked or not.
     *
     *  @param[out] t True/False indicates video is blocked or not.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError blocked(rtString& t) const;

    /**
     *  @brief This method will block the video and mute the audio if flag is true.
     *
     *  Otherwise it will unblock the video and restore the audio to the current volume setting.
     *  Any TSB buffering will be unaffected
     *
     *  @param[in] t  Value to enable/disable the blocking.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setBlocked(rtString const& t);

    /**
     *  @brief This API indicates autoplay is enabled or not.
     *
     *  @param[out] t  Value indicates autoplay status.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError autoPlay(rtString& t) const;

    /**
     *  @brief This API sets the auto play flag of the player.
     *
     *  If set to true the player will start playback as soon as possible.
     *  If the value is false user has to start playback.
     *
     *  @param[in] t  Value to enable/disable autoplay.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setAutoPlay(rtString const& t);

    /**
     *  @brief This API is intended to check recording status.
     *
     *  @param[out] t  Recording status
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError isInProgressRecording(rtString& t) const;

    /**
     *  @brief This API is intended to set the recording state.
     *
     *  @param[in] t  Recording state to set.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setIsInProgressRecording(rtString const& t);

    /**
     *  @brief This API returns either FULL" or "NONE".
     *
     *  When FULL, content is stretched to width and height of player.
     *  When NONE, content is best fit to width or height of player without distorting the video's aspect ratio.
     *
     *  @param[out] t  Indicates the value FULL or NONE
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError zoom(rtString& t) const;

    /**
     *  @brief This API sets the value 0 or 1 depending on the argument "FULL" or "NONE".
     *
     *  @param[in] t   Value to set
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setZoom(rtString const& t);

    /**
     *  @brief This API  returns the size of the buffer in milliseconds.
     *
     *  The networkBuffer determines the number of milliseconds of the live stream
     *  that will be cached in the buffer before being consumed by the player.
     *
     *  @param[out] t  Buffer size
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError networkBufferSize(int32_t& t) const;

    /**
     *  @brief The API sets the network buffer size.
     *
     *  @param[in] t  Buffer size in milliseconds.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setNetworkBufferSize(int32_t const& t);

    /**
     *  @brief This API returns the size of the video buffer.
     *
     *  @param[out] t  size of the video buffer
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError videoBufferLength(float& t) const;

    /**
     *  @brief This API is to set the range of buffered video.
     *
     *  @param[in] t  Length to set.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setVideoBufferLength(float const& t);

    /**
     *  @brief This API checks EISS(ETV Integrated Signaling System) filter is enabled or not.
     *
     *  @param[out] t  Indicates the EISS status.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError eissFilterStatus(rtString& t) const;

    /**
     *  @brief This API is to enable/disable the EISS filter.
     *
     *  @param[in] t  Enable/disable value.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setEissFilterStatus(rtString const& t);

    /**
     *  @brief This API returns the start time of the video.
     *
     *  @param[out] t  Start time of the video
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError loadStartTime(int64_t& t) const;

    /**
     *  @brief This API is to set the start time of the video.
     *
     *  @param[in] t  start time to set.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setLoadStartTime(int64_t const& t);

    /**
     *  @brief This API returns the closed caption status.
     *
     *  - False indicates captioning is not enabled (default)
     *  - True  indicates captioning is enabled
     *
     *  @param[out] t  Indicates the closed caption status
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError closedCaptionsEnabled(rtString& t) const;

    /**
     *  @brief This method allows close captioning to be enabled.
     *
     *  Close captioning is disabled by default in the player.
     *
     *  @param[in] t  Parameter to enable/disable closed caption.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setClosedCaptionsEnabled(rtString const& t);

    /**
     *  @brief Returns the list of supported closed caption options supported by this player.
     *
     *  @param[out] t  closed caption options.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError closedCaptionsOptions(rtObjectRef& t) const;

    /**
     *  @brief Sets the close caption options to use when close captioning is enabled.
     *
     *  List of option keys are:
     *   - textSize
     *   - fontStyle
     *   - textEdgeStyle
     *   - textEdgeColor
     *   - textForegroundColor
     *   - textForegroundOpacity
     *   - textItalicized
     *   - textUnderline
     *   - windowFillColor
     *   - windowFillOpacity
     *   - windowBorderEdgeColor
     *   - windowBorderEdgeStyle
     *
     *  @param[in] t  Options to set.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setClosedCaptionsOptions(rtObjectRef const& t);

    /**
     *  @brief Returns the available closed caption tracks for this stream.
     *
     *  @param[out] t  Closed caption tracks supported.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError closedCaptionsTrack(rtString& t) const;

     /**
     *  @brief Sets the caption track to be played when closedCaptionEnabled is true.
     *
     *  @param[in] t  Caption track to set.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setClosedCaptionsTrack(rtString const& t);

    /**
     *  @brief Returns the content options.
     *
     *  Content options are an array of hashes containing key/value pairs that define additional properties for the video resource.
     *  Eg :  contentOptions: {"assetType":"T6_LINEAR","streamId":"6295598249589545163","resumePosition":-1000,
     *  "isContentPosition":false,"mediaGuid":null,"assetEngine":"aamp",
     *  "adConfig":{"type":"clinear","terminalAddress":"20:F1:9E:2D:32:6B"}}
     *
     *  @param[out] t Indicates the content options.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError contentOptions(rtObjectRef& t) const;

    /**
     *  @brief This API is to update the content option.
     *
     *  @param[in] t  Option to update.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setContentOptions(rtObjectRef const& t);

    /**
     *  @brief This API returns the duration of video in milliseconds.
     *
     *  @param[out] t  Total video duration
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError duration(float& t) const;

    /**
     *  @brief This method returns the languages available on the stream.
     *
     *  @param[out] t  Audio languages supported.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError availableAudioLanguages(rtString& t) const;

    /**
     *  @brief This API was supposed to return array of the available captions languages for this video.
     *
     *  @param[out] t  Closed caption languages
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError availableClosedCaptionsLanguages(rtString& t) const;

    /**
     *  @brief This API returns available playback rates for this video.
     *
     *  @param[out] t   playback rates.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError availableSpeeds(rtString& t) const;

    /**
     *  @brief This API returns true, if TSB is enabled for this resource.
     *
     *  Defaults to false.
     *
     *  @param[out] t  TSB enabled/disabled status.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError tsbEnabled(rtString& t) const;

    /**
     *  @brief This API is to enable/disable TSB.
     *
     *  @param[in] t  Value to set.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setTsbEnabled(rtString const& t);

    //rtRemote methods

    /**
     *  @brief API to draw a rectangle.
     *
     *  @param[in] x  X co-ordinate
     *  @param[in] y  Y co-ordinate
     *  @param[in] w  Width of the video
     *  @param[in] h  Height of the video
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setVideoRectangle(int32_t x, int32_t y, int32_t w, int32_t h);

    /**
     *  @brief This API starts video playback.
     *
     *  If the stream is paused the stream will resume playback from the playhead position.
     *  If the stream is not paused the stream will start playback from the beginning of the stream.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError play();

    /**
     *  @brief This API pause the stream and retain the playhead position.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError pause();

    /**
     *  @brief Moves the playhead to the live position in the player.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError seekToLive();
    /**
     *  @brief This API stops the stream and releases unnecessary resources.
     *
     *  The playhead position will be lost and the last video frame should be cleared.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError stop();

    /**
     *  @brief This is an alternative to setting the speed property directly.
     *
     *  The overshoot value is a positive or negative value which expresses the number of milliseconds
     *  which can be used to compensate for overshooting the mark when changing speeds.
     *
     *  @param[in] speed            Speed in milliseconds
     *  @param[in] overshootTime    Time in milliseconds
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError changeSpeed(float speed, int32_t overshootTime);

    /**
     *  @brief Sets the position of the video by adding the given number of seconds.
     *
     *  Seconds may be positive or negative.
     *  But should not cause the position to be less than zero or greater than the duration.
     *
     *  @param[in] seconds Seek time in seconds.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setPositionRelative(float seconds);

    /**
     *  @brief This API requests the onStatus event to be fired.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError requestStatus();

     /**
     *  @brief Sets additional auth data on the player that the player may need to continue playback.
     *
     *  @param[in] t  Key/value pairs required for additional authentication and authorization.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setAdditionalAuth(rtObjectRef const& t);

    /**
     *  @brief  Register an event listener for the specified event name.
     *
     *  @param[in] eventName Events associated with the mediaplayer onMediaOpened, onBuffering, onPlaying etc.
     *  @param[in] f         Listener function associated with the event.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError setListener(rtString eventName, const rtFunctionRef& f);

    /**
     *  @brief This API deregisters  the listeners  associated with the event.
     *
     *  @param[in] eventName Events associated with the mediaplayer onMediaOpened, onBuffering, onPlaying etc.
     *  @param[in] f         Listener function associated with the event.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError delListener(rtString  eventName, const rtFunctionRef& f);

    /**
     *  @brief  Request to destroy the video resource.
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError destroy();

    /**
     *  @brief Open the CAS Management Service Session for CMI.
     *
     *  This is to create a CAS management service session
     *  to start receiving data to/from the CAS System.
     *
     *  @param[in] param    Json string OpenData to CAS
     *  @param[out] resp    Response returns back to CAS Management client
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError open(rtString param, rtString& resp);

    /**
     *  @brief Send the data to CAS System.
     *
     *  This is to pass the data to CAS system to process
     *  the result of this back through EventData asynchronously
     *
     *  @param[in] data    Json string XferInfo to CAS
     *  @param[out] resp    Response returns back to CAS Management client
     *
     *  @return Returns the status of the operation.
     *  @retval RT_OK on success, appropriate error code otherwise.
     */
    rtError sendCASData(rtString data, rtString resp);

    /**
     *  @brief  Returns the blocked status.
     *
     *  @return Returns the status of the operation.
     *  @retval TRUE/FALSE depending upon the status.
     */
    bool getIsBlocked() const;

     /**
     *  @brief  Returns the system monotonic time.
     *
     *  It is the value returned by g_get_monotonic_time()
     *
     *  @return Returns time in microseconds.
     */
    const gint64& getSetURLTime() const;

    /**
     *  @brief  Returns the value set using the function setLoadStartTime().
     *
     *  @return Returns the time in microseconds.
     */
    const gint64& getLoadStartTime() const;

    /**
     *  @brief  Returns the player start time.
     *
     *  @return Returns the start time in microseconds.
     */
    const gint64& getPlayStartTime() const;
    
    /**
     *  @brief This function is used to trigger the events.
     *
     *  @return Returns the reference of event emitter class .
     */
    EventEmitter& getEventEmitter();

    /**
     *  @brief This API receives the decoder handle and updates the closedcaption state.
     *
     *  @param[in] handle Handle to check closedcaption is enabled or not.
     */
    void onVidHandleReceived(uint32_t handle);

    /**
     *  @brief  This API requests for OnMediaOpenedEvent() to be fired and updates the media data like speed,
     *  duration,available speeds, available languages etc.
     *
     *  @param[in] languages   Available AudioLanguages
     *  @param[in] speeds      Available playback rates
     *  @param[in] duration    Duration of the video
     *  @param[in] width       Width of the video
     *  @param[in] height      Heigt of the video
     */
    void updateVideoMetadata(const std::string& languages, const std::string& speeds, int duration, int width, int height);

private:
    rtError startQueuedTune();
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
    bool m_urlQueued;
    bool m_isBlocked;
    bool m_eissFilterStatus;
    bool m_isInProgressRecording;
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
    float m_duration;
    std::string m_availableAudioLanguages;
    std::string m_availableClosedCaptionsLanguages;
    std::string m_availableSpeeds;
};
#endif  // _RT_AAMP_PLAYER_H_

/**
 * @}
 */


