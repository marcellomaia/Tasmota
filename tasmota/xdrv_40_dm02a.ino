/*
  xdrv_40_DM02A.ino - custom dimmer support for Tasmota

  Copyright (C) 2020  Theo Arends

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_LIGHT
#ifdef USE_DM02A_MODULE

#define XDRV_40                   40

#define EN_PIN  4
#define CH_PIN  2
#define SIG_PIN 12

#undef ENABLE_FEEDBACK
#define SDRV40_LOG_LEVEL LOG_LEVEL_INFO
//#define SDRV40_LOG_LEVEL LOG_LEVEL_DEBUG

#include <DM02A.h>  //Inclui a biblioteca para comunicar com o dimmer DM02A
DM02A dimmer(SIG_PIN, CH_PIN, EN_PIN);//SIG, CH, EN - Se não usar o EN, deixar sem informar

struct CUSTOMDUALDIMMER {
  uint8_t dimmer = 70;     
} dimmerData[2];

/********************************************************************************************/


/********************************************************************************************/
#ifdef ENABLE_FEEDBACK

void DM02AInitState(){
    for(int i=0 ; i< 2; i++) {
        uint8_t level = dimmer.feedback(i);

        AddLog_P2(SDRV40_LOG_LEVEL, PSTR("DuamDimmerInitState Got Level %d on channel %d"), level, i);
        if ( false && level > 0 ) {
            if ( dimmerData[i].dimmer != level || !dimmerData[i].IsOn ) {
                dimmerData[i].dimmer = level;

                level = map(level, 0, 70, Settings.dimmer_hw_min, Settings.dimmer_hw_max );
                char scmnd[20];
                snprintf_P(scmnd, sizeof(scmnd), PSTR(D_CMND_CHANNEL "%d %d"), (i+1), level);
                ExecuteCommand(scmnd, SRC_SWITCH);
                AddLog_P2(SDRV40_LOG_LEVEL, scmnd);
                AddLog_P2(SDRV40_LOG_LEVEL, PSTR("DuamDimmerInitState Update Level for %d to %d"), i, level);

                if ( !dimmerData[i].IsOn ) {
                    dimmerData[i].IsOn = true;
                    AddLog_P2(LOG_LEVEL_DEBUG, PSTR("SEND POWER ON, Channel: %d"), i);
                    ExecuteCommandPower(i+1, 1, SRC_SWITCH);
                }
            }
        } else {
            if ( dimmerData[i].IsOn ) {
                AddLog_P2(SDRV40_LOG_LEVEL, PSTR("SEND POWER ON, Channel: %d"), i);
                ExecuteCommandPower(i+1, 0, SRC_SWITCH);
                dimmerData[i].IsOn = false;
            }
        }
    }
}
#endif

bool SendPower(){
  bool channelState[] = { ((XdrvMailbox.index&1)!=0), ((XdrvMailbox.index&2)!=0) };
  AddLog_P2(SDRV40_LOG_LEVEL, PSTR("SendPower cmnd:%d index:%d "), XdrvMailbox.command_code, XdrvMailbox.index );
  if ( XdrvMailbox.command )
    AddLog_P2(SDRV40_LOG_LEVEL, XdrvMailbox.command );

  for( int i=0; i<2; i++) {
    dimmer.EnviaNivel( (channelState[i]) ? dimmerData[i].dimmer : 0, i);
  }
}

bool SetDimmerLevel(){
    AddLog_P2(SDRV40_LOG_LEVEL, PSTR("SetDimmerLevel (Cmd: %d, Payload: %d, index:%d"), XdrvMailbox.command_code, XdrvMailbox.payload, XdrvMailbox.index );
    if ( XdrvMailbox.command )
        AddLog_P2(SDRV40_LOG_LEVEL, XdrvMailbox.command );

    if ( XdrvMailbox.command_code == 12 && XdrvMailbox.index <= 1 ) {
        uint8_t level = map(XdrvMailbox.payload, Settings.dimmer_hw_min, Settings.dimmer_hw_max, 0, 70);
        dimmerData[XdrvMailbox.index].dimmer = level;

        dimmer.EnviaNivel(dimmerData[XdrvMailbox.index].dimmer, XdrvMailbox.index);
    } else if ( XdrvMailbox.command_code == 0 ) {
        uint8_t channels[5];
        light_state.getChannelsRaw((uint8_t*)&channels);
        AddLog_P2(SDRV40_LOG_LEVEL, PSTR("Initialize Dimmer Levels:(%d, %d) - Original"), channels[0], channels[1]);
        channels[0] = map(channels[0], 0, 255, 0, 70);
        channels[1] = map(channels[1], 0, 255, 0, 70);
        AddLog_P2(SDRV40_LOG_LEVEL, PSTR("Initialize Levels:(%d, %d) - Mapped"), channels[0], channels[1]);
    }
}


bool DM02AModuleSelected(void)
{
    pinMode(SIG_PIN, OUTPUT);
    pinMode(CH_PIN, OUTPUT);
    pinMode(EN_PIN, OUTPUT);

/*
	digitalWrite(CH_PIN,LOW);	//Inicia selecionando o canal 1
	digitalWrite(SIG_PIN,HIGH);	//Previne o envio de pulso
	digitalWrite(EN_PIN,LOW);	//Inicia com a placa desativada
*/
    AddLog_P2(LOG_LEVEL_INFO, PSTR("INIT SIG: %d, CH: %d, EN:%d"), SIG_PIN, CH_PIN, EN_PIN);

    devices_present++;
    devices_present++;
    
    light_type = LT_PWM2;
    
    //setoption68 ON
    Settings.flag3.pwm_multi_channels = 1;

    return true;
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/
#ifdef ENABLE_FEEDBACK
int interval = 0;
#endif

bool Xdrv40(uint8_t function)
{
  bool result = false;

  if (DM02A_MODULE == my_module_type) {
    switch (function) {
#ifdef ENABLE_FEEDBACK        
      case FUNC_EVERY_SECOND:
        if ( interval == 0 ) {
            DM02AInitState();
            interval = 10;
        }
        interval--;
        break;
#endif        
      case FUNC_SET_DEVICE_POWER:
        AddLog_P2(SDRV40_LOG_LEVEL, PSTR("FUNC_SET_DEVICE_POWER"));
        result = SendPower();
        break;
      case FUNC_SET_CHANNELS:
        AddLog_P2(SDRV40_LOG_LEVEL, PSTR("FUNC_SET_CHANNELS"));
        result = SetDimmerLevel();
        break;
      case FUNC_MODULE_INIT:
        result = DM02AModuleSelected();
        break;
    }
  }
  return result;
}

#endif  // USE_DM02A_MODULE
#endif  // USE_LIGHT
