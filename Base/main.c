/*
TM Creator - Little software for TrackMania files packaging
Copyright (C) 2008-2013  Cokie

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#define _WIN32_IE 0x0600
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <windows.h>
#include <commctrl.h>

#include "define.h"
#include "fonctions_repertoire.h"
#include "fonctions_process.h"
#include "../Creator/fonctions_cryptage.h"

#define MAX_CHAINE 5000



// fonctions
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);    // callback de la fenetre principale
LRESULT CALLBACK PwdProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EndDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int create_dir(char directory[]);
void centrer_fenetre(HWND hwnd1, HWND hwnd2);
int convert_from_environment(char string[], char converted[]);


//structures
typedef struct Info_file
{
        char name[100];
        char dir[MAX_CHAINE];
        int size;
} Info_file;


//variales globales, classées par type
HWND mainWnd,     //fenêtre principale
     endDialog;

HINSTANCE mainInstance;

HICON iconKey,
      iconGbx;


char userprofile[MAX_CHAINE],
     TMDestination[MAX_CHAINE] = "\0",
     TMDestinationE[MAX_CHAINE] = "\0",
     *dossierBase = NULL,
     appName[5000],
     password[MAX_CHAINE] = "\0";

int filesBegin = -1,
    numeroFichier = 0,
    nombreFichiers = 0;

FILE *executableFile = NULL;


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow
                     )

{
    /*
    Traitement de la ligne de commande
    Syntaxe obligatoire :
       => nomApplication "adresse"   (pour commande DOS)
    ou => adresse\\nomApplication
    */

    dossierBase = GetCommandLine();
    strcpy(appName, dossierBase);
    *(strstr(appName, ".exe") + 4) = '\0';
    if (strrchr(appName, '\\') != NULL)
          strcpy(appName, strrchr(appName, '\\') + 1);

    if (strchr(dossierBase, '"') != NULL)
    {
           strcpy(dossierBase, strchr(dossierBase, '"') + 1);
           *strchr(dossierBase, '"') = '\0';
    }

    while (strstr(dossierBase, ".exe") != NULL)
          *strrchr(dossierBase, '\\') = '\0';

    chdir(dossierBase);  //rétablissement du dossier de lancement de l'application

    iconKey = LoadImage(hInstance, "icoKey", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconGbx = LoadImage(hInstance, "icoGbx", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

    mainInstance = hInstance;
    CRYPT_init();

    endDialog = CreateDialog(hInstance, "end", NULL, (DLGPROC)EndDlgProc);

    if (DialogBox(hInstance, "window", NULL, (DLGPROC)WndProc))
    {
        if (test_exist_process("TMForever.exe"))
           MessageBox(NULL, "Copie terminée ! Veuillez redémarrez TrackMania !", "Information", MB_OK | MB_ICONINFORMATION);
        else MessageBox(NULL, "Copie terminée ! Bon jeu !", "Information", MB_OK | MB_ICONINFORMATION);
        ShellExecute(NULL, "explore", TMDestination, NULL, NULL, SW_SHOWNORMAL);

        ShowWindow(endDialog, SW_SHOW);

        MSG messages;
        while (GetMessage (&messages, NULL, 0, 0))
        {
            TranslateMessage(&messages);
            DispatchMessage(&messages);
        }
    }

    return 1;
}





LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                InitCommonControls();

                CreateWindowEx(
                        0,
                        PROGRESS_CLASS,
                        "",
                        WS_CHILD | PBS_SMOOTH | WS_VISIBLE,
                        15,
                        80,
                        270,
                        20,
                        hwnd,
                        (HMENU) IDPB_PROGRESS,
                        mainInstance,
                        NULL
                   );

                centrer_fenetre(hwnd, NULL);
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconGbx);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconGbx);

                char IDString[MAX_CHAINE] = "Beginning";
                strcat(IDString, "-files");

                SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, userprofile);
                //GetEnvironmentVariable("userprofile", userprofile, MAX_CHAINE);
                sprintf(TMDestination, "%s\\TrackMania", userprofile);

                if ((executableFile = fopen(appName, "rb")) == NULL)
                {
                   MessageBox(NULL, "Impossible d'extraire les fichiers.", "Erreur !", MB_OK | MB_ICONERROR);
                   EndDialog(hwnd, 0);
                   return FALSE;
                }

                void *ptv = malloc(1000000);
                int read = fread(ptv, 1, 1000000, executableFile), success = 0, i;
                char *beginning = NULL;

                for (i = 0 ; i < read && !success ; i++)
                {
                    if (*( (char*)ptv + i ) == 'B' && (beginning = strstr(ptv + i, IDString)) != NULL)
                        success = 1;
                }

                if (!success)
                {
                   MessageBox(NULL, "Fichiers à extraire introuvables.", "Erreur !", MB_OK | MB_ICONERROR);
                   EndDialog(hwnd, 0);
                   return FALSE;
                }

                fseek(executableFile, 0, SEEK_END);
                int size = ftell(executableFile);
                fseek(executableFile, (int)beginning - (int)ptv + strlen(IDString), SEEK_SET);
                free(ptv);

                int length = 0;
                fread(&length, sizeof(int), 1, executableFile);
                fread(TMDestinationE, 1, length, executableFile);
                convert_from_environment(TMDestinationE, TMDestination);

                if (test_exist(TMDestination) != 2)
                {
                   sprintf(buffer, "Le dossier d'extraction réclamé par le package n'existe pas sur cet ordinateur !\nAutorisez-vous le programme à créer les dossiers nécessaires à cet emplacement :\n\"%s\" ?", TMDestination);
                   if (MessageBox(NULL, buffer, "Attention !", MB_YESNO | MB_ICONWARNING) == IDNO)
                   {
                      EndDialog(hwnd, 0);
                      return FALSE;
                   }
                }

                char header[MAX_CHAINE];
                fread(header, 1, strlen(CRYPT_signature), executableFile);
                if (strcmp(header, CRYPT_signature) != 0 && !DialogBoxParam(mainInstance, "password", hwnd, (DLGPROC)PwdProc, (LPARAM)header))
                {
                    MessageBox(hwnd, "Mot de passe incorrect !", "Erreur", MB_OK | MB_ICONWARNING);
                    fclose(executableFile);
                    EndDialog(hwnd, 0);
                    return FALSE;
                }

                fread(&nombreFichiers, sizeof(int), 1, executableFile);

                size -= ftell(executableFile) + (nombreFichiers * sizeof(Info_file));
                size /= pow(2,10);

                sprintf(buffer, "Extraction de fichiers vers le répertoire :\n%s\nNombre de fichiers : %d\nPoids estimé : %d Ko\nAutorisez-vous le programme à copier sans les exécuter les fichiers stockés ?", TMDestination, nombreFichiers, size);
                if (MessageBox(NULL, buffer, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
                {
                    fclose(executableFile);
                    EndDialog(hwnd, 0);
                    return FALSE;
                }

                numeroFichier = 0;
                SetTimer(hwnd, 0, 50, NULL);
                return FALSE;
           }
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch (Select)
                {
                       case IDP_CANCEL:
                            EndDialog(hwnd, 0);
                            return FALSE;
                }

                return FALSE;
           case WM_CLOSE:
                SendMessage(hwnd, WM_COMMAND, IDP_CANCEL, 0);
                return FALSE;
           case WM_TIMER:
           {
                KillTimer(hwnd, 0);
                char buffer[MAX_CHAINE] = "\0";

                if (numeroFichier == nombreFichiers)
                {
                   fclose(executableFile);
                   EndDialog(hwnd, 1);
                   return FALSE;
                }

                numeroFichier++;
                SendMessage(GetDlgItem(hwnd, IDPB_PROGRESS), PBM_SETPOS, (numeroFichier - 1) * 100 / nombreFichiers, 0);
                sprintf(buffer, "Copie du fichier %d sur %d", numeroFichier, nombreFichiers);
                SetDlgItemText(hwnd, IDT_COUNT, buffer);

                Info_file info;
                fread(&info, sizeof(Info_file), 1, executableFile);

                void *file = malloc(info.size);
                fread(file, info.size, 1, executableFile);

                if (info.dir[1] != ':')
                   sprintf(buffer, "...\\%s\\%s", info.dir, info.name);
                else sprintf(buffer, "%s\\%s", info.dir, info.name);
                SetDlgItemText(hwnd, IDT_ADRESS, buffer);

                if (info.dir[1] == ':')
                {
                   sprintf(buffer, "Un fichier doit être extrait en dehors du dossier de base :\n%s\\%s\nAutorisez-vous la copie et la création éventuelle de répertoires ?", info.dir, info.name);
                   if (MessageBox(NULL, buffer, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
                   {
                       SetTimer(hwnd, 0, 50, NULL);
                       return FALSE;
                   }
                }
                else
                {
                    sprintf(buffer, "%s\\%s", TMDestination, info.dir);
                    strcpy(info.dir, buffer);
                }

                if (test_exist(info.dir) != 2)
                   create_dir(info.dir);

                sprintf(buffer, "%s\\%s", info.dir, info.name);

                if (test_exist(buffer) == 1)
                {
                    strcat(buffer, "\nCe fichier existe déjà. Remplacer ?");
                    if (MessageBox(NULL, buffer, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
                    {
                        SetTimer(hwnd, 0, 50, NULL);
                        return FALSE;
                    }
                    *strrchr(buffer, '\n') = '\0';
                }

                FILE *destination = NULL;
                if ((destination = fopen(buffer, "wb")) == NULL)
                {
                    strcat(buffer, "\nImpossible de créer le fichier.");
                    MessageBox(NULL, buffer, "Erreur !", MB_OK | MB_ICONERROR);
                    SetTimer(hwnd, 0, 50, NULL);
                    return FALSE;
                }

                if (password[0] != '\0')
                   CRYPT_decrypter(file, info.size, password, 0);
                fwrite(file, info.size, 1, destination);
                fclose(destination);

                HWND listWnd = GetDlgItem(endDialog, IDLV_LIST);

                LVITEM lvi;
                lvi.mask = LVIF_TEXT;
                lvi.state = 0;
                lvi.stateMask = 0;
                lvi.iItem = ListView_GetItemCount(listWnd);
                lvi.iSubItem = 0;
                lvi.pszText = buffer;
                ListView_InsertItem(listWnd, &lvi);

                SetTimer(hwnd, 0, 50, NULL);
                return FALSE;
           }

    }

    return FALSE;
}



int create_dir(char directory[])
{
     if (directory[1] != ':')
        return 0;

     char *position = NULL,
          *dir = malloc(strlen(directory) + 1);

     strcpy(dir, directory);
     int count;

     for (count = 0 ; (position = strrchr(dir, '\\')) != NULL ; count++)
           *position = '\0';

     count--;
     dir[strlen(dir)] = '\\';

     int i;
     for (i = 0 ; i <= count ; i++)
     {
         CreateDirectory(dir, NULL);
         dir[strlen(dir)] = '\\';
     }

     if (test_exist(dir) != 2)
        i = 0;
     else i = 1;

     free(dir);
     return i;
}


LRESULT CALLBACK PwdProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
                centrer_fenetre(hwnd, NULL);
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconKey);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconKey);
                SetDlgItemText(hwnd, IDE_MEMORY, (char*) lParam);
                return FALSE;
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDP_OK:
                      {
                           GetDlgItemText(hwnd, IDE_PWD, password, MAX_CHAINE);
                           GetDlgItemText(hwnd, IDE_MEMORY, buffer, MAX_CHAINE);

                           CRYPT_decrypter(buffer, strlen(CRYPT_signature), password, 0);
                           if (strcmp(buffer, CRYPT_signature) != 0)
                              EndDialog(hwnd, 0);
                           else EndDialog(hwnd, 1);

                           return FALSE;
                      }
                }

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK EndDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
           case WM_INITDIALOG:
                centrer_fenetre(hwnd, NULL);
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconGbx);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconGbx);

                RECT rect;
                GetClientRect(hwnd, &rect);

                HWND listWnd = CreateWindowEx(
                      WS_EX_CLIENTEDGE,
                      WC_LISTVIEW,
                      "",
                      WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER,
                      0,
                      0,
                      rect.right,
                      rect.bottom,
                      hwnd,
                      (HMENU) IDLV_LIST,
                      mainInstance,
                      NULL
                   );

                LVCOLUMN lvcol;
                lvcol.mask = LVCF_WIDTH;
                lvcol.cx = rect.right * 3;
                ListView_InsertColumn(listWnd, 0, &lvcol);
                ListView_SetExtendedListViewStyle(listWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                PostQuitMessage(0);
                return FALSE;
    }

    return FALSE;
}



