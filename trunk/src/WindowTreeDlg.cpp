// SendMessage - a tool to send custom messages

// Copyright (C) 2010, 2012 - Stefan Kueng

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
#include "resource.h"
#include "WindowTreeDlg.h"
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")

CWindowTreeDlg::CWindowTreeDlg(HWND hParent, HWND actualHandle)
    : m_hParent(hParent)
    , m_SelectedWindow(actualHandle)
{
}

CWindowTreeDlg::~CWindowTreeDlg(void)
{
}

LRESULT CWindowTreeDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, IDI_SENDMESSAGE);

            m_resizer.Init(hwndDlg);
            m_resizer.AddControl(hwndDlg, IDC_WINDOWTREE, RESIZER_TOPLEFTBOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_REFRESH, RESIZER_BOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDOK, RESIZER_BOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDCANCEL, RESIZER_BOTTOMRIGHT);

            ExtendFrameIntoClientArea(IDC_WINDOWTREE, IDC_WINDOWTREE, IDC_WINDOWTREE, IDC_WINDOWTREE);
            m_aerocontrols.SubclassControl(GetDlgItem(*this, IDC_REFRESH));
            m_aerocontrols.SubclassControl(GetDlgItem(*this, IDOK));
            m_aerocontrols.SubclassControl(GetDlgItem(*this, IDCANCEL));
            if (m_Dwm.IsDwmCompositionEnabled())
                m_resizer.ShowSizeGrip(false);

            RefreshTree();
        }
        return FALSE;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam), HIWORD(wParam));
    case WM_SIZE:
        {
            m_resizer.DoResize(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_GETMINMAXINFO:
        {
            MINMAXINFO * mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = m_resizer.GetDlgRect()->right;
            mmi->ptMinTrackSize.y = m_resizer.GetDlgRect()->bottom;
            return 0;
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

LRESULT CWindowTreeDlg::DoCommand(int id, int /*msg*/)
{
    switch (id)
    {
    case IDOK:
        m_SelectedWindow = GetSelectedWindowHandle();
    case IDCANCEL:
        EndDialog(*this, id);
        break;
    case IDC_REFRESH:
        RefreshTree();
        break;
    }
    return 1;
}

bool CWindowTreeDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
    }
    return false;
}

bool CWindowTreeDlg::RefreshTree()
{
    TreeView_DeleteAllItems(GetDlgItem(*this, IDC_WINDOWTREE));
    m_lastTreeItem = TVI_ROOT;
    EnumWindows(WindowEnumerator, (LPARAM)this);
    SelectTreeItem(m_SelectedWindow);
    return true;
}

BOOL CWindowTreeDlg::WindowEnumerator(HWND hwnd, LPARAM lParam)
{
    CWindowTreeDlg * pThis = (CWindowTreeDlg*)lParam;

    TCHAR buf[4096];
    GetWindowString(hwnd, buf, 4096);

    TVINSERTSTRUCT is = {0};
    is.hParent = TVI_ROOT;
    is.hInsertAfter = TVI_SORT;
    is.itemex.mask = TVIF_TEXT | TVIF_PARAM;
    is.itemex.pszText = buf;
    is.itemex.lParam = (LPARAM)hwnd;

    pThis->m_lastTreeItem = TreeView_InsertItem(GetDlgItem(*pThis, IDC_WINDOWTREE), &is);
    EnumChildWindows(hwnd, ChildWindowEnumerator, lParam);
    return TRUE;
}

BOOL CWindowTreeDlg::ChildWindowEnumerator(HWND hwnd, LPARAM lParam)
{
    CWindowTreeDlg * pThis = (CWindowTreeDlg*)lParam;

    TCHAR buf[4096];
    GetWindowString(hwnd, buf, 4096);

    TVINSERTSTRUCT is = {0};
    is.hParent = pThis->m_lastTreeItem;
    is.hInsertAfter = TVI_SORT;
    is.itemex.mask = TVIF_TEXT | TVIF_PARAM;
    is.itemex.pszText = buf;
    is.itemex.lParam = (LPARAM)hwnd;

    TreeView_InsertItem(GetDlgItem(*pThis, IDC_WINDOWTREE), &is);
    return TRUE;
}

void CWindowTreeDlg::GetWindowString(HWND hwnd, TCHAR * buf, int bufsize)
{
    TCHAR tmpbuf[4096] = {0};
    TCHAR szClassName[100] = {0};
    TCHAR szProcName[MAX_PATH] = {0};
    TCHAR * pProcName = szProcName;

    GetWindowText(hwnd, tmpbuf, 4096);
    if (GetClassName(hwnd, szClassName, _countof(szClassName))==0)
        szClassName[0] = 0;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid)
    {
        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (hProc)
        {
            if (GetProcessImageFileName(hProc, szProcName, MAX_PATH))
            {
                pProcName = _tcsrchr(szProcName, '\\');
                if (pProcName)
                    pProcName++;
                else
                    pProcName = szProcName;
            }
            CloseHandle(hProc);
        }
    }
    _sntprintf_s(buf, bufsize, _TRUNCATE, _T("Window 0x%08X : Process \"%s\" : class \"%s\" : title \"%s\""), hwnd, pProcName, szClassName, tmpbuf);
}

HWND CWindowTreeDlg::GetSelectedWindowHandle()
{
    HTREEITEM selItem = TreeView_GetSelection(GetDlgItem(*this, IDC_WINDOWTREE));
    if (selItem == NULL)
        return NULL;

    TVITEM tvi = {0};
    tvi.hItem = selItem;
    tvi.mask = TVIF_PARAM;
    TreeView_GetItem(GetDlgItem(*this, IDC_WINDOWTREE), &tvi);
    return (HWND)tvi.lParam;
}

void CWindowTreeDlg::SelectTreeItem(HWND windowHwnd)
{
    HWND _windowTreeHwnd = GetDlgItem(*this, IDC_WINDOWTREE);
    HTREEITEM actualItem = TreeView_GetFirstVisible(_windowTreeHwnd);

    // Search by the item into the list
    while (actualItem != NULL)
    {
        TVITEM tvi = {0};
        tvi.hItem = actualItem;
        tvi.mask = TVIF_PARAM;
        TreeView_GetItem(_windowTreeHwnd, &tvi);

        // If it is the item, select it and break the search
        if ((HWND)tvi.lParam == windowHwnd)
        {
            TreeView_SelectItem(_windowTreeHwnd, actualItem);
            break;
        }

        // Find the next item in the TreeView
        HTREEITEM nextItem = TreeView_GetChild(_windowTreeHwnd, actualItem);
        if (nextItem == NULL)
            nextItem = TreeView_GetNextSibling(_windowTreeHwnd, actualItem);

        HTREEITEM parentItem = actualItem;
        while ((nextItem == NULL) && (parentItem != NULL))
        {
            parentItem = TreeView_GetParent(_windowTreeHwnd, parentItem);
            nextItem = TreeView_GetNextSibling(_windowTreeHwnd, parentItem);
        }
        actualItem = nextItem;
    }
}
