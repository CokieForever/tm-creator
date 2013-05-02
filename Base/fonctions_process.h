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

#ifndef FONCTIONS_PROCESS

#define FONCTIONS_PROCESS

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <psapi.h>
#include <string.h>

#ifndef TAILLE_MAX_TAB
#define TAILLE_MAX_TAB 500
#endif

#ifndef TAILLE_MAX_CHAINE
#define TAILLE_MAX_CHAINE 5000
#endif

int test_exist_process(const char* monProcess);

#endif
