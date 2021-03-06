/* 
 *  Misc Stuff (profiling) for MPEG2Dec3
 *
 *	Copyright (C) 2002-2003 Marc Fauconneau <marc.fd@liberysurf.fr>
 *
 *  This file is part of MPEG2Dec3, a free MPEG-2 decoder
 *	
 *  MPEG2Dec3 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  MPEG2Dec3 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#include "global.h"

static ui64 local;
static TCHAR buffer[256];

unsigned __int64 read_counter(void)
{
	unsigned __int64 ts;
	unsigned __int32 ts1, ts2;
	__asm {
		rdtsc
		mov ts1, eax
		mov ts2, edx
	}
	ts = ((unsigned __int64) ts2 << 32) | ((unsigned __int64) ts1);
	return ts;
}

// get CPU frequency in Hz (1MHz precision)
ui64 get_freq(void)
{
	unsigned __int64  x = 0;
	time_t i;
	i = time(NULL);
	while (i == time(NULL));
	x -= read_counter();
	i++;
	while (i == time(NULL));
	x += read_counter();
	return x;
}

void init_first(ts* timers) {
	memset(timers,0,sizeof(ts));
	timers->div = 0; timers->sum = 0;
	timers->freq = get_freq()/1000;
//	timers->freq = 1412*1000000; // shortcut for my athlon xp 1600+ (1.4 Gz)
}

void init_timers(ts* timers) {
	timers->idct = 0;
	timers->conv = 0;
	timers->mcpy = 0;
	timers->post = 0;
	timers->dec = 0;
	timers->bit = 0;
	timers->decMB = 0;
	timers->mcmp = 0;
	timers->addb = 0;
	timers->overall = 0;
	if (timers->div > 25) { timers->div = 0; timers->sum = 0; }
}

void start_timer() {
	local = read_counter();
}

ui64 read_timer()
{
	ui64 tmp;

	tmp = local;
	local = read_counter();
	return (local - tmp);
}

void start_timer2(ui64* timer) {
	*timer -= read_counter();
}

void stop_timer(ui64*timer) {
	*timer += (read_counter() - local);
}

void stop_timer2(ui64* timer) {
	*timer += read_counter();
}

void timer_debug(ts* timers) {
	ts tim = *timers;
	//tim.freq = ;
//	sprintf(buffer,"conv = %I64d ",tim.conv);
//	sprintf(buffer,"idct = %I64d overall=%I64d idct% = %f",tim.idct,tim.overall,(double)tim.idct*100/tim.overall);
	_stprintf_s(buffer, _T("| dec%% = %f > mcmp%% = %f addb%% = %f idct%% = %f decMB%% = %f bit%% = %f | conv%% = %f | post%% = %f | mcpy%% = %f | msec = %f fps = %f mean = %f"),
		(double)tim.dec*100/tim.overall,
		(double)tim.idct*100/tim.overall,
		(double)tim.addb*100/tim.overall,
		(double)tim.mcmp*100/tim.overall,
		(double)tim.decMB*100/tim.overall,
		(double)tim.bit*100/tim.overall,
		(double)tim.conv*100/tim.overall,
		(double)tim.post*100/tim.overall,
		(double)tim.mcpy*100/tim.overall,
		(double)tim.overall*1000/tim.freq,
		tim.freq/(double)tim.overall,
		(tim.freq)/((double)tim.sum/tim.div)
		);
	OutputDebugString(buffer);
}

int dprintf(LPTSTR fmt, ...)
{
	TCHAR printString[1000];
	va_list argp;
	va_start(argp, fmt);
	_vstprintf_s(printString, fmt, argp);
	va_end(argp);
    OutputDebugString(printString);
	return _tcslen(printString);
}

CString FindExtension(const CString& strPath)
{
	CString strResult;

	int nPos = 0;
	nPos = strPath.ReverseFind(_T('.'));
	if (nPos != -1)
	{
		int nLength = strPath.GetLength();
		strResult = strPath.Right(nLength - nPos);
	}

	return strResult;
}

void RemoveExtension(CString &strPath)
{
	RenameExtension(strPath, _T(""));
}

// Removes extension from path.
void RenameExtension(CString &strPath, const CString &strExtension)
{
	int nIndex = strPath.ReverseFind(_T('.'));
	if (nIndex != -1)
	{
		strPath.Truncate(nIndex);
		if (!strExtension.IsEmpty())
		{
			if (strExtension[0] == _T('.'))
				strPath.Append(strExtension);
			else
			{
				strPath += _T(".");
				strPath += strExtension;
			}
		}
	}
}

// Removes trailing filename and backslash: c:\dir\filename.ext -> c:\dir
void RemoveFileSpec(CString &strPath)
{
	int nIndex = strPath.ReverseFind(_T('\\'));
	if (nIndex != -1)
	{
		strPath.Truncate(nIndex);
	}
}

// Removes the path portion of a fully qualified path and file
void StripPath(CString &strPath)
{
	int nIndex = strPath.ReverseFind(_T('\\'));
	if (nIndex != -1)
	{
		int nLength = strPath.GetLength();
		// one symbol - backslash, do nothing
		if (nLength != 1)
			// directory with backslash
			if (nIndex == nLength - 1)
				strPath.Truncate(nIndex);
		// directory with filename
		else
			strPath = strPath.Right(nLength - nIndex - 1);
	}
}

void AddBackslash(CString &strPath)
{
	if (!strPath.IsEmpty())
		if (strPath.Right(1) != _T('\\'))
			strPath.Append(_T("\\"));
}

void GetCurrentDirectory(CString &strBuffer)
{
	DWORD dwLength = GetCurrentDirectory(0, NULL);
	GetCurrentDirectory(dwLength, strBuffer.GetBufferSetLength(dwLength - 1));
	strBuffer.ReleaseBuffer();
	
}
