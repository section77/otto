//*************************************************************************
//*
//* initial sketch for the s77 otto building workshop from 07.08.2019
//*
//*
//* - depends on 'SerialUI' in version 1.14.0
//*   (Sketch / Include Library / Manage Libraries ... / SerialUI)
//*
//*
//* if you cover otto's eyes on startup, this sketch starts
//* the 'Otto DIY Calibrarion Tool' mode.
//* otherwise otto dances on initialization and waits if he
//* detects some obstacles. if he detects one, he walks some steps back.
//*
//*
//*
//* based of:
//*
//*  Otto DIY Calibration Tool
//*  (c) 16/07/2016 - Rafael López Verdejo
//*  rlopezverdejo@gmail.com
//*  <http://www.ottodiy.com/.
//*  This program is free software: you can redistribute it and/or modify
//*  it under the terms of the GNU General Public License as published by
//*  the Free Software Foundation, either version 3 of the License, or
//*  (at your option) any later version.
//*
//*  This program is distributed in the hope that it will be useful,
//*  but WITHOUT ANY WARRANTY; without even the implied warranty of
//*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//*  GNU General Public License for more details.
//*
//*  You should have received a copy of the GNU General Public License
//*  along with this program. If not, see <http://www.gnu.org/licenses/>.
//*
//*************************************************************************

#include <Servo.h>
#include <Oscillator.h>
#include <EEPROM.h>
#include <SerialUI.h>
#include <Otto.h>
Otto Otto;  //This is Otto!

#define N_SERVOS 4 // # servos

#define RA_ID 0 // Right ankle ID
#define LA_ID 1 // Left ankle ID
#define RH_ID 2 // Right hip ID
#define LH_ID 3 // Left hip ID

#define RA_PIN 5 // Right ankle PIN
#define LA_PIN 4 // Left ankle PIN
#define RH_PIN 3 // Right hip PIN
#define LH_PIN 2 // Left hip PIN
  #define PIN_YL 2 //servo[2]
  #define PIN_YR 3 //servo[3]
  #define PIN_RL 4 //servo[4]
  #define PIN_RR 5 //servo[5]



#define EEPROM_BASE_ADDRESS 0

// MENU CONFIGURATION

SUI_DeclareString(device_greeting, "Otto's calibration tool\r\nCopyright (C) 2016 Rafael Lopez Verdejo [rlopezverdejo@gmail.com] \r\nGPL License\r\nWrite ? for help.");
SUI_DeclareString(top_menu_title, "Main menu");

SUI_DeclareString(la_key, "la");
SUI_DeclareString(la_help, "Left ankle trim");
SUI_DeclareString(la_plus_key, "+");
SUI_DeclareString(la_plus_help, "Left ankle trim +1");
SUI_DeclareString(la_minus_key, "-");
SUI_DeclareString(la_minus_help, "Left ankle trim -1");

SUI_DeclareString(ra_key, "ra");
SUI_DeclareString(ra_help, "Right ankle trim");
SUI_DeclareString(ra_plus_key, "+");
SUI_DeclareString(ra_plus_help, "Right ankle trim +1");
SUI_DeclareString(ra_minus_key, "-");
SUI_DeclareString(ra_minus_help, "Right ankle trim -1");

SUI_DeclareString(lh_key, "lh");
SUI_DeclareString(lh_help, "Left hip trim");
SUI_DeclareString(lh_plus_key, "+");
SUI_DeclareString(lh_plus_help, "Left hip trim +1");
SUI_DeclareString(lh_minus_key, "-");
SUI_DeclareString(lh_minus_help, "Left hip trim -1");

SUI_DeclareString(rh_key, "rh");
SUI_DeclareString(rh_help, "Right hip trim");
SUI_DeclareString(rh_plus_key, "+");
SUI_DeclareString(rh_plus_help, "Right hip trim +1");
SUI_DeclareString(rh_minus_key, "-");
SUI_DeclareString(rh_minus_help, "Right hip trim -1");

SUI_DeclareString(info_key, "info");
SUI_DeclareString(info_help, "Show the trim values loaded into memory");

SUI_DeclareString(save_key, "save");
SUI_DeclareString(save_help, "Save memory trim values to EEPROM");

SUI_DeclareString(load_key, "load");
SUI_DeclareString(load_help, "Load memory trim values from EEPROM");

SUI_DeclareString(reset_key, "reset");
SUI_DeclareString(reset_help, "Reset memory trim values");

SUI::SerialUI mySUI = SUI::SerialUI(device_greeting);

Oscillator servo[N_SERVOS];



boolean obstacleDetected(){
   int distance = Otto.getDistance();
   Serial.print("distance: ");
   Serial.print(distance);
   Serial.println();
   if(distance < 15) {
     return true;
   } else {
     return false;
   }
}

bool calibrationMode = false;

void setup()
{
  Otto.init(PIN_YL,PIN_YR,PIN_RL,PIN_RR,true);
  Otto.home();
  if(obstacleDetected()){
    calibrationMode = true;
    calibration();
  } else {
    wakeupDance();
  }
}

