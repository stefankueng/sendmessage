// SendMessage - a tool to send custom messages

// Copyright (C) 2010, 2012, 2014-2015, 2021 - Stefan Kueng

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
#include <shlwapi.h>

CWindowTreeDlg::CWindowTreeDlg(HWND hParent, HWND actualHandle)
    : m_hParent(hParent)
    , m_lastTreeItem(nullptr)
    , m_selectedWindow(actualHandle)
{
}

CWindowTreeDlg::~CWindowTreeDlg()
{
}

LRESULT CWindowTreeDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
        case WM_INITDIALOG: {
            InitDialog(hwndDlg, IDI_SENDMESSAGE);

            m_resizer.Init(hwndDlg);
            m_resizer.AddControl(hwndDlg, IDC_FILTER, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_WINDOWTREE, RESIZER_TOPLEFTBOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_REFRESH, RESIZER_BOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDOK, RESIZER_BOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDCANCEL, RESIZER_BOTTOMRIGHT);

            ExtendFrameIntoClientArea(IDC_WINDOWTREE, IDC_WINDOWTREE, IDC_WINDOWTREE, IDC_WINDOWTREE);
            m_aeroControls.SubclassControl(GetDlgItem(*this, IDC_REFRESH));
            m_aeroControls.SubclassControl(GetDlgItem(*this, IDOK));
            m_aeroControls.SubclassControl(GetDlgItem(*this, IDCANCEL));
            if (m_dwm.IsDwmCompositionEnabled())
                m_resizer.UseSizeGrip(false);

            RefreshTree();
        }
            return FALSE;
        case WM_DESTROY:
            KillTimer(*this, 0);
            break;
        case WM_TIMER:
            KillTimer(*this, 0);
            RefreshTree();
            return TRUE;
        case WM_COMMAND:
            return DoCommand(LOWORD(wParam), HIWORD(wParam));
        case WM_SIZE: {
            m_resizer.DoResize(LOWORD(lParam), HIWORD(lParam));
        }
        break;
        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi       = reinterpret_cast<MINMAXINFO*>(lParam);
            mmi->ptMinTrackSize.x = m_resizer.GetDlgRect()->right;
            mmi->ptMinTrackSize.y = m_resizer.GetDlgRect()->bottom;
            return 0;
        }
        default:
            return FALSE;
    }
    return FALSE;
}

