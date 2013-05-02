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
#define _WIN32_WINNT 0x0502
#define WINVER 0x0502

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <windows.h>
#include <shlobj.h>
#include <Objbase.h>
#include <commctrl.h>
#define _WINDOWS_
#include "fmod.h"  //Old version of FMOD
#include "FreeImage.h"
#include "fonctions_repertoire.h"
#include "fonctions_cryptage.h"
#include "fonctions_registre.h"
#include "htmlhelp.h"

#include "define.h"

#define MAX_CHAINE 5000


typedef struct Info_Challenge
{
        char name[100];
        char author[100];
        int tall;
        int weight;
        SYSTEMTIME authorTime;
        SYSTEMTIME goldTime;
        SYSTEMTIME silverTime;
        SYSTEMTIME bronzeTime;
        HBITMAP img;
        char dep[10000];
} Info_Challenge;

typedef struct Info_Image
{
        char name[MAX_CHAINE];
        char type[100];
        int weight;
        int w;
        int h;
        HBITMAP img;
} Info_Image;

typedef struct Info_Replay
{
        char track[MAX_CHAINE];
        char author[100];
        SYSTEMTIME time;
        int weight;
} Info_Replay;

typedef struct Info_Music
{
        char name[MAX_CHAINE];
        char type[100];
        SYSTEMTIME length;
        int weight;
} Info_Music;

typedef struct Info_Profile
{
        char name[100];
        int weight;
} Info_Profile;

typedef struct Info_Locator
{
        char name[MAX_CHAINE];
        char url[MAX_CHAINE];
        char targetType[100];
} Info_Locator;

typedef struct Info_Unknow
{
        char name[MAX_CHAINE];
        char type[100];
        int weight;
} Info_Unknow;


// fonctions
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);    // callback de la fenetre principale
LRESULT CALLBACK listProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PropertiesProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MpProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BeginPackageProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DragAndDropProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK baseModifProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChooseTMDProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChooseTMIDProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int retrieve_type(char track[]);
HBITMAP open_image(char adress[], int type, char temporaryFile[], RECT *size);
int retrieve_info_challenge(char adress[], Info_Challenge *info);
int retrieve_info_image(char adress[], Info_Image *info);
int retrieve_info_profile(char adress[], Info_Profile *info);
int retrieve_info_music(char adress[], Info_Music *info);
int retrieve_info_replay(char adress[], Info_Replay *info);
int retrieve_info_locator(char adress[], Info_Locator *info);
int retrieve_info_unknow(char adress[], Info_Unknow *info);
void conversion_time(SYSTEMTIME *st, int time);
int retrieve_info_text(char adress[], char text[], HBITMAP *bmp);
void convert_name(char name[], char converted[]);
void centrer_fenetre(HWND hwnd1, HWND hwnd2);
int retrieve_multi_select(OPENFILENAME ofn, char *tab[], int tailleTab, int autoAlloc);
int add_item(HWND hwnd, char adress[]);
int get_selected_items(HWND hwnd, int **ptOut);
int is_item_selected(HWND hwnd, int item);
int get_first_selected(HWND hwnd);
int select_items(HWND hwnd, int *tab, int array, int erase);
int create_icon(int destroy);
int convert_from_environment(char string[], char converted[]);
int create_toolTips();
int print_icon_menu();

unsigned char convert_utf8_to_ansi(unsigned char in[]);
int convert_string_to_ansi(char in[], char out[]);


//variales globales, classées par type
HWND mainWnd,     //fenêtre principale
     propertiesListWnd,
     mpWnd,
     listWnd,
     toolBarWnd,
     dragAndDropWnd,
     tableauToolTips[20] = {NULL},
     listViewWnd;

HBITMAP imageDeFond = NULL,
        imageAffichage = NULL;

HINSTANCE mainInstance;

HIMAGELIST imgList,
           sysImgList;

HICON iconExecute,
      iconVoid,
      iconDelete,
      iconOpen,
      iconPackage,
      iconPlay,
      iconPause,
      iconStop,
      iconProperties,
      iconAdd,
      iconSave,
      mainIcon;

DWORD dwCookie = 0;

RECT tailleListWnd,
     tailleMainWnd,
     tailleRectangle;

int minListWnd,
    numeroFichier = 0,
    nombreFichiers = 0,
    typeAffichage = AFF_NONE,
    useFmod = 0,
    propertiesIndex = 0,
    channel = -1,
    includeOutOfDir = 1,
    includeUnknow = 1,
    usePassword = 0,
    nombreFichiersCopies = 0,
    positionNombreFichiers = 0,
    modified = 0,
    defilActive = 1,
    caption, edge,
    dragFiles = 0,
    isBuilding = 0;

int IDiconExecute,
    IDiconVoid,
    IDiconDelete,
    IDiconOpen,
    IDiconPackage,
    IDiconProperties,
    IDiconSave,
    IDiconAdd;

FILE *fichierListeImages = NULL,
     *exeFile = NULL;

char baseFileName[MAX_CHAINE] = "base.exe",
     *dossierBase = NULL,
     appName[5000],
     argts[5000] = "\0",
     informationText[MAX_CHAINE] = "\0",
     TMDestination[MAX_CHAINE] = "\0",
     TMDestinationE[MAX_CHAINE] = "%mydocuments%\\TrackMania",
     TMInstallDir[MAX_CHAINE] = "\0",
     TMDirectory[MAX_CHAINE] = "\0",
     userprofile[MAX_CHAINE] = "\0",
     adresseMusique[MAX_CHAINE] = "\0",
     packagePassword[MAX_CHAINE] = "\0",
     *tableauFichiersAOuvrir[MAX_CHAINE] = {NULL},
     fichierInit[MAX_CHAINE] = "config.ini",
     fichierInit2[MAX_CHAINE] = "dirs.ini",
     adresseFichierListe[MAX_CHAINE] = "\0",
     helpFileAdress[MAX_CHAINE] = "Aide.chm";

