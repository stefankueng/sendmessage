// SendMessage - a tool to send custom messages

// Copyright (C) 2010, 2012-2013, 2018, 2021 - Stefan Kueng

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
#include "SendMessage.h"
#include "MainDlg.h"
#include "AboutDlg.h"
#include "WinMessage.h"
#include "CmdLineParser.h"
#include "SmartHandle.h"
#include "AccessibleName.h"
#include <set>
#include <Psapi.h>

// Global Variables:
HINSTANCE     hInst; // current instance
std::wstring  windowTitle;
std::wstring  windowClass;
std::wstring  windowAccessiblename;
std::wstring  windowProcessname;

BOOL CALLBACK ChildWindowEnumerator(HWND hwnd, LPARAM lParam)
{
    auto* pSet = reinterpret_cast<std::set<HWND>*>(lParam);

    if (!windowTitle.empty())
    {
        TCHAR buf[4096]{};
        GetWindowText(hwnd, buf, _countof(buf));
        if (windowTitle.substr(0, 4096).compare(buf) != 0)
            return TRUE;
    }
    if (!windowClass.empty())
    {
        TCHAR szClassName[100]{};
        GetClassName(hwnd, szClassName, _countof(szClassName));
        if (windowClass.compare(szClassName) != 0)
            return TRUE;
    }
    if (!windowAccessiblename.empty())
    {
        if (windowAccessiblename.compare(CAccessibleName(hwnd).c_str()) != 0)
            return TRUE;
    }
    if (!windowProcessname.empty())
    {
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid)
        {
            CAutoGeneralHandle hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
            if (hProc)
            {
                TCHAR  szProcName[MAX_PATH] = {0};
                TCHAR* pProcName            = szProcName;
                if (GetProcessImageFileName(hProc, szProcName, _countof(szProcName)))
                {
                    pProcName = _tcsrchr(szProcName, '\\');
                    if (pProcName)
                        pProcName++;
                    else
                        pProcName = szProcName;
                    if (windowProcessname.compare(pProcName) != 0)
                        return TRUE;
                }
            }
        }
    }

    pSet->insert(hwnd);

    return TRUE;
}

BOOL CALLBACK WindowEnumerator(HWND hwnd, LPARAM lParam)
{
    auto* pSet     = reinterpret_cast<std::set<HWND>*>(lParam);

    bool  bMatches = true;
    if (!windowTitle.empty())
    {
        TCHAR buf[4096];
        GetWindowText(hwnd, buf, _countof(buf));
        if (windowTitle.substr(0, 4096).compare(buf) != 0)
            bMatches = false;
    }
    if (!windowClass.empty())
    {
        TCHAR szClassName[100] = {0};
        GetClassName(hwnd, szClassName, _countof(szClassName));
        if (windowClass.compare(szClassName) != 0)
            bMatches = false;
    }
    if (!windowAccessiblename.empty())
    {
        if (windowAccessiblename.compare(CAccessibleName(hwnd).c_str()) != 0)
            bMatches = false;
    }
    if (!windowProcessname.empty())
    {
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid)
        {
            CAutoGeneralHandle hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
            if (hProc)
            {
                TCHAR  szProcName[MAX_PATH] = {0};
                TCHAR* pProcName            = szProcName;
                if (GetProcessImageFileName(hProc, szProcName, _countof(szProcName)))
                {
                    pProcName = _tcsrchr(szProcName, '\\');
                    if (pProcName)
                        pProcName++;
                    else
                        pProcName = szProcName;
                    if (windowProcessname.compare(pProcName) != 0)
                        bMatches = false;
                }
                else
                    bMatches = false;
            }
            else
                bMatches = false;
        }
        else
            bMatches = false;
    }
    if (bMatches)
        pSet->insert(hwnd);

    if (windowProcessname.empty() || !windowTitle.empty() || !windowClass.empty() || !windowAccessiblename.empty())
        EnumChildWindows(hwnd, ChildWindowEnumerator, lParam);

    return TRUE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    ::OleInitialize(nullptr);
    ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    // we need some of the common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_LINK_CLASS | ICC_LISTVIEW_CLASSES | ICC_PAGESCROLLER_CLASS | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES | ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES | ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    CCmdLineParser parser(lpCmdLine);

    INT_PTR        ret = 0;
    if (parser.HasKey(_T("about")) || parser.HasKey(_T("?")) || parser.HasKey(_T("help")))
    {
        CAboutDlg aboutDlg(nullptr);
        ret = aboutDlg.DoModal(hInstance, IDD_ABOUTBOX, nullptr, NULL);
    }
    else if (parser.HasKey(L"message"))
    {
        UINT   msg    = 0;
        WPARAM wParam = 0;
        LPARAM lParam = 0;

        if (parser.HasVal(L"message"))
        {
            msg = WinMessages::Instance().ParseMsg(parser.GetVal(L"message"));
        }
        if (parser.HasVal(L"wparam"))
            wParam = parser.GetLongVal(L"wparam");
        if (parser.HasVal(L"lparam"))
            lParam = (LPARAM)parser.GetLongLongVal(L"lparam");

        bool           bSend = !parser.HasKey(L"post");

        std::set<HWND> hwndset;

        if (parser.HasVal(L"windowhandle"))
            hwndset.insert(reinterpret_cast<HWND>(parser.GetLongLongVal(L"windowhandle")));
        else
        {
            if (parser.HasVal(L"windowtitle"))
                windowTitle = parser.GetVal(L"windowtitle");
            if (parser.HasVal(L"windowclass"))
                windowClass = parser.GetVal(L"windowclass");
            if (parser.HasVal(L"accessiblename"))
                windowAccessiblename = parser.GetVal(L"accessiblename");
            if (parser.HasVal(L"processname"))
                windowProcessname = parser.GetVal(L"processname");

            EnumWindows(WindowEnumerator, reinterpret_cast<LPARAM>(&hwndset));
        }

        for (auto it = hwndset.cbegin(); it != hwndset.cend(); ++it)
        {
            if (bSend)
                ret = static_cast<int>(SendMessage(*it, msg, wParam, lParam));
            else
                ret = PostMessage(*it, msg, wParam, lParam);
        }
    }
    else
    {
        CMainDlg mainDlg(nullptr);
        ret = mainDlg.DoModal(hInstance, IDD_MAINDLG, nullptr, IDR_MAINDLG);
    }

    ::CoUninitialize();
    ::OleUninitialize();
    return static_cast<int>(ret);
}
