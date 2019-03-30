/*
 * If not stated otherwise in this file or this component's Licenses.txt file the 
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the License);
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
"use strict";
/**
 * Created by chesli200 on 2/21/15.
 */
Object.defineProperty(exports, "__esModule", { value: true });
var document = /** @class */ (function () {
    function document() {
        this.hidden = true;
    };
    document.prototype.addEventListener = function (a,b,c) {
        return null;
    };
    document.prototype.removeEventListener = function (a,b,c) {
        return null;
    };
    document.prototype.createElement = function (a) {
        return null;
    };
    document.prototype.getElementById = function (a) {
        return null;
    };
    document.prototype.getElementsByTagName = function (a) {
        return null;
    };
    document.prototype.createStyleSheet = function (a) {
        return null;
    };
}());
// include css in build
require("../static/css/playerPlatform.css");
// required modules
require("./handlers/RetryHandler");
require("./handlers/PlaybackStalledHandler");
require("./handlers/AnalyticsHandler");
require("./engines/EngineSelector");
require("./engines/null/NullPlayer");
require("./handlers/CrossStreamPreventionHandler");
var constants = require("./PlayerPlatformConstants");
var events = require("./PlayerPlatformAPIEvents");
var Logger_1 = require("./util/Logger");
// exposed modules
var AssetFactory_1 = require("./assets/AssetFactory");
exports.Asset = AssetFactory_1.AssetFactory;
var PlayerPlatformAPI_1 = require("./PlayerPlatformAPI");
exports.PlayerPlatformAPI = PlayerPlatformAPI_1.PlayerPlatformAPI;
var ConfigurationManager_1 = require("./ConfigurationManager");
exports.ConfigurationManager = ConfigurationManager_1.ConfigurationManager;
exports.getConfigurationManager = ConfigurationManager_1.getConfigurationManager;
var VideoAd_1 = require("./ads/VideoAd");
exports.VideoAd = VideoAd_1.VideoAd;
var VideoAdBreak_1 = require("./ads/VideoAdBreak");
exports.VideoAdBreak = VideoAdBreak_1.VideoAdBreak;
var Logger_2 = require("./util/Logger");
exports.LogLevel = Logger_2.LogLevel;
exports.setLogLevel = Logger_2.setLogLevel;
exports.setLogSinks = Logger_2.setLogSinks;
exports.addLogSink = Logger_2.addLogSink;
exports.ConsoleSink = Logger_2.ConsoleSink;
exports.Events = events;
exports.Constants = constants;
//Logger_1.setLogLevel(__loglevel);
//# sourceMappingURL=index.js.map
var XREMain_1 = require("./xre/XREMain");
exports.XREMain = XREMain_1.XREMain;
var PSDKPlayer_1 = require("./engines/psdk/PSDKPlayer");
exports.PSDKPlayer = PSDKPlayer_1.PSDKPlayer;
var AAMPPlayer_1 = require("./engines/aamp/AAMPPlayer");
exports.AAMPPlayer = AAMPPlayer_1.AAMPPlayer;
var ManifestManipulatorAdManager_1 = require("./ads/ManifestManipulator/ManifestManipulatorAdManager");
exports.ManifestManipulatorAdManager = ManifestManipulatorAdManager_1.ManifestManipulatorAdManager;
var CLinearAdManager_1 = require("./ads/scte/CLinearAdManager");
exports.CLinearAdManager = CLinearAdManager_1.CLinearAdManager;
var CustomAdManager_1 = require("./ads/custom/CustomAdManager");
exports.CustomAdManager = CustomAdManager_1.CustomAdManager;

