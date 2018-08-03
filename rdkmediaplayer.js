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

var testsEnabled = true;
var controlsEnabled = false;
var splashEnabled = false;
var logsEnabled = true;
var remoteEnabled = false;
var autoTuneEnabled = false;
var blockTune = false;

//when testsEnabled a channel change test will do MAX_CHANNEL_CHANGES tunes with a random delay between each tune of 0-MAX_TUNE_TIME miliseconds
var MAX_CHANNEL_CHANGES = 100;
var MAX_TUNE_TIME = 10000;

var IP_VOD=0;
var IP_CDVR=1;
var IP_LINEAR=2;
var QAM_LINEAR=3;//qam test will only be enabled on pacexg1v3

var streams = [
    //IP VOD
    [
        "http://ccr.col-jitp2.xcr.comcast.net/omg11/6/633/Equalizer_HD_VOD_R3_AUTH_movie_LVLH06.m3u8",
        "http://ccr.col-jitp2.xcr.comcast.net/omg01/129/205/180710_3758260_Harry_Potter_and_the_Sorcerer_s_Stone_LVLH05.m3u8",
        "http://ccr.col-jitp2.xcr.comcast.net/omg09/805/285/Deadpool_HD_VOD_AUTH_movie_LVLH01.m3u8",
        "http://ccr.col-jitp2.xcr.comcast.net/omg16/715/281/Sicario_HD_VOD_AUTH_movie_LVLH01.m3u8",
        "http://ccr.col-jitp2.xcr.comcast.net/omg16/719/773/TVN_TNTD1005081700016524_HD_LVLH01.m3u8",
        "http://ccr.ipvod-t6.xcr.comcast.net/ipvod1/XPPK0001527933029001/movie/1531279737539/manifest.m3u8",
        "http://ccr.col-jitp2.xcr.comcast.net/omg09/505/757/TVN_TBSD1005221800041819_SD_LVLS02.m3u8",
        "http://ccr.col-jitp2.xcr.comcast.net/omg01/404/529/TVN_TNTD1005241800030546_R_SD_LVLS03.m3u8",
        "http://ccr.ipvod-t6.xcr.comcast.net/ipvod1/XPPK0001522490182001/movie/1526680520754/manifest.m3u8"
    ],
    //IP CDVR
    [ 
        "http://ccr.cdvr-rio-dnly-atl.xcr.comcast.net/V6184936945826443811L200584520724000018.m3u8",
        "http://ccr.cdvr-rio-dnly-atl.xcr.comcast.net/V6184936945826443811L200586230725010018.m3u8",
        "http://ccr.cdvr-rio-dnly-atl.xcr.comcast.net/V6184936945826443811L200591860724072418.m3u8",
        "http://ccr.cdvr-rio-dnly-atl.xcr.comcast.net/V6184936945826443811L200600480720230018.m3u8",
        "http://ccr.cdvr-rio-dnly-atl.xcr.comcast.net/V6184936945826443811L200569050720000018.m3u8",
        "http://ccr.cdvr-rio-dnly-atl.xcr.comcast.net/V6184936945826443811L200618120719010018.m3u8",
        "http://ccr.cdvr-rio-dnly-atl.xcr.comcast.net/V6184936945826443811L200644900511020018.m3u8",
        "http://ccr.cdvr-rio-dnly-atl.xcr.comcast.net/V6184936945826443811L200204300507013018.m3u8"
    ],
    //IP LINEAR
    [ 
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/BBCAM_HD_NAT_18399_0_8519783297380464163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/WAGAD_HD_VINdGA_11276_0_6859364719562080163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WSBHD_HD_VINdGA_11538_0_5906224414376517163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/BTVHD_HD_NAT_23449_0_8580316105738446163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WXIAD_HD_VINdGA_11270_0_6108877978320289163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/TBSHD_HD_VINdGA_12817_1_5306027669102239163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/GPBDT_HD_VINdGA_16822_0_6702778145056519163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/WGCLD_HD_VINdGA_11272_0_8926612883165527163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/WUPAH_HD_VINdGA_12482_0_5091189193116757163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/HSNHD_HD_NAT_18592_0_6306128565430219163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/WPXAD_HD_VINdGA_18260_0_5676530272632454163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/WATLD_HD_VINdGA_12295_0_7357420591058259163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/UNIAT_HD_VINdGA_18870_0_7478923966817733163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/WGNAD_HD_NAT_16578_0_6962262367300850163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/WPBAD_HD_VINdGA_13035_0_6552011402491408163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/WE_HD_HD_NAT_16405_0_5054610152770910163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/DESTD_HD_NAT_16817_0_4680410477165362163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/FYIHD_HD_NAT_16430_0_5024523389765867163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/QVCHD_HD_NAT_16762_0_7397413520572334163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/ESWHD_HD_NAT_16675_0_8106614643628609163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/CBSSD_HD_NAT_16852_0_7308451098232889163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/MLBHD_HD_VINdGA_17535_1_6648676818503987163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/BETD_HD_NAT_18170_0_5521303082767577163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/CMTHD_HD_NAT_17303_0_8489055564075118163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/UPHD_HD_NAT_18863_0_7132914251771213163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/Ex_HD_HD_NAT_17446_0_5758345042300784163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/MSNBC_HD_NAT_18292_0_8348966700753924163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/FBNHD_HD_NAT_16298_0_5381829188959497163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WEAHD_HD_VINdGA_16251_21_6533405827587038163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/TLCHD_HD_NAT_16123_0_5874463359705343163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/CNNHD_HD_NAT_16141_0_5646493630829879163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/HLNHD_HD_NAT_18324_0_6426278603478736163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/CNBCD_HD_NAT_16332_0_5508729560697580163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/FNCHD_HD_NAT_16756_0_5884597068415311163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/AnEHD_HD_NAT_14710_0_6713276793419826163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/NGCHD_HD_NAT_13745_0_6728312959800788163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/DSCHD_HD_NAT_16122_0_5965163366898931163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/TNTHD_HD_NAT_11911_0_5935809569364178163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/USAHD_HD_NAT_16129_0_6268853190323114163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/FXHD_HD_NAT_16346_0_8602725829654720163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/VELOC_HD_NAT_10935_0_5279400581678749163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/NBCSH_HD_VINdGA_15201_1_8119368528224581163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/ES2HD_HD_VINdGA_12800_1_6390870912565095163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/FSNHD_HD_VINdGA_18480_1_5941974573549670163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/GOLFD_HD_NAT_16403_0_6257457287223774163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/MTVL_HD_NAT_13551_0_4737598581167412163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/VH1HD_HD_NAT_17355_0_5629501378424755163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/HISHD_HD_NAT_16098_0_7470481520976349163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/MTVHD_HD_NAT_17356_0_6758960311495311163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/FREE_HD_NAT_16692_0_6878338946670573163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/LIFED_HD_NAT_16798_0_6447136429865014163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/AMCHD_HD_NAT_16414_0_9214799011838992163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/FOODD_HD_NAT_13907_0_7140614049851369163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/TRTVD_HD_NAT_18411_0_7146321324058699163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/HGTVD_HD_NAT_13738_0_5726980668815640163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/TRAVD_HD_NAT_16485_0_8987205474424984163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/APHD_HD_NAT_16120_0_7545320481318804163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/TOOND_HD_NAT_16548_0_5212050385360922163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/NICHD_HD_NAT_16713_0_8910151615109183163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/DISHD_HD_NAT_16686_0_7033467517825632163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/DXDHD_HD_NAT_16745_0_7932601938684906163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/HCHD_HD_NAT_18862_0_9145527217116798163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/CMDYD_HD_NAT_17354_0_5056164493401972163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/TCMHD_HD_NAT_18333_0_6663191288268173163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/OLYHD_HD_NAT_11684_0_6280906050452802163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/TVONE_HD_NAT_17485_0_6774211691702437163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/BRVOD_HD_NAT_16182_0_5229546089320460163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/FUSE_HD_NAT_16413_0_8790512000839764163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/SYFYD_HD_NAT_16291_0_7280561135462210163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/FS1HD_HD_VINdGA_24912_1_6774509089883178163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/SCIHD_HD_NAT_16121_0_5694356432536312163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/PARHD_HD_NAT_16547_0_8089955797307534163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/NFLHD_HD_VINdGA_12393_1_6439509850442331163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/HBOHD_HD_NAT_15152_0_5939026565177792163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/NHLHD_HD_VINdGA_16279_1_7957310349182987163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/NBAHD_HD_VINdGA_17538_0_5718368376909713163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/ESPND_HD_VINdGA_11299_1_8689790441715610163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/ESPNU_HD_VINdGA_16967_0_7053565089867894163.m3u8",
        "http://ccr.linear-vin-ga-pil-red.xcr.comcast.net/FSSED_HD_VINdGA_18663_0_6740943472012370163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/MAXHD_HD_NAT_11750_0_5650733033433752163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/SHOHD_HD_NAT_10911_0_5603710716644917163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/VICEH_HD_NAT_18755_0_8774012364712648163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/IDHD_HD_NAT_18662_0_6735664365558589163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/MGMHD_HD_NAT_16345_0_7477540787636571163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/STZHD_HD_NAT_11716_0_7509424492416089163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/HMMHD_HD_NAT_16672_0_7012235333044661163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/LMHD_HD_NAT_16240_0_8980645024256258163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/IFCHD_HD_NAT_16412_0_6333999742934466163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WSB_SD_VINdGA_6035_0_7126251656397319163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WAGA_SD_VINdGA_5495_0_6966254591611536163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WXIA_SD_VINdGA_6173_0_7813016751463178163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WPCH_SD_VINdGA_11496_0_5844337226729719163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/GPB_SD_VINdGA_13774_0_7320455467539992163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WGCL_SD_VINdGA_5724_0_7658826722957812163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WUPA_SD_VINdGA_6122_0_8221515987219234163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WHSG_SD_VINdGA_5760_0_6350749165724403163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/ION_SD_VINdGA_8667_0_5204330350608102163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/MYATL_SD_VINdGA_5515_0_5897837958017967163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WUVG_SD_VINdGA_5925_0_6337149440287653163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WUVM_SD_VINdGA_16083_0_8141873353873292163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/PBA_SD_VINdGA_5969_0_4926705098188004163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/MeTV_SD_VINdGA_4535_0_9043270805490051163.m3u8",
        "http://ccr.linear-vin-ga-pil.xcr.comcast.net/WKTB_SD_VINdGA_24269_0_7571186369162032163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/HSN_SD_NAT_4130_0_6518435840595755163.m3u8",
        "http://ccr.linear-nat-pil-red.xcr.comcast.net/QVC_SD_NAT_4164_0_5141888018197226163.m3u8"
    ],
    //QAM LINEAR
    [ 
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3ece",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3ece",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2d12",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2c0c",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x5b99",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2c06",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3211",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x41b6",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2c08",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x30c2",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x48a0",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4754",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3007",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x49b6",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x40c2",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x32eb",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4015",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x41b1",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x402e",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x417a",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4123",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x41d4",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x447f",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x46fa",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4397",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x49af",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4426",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4774",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3faa",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3f7b",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3efb",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3f0d",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4794",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3fcc",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4174",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3976",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x35b1",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3efa",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2e87",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3f01",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3fda",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2ab7",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3b61",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2c23",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3200",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4830",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4013",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x34ef",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x43cb",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3ee2",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x43cc",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4134",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x419e",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x401e",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3653",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x47eb",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x35aa",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4065",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3ef8",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x40a4",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4149",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x412e",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4169",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x49ae",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x43ca",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x479d",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2da4",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x444d",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3f36",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x401d",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3fa3",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x6150",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3ef9",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x40a3",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3069",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3b30",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3f97",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4482",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4247",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x48e7",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2de6",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2a9f",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4943",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x48e6",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3fd9",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2dc4",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x4120",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3f70",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x401c",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3eff",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x1588",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x1793",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x1577",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x181d",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x2ce8",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x35ce",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x165c",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x17ea",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x1680",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x21db",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x158b",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x1725",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x3ed3",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x1751",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x11b7",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x5ecd",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x1022",
        "http://127.0.0.1:8080/hnStreamStart?DTCP1HOST=127.0.0.1&DTCP1PORT=5000&live=ocap://0x1044"
    ]
];

