//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// C-Styled functions to create a dump for transport
//
// This functiosn can dump the measured profile data
// into a .001 file that can be interpreted by transport
// an external program
//
// @Author: Anton Metzger
// @Maintainer: Jochem Snuvernik

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "data_dump.h"

#define True                                    (1==1)
#define False                                    !True

static void RemoveLeadingBlanks(char * *s)
{
  if ((s == NULL) || (*s == NULL))
    return;

  while (isspace(* *s) && (* *s != '\0'))
    (*s)++;

}

static void RemoveTrailingBlanks(char *s)
{
  char            *t;

  if ((s == NULL) || (strlen(s) == 0))
    return;

  t = s + strlen(s) - 1;
  while ((*t == ' ') && (t > s)) {
    *t = '\0';
    t--;
  }
}

static void getStringComponent(char *inStr, char delim, char *outStr)
{
  size_t i;
  int j, pos = - 1;

  *outStr = '\0';
  for (i = 0; i < strlen(inStr); i++) {
    if (inStr[i] == delim) {
      pos = i;
      break;
    }
  }
  if (pos == - 1)
    return;

  for (j = 0, i = pos + 1; (inStr[i] != delim) && (inStr[i] != '\0'); i++, j++)
    outStr[j] = inStr[i];

  outStr[j] = '\0';
}

static int getTransportToken(char *inStr, float *Code)
{
  return (sscanf(inStr, "%f", Code));
}

static FILE *fpta;

static int getDevicefromLabel(char *bl, char *label, char *device)
{
  char line[80], alias[40], name[40], bl1[20], bl2[20], bl3[20];
  rewind(fpta);
  while (!feof(fpta)) {
    fgets(line, sizeof(line), fpta);
    sscanf(line, "%s %s %s %s %s", bl1, bl2, bl3, name, alias);
    if((strncmp(bl, bl1, 2) == (char) 0) || (strncmp(bl, bl2, 2) == (char) 0) || (strncmp(bl, bl3, 2) == (char) 0)) {
      if(strcmp(label, alias) == (char) 0) {
        strcpy(device, name);
        return True;
      }
    }
  }
  return False;
}

static int getValueforDevice(char *Quad, int nbDevs, str9 *Quads, int *ptr)
{
  int i;
  for (i=0; i<nbDevs; i++) {
    if(strcmp(Quad, Quads[i]) == (char) 0) {
      *ptr = i;
      return True;
    }
  }
  return False;
}

static int getValueforProfile(char *Prof, int nbProfs, str9 *Profs, int *ptr)
{
  int i;
  for (i=0; i<nbProfs; i++) {
    if(strcmp(Prof, Profs[i]) == (char) 0) {
      *ptr = i;
      return True;
    }
  }
  return False;
}

char* strUpr(char *str)
{
   char           *ptr = str;

   while ((*ptr = (char) toupper(*ptr)) != '\0')
      ptr++;
   return (str);
}

char* strLwr(char* str)
{
   char           *ptr = str;

   while ((*ptr = (char) tolower(*ptr)) != '\0')
      ptr++;
   return (str);
}

