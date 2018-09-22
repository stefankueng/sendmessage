// SendMessage - a tool to send custom messages

// Copyright (C) 2018 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#pragma once

// this file is used if the NAnt script didn't run and
// didn't create the real version.h file.
// only needed if users don't need correct version info
// or for the continuous integration servers where NAnt and TGit isn't available

#define FILEVER         1, 0, 0, 0
#define PRODUCTVER      1, 0, 0, 0
#define STRFILEVER      "1.0.0.0\0"
#define STRPRODUCTVER   "1.0.0.0\0"

#define SM_VERMAJOR     1
#define SM_VERMINOR     0
#define SM_VERMICRO     0
#define SM_VERBUILD     0
#define SM_VERDATE      "2000/01/01 00:00:00"
