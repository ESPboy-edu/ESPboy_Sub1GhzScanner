/*
ESPboy Sub1GhzScaner using CC1101 module
by RomanS 03.12.2021
check ESPboy_Sub1GhzInspector
and www.espboy.com project
*/

#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <ESP_EEPROM.h>

#define CC1101chipSelectPin D8
#define TFTchipSelectPin    8

#define FIRMWARE_ID  0xAAEA

float startFreq = 433;
int8_t rssiThreshold = -60;
float freqStep = 0.01;
uint8_t scaleFactor = 2;

float firstHigh = -1; 
float lastHigh = -1;
bool redVertFlag = false;

uint32_t savingTimer;
bool savingFlag = false;

struct SaveStruct{
 uint32_t saveFirmwareID;
 float saveStartFreq;
 int8_t saveRssiThreshold;
 float saveFreqStep;
}saveStruct;

int8_t scanDat[128];

uint32_t PROGMEM ironPalette[] = {
0x00000a, 0x000014, 0x00001e, 0x000025, 0x00002a, 0x00002e, 0x000032, 0x000036, 0x00003a, 0x00003e,
0x000042, 0x000046, 0x00004a, 0x00004f, 0x000052, 0x010055, 0x010057, 0x020059, 0x02005c, 0x03005e,
0x040061, 0x040063, 0x050065, 0x060067, 0x070069, 0x08006b, 0x09006e, 0x0a0070, 0x0b0073, 0x0c0074,
0x0d0075, 0x0d0076, 0x0e0077, 0x100078, 0x120079, 0x13007b, 0x15007c, 0x17007d, 0x19007e, 0x1b0080, 
0x1c0081, 0x1e0083, 0x200084, 0x220085, 0x240086, 0x260087, 0x280089, 0x2a0089, 0x2c008a, 0x2e008b, 
0x30008c, 0x32008d, 0x34008e, 0x36008e, 0x38008f, 0x390090, 0x3b0091, 0x3c0092, 0x3e0093, 0x3f0093, 
0x410094, 0x420095, 0x440095, 0x450096, 0x470096, 0x490096, 0x4a0096, 0x4c0097, 0x4e0097, 0x4f0097, 
0x510097, 0x520098, 0x540098, 0x560098, 0x580099, 0x5a0099, 0x5c0099, 0x5d009a, 0x5f009a, 0x61009b, 
0x63009b, 0x64009b, 0x66009b, 0x68009b, 0x6a009b, 0x6c009c, 0x6d009c, 0x6f009c, 0x70009c, 0x71009d, 
0x73009d, 0x75009d, 0x77009d, 0x78009d, 0x7a009d, 0x7c009d, 0x7e009d, 0x7f009d, 0x81009d, 0x83009d, 
0x84009d, 0x86009d, 0x87009d, 0x89009d, 0x8a009d, 0x8b009d, 0x8d009d, 0x8f009c, 0x91009c, 0x93009c, 
0x95009c, 0x96009b, 0x98009b, 0x99009b, 0x9b009b, 0x9c009b, 0x9d009b, 0x9f009b, 0xa0009b, 0xa2009b, 
0xa3009b, 0xa4009b, 0xa6009a, 0xa7009a, 0xa8009a, 0xa90099, 0xaa0099, 0xab0099, 0xad0099, 0xae0198, 
0xaf0198, 0xb00198, 0xb00198, 0xb10197, 0xb20197, 0xb30196, 0xb40296, 0xb50295, 0xb60295, 0xb70395, 
0xb80395, 0xb90495, 0xba0495, 0xba0494, 0xbb0593, 0xbc0593, 0xbd0593, 0xbe0692, 0xbf0692, 0xbf0692, 
0xc00791, 0xc00791, 0xc10890, 0xc10990, 0xc20a8f, 0xc30a8e, 0xc30b8e, 0xc40c8d, 0xc50c8c, 0xc60d8b, 
0xc60e8a, 0xc70f89, 0xc81088, 0xc91187, 0xca1286, 0xca1385, 0xcb1385, 0xcb1484, 0xcc1582, 0xcd1681, 
0xce1780, 0xce187e, 0xcf187c, 0xcf197b, 0xd01a79, 0xd11b78, 0xd11c76, 0xd21c75, 0xd21d74, 0xd31e72, 
0xd32071, 0xd4216f, 0xd4226e, 0xd5236b, 0xd52469, 0xd62567, 0xd72665, 0xd82764, 0xd82862, 0xd92a60, 
0xda2b5e, 0xda2c5c, 0xdb2e5a, 0xdb2f57, 0xdc2f54, 0xdd3051, 0xdd314e, 0xde324a, 0xde3347, 0xdf3444, 
0xdf3541, 0xdf363d, 0xe0373a, 0xe03837, 0xe03933, 0xe13a30, 0xe23b2d, 0xe23c2a, 0xe33d26, 0xe33e23, 
0xe43f20, 0xe4411d, 0xe4421c, 0xe5431b, 0xe54419, 0xe54518, 0xe64616, 0xe74715, 0xe74814, 0xe74913, 
0xe84a12, 0xe84c10, 0xe84c0f, 0xe94d0e, 0xe94d0d, 0xea4e0c, 0xea4f0c, 0xeb500b, 0xeb510a, 0xeb520a, 
0xeb5309, 0xec5409, 0xec5608, 0xec5708, 0xec5808, 0xed5907, 0xed5a07, 0xed5b06, 0xee5c06, 0xee5c05, 
0xee5d05, 0xee5e05, 0xef5f04, 0xef6004, 0xef6104, 0xef6204, 0xf06303, 0xf06403, 0xf06503, 0xf16603, 
0xf16603, 0xf16703, 0xf16803, 0xf16902, 0xf16a02, 0xf16b02, 0xf16b02, 0xf26c01, 0xf26d01, 0xf26e01, 
0xf36f01, 0xf37001, 0xf37101, 0xf37201, 0xf47300, 0xf47400, 0xf47500, 0xf47600, 0xf47700, 0xf47800, 
0xf47a00, 0xf57b00, 0xf57c00, 0xf57e00, 0xf57f00, 0xf68000, 0xf68100, 0xf68200, 0xf78300, 0xf78400, 
0xf78500, 0xf78600, 0xf88700, 0xf88800, 0xf88800, 0xf88900, 0xf88a00, 0xf88b00, 0xf88c00, 0xf98d00, 
0xf98d00, 0xf98e00, 0xf98f00, 0xf99000, 0xf99100, 0xf99200, 0xf99300, 0xfa9400, 0xfa9500, 0xfa9600, 
0xfb9800, 0xfb9900, 0xfb9a00, 0xfb9c00, 0xfc9d00, 0xfc9f00, 0xfca000, 0xfca100, 0xfda200, 0xfda300, 
0xfda400, 0xfda600, 0xfda700, 0xfda800, 0xfdaa00, 0xfdab00, 0xfdac00, 0xfdad00, 0xfdae00, 0xfeaf00, 
0xfeb000, 0xfeb100, 0xfeb200, 0xfeb300, 0xfeb400, 0xfeb500, 0xfeb600, 0xfeb800, 0xfeb900, 0xfeb900, 
0xfeba00, 0xfebb00, 0xfebc00, 0xfebd00, 0xfebe00, 0xfec000, 0xfec100, 0xfec200, 0xfec300, 0xfec400, 
0xfec500, 0xfec600, 0xfec700, 0xfec800, 0xfec901, 0xfeca01, 0xfeca01, 0xfecb01, 0xfecc02, 0xfecd02, 
0xfece03, 0xfecf04, 0xfecf04, 0xfed005, 0xfed106, 0xfed308, 0xfed409, 0xfed50a, 0xfed60a, 0xfed70b, 
0xfed80c, 0xfed90d, 0xffda0e, 0xffda0e, 0xffdb10, 0xffdc12, 0xffdc14, 0xffdd16, 0xffde19, 0xffde1b, 
0xffdf1e, 0xffe020, 0xffe122, 0xffe224, 0xffe226, 0xffe328, 0xffe42b, 0xffe42e, 0xffe531, 0xffe635, 
0xffe638, 0xffe73c, 0xffe83f, 0xffe943, 0xffea46, 0xffeb49, 0xffeb4d, 0xffec50, 0xffed54, 0xffee57, 
0xffee5b, 0xffee5f, 0xffef63, 0xffef67, 0xfff06a, 0xfff06e, 0xfff172, 0xfff177, 0xfff17b, 0xfff280,
0xfff285, 0xfff28a, 0xfff38e, 0xfff492, 0xfff496, 0xfff49a, 0xfff59e, 0xfff5a2, 0xfff5a6, 0xfff6aa,
0xfff6af, 0xfff7b3, 0xfff7b6, 0xfff8ba, 0xfff8bd, 0xfff8c1, 0xfff8c4, 0xfff9c7, 0xfff9ca, 0xfff9cd, 
0xfffad1, 0xfffad4, 0xfffbd8, 0xfffcdb, 0xfffcdf, 0xfffde2, 0xfffde5, 0xfffde8, 0xfffeeb, 0xfffeee, 
0xfffef1, 0xfffef4, 0xfffff6};



