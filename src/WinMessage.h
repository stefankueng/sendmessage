// SendMessage - a tool to send custom messages

// Copyright (C) 2013, 2015, 2021 - Stefan Kueng

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
#include "ResourceTextFile.h"
#include "XMLite.h"

struct WinMessage
{
    std::wstring                                  messagename;
    UINT                                          message;
    std::vector<std::tuple<std::wstring, WPARAM>> wparams;
    std::vector<std::tuple<std::wstring, LPARAM>> lparams;
};

class WinMessages
{
private:
    WinMessages()
        : m_bInit(false)
    {
    }
    ~WinMessages()
    {
    }

public:
    static WinMessages& Instance();

    size_t              GetCount() const { return m_messages.size(); }
    WinMessage          At(size_t index) const { return m_messages[index]; }
    UINT                ParseMsg(LPCTSTR text) const;
    UINT                GetApiMessageCode(LPCTSTR text) const;

private:
    void                    Init();
    bool                    m_bInit;
    CResourceTextFile       m_xmlResource;
    XNode                   m_xml;

    std::vector<WinMessage> m_messages;
};
