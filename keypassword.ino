#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <OneButton.h>
#include <SimpleTimer.h>
#include <TFT_eSPI.h>

#define WM_DEBUG_LEVEL WM_DEBUG_DEV

#include <WiFiManager.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

#define BTN_PIN 0

#define LED_DI_PIN 40
#define LED_CI_PIN 39

#define TFT_CS_PIN 4
#define TFT_SDA_PIN 3
#define TFT_SCL_PIN 5
#define TFT_DC_PIN 2
#define TFT_RES_PIN 1
#define TFT_LEDA_PIN 38

#define FF17 &FreeSans9pt7b
#define FF18 &FreeSans12pt7b

#define PASSWORD_TIMEOUT 2000 //2s
#define GREETING_TIMEOUT 2000 //2s
#define AP_MODE_OFF_TIMEOUT 2000 //2s
#define AP_EXIT_TIMEOUT 60 //60s
#define ESP_BOOT_DELAY 5000 //5s

#define AP_NAME "KeyPassword"
#define CONFIG_FILE "/config.json"
#define DEFAULT_CURRENT_PASSWORD 1
#define DEFAULT_AUTO_DELAY "5"
#define DEFAULT_AUTO_VALUE 1

const String CUSTOM_ROUTE_TITLE = "Parameters";
const String CUSTOM_ROUTE = "/params";

#define GREETING "Hello"
#define GREETING2 "KeyPassword"
#define SELECTED_PASSWORD "Password:"

#define PARAMS_TITLE "Params"
#define CURRENT_PASSWORD_KEY "current_password"
#define PASSWORD1_KEY "password1"
#define PASSWORD1_TITLE "Password 1"

#define PASSWORD2_KEY "password2"
#define PASSWORD2_TITLE "Password 2"

#define PASSWORD3_KEY "password3"
#define PASSWORD3_TITLE "Password 3"

#define PASSWORD4_KEY "password4"
#define PASSWORD4_TITLE "Password 4"

#define PASSWORD5_KEY "password5"
#define PASSWORD5_TITLE "Password 5"

#define PASSWORD_AUTO_DELAY_KEY "password_auto_delay"
#define PASSWORD_AUTO_DELAY_TITLE "Auto enter password delay"

#define PASSWORD_AUTO_KEY "password_auto"
#define PASSWORD_AUTO_TITLE "Auto enter password"

TFT_eSPI tft = TFT_eSPI();
OneButton button(BTN_PIN, true);
SimpleTimer timer;
USBHIDKeyboard Keyboard;

WiFiManager wm;

int passwordTimerId = 0;
bool isApMode = false;

WiFiManagerParameter *currentPasswordTitle;
WiFiManagerParameter *currentPassword1;
WiFiManagerParameter *currentPassword2;
WiFiManagerParameter *currentPassword3;
WiFiManagerParameter *currentPassword4;
WiFiManagerParameter *currentPassword5;

WiFiManagerParameter *password1, *password2, *password3, *password4, *password5;
WiFiManagerParameter *passwordAutoDelay, *passwordAuto;

int currentPasswordValue = DEFAULT_CURRENT_PASSWORD;
char password1Value[20];
char password2Value[20];
char password3Value[20];
char password4Value[20];
char password5Value[20];
char passwordAutoDelayValue[6] = DEFAULT_AUTO_DELAY;
int passwordAutoValue = DEFAULT_AUTO_VALUE;

String getSelectedPasswordTitle() {
  switch (currentPasswordValue) {
    case 1:
      return PASSWORD1_TITLE;
    case 2:
      return PASSWORD2_TITLE;
    case 3:
      return PASSWORD3_TITLE;
    case 4:
      return PASSWORD4_TITLE;
    case 5:
      return PASSWORD5_TITLE;        
  }
}

String getSelectedPassword() {
  switch (currentPasswordValue) {
    case 1:
      return password1Value;
    case 2:
      return password2Value;
    case 3:
      return password3Value;
    case 4:
      return password4Value;
    case 5:
      return password5Value;        
  }
}

static void handleClick() {
  if (isApMode) {
    return;
  }

  showSmallText(SELECTED_PASSWORD);
  showText(getSelectedPasswordTitle());
  switchOnDisplay(true);

  sendString(getSelectedPassword());

  if (passwordTimerId > 0) {
    timer.restartTimer(passwordTimerId);
  }
  else {
    passwordTimerId = timer.setTimeout(PASSWORD_TIMEOUT, hideSelectedPassword);
  }
}

void sendString(String text) {
  Keyboard.print(text);
  delay(300);
  Keyboard.press(KEY_RETURN);
  delay(100);
  Keyboard.releaseAll();
}

void setupButton() {
  button.attachClick(handleClick);
  button.attachLongPressStop(handleLongPressStop);
}

