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

#pragma once
#include "basedialog.h"
#include "AeroControls.h"
#include "DlgResizer.h"

/**
 * dialog showing a tree of all windows
 */
class CWindowTreeDlg : public CDialog
{
public:
    CWindowTreeDlg(HWND hParent, HWND actualHandle);
    ~CWindowTreeDlg(void);

    HWND                    GetSelectedWindow() { return m_SelectedWindow; }
protected:
    LRESULT CALLBACK        DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT                 DoCommand(int id, int msg);
    bool                    PreTranslateMessage(MSG* pMsg);
    static void             GetWindowString(HWND hwnd, TCHAR * buf, int bufsize);
    bool                    RefreshTree();
    static BOOL CALLBACK    WindowEnumerator(HWND hwnd, LPARAM lParam);
    static BOOL CALLBACK    ChildWindowEnumerator(HWND hwnd, LPARAM lParam);
    HWND                    GetSelectedWindowHandle();
    void                    SelectTreeItem(HWND windowHwnd);

private:
    HWND                    m_hParent;
    AeroControlBase         m_aerocontrols;
    CDlgResizer             m_resizer;
    HTREEITEM               m_lastTreeItem;
    HWND                    m_SelectedWindow;

};