FSOUND_STREAM *stream = NULL;


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow
                     )
{
    InitCommonControls();
    HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD)&dwCookie) ;

    mainInstance = hInstance;
    char buffer[MAX_CHAINE] = "\0";

    /*Info_Replay info;
    sprintf(buffer, "%ld", retrieve_info_replay("a.gbx", &info));
    MessageBox(NULL, buffer, "inf", MB_OK);*/

    /*
    Traitement de la ligne de commande
    Syntaxe obligatoire :
       => adresse\\nomApplication /argts
    */

    dossierBase = GetCommandLine(),

    strcpy(appName, dossierBase);
    *(strstr(appName, ".exe") + 4) = '\0';
    if (strrchr(appName, '\\') != NULL)
          strcpy(appName, strrchr(appName, '\\') + 1);

    if (strchr(dossierBase, '/') != NULL)
          strcpy(argts, strchr(dossierBase, '/') + 1);
    if (strchr(argts, '"') != NULL)
       *strchr(argts, '"') = '\0';

    if (strchr(dossierBase, '"') != NULL)
    {
           strcpy(dossierBase, strchr(dossierBase, '"') + 1);
           *strchr(dossierBase, '"') = '\0';
    }

    while (strstr(dossierBase, ".exe") != NULL)
          *strrchr(dossierBase, '\\') = '\0';

    chdir(dossierBase);  //rétablissement du dossier de lancement de l'application


    strcpy(buffer, baseFileName);
    sprintf(baseFileName, "%s\\%s", dossierBase, buffer);
    strcpy(buffer, fichierInit);
    sprintf(fichierInit, "%s\\%s", dossierBase, buffer);
    strcpy(buffer, fichierInit2);
    sprintf(fichierInit2, "%s\\%s", dossierBase, buffer);
    strcpy(buffer, helpFileAdress);
    sprintf(helpFileAdress, "%s\\%s", dossierBase, buffer);

    mainIcon = LoadImage(hInstance, "mainIco", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconExecute = LoadImage(hInstance, "icoExe", IMAGE_ICON, 20, 20, LR_DEFAULTCOLOR);
    iconVoid = LoadImage(hInstance, "icoVoid", IMAGE_ICON, 20, 20, LR_DEFAULTCOLOR);
    iconDelete = LoadImage(hInstance, "icoDelete", IMAGE_ICON, 20, 20, LR_DEFAULTCOLOR);
    iconOpen = LoadImage(hInstance, "icoOpen", IMAGE_ICON, 20, 20, LR_DEFAULTCOLOR);
    iconPackage = LoadImage(hInstance, "icoPackage", IMAGE_ICON, 20, 20, LR_DEFAULTCOLOR);
    iconProperties = LoadImage(hInstance, "icoProp", IMAGE_ICON, 20, 20, LR_DEFAULTCOLOR);
    iconSave = LoadImage(hInstance, "icoSave", IMAGE_ICON, 20, 20, LR_DEFAULTCOLOR);
    iconAdd = LoadImage(hInstance, "icoAdd", IMAGE_ICON, 20, 20, LR_DEFAULTCOLOR);
    iconPlay = LoadImage(hInstance, "icoPlay", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconPause = LoadImage(hInstance, "icoPause", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconStop = LoadImage(hInstance, "icoStop", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);


    FILE *file = NULL;

    if ((file = fopen(fichierInit2, "r")) != NULL)
    {
       fgets(TMDirectory, MAX_CHAINE, file);
       if (strchr(TMDirectory, '\n') != NULL)
          *strchr(TMDirectory, '\n') = '\0';

       fgets(TMInstallDir, MAX_CHAINE, file);
       if (strchr(TMInstallDir, '\n') != NULL)
          *strchr(TMInstallDir, '\n') = '\0';

       fclose(file);
    }

    if (test_exist(TMDirectory) != 2)
       convert_from_environment(TMDestinationE, TMDirectory);

    if (test_exist(TMInstallDir) != 2)
       convert_from_environment("%programfiles%\\TMNationsForever", TMInstallDir);


    if (test_exist(TMDirectory) != 2)
       DialogBox(hInstance, "chooseTMD", NULL, (DLGPROC)ChooseTMDProc);

    if (test_exist(TMInstallDir) != 2)
       DialogBox(hInstance, "chooseTMID", NULL, (DLGPROC)ChooseTMIDProc);

    if ((file = fopen(fichierInit2, "w")) != NULL)
    {
       fprintf(file, "%s\n%s", TMDirectory, TMInstallDir);
       fclose(file);
    }

    convert_from_environment(TMDestinationE, TMDestination);

    if (!FSOUND_Init(44100, 32, FSOUND_INIT_ACCURATEVULEVELS)) // Initialisation de FMOD. Si erreur alors...
    {
        if (MessageBox(NULL, "Impossible de démarrer FMOD.\nCertaines fonctions (audio) ne seront pas disponibles.\nContinuer malgré tout ?", "Attention", MB_ICONWARNING | MB_YESNO) == IDNO)
           exit(EXIT_FAILURE);
    }
    else useFmod = 1;

    CRYPT_init();

    sprintf(buffer, "%s\\mainIcon.ico", dossierBase);
    char command[MAX_CHAINE];
    sprintf(command, "\"%s\\%s\" /%%1", dossierBase, appName);
    assoc_extension(".tml", "tmcList\nListe TMC", buffer, command);

    WNDCLASSEX wincl;    // classe de la fenêtre principale
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wincl.lpfnWndProc = WndProc;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hInstance = hInstance;
    wincl.hIcon = LoadIcon(NULL,IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wincl.lpszMenuName = NULL;
    wincl.lpszClassName = "std";
    wincl.hIconSm = LoadIcon(NULL,IDI_APPLICATION);

    if (!RegisterClassEx (&wincl))
        return 0;

    mainWnd = CreateWindowEx(
          WS_EX_CLIENTEDGE,
          "std",
          "TM Creator",
          WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          NULL,
          NULL,
          hInstance,
          NULL
       );

    SendMessage(mainWnd, WM_SETICON, ICON_BIG, (LPARAM)mainIcon);
    SendMessage(mainWnd, WM_SETICON, ICON_SMALL, (LPARAM)mainIcon);

    HWND hwnd = CreateDialog(hInstance, "list", mainWnd, (DLGPROC) listProc);
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    dragAndDropWnd = CreateDialog(hInstance, "dragAndDrop", hwnd, (DLGPROC) DragAndDropProc);
    RECT rect;
    GetClientRect(dragAndDropWnd, &rect);
    SetWindowPos(dragAndDropWnd, NULL, 0, 0, rect.right, rect.bottom, SWP_NOZORDER | SWP_NOMOVE);

    listViewWnd = CreateWindowEx(
          0,
          WC_LISTVIEW,
          "",
          WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_NOCOLUMNHEADER,
          2,
          2,
          rect.right - 4,
          rect.bottom - 4,
          dragAndDropWnd,
          (HMENU) IDLV_LIST,
          hInstance,
          NULL
       );

    LVCOLUMN lvcol;
    lvcol.mask = LVCF_WIDTH;
    lvcol.cx = rect.right - 4 - GetSystemMetrics(SM_CXVSCROLL);
    ListView_InsertColumn(listViewWnd, 0, &lvcol);

    GetWindowRect(dragAndDropWnd, &rect);
    GetWindowRect(mainWnd, &tailleMainWnd);
    GetWindowRect(hwnd, &tailleListWnd);
    minListWnd = tailleListWnd.right - tailleListWnd.left;

    caption = GetSystemMetrics(SM_CYCAPTION);
    edge = GetSystemMetrics(SM_CXEDGE);


    toolBarWnd = CreateWindowEx(
               0,
               TOOLBARCLASSNAME,
               NULL,
               WS_CHILD | WS_VISIBLE | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | TBSTYLE_TOOLTIPS,
               rect.left - tailleListWnd.left - 3,
               rect.bottom - tailleListWnd.top + 20,
               rect.right - rect.left,
               50,
               hwnd,
               NULL,
               hInstance,
               NULL
       );


    imgList = ImageList_Create(20, 20, ILC_COLOR32 | ILC_MASK, 8, 0);
    IDiconExecute = ImageList_AddIcon(imgList, iconExecute);
    IDiconVoid = ImageList_AddIcon(imgList, iconVoid);
    IDiconDelete = ImageList_AddIcon(imgList, iconDelete);
    IDiconOpen = ImageList_AddIcon(imgList, iconOpen);
    IDiconPackage = ImageList_AddIcon(imgList, iconPackage);
    IDiconProperties = ImageList_AddIcon(imgList, iconProperties);
    IDiconSave = ImageList_AddIcon(imgList, iconSave);
    IDiconAdd = ImageList_AddIcon(imgList, iconAdd);

    SendMessage(toolBarWnd, TB_SETIMAGELIST, 0, (LPARAM)imgList);
    SendMessage(toolBarWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SendMessage(toolBarWnd, TB_SETMAXTEXTROWS, 0, 0);

    TBBUTTON tb;
    tb.iBitmap = IDiconAdd;
    tb.idCommand = IDP_ADD;
    tb.dwData = 0;
    tb.iString = (int)buffer;
    tb.fsState = TBSTATE_ENABLED;
    tb.fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
    strcpy(buffer, "Ajouter un fichier");
    SendMessage(toolBarWnd, TB_ADDBUTTONS, 1, (LPARAM)&tb);

    tb.iBitmap = IDiconDelete;
    tb.idCommand = IDP_DELETE;
    strcpy(buffer, "Supprimer de la liste");
    SendMessage(toolBarWnd, TB_ADDBUTTONS, 1, (LPARAM)&tb);

    tb.iBitmap = IDiconVoid;
    tb.idCommand = IDP_DELETEALL;
    strcpy(buffer, "Vider la liste");
    SendMessage(toolBarWnd, TB_ADDBUTTONS, 1, (LPARAM)&tb);

    tb.iBitmap = IDiconProperties;
    tb.idCommand = IDP_PROPERTIES;
    strcpy(buffer, "Propriétés");
    SendMessage(toolBarWnd, TB_ADDBUTTONS, 1, (LPARAM)&tb);

    tb.iBitmap = IDiconPackage;
    tb.idCommand = IDP_BUILD;
    strcpy(buffer, "Construire le package");
    SendMessage(toolBarWnd, TB_ADDBUTTONS, 1, (LPARAM)&tb);

    tb.iBitmap = IDiconExecute;
    tb.idCommand = IDP_EXE;
    strcpy(buffer, "Ouvrir le fichier");
    SendMessage(toolBarWnd, TB_ADDBUTTONS, 1, (LPARAM)&tb);

    tb.iBitmap = IDiconOpen;
    tb.idCommand = IDP_OPEN;
    strcpy(buffer, "Ouvrir un projet");
    SendMessage(toolBarWnd, TB_ADDBUTTONS, 1, (LPARAM)&tb);

    tb.iBitmap = IDiconSave;
    tb.idCommand = IDP_SAVE;
    strcpy(buffer, "Sauvegarder le projet");
    SendMessage(toolBarWnd, TB_ADDBUTTONS, 1, (LPARAM)&tb);

    /*SendMessage(GetDlgItem(hwnd, IDP_BUILD), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconPackage);
    SendMessage(GetDlgItem(hwnd, IDP_EXE), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconExecute);
    SendMessage(GetDlgItem(hwnd, IDP_ADD), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconAdd);
    SendMessage(GetDlgItem(hwnd, IDP_DELETE), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconDelete);
    SendMessage(GetDlgItem(hwnd, IDP_DELETEALL), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconVoid);
    SendMessage(GetDlgItem(hwnd, IDP_PROPERTIES), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconProperties);
    SendMessage(GetDlgItem(hwnd, IDP_SAVE), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconSave);
    SendMessage(GetDlgItem(hwnd, IDP_OPEN), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconOpen);*/


    SetWindowPos(mainWnd, NULL, 0, 0, 730, tailleListWnd.bottom - tailleListWnd.top, SWP_NOMOVE | SWP_NOZORDER);
    centrer_fenetre(mainWnd, NULL);
    GetClientRect(mainWnd, &tailleMainWnd);

    CreateWindowEx(
          0,
          "button",
          "a",
          WS_CHILD | WS_VISIBLE | BS_ICON,
          tailleMainWnd.right - 60,
          tailleMainWnd.bottom - 30,
          50,
          20,
          mainWnd,
          (HMENU) IDP_DEFIL,
          hInstance,
          NULL
       );
    SendMessage(GetDlgItem(mainWnd, IDP_DEFIL), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconPause);


    mpWnd = CreateDialog(hInstance, "musicplayer", mainWnd, (DLGPROC)MpProc);

    CreateWindowEx(
           0,
           TRACKBAR_CLASS,
           "\0",
           WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_NOTICKS,
           12,
           50,
           245,
           30,
           mpWnd,
           (HMENU) IDTB_MUSIC,
           mainInstance,
           NULL
        );

    SendMessage(GetDlgItem(mpWnd, IDP_PLAYMUSIC), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconPlay);
    SendMessage(GetDlgItem(mpWnd, IDP_STOPMUSIC), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconStop);
    SendMessage(GetDlgItem(mpWnd, IDP_PAUSEMUSIC), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconPause);

    create_toolTips();
    lister_tout("images", ".bmp", 0, 0, "list.txt", 1, NULL);

    if ((fichierListeImages = fopen("list.txt", "r")) != NULL)
        fgets(buffer, MAX_CHAINE, fichierListeImages);
    if (strchr(buffer, '\n') != NULL)
       *strchr(buffer, '\n') = '\0';
    imageDeFond = (HBITMAP) LoadImage(mainInstance, buffer, IMAGE_BITMAP, tailleMainWnd.right - minListWnd, tailleMainWnd.bottom, LR_LOADFROMFILE);
    SetTimer(mainWnd, 0, 10000, NULL);

    ShowWindow(mainWnd, SW_SHOW);
    listWnd = hwnd;


    char adress[MAX_CHAINE] = "\\";
    if ((file = fopen(fichierInit, "r")) != NULL)
    {
       fgets(adress, MAX_CHAINE, file);
       fclose(file);
    }

    if (argts[0] == '\0' && MessageBox(hwnd, "Voulez-vous ouvrir un projet existant (oui)\nou en créer un nouveau (non) ?", "Démarrage", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        if (strchr(adress, '\\') != NULL)
           *strrchr(adress, '\\') = '\0';
        else strcpy(adress, dossierBase);

        OPENFILENAME ofn;
        memset(&ofn, 0, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwnd;
        ofn.nMaxFile = MAX_CHAINE;
        ofn.lpstrFilter = "TMC listes  (*.tml)\0*.tml\0Tous les fichiers  (*.*)\0*.*\0";
        ofn.lpstrFile = argts;
        ofn.lpstrTitle = "Ouvrir un projet";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
        ofn.lpstrInitialDir = adress;

        if (!GetOpenFileName(&ofn))
           argts[0] = '\0';
    }

    if (argts[0] != '\0')
       SendMessage(hwnd, WM_COMMAND, IDP_OPEN, 0);


    MSG messages;
    while (GetMessage (&messages, NULL, 0, 0))
    {
        if (IsDialogMessage(hwnd, &messages) == 0)
        {
            TranslateMessage(&messages);
            DispatchMessage(&messages);
        }
    }

    HtmlHelp(NULL, NULL, HH_UNINITIALIZE, dwCookie);
    return messages.wParam;
}





LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE] = "\0";
    HDC hdc;
    PAINTSTRUCT paintst;
    int Select;

    switch (msg)
    {
        case WM_DESTROY:
             PostQuitMessage (0);
             break;
        case WM_CLOSE:
             if (modified)
             {
                 switch (MessageBox(mainWnd, "Sauvegarder le projet ?", "Confirmation", MB_YESNOCANCEL | MB_ICONQUESTION))
                 {
                     case IDYES:
                          SendMessage(listWnd, WM_COMMAND, IDP_SAVE, 0);
                          break;
                     case IDCANCEL:
                          return FALSE;
                 }
             }

             DestroyWindow(hwnd);
             return FALSE;
        case WM_SYSCOMMAND:
             if (wParam == SC_MINIMIZE)
             {
                ShowWindow(hwnd, SW_MINIMIZE);
                ShowWindow(hwnd, SW_HIDE);
                create_icon(0);
             }
             else DefWindowProc(hwnd, msg, wParam, lParam);
             return FALSE;
        case WM_NOTIFY:
        {
             NMHDR *nmhdr = (NMHDR*)lParam;

             switch (nmhdr->code)
             {
                    case TTN_SHOW:
                         return FALSE;
                    case TTN_POP:
                         create_toolTips();
                         return FALSE;
             }

             return FALSE;
        }
        case WM_PAINT:
        {
             if (imageDeFond == NULL)
                return FALSE;

             hdc = BeginPaint(hwnd, &paintst);

             HDC hdcMemory;

             hdcMemory = CreateCompatibleDC(hdc);
             SelectObject(hdcMemory, imageDeFond);
             BitBlt(hdc, minListWnd, 0, tailleMainWnd.right - minListWnd, tailleMainWnd.bottom, hdcMemory, 0, 0, SRCCOPY);


             HBRUSH brush = GetStockObject(BLACK_BRUSH);
             BLENDFUNCTION bf;
             bf.BlendOp = AC_SRC_OVER;
             bf.BlendFlags = 0;
             bf.SourceConstantAlpha = 150;
             bf.AlphaFormat = 0;

             RECT rect;
             rect.left = 0;
             rect.right = tailleMainWnd.right - (tailleListWnd.right - tailleListWnd.left) - 100;
             rect.top = 0;
             rect.bottom = tailleMainWnd.bottom - 100;
             HBITMAP hbmp = CreateCompatibleBitmap(hdcMemory,rect.right,rect.bottom);
             SelectObject(hdcMemory, hbmp);
             SelectObject(hdcMemory, brush);

             HPEN hpen = CreatePen(PS_DASHDOTDOT, 1, RGB(255, 255, 0));
             SelectObject(hdcMemory, hpen);
             Rectangle(hdcMemory, rect.left, rect.top, rect.right, rect.bottom);

             AlphaBlend(hdc,
                        tailleListWnd.right - tailleListWnd.left + 50,
                        50,
                        rect.right,
                        rect.bottom,
                        hdcMemory,
                        0,
                        0,
                        rect.right,
                        rect.bottom,
                        bf);

             rect.top = 50;
             rect.bottom += 50;
             rect.left = tailleListWnd.right - tailleListWnd.left + 50;
             rect.right += tailleListWnd.right - tailleListWnd.left + 50;
             tailleRectangle = rect;

             BITMAP bm;
             RECT clipRect;
             clipRect.left = rect.left + 10;
             clipRect.top = rect.top + 10;
             clipRect.right = rect.right - 10;
             clipRect.bottom = rect.bottom - 10;

             SetTextColor(hdc, RGB(255, 255, 255));
             SetBkMode(hdc, TRANSPARENT);

             switch (typeAffichage)
             {
                    case AFF_CHALLENGE:
                    {
                         SelectObject(hdcMemory, imageAffichage);
                         GetObject(imageAffichage, sizeof(bm), &bm);

                         POINT points[4];
                         points[2].x = (rect.right - rect.left - bm.bmWidth - 20) / 2 + rect.left + 10;
                         points[2].y = rect.top + 10;
                         points[1].x = points[2].x + bm.bmWidth;
                         points[1].y = points[2].y + bm.bmHeight;
                         points[0].x = points[2].x;
                         points[0].y = points[1].y;
                         points[3].x = points[1].x;
                         points[3].y = points[2].y;

                         PlgBlt(hdc, points, hdcMemory, 0, 0, bm.bmWidth, bm.bmHeight, NULL, 0, 0);
                         clipRect.top += bm.bmHeight + 10;
                         DrawText(hdc, informationText, -1, &clipRect, DT_LEFT | DT_NOPREFIX);
                         break;
                    }
                    case AFF_IMAGE:
                         bf.SourceConstantAlpha = 230;
                         SelectObject(hdcMemory, imageAffichage);
                         GetObject(imageAffichage, sizeof(bm), &bm);
                         clipRect.top += bm.bmHeight + 10;
                         AlphaBlend(hdc, (rect.right - rect.left - bm.bmWidth - 20) / 2 + rect.left + 10, rect.top + 10, bm.bmWidth, bm.bmHeight, hdcMemory, 0, 0, bm.bmWidth, bm.bmHeight, bf);

                         DrawText(hdc, informationText, -1, &clipRect, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
                         break;
                    case AFF_LOCATOR:
                    case AFF_UNKNOW:
                    case AFF_PROFILE:
                    case AFF_REPLAY:
                         DrawText(hdc, informationText, -1, &clipRect, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
                         break;
                    case AFF_MUSIC:
                         GetClientRect(mpWnd, &rect);
                         clipRect.bottom -= rect.bottom + 20;

                         DrawText(hdc, informationText, -1, &clipRect, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
                         if (useFmod)
                            SetWindowPos(mpWnd, NULL, (clipRect.right - clipRect.left - rect.right - 20) / 2 + 10 + clipRect.left, clipRect.bottom + 10, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
                         break;
             }

             DeleteDC(hdcMemory);
             DeleteObject(hpen);
             DeleteObject(hbmp);

             EndPaint(hwnd, &paintst);
             return FALSE;
        }
        case WM_COMMAND:
             Select = LOWORD(wParam);
             switch (Select)
             {
                    case IDP_DEFIL:
                         if (defilActive)
                         {
                             KillTimer(hwnd, 0);
                             SendMessage(GetDlgItem(mainWnd, IDP_DEFIL), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconPlay);
                             defilActive = 0;
                         }
                         else
                         {
                             SetTimer(hwnd, 0, 10000, NULL);
                             SendMessage(GetDlgItem(mainWnd, IDP_DEFIL), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconPause);
                             defilActive = 1;
                         }
                         return FALSE;
                    case IM_OPEN:
                    case ICON_ID:
                         if (lParam == WM_LBUTTONDBLCLK || wParam == IM_OPEN)
                         {
                             create_icon(1);
                             ShowWindow(hwnd, SW_SHOW);
                             ShowWindow(hwnd, SW_RESTORE);
                         }
                         else if (lParam == WM_RBUTTONDOWN)
                              print_icon_menu();

                         return FALSE;
             }

             return FALSE;
        case WM_TIMER:
        {
             if (fichierListeImages != NULL && fgets(buffer, MAX_CHAINE, fichierListeImages) == NULL)
             {
                rewind(fichierListeImages);
                fgets(buffer, MAX_CHAINE, fichierListeImages);
             }

             if (strchr(buffer, '\n') != NULL)
                *strchr(buffer, '\n') = '\0';
             if (imageDeFond != NULL)
                DeleteObject(imageDeFond);
             imageDeFond = (HBITMAP) LoadImage(mainInstance, buffer, IMAGE_BITMAP, tailleMainWnd.right - minListWnd, tailleMainWnd.bottom, LR_LOADFROMFILE);

             RECT rect = tailleMainWnd;
             rect.left = minListWnd;

             InvalidateRect(mainWnd, &rect, FALSE);
             UpdateWindow(mainWnd);
             return FALSE;
        }
        default:
            return DefWindowProc (hwnd, msg, wParam, lParam);
    }

    return 0;
}


LRESULT CALLBACK listProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE] = "\0";
    int Select;

    switch (msg)
    {
           case WM_NOTIFY:
           {
                NMHDR *nmhdr = (NMHDR*)lParam;

                switch (nmhdr->code)
                {
                       case TTN_SHOW:
                            return FALSE;
                       case TTN_POP:
                            create_toolTips();
                            return FALSE;
                 }

                 return FALSE;
           }
           case WM_TIMER:
           {
                KillTimer(hwnd, 0);

                if (numeroFichier >= nombreFichiers)
                {
                    fseek(exeFile, positionNombreFichiers, SEEK_SET);
                    fwrite(&nombreFichiersCopies, sizeof(int), 1, exeFile);
                    fclose(exeFile);

                    SendMessage(toolBarWnd, TB_ENABLEBUTTON, IDP_ADD, TRUE);
                    SendMessage(toolBarWnd, TB_ENABLEBUTTON, IDP_DELETE, TRUE);
                    SendMessage(toolBarWnd, TB_ENABLEBUTTON, IDP_DELETEALL, TRUE);
                    SendMessage(toolBarWnd, TB_ENABLEBUTTON, IDP_BUILD, TRUE);
                    SendMessage(toolBarWnd, TB_ENABLEBUTTON, IDP_OPEN, TRUE);

                    isBuilding = 0;
                    MessageBox(hwnd, "Construction terminée !", "Information", MB_OK | MB_ICONINFORMATION);
                    return FALSE;
                }

                ComboBox_GetLBText(GetDlgItem(hwnd, IDCB_HIDDENLIST), numeroFichier, buffer);
                numeroFichier++;

                *strchr(buffer, '\n') = '\0';
                if ((*(strchr(buffer, '\0') + 2) != ':' && !includeOutOfDir) || (!includeUnknow && retrieve_type(buffer) == TRACK_UNKNOW))
                {
                   SetTimer(hwnd, 0, 100, NULL);
                   return FALSE;
                }

                FILE *file = fopen(buffer, "rb");
                if (file == NULL)
                {
                    strcat(buffer, "\nImpossible d'ouvrir le fichier.");
                    MessageBox(hwnd, buffer, "Erreur !", MB_OK | MB_ICONERROR);
                    SetTimer(hwnd, 0, 100, NULL);
                    return FALSE;
                }

                fseek(file, 0, SEEK_END);
                int size = ftell(file);
                rewind(file);

                void *ptv = malloc(size);
                if (fread(ptv, 1, size, file) <= 0)
                {
                    strcat(buffer, "\nImpossible de lire le fichier.");
                    MessageBox(hwnd, buffer, "Erreur !", MB_OK | MB_ICONERROR);
                    SetTimer(hwnd, 0, 100, NULL);
                    free(ptv);
                    fclose(file);
                    return FALSE;
                }

                fclose(file);

                char name[MAX_CHAINE] = "\0";
                strcpy(name, strrchr(buffer, '\\') + 1);
                name[99] = '\0';
                fwrite(name, 1, 100, exeFile);

                strcpy(name, strchr(buffer, '\0') + 1);
                fwrite(name, 1, MAX_CHAINE, exeFile);

                fwrite(&size, sizeof(int), 1, exeFile);
                nombreFichiersCopies++;

                if (usePassword)
                   CRYPT_crypter(ptv, size, packagePassword, 0);
                fwrite(ptv, 1, size, exeFile);

                SetTimer(hwnd, 0, 100, NULL);
                return FALSE;
           }
           case WM_COMMAND:
                Select = LOWORD(wParam);
                switch (Select)
                {
                       case IDP_ADD:
                       {
                            if (isBuilding)
                               return FALSE;

                            int count = 0;

                            if (!dragFiles)
                            {
                                OPENFILENAME ofn;

                                memset(&ofn, 0, sizeof(OPENFILENAME));
                                ofn.lStructSize = sizeof(OPENFILENAME);
                                ofn.hwndOwner = hwnd;
                                ofn.nMaxFile = MAX_CHAINE;
                                ofn.lpstrFilter = "Fichiers GBX  (*.gbx)\0*.gbx\0Tous les fichiers  (*.*)\0*.*\0\0";
                                ofn.lpstrFile = buffer;
                                ofn.lpstrTitle = "Ajouter un fichier";
                                ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
                                ofn.lpstrInitialDir = NULL;

                                if (!GetOpenFileName(&ofn))
                                   return FALSE;

                                count = retrieve_multi_select(ofn, tableauFichiersAOuvrir, MAX_CHAINE, 2);
                            }
                            else
                            {
                                count = dragFiles;
                                dragFiles = 0;
                            }

                            int i = 0;
                            for (i = 0 ; i < count ; i++)
                            {
                                strcpy(buffer, tableauFichiersAOuvrir[i]);
                                add_item(listViewWnd, buffer);

                                char destination[MAX_CHAINE] = "\0";
                                if (strstr(buffer, TMDestination) != NULL)
                                {
                                   strcpy(destination, buffer + strlen(TMDestination) + 1);
                                   if (strchr(destination, '\\') != NULL)
                                      *strrchr(destination, '\\') = '\0';
                                }
                                else
                                {
                                    switch (retrieve_type(buffer))
                                    {
                                           case TRACK_CHALLENGE:
                                                strcpy(destination, "Tracks\\Challenges\\Downloaded");
                                                break;
                                           case TRACK_MUSIC:
                                                strcpy(destination, "ChallengeMusics");
                                                break;
                                           case TRACK_IMAGE:
                                                strcpy(destination, "MediaTracker\\Images");
                                                break;
                                           case TRACK_SOUND:
                                                strcpy(destination, "MediaTracker\\Sounds");
                                                break;
                                           case TRACK_REPLAY:
                                                strcpy(destination, "Tracks\\Replays\\Downloaded");
                                                break;
                                           case TRACK_CAMPAIGN:
                                                strcpy(destination, "Tracks\\Campaigns\\Downloaded");
                                                break;
                                           case TRACK_PROFILE:
                                                strcpy(destination, "Profiles");
                                                break;
                                    }
                                }

                                sprintf(buffer, "%s\n%s", buffer, destination);
                                ComboBox_AddString(GetDlgItem(hwnd, IDCB_HIDDENLIST), buffer);
                                *strchr(buffer, '\n') = '\0';

                                Info_Challenge info;
                                if (retrieve_info_challenge(buffer, &info) && info.dep[0] != '\0')
                                {
                                     char text[MAX_CHAINE];
                                     sprintf(text, "Voulez-vous ajouter également à la liste les dépendances du challenge\n\"%s\" ?", buffer);
                                     if (MessageBox(mainWnd, text, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDYES)
                                     {
                                         dragFiles = 0;
                                         int j;

                                         char **tab = malloc(count * sizeof(char*));
                                         for (j = 0 ; j < count ; j++)
                                         {
                                             tab[j] = malloc(MAX_CHAINE);
                                             strcpy(tab[j], tableauFichiersAOuvrir[j]);
                                         }

                                         for (j = 0 ; info.dep[0] != '\0' ; j++)
                                         {
                                             if (tableauFichiersAOuvrir[j] != NULL)
                                                free(tableauFichiersAOuvrir[j]);
                                             tableauFichiersAOuvrir[j] = malloc(MAX_CHAINE);

                                             if (strchr(info.dep, '\n') != NULL)
                                                *strchr(info.dep, '\n') = '\0';

                                             strcpy(tableauFichiersAOuvrir[j], info.dep);
                                             strcpy(info.dep, strchr(info.dep, '\0') + 1);

                                             dragFiles++;
                                         }

                                         SendMessage(hwnd, WM_COMMAND, IDP_ADD, 0);

                                         for (j = 0 ; j < count ; j++)
                                         {
                                             strcpy(tableauFichiersAOuvrir[j], tab[j]);
                                             free(tab[j]);
                                         }
                                         free(tab);
                                     }
                                }
                            }

                            modified = 1;
                            return FALSE;
                       }
                       case IDP_BUILD:
                       {
                            if (isBuilding)
                               return FALSE;

                            if (ListView_GetItemCount(listViewWnd) <= 0)
                               return FALSE;

                            if (!DialogBox(mainInstance, "beginpackage", mainWnd, (DLGPROC)BeginPackageProc))
                               return FALSE;

                            OPENFILENAME ofn;

                            memset(&ofn, 0, sizeof(OPENFILENAME));
                            ofn.lStructSize = sizeof(OPENFILENAME);
                            ofn.hwndOwner = hwnd;
                            ofn.nMaxFile = MAX_CHAINE;
                            ofn.lpstrFilter = "Exécutables  (*.exe)\0*.exe\0Tous les fichiers  (*.*)\0*.*\0\0";
                            ofn.lpstrFile = buffer;
                            ofn.lpstrTitle = "Enregistrer sous";
                            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER;
                            ofn.lpstrInitialDir = NULL;
                            ofn.lpstrDefExt = ".exe";

                            if (!GetSaveFileName(&ofn))
                               return FALSE;

                            if ((exeFile = fopen(buffer, "wb")) == NULL)
                            {
                                strcat(buffer, "\nImpossible d'ouvrir le fichier.");
                                MessageBox(hwnd, buffer, "Erreur !", MB_OK | MB_ICONERROR);
                                return FALSE;
                            }

                            FILE *file = fopen(baseFileName, "rb");
                            if (file == NULL)
                            {
                                MessageBox(hwnd, "Impossible d'ouvrir le fichier de base !", "Erreur !", MB_OK | MB_ICONERROR);
                                fclose(exeFile);
                                return FALSE;
                            }

                            fseek(file, 0, SEEK_END);
                            int size = ftell(file);
                            rewind(file);
                            void *ptv = malloc(size);

                            fread(ptv, 1, size, file);
                            fwrite(ptv, 1, size, exeFile);

                            char name[MAX_CHAINE] = "Beginning-files";
                            fwrite(name, 1, strlen(name), exeFile);

                            int length = strlen(TMDestinationE);
                            fwrite(&length, sizeof(int), 1, exeFile);
                            fwrite(TMDestinationE, 1, length, exeFile);

                            char header[MAX_CHAINE] = "\0";
                            if (usePassword)
                               CRYPT_encoder_header(packagePassword, header, MAX_CHAINE);
                            else strcpy(header, CRYPT_signature);
                            fwrite(header, 1, strlen(header), exeFile);

                            nombreFichiers = ComboBox_GetCount(GetDlgItem(hwnd, IDCB_HIDDENLIST));
                            nombreFichiersCopies = 0;
                            positionNombreFichiers = ftell(exeFile);
                            fwrite(&nombreFichiers, sizeof(int), 1, exeFile);

                            numeroFichier = 0;
                            SendMessage(toolBarWnd, TB_ENABLEBUTTON, IDP_ADD, FALSE);
                            SendMessage(toolBarWnd, TB_ENABLEBUTTON, IDP_DELETE, FALSE);
                            SendMessage(toolBarWnd, TB_ENABLEBUTTON, IDP_DELETEALL, FALSE);
                            SendMessage(toolBarWnd, TB_ENABLEBUTTON, IDP_BUILD, FALSE);
                            SendMessage(toolBarWnd, TB_ENABLEBUTTON, IDP_OPEN, FALSE);

                            isBuilding = 1;
                            SetTimer(hwnd, 0, 100, NULL);
                            return FALSE;
                       }
                       case IDP_DELETE:
                       {
                            if (isBuilding)
                               return FALSE;

                            int *tab = {NULL};
                            int count = get_selected_items(GetDlgItem(dragAndDropWnd, IDLV_LIST), &tab), i;

                            if (count > 1)
                               sprintf(buffer, "Voulez-vous vraiment effacer ces %d éléments de la liste ?", count);
                            else if (count == 1)
                                 strcpy(buffer, "Voulez-vous vraiment effacer cet élément de la liste ?");
                            else return FALSE;

                            if (MessageBox(mainWnd, buffer, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
                               return FALSE;

                            for (i = 0 ; i < count ; i++)
                            {
                                ListView_DeleteItem(listViewWnd, tab[i]);
                                ComboBox_DeleteString(GetDlgItem(hwnd, IDCB_HIDDENLIST), tab[i]);
                            }

                            free(tab);
                            modified = 1;
                            return FALSE;
                       }
                       case IDP_DELETEALL:
                            if (isBuilding)
                               return FALSE;

                            if (MessageBox(mainWnd, "Voulez-vous vraiment effacer la liste en entier ?", "Attention", MB_YESNO | MB_ICONWARNING) == IDNO)
                               return FALSE;
                            ListView_DeleteAllItems(listViewWnd);
                            ComboBox_ResetContent(GetDlgItem(hwnd, IDCB_HIDDENLIST));
                            modified = 1;
                            return FALSE;
                       case IDP_PROPERTIES:
                            if (ListView_GetSelectedCount(listViewWnd) > 0)
                               DialogBoxParam(mainInstance, "properties", hwnd, (DLGPROC)PropertiesProc, (LPARAM)hwnd);
                            return FALSE;
                       case IDP_EXE:
                       {
                            int *tab = {NULL};
                            int count = get_selected_items(GetDlgItem(dragAndDropWnd, IDLV_LIST), &tab);

                            if (count <= 0)
                            {
                               free(tab);
                               return FALSE;
                            }

                            ComboBox_SetCurSel(GetDlgItem(hwnd, IDCB_HIDDENLIST), tab[0]);
                            ComboBox_GetText(GetDlgItem(hwnd, IDCB_HIDDENLIST), buffer, MAX_CHAINE);
                            if (strchr(buffer, '\n') != NULL)
                               *strchr(buffer, '\n') = '\0';
                            ShellExecute(NULL, "open", buffer, NULL, NULL, SW_SHOW);

                            free(tab);
                            return FALSE;
                       }
                       case IDP_OPEN:
                       {
                            if (isBuilding)
                               return FALSE;

                            if (modified)
                            {
                                switch (MessageBox(mainWnd, "Sauvegarder le projet ?", "Confirmation", MB_YESNOCANCEL | MB_ICONINFORMATION))
                                {
                                       case IDYES:
                                            SendMessage(hwnd, WM_COMMAND, IDP_SAVE, 0);
                                            break;
                                       case IDCANCEL:
                                            return FALSE;
                                }
                            }

                            if (argts[0] == '\0')
                            {
                                OPENFILENAME ofn;
                                memset(&ofn, 0, sizeof(OPENFILENAME));
                                ofn.lStructSize = sizeof(OPENFILENAME);
                                ofn.hwndOwner = hwnd;
                                ofn.nMaxFile = MAX_CHAINE;
                                ofn.lpstrFilter = "TMC listes  (*.tml)\0*.tml\0Tous les fichiers  (*.*)\0*.*\0";
                                ofn.lpstrFile = buffer;
                                ofn.lpstrTitle = "Ouvrir un projet";
                                ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
                                ofn.lpstrInitialDir = NULL;

                                if (!GetOpenFileName(&ofn))
                                   return FALSE;
                            }
                            else
                            {
                                strcpy(buffer, argts);
                                argts[0] = '\0';
                            }

                            strcpy(adresseFichierListe, buffer);

                            ListView_DeleteAllItems(listViewWnd);
                            ComboBox_ResetContent(GetDlgItem(hwnd, IDCB_HIDDENLIST));

                            FILE *file = fopen(buffer, "r");
                            if (file == NULL)
                            {
                               MessageBox(mainWnd, "Impossible d'ouvrir le fichier", "Erreur !", MB_OK);
                               return FALSE;
                            }

                            modified = 0;

                            char buffer2[MAX_CHAINE] = "\0";
                            int noAdd = 0;

                            fgets(TMDestinationE, MAX_CHAINE, file);
                            if (strchr(TMDestinationE, '\n') != NULL)
                               *strchr(TMDestinationE, '\n') = '\0';
                            convert_from_environment(TMDestinationE, TMDestination);

                            while (fgets(buffer, MAX_CHAINE, file) != NULL)
                            {
                                  fgets(buffer2, MAX_CHAINE, file);
                                  if (strchr(buffer2, '\n') != NULL);
                                     *strrchr(buffer2, '\n') = '\0';

                                  if (strchr(buffer, '\n') != NULL);
                                     *strrchr(buffer, '\n') = '\0';

                                  noAdd = 0;
                                  if (test_exist(buffer) != 1)
                                  {
                                      sprintf(buffer2, "%s\nImpossible d'ouvrir le fichier. Maintenir dans la liste ?", buffer);
                                      if (MessageBox(mainWnd, buffer2, "Attention", MB_YESNO | MB_ICONWARNING) == IDNO)
                                      {
                                         noAdd = 1;
                                         modified = 1;
                                      }
                                  }

                                  sprintf(buffer, "%s\n%s", buffer, buffer2);
                                  if (!noAdd)
                                     ComboBox_AddString(GetDlgItem(hwnd, IDCB_HIDDENLIST), buffer);

                                  if (strchr(buffer, '\n') != NULL);
                                     *strrchr(buffer, '\n') = '\0';

                                  if (!noAdd)
                                     add_item(listViewWnd, buffer);
                            }

                            fclose(file);

                            if ((file = fopen(fichierInit, "w")) != NULL)
                            {
                                fputs(adresseFichierListe, file);
                                fclose(file);
                            }
                            return FALSE;
                       }
                       case IDP_SAVE:
                       {
                            OPENFILENAME ofn;
                            memset(&ofn, 0, sizeof(OPENFILENAME));
                            ofn.lStructSize = sizeof(OPENFILENAME);
                            ofn.hwndOwner = mainWnd;
                            ofn.lpstrFilter = "TMC listes  (*.tml)\0*.tml\0Tous les fichiers  (*.*)\0*.*\0";
                            ofn.lpstrFile = buffer;
                            ofn.nMaxFile = MAX_CHAINE;
                            ofn.lpstrTitle = "Sauvegarder sous";
                            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER;
                            ofn.lpstrInitialDir = NULL;
                            ofn.lpstrDefExt = ".tml";
                            if (!GetSaveFileName(&ofn))
                               return FALSE;

                            strcpy(adresseFichierListe, buffer);

                            FILE *file = fopen(buffer, "w");
                            if (file == NULL)
                            {
                                MessageBox(hwnd, "Impossible d'écrire le fichier", "Erreur !", MB_OK | MB_ICONERROR);
                                return FALSE;
                            }

                            int count = ComboBox_GetCount(GetDlgItem(hwnd, IDCB_HIDDENLIST)), i;

                            fprintf(file, "%s\n", TMDestinationE);
                            for (i = 0 ; i < count ; i++)
                            {
                                ComboBox_SetCurSel(GetDlgItem(hwnd, IDCB_HIDDENLIST), i);
                                ComboBox_GetText(GetDlgItem(hwnd, IDCB_HIDDENLIST), buffer, MAX_CHAINE);
                                fprintf(file, "%s\n", buffer);
                            }

                            fclose(file);

                            if ((file = fopen(fichierInit, "w")) != NULL)
                            {
                                fputs(adresseFichierListe, file);
                                fclose(file);
                            }

                            modified = 0;
                            MessageBox(mainWnd, "Fichier sauvegardé !", "Information", MB_OK | MB_ICONINFORMATION);
                            return FALSE;
                       }
                       case IDP_DEF:
                       {
                           if (GetFocus() == listViewWnd)
                           {
                              SendMessage(hwnd, WM_COMMAND, IDP_PROPERTIES, 0);
                              return FALSE;
                           }

                           GetDlgItemText(hwnd, IDE_LIST, buffer, MAX_CHAINE);

                           LVFINDINFO lvfi;
                           lvfi.flags = LVFI_PARTIAL | LVFI_STRING | LVFI_WRAP;
                           lvfi.psz = buffer;

                           int index = ListView_FindItem(listViewWnd, get_first_selected(listViewWnd), &lvfi);
                           if (index < 0)
                              return FALSE;

                           select_items(listViewWnd, &index, 1, 1);

                           NMITEMACTIVATE nmia;
                           nmia.iItem = index;
                           nmia.hdr.hwndFrom = listViewWnd;
                           nmia.hdr.idFrom = IDLV_LIST;
                           nmia.hdr.code = NM_CLICK;
                           SendMessage(dragAndDropWnd, WM_NOTIFY, IDLV_LIST, (LPARAM)&nmia);

                           return FALSE;
                       }
                }
                return FALSE;
    }

    return FALSE;
}



LRESULT CALLBACK PropertiesProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                centrer_fenetre(hwnd, GetParent(hwnd));

                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconProperties);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconProperties);

                propertiesListWnd = (HWND)lParam;

                propertiesIndex = get_first_selected(listViewWnd);
                ComboBox_SetCurSel(GetDlgItem(propertiesListWnd, IDCB_HIDDENLIST), propertiesIndex);
                ComboBox_GetText(GetDlgItem(propertiesListWnd, IDCB_HIDDENLIST), buffer, MAX_CHAINE);

                char adress[MAX_CHAINE];
                strcpy(adress, buffer);
                if (strchr(adress, '\n') != NULL)
                   *strchr(adress, '\n') = '\0';
                if (strchr(buffer, '\n') != NULL)
                   strcpy(buffer, strchr(buffer, '\n') + 1);

                SetDlgItemText(hwnd, IDE_DEST, buffer);
                if (buffer[0] == '\0' || buffer[1] != ':')
                   CheckDlgButton(hwnd, IDC_USETMDEST, BST_CHECKED);
                sprintf(buffer, "Fichier : %s", adress);
                SetDlgItemText(hwnd, IDT_FILE, buffer);

                sprintf(buffer, "%s\\...", TMDestination);
                SetDlgItemText(hwnd, IDC_USETMDEST, buffer);

                if (isBuilding)
                   EnableWindow(GetDlgItem(hwnd, IDP_OK), FALSE);

                SetFocus(GetDlgItem(hwnd, IDC_USETMDEST));
                return FALSE;
           }
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDP_OK:
                      {
                           char dest[MAX_CHAINE];
                           GetDlgItemText(hwnd, IDE_DEST, dest, MAX_CHAINE);
                           char adress[MAX_CHAINE];
                           GetDlgItemText(hwnd, IDT_FILE, adress, MAX_CHAINE);
                           strcpy(adress, adress + strlen("Fichier : "));

                           sprintf(buffer, "%s\n%s", adress, dest);
                           ComboBox_DeleteString(GetDlgItem(propertiesListWnd, IDCB_HIDDENLIST), propertiesIndex);
                           ComboBox_InsertString(GetDlgItem(propertiesListWnd, IDCB_HIDDENLIST), propertiesIndex, buffer);

                           modified = 1;
                           EndDialog(hwnd, 1);
                           return FALSE;
                      }
                      case IDP_CANCEL:
                           EndDialog(hwnd, 0);
                           return FALSE;
                      case IDP_BASEEDIT:
                           if (DialogBox(mainInstance, "baseModif", hwnd, (DLGPROC)baseModifProc))
                              SetDlgItemText(hwnd, IDC_USETMDEST, TMDestination);
                           return FALSE;
                      case IDP_BROWSE:
                      {
                           char title[MAX_CHAINE] = "Choisissez un répertoire d'extraction.";
                           buffer[0] = '\0';
                           ITEMIDLIST *idPath;

                           BROWSEINFO bi;
                           bi.hwndOwner = hwnd;
                           bi.pidlRoot = NULL;
                           bi.pszDisplayName = buffer;
                           bi.lpszTitle = title;
                           bi.ulFlags = BIF_NEWDIALOGSTYLE;
                           bi.lpfn = NULL;
                           bi.lParam = 0;
                           bi. iImage = 0;
                           idPath = SHBrowseForFolder(&bi);

                           if (idPath != NULL)
                           {
                               SHGetPathFromIDList(idPath, buffer);
                               if (IsDlgButtonChecked(hwnd, IDC_USETMDEST) && strstr(buffer, TMDestination) != NULL)
                                  strcpy(buffer, buffer + strlen(TMDestination) + 1);
                               SetDlgItemText(hwnd, IDE_DEST, buffer);
                               CoTaskMemFree(idPath);
                           }

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


LRESULT CALLBACK MpProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select, time = 0;
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                       case IDP_PLAYMUSIC:
                            if (channel >= 0 && FSOUND_GetPaused(channel))
                            {
                                 FSOUND_SetPaused(channel, FALSE);
                                 SetTimer(hwnd, 0, 50, NULL);
                            }
                            else
                            {
                                stream = FSOUND_Stream_Open(adresseMusique, FSOUND_MPEGACCURATE, 0, 0);

                                if (stream == NULL)
                                   MessageBox(hwnd, "Impossible de charger la musique.", "Erreur !", MB_OK | MB_ICONERROR);
                                else
                                {
                                    channel = FSOUND_Stream_Play(FSOUND_FREE, stream);
                                    SendMessage(GetDlgItem(hwnd, IDTB_MUSIC), TBM_SETRANGE, TRUE, MAKELONG(0, FSOUND_Stream_GetLengthMs(stream) / 100.0));
                                    SetTimer(hwnd, 0, 100, NULL);
                                }
                            }

                            return FALSE;
                       case IDP_PAUSEMUSIC:
                            FSOUND_SetPaused(channel, TRUE);
                            KillTimer(hwnd, 0);
                            return FALSE;
                       case IDP_STOPMUSIC:
                            if (stream == NULL)
                               return FALSE;

                            KillTimer(hwnd, 0);

                            FSOUND_Stream_Stop(stream);
                            FSOUND_Stream_Close(stream);

                            stream = NULL;
                            channel = -1;

                            SetDlgItemText(hwnd, IDT_MUSICTIME, "00:00");
                            SendMessage(GetDlgItem(hwnd, IDTB_MUSIC), TBM_SETPOS, TRUE, 0);
                            return FALSE;
                }
                return FALSE;
           case WM_DESTROY:
                EndDialog(hwnd, 0);
                return FALSE;
           case WM_HSCROLL:
                if ((LOWORD(wParam) != TB_THUMBTRACK && LOWORD(wParam) != TB_THUMBPOSITION) || stream == NULL)
                   return FALSE;

                SendMessage(GetDlgItem(hwnd, IDTB_MUSIC), TBM_SETPOS, TRUE, HIWORD(wParam));
                FSOUND_Stream_SetTime(stream, HIWORD(wParam) * 100);
                return FALSE;
           case WM_TIMER:
                if (stream != NULL)
                   time = FSOUND_Stream_GetTime(stream);

                sprintf(buffer, "%02d:%02d", time / 60000, (time / 1000) % 60);
                SetDlgItemText(hwnd, IDT_MUSICTIME, buffer);
                SendMessage(GetDlgItem(hwnd, IDTB_MUSIC), TBM_SETPOS, TRUE, time / 100.0);
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK BeginPackageProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;
    char buffer[MAX_CHAINE] = "\0";
    BOOL state;

    switch (msg)
    {
           case WM_INITDIALOG:
                centrer_fenetre(hwnd, GetParent(hwnd));

                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconPackage);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconPackage);

                if (includeUnknow)
                   CheckDlgButton(hwnd, IDC_INCLUDEUNKNOW, BST_CHECKED);
                if (includeOutOfDir)
                   CheckDlgButton(hwnd, IDC_INCLUDEOUT, BST_CHECKED);
                if (usePassword)
                   CheckDlgButton(hwnd, IDC_PASSWORD, BST_CHECKED);

                SetDlgItemText(hwnd, IDE_PASSWORD, packagePassword);
                SetDlgItemText(hwnd, IDE_PASSWORD2, packagePassword);

                if (IsDlgButtonChecked(hwnd, IDC_PASSWORD))
                   state = TRUE;
                else state = FALSE;

                EnableWindow(GetDlgItem(hwnd, IDE_PASSWORD), state);
                EnableWindow(GetDlgItem(hwnd, IDE_PASSWORD2), state);
                EnableWindow(GetDlgItem(hwnd, IDT_TEXT), state);

                return FALSE;
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDP_CANCEL:
                           EndDialog(hwnd, 0);
                           return FALSE;
                      case IDC_PASSWORD:
                           if (IsDlgButtonChecked(hwnd, IDC_PASSWORD))
                              state = TRUE;
                           else state = FALSE;

                           EnableWindow(GetDlgItem(hwnd, IDE_PASSWORD), state);
                           EnableWindow(GetDlgItem(hwnd, IDE_PASSWORD2), state);
                           EnableWindow(GetDlgItem(hwnd, IDT_TEXT), state);

                           return FALSE;
                      case IDP_OK:
                      {
                           if (IsDlgButtonChecked(hwnd, IDC_INCLUDEUNKNOW))
                              includeUnknow = 1;
                           else includeUnknow = 0;

                           if (IsDlgButtonChecked(hwnd, IDC_INCLUDEOUT))
                              includeOutOfDir = 1;
                           else includeOutOfDir = 0;

                           if (IsDlgButtonChecked(hwnd, IDC_PASSWORD))
                              usePassword = 1;
                           else usePassword = 0;

                           char buffer2[MAX_CHAINE] = "\0";
                           GetDlgItemText(hwnd, IDE_PASSWORD, buffer, MAX_CHAINE);
                           GetDlgItemText(hwnd, IDE_PASSWORD2, buffer2, MAX_CHAINE);
                           if (strcmp(buffer, buffer2) != 0)
                           {
                              MessageBox(hwnd, "Mots de passe différents !", "Attention !", MB_OK | MB_ICONWARNING);
                              return FALSE;
                           }

                           strcpy(packagePassword, buffer);
                           if (packagePassword[0] == '\0')
                              usePassword = 0;

                           EndDialog(hwnd, 1);
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


LRESULT CALLBACK DragAndDropProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE] = "\0";
    RECT rect;
    PAINTSTRUCT paintst;
    HDC hdc;

    switch (msg)
    {
           case WM_PAINT:
           {
               hdc = BeginPaint(hwnd, &paintst);

               HBRUSH brush = GetStockObject(WHITE_BRUSH);
               SelectObject(hdc, brush);
               HPEN pen = GetStockObject(DC_PEN);
               SelectObject(hdc, pen);
               SetDCPenColor(hdc, RGB(150, 150, 150));
               GetClientRect(hwnd, &rect);
               Rectangle(hdc, 0, 0, rect.right, rect.bottom);

               DeleteObject(brush);
               DeleteObject(pen);
               EndPaint(hwnd, &paintst);

               return FALSE;
           }
           case WM_DROPFILES:
           {
                 HDROP hDrop = (HDROP)wParam;
                 dragFiles = 0;
                 int count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0), i;

                 for (i = 0 ; i < count ; i++)
                 {
                     DragQueryFile(hDrop, i, buffer, MAX_CHAINE);

                     if (tableauFichiersAOuvrir[i] != NULL)
                        free(tableauFichiersAOuvrir[i]);
                     tableauFichiersAOuvrir[i] = malloc(MAX_CHAINE);
                     strcpy(tableauFichiersAOuvrir[i], buffer);

                     dragFiles++;
                 }

                 DragFinish(hDrop);
                 SendMessage(listWnd, WM_COMMAND, IDP_ADD, 0);
                 return FALSE;
           }
           case WM_NOTIFY:
           {
                NMHDR *nmhdr = (NMHDR*)lParam;

                switch (nmhdr->code)
                {
                       case TTN_SHOW:
                            return FALSE;
                       case TTN_POP:
                            create_toolTips();
                            return FALSE;
                       case NM_DBLCLK:
                            SendMessage(listWnd, WM_COMMAND, IDP_PROPERTIES, 0);
                            return FALSE;
                       case LVN_KEYDOWN:
                       {
                            NMLVKEYDOWN *nmlvkd = (NMLVKEYDOWN*)lParam;
                            if (nmlvkd->wVKey == VK_DELETE)
                               SendMessage(listWnd, WM_COMMAND, IDP_DELETE, 0);
                            return FALSE;
                       }
                       case LVN_ITEMCHANGED:
                       {
                            if (nmhdr->idFrom != IDLV_LIST)
                               return FALSE;

                            NMLISTVIEW *nmlv = (NMLISTVIEW*)lParam;
                            if ((nmlv->uNewState & LVIS_SELECTED) != LVIS_SELECTED)
                               return FALSE;

                            int index = nmlv->iItem;
                            if (index < 0)
                               return FALSE;

                            ComboBox_GetLBText(GetDlgItem(listWnd, IDCB_HIDDENLIST), index, buffer);
                            if (strchr(buffer, '\n') != NULL)
                               *strchr(buffer, '\n') = '\0';

                            char temp[MAX_CHAINE];
                            sprintf(temp, "%s\\%s", dossierBase, "temp.bmp");
                            if (imageAffichage != NULL)
                               DeleteObject(imageAffichage);

                            ShowWindow(mpWnd, SW_HIDE);
                            SendMessage(mpWnd, WM_COMMAND, IDP_STOPMUSIC, 0);

                            switch (retrieve_info_text(buffer, informationText, &imageAffichage))
                            {
                                    case TRACK_CHALLENGE:
                                         typeAffichage = AFF_CHALLENGE;
                                         break;
                                    case TRACK_IMAGE:
                                         typeAffichage = AFF_IMAGE;
                                         break;
                                    case TRACK_MUSIC:
                                         strcpy(adresseMusique, buffer);
                                         typeAffichage = AFF_MUSIC;
                                         break;
                                    case TRACK_REPLAY:
                                         typeAffichage = AFF_REPLAY;
                                         break;
                                    case TRACK_PROFILE:
                                         typeAffichage = AFF_PROFILE;
                                         break;
                                    case TRACK_LOCATOR:
                                         typeAffichage = AFF_LOCATOR;
                                         break;
                                    default:
                                            typeAffichage = AFF_UNKNOW;
                                            break;
                            }

                            RECT rect = tailleMainWnd;
                            rect.left = minListWnd;
                            InvalidateRect(mainWnd, &rect, FALSE);
                            UpdateWindow(mainWnd);

                            if (strchr(buffer, '\\') != NULL)
                               strcpy(buffer, strrchr(buffer, '\\') + 1);
                            SetDlgItemText(listWnd, IDE_LIST, buffer);

                            return FALSE;
                       }
                }

                return FALSE;
           }
    }
    return FALSE;
}


LRESULT CALLBACK baseModifProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconProperties);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconProperties);
                SetDlgItemText(hwnd, IDE_BASE, TMDestinationE);
                SetDlgItemText(hwnd, IDT_BASE, TMDestination);

                if (isBuilding)
                   EnableWindow(GetDlgItem(hwnd, IDP_OK), FALSE);
                return FALSE;
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                       case IDP_CANCEL:
                            EndDialog(hwnd, 0);
                            return FALSE;
                       case IDP_HELP:
                            sprintf(buffer, "%s::/Edition_du_repertoire_d_extraction.htm", helpFileAdress);
                            HtmlHelp(GetDesktopWindow(), buffer, HH_DISPLAY_TOPIC, 0);
                            return FALSE;
                       case IDE_BASE:
                       {
                            if (HIWORD(wParam) != EN_CHANGE)
                               return FALSE;
                            GetDlgItemText(hwnd, IDE_BASE, buffer, MAX_CHAINE);

                            char converted[MAX_CHAINE] = "\0";
                            convert_from_environment(buffer, converted);
                            SetDlgItemText(hwnd, IDT_BASE, converted);
                            return FALSE;
                       }
                       case IDP_OK:
                            GetDlgItemText(hwnd, IDE_BASE, buffer, MAX_CHAINE);

                            if (strchr(buffer + 3, ':') != NULL)
                               MessageBox(hwnd, "Adresse invalide", "Attention", MB_OK | MB_ICONWARNING);
                            else
                            {
                                strcpy(TMDestinationE, buffer);
                                convert_from_environment(TMDestinationE, TMDestination);
                                EndDialog(hwnd, 1);
                            }

                            return FALSE;
                }

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK ChooseTMDProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
                centrer_fenetre(hwnd, NULL);
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconPackage);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconPackage);

                return FALSE;
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDP_CANCEL:
                           EndDialog(hwnd, 0);
                           return FALSE;
                      case IDP_BROWSE:
                      {
                           BROWSEINFO bi;
                           bi.hwndOwner = hwnd;
                           bi.pidlRoot = NULL;
                           bi.pszDisplayName = NULL;
                           bi.lpszTitle = "Choisissez un dossier.";
                           bi.ulFlags = BIF_NEWDIALOGSTYLE;
                           bi.lpfn = NULL;
                           bi.lParam = 0;
                           bi. iImage = 0;
                           ITEMIDLIST *idPath = SHBrowseForFolder(&bi);

                           if (idPath != NULL)
                           {
                               SHGetPathFromIDList(idPath, buffer);
                               SetDlgItemText(hwnd, IDE_TMD, buffer);
                               CoTaskMemFree(idPath);
                           }

                           return FALSE;
                      }
                      case IDP_OK:
                           GetDlgItemText(hwnd, IDE_TMD, buffer, MAX_CHAINE);

                           if (test_exist(buffer) != 2)
                              MessageBox(hwnd, "Le dossier spécifié n'existe pas ou est inaccessible.", "Attention", MB_OK | MB_ICONWARNING);
                           else
                           {
                               strcpy(TMDirectory, buffer);
                               EndDialog(hwnd, 1);
                           }

                           return FALSE;
                }

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK ChooseTMIDProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
                centrer_fenetre(hwnd, NULL);
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconPackage);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconPackage);

                return FALSE;
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDP_CANCEL:
                           EndDialog(hwnd, 0);
                           return FALSE;
                      case IDP_BROWSE:
                      {
                           BROWSEINFO bi;
                           bi.hwndOwner = hwnd;
                           bi.pidlRoot = NULL;
                           bi.pszDisplayName = NULL;
                           bi.lpszTitle = "Choisissez un dossier.";
                           bi.ulFlags = BIF_NEWDIALOGSTYLE;
                           bi.lpfn = NULL;
                           bi.lParam = 0;
                           bi. iImage = 0;
                           ITEMIDLIST *idPath = SHBrowseForFolder(&bi);

                           if (idPath != NULL)
                           {
                               SHGetPathFromIDList(idPath, buffer);
                               SetDlgItemText(hwnd, IDE_TMD, buffer);
                               CoTaskMemFree(idPath);
                           }

                           return FALSE;
                      }
                      case IDP_OK:
                           GetDlgItemText(hwnd, IDE_TMID, buffer, MAX_CHAINE);

                           if (test_exist(buffer) != 2)
                              MessageBox(hwnd, "Le dossier spécifié n'existe pas ou est inaccessible.", "Attention", MB_OK | MB_ICONWARNING);
                           else
                           {
                               strcpy(TMInstallDir, buffer);
                               EndDialog(hwnd, 1);
                           }

                           return FALSE;
                }

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}



