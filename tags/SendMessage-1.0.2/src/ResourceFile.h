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

class CResourceFile
{
public:
    CResourceFile();
    CResourceFile(const CResourceFile&);
    virtual ~CResourceFile();

    BYTE *  GetByteBuffer()         { return m_pBytes; }
    int     GetLength()             { return m_nBufLen; }
    int     GetPosition()           { return m_nPosition; }
    BOOL    IsAtEOF()               { return m_nPosition >= GetLength(); }
    BOOL    IsOpen()                { return m_bIsOpen; }

public:
    virtual void    Close();
    virtual BYTE *  DetachByteBuffer();
    virtual BYTE *  DuplicateByteBuffer();
    virtual BOOL    Open(HINSTANCE hInstance,
                    LPCTSTR lpszResId,
                    LPCTSTR lpszResType);
    virtual int     Read(BYTE *buf, int nBufLen);
    virtual int     Seek(int offset, int origin);
    virtual int     SeekToBegin();
    virtual int     SeekToEnd();
    virtual void    SetByteBuffer(BYTE * buf, DWORD len);

protected:
    BYTE  *     m_pBytes;               // binary buffer
    int         m_nBufLen;              // size of m_pszResource: TCHARs for text,
                                        // bytes for binary
    int         m_nPosition;            // buffer position: TCHARs for text,
                                        // bytes for binary
    BOOL        m_bText;                // TRUE = text, FALSE = binary
    BOOL        m_bIsOpen;              // TRUE = text file resource is open
    BOOL        m_bDoNotDeleteBuffer;   // TRUE = buffer allocated externally
                                        // or detached; do not delete
};

