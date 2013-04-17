// SendMessage - a tool to send custom messages

// Copyright (C) 2013 - Stefan Kueng

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
#include "WinMessage.h"
#include "resource.h"

extern HINSTANCE hInst;

UINT WinMessages::GetApiMessageCode(LPCTSTR text)
{
    for (const auto& it:m_messages)
    {
        if (_wcsicmp(it.messagename.c_str(), text)==0)
            return it.message;
    }
    return 0;
}

UINT WinMessages::ParseMsg(LPCTSTR text)
{
    UINT msg = 0;
    TCHAR * endptr = NULL;

    msg = _tcstol(text, &endptr, 0);
    if (msg == 0)
    {
        // Find API Message
        msg = GetApiMessageCode(text);
    }
    if (msg == 0)
    {
        // Get custom message
        msg = ::RegisterWindowMessage(text);
    }
    return msg;
}

WinMessages& WinMessages::Instance()
{
    static WinMessages instance;
    if (!instance.m_bInit)
        instance.Init();
    return instance;

}

void WinMessages::Init()
{
    m_xmlResource.Open(hInst, MAKEINTRESOURCE(IDR_WINDOWMESSAGESXML), _T("TEXT"), CResourceTextFile::ConvertToUnicode);
    m_xml.Load(m_xmlResource.GetTextBuffer());
    // now fill the window message combo box
    XNodes childs;
    childs = m_xml.GetChilds(_T("Message") );
    for( size_t i = 0 ; i < childs.size(); ++i)
    {
        WinMessage msg;
        msg.messagename = childs[i]->GetAttrValue(_T("description"));
        TCHAR * endptr = NULL;
        msg.message = _tcstol(childs[i]->GetAttrValue(_T("value")), &endptr, 0);

        XNodes wparams = childs[i]->GetChilds(_T("wparam"));
        for (size_t j = 0; j < wparams.size(); ++j)
        {
            std::wstring desc = wparams[j]->GetAttrValue(_T("description"));
            TCHAR * endptr = NULL;
            WPARAM m = _tcstol(wparams[j]->GetAttrValue(_T("value")), &endptr, 0);
            msg.wparams.push_back(std::make_tuple(desc, m));
        }
        XNodes lparams = childs[i]->GetChilds(_T("lparam"));
        for (size_t j = 0; j < lparams.size(); ++j)
        {
            std::wstring desc = lparams[j]->GetAttrValue(_T("description"));
            TCHAR * endptr = NULL;
            UINT m = _tcstol(lparams[j]->GetAttrValue(_T("value")), &endptr, 0);
            msg.lparams.push_back(std::make_tuple(desc, m));
        }

        m_messages.push_back(msg);
    }
    m_bInit = true;
}