int retrieve_type(char track[])
{
    char buffer[MAX_CHAINE];
    int i;
    strcpy(buffer, track);

    for (i = 0 ; buffer[i] != '\0' ; i++)
        buffer[i] = toupper(buffer[i]);

    if (strstr(buffer, ".BIK") != NULL)
         return TRACK_IMAGE;
    else if (strstr(buffer, ".WAV") != NULL)
         return TRACK_SOUND;
    else if (strstr(buffer, ".CONSTRUCTIONCAMPAIGN.GBX") != NULL)
         return TRACK_CAMPAIGN;
    else return retrieve_info_text(track, buffer, NULL);
}


HBITMAP open_image(char adress[], int type, char temporaryFile[], RECT *size)
{
        char buffer[MAX_CHAINE] = "\0";
        int i;

        if (type == IMG_UNKNOW)
        {
                 strcpy(buffer, adress);
                 if (strchr(buffer, '.') != NULL)
                    strcpy(buffer, strrchr(buffer, '.') + 1);
                 for (i = 0 ; buffer[i] != '\0' ; i++)
                     buffer[i] = toupper(buffer[i]);

                 if (strcmp(buffer, "BMP") == 0)
                    type = IMG_BMP;
                 else if (strcmp(buffer, "JPG") == 0 || strcmp(buffer, "JPEG") == 0)
                      type = IMG_JPG;
                 else if (strcmp(buffer, "PNG") == 0)
                      type = IMG_PNG;
                 else if (strcmp(buffer, "TIF") == 0 || strcmp(buffer, "TIFF") == 0)
                      type = IMG_TIF;
                 else if (strcmp(buffer, "ICO") == 0)
                      type = IMG_ICO;
                 else if (strcmp(buffer, "TGA") == 0)
                      type = IMG_TARGA;
                 else if (strcmp(buffer, "GIF") == 0)
                      type = IMG_GIF;
                 else if (strcmp(buffer, "DDS") == 0)
                      type = IMG_DDS;
        }


        HBITMAP handle = NULL;
        FIBITMAP *bitmap = FreeImage_Load(type, adress, 0);
        FreeImage_Save(FIF_BMP, bitmap, temporaryFile, 0);

        if (size != NULL)
           handle = (HBITMAP) LoadImage(NULL, temporaryFile, IMAGE_BITMAP, size->right, size->bottom, LR_LOADFROMFILE);
        else handle = (HBITMAP) LoadImage(NULL, temporaryFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

        FreeImage_Unload(bitmap);

        return handle;
}



int retrieve_info_text(char adress[], char text[], HBITMAP *bmp)
{
    Info_Image info1;
    Info_Challenge info2;
    Info_Replay info3;
    Info_Music info4;
    Info_Profile info5;
    Info_Unknow info6;
    Info_Locator info7;


    if (retrieve_info_image(adress, &info1))
    {
        sprintf(text, "IMAGE\r\nFichier : %s\r\nType : %s\r\nDimensions : %d x %d\r\nPoids : %d Ko", info1.name, info1.type, info1.w, info1.h, info1.weight);
        if (bmp != NULL)
           *bmp = info1.img;
        else DeleteObject(info1.img);
        return TRACK_IMAGE;
    }
    else if (retrieve_info_music(adress, &info4))
    {
         if (info4.length.wYear <= 0)
            sprintf(text, "MUSIQUE\r\nFichier : %s\r\nType : %s\r\nLongueur : %02d:%02d:%02d\r\nPoids : %d Ko", info4.name, info4.type, info4.length.wHour, info4.length.wMinute, info4.length.wSecond, info4.weight);
         else sprintf(text, "MUSIQUE\r\nFichier : %s\r\nType : %s\r\nLongueur : inconnue\r\nPoids : %d Ko", info4.name, info4.type, info4.weight);
         return TRACK_MUSIC;
    }
    else if (retrieve_info_locator(adress, &info7))
    {
         sprintf(text, "LOCATOR\nNom du fichier : %s\nURL cible : %s\nFichier cible : %s", info7.name, info7.url, info7.targetType);
         return TRACK_LOCATOR;
    }
    else if (retrieve_info_challenge(adress, &info2))
    {
         sprintf(text, "CHALLENGE\r\n%s, par %s\r\n%d Coopers, %d Ko\r\n", info2.name, info2.author, info2.tall, info2.weight);

         if (info2.authorTime.wHour > 0)
            sprintf(text, "%s\nMédaille auteur : %02d:%02d:%02d.%02d", text, info2.authorTime.wHour, info2.authorTime.wMinute, info2.authorTime.wSecond, info2.authorTime.wMilliseconds / 10);
         else sprintf(text, "%s\nMédaille auteur : %02d:%02d.%02d", text, info2.authorTime.wMinute, info2.authorTime.wSecond, info2.authorTime.wMilliseconds / 10);
         if (info2.authorTime.wHour > 0)
            sprintf(text, "%s\nMédaille or : %02d:%02d:%02d.%02d", text, info2.goldTime.wHour, info2.goldTime.wMinute, info2.goldTime.wSecond, info2.goldTime.wMilliseconds / 10);
         else sprintf(text, "%s\nMédaille or : %02d:%02d.%02d", text, info2.goldTime.wMinute, info2.goldTime.wSecond, info2.goldTime.wMilliseconds / 10);
         if (info2.silverTime.wHour > 0)
            sprintf(text, "%s\nMédaille argent : %02d:%02d:%02d.%02d", text, info2.silverTime.wHour, info2.silverTime.wMinute, info2.silverTime.wSecond, info2.silverTime.wMilliseconds / 10);
         else sprintf(text, "%s\nMédaille argent : %02d:%02d.%02d", text, info2.silverTime.wMinute, info2.silverTime.wSecond, info2.silverTime.wMilliseconds / 10);
         if (info2.bronzeTime.wHour > 0)
            sprintf(text, "%s\nMédaille bronze : %02d:%02d:%02d.%02d", text, info2.bronzeTime.wHour, info2.bronzeTime.wMinute, info2.bronzeTime.wSecond, info2.bronzeTime.wMilliseconds / 10);
         else sprintf(text, "%s\nMédaille bronze : %02d:%02d.%02d", text, info2.bronzeTime.wMinute, info2.bronzeTime.wSecond, info2.bronzeTime.wMilliseconds / 10);

         if (bmp != NULL)
           *bmp = info2.img;
        else DeleteObject(info2.img);
        return TRACK_CHALLENGE;
    }
    else if (retrieve_info_replay(adress, &info3))
    {
         if (info3.time.wHour > 0)
            sprintf(text, "REPLAY\r\nCircuit : %s\r\nTemps : %02d:%02d:%02d.%02d\r\nAuteur : %s", info3.track, info3.time.wHour, info3.time.wMinute, info3.time.wSecond, info3.time.wMilliseconds / 10, info3.author);
         else sprintf(text, "REPLAY\r\nCircuit : %s\r\nTemps : %02d:%02d.%02d\r\nAuteur : %s", info3.track, info3.time.wMinute, info3.time.wSecond, info3.time.wMilliseconds / 10, info3.author);
         return TRACK_REPLAY;
    }
    else if (retrieve_info_profile(adress, &info5))
    {
         sprintf(text, "PROFIL\r\nNom : %s\r\nPoids : %d Ko", info5.name, info5.weight);
         return TRACK_PROFILE;
    }
    else
    {
        retrieve_info_unknow(adress, &info6);
        sprintf(text, "FICHIER NON RECONNU\nNom : %s\nType : %s\nPoids : %d Ko", info6.name, info6.type, info6.weight);
        return TRACK_UNKNOW;
    }
}


int retrieve_info_challenge(char adress[], Info_Challenge *info)
{
    FILE *file = fopen(adress, "rb");
    if (file == NULL)
       return 0;

    int identifier;
    fseek(file, 9, SEEK_SET);
    fread(&identifier, sizeof(int), 1, file);

    if (identifier != 50606080)
    {
       fclose(file);
       return 0;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    info->weight = size / pow(2,10);
    rewind(file);

    void *data = malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    char *beginning = NULL;
    int success = 0, i, j;

    for (i = 0 ; i < size && !success ; i++)
    {
       if (*((char*)data + i) == 'n' && (beginning = strstr(data + i, "name=")) != NULL)
          success = 1;
    }
    i = (int)beginning - (int)data;

    char *beginning2 = NULL;
    success = 0;
    for (j = 0 ; j < size && !success ; j++)
    {
       if (*((char*)data + j) == '/' && (beginning2 = strstr(data + j, "/><deps>")) != NULL)
          success = 1;
    }
    j = (int)beginning2 - (int)data;

    char string[50000] = "\0";
    int length = j - i;
    CopyMemory(string, beginning, length);
    for (i = 0 ; i < length ; i++)
    {
        if (string[i] == '\0')
           string[i] = '\n';
    }

    beginning = strchr(string, '"') + 1;
    char *end = strchr(beginning, '"');
    CopyMemory(info->name, beginning, end - beginning);
    info->name[end - beginning] = '\0';
    convert_name(info->name, info->name);

    beginning = strchr(end + 1, '"') + 1;
    end = strchr(beginning, '"');
    CopyMemory(info->author, beginning, end - beginning);
    info->author[end - beginning] = '\0';
    convert_name(info->author, info->author);

    beginning = strstr(end + 1, "price=") + strlen("price=\"");
    info->tall = strtol(beginning, NULL, 10);

    beginning = strstr(end + 1, "bronze=") + strlen("bronze=\"");
    conversion_time(&(info->bronzeTime), strtol(beginning, NULL, 10));

    beginning = strstr(end + 1, "silver=") + strlen("silver=\"");
    conversion_time(&(info->silverTime), strtol(beginning, NULL, 10));

    beginning = strstr(end + 1, "gold=") + strlen("gold=\"");
    conversion_time(&(info->goldTime), strtol(beginning, NULL, 10));

    beginning = strstr(end + 1, "authortime=") + strlen("authortime=\"");
    conversion_time(&(info->authorTime), strtol(beginning, NULL, 10));


    success = 0;
    for (i = 0 ; i < size && !success ; i++)
    {
       if (*((char*)data + i) == '<' && (beginning = strstr(data + i, "<Thumbnail.jpg>")) != NULL)
          success = 1;
    }
    beginning += strlen("<Thumbnail.jpg>");
    i = (int)beginning - (int)data;

    success = 0;
    for (j = 0 ; j < size && !success ; j++)
    {
       if (*((char*)data + j) == '<' && (beginning2 = strstr(data + j, "</Thumbnail.jpg>")) != NULL)
          success = 1;
    }
    j = (int)beginning2 - (int)data;

    CopyMemory(string, beginning, j - i);

    char buffer[MAX_CHAINE] = "\0";
    sprintf(buffer, "%s\\temp.jpg", dossierBase);
    if ((file = fopen(buffer, "wb")) != NULL)
    {
        fwrite(string, 1, j - i, file);
        fclose(file);

        char temp[MAX_CHAINE];
        sprintf(temp, "%s\\temp.bmp", dossierBase);
        RECT rect;
        rect.right = 100;
        rect.bottom = 100;
        info->img = open_image(buffer, IMG_JPG, temp, &rect);
    }

    success = 0;
    for (i = 0 ; i < size && !success ; i++)
    {
       if (*((char*)data + i) == '<' && (beginning = strstr(data + i, "<deps>")) != NULL)
          success = 1;
    }
    beginning += strlen("<deps>");
    i = (int)beginning - (int)data;

    success = 0;
    for (j = 0 ; j < size && !success ; j++)
    {
       if (*((char*)data + j) == '<' && (beginning2 = strstr(data + j, "</deps>")) != NULL)
          success = 1;
    }
    j = (int)beginning2 - (int)data;

    CopyMemory(string, beginning, j - i);

    info->dep[0] = '\0';
    while ((beginning = strchr(string, '"')) != NULL)
    {
          strcpy(string, beginning + 1);
          end = strchr(string, '"');
          if (end == NULL)
             break;

          *end = '\0';
          strcpy(buffer, string);
          strcpy(string, end + 1);

          if (strlen(buffer) + strlen(info->dep) >= 10000)
             break;

          char adress[MAX_CHAINE] = "\0";
          sprintf(adress, "%s\\GameData\\%s", TMInstallDir, buffer);
          if (test_exist(adress) != 1)
             sprintf(adress, "%s\\%s", TMDirectory, buffer);

          if (test_exist(adress) == 1)
          {
              if (info->dep[0] != '\0')
                 sprintf(info->dep, "%s\n%s", info->dep, adress);
              else strcpy(info->dep, adress);
          }
    }

    strcat(info->dep, "\n");
    free(data);
    return 1;
}

int retrieve_info_profile(char adress[], Info_Profile *info)
{
    FILE *file = fopen(adress, "rb");
    if (file == NULL)
       return 0;

    int identifier;
    fseek(file, 9, SEEK_SET);
    fread(&identifier, sizeof(int), 1, file);

    if (identifier != 604721152)
    {
       fclose(file);
       return 0;
    }

    fseek(file, 20, SEEK_CUR);
    fread(info->name, 1, 100, file);
    info->name[99] = '\0';
    if (strchr(info->name, 16) != NULL)
       *strchr(info->name, 16) = '\0';
    convert_name(info->name, info->name);

    fseek(file, 0, SEEK_END);
    info->weight = ftell(file) / pow(2,10);

    fclose(file);
    return 1;
}

int retrieve_info_music(char adress[], Info_Music *info)
{
    char extension[MAX_CHAINE];

    if (strchr(adress, '.') != NULL)
       strcpy(extension, strrchr(adress, '.') + 1);

    int i;
    for (i = 0 ; extension[i] != '\0' ; i++)
        extension[i] = toupper(extension[i]);

    if (strcmp(extension, "WAV") != 0 &&
        strcmp(extension, "OGG") != 0 &&
        strcmp(extension, "MUX") != 0 &&
        strcmp(extension, "MP3") != 0 &&
        strcmp(extension, "WMA") != 0 &&
        strcmp(extension, "AAC") != 0) return 0;

    SHFILEINFO shfi;
    SHGetFileInfo(adress, 0, &shfi, sizeof(SHFILEINFO), SHGFI_TYPENAME);
    strcpy(info->type, shfi.szTypeName);

    if (strchr(adress, '\\') != NULL)
       strcpy(info->name, strrchr(adress, '\\') + 1);
    else strcpy(info->name, adress);
    if (strchr(info->name, '.') != NULL)
       *strrchr(info->name, '.') = '\0';

    HANDLE handle = CreateFile(adress, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle != INVALID_HANDLE_VALUE)
    {
       info->weight = GetFileSize(handle, NULL) / pow(2,10);
       CloseHandle(handle);
    }

    info->length.wYear = 1;
    if (useFmod && strcmp(extension, "MUX") != 0)
    {
       FSOUND_STREAM *stream = FSOUND_Stream_Open(adress, FSOUND_LOOP_OFF | FSOUND_MPEGACCURATE, 0, 0);
       if (stream != NULL)
          conversion_time(&(info->length), FSOUND_Stream_GetLengthMs(stream));

       FSOUND_Stream_Close(stream);
    }

    return 1;
}

int retrieve_info_replay(char adress[], Info_Replay *info)
{
    FILE *file = fopen(adress, "rb");
    if (file == NULL)
       return 0;

    int identifier;
    fseek(file, 9, SEEK_SET);
    fread(&identifier, sizeof(int), 1, file);
    if (identifier != 604495872 && identifier != 50933760)
    {
       fclose(file);
       return 0;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    info->weight = size / pow(2,10);
    rewind(file);

    void *data = malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    int success = 0, i;
    char *beginning = NULL;

    for (i = 0 ; i < size && !success ; i++)
    {
       if (*((char*)data + i) == '<' && (beginning = strstr(data + i, "<header type=\"")) != NULL)
          success = 1;
    }

    char string[50000] = "\0";
    CopyMemory(string, data, (int)beginning - (int)data);

    i = (int)beginning - (int)data - sizeof(int);
    char *end = string + i;
    for (i  = i - 1 ; string[i] >= '0' ; i--);
    char *begin = string + i + 1;

    CopyMemory(info->author, begin, end - begin);
    info->author[end - begin] = '\0';
    convert_name(info->author, info->author);

    success = 0;
    for (i = 0 ; i < size - (int)beginning + (int)data && !success ; i++)
    {
       if (*((char*)beginning + i) == '<' && (begin = strstr(beginning + i, "</header>")) != NULL)
          success = 1;
    }
    CopyMemory(string, beginning, (int)begin - (int)data);

    begin = strstr(string, "<times best=") + strlen("<times best=\"");
    conversion_time(&(info->time), strtol(begin, NULL, 10));

    strcpy(info->track, adress);
    char *position = NULL;
    if ((position = strrchr(info->track, '\\')) != NULL)
       strcpy(info->track, position + 1);
    if ((position = strstr(info->track, ".Gbx")) != NULL)
       *position = '\0';
    if ((position = strstr(info->track, ".Replay")) != NULL)
       *position = '\0';
    convert_name(info->track, info->track);

    return 1;
}



int retrieve_info_image(char adress[], Info_Image *info)
{
    char extension[MAX_CHAINE];

    if (strchr(adress, '.') != NULL)
       strcpy(extension, strrchr(adress, '.') + 1);

    int i;
    for (i = 0 ; extension[i] != '\0' ; i++)
        extension[i] = toupper(extension[i]);

    if (strcmp(extension, "BMP") != 0 &&
        strcmp(extension, "JPG") != 0 &&
        strcmp(extension, "JPEG") != 0 &&
        strcmp(extension, "PNG") != 0 &&
        strcmp(extension, "TIF") != 0 &&
        strcmp(extension, "TIFF") != 0 &&
        strcmp(extension, "TGA") != 0 &&
        strcmp(extension, "ICO") != 0 &&
        strcmp(extension, "GIF") != 0 &&
        strcmp(extension, "DDS") != 0) return 0;

    char temp[MAX_CHAINE];
    sprintf(temp, "%s\\temp.bmp", dossierBase);
    info->img = open_image(adress, IMG_UNKNOW, temp, NULL);

    if (info->img == NULL)
       return 0;

    BITMAP bmp;
    GetObject(info->img, sizeof(bmp), &bmp);
    info->w = bmp.bmWidth;
    info->h = bmp.bmHeight;
    strcpy(info->name, adress);
    if (strchr(info->name, '\\') != NULL)
       strcpy(info->name, strrchr(info->name, '\\') + 1);
    if (strchr(info->name, '.') != NULL)
       *strrchr(info->name, '.') = '\0';

    SHFILEINFO shfi;
    SHGetFileInfo(adress, 0, &shfi, sizeof(SHFILEINFO), SHGFI_TYPENAME);
    strcpy(info->type, shfi.szTypeName);

    HANDLE handle = CreateFile(adress, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle != INVALID_HANDLE_VALUE)
    {
       info->weight = GetFileSize(handle, NULL) / pow(2,10);
       CloseHandle(handle);
    }

    RECT rect;
    rect.right = info->w;
    rect.bottom = info->h;

    float factor = 1;
    if (rect.bottom > 100)
       factor = 100.0 / rect.bottom;
    rect.bottom *= factor;
    rect.right *= factor;

    factor = 1;
    if (rect.right > tailleRectangle.right - tailleRectangle.left - 20)
       factor = (tailleRectangle.right - tailleRectangle.left - 20) * 1.0 / rect.right;
    rect.bottom *= factor;
    rect.right *= factor;

    DeleteObject(info->img);
    info->img = open_image(adress, IMG_UNKNOW, temp, &rect);

    return 1;
}

int retrieve_info_locator(char adress[], Info_Locator *info)
{
    char extension[MAX_CHAINE];
    if (strchr(adress, '.') != NULL)
       strcpy(extension, strrchr(adress, '.') + 1);

    int i;
    for (i = 0 ; extension[i] != '\0' ; i++)
        extension[i] = toupper(extension[i]);

    if (strcmp(extension, "LOC") != 0)
       return FALSE;

    strcpy(info->name, adress);
    if (strchr(info->name, '\\') != NULL)
       strcpy(info->name, strrchr(info->name, '\\') + 1);
    if (strchr(info->name, '.') != NULL)
       *strrchr(info->name, '.') = '\0';

    FILE *file = fopen(adress, "r");
    if (file == NULL)
    {
             strcpy(info->url, "Impossible d'ouvrir le fichier");
             return 0;
    }

    fgets(info->url, MAX_CHAINE, file);
    if (strchr(info->url, '\n') != NULL)
       *strchr(info->url, '\n') = '\0';

    fclose(file);

    char url[MAX_CHAINE];
    if (strchr(info->url, '/') != NULL)
       strcpy(url, strrchr(info->url, '/') + 1);

    if (strchr(url, '.') != NULL)
    {
       strcpy(extension, strrchr(url, '.') + 1);
       char buffer[MAX_CHAINE];
       sprintf(buffer, "%s\\bingo.%s", dossierBase, extension);
       if ((file = fopen(buffer, "w")) != NULL)
          fclose(file);

       SHFILEINFO shfi;
       SHGetFileInfo(buffer, 0, &shfi, sizeof(SHFILEINFO), SHGFI_TYPENAME);
       strcpy(info->targetType, shfi.szTypeName);
    }
    else strcpy(info->targetType, "Type indéfini");


    return 1;
}

int retrieve_info_unknow(char adress[], Info_Unknow *info)
{
    strcpy(info->name, adress);
    if (strchr(info->name, '\\') != NULL)
       strcpy(info->name, strrchr(info->name, '\\') + 1);
    if (strchr(info->name, '.') != NULL)
       *strrchr(info->name, '.') = '\0';

    HANDLE handle = CreateFile(adress, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
             strcpy(info->type, "Impossible d'ouvrir le fichier");
             info->weight = 0;
             return 0;
    }

    info->weight = GetFileSize(handle, NULL) / pow(2,10);
    CloseHandle(handle);

    SHFILEINFO shfi;
    SHGetFileInfo(adress, 0, &shfi, sizeof(SHFILEINFO), SHGFI_TYPENAME);
    strcpy(info->type, shfi.szTypeName);

    if (info->type[0] == '\0')
       strcpy(info->type, "Inconnu");

    return 1;
}



void conversion_time(SYSTEMTIME *st, int time)
{
    if (st == NULL)
       return;

    st->wYear = time / (3600000 * 24 * 365.25);
    time -= st->wYear * (3600000 * 24 * 365.25);
    st->wMonth = time / (3600000 * 24 * (365.25 / 12));
    time -= st->wMonth * (3600000 * 24 * (365.25 / 12));
    st->wDay = time / (3600000 * 24);
    time -= st->wDay * (3600000 * 24);
    st->wHour = time / 3600000;
    time -= st->wHour * 3600000;
    st->wMinute = time / 60000;
    time -= st->wMinute * 60000;
    st->wSecond = time / 1000;
    time -= st->wSecond * 1000;
    st->wMilliseconds = time;

    return;
}


void convert_name(char name[], char converted[])
{
     if (name != converted)
        strcpy(converted, name);

     int i, j;
     for (i = 0 ; converted[i] != '\0' ; i++)
     {
         if (converted[i] == '$')
         {
             if (converted[i + 1] == '$')
                strcpy(converted + i, converted + i + 1);
             else if (converted[i + 1] == '\0')
                  converted[i] = '\0';
             else if ((converted[i + 1] >= '0' && converted[i + 1] <= '9') || (toupper(converted[i + 1]) >= 'A' && toupper(converted[i + 1]) <= 'F'))
             {
                  for (j = i + 1 ; converted[j] != '\0' && j < i + 4 ; j++);
                  strcpy(converted + i, converted + j);
             }
             else strcpy(converted + i, converted + i + 2);
         }
     }

     convert_string_to_ansi(converted, converted);
     return;
}


unsigned char convert_utf8_to_ansi(unsigned char in[])
{
    if (in[0] <= 127)
       return in[0];

    if (in[0] < 192 || in[0] > 223)
       return '?';

    unsigned char c1 = in[0];
    unsigned char c2 = in[1];

    if (c2 < 128 || c2 > 191)
       return '?';

    c1 -= 192;
    c2 -= 128;

    switch (c1)
    {
           case 0:
                return c2;
           case 1:
                return c2 + 64;
           case 2:
                return c2 + 128;
           case 3:
                return c2 + 192;
           default:
                   return '#';
    }
}


int convert_string_to_ansi(char in[], char out[])
{
    if (in != out)
       strcpy(out, in);

    if (out[0] == '\0')
       return 0;

    int i;
    char converted;

    for (i = 0 ; out[i + 1] != '\0' ; i++)
    {
        converted = convert_utf8_to_ansi((unsigned char*)out + i);
        if (converted != out[i])
        {
           out[i] = converted;
           strcpy(out + i + 1, out + i + 2);
        }
    }

    return 1;
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


int retrieve_multi_select(OPENFILENAME ofn, char *tab[], int tailleTab, int autoAlloc)
{
    //autoAlloc : 1 = allouer automatiquement la mémoire nécessaire ; 2 = également, désallouer si nécessaire ; 0 = rien

    if (ofn.nMaxFile <= 0)
       ofn.nMaxFile = MAX_PATH;

    char *dir = malloc(ofn.nMaxFile);
    strcpy(dir, ofn.lpstrFile);
    char *position = ofn.lpstrFile + ofn.nFileOffset;
    *(dir + ofn.nFileOffset) = '\0';

    if (dir[strlen(dir) - 1] == '\\')
       dir[strlen(dir) - 1] = '\0';
    if (*position == '\\')
       position += 1;

    char *nom = malloc(ofn.nMaxFile);
    int i;
    for (i = 0 ; i < tailleTab && i < ofn.nMaxFile && position != NULL && *position != '\0' ; i++)
    {
        sprintf(nom, "%s\\%s", dir, position);
        if (autoAlloc == 2 && tab[i] != NULL)
           free(tab[i]);
        if (autoAlloc)
           tab[i] = malloc(ofn.nMaxFile);
        strcpy(tab[i], nom);

        position = strchr(position, '\0');
        if (position != NULL)
           position += 1;
    }

    if (i < tailleTab)
    {
       if (autoAlloc == 2 && tab[i] != NULL)
       {
           free(tab[i]);
           tab[i] = NULL;
       }
       else
       {
           if (autoAlloc)
              tab[i] = malloc(ofn.nMaxFile);
           tab[i][0] = '\0';
       }
    }

    free(nom);
    free(dir);
    return i;
}


int create_toolTips()
{
    if (tableauToolTips[0] != NULL)
       DestroyWindow(tableauToolTips[0]);

    TOOLINFO ti;
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = dragAndDropWnd;
    ti.uId = (int)listViewWnd;
    ti.hinst = mainInstance;
    ti.lpszText = "Déplacez des fichiers ici";
    ti.lParam = 0;

    HWND hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   NULL,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   ti.hwnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[0] = hwndTip;

    if (tableauToolTips[1] != NULL)
       DestroyWindow(tableauToolTips[1]);

    ti.hwnd = mainWnd;
    ti.uId = (int)GetDlgItem(mainWnd, IDP_DEFIL);
    if (defilActive)
       ti.lpszText = "Arrêter le défilement";
    else ti.lpszText = "Reprendre le défilement";

    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   NULL,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   mainWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[1] = hwndTip;

    if (tableauToolTips[2] != NULL)
       DestroyWindow(tableauToolTips[2]);

    ti.hwnd = listWnd;
    ti.uId = (int)GetDlgItem(ti.hwnd, IDE_LIST);
    ti.lpszText = "Entrez un texte à rechercher";

    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   NULL,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   ti.hwnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[2] = hwndTip;

    return 1;
}


int add_item(HWND hwnd, char adress[])
{
    char buffer[MAX_CHAINE] = "\0",
         *position = NULL;

    if ((GetFileAttributes(adress) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
    {
       sprintf(buffer, "%s\nImpossible d'ajouter un répertoire !", adress);
       MessageBox(mainWnd, buffer, "Attention", MB_OK | MB_ICONWARNING);
       return 0;
    }

    SHFILEINFO shfi;
    sysImgList = (HIMAGELIST) SHGetFileInfo(adress, 0, &shfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
    ListView_SetImageList(hwnd, sysImgList, LVSIL_SMALL);

    strcpy(buffer, adress);
    if ((position = strrchr(buffer, '\\')) != NULL)
       strcpy(buffer, position + 1);


    LVITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iItem = ListView_GetItemCount(hwnd) + 1;
    lvi.iSubItem = 0;
    lvi.pszText = buffer;
    lvi.iImage = shfi.iIcon;

    return ListView_InsertItem(hwnd, &lvi);
}

int get_selected_items(HWND hwnd, int **ptOut)
{
    int count = ListView_GetItemCount(hwnd),
        selected = ListView_GetSelectedCount(hwnd);

    *ptOut = (int*) calloc(selected, sizeof(int));

    int i, j = 0;
    for (i = count - 1 ; i >= 0 ; i--)
    {
        if (is_item_selected(hwnd, i))
        {
           (*ptOut)[j] = i;
           j++;
        }
    }

    return selected;
}

int is_item_selected(HWND hwnd, int item)
{
    int state = LOBYTE(ListView_GetItemState(hwnd, item, LVIS_SELECTED));
    if ((state & LVIS_SELECTED) == LVIS_SELECTED)
       return 1;
    else return 0;
}


int get_first_selected(HWND hwnd)
{
    if (ListView_GetSelectedCount(hwnd) <= 0)
       return -1;
    else return ListView_GetNextItem(hwnd, -1, LVNI_SELECTED);
}


int select_items(HWND hwnd, int *tab, int array, int erase)
{
    SetFocus(hwnd);
    if (erase)
        ListView_SetItemState(hwnd, -1, 0, LVIS_SELECTED);

    int i;
    for (i = 0 ; i < array ; i++)
    {
        if (tab[i] != -1)
           ListView_SetItemState(hwnd, tab[i], LVIS_SELECTED, LVIS_SELECTED);
    }

    return 1;
}


int create_icon(int destroy)
{
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = mainWnd;
    nid.uID = ICON_ID;
    nid.uCallbackMessage = WM_COMMAND;
    nid.hIcon = mainIcon;
    strcpy(nid.szTip, "TM Creator");

    if (!destroy)
    {
        nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        Shell_NotifyIcon(NIM_ADD, &nid);
    }
    else
    {
        nid.uFlags = 0;
        Shell_NotifyIcon(NIM_DELETE, &nid);
    }

    return 1;
}


int print_icon_menu()
{
      HMENU hMenu;
      POINT pt;
      GetCursorPos(&pt);

      hMenu = CreatePopupMenu();
      AppendMenu(hMenu, MF_STRING, IM_OPEN, "Ouvrir TM Creator");

      SetMenuDefaultItem(hMenu, IM_OPEN, FALSE);

      SetForegroundWindow(mainWnd);
      int result = TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, mainWnd, NULL);
      PostMessage(mainWnd, WM_NULL, 0, 0);

      if (result)
         return 1;
      else return 0;
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