void setupTft() {
  pinMode(TFT_LEDA_PIN, OUTPUT);

  tft.init();
  switchOnDisplay(false);
  tft.setRotation(1);
  
  clearScreen();
}

void setupUSB() {
  Keyboard.begin();
  USB.begin();
}


void handleParamsRoute(){
  String page = wm.getHTTPHead(PARAMS_TITLE);
  String pitem = FPSTR(HTTP_FORM_START);
  pitem.replace(FPSTR(T_v), F("wifisave")); // set form action
  page += pitem;
  page += wm.getParamOut();
  page += FPSTR(HTTP_FORM_END);
  page += FPSTR(HTTP_BACKBTN);
  page += FPSTR(HTTP_END);

  wm.server->send(200, FPSTR(HTTP_HEAD_CT), page);
}

void bindServerCallback(){
  wm.server->on(CUSTOM_ROUTE, handleParamsRoute);
}

void setupWifiManager() {
  WiFi.mode(WIFI_STA);

  std::vector<const char *> menu = { "custom", "exit" };
  wm.setMenu(menu);

  String menuHtml = "<form action='" + CUSTOM_ROUTE + "' method='get'><button>" + CUSTOM_ROUTE_TITLE + "</button></form><br/>\n"; 
  wm.setCustomMenuHTML(menuHtml);

  wm.setWebServerCallback(bindServerCallback);

  loadParams();

  addCustomParameters();

  wm.setSaveParamsCallback(saveParams);
}

void setup() {
  setupTft();
  setupButton();
  setupUSB();
  setupWifiManager();

  showGreeting();
  
  autoStartPassword();
}

void switchOnDisplay(bool isEnabled) {
  digitalWrite(TFT_LEDA_PIN, isEnabled  ? 0 : 1); //Switch display LED off
}

void hideSelectedPassword() {
  switchOnDisplay(false);
  clearScreen();
}

void handleLongPressStop() {
  passwordTimerId = 0;
  isApMode = true;

  showSmallText(WiFi.softAPIP().toString());
  showText(AP_NAME);
  switchOnDisplay(true);

  wm.setConfigPortalTimeout(AP_EXIT_TIMEOUT);

  if (!wm.startConfigPortal(AP_NAME, AP_NAME)) {
    reboot();
  }
}

void reboot() {
  ESP.restart();
  delay(ESP_BOOT_DELAY);
}

void clearScreen() {
  tft.fillScreen(TFT_BLACK);
}

void showSmallText(String text) {
  tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
  tft.setFreeFont(FF17);
  tft.drawString(text, 5, 15);
}

void showText(String text) {
  tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
  tft.setFreeFont(FF18);
  tft.drawString(text, 5, 45);
}

void showGreeting() {
  switchOnDisplay(true);
  showSmallText(GREETING);
  showText(GREETING2);
  delay(GREETING_TIMEOUT);
  switchOnDisplay(false);
  clearScreen();
}

void loadParams() {
  if (!SPIFFS.begin()) {
    return;
  }  
  if (SPIFFS.exists(CONFIG_FILE)) {
    File configFile = SPIFFS.open(CONFIG_FILE, "r");
    if (!configFile) {
      SPIFFS.end();
      return;
    }
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);

    configFile.readBytes(buf.get(), size);

    DynamicJsonDocument json(1024);
    auto deserializeError = deserializeJson(json, buf.get());
    if (!deserializeError) {
      if (json.containsKey(CURRENT_PASSWORD_KEY)) currentPasswordValue = atoi(json[CURRENT_PASSWORD_KEY]);
      if (json.containsKey(PASSWORD1_KEY)) strcpy(password1Value, json[PASSWORD1_KEY]);
      if (json.containsKey(PASSWORD2_KEY)) strcpy(password2Value, json[PASSWORD2_KEY]);
      if (json.containsKey(PASSWORD3_KEY)) strcpy(password3Value, json[PASSWORD3_KEY]);
      if (json.containsKey(PASSWORD4_KEY)) strcpy(password4Value, json[PASSWORD4_KEY]);
      if (json.containsKey(PASSWORD5_KEY)) strcpy(password5Value, json[PASSWORD5_KEY]);
      if (json.containsKey(PASSWORD_AUTO_DELAY_KEY)) strcpy(passwordAutoDelayValue, json[PASSWORD_AUTO_DELAY_KEY]);
      if (json.containsKey(PASSWORD_AUTO_KEY)) passwordAutoValue = atoi(json[PASSWORD_AUTO_KEY]);
    }
    configFile.close();
    SPIFFS.end();
  }
}