int updateTransportFile(char *transLine, int nbDevs, str9 *Quads, int *QuadsSign, float *values,
                        str50 *qerrMsg, int nbProfs, str9 *Profs, str50 *perrMsg, float *sigma2,
                        char *broDev, char *broUnit, float broVal, char *actualTime,
                        char *fileName, char *message,
                        int *nbHor, float *sigma2h, int *nbVer, float *sigma2v)
{
  char            *dataFolder  = getenv("TRANSDATA");
  char            *transFolder = getenv("TRANSMESS");
  char            aliasFile[256], blrefsFile[256];
  int             nbline = 0;
  int             found = False;
  FILE            *fptr, *fpto;
  char            line[512], label[80];
  char            *temp;
  float           code;
  char            beamline[20], bl[20], for001File[20], for004File[20];
  char            file[80];
  char            mess[10000];

  int             firstImpuls = True;

  float           impuls;

  if(strstr(broUnit, "MeV/c") != (char *) 0) {
    impuls = broVal / 1000.0 ;     /* MeV/c ==> GeV/c */
  }
  else {
    impuls = broVal / 33.356;      /* kGaussm ==> 10**6/c=3.33564 10-3 */
  }

  if(broVal < 0.01) firstImpuls = False;        /* not valid */

  strcpy(beamline, transLine);
  strUpr(beamline);
  sprintf(blrefsFile, "%s/blrefs.dat", dataFolder);
  if ((fptr = fopen(blrefsFile, "r")) == (FILE *)0) {
    sprintf(mess, "file %s open error\n", blrefsFile);
    return -1;
  }
  sprintf(aliasFile, "%s/alias.dat", dataFolder);
  if ((fpta = fopen(aliasFile, "r")) == (FILE *)0) {
    sprintf(mess, "file %s open error\n", aliasFile);
    return -2;
  }
  
  /* scan the file */

  while (!feof(fptr)) {
    fgets(line, sizeof(line), fptr);
    sscanf(line, "%s %s %s", bl, for001File, for004File);
    if(strcmp(beamline, bl) == (char) 0) {
      //        printf("found for beamline %s files %s %s\n", beamline, for001File, for004File);
      found = True;
      break;
    }
  }
  fclose(fptr);

  if(!found) {
    sprintf(mess, "\nfailed to find the name of the reference file for beamline=%s in blrefs.dat\n", beamline);
    return -3;
  }

  /* copy display file to correct spot */

  strLwr(for004File);
  sprintf(file, "%s/TransEnvFit/%s", dataFolder, for004File);
  if ((fptr = fopen(file, "r")) == (FILE *)0) {
    sprintf(mess, "file %s open error\n", file);
    return -4;
  } else {
    sprintf(mess, "file %s is used for TRANSPORT\n", file);
  }
  sprintf(file, "%s/FOR004.DAT", transFolder);
  if ((fpto = fopen(file, "w")) == (FILE *)0) {
    sprintf(mess, "file for004.dat open error\n");
    return -5;
  }
  while (!feof(fptr)) {
    fgets(line, sizeof(line), fptr);
    temp = line;
    RemoveLeadingBlanks(&temp);
    fprintf(fpto, "%s", temp);
  }
  fclose(fptr);
  fclose(fpto);

  /* get the original transport envelope fit file and replace the measured values */
  strLwr(for001File);
  sprintf(file, "%s/TransEnvFit/%s", dataFolder, for001File);
  if ((fptr = fopen(file, "r")) == (FILE *)0) {
    sprintf(mess, "file %s open error\n", file);
    return -6;
  } else {
    sprintf(mess, "%sfile %s is used for TRANSPORT\n\n", mess, file);
  }
  sprintf(file, "%s/FOR001.DAT", transFolder);
  if ((fpto = fopen(file, "w")) == (FILE *)0) {
    sprintf(mess, "file for001.dat open error\n");
    return -7;
  }

  /* scan the file */

  nbline = 0;
  while (!feof(fptr)) {
    fgets(line, sizeof(line), fptr);

    /* remove linefeed */

    if (line[strlen(line) - 1] == '\n')
      line[strlen(line) - 1] = '\0';

    /* remove leading and trailing blancs */

    RemoveTrailingBlanks(line);
    temp = line;
    RemoveLeadingBlanks(&temp);
    if (strlen(temp) != 0) {
      //printf("read <%s> %x\n", line,line[strlen(line) - 1] );
      getStringComponent(temp, '/', label);
      //printf("consider label=%s\n", label);
      nbline++;
      if (getTransportToken(temp, &code)) {
        int             Tcode = (int) code;
        switch (Tcode) {

          /* we have to treat impuls */

        case 1: {
          int             num;
          float           f1, f2, f3, f4, f5, f6, f7, f8;
          if(!firstImpuls) {
            fprintf(fpto, "%s\n", temp);
          } else {
            firstImpuls = False;
            num = sscanf(temp, "%f %f %f %f %f %f %f %f", &f1, &f2, &f3, &f4, &f5,
                         &f6, &f7, &f8);
            if (num == 8) {
              sprintf(mess, "%sreplace impuls with value %f GeV/c, ", mess, impuls);
              sprintf(mess, "%sdata taken from %s with bro=%.3f %s\n",
                      mess, broDev, broVal, broUnit);
              fprintf(fpto, "%.0f. %.3f %.3f %.3f %.3f %.3f %.3f %.3f /%s/ ;\n",
                      f1,f2,f3,f4,f5,f6,f7,impuls,label);
            } else {
              fprintf(fpto, "%s\n", temp);
            }
          }
          break;
        }

          /* we have a quadrupole card */

        case 5: {
          int             num;
          float           f1, f2, f3, f4;
          char            device[20];

          num = sscanf(temp, "%f %f %f %f", &f1, &f2, &f3, &f4);
          if (num == 4) {
            //printf("replace quad for label %s\n", label);
            if(getDevicefromLabel(beamline, label, device)) {
              int ptr;
              if(getValueforDevice(device, nbDevs, Quads, &ptr)) {
                fprintf(fpto, "%.0f. %.3f %.3f %.3f /%s/ ;\n", f1,f2,values[ptr]* QuadsSign[ptr],f4, label);
                sprintf(mess, "%sdevice %8s with value=%8.3f to label <%s> %s\n",
                        mess, device, values[ptr] * QuadsSign[ptr], label, qerrMsg[ptr]);
              } else {
                sprintf(mess, "%sdevice %8s with value %8.3f could not be attributed to <%s>\n",
                        mess, device, values[ptr], label);
                fprintf(fpto, "%s\n", temp);
              }
            }
            else {
              sprintf(mess, "%scould not get associated device for label <%s>\n", mess, label);
              fprintf(fpto, "%s\n", temp);
            }
          } else {
            fprintf(fpto, "%s\n", temp);
          }
          break;
        }

          /* we have a solenoid card */

        case 19: {
          int             num;
          float           f1, f2, f3;
          char            device[20];

          num = sscanf(temp, "%f %f %f", &f1, &f2, &f3);
          if (num == 3) {
            printf("replace quad for label %s\n", label);
            if(getDevicefromLabel(beamline, label, device)) {
              int ptr;
              if(getValueforDevice(device, nbDevs, Quads, &ptr)) {
                fprintf(fpto, "%.0f. %.3f %.3f /%s/ ;\n", f1,f2,values[ptr],label);
                sprintf(mess, "%sdevice %8s with value=%8.3f to label <%s> %s\n",
                        mess, device, values[ptr], label, qerrMsg[ptr]);
              } else {
                sprintf(mess, "%sdevice %8s with value %8.3f could not be attributed to <%s>\n",
                        mess, device, values[ptr], label);
                fprintf(fpto, "%s\n", temp);
              }
            }
            else {
              sprintf(mess, "%scould not get associated device for label <%s>\n", mess, label);
              fprintf(fpto, "%s\n", temp);
            }
          } else {
            fprintf(fpto, "%s\n", temp);
          }
          break;
        }
          /* we have profile or other fit condition */

        case - 10:
        case 10: {
          int             num;
          float           f1, f2, f3, f4, f5;
          char            device[20];

          num = sscanf(temp, "%f %f %f %f %f", &f1, &f2, &f3, &f4, &f5);

          if (num == 5) {
            /* transport flag for horiz/vert */
            int             fla1 = f2;
            int             fla2 = f3;

            if (((fla1 == 1) && (fla2 == 1)) || ((fla1 == 3) && (fla2 == 3)) ){
              float value;
              /*
                if(fla1 == 1)
                printf("replace horiz profil for label %s\n", label);
                else
                printf("replace vert profil for label %s\n", label);
              */
              if(getDevicefromLabel(beamline, label, device)) {
                int ptr;
                int error = False;
                if(getValueforProfile(device, nbProfs, Profs, &ptr)) {
                  if((int) (sigma2[ptr]+0.001) == 999) error = True;
                  if(error) {
                    value =0.0;
                    f5 = 50.0;
                  } else {
                    value = sigma2[ptr];
                    if(fla1 == 1) {
                      if(error) sigma2h[*nbHor] = 0; else sigma2h[*nbHor] = value;
                      *nbHor = *nbHor + 1;
                    } else {
                      if(error) sigma2v[*nbVer] = 0; else sigma2v[*nbVer] = value;
                      *nbVer = *nbVer + 1;
                    }
                  }
                  fprintf(fpto, "%.0f. %.0f. %.0f. %.3f %.3f /%s/ ;\n",
                          f1, f2, f3, value, f5, label);
                  sprintf(mess, "%sdevice %8s with value=%8.3f to label <%s> %s\n",
                          mess, device, value, label, perrMsg[ptr]);

                } else {
                  fprintf(fpto, "%.0f. %.0f. %.0f. %.3f %.3f /%s/ ;\n",
                          f1, f2, f3, 0.0, 1000.0, label);
                  sprintf(mess,"%sprofil %8s could not be found for label <%s>\n",
                          mess, device, label);
                  //fprintf(fpto, "%s\n", temp);
                  if(fla1 == 1) {
                    if(error) sigma2h[*nbHor] = 0; else sigma2h[*nbHor] = value;
                    *nbHor = *nbHor + 1;
                  } else {
                    if(error) sigma2v[*nbVer] = 0; else sigma2v[*nbVer] = value;
                    *nbVer = *nbVer + 1;
                  }
                }
              }
              else {
                sprintf(mess,"%scould not get associated device for label <%s>\n", mess, label);
                fprintf(fpto, "%s\n", temp);
              }
            } else {
              fprintf(fpto, "%s\n", temp);
            }
          }  else {
            fprintf(fpto, "%s\n", temp);
          }
          break;
        }
        default:
          fprintf(fpto, "%s\n", temp);
        }
      }
      /* no legal transport code found */

      else {
        if(nbline == 1) {
          temp[strlen(temp) - 1] = '\0';
          fprintf(fpto,"%s measured @ %s %s/\n", temp, actualTime, message);
        }
        else
          fprintf(fpto, "%s\n", temp);
      }
    }
  }

  fclose(fpto);

  /* copy the files for001 and for004 to a sure spot with a time stamp */

  sprintf(file, "%s/FOR001.DAT", transFolder);
  if ((fptr = fopen(file, "r")) == (FILE *)0) {
    printf("file FOR001.DAT open error\n");
    return -9;
  }
  sprintf(file, "%s.001", fileName);
  if ((fpto = fopen(file, "w")) == (FILE *)0) {
    printf("file %s open error\n", file);
    return -10;
  }
  while (!feof(fptr)) {
    fgets(line, sizeof(line), fptr);
    fprintf(fpto, "%s", line);
  }
  fclose(fptr);
  fclose(fpto);

  return 1;
}
