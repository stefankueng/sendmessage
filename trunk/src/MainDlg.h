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
#include "BaseDialog.h"
#include "AeroControls.h"
#include "HyperLink.h"
#include "ResourceTextFile.h"
#include "XMLite.h"
#include <string>
#include <vector>

#define ID_ABOUTBOX         0x0010

/**
 * main dialog.
 */
class CMainDlg : public CDialog
{
public:
    CMainDlg(HWND hParent);
    ~CMainDlg(void);

protected:
    LRESULT CALLBACK        DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT                 DoCommand(int id, int msg);
    bool                    PreTranslateMessage(MSG* pMsg);

    void                    SaveWndPosition();
    bool                    SearchWindow();
    bool                    DoMouseMove(UINT message, WPARAM wParam, LPARAM lParam);
    bool                    DoMouseUp(UINT message, WPARAM wParam, LPARAM lParam);
    bool                    CheckWindowValidity(HWND hwndToCheck);
    bool                    DisplayInfoOnFoundWindow(HWND hwndFoundWindow);
    bool                    RefreshWindow(HWND hwndWindowToBeRefreshed);
    bool                    HighlightFoundWindow(HWND hwndFoundWindow);
    bool                    SendPostMessage(UINT id);
    HWND                    GetSelectedHandle();

private:
    HWND                    m_hParent;
    CHyperLink              m_link;
    AeroControlBase         m_aerocontrols;
    bool                    m_bStartSearchWindow;
    HWND                    m_hwndFoundWindow;
    HPEN                    m_hRectanglePen;
    CResourceTextFile       m_xmlResource;
    XNode                   m_xml;
};