void saveParams() {
  if (!SPIFFS.begin()) {
    return;
  }
  File configFile = SPIFFS.open(CONFIG_FILE, "w");
  if (!configFile) {
    SPIFFS.end();
    return;
  }
  DynamicJsonDocument json(1024);

  json[CURRENT_PASSWORD_KEY] = wm.server->arg(CURRENT_PASSWORD_KEY); 
  json[PASSWORD1_KEY] = password1->getValue();
  json[PASSWORD2_KEY] = password2->getValue();
  json[PASSWORD3_KEY] = password3->getValue();
  json[PASSWORD4_KEY] = password4->getValue();
  json[PASSWORD5_KEY] = password5->getValue();
  json[PASSWORD_AUTO_DELAY_KEY] = passwordAutoDelay->getValue();
  json[PASSWORD_AUTO_KEY] = (wm.server->arg(PASSWORD_AUTO_KEY) == "1") ? "1" : "0";

  serializeJson(json, configFile);
  configFile.close();

  SPIFFS.end();
    
  wm.handleExit();
}

void addCustomParameters() {
  currentPasswordTitle = new WiFiManagerParameter("<br/><p>Current password</p>");
  wm.addParameter(currentPasswordTitle);

  currentPassword1 = (currentPasswordValue == 1) ? new WiFiManagerParameter("<input type='radio' id='choice1' name='current_password' value='1' checked><label for='choice1'> Password 1</label><br/><br/>")
  : new WiFiManagerParameter("<input type='radio' id='choice1' name='current_password' value='1'><label for='choice1'> Password 1</label><br/><br/>");
  wm.addParameter(currentPassword1);

  currentPassword2 = (currentPasswordValue == 2) ? new WiFiManagerParameter("<input type='radio' id='choice2' name='current_password' value='2' checked><label for='choice2'> Password 2</label><br/><br/>")
  : new WiFiManagerParameter("<input type='radio' id='choice2' name='current_password' value='2'><label for='choice2'> Password 2</label><br/><br/>");
  wm.addParameter(currentPassword2);

  currentPassword3 = (currentPasswordValue == 3) ? new WiFiManagerParameter("<input type='radio' id='choice3' name='current_password' value='3' checked><label for='choice3'> Password 3</label><br/><br/>")
  : new WiFiManagerParameter("<input type='radio' id='choice3' name='current_password' value='3'><label for='choice3'> Password 3</label><br/><br/>");
  wm.addParameter(currentPassword3);

  currentPassword4 = (currentPasswordValue == 4) ? new WiFiManagerParameter("<input type='radio' id='choice4' name='current_password' value='4' checked><label for='choice4'> Password 4</label><br/><br/>")
  : new WiFiManagerParameter("<input type='radio' id='choice4' name='current_password' value='4'><label for='choice4'> Password 4</label><br/><br/>");
  wm.addParameter(currentPassword4);

  currentPassword5 = (currentPasswordValue == 5) ? new WiFiManagerParameter("<input type='radio' id='choice5' name='current_password' value='5' checked><label for='choice5'> Password 5</label><br/><br/>")
  : new WiFiManagerParameter("<input type='radio' id='choice5' name='current_password' value='5'><label for='choice5'> Password 5</label><br/><br/>");
  wm.addParameter(currentPassword5);

  password1 = new WiFiManagerParameter(PASSWORD1_KEY, PASSWORD1_TITLE, password1Value, 20);
  wm.addParameter(password1);

  password2 = new WiFiManagerParameter(PASSWORD2_KEY, PASSWORD2_TITLE, password2Value, 20);
  wm.addParameter(password2);

  password3 = new WiFiManagerParameter(PASSWORD3_KEY, PASSWORD3_TITLE, password3Value, 20);
  wm.addParameter(password3);

  password4 = new WiFiManagerParameter(PASSWORD4_KEY, PASSWORD4_TITLE, password4Value, 20);
  wm.addParameter(password4);

  password5 = new WiFiManagerParameter(PASSWORD5_KEY, PASSWORD5_TITLE, password5Value, 20);
  wm.addParameter(password5);

  passwordAutoDelay = new WiFiManagerParameter(PASSWORD_AUTO_DELAY_KEY, PASSWORD_AUTO_DELAY_TITLE, passwordAutoDelayValue, 5);
  wm.addParameter(passwordAutoDelay);

  passwordAuto = (passwordAutoValue == 1) ? new WiFiManagerParameter(PASSWORD_AUTO_KEY, PASSWORD_AUTO_TITLE, "1", 2, "type='checkbox' checked", WFM_LABEL_AFTER)
  : new WiFiManagerParameter(PASSWORD_AUTO_KEY, PASSWORD_AUTO_TITLE, "1", 2, "type='checkbox'", WFM_LABEL_AFTER);
  wm.addParameter(passwordAuto);
}

void autoStartPassword() {
  if (passwordAutoValue == 0) {
    return;
  }
  int passwordAutoDelay = atoi(passwordAutoDelayValue);
  if (passwordAutoDelay > 0) {
    delay(passwordAutoDelay * 1000);
  }
  handleClick();
}

void loop() {
  if (isApMode) {
    wm.process();
  }
  button.tick();
  timer.run();
}
