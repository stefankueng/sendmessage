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

#pragma once

#include "resource.h"

class CAccessibleName
{
    BSTR m_str;
public:
    explicit CAccessibleName(HWND hwnd): m_str(NULL)
    {
        IAccessible *lpAccessible = NULL;
        if (SUCCEEDED(AccessibleObjectFromWindow(hwnd, OBJID_WINDOW, IID_IAccessible, (void **)&lpAccessible)))
        {
            VARIANT varChild;
            varChild.vt = VT_I4;
            varChild.lVal = 0;
            lpAccessible->get_accName(varChild, &m_str);
            lpAccessible->Release();
        }
    }
    ~CAccessibleName()
    {
        SysFreeString(m_str);
    }
    LPCWSTR c_str() const
    {
        return m_str ? m_str : L"";
    }
};
