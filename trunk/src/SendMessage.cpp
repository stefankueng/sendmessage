// SendMessage - a tool to send custom messages

// Copyright (C) 2010 - Stefan Kueng

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
#include "stdafx.h"
#include "Resource.h"
#include "SendMessage.h"
#include "MainDlg.h"
#include "AboutDlg.h"
#include "CmdLineParser.h"

#pragma comment(lib, "comctl32.lib")

// Global Variables:
HINSTANCE hInst;								// current instance

// Forward declarations of functions included in this code module:

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	::OleInitialize(NULL);
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	// we need some of the common controls
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LINK_CLASS|ICC_LISTVIEW_CLASSES|ICC_PAGESCROLLER_CLASS
		|ICC_PROGRESS_CLASS|ICC_STANDARD_CLASSES|ICC_TAB_CLASSES|ICC_TREEVIEW_CLASSES
		|ICC_UPDOWN_CLASS|ICC_USEREX_CLASSES|ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);

	CCmdLineParser parser(lpCmdLine);

	int ret = 0;
	if (parser.HasKey(_T("about"))||parser.HasKey(_T("?"))||parser.HasKey(_T("help")))
	{
		CAboutDlg aboutDlg(NULL);
		ret= aboutDlg.DoModal(hInstance, IDD_ABOUTBOX, NULL, NULL);
	}
	else
	{
		CMainDlg mainDlg(NULL);
		ret = mainDlg.DoModal(hInstance, IDD_MAINDLG, NULL, IDR_MAINDLG);
	}

	::CoUninitialize();
	::OleUninitialize();
	return ret;
}



