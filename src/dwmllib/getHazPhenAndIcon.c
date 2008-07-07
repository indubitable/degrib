/******************************************************************************
 * getHazPhenAndIcon() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE  
 *   Get the English translations for the NDFD hazard phenomena part of the
 *   "hazard ugly string". For example, NDFD coverage "SC" is converted to
 *   its english equivilant "Small Craft". Also, if phenomena has an icon,
 *   get that too.
 *
 *   
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD hazard phenomena. (Input)
 * transStr = Outgoing string with the translated phenomena. (Output)
 *   f_icon = Denotes if this hazard phenomena has an associated icon. (Output).
 *  iconStr = Outgoing icon string if phenomena has an icon. (Output).
 *    hzSig = Significance portion of hazard. Used to determine icon. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   6/2008 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void  getHazPhenAndIcon(char *uglyStr, char *transStr, int *f_icon, 
                        char *iconStr, char *hzSig)
{
   if (strcmp(uglyStr, "AF") == 0)
   {
      strcpy(transStr, "Ashfall");
      return;
   }

   else if (strcmp(uglyStr, "AS") == 0)
   {
      strcpy(transStr, "Air Stagnation");
      return;
   }

   else if (strcmp(uglyStr, "BS") == 0)
   {
      strcpy(transStr, "Blowing Snow");
      return;
   }

   else if (strcmp(uglyStr, "BW") == 0)
   {
      strcpy(transStr, "Brisk Wind");
      return;
   }

   else if (strcmp(uglyStr, "BZ") == 0)
   {
      strcpy(transStr, "Blizzard");
      return;
   }

   else if (strcmp(uglyStr, "CF") == 0)
   {
      strcpy(transStr, "Coastal Flood");
      return;
   }

   else if (strcmp(uglyStr, "DS") == 0)
   {
      strcpy(transStr, "Dust Storm");
      return;
   }

   else if (strcmp(uglyStr, "DU") == 0)
   {
      strcpy(transStr, "Blowing Dust");
      return;
   }

   else if (strcmp(uglyStr, "EC") == 0)
   {
      strcpy(transStr, "Extreme Cold");
      return;
   }

   else if (strcmp(uglyStr, "EH") == 0)
   {
      strcpy(transStr, "Excessive Heat");
      return;
   }

   else if (strcmp(uglyStr, "FA") == 0)
   {
      strcpy(transStr, "Areal Flood");
      return;
   }

   else if (strcmp(uglyStr, "FF") == 0)
   {
      strcpy(transStr, "Flash Flood");
      return;
   }

   else if (strcmp(uglyStr, "FG") == 0)
   {
      strcpy(transStr, "Dense Fog");
      return;
   }

   else if (strcmp(uglyStr, "FR") == 0)
   {
      strcpy(transStr, "Frost");
      return;
   }

   else if (strcmp(uglyStr, "FW") == 0)
   {
      strcpy(transStr, "Fire Weather");
      return;
   }

   else if (strcmp(uglyStr, "FZ") == 0)
   {
      strcpy(transStr, "Freeze");
      return;
   }

   else if (strcmp(uglyStr, "GL") == 0)
   {
      strcpy(transStr, "Gale");
      *f_icon = 1;
      strcpy(iconStr, "mf_gale.gif");
      return;
   }

   else if (strcmp(uglyStr, "HF") == 0)
   {
      strcpy(transStr, "Hurricane Force Wind");
      *f_icon = 1;
      strcpy(iconStr, "mf_hurr.gif");
      return;
   }

   else if (strcmp(uglyStr, "HI") == 0)
   {
      strcpy(transStr, "Hurricane Wind");
      *f_icon = 1;
      strcpy(iconStr, "mf_hurr.gif");
      return;
   }

   else if (strcmp(uglyStr, "HS") == 0)
   {
      strcpy(transStr, "Heavy Snow");
      return;
   }

   else if (strcmp(uglyStr, "HT") == 0)
   {
      strcpy(transStr, "Heat");
      return;
   }

   else if (strcmp(uglyStr, "HU") == 0)
   {
      strcpy(transStr, "Hurricane");
      return;
   }

   else if (strcmp(uglyStr, "HW") == 0)
   {
      strcpy(transStr, "High Wind");
      return;
   }

   else if (strcmp(uglyStr, "HZ") == 0)
   {
      strcpy(transStr, "Hard Freeze");
      return;
   }

   else if (strcmp(uglyStr, "IP") == 0)
   {
      strcpy(transStr, "Sleet");
      return;
   }

   else if (strcmp(uglyStr, "IS") == 0)
   {
      strcpy(transStr, "Ice Storm");
      return;
   }

   else if (strcmp(uglyStr, "LB") == 0)
   {
      strcpy(transStr, "Lake Effect Snow and Blowing Snow");
      return;
   }

   else if (strcmp(uglyStr, "LE") == 0)
   {
      strcpy(transStr, "Lake Effect");
      return;
   }

   else if (strcmp(uglyStr, "LO") == 0)
   {
      strcpy(transStr, "Low Water");
      return;
   }

   else if (strcmp(uglyStr, "LS") == 0)
   {
      strcpy(transStr, "Lakeshore Flood");
      return;
   }

   else if (strcmp(uglyStr, "LW") == 0)
   {
      strcpy(transStr, "Lake Wind");
      return;
   }

   else if (strcmp(uglyStr, "MA") == 0)
   {
      strcpy(transStr, "Marine");
      return;
   }

   else if (strcmp(uglyStr, "RB") == 0)
   {
      strcpy(transStr, "Small Craft, for Rough Bar");
      *f_icon = 1;
      strcpy(iconStr, "mf_smcraft.gif");
      return;
   }

   else if (strcmp(uglyStr, "SB") == 0)
   {
      strcpy(transStr, "Snow and Blowing Snow");
      return;
   }

   else if (strcmp(uglyStr, "SC") == 0)
   {
      strcpy(transStr, "Small Craft");
      *f_icon = 1;
      strcpy(iconStr, "mf_smcraft.gif");
      return;
   }

   else if (strcmp(uglyStr, "SE") == 0)
   {
      strcpy(transStr, "Hazardous Seas");
      return;
   }

   else if (strcmp(uglyStr, "SI") == 0)
   {
      strcpy(transStr, "Small Craft, for Winds");
      *f_icon = 1;
      strcpy(iconStr, "mf_smcraft.gif");
      return;
   }

   else if (strcmp(uglyStr, "SM") == 0)
   {
      strcpy(transStr, "Dense Smoke");
      return;
   }

   else if (strcmp(uglyStr, "SN") == 0)
   {
      strcpy(transStr, "Snow");
      return;
   }

   else if (strcmp(uglyStr, "SR") == 0)
   {
      strcpy(transStr, "Storm");
      return;
   }

   else if (strcmp(uglyStr, "SU") == 0)
   {
      strcpy(transStr, "High Surf");
      return;
   }

   else if (strcmp(uglyStr, "SV") == 0)
   {
      strcpy(transStr, "Severe Thunderstorm");
      return;
   }

   else if (strcmp(uglyStr, "SW") == 0)
   {
      strcpy(transStr, "Small Craft, for Hazardous Seas");
      *f_icon = 1;
      strcpy(iconStr, "mf_smcraft.gif");
      return;
   }

   else if (strcmp(uglyStr, "TI") == 0)
   {
      strcpy(transStr, "Tropical Storm Wind");
      *f_icon = 1;
      strcpy (iconStr, "mf_storm.gif");
      return;
   }

   else if (strcmp(uglyStr, "TO") == 0)
   {
      strcpy(transStr, "Tornado");
      return;
   }

   else if (strcmp(uglyStr, "TR") == 0)
   {
      strcpy(transStr, "Tropical Storm");
      *f_icon = 1;
      strcpy (iconStr, "mf_storm.gif");
      return;
   }

   else if (strcmp(uglyStr, "TS") == 0)
   {
      strcpy(transStr, "Tsunami");
      *f_icon = 1;
      strcpy (iconStr, "m_wave.gif");
      return;
   }

   else if (strcmp(uglyStr, "TY") == 0)
   {
      strcpy(transStr, "Typhoon");
      return;
   }

   else if (strcmp(uglyStr, "UP") == 0)
   {
      strcpy(transStr, "Freezing Spray");
      return;
   }

   else if (strcmp(uglyStr, "WC") == 0)
   {
      strcpy(transStr, "Wind Chill");
      return;
   }

   else if (strcmp(uglyStr, "WI") == 0)
   {
      strcpy(transStr, "Wind");
      return;
   }

   else if (strcmp(uglyStr, "WS") == 0)
   {
      strcpy(transStr, "Winter Storm");
      return;
   }

   else if (strcmp(uglyStr, "WW") == 0)
   {
      strcpy(transStr, "Winter Weather");
      return;
   }

   else if (strcmp(uglyStr, "ZF") == 0)
   {
      strcpy(transStr, "Freezing Fog");
      return;
   }

   else if (strcmp(uglyStr, "ZR") == 0)
   {
      strcpy(transStr, "Freezing Rain");
      return;
   }

   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "none");
      return;
   }

   return;
}