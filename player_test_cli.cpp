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

/******* This is the test code for rdkmediaplayer *********/

#include <unistd.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <glib.h>
#ifdef ENABLE_BREAKPAD
#include <client/linux/handler/exception_handler.h>
#endif
#include <rtRemote.h>
#include "rdkmediaplayer.h"
#include "logger.h"
#include "glib_tools.h"
#include <string>

GMainLoop *gMainLoop = 0;

#ifdef ENABLE_BREAKPAD
namespace
{

bool breakpadCallback(const google_breakpad::MinidumpDescriptor& /*descriptor*/, void* /*context*/, bool succeeded) {
  rtRemoteShutdown();
  return succeeded;
}

void installExceptionHandler() {
  static google_breakpad::ExceptionHandler* excHandler = NULL;
  delete excHandler;
  excHandler = new google_breakpad::ExceptionHandler(
      google_breakpad::MinidumpDescriptor("/opt/minidumps"),
      NULL,
      breakpadCallback,
      NULL,
      true,
      -1);
}

}  // namespace
#endif

rtError
onEvent(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  // this matches the signature of the server's function
  LOG_TRACE("******** onEvent ********");
  LOG_TRACE("argc:%d", argc);
  LOG_TRACE("argv[0]:%s", argv[0].toString().cString());

          if (argc >= 1) 
          {
            rtObjectRef event = argv[0].toObject();
            rtString eventName = event.get<rtString>("name");
            LOG_TRACE("Received event: %s", eventName.cString());
          }

  (void) result;
  (void) argp;

          if (result)
            *result = rtValue(true);

  LOG_TRACE("******** onEvent - End ********");
  return RT_OK;
}

void* processMessage(void* args)
{
  while(1)
  {
    rtError err = rtRemoteProcessSingleItem();
    usleep(10000);
  }
}