ESPboyInit myESPboy;
TFT_eSprite sprSpectre = TFT_eSprite(&myESPboy.tft);
TFT_eSprite sprWaterfall = TFT_eSprite(&myESPboy.tft);
TFT_eSprite sprNumbers = TFT_eSprite(&myESPboy.tft);



void initCC1101(){
  myESPboy.mcp.digitalWrite(TFTchipSelectPin, HIGH);
  digitalWrite(CC1101chipSelectPin, LOW);
  
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setRxBW(58);
  ELECHOUSE_cc1101.SetRx();
  
  myESPboy.mcp.digitalWrite(TFTchipSelectPin, LOW);
  digitalWrite(CC1101chipSelectPin, HIGH);
}



void getRSSIcc1101(float freqSet){
  myESPboy.mcp.digitalWrite(TFTchipSelectPin, HIGH);
  digitalWrite(CC1101chipSelectPin, LOW);
  
  for(uint8_t i=0; i < 128; i++){
    ELECHOUSE_cc1101.setMHZ(freqSet);
    scanDat[i] = ELECHOUSE_cc1101.getRssi();
    freqSet += freqStep;
  }

  myESPboy.mcp.digitalWrite(TFTchipSelectPin, LOW);
  digitalWrite(CC1101chipSelectPin, HIGH);
}



