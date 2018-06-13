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
var stuff = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/frameworks/";
px.configImport({"stuff:":stuff});
px.import({scene : 'px:scene.1.js',
           keys: 'px:tools.keys.js'
}).then( function importsAreReady(imports) {

var showDebugUI = true;
var handleRemote = true;

var streams = [ 
    ["IPDVR1",  "https://xtvapi.cloudtv.comcast.net/recording/V6184936945826443811L200584520123010018/stream.m3u8"],
    ["OCAP 1",  "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3b30"],
    ["OCAP 2",  "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3b30"],
    ["IPLIN1",  "http://ccr.linear-nat-pil-red.xcr.comcast.net/DSCHD_HD_NAT_16122_0_5965163366898931163-eac3.m3u8"],
    ["OCAP 3",  "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3b30"],
    ["OCAP 4",  "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3b30"],
    ["IPDVR2",  "http://ccr.cdvr-vin2.xcr.comcast.net/V6184936945826443811L200644901013040017.m3u8"],
    ["IPLIN2",  "http://ccr.linear-nat-pil-red.xcr.comcast.net/BBCAM_HD_NAT_18399_0_8519783297380464163.m3u8"],
    ["OCAP 5",  "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3b30"],
    ["IPEAS",   "http://edge.ip-eas-dns.xcr.comcast.net/ZCZC-PEP-NPT-000000-0030-2701820-/en-US.m3u8"]
];
 
var scene = imports.scene;
var keys = imports.keys;
var splash = null;
var controls = null;
var logpanel = null;
var waylandObj = null;
var player = null;
var autoTune = true;
var audioLanguages = null;
var currentSpeed = 1.0;
var isTuned = false;

module.exports.wantsClearscreen = function()
{
    return false;
};

function log(msg)
{
	console.log("rdkmediaplayer.js: " + msg);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//UI CONTROLS
function centerText(textObj)
{
    var size = textObj.font.measureText(textObj.pixelSize, textObj.text);
    textObj.x = (textObj.parent.w - size.w) / 2;
    textObj.y = (textObj.parent.h - size.h) / 2;
}

function ControlButton(inTb, inId, inX, inY, inW, inH, inAction, text, inURL)
{
	this.tb = inTb;
    this.id = inId;
	this.focused = scene.create({t: "rect",
								x: inX,
								y: inY,
								w: inW,
								h: inH,
								parent: inTb.background,
								draw: false,
								fillColor:0xFFFFFFFF});
	this.unfocused = scene.create({t: "rect",
								x: inX,
								y: inY,
								w: inW,
								h: inH,
								parent: inTb.background,
								draw: true,
								fillColor:0x555555FF});
	this.focText = scene.create({t: "text",
								x: 0,
								y: 0,
								w: inW,
								h: inH,
								parent: this.focused,
                                textColor:0x000000FF,
								text: text });
    this.focText.ready.then(centerText);
	this.unfocText = scene.create({t: "text",
								x: 0,
								y: 0,
								w: inW,
								h: inH,
								parent: this.unfocused,
								text: text });
    this.unfocText.ready.then(centerText);
	this.isFocused = false;
	this.isVisible = true;
	this.left = null;
	this.right = null;
	this.action = inAction
    this.url = inURL;
}

ControlButton.prototype.updateState = function()
{
	if(this.isVisible)
	{
		if(this.isFocused)
		{
			this.focused.draw = true;
			this.unfocused.draw = false;
		}
		else
		{
			this.focused.draw = false;
			this.unfocused.draw = true;
		}
	}
	else
	{
		this.focused.draw = false;
		this.unfocused.draw = false;
	}
}

ControlButton.prototype.setFocused = function(isFocused)
{
	this.isFocused = isFocused
	this.updateState();
}

ControlButton.prototype.setVisible = function(isVisible)
{
	this.isVisible = isVisible
	this.updateState();
}

function ControlPanel()
{
    var PH = 150;//panel height
    var RH = PH/3;//row height

	this.background = scene.create({t: "rect",
								    x: 0,
								    y: scene.h - PH,
								    w: scene.w,
								    h: PH,
								    parent: scene.root,
								    fillColor:0x000000AA });
    var W = 100;//button width
    var H = 30;//button height
    var S = 10;//space between button
	var X = scene.w/2.0 - (W*5) - (S*4.5);
	var Y = RH*0.5 - H/2;
    var D = W+S;

    this.divider0 = scene.create({t: "rect",
							    x: 0,
							    y: 0,
							    w: scene.w,
							    h: 2,
							    parent: this.background,
							    fillColor:0xFFFFFF55 }); 

    this.title0 = scene.create({t: "text",
							    x: 10,
							    y: 5,
							    parent: this.background,
                                pixelSize: 14,
							    text:"Info" }); 

    this.streamName = scene.create({t: "text",
							    x: X,
							    y: Y,
							    parent: this.background,
							    text:"" }); 

    this.streamPosition = scene.create({t: "text",
							    x: X+300,
							    y: Y,
							    parent: this.background,
							    text:"" }); 

	Y = RH*1.5 - H/2;

    this.divider1 = scene.create({t: "rect",
							    x: 10,
							    y: RH,
							    w: scene.w-20,
							    h: 2,
							    parent: this.background,
							    fillColor:0xFFFFFF33 }); 
    this.title1 = scene.create({t: "text",
							    x: 10,
							    y: RH+5,
							    parent: this.background,
                                pixelSize: 14,
							    text:"Controls" }); 


    this.buttons = [
        new ControlButton(this, 0, X+D*0, Y, W, H, doPause, "PAUSE", null),
        new ControlButton(this, 1, X+D*1, Y, W, H, doPlay, "PLAY", null),
        new ControlButton(this, 2, X+D*2, Y, W, H, doRestart, "RESTART",null),
        new ControlButton(this, 3, X+D*3, Y, W, H, doLive, "LIVE",null),
        new ControlButton(this, 4, X+D*4, Y, W, H, doREW, "REW", null),
        new ControlButton(this, 5, X+D*5, Y, W, H, doFFW, "FFW", null),
        new ControlButton(this, 6, X+D*6, Y, W, H, doSeekBk, "SEEK-", null),
        new ControlButton(this, 7, X+D*7, Y, W, H, doSeekFr, "SEEK+",null),
        new ControlButton(this, 8, X+D*8, Y, W, H, doCC, "CC", null),
        new ControlButton(this, 9, X+D*9, Y, W, H, doSap, "SAP",null),
    ];
    var len = this.buttons.length;
    for (i = 0; i < len-1; i++)
    {
        this.buttons[i].right = this.buttons[i+1];
        this.buttons[i+1].left = this.buttons[i];
    }
    this.buttons[0].left = this.buttons[len-1];
    this.buttons[len-1].right = this.buttons[0];

	Y = RH*2.5 - H/2;

    this.channels = [];
    for(i = 0; i < streams.length; i++)
    {
        this.channels.push( new ControlButton(this, 10+i, X+D*i, Y, W, H,  loadStream, streams[i][0], streams[i][1]) );
    }
    len = this.channels.length;
    for (i = 0; i < len-1; i++)
    {
        this.channels[i].right = this.channels[i+1];
        this.channels[i+1].left = this.channels[i];
    }
    if(len > 0)
    {
        this.channels[0].left = this.channels[len-1];
        this.channels[len-1].right = this.channels[0];
    }

    this.divider2 = scene.create({t: "rect",
							    x: 10,
							    y: RH*2,
							    w: scene.w-20,
							    h: 2,
							    parent: this.background,
							    fillColor:0xFFFFFF33 }); 
    this.title2 = scene.create({t: "text",
							    x: 10,
							    y: RH*2+5,
							    parent: this.background,
                                pixelSize: 14,
							    text:"Streams" }); 
	currentSpeed = 1.0;
	this.supportedSpeeds = [-64.0, -32.0, -16.0, -4.0, -1.0, 0.0, 1.0, 4.0, 16.0, 32.0, 64.0];
	this.focused = true;
	this.focusedButton 	= this.buttons[0];
	this.updateState();
}

ControlPanel.prototype.updateState = function()
{
	this.focusedButton.setFocused(true);
	this.background.draw = this.focused;
    logpanel.setVisible(this.focused);
}

ControlPanel.prototype.navLeft = function()
{
	if(this.focusedButton.left)
	{
		this.focusedButton.setFocused(false);
		this.focusedButton = this.focusedButton.left;
		this.focusedButton.setFocused(true);
	}
}

ControlPanel.prototype.navRight = function()
{
	if(this.focusedButton.right)
	{
		this.focusedButton.setFocused(false);
		this.focusedButton = this.focusedButton.right;
		this.focusedButton.setFocused(true);
	}
}

ControlPanel.prototype.navUp = function()
{
    if(this.focusedButton.id >= 10)
    {
		this.focusedButton.setFocused(false);
		this.focusedButton = this.buttons[this.focusedButton.id-10];
		this.focusedButton.setFocused(true);
    }
    else
    {
        this.focused = false;
	    this.background.draw = false;
        logpanel.setVisible(false);
    }
}

ControlPanel.prototype.navDown = function()
{
    if(!this.focused)
    {
		controls.focused = true;
		controls.background.draw = true;
        logpanel.setVisible(true);
    }
    else if(this.focusedButton.id < 10 && this.channels.length > 0)
    {
		this.focusedButton.setFocused(false);
        if(this.focusedButton.id > this.channels.length-1)
       		this.focusedButton = this.channels[this.channels.length-1];
        else
    		this.focusedButton = this.channels[this.focusedButton.id];
		this.focusedButton.setFocused(true);
    }
}

ControlPanel.prototype.doAction = function()
{
	this.focusedButton.action.call();
}

ControlPanel.prototype.loadStream = function()
{
	log("loadStream");
    var id = this.focusedButton.id;
    if(id >= 10)
        id -= 10;
    else
        id = 0;

    this.streamName.text = this.channels[id].focText.text + ": " + this.channels[id].url;
    
    var nameSize = this.streamName.font.measureText(this.streamName.pixelSize, this.streamName.text);
    this.streamPosition.x = this.streamName.x +nameSize.w + 50;
    this.streamPosition.text = "Position 0 of 0";
    
    logpanel.clear();
    splash.setVisible(true);

    doLoad(this.channels[id].url);
}

ControlPanel.prototype.updateTime = function(position, duration)
{
    //log("updateTime: " + t);
    this.streamPosition.text = "Position " + position.toFixed(2) + " of " + duration.toFixed(2);
}

function loadStream()
{
    controls.loadStream();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//LOG PANEL

function LogPanel()
{
    var PW = 200; //panel width
	this.background = scene.create({t: "rect",
						            x: scene.w-PW,
						            y: 0,
						            w: PW,
						            h: 0,
						            parent: scene.root,
						            fillColor:0x000000AA });

    this.textbox = scene.create({t:"textBox",
							    x: 10,
							    y: 0,
							    w: this.background.w-10,
							    h: 720,
							    parent: this.background,
                                pixelSize: 12,
                                wordWrap:true,
                                truncation:scene.truncation.TRUNCATE_AT_WORD, 
                                ellipsis:true });
}

LogPanel.prototype.setVisible = function(visible)
{
    this.background.draw = visible;
}

LogPanel.prototype.clear = function()
{
    this.textbox.text = "";
}

LogPanel.prototype.log = function(message)
{
    log(message);
    this.textbox.text = this.textbox.text +  message + "\n";
    var size = this.textbox.font.measureText(this.textbox.pixelSize, this.textbox.text);
    this.background.h = size.h;
    //do some rudimentary scrolling
    while(this.background.h > controls.background.y)
    {
        var text = this.textbox.text;
        var i = text.indexOf("\n");
        if(i >= 0)
        {
            text = text.substr(i+1, text.length - i+1);
            this.textbox.text = text;
            var size = this.textbox.font.measureText(this.textbox.pixelSize, this.textbox.text);
            this.background.h = size.h;
        }
        else
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//PLAYER STUFF

function onEvent(e)
{
    if(showDebugUI)
        logpanel.log("Event " + e.name);
}

function onMediaOpened(e)
{
    if(showDebugUI)
        logpanel.log("Event " + e.name 
            + "\n   type:" + e.mediaType
            + "\n   width:" + e.width
            + "\n   height:" + e.height
            + "\n   speed:" + e.availableSpeeds
            + "\n   sap:" + e.availableAudioLanguages
            + "\n   cc:" + e.availableClosedCaptionsLanguages 
            //+ "\n   customProperties:" + e.customProperties
            //+ "\n   mediaSegments:" + e.mediaSegments
        );

    audioLanguages = e.availableAudioLanguages.split(",");
}

function onProgress(e)
{
    if(showDebugUI)
    {
        /*
        logpanel.log("Event " + e.name 
            + "\n   position:" + e.position 
            + "\n   duration:" + e.duration
            + "\n   speed:" + e.speed
            + "\n   start:" + e.start
            + "\n   end:" + e.end
        );
        */
        if(!isTuned)
        {
            isTuned = true;
            splash.setVisible(false);
            logpanel.log("Tune Complete");
        }
        controls.updateTime(e.position, e.duration);
    }
}

function onStatus(e)
{
    if(showDebugUI)
        logpanel.log("Event " + e.name 
            + "\n   position:" + e.position 
            + "\n   duration:" + e.duration
            + "\n   width:" + e.width 
            + "\n   height:" + e.height 
            + "\n   isLive:" + e.isLive 
            + "\n   isBuffering:" + e.isBuffering 
            + "\n   connectionURL:" + e.connectionURL 
            + "\n   bufferPercentage:" + e.bufferPercentage
            + "\n   dynamicProps:" + e.dynamicProps
            + "\n   netStreamInfo:" + e.netStreamInfo  
    );
}

function onWarningOrError(e)
{
    if(showDebugUI)
        logpanel.log("Event " + e.name + "\n   code:" + e.code + "\n   desc:" + e.description);
}

function onSpeedChange(e)
{
    if(showDebugUI)
        logpanel.log("Event " + e.name + "\n   speed:" + e.speed);
}

function registerPlayerEvents()
{
    player.on("onMediaOpened",onMediaOpened);
    player.on("onProgress",onProgress);
    player.on("onStatus",onStatus);
    player.on("onWarning",onWarningOrError);
    player.on("onError",onWarningOrError);
    player.on("onSpeedChange",onSpeedChange);
    player.on("onClosed",onEvent);
    player.on("onPlayerInitialized",onEvent);
    player.on("onBuffering",onEvent);
    player.on("onPlaying",onEvent);
    player.on("onPaused",onEvent);
    player.on("onComplete",onEvent);
    player.on("onIndividualizing",onEvent);
    player.on("onAcquiringLicense",onEvent);
    player.on("onDRMMetadata",onEvent);
    player.on("onSegmentStarted",onEvent);
    player.on("onSegmentCompleted",onEvent);
    player.on("onSegmentWatched",onEvent);
    player.on("onBufferWarning",onEvent);
    player.on("onPlaybackSpeedsChanged",onEvent);
    player.on("onAdditionalAuthRequired",onEvent);
}

function unRegisterPlayerEvents()
{
    player.delListener("onMediaOpened",onMediaOpened);
    player.delListener("onClosed",onClosed);
    player.delListener("onPlayerInitialized",onPlayerInitialized);
    player.delListener("onBuffering",onBuffering);
    player.delListener("onPlaying",onPlaying);
    player.delListener("onPaused",onPaused);
    player.delListener("onComplete",onComplete);
    player.delListener("onIndividualizing",onIndividualizing);
    player.delListener("onAcquiringLicense",onAcquiringLicense);
    player.delListener("onProgress",onProgress);
    player.delListener("onWarning",onWarning);
    player.delListener("onError",onError);
    player.delListener("onSpeedChange",onSpeedChange);
    player.delListener("onDRMMetadata",onDRMMetadata);
    player.delListener("onSegmentStarted",onSegmentStarted);
    player.delListener("onSegmentCompleted",onSegmentCompleted);
    player.delListener("onSegmentWatched",onSegmentWatched);
    player.delListener("onBufferWarning",onBufferWarning);
    player.delListener("onPlaybackSpeedsChanged",onPlaybackSpeedsChanged);
    player.delListener("onAdditionalAuthRequired",onAdditionalAuthRequired);
}

function hackToMakeEventsFlow1()
{
    setInterval(hackToMakeEventsFlow2, 250);
}

function hackToMakeEventsFlow2()
{
    player.volume;
}

function doLoad(url)
{
    log("doLoad");
    isTuned = false;
	player.url = url;
	player.setVideoRectangle(0,0,scene.w,scene.h);
}

function doPause()
{
    log("doPause");
	currentSpeed = 0.0;
	player.pause();
    if(showDebugUI)
        controls.updateState();
}

function doPlay()
{
    log("doPlay");
	currentSpeed = 1.0;
	player.play();
    if(showDebugUI)
        controls.updateState();
}

function doFFW()
{
	log("doFFW");

	if(currentSpeed < 0)
		currentSpeed = 4;
	else
	if(currentSpeed == 0)
	{
        currentSpeed = 1;
		player.play();
		return;
	}
	else
	if(currentSpeed == 1)
		currentSpeed = 4;
	else
	if(currentSpeed == 4)
		currentSpeed = 16;
	else
	if(currentSpeed == 16)
		currentSpeed = 32;
	else
	if(currentSpeed == 32)
		currentSpeed = 64;
	else
		return;

	player.speed = currentSpeed;

    if(showDebugUI)
        controls.updateState();
}

function doREW()
{
	log("doREW");

	if(currentSpeed > -4)
		currentSpeed = -4;
	else
	if(currentSpeed == -4)
		currentSpeed = -16;
	else
	if(currentSpeed == -16)
		currentSpeed = -32;
	else
	if(currentSpeed == -32)
		currentSpeed = -64;
	else
		return;

	player.speed = currentSpeed;

    if(showDebugUI)
        controls.updateState();
}

function doRestart()
{
	log("doRestart");
	player.position = 0;
    if(showDebugUI)
        controls.updateState();
}

function doLive()
{
	log("doLive");
	player.seekToLive();
    if(showDebugUI)
        controls.updateState();
}

function doSeekFr()
{
	log("doSeekFr");
    player.position = player.position + 60000.0;
}

function doSeekBk()
{
	log("doSeekBk");
    player.position = player.position - 60000.0;
}

function doSap()
{
	log("doSap");
    if(audioLanguages == null || audioLanguages.length < 2)
        return;
    if(player.audioLanguage == audioLanguages[0])
        player.audioLanguage = audioLanguages[1];
    else
        player.audioLanguage = audioLanguages[0];
    if(showDebugUI)
        controls.updateState();
}

function doCC()
{
	log("doCC");
    if(player.closedCaptionsEnabled == "true")
        player.closedCaptionsEnabled = "false";
    else
        player.closedCaptionsEnabled = "true";
    if(showDebugUI)
        controls.updateState();
}

function handleWaylandRemoteSuccess(wayland)
{
	log("Handle wayland success");
	waylandObj = wayland;
	waylandObj.moveToBack();
	player = wayland.api;
    registerPlayerEvents();
/*
    player.closedCaptionsOptions = { 
        textSize:             "textSizeVal",
        fontStyle:            "fontStyleVal",
        textForegroundColor:  "textForegroundColorVal",
        textForegroundOpacity:"textForegroundOpacityVal",
        textBackgroundColor:  "textBackgroundColorVal",
        textBackgroundOpacity:"textBackgroundOpacityVal",
        textItalicized:       "textItalicizedVal",
        textUnderline:        "textUnderlineVal",
        windowFillColor:      "windowFillColorVal",
        windowFillOpacity:    "windowFillOpacityVal",
        windowBorderEdgeColor:"windowBorderEdgeColorVal",
        windowBorderEdgeStyle:"windowBorderEdgeStyleVal",
        textEdgeColor:        "textEdgeColorVal",
        textEdgeStyle:        "textEdgeStyleVal",
        fontSize:             "fontSizeVal" 
        };
*/
    if(streams.length > 0)
    {
        if(showDebugUI)
            loadStream();
        else
            doLoad(streams[0][1]);
    }

    setTimeout(hackToMakeEventsFlow1, 1000);
}

function handleWaylandRemoteError(error)
{
	log("Handle wayland error");
    //delete process.env.PLAYERSINKBIN_USE_WESTEROSSINK;
    if(showDebugUI)
    	splash.setVisible(false);
}

if(showDebugUI || handleRemote)
{
    scene.root.on("onKeyDown", function (e) 
    {
	    var code  = e.keyCode;
	    var flags = e.flags;

	    log("onKeyDown " + code);

        if(showDebugUI)
        {
	        if(code == keys.ENTER || code == 0)
	        {
		        if(controls.focused)
		        {
			        controls.doAction();
		        }
	        }
	        else if(code == keys.UP)
	        {
		        if(controls.focused)
			        controls.navUp();
	        }
	        else if(code == keys.DOWN)
	        {
		        controls.navDown();
	        }
	        else if(code == keys.LEFT)
	        {
		        if(controls.focused)
			        controls.navLeft();
	        }
	        else if(code == keys.RIGHT)
	        {
		        if(controls.focused)
			        controls.navRight();
	        }
        }

        if(handleRemote)
        {
	        if(code == keys.F)
	        {
		        doFFW();
	        }
	        else if(code == keys.W)
	        {
		        doREW();
	        }
	        else if(code == keys.P)
	        {
                if(currentSpeed == 1)
                    doPause();
                else
                    doPlay();
	        }
	        else if(code == keys.PAGEUP)
	        {
		        doSeekFr();
	        }
	        else if(code == keys.PAGEDOWN)
	        {
		        doSeekBk();
	        }
        }
    });
}

function SplashScreen()
{
	this.background = scene.create({ t: "rect",
								x: 0,
								y: 0,
								w: scene.w,
								h: scene.h,
								parent: scene.root,
								fillColor:0x000000FF});
	this.text = scene.create({ t: "text",
								x: scene.w/2-100,
								y: scene.h/2-20,
								w: 0,
								h: 0,
								parent: this.background,
								pixelSize: 50,
								text: "Loading"});
    this.text.ready.then(centerText);

}

SplashScreen.prototype.setVisible = function(visible)
{
	this.background.draw = visible;
	this.text.draw = visible;
}

if(showDebugUI)
{
    splash = new SplashScreen();
    logpanel = new LogPanel();
    controls = new ControlPanel();
}

if(autoTune)
{
	waylandObj = scene.create( {t:"wayland", x:0, y:0, w:1280, h:720, parent:scene.root,cmd:"rdkmediaplayer"} );
	waylandObj.remoteReady.then(handleWaylandRemoteSuccess, handleWaylandRemoteError);
	waylandObj.focus = true;
	waylandObj.moveToBack();
}
else
{
    if(showDebugUI)
	    splash.setVisible(false);
}

if(showDebugUI)
    controls.background.moveToFront();

}).catch( function importFailed(err){
  console.error("Import failed for rdkmediaplayer.js: " + err)
});