int main(int argc, char *argv[]) {

    int quit = 0; /* 1 == quit loop */
     int c = 0;
    char lang;
    rtString url_out;
    log_init();
    LOG_INFO("Init rtRemote");
    rtError rc;
    rc = rtRemoteInit();
    pthread_t processMsgThread;
    ASSERT(rc == RT_OK);
    const char opendata[256] = "{\"mediaurl\": \"tune://frequency=306000000&modulation=16&symbol_rate=6900000&pgmno=1001&\",\"mode\": \"MODE_LIVE\",\"manage\": \"MANAGE_NONE\",\"casinitdata\": \"<base64 data>\",\"casocdmid\": \"\"}";
    const char opendata1[256] = "{\"mediaurl\": \"tune://frequency=322000000&modulation=16&symbol_rate=6900000&pgmno=1&\",\"mode\": \"MODE_LIVE\",\"manage\": \"MANAGE_NONE\",\"casinitdata\": \"<base64 data>\",\"casocdmid\": \"\"}";
    bool channel1 = 0;

    const char* objectName = getenv("PX_WAYLAND_CLIENT_REMOTE_OBJECT_NAME");
    if (!objectName) objectName = "rdkmediaplayer";
    LOG_INFO("Locate: %s", objectName);

    rtObjectRef playerRef;
    rc = rtRemoteLocateObject(objectName, playerRef);
    pthread_create(&processMsgThread, NULL, processMessage, NULL);
    ASSERT(rc == RT_OK);
    LOG_INFO("found object : %s", objectName);
    LOG_INFO("Start main loop");

    rtString supSpeeds;
    rc  = playerRef.get("availableSpeeds", supSpeeds);
    if (rc != RT_OK)
    {
        LOG_INFO("failed to get availableSpeeds: %d", rc);
    }
    else
    {
        LOG_INFO("supSpeeds = %s", supSpeeds.cString());
    }

    if(argc <= 1)
    {
       printf("URI was NULL.  Use the default json data with url");
       rc = playerRef.send("open", rtString(opendata));
    }
    else if(strchr(argv[1], '{') != NULL)
    {
       printf("URI contains json data");
       rc = playerRef.send("open", rtString(argv[1]));
    }
    else
    {
       printf( "URL is: %s\n", argv[1] );
       rc = playerRef.set("url", rtString(argv[1]));
    }

    if (rc != RT_OK)
    {
        LOG_INFO("failed to set URL: %d", rc);
    }
    else
    {
        playerRef.send("on","onMediaOpened",new rtFunctionCallback(onEvent));
        playerRef.send("on","onProgress",new rtFunctionCallback(onEvent));
        playerRef.send("on","onStatus",new rtFunctionCallback(onEvent));
        playerRef.send("on","onWarning",new rtFunctionCallback(onEvent));
        playerRef.send("on","onError",new rtFunctionCallback(onEvent));
        playerRef.send("on","onSpeedChange",new rtFunctionCallback(onEvent));
        playerRef.send("on","onClosed",new rtFunctionCallback(onEvent));
        playerRef.send("on","onPlayerInitialized",new rtFunctionCallback(onEvent));
        playerRef.send("on","onBuffering",new rtFunctionCallback(onEvent));
        playerRef.send("on","onPlaying",new rtFunctionCallback(onEvent));
        playerRef.send("on","onPaused",new rtFunctionCallback(onEvent));
        playerRef.send("on","onComplete",new rtFunctionCallback(onEvent));
        playerRef.send("on","onIndividualizing",new rtFunctionCallback(onEvent));
        playerRef.send("on","onAcquiringLicense",new rtFunctionCallback(onEvent));
        playerRef.send("on","onDRMMetadata",new rtFunctionCallback(onEvent));
        playerRef.send("on","onSegmentStarted",new rtFunctionCallback(onEvent));
        playerRef.send("on","onSegmentCompleted",new rtFunctionCallback(onEvent));
        playerRef.send("on","onSegmentWatched",new rtFunctionCallback(onEvent));
        playerRef.send("on","onBufferWarning",new rtFunctionCallback(onEvent));
        playerRef.send("on","onPlaybackSpeedsChanged",new rtFunctionCallback(onEvent));
        playerRef.send("on","onAdditionalAuthRequired",new rtFunctionCallback(onEvent));

        LOG_INFO("URL is set.");
        url_out = playerRef.get<rtString>("url");
        LOG_INFO("get url value : %s", url_out.cString());

        rc = playerRef.send("play");
        if (rc != RT_OK)
        {
          LOG_INFO("failed to set URL to play: %d", rc);
        }
        LOG_INFO("Play the url %s, result: %d", url_out.cString(), rc);
    }

  /* loop until user wants to quick playback */
  while( !quit )
    {
        printf("Playback Options:\n");
        printf("p - Pause Playback\n");
        printf("r - Resume Playback\n");
        printf("s - Stop Playback\n");
        printf("l - Set Audio Language\n");
        printf("t - Set  speed\n");
        printf("c - Enable Subt.\n");
        printf("z - Channel Change\n");
        printf("q - Quit\n");
        printf("\n");

         c = getchar();
         switch (c)
        {
            case 'p':
                /* Pause playback */
                rc = playerRef.send("pause");
                if (rc != RT_OK)
                {
                    LOG_INFO("failed to set URL to PAUSE: %d", rc);
                }
                LOG_INFO("PAUSE the url %s, result: %d", url_out.cString(), rc);
                break;
            case 'r':
                /* resume playback */
                rc = playerRef.send("play");
                if (rc != RT_OK)
                {
                   LOG_INFO("failed to set URL play: %d", rc);
                }
               LOG_INFO("Play the url %s, result: %d", url_out.cString(), rc);
               break;
            case 's':
                 /* stop playback */
                 rc = playerRef.send("stop");
                 if (rc != RT_OK)
                 {
                     LOG_INFO("failed to set URL play: %d", rc);
                 }
                 LOG_INFO("stop the url %s, result: %d", url_out.cString(), rc);
                 break;
            case 'l':
                printf("e - To Set English Language\n");
                printf("d - To Set deustch Language\n");
                scanf(" %c", &lang);
                switch(lang)
                {
                    case 'e':
                        LOG_INFO("Set English language");
                        rc = playerRef.set("audioLanguage", "eng");
                        if (rc != RT_OK)
                        {
                            LOG_INFO("failed to set English Language: %d", rc);
                        }
                        break;
                    case 'd':
                        LOG_INFO("Set Deustch language");
                        rc = playerRef.set("audioLanguage", "deu");
                        if (rc != RT_OK)
                        {
                            LOG_INFO("failed to set Deustch Language: %d", rc);
                        }
                        break;
                    default:
                        break;
                }
                break;
            case 't':
                LOG_INFO("feature not implemeted");
                break;
            case 'c':
                LOG_INFO("feature not implemeted");
                break;
            case 'z':
                /* Stop and Zap channels */
                rc = playerRef.send("stop");
                if (rc != RT_OK)
                {
                    LOG_INFO("failed to set URL play: %d", rc);
                }
                LOG_INFO("stop the url %s, result: %d", url_out.cString(), rc);

                if(channel1)
                {
                    channel1 = 0;
                    printf("[CC] URI contains json data = %s\n", opendata);
                    rc = playerRef.send("open", rtString(opendata));
                }
                else{
                    channel1 = 1;
                    printf("[CC] URI contains json data = %s\n", opendata1);
                    rc = playerRef.send("open", rtString(opendata1));
                }
                sleep(1);
                url_out = playerRef.get<rtString>("url");
                LOG_INFO("get url value : %s", url_out.cString());
                /* Start playback */
                rc = playerRef.send("play");
                if (rc != RT_OK)
                {
                    LOG_INFO("failed to set URL play: %d", rc);
                }
                LOG_INFO("Play the url %s, result: %d", url_out.cString(), rc);
                break;
            case 'q':
                quit = 1;
                break;
            default:
                break;
        }
    }

    gMainLoop = 0;

    playerRef = nullptr;

    LOG_INFO("Shutdown");
    rc = rtRemoteShutdown();
    ASSERT(rc == RT_OK);

    return 0;
}