void drawRSSI(){
 static uint8_t redVertFirst, redVertLast;
  
  for(uint8_t i=0; i < 128; i++){
    if (scanDat[i] < rssiThreshold) 
      sprSpectre.drawFastVLine(i, abs(scanDat[i]/2), (100-abs(scanDat[i])/2), TFT_BLUE);
    else
      sprSpectre.drawFastVLine(i, abs(scanDat[i]/2), (100-abs(scanDat[i])/2), TFT_RED);
  }

 for(uint8_t i=0; i<128; i++){
   if (scanDat[i] >= rssiThreshold){
      redVertFirst = i;
      firstHigh = startFreq+i*freqStep;
      redVertFlag = true;
      break;}
 }

 for(uint8_t i=127; i>0; i--){
   if (scanDat[i] >= rssiThreshold){
      redVertLast = i;
      lastHigh = startFreq+i*freqStep;
      break;}
 }

 sprSpectre.drawFastVLine(64, 0, 51, TFT_DARKGREY); 
 sprSpectre.drawFastHLine(0, abs(rssiThreshold/2), 128, TFT_DARKGREY); 
 
 if (redVertFlag){
    redVertFlag = false;
    sprSpectre.drawFastVLine(redVertFirst, 0, 50, TFT_RED);
    sprSpectre.drawFastVLine(redVertLast, 0, 50, TFT_RED);
    drawNumbers();
    sprNumbers.pushSprite(0, 112);
 }

 for(uint8_t i=0; i<128; i++){
   uint16_t addr = (100+scanDat[i])*4*scaleFactor;
   if (addr > 430) {addr = 430; scaleFactor = 1;};
   sprWaterfall.drawPixel(i, 58, myESPboy.tft.color24to16(pgm_read_dword(ironPalette+addr)));
 }

 sprWaterfall.drawPixel(64, 58, TFT_DARKGREY);
}