void centrer_fenetre(HWND hwnd1, HWND hwnd2)
{
     RECT screen;
     screen.right = GetSystemMetrics(SM_CXFULLSCREEN);
     screen.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
     screen.top = 0;
     screen.left = 0;

     if (hwnd2 != NULL)
     {
          RECT rect;
          GetWindowRect(hwnd2, &rect);
          GetClientRect(hwnd2, &screen);
          screen.top = rect.top;
          screen.left = rect.left;
     }

     RECT wndRect;
     GetWindowRect(hwnd1, &wndRect);

     SetWindowPos(hwnd1, NULL, (screen.right - (wndRect.right - wndRect.left)) / 2 + screen.left, (screen.bottom - (wndRect.bottom - wndRect.top)) / 2 + screen.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

     return;
}


int convert_from_environment(char string[], char converted[])
{
    char buffer[MAX_CHAINE] = "\0",
         variable[MAX_CHAINE] = "\0",
         *position = NULL;

    if (buffer == NULL)
       return 0;

    strcpy(buffer, string);

    int i, j;
    for (i = 0 ; buffer[i] != '\0' ; i++)
    {
        if (buffer[i] == '%')
        {
             strcpy(variable, buffer + i + 1);
             if ((position = strchr(variable, '%')) != NULL)
                *position = '\0';

             int CSIDLValue = 0;

             for (j = 0 ; variable[j] != '\0' ; j++)
                 variable[j] = toupper(variable[j]);

             if (strcmp(variable, "COMMONDESKTOP") == 0)
                CSIDLValue = CSIDL_COMMON_DESKTOPDIRECTORY;
             else if (strcmp(variable, "COMMONDOCUMENTS") == 0)
                CSIDLValue = CSIDL_COMMON_DOCUMENTS;
             else if (strcmp(variable, "COMMONMUSIC") == 0)
                CSIDLValue = CSIDL_COMMON_MUSIC;
             else if (strcmp(variable, "COMMONPICTURES") == 0)
                CSIDLValue = CSIDL_COMMON_PICTURES;
             else if (strcmp(variable, "COMMONPROGRAMS") == 0)
                CSIDLValue = CSIDL_COMMON_PROGRAMS;
             else if (strcmp(variable, "COMMONSTARTMENU") == 0)
                CSIDLValue = CSIDL_COMMON_STARTMENU;
             else if (strcmp(variable, "COMMONSTARTUP") == 0)
                CSIDLValue = CSIDL_COMMON_STARTUP;
             else if (strcmp(variable, "COMMONVIDEO") == 0)
                CSIDLValue = CSIDL_COMMON_VIDEO;
             else if (strcmp(variable, "DESKTOP") == 0)
                CSIDLValue = CSIDL_DESKTOPDIRECTORY;
             else if (strcmp(variable, "MYDOCUMENTS") == 0)
                CSIDLValue = CSIDL_PERSONAL;
             else if (strcmp(variable, "MYMUSIC") == 0)
                CSIDLValue = CSIDL_MYMUSIC;
             else if (strcmp(variable, "MYPICTURES") == 0)
                CSIDLValue = CSIDL_MYPICTURES;
             else if (strcmp(variable, "MYVIDEO") == 0)
                CSIDLValue = CSIDL_MYVIDEO;
             else if (strcmp(variable, "PROGRAMS") == 0)
                CSIDLValue = CSIDL_PROGRAMS;
             else if (strcmp(variable, "STARTMENU") == 0)
                CSIDLValue = CSIDL_STARTMENU;
             else if (strcmp(variable, "STARTUP") == 0)
                CSIDLValue = CSIDL_STARTUP;
             else if (strcmp(variable, "SYSTEM") == 0)
                CSIDLValue = CSIDL_SYSTEM;

             if (CSIDLValue != 0)
                SHGetFolderPath(NULL, CSIDLValue, NULL, 0, variable);
             else
             {
                 char variable2[MAX_CHAINE] = "\0";
                 if (variable[0] != '\0')
                    GetEnvironmentVariable(variable, variable2, MAX_CHAINE);
                 else variable2[0] = '%';
                 strcpy(variable, variable2);
             }

             if ((position = strchr(buffer + i + 1, '%')) != NULL)
                strcpy(buffer + i, position + 1);
             else buffer[i] = '\0';

             char part1[MAX_CHAINE] = "\0";
             strcpy(part1, buffer);
             part1[i] = '\0';

             char part2[MAX_CHAINE];
             strcpy(part2, buffer + i);

             if (variable[0] != '\0')
                sprintf(buffer, "%s%s%s", part1, variable, part2);
        }
    }

    strcpy(converted, buffer);
    return 1;
}