var scene = imports.scene;
var keys = imports.keys;
var splash = null;
var controls = null;
var logpanel = null;
var testpanel = null;
var waylandObj = null;
var player = null;
var audioLanguages = null;
var currentSpeed = 1.0;
var isTuned = false;
var testCases = null;
var currentTest = null;
var failedTests = [];
var totalTests = 0;
var stream_id = [0,0,0,0];
var stream_type = IP_LINEAR;//IP_VOD,IP_CDVR,IP_LINEAR,QAM_LINEAR
var logDate = new Date();
var logBaseTime = logDate.getTime();
var tuneStartTime;
var tuneEndTime;
var enableQam = false;

module.exports.wantsClearscreen = function()
{
    return false;
};

function log(msg)
{
    var d = new Date();
	console.log(d.getTime() - logBaseTime + ": rdkmediaplayer.js: " + msg);
}

function getCurrentStreamURL()
{
    return streams[stream_type][stream_id[stream_type]];
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
    var PH = 100;//panel height
    var RH = PH/2;//row height

	this.background = scene.create({t: "rect",
								    x: 0,
								    y: scene.h - PH,
								    w: scene.w,
								    h: PH,
								    parent: scene.root,
								    fillColor:0x000000AA });
    var W = 80;//button width
    var H = 30;//button height
    var S = 6;//space between button
	var X = scene.w/2.0 - (W*7) - (S*6.5);
	var Y = RH*0.5 - H/2;
    var D = W+S;

    this.divider0 = scene.create({t: "rect",
							    x: 0,
							    y: 0,
							    w: scene.w,
							    h: 2,
							    parent: this.background,
							    fillColor:0xFFFFFF55 }); 

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
    this.buttons = [
        new ControlButton(this, 0, X+D*0, Y, W, H, doIPVod, "IP VOD",null),
        new ControlButton(this, 1, X+D*1, Y, W, H, doIPCdvr, "IP CDVR",null),
        new ControlButton(this, 2, X+D*2, Y, W, H, doIPLinear, "IP LIN",null),
        new ControlButton(this, 3, X+D*3, Y, W, H, doQAMLinear, "QAM LIN",null),
        new ControlButton(this, 4, X+D*4, Y, W, H, doPause, "PAUSE", null),
        new ControlButton(this, 5, X+D*5, Y, W, H, doPlay, "PLAY", null),
        new ControlButton(this, 6, X+D*6, Y, W, H, doRestart, "RESTART",null),
        new ControlButton(this, 7, X+D*7, Y, W, H, doLive, "LIVE",null),
        new ControlButton(this, 8, X+D*8, Y, W, H, doREW, "REW", null),
        new ControlButton(this, 9, X+D*9, Y, W, H, doFFW, "FFW", null),
        new ControlButton(this, 10, X+D*10, Y, W, H, doSeekBk, "SEEK-", null),
        new ControlButton(this, 11, X+D*11, Y, W, H, doSeekFr, "SEEK+",null),
        new ControlButton(this, 12, X+D*12, Y, W, H, doCC, "CC", null),
        new ControlButton(this, 13, X+D*13, Y, W, H, doSap, "SAP",null),
    ];
    var len = this.buttons.length;
    for (i = 0; i < len-1; i++)
    {
        this.buttons[i].right = this.buttons[i+1];
        this.buttons[i+1].left = this.buttons[i];
    }
    this.buttons[0].left = this.buttons[len-1];
    this.buttons[len-1].right = this.buttons[0];

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
    if(logsEnabled)
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
    this.focused = false;
    this.background.draw = false;
    if(logsEnabled)
        logpanel.setVisible(false);
}

