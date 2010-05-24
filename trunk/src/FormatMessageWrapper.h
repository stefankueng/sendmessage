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

/**
 * A wrapper class for calling the FormatMessage() Win32 function and controlling
 * the lifetime of the allocated error message buffer.
 */

class CFormatMessageWrapper
{
private:
    LPTSTR buffer;
    DWORD result;
    void release();
    void obtainMessage() { obtainMessage(::GetLastError()); }
    void obtainMessage(DWORD errorCode);

public:
    CFormatMessageWrapper() : buffer(0), result(0) {obtainMessage();}
    CFormatMessageWrapper(DWORD lastError) : buffer(0), result(0) {obtainMessage(lastError);}
    ~CFormatMessageWrapper() { release(); }
    operator LPCTSTR() { return buffer; }
    operator bool() { return result != 0; }
    bool operator!() { return result == 0; }
};

inline void CFormatMessageWrapper::obtainMessage(DWORD errorCode)
{
    // First of all release the buffer to make it possible to call this
    // method more than once on the same object.
    release();
    result = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                            NULL,
                            errorCode,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                            (LPTSTR) &buffer,
                            0,
                            NULL
                            );
}

inline void CFormatMessageWrapper::release()
{
    if(buffer != 0)
    {
        LocalFree(buffer);
        buffer = 0;
    }

    result = 0;
}