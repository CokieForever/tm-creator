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

#include "fonctions_repertoire.h"

int lister_tout(char dir[], char filePartName[], int includeDirs, int includeSubDirs, char fileOutName[], int clearBeforeWrite, WindowInfo *wi)
{
    char buffer[MAX_CHAINE],
         directory[MAX_CHAINE],
         *position = NULL,
         text[MAX_CHAINE];

    strcpy(directory, dir);
    if (directory[strlen(directory) - 1] == '\\')
       directory[strlen(directory) - 1] = '\0';

    sprintf(buffer, "%s\\*", directory);

    WIN32_FIND_DATA wfd;
    HANDLE hSearch[100] = {NULL};
    hSearch[0] = FindFirstFile(buffer, &wfd);

    FILE *file = NULL;
    if (clearBeforeWrite)
       file = fopen(fileOutName, "w");
    else file = fopen(fileOutName, "a");

    if (file == NULL)
       return 0;


    int i = 0, fileFound, countFound = 0, countAdded = 0,
        lastUpdate = GetTickCount(), quit = 0, cursorChanged = 0;
    HCURSOR previousCursor;

    while (i >= 0 && hSearch[0] != INVALID_HANDLE_VALUE)
    {
          sprintf(buffer, "%s\\%s", directory, wfd.cFileName);
          countFound++;

          sprintf(text, "Fichier : %s", buffer);
          if (wi != NULL && wi->wndFileName != NULL)
             SetDlgItemText(wi->wndFileName, wi->itemFileName, text);

          if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
          {
             if (filePartName == NULL || strstr(wfd.cFileName, filePartName) != NULL)
             {
                countAdded++;
                fprintf(file, "%s\\%s\n", directory, wfd.cFileName);
             }

             fileFound = FindNextFile(hSearch[i], &wfd);
          }
          else if (strcmp(wfd.cFileName, ".") != 0 && strcmp(wfd.cFileName, "..") != 0)
          {
              if (includeDirs && (filePartName == NULL || strstr(wfd.cFileName, filePartName) != NULL))
              {
                 countAdded++;
                 fprintf(file, "%s\\%s\n", directory, wfd.cFileName);
              }

              if (includeSubDirs && i < 100)
              {
                  i++;
                  sprintf(buffer, "%s\\%s", directory, wfd.cFileName);
                  strcpy(directory, buffer);
                  strcat(buffer, "\\*");
                  hSearch[i] = FindFirstFile(buffer, &wfd);
                  if (hSearch[i] == INVALID_HANDLE_VALUE)
                     fileFound = 0;
              }
              else fileFound = FindNextFile(hSearch[i], &wfd);
          }
          else fileFound = FindNextFile(hSearch[i], &wfd);

          sprintf(text, "Trouvés : %d", countFound);
          if (wi != NULL && wi->wndNumberFilesFound != NULL)
             SetDlgItemText(wi->wndNumberFilesFound, wi->itemNumberFilesFound, text);

          sprintf(text, "Ajoutés : %d", countAdded);
          if (wi != NULL && wi->wndNumberFilesAdded != NULL)
             SetDlgItemText(wi->wndNumberFilesAdded, wi->itemNumberFilesAdded, text);

          if (wi != NULL && wi->cancelButton != NULL)
          {
             RECT rect;
             GetWindowRect(wi->cancelButton, &rect);
             POINT pt;
             GetCursorPos(&pt);

             if (GetForegroundWindow() == GetParent(wi->cancelButton) && pt.x >= rect.left && pt.x <= rect.right && pt.y <= rect.bottom && pt.y >= rect.top)
             {
                 if (HIBYTE(GetAsyncKeyState(VK_LBUTTON)))
                    quit = 1;
                 if (!cursorChanged)
                 {
                    previousCursor = SetCursor(LoadCursor(NULL, IDC_HAND));
                    cursorChanged = 1;
                 }
             }
             else if (cursorChanged)
             {
                  SetCursor(previousCursor);
                  cursorChanged = 0;
             }
          }

          if (wi != NULL && GetTickCount() - lastUpdate > 1000)
          {
               RECT rect;
               if (wi->wndFileName != NULL)
               {
                   GetClientRect(wi->wndFileName, &rect);
                   InvalidateRect(wi->wndFileName, &rect, FALSE);
                   UpdateWindow(wi->wndFileName);
               }

               if (wi->wndNumberFilesAdded != NULL)
               {
                   GetClientRect(wi->wndNumberFilesAdded, &rect);
                   InvalidateRect(wi->wndNumberFilesAdded, &rect, FALSE);
                   UpdateWindow(wi->wndNumberFilesAdded);
               }

               if (wi->wndNumberFilesFound != NULL)
               {
                   GetClientRect(wi->wndNumberFilesFound, &rect);
                   InvalidateRect(wi->wndNumberFilesFound, &rect, FALSE);
                   UpdateWindow(wi->wndNumberFilesFound);
               }
          }


          while ((!fileFound || quit) && i >= 0)
          {
             FindClose(hSearch[i]);
             i--;

             if ((position = strrchr(directory, '\\')) != NULL)
                *position = '\0';

             if (i >= 0)
                fileFound = FindNextFile(hSearch[i], &wfd);
          }
    }

    FindClose(hSearch[0]);
    fclose(file);
    return countAdded;
}



int test_exist(char nomFichier[])
{
    int i;

    FILE *fichier = NULL;
    for (i = 0 ; (fichier = fopen(nomFichier, "r")) == NULL && i <= 10 ; i++);
    if (fichier != NULL)
    {
       while(fclose(fichier) == EOF);
       return 1;
    }

    DIR *repertoire = NULL;
    for (i = 0 ; (repertoire = opendir(nomFichier)) == NULL && i <= 10 ; i++);
    if (repertoire != NULL)
    {
       closedir(repertoire);
       return 2;
    }

    return 0;
}