void drawNumbers(){
  sprNumbers.fillSprite(TFT_BLACK);

  sprNumbers.setTextColor(TFT_GREEN);
  sprNumbers.drawString((String)startFreq, 0, 0);
  sprNumbers.drawString((String)(startFreq+128*freqStep), 128-6*6, 0);
  
  sprNumbers.setTextColor(TFT_WHITE);
  sprNumbers.drawString((String)(startFreq+64*freqStep), 46, 0);

  sprNumbers.setTextColor(TFT_YELLOW);
  sprNumbers.drawString((String)(freqStep), 58, 8); 

  sprNumbers.setTextColor(TFT_RED);
  String toPrintF, toPrintL;
  if(firstHigh == -1) toPrintF = "---.--";
  else toPrintF = (String)firstHigh;
  if(lastHigh == -1) toPrintL = "---.--";
  else toPrintL = (String)lastHigh;
  sprNumbers.drawString(toPrintF, 0, 8);
  sprNumbers.drawString(toPrintL, 128-6*6, 8);
}



void doKeysActions(){
 static uint16_t getKey;
  getKey = myESPboy.getKeys();
  if(getKey) {
    if(getKey&PAD_DOWN && rssiThreshold >-95 ) rssiThreshold--;
    if(getKey&PAD_UP && rssiThreshold<-5) rssiThreshold++;
    if(getKey&PAD_RIGHT) startFreq+=freqStep;
    if(getKey&PAD_LEFT) startFreq-=freqStep;
    if(getKey&PAD_ACT && freqStep>0.01) freqStep -= 0.01;
    if(getKey&PAD_ESC && freqStep<0.99) freqStep += 0.01;
    if(getKey&PAD_LFT || getKey&PAD_RGT) {
      myESPboy.tft.setTextColor(TFT_RED);
      myESPboy.tft.drawString("PAUSE", 50, 16); 
      while(myESPboy.getKeys()) delay(100); 
      while(!myESPboy.getKeys()) delay(100);}
    delay(30);

    savingTimer = millis();
    savingFlag = true;

    drawNumbers();
    sprNumbers.pushSprite(0, 112);
  }
}



void loadParam(){
  EEPROM.get(0, saveStruct);
  if (saveStruct.saveFirmwareID != FIRMWARE_ID) saveParam();
  else{
    startFreq = saveStruct.saveStartFreq;
    rssiThreshold = saveStruct.saveRssiThreshold;
    freqStep = saveStruct.saveFreqStep;
  }
  savingFlag = false;
}



void saveParam(){
  saveStruct.saveFirmwareID = FIRMWARE_ID;
  saveStruct.saveStartFreq = startFreq;
  saveStruct.saveRssiThreshold = rssiThreshold;
  saveStruct.saveFreqStep = freqStep;

  EEPROM.put(0, saveStruct);
  EEPROM.commit();

  savingFlag = false;
}



void setup(){  
  Serial.begin(115200);
  EEPROM.begin(sizeof(saveStruct));
  myESPboy.begin("Sub1Ghz scanner");
  sprSpectre.createSprite(128, 51);
  sprWaterfall.createSprite(128, 59);
  sprNumbers.createSprite(128, 16);
  sprWaterfall.fillSprite(TFT_BLACK);
  sprNumbers.fillSprite(TFT_BLACK);
  initCC1101();
  loadParam();
  drawNumbers();
  sprNumbers.pushSprite(0, 112);
}



void loop(){ 
  getRSSIcc1101(startFreq);

  sprSpectre.fillSprite(TFT_BLACK);
  sprWaterfall.scroll(0,-1);
  drawRSSI();
  sprSpectre.pushSprite(0, 0);
  sprWaterfall.pushSprite(0, 52);
  
  doKeysActions();

  if (savingFlag && millis() - savingTimer > 3000) saveParam();
}