LRESULT CWindowTreeDlg::DoCommand(int id, int msg)
{
    switch (id)
    {
        case IDOK:
            m_selectedWindow = GetSelectedWindowHandle();
            [[fallthrough]];
        case IDCANCEL:
            EndDialog(*this, id);
            break;
        case IDC_FILTER:
            switch (msg)
            {
                case EN_CHANGE:
                    SetTimer(*this, 0, 500, nullptr);
            }
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
    auto hTree = GetDlgItem(*this, IDC_WINDOWTREE);
    SendMessage(hTree, WM_SETREDRAW, FALSE, 0);
    TreeView_DeleteAllItems(hTree);
    m_lastTreeItem = TVI_ROOT;
    EnumWindows(WindowEnumerator, reinterpret_cast<LPARAM>(this));
    SendMessage(hTree, WM_SETREDRAW, TRUE, 0);
    SelectTreeItem(m_selectedWindow);
    return true;
}

BOOL CWindowTreeDlg::WindowEnumerator(HWND hwnd, LPARAM lParam)
{
    auto* pThis = reinterpret_cast<CWindowTreeDlg*>(lParam);

    TCHAR buf[4096]{};
    GetWindowString(hwnd, buf, _countof(buf));

    TCHAR filter[MAX_PATH]{};
    ::GetDlgItemText(*pThis, IDC_FILTER, filter, _countof(filter));
    if (filter[0] == '\0' || StrStrI(buf, filter) != nullptr)
    {
        TVINSERTSTRUCT is     = {nullptr};
        is.hParent            = TVI_ROOT;
        is.hInsertAfter       = TVI_SORT;
        is.itemex.mask        = TVIF_TEXT | TVIF_PARAM;
        is.itemex.pszText     = buf;
        is.itemex.lParam      = reinterpret_cast<LPARAM>(hwnd);

        pThis->m_lastTreeItem = TreeView_InsertItem(GetDlgItem(*pThis, IDC_WINDOWTREE), &is);
    }
    else
    {
        pThis->m_lastTreeItem = TVI_ROOT;
    }
    EnumChildWindows(hwnd, ChildWindowEnumerator, lParam);
    return TRUE;
}

BOOL CWindowTreeDlg::ChildWindowEnumerator(HWND hwnd, LPARAM lParam)
{
    auto* pThis = reinterpret_cast<CWindowTreeDlg*>(lParam);

    TCHAR buf[4096]{};
    GetWindowString(hwnd, buf, _countof(buf));

    TCHAR filter[MAX_PATH]{};
    ::GetDlgItemText(*pThis, IDC_FILTER, filter, _countof(filter));
    if (filter[0] == '\0' || (StrStrI(buf, filter) != nullptr && GetDlgItem(*pThis, IDC_FILTER) != hwnd))
    {
        TVINSERTSTRUCT is = {nullptr};
        is.hParent        = pThis->m_lastTreeItem;
        is.hInsertAfter   = TVI_SORT;
        is.itemex.mask    = TVIF_TEXT | TVIF_PARAM;
        is.itemex.pszText = buf;
        is.itemex.lParam  = reinterpret_cast<LPARAM>(hwnd);

        TreeView_InsertItem(GetDlgItem(*pThis, IDC_WINDOWTREE), &is);
    }
    return TRUE;
}

void CWindowTreeDlg::GetWindowString(HWND hwnd, TCHAR* buf, int bufsize)
{
    TCHAR  tmpbuf[4096]         = {0};
    TCHAR  szClassName[100]     = {0};
    TCHAR  szProcName[MAX_PATH] = {0};
    TCHAR* pProcName            = szProcName;

    GetWindowText(hwnd, tmpbuf, _countof(tmpbuf));
    if (GetClassName(hwnd, szClassName, _countof(szClassName)) == 0)
        szClassName[0] = 0;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid)
    {
        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (hProc)
        {
            if (GetProcessImageFileName(hProc, szProcName, _countof(szProcName)))
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
    _sntprintf_s(buf, bufsize, _TRUNCATE, _T("Window 0x%p : Process \"%s\" : class \"%s\" : title \"%s\""), hwnd, pProcName, szClassName, tmpbuf);
}

HWND CWindowTreeDlg::GetSelectedWindowHandle()
{
    HTREEITEM selItem = TreeView_GetSelection(GetDlgItem(*this, IDC_WINDOWTREE));
    if (selItem == nullptr)
        return nullptr;

    TVITEM tvi = {0};
    tvi.hItem  = selItem;
    tvi.mask   = TVIF_PARAM;
    TreeView_GetItem(GetDlgItem(*this, IDC_WINDOWTREE), &tvi);
    return reinterpret_cast<HWND>(tvi.lParam);
}

void CWindowTreeDlg::SelectTreeItem(HWND windowHwnd)
{
    HWND      windowTreeHwnd = GetDlgItem(*this, IDC_WINDOWTREE);
    HTREEITEM actualItem     = TreeView_GetFirstVisible(windowTreeHwnd);

    // Search by the item into the list
    while (actualItem != nullptr)
    {
        TVITEM tvi = {0};
        tvi.hItem  = actualItem;
        tvi.mask   = TVIF_PARAM;
        TreeView_GetItem(windowTreeHwnd, &tvi);

        // If it is the item, select it and break the search
        if (reinterpret_cast<HWND>(tvi.lParam) == windowHwnd)
        {
            TreeView_SelectItem(windowTreeHwnd, actualItem);
            break;
        }

        // Find the next item in the TreeView
        HTREEITEM nextItem = TreeView_GetChild(windowTreeHwnd, actualItem);
        if (nextItem == nullptr)
            nextItem = TreeView_GetNextSibling(windowTreeHwnd, actualItem);

        HTREEITEM parentItem = actualItem;
        while ((nextItem == nullptr) && (parentItem != nullptr))
        {
            parentItem = TreeView_GetParent(windowTreeHwnd, parentItem);
            nextItem   = TreeView_GetNextSibling(windowTreeHwnd, parentItem);
        }
        actualItem = nextItem;
    }
}