ControlPanel.prototype.navDown = function()
{
    if(!this.focused)
    {
		controls.focused = true;
		controls.background.draw = true;
        if(logsEnabled)
            logpanel.setVisible(true);
    }
}

ControlPanel.prototype.doAction = function()
{
	this.focusedButton.action.call();
}

ControlPanel.prototype.setStream = function()
{
    this.streamName.text = getCurrentStreamURL();
    
    var nameSize = this.streamName.font.measureText(this.streamName.pixelSize, this.streamName.text);
    this.streamPosition.x = this.streamName.x +nameSize.w + 50;
    this.streamPosition.text = "Position 0 of 0";
}

ControlPanel.prototype.updateTime = function(position, duration)
{
    //log("updateTime: " + t);
    this.streamPosition.text = "Position " + position.toFixed(2) + " of " + duration.toFixed(2);
}

function loadStream()
{
    if(logsEnabled)
        logpanel.clear();
    if(splashEnabled)
        splash.setVisible(true);
    if(controlsEnabled)
        controls.setStream();
    doLoad(getCurrentStreamURL());
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//LOG PANEL

function LogPanel(x, w)
{
    var PW = w; //panel width
	this.background = scene.create({t: "rect",
						            x: x,
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
    var text = this.textbox.text +  message + "\n";
    var pixsize = this.textbox.pixelSize;
    this.textbox.text = text;
    var size = this.textbox.font.measureText(pixsize, text);
    this.background.h = size.h;
    //do some rudimentary scrolling
    var maxheight;
    if(controlsEnabled)
        maxheight = controls.background.y
    else
        maxheight = scene.h-60;
    while(this.background.h > maxheight)
    {
        text = this.textbox.text;
        var i = text.indexOf("\n");
        if(i >= 0)
        {
            text = text.substr(i+1, text.length - i+1);
            this.textbox.text = text;
            var size = this.textbox.font.measureText(pixsize, text);
            this.background.h = size.h;
        }
        else
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//PLAYER STUFF

function onMediaOpened(e)
{
    if(logsEnabled)
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

    if(currentTest != null)
    {
        currentTest.onEvent(e);
    }
}
function onProgress(e)
{
    if(!isTuned)
    {
        isTuned = true;
        var d = new Date();
        tuneEndTime = d.getTime() - tuneStartTime;

        if(splashEnabled)
            splash.setVisible(false);
        if(logsEnabled) {
            logpanel.log("Tune Complete in " + tuneEndTime + "ms");
        }

    }
    if(controlsEnabled)
        controls.updateTime(e.position, e.duration);

    if(currentTest != null)
    {
        currentTest.onEvent(e);
    }
}
function onError(e)
{
    if(logsEnabled)
        logpanel.log("Tune error:" + e.code + "\n" + e.description);

    if(currentTest != null)
    {
        currentTest.onEvent(e);
        testpanel.log("Tune error:" + e.code + "\n" + e.description);
    }
}

function onEvent(e)
{
    if(logsEnabled)
        logpanel.log("Event " + e.name);

    if(currentTest != null)
    {
        currentTest.onEvent(e);
    }
}

function registerPlayerEvents()
{
    player.on("onMediaOpened",onMediaOpened);
    player.on("onProgress",onProgress);
    player.on("onStatus",onEvent);
    player.on("onWarning",onEvent);
    player.on("onError",onError);
    player.on("onSpeedChange",onEvent);
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
    player.delListener("onProgress",onProgress);
    player.delListener("onClosed",onEvent);
    player.delListener("onPlayerInitialized",onEvent);
    player.delListener("onBuffering",onEvent);
    player.delListener("onPlaying",onEvent);
    player.delListener("onPaused",onEvent);
    player.delListener("onComplete",onEvent);
    player.delListener("onIndividualizing",onEvent);
    player.delListener("onAcquiringLicense",onEvent);
    player.delListener("onWarning",onEvent);
    player.delListener("onError",onError);
    player.delListener("onSpeedChange",onEvent);
    player.delListener("onDRMMetadata",onEvent);
    player.delListener("onSegmentStarted",onEvent);
    player.delListener("onSegmentCompleted",onEvent);
    player.delListener("onSegmentWatched",onEvent);
    player.delListener("onBufferWarning",onEvent);
    player.delListener("onPlaybackSpeedsChanged",onEvent);
    player.delListener("onAdditionalAuthRequired",onEvent);
}

function hackToMakeEventsFlow1()
{
    hackTimer = setInterval(hackToMakeEventsFlow2, 250);
}

function hackToMakeEventsFlow2()
{
    player.volume;
}

function doLoad(url)
{
    if(blockTune)
        return;
    log("doLoad:" + url);
    var d = new Date();
    tuneStartTime = d.getTime();
    isTuned = false;
	player.url = url;
	player.setVideoRectangle(0,0,scene.w,scene.h);
	currentSpeed = 1.0;
    player.play();
}

function doPause()
{
    log("doPause");
	currentSpeed = 0.0;
	player.pause();
    if(controlsEnabled)
        controls.updateState();
}

function doPlay()
{
    log("doPlay");
	currentSpeed = 1.0;
	player.play();
    if(controlsEnabled)
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

    if(controlsEnabled)
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

    if(controlsEnabled)
        controls.updateState();
}

function doRestart()
{
	log("doRestart");
	player.position = 0;
    if(controlsEnabled)
        controls.updateState();
}

function doLive()
{
	log("doLive");
	player.seekToLive();
    if(controlsEnabled)
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
    if(controlsEnabled)
        controls.updateState();
}

function doCC()
{
	log("doCC");
    if(player.closedCaptionsEnabled == "true")
        player.closedCaptionsEnabled = "false";
    else
        player.closedCaptionsEnabled = "true";
    if(controlsEnabled)
        controls.updateState();
}

function doStream(type)
{
    if(stream_type == type)
    {
        //channel up
        stream_id[type]++;
        if(stream_id[type] >= streams[type].length)
            stream_id[type]=0;
    }
    else
        stream_type = type;
    loadStream();
}

function doIPVod()
{
    doStream(IP_VOD);
}
function doIPCdvr()
{
    doStream(IP_CDVR);
}
function doIPLinear()
{
    doStream(IP_LINEAR);
}
function doQAMLinear()
{
    if(enableQam)
        doStream(QAM_LINEAR);
}

function doChannelChangeRandom()
{
    var len = 3;
    if(enableQam)
        len = 4;
    var total = 0;
    for(var i = 0; i < len; i++)
        total+= streams[i].length;
    var id = Math.floor(Math.random() * total);
    total = 0;
    for(var i = 0; i < len; i++)
    {
        if(id < total + streams[i].length)
        {
            stream_type = i;
            stream_id[i] = id - total;
            break;
        }
        total += streams[i].length;
    }
    loadStream();
}

function handleWaylandRemoteSuccess(wayland)
{
	log("Handle wayland success");
	waylandObj = wayland;
	waylandObj.moveToBack();
	player = wayland.api;
    registerPlayerEvents();
    if(autoTuneEnabled)
        doChannelChangeRandom();
    else if(splashEnabled)
        splash.setVisible(false);
    setTimeout(hackToMakeEventsFlow1, 1000);
    if(testsEnabled)
        startTests();
}

function handleWaylandRemoteError(error)
{
	log("Handle wayland error");
    //delete process.env.PLAYERSINKBIN_USE_WESTEROSSINK;
    if(splashEnabled)
    	splash.setVisible(false);
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

//TESTING

//onMediaOpened
function TestMediaOpened()
{
    this.name = "TestMediaOpened";
}
TestMediaOpened.prototype.start = function()
{
    this.tmr = setTimeout(testTimeout, 10000);
    //setTimeout(onMediaOpened,3000);
    loadStream();
}
TestMediaOpened.prototype.onEvent = function(e)
{
    if(e.name == "onMediaOpened")
    {
        clearTimeout(this.tmr);
        testSuccess(this);
    }
}

//onProgress
function TestProgress()
{
    this.name = "TestProgress";
}
TestProgress.prototype.start = function()
{
    this.tmr = setTimeout(testTimeout, 20000);
    this.count = 0;
    loadStream();
}
TestProgress.prototype.onEvent = function(e)
{
    if(e.name == "onProgress")
    {
        this.count++;
        if(this.count == 10)
        {
            clearTimeout(this.tmr);
            testSuccess(this);
        }
    }
}

//channel change
function changeChannelTimeout()
{
    currentTest.changeChannel();
}
function TestChannelChange()
{
    this.name = "TestChannelChange";
    this.timeoutVal = 0;
    this.timeoutId = 0;
    this.totalAttemptCount = 0;
    this.totalTuneTime = 0;
    this.totalTuneCount = 0;
    this.totalQamTuneTime = 0;
    this.totalQamTuneCount = 0;
    this.fail = false;
    this.totalStatusTuneCount = 0;
    this.totalStatusTuneFails = 0;
    this.totalInteruptedCount = 0;
    this.STATUS_TUNE_TIMEOUT = 20000;
    this.STATUS_TUNE_INTERVAL = 10;
}
TestChannelChange.prototype.start = function()
{
    this.changeChannel();
}
TestChannelChange.prototype.onEvent = function(e)
{
    if(tuneEndTime != 0 && e.name == "onProgress")
    {
        //jump quickly to next tune instead of waiting on full tune timeout
        if(this.timeoutVal == this.STATUS_TUNE_TIMEOUT && this.timeoutId)
        {
            clearTimeout(this.timeoutId);
            this.timeoutId = 0;
            setTimeout(changeChannelTimeout,2000);
        }
    }
    else if(e.name == "onError")
    {
        
    }
}
TestChannelChange.prototype.changeChannel = function()
{
    //process previous tune results
    if(this.totalAttemptCount > 0)
    {
        if(this.timeoutVal == this.STATUS_TUNE_TIMEOUT)//status tunes are done to ensure player can still tune after getting a bunch of random channel changes
        {
            this.totalStatusTuneCount++;
            if(tuneEndTime == 0)
            {
                this.fail = true;
                this.totalStatusTuneFails++;
                testpanel.log("status tune fail");
            }
            else
            {
                testpanel.log("status tune success");
            }
        }
        if(tuneEndTime > 0)//all tunes get their time recorded
        {
            if(stream_type == QAM_LINEAR)
            {
                this.totalQamTuneTime += tuneEndTime;
                this.totalQamTuneCount++;
            }
            else
            {
                this.totalTuneTime += tuneEndTime;
                this.totalTuneCount++;
            }
        }
        else
        {
            this.totalInteruptedCount++;
        }
    }

    if(this.totalAttemptCount < MAX_CHANNEL_CHANGES)
    {
        //perform new channel change
        if( this.totalAttemptCount == 0)//first tune
            this.timeoutVal = MAX_TUNE_TIME;
        else if( ((this.totalAttemptCount+1) % this.STATUS_TUNE_INTERVAL) == 0 )//allow a full tune to ensure player can still achieve a tune
            this.timeoutVal = this.STATUS_TUNE_TIMEOUT;
        else // all other tunes get a random time
            this.timeoutVal = Math.random() * MAX_TUNE_TIME; //random between 0 and 10 seconds
        this.timeoutId = setTimeout(changeChannelTimeout,this.timeoutVal);
        if( ((this.totalAttemptCount+2) % this.STATUS_TUNE_INTERVAL) == 0 )
            testpanel.log("status tune in " + Math.round(this.timeoutVal) + " ms");
        else
            testpanel.log("next tune in " + Math.round(this.timeoutVal) + " ms");
        var d = new Date();
        this.totalAttemptCount++;
        doChannelChangeRandom();
    }
    else
    {
        var grandTotalTunes = this.totalTuneCount+this.totalQamTuneCount;
        testpanel.log("total tunes attempted: " + this.totalAttemptCount);
        testpanel.log("total tunes completed: " + grandTotalTunes);
        testpanel.log("total tunes interupted: " + this.totalInteruptedCount);
        testpanel.log("total status tunes: " + this.totalStatusTuneCount);
        testpanel.log("total status fails: " + this.totalStatusTuneFails);
        testpanel.log("total ip tunes completed: " + this.totalTuneCount);
        testpanel.log("total qam tunes completed: " + this.totalQamTuneCount);
        if(this.totalTuneCount>0)
            testpanel.log("avg ip tune time: " + Math.floor(this.totalTuneTime / this.totalTuneCount));
        else
            testpanel.log("avg ip tune time: 0");
        if(this.totalQamTuneCount>0)
            testpanel.log("avg qam tune time: " + Math.floor(this.totalQamTuneTime / this.totalQamTuneCount));
        else
            testpanel.log("avg qam tune time: 0");
        if(this.fail)
            testFail();
        else
            testSuccess();
    }
}

function startTests()
{
    testpanel.log("Starting automated tests");
    testCases = [];
    //testCases.push(new TestMediaOpened());
    //testCases.push(new TestProgress());
    testCases.push(new TestChannelChange());
    totalTests = testCases.length;
    setTimeout(startNextTest, 1000);
}
function startNextTest()
{
    if(testCases.length > 0)
    {
        currentTest = testCases.shift();
        testpanel.log(currentTest.name + " start");
        currentTest.start();
    }
    else
    {
        testpanel.log("All tests complete");
        testpanel.log(totalTests - failedTests.length + " succeeded");
        testpanel.log(failedTests.length + " failed");
        if(failedTests.length>0)
        {
            showTestResultImage(false);
            testpanel.log("Failed tests:");
            for(var i = 0; i < failedTests.length; ++i)
                testpanel.log(failedTests[i]);
        }
        else
        {
            showTestResultImage(true);
        }
        testpanel.log("\n\n");
        player.stop();
    }
}
function testSuccess()
{
    testpanel.log(currentTest.name + " success");
    setTimeout(startNextTest, 250);
}
function testTimeout()
{
    failedTests.push(currentTest.name);
    testpanel.log(currentTest.name + " timeout");
    setTimeout(startNextTest, 250);
}
function testFail()
{
    failedTests.push(currentTest.name);
    testpanel.log(currentTest.name + " fail");
    setTimeout(startNextTest, 250);
}
function showTestResultImage(success)
{
    var color;
    if(success)
        color = 0x00FF00FF
    else
        color = 0xFF0000FF
	this.result = scene.create({t: "rect",
								    x: 0,
								    y: 0,
								    w: 100,
								    h: 100,
								    parent: scene.root,
								    fillColor:color });
}
//END TESTING

function doControlAction()
{
    if(controls.focused)
    {
        controls.doAction();
    }
}

if(logsEnabled)
    logpanel = new LogPanel(scene.w-200,200);
if(splashEnabled)
    splash = new SplashScreen();
if(controlsEnabled)
    controls = new ControlPanel();
if(testsEnabled)
    testpanel = new LogPanel(scene.w-500,200);

if(controlsEnabled || remoteEnabled)
{
    var lastKeyCode = -1;
    var lastKeyDate = new Date();
    scene.root.on("onKeyDown", function (e) 
    {
	    var code  = e.keyCode;
	    var flags = e.flags;

/*
        //drop repeated keys begin
        if(code == lastKeyCode)
        {
            var d = new Date();
            var isRepeat = (d.getTime() - lastKeyDate.getTime() < 250);
            lastKeyDate = d;
            if(isRepeat)
            {
                log("rdkmediaplayer.js onKeyDown KEY REPEATED code:" + code + " flags:"+flags);
                return;
            }
        }
        lastKeyCode = code;
        lastKeyDate = new Data();
        //drop repeated keys end
*/

	    log("rdkmediaplayer.js onKeyDown code:" + code + " flags:"+flags);

        if(controlsEnabled)
        {
	        if(code == keys.ENTER || code == 0)
	        {
		        if(controls.focused)
		        {
			        setTimeout(doControlAction,30);//use setTimeout to workaround getting multiple onKeyDowns if we do a heavy action such as player.url
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

        if(remoteEnabled)
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

process.env.PLAYERSINKBIN_USE_WESTEROSSINK="1";
process.env.XDG_RUNTIME_DIR="/tmp";

waylandObj = scene.create( {t:"wayland", x:0, y:0, w:1280, h:720, parent:scene.root,cmd:"rdkmediaplayer"} );
waylandObj.remoteReady.then(handleWaylandRemoteSuccess, handleWaylandRemoteError);
waylandObj.focus = true;
waylandObj.moveToBack();

if(controlsEnabled)
    controls.background.moveToFront();


function getModel() {
    return new Promise(function (resolve, reject) {
        try {
            var s = scene.getService("systemService");
            s.setApiVersionNumber(8);
            var callStr = s.callMethod("getXconfParams");
            resolve(JSON.parse(callStr)["xconfParams"][0].model.trim());
        } catch (e) {
            reject(new Error("getModel failed: "+e));
        }
    });
}

getModel().then(function (val) {
    log("model="+val);
    if(val.substr(0,7) == "PX013AN")
    {
        log("enabling qam");
        enableQam = true;
        if(testsEnabled)
            testpanel.log("enabling qam");
    }
});


log("js fully loaded");

}).catch( function importFailed(err){
  console.error("Import failed for rdkmediaplayer.js: " + err)
});