void wakeupDance() {

  Otto.jitter(10);
  Otto.home();
  delay(1000);

  Otto.moonwalker(10, 900, 30);
  Otto.home();
  delay(1000);

  Otto.tiptoeSwing(10, 900, 30);
}
void calibration() {
  mySUI.begin(9600);
  mySUI.setMaxIdleMs(60000);

  servo[RA_ID].attach(RA_PIN);
  servo[LA_ID].attach(LA_PIN);
  servo[RH_ID].attach(RH_PIN);
  servo[LH_ID].attach(LH_PIN);

  resetServos();

  SUI::Menu * mainMenu = mySUI.topLevelMenu();
  mainMenu->setName(top_menu_title);
  SUI::Menu * laMenu = mainMenu->subMenu(la_key, la_help);
  laMenu->addCommand(la_plus_key, CB_leftAnklePlus, la_plus_help);
  laMenu->addCommand(la_minus_key, CB_leftAnkleMinus, la_minus_help);
  SUI::Menu * raMenu = mainMenu->subMenu(ra_key, ra_help);
  raMenu->addCommand(ra_plus_key, CB_rightAnklePlus, ra_plus_help);
  raMenu->addCommand(ra_minus_key, CB_rightAnkleMinus, ra_minus_help);
  SUI::Menu * lhMenu = mainMenu->subMenu(lh_key, lh_help);
  lhMenu->addCommand(lh_plus_key, CB_leftHipPlus, lh_plus_help);
  lhMenu->addCommand(lh_minus_key, CB_leftHipMinus, lh_minus_help);
  SUI::Menu * rhMenu = mainMenu->subMenu(rh_key, rh_help);
  rhMenu->addCommand(rh_plus_key, CB_rightHipPlus, rh_plus_help);
  rhMenu->addCommand(rh_minus_key, CB_rightHipMinus, rh_minus_help);
  mainMenu->addCommand(info_key, CB_info, info_help);
  mainMenu->addCommand(save_key, CB_save, save_help);
  mainMenu->addCommand(load_key, CB_load, load_help);
  mainMenu->addCommand(reset_key, CB_reset, reset_help);
}

void loop()
{

  if(calibrationMode) {
    if (mySUI.checkForUser(150)) {
      mySUI.enter();

      while (mySUI.userPresent())
      {
        mySUI.handleRequests();
      }
    }
  } else {
     if(obstacleDetected()) {
        // go back
        Otto.walk(1,1000,-1);
     }
  }
}

/************ CALLBACKS *************/

void CB_leftAnklePlus()
{
  servo[LA_ID].SetTrim(servo[LA_ID].getTrim()+1);
  servo[LA_ID].SetPosition(90);
}

void CB_leftAnkleMinus()
{
  servo[LA_ID].SetTrim(servo[LA_ID].getTrim()-1);
  servo[LA_ID].SetPosition(90);
}

void CB_rightAnklePlus()
{
  servo[RA_ID].SetTrim(servo[RA_ID].getTrim()+1);
  servo[RA_ID].SetPosition(90);
}

void CB_rightAnkleMinus()
{
  servo[RA_ID].SetTrim(servo[RA_ID].getTrim()-1);
  servo[RA_ID].SetPosition(90);
}

void CB_leftHipPlus()
{
  servo[LH_ID].SetTrim(servo[LH_ID].getTrim()+1);
  servo[LH_ID].SetPosition(90);
}

void CB_leftHipMinus()
{
  servo[LH_ID].SetTrim(servo[LH_ID].getTrim()-1);
  servo[LH_ID].SetPosition(90);
}

void CB_rightHipPlus()
{
  servo[RH_ID].SetTrim(servo[RH_ID].getTrim()+1);
  servo[RH_ID].SetPosition(90);
}

void CB_rightHipMinus()
{
  servo[RH_ID].SetTrim(servo[RH_ID].getTrim()-1);
  servo[RH_ID].SetPosition(90);
}

void CB_info()
{
  showTrims();
}

void CB_save()
{
  // SAVE EEPROM VALUES

  for (int i = 0; i < N_SERVOS; i++) {
    EEPROM.write(EEPROM_BASE_ADDRESS + i, servo[i].getTrim());
  }

  mySUI.println("Trim values saved");
}

void CB_load()
{
  int trim;

  // LOAD EEPROM VALUES

  for (int i = 0; i < N_SERVOS; i++)
  {
    trim = EEPROM.read(EEPROM_BASE_ADDRESS + i);
    if (trim > 128) trim = trim - 256;
    servo[i].SetTrim(trim);
  }

  showTrims();
  centerServos();
}

void CB_reset()
{
  resetServos();
  showTrims();
  centerServos();
}


/************ FUNCTIONS *************/

void centerServos()
{
  for (int i = 0; i < N_SERVOS; i++) servo[i].SetPosition(90);
  delay(200);
}

void resetServos()
{
  // RESET MEMORY VALUES

  for (int i = 0; i < N_SERVOS; i++)
  {
    servo[i].SetTrim(0);
  }
}

void showTrims()
{
  mySUI.print("Right ankle: ");
  mySUI.println(servo[RA_ID].getTrim());
  mySUI.print("Left ankle: ");
  mySUI.println(servo[LA_ID].getTrim());
  mySUI.print("Right hip: ");
  mySUI.println(servo[RH_ID].getTrim());
  mySUI.print("Left hip: ");
  mySUI.println(servo[LH_ID].getTrim());
}

