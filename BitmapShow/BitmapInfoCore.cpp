/************************************************************

BSD 2-Clause License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Copyright (c) 2017, KondeU, All rights reserved.

Project:     BitmapShow
File:        BitmapInfoCore.cpp
Description: Could only run on Windows. Extract bitmap information and data.
Date:        2017-10-26
Version:     1.1
Authors:     Deyou Kong <370242479@qq.com>
History:     01, 17-10-26, Deyou Kong, Create file and implement it.

************************************************************/

#include <Windows.h>

void Printf(HWND hwnd, TCHAR * szFormat, ...)
{
	// "hwnd" is a edit window

	TCHAR szBuffer[1024]; // Make sure the size of string is less than 1024

	va_list pArgList;

	va_start(pArgList, szFormat);
	wvsprintf(szBuffer, szFormat, pArgList);
	va_end(pArgList);

	SendMessage(hwnd, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
	SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM)szBuffer);
	SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
}

PBYTE BitmapInfo(HWND hwnd, TCHAR * szFileName, DWORD & dwWidth, DWORD & dwHeight)
{
	dwWidth  = 0;
	dwHeight = 0;

	// Display the file name
	Printf(hwnd, TEXT("File: %s\r\n\r\n"), szFileName);

	// Open the file
	HANDLE hFile = CreateFile(
		szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		Printf(hwnd, TEXT("Cannot open file.\r\n\r\n"));
		return nullptr;
	}

	// Get the size of the file
	DWORD dwHighSize = 0;
	DWORD dwFileSize = GetFileSize(hFile, &dwHighSize);
	if (dwHighSize)
	{
		Printf(hwnd, TEXT("Cannot deal with >4G files.\r\n\r\n"));
		CloseHandle(hFile);
		return nullptr;
	}

	// Allocate memory for the file
	static PBYTE pFile = nullptr;
	if (pFile)
	{
		pFile = (PBYTE)realloc(pFile, dwFileSize);
	}
	else
	{
		pFile = (PBYTE)malloc(dwFileSize);
	}
	if (!pFile)
	{
		Printf(hwnd, TEXT("Cannot allocate memory.\r\n\r\n"));
		CloseHandle(hFile);
		return nullptr;
	}

	// Read the file
	SetCursor(LoadCursor(NULL, IDC_WAIT));
	ShowCursor(TRUE);
	DWORD dwBytesRead;
	BOOL bSuccess = ReadFile(hFile, pFile, dwFileSize, &dwBytesRead, NULL);
	ShowCursor(FALSE);
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	// Check if failed
	if (!bSuccess || (dwBytesRead != dwFileSize))
	{
		Printf(hwnd, TEXT("Could not read file.\r\n\r\n"));
		CloseHandle(hFile);
		return nullptr;
	}

	if ((((BITMAPFILEHEADER *)pFile)->bfType != *(WORD *)"BM"))
	{
		Printf(hwnd, TEXT("It is not a bitmap file.\r\n\r\n"));
		CloseHandle(hFile);
		return nullptr;
	}

	// Close the file
	CloseHandle(hFile);

	// Read bitmap file successfully, display bitmap infomation now
	static TCHAR * szInfoName[] =
	{
		TEXT("BITMAPCOREHEADER"),
		TEXT("BITMAPINFOHEADER"),
		TEXT("BITMAPV4HEADER"),
		TEXT("BITMAPV5HEADER")
	};
	static TCHAR * szCompression[] =
	{
		TEXT("BI_RGB"),
		TEXT("BI_RLE8"),
		TEXT("BI_RLE4"),
		TEXT("BI_BITFIELDS"),
		TEXT("Unknown")
	};

	// Display file size
	Printf(hwnd, TEXT("File size = %u bytes\r\n\r\n"), dwFileSize);

	// Display BITMAPFILEHEADER structure
	BITMAPFILEHEADER * pbmfh = (BITMAPFILEHEADER *)pFile;
	Printf(hwnd, TEXT("BITMAPFILEHEADER\r\n"));
	Printf(hwnd, TEXT("\t.bfType = 0x%X\r\n"),      pbmfh->bfType);
	Printf(hwnd, TEXT("\t.bfSize = %u\r\n"),        pbmfh->bfSize);
	Printf(hwnd, TEXT("\t.bfReserved1 = %u\r\n"),   pbmfh->bfReserved1);
	Printf(hwnd, TEXT("\t.bfReserved2 = %u\r\n"),   pbmfh->bfReserved2);
	Printf(hwnd, TEXT("\t.bfOffBits = %u\r\n\r\n"), pbmfh->bfOffBits);

	// Determine version
	BITMAPV5HEADER * pbmih = (BITMAPV5HEADER *)(pFile + sizeof(BITMAPFILEHEADER));
	int iVersion = 0;
	TCHAR * szVerAdd = TEXT("");
	switch (pbmih->bV5Size)
	{
	case sizeof(BITMAPCOREHEADER):
		iVersion = 0;
		break;

	case sizeof(BITMAPINFOHEADER):
		iVersion = 1;
		szVerAdd = TEXT("i");
		break;

	case sizeof(BITMAPV4HEADER):
		iVersion = 2;
		szVerAdd = TEXT("V4");
		break;

	case sizeof(BITMAPV5HEADER):
		iVersion = 3;
		szVerAdd = TEXT("V5");
		break;

	default:
		Printf(hwnd, TEXT("Unknown header size of %u.\r\n\r\n"), pbmih->bV5Size);
		return nullptr;
	}

	// Display the bitmap version
	Printf(hwnd, TEXT("%s\r\n"), szInfoName[iVersion]);

	// Bitmap version lists:
	// BITMAPCOREHEADER
	// BITMAPINFOHEADER
	// BITMAPV4HEADER
	// BITMAPV5HEADER

	// Display the BITMAPCOREHEADER
	// (Haved: BITMAPCOREHEADER)
	if (pbmih->bV5Size == sizeof(BITMAPCOREHEADER))
	{
		BITMAPCOREHEADER * pbmch = (BITMAPCOREHEADER *)pbmih;
		Printf(hwnd, TEXT("\t.bcSize = %u\r\n"),         pbmch->bcSize);
		Printf(hwnd, TEXT("\t.bcWidth = %u\r\n"),        pbmch->bcWidth);
		Printf(hwnd, TEXT("\t.bcHeight = %u\r\n"),       pbmch->bcHeight);
		Printf(hwnd, TEXT("\t.bcPlanes = %u\r\n"),       pbmch->bcPlanes);
		Printf(hwnd, TEXT("\t.bcBitCount = %u\r\n\r\n"), pbmch->bcBitCount);
		dwWidth  = pbmch->bcWidth;
		dwHeight = pbmch->bcHeight;
		return pFile;
	}

	// Display the BITMAPINFOHEADER
	// (Haved: BITMAPINFOHEADER & BITMAPV4HEADER & BITMAPV5HEADER)
	Printf(hwnd, TEXT("\t.b%sSize = %u\r\n"),     szVerAdd, pbmih->bV5Size);
	Printf(hwnd, TEXT("\t.b%sWidth = %i\r\n"),    szVerAdd, pbmih->bV5Width);
	Printf(hwnd, TEXT("\t.b%sHeight = %i\r\n"),   szVerAdd, pbmih->bV5Height);
	Printf(hwnd, TEXT("\t.b%sPlanes = %u\r\n"),   szVerAdd, pbmih->bV5Planes);
	Printf(hwnd, TEXT("\t.b%sBitCount = %u\r\n"), szVerAdd, pbmih->bV5BitCount);
	Printf(hwnd, TEXT("\t.b%sCompression = %s\r\n"),
		szVerAdd, szCompression[min(4, pbmih->bV5Compression)]);
	Printf(hwnd, TEXT("\t.b%sSizeImage = %u\r\n"),     szVerAdd, pbmih->bV5SizeImage);
	Printf(hwnd, TEXT("\t.b%sXPelsPerMeter = %i\r\n"), szVerAdd, pbmih->bV5XPelsPerMeter);
	Printf(hwnd, TEXT("\t.b%sYPelsPerMeter = %i\r\n"), szVerAdd, pbmih->bV5YPelsPerMeter);
	Printf(hwnd, TEXT("\t.b%sClrUsed = %i\r\n"),          szVerAdd, pbmih->bV5ClrUsed);
	Printf(hwnd, TEXT("\t.b%sClrImportant = %i\r\n\r\n"), szVerAdd, pbmih->bV5ClrImportant);
	dwWidth  = pbmih->bV5Width;
	dwHeight = pbmih->bV5Height;

	// Display the BITMAPINFOHEADER
	// (Haved: BITMAPINFOHEADER)
	if (pbmih->bV5Size == sizeof(BITMAPINFOHEADER))
	{
		if (BI_BITFIELDS == pbmih->bV5Compression)
		{
			Printf(hwnd, TEXT("Red Mask   = %08X\r\n"),     pbmih->bV5RedMask);
			Printf(hwnd, TEXT("Green Mask = %08X\r\n"),     pbmih->bV5GreenMask);
			Printf(hwnd, TEXT("Blue Mask  = %08X\r\n\r\n"), pbmih->bV5BlueMask);
		}
		return pFile;
	}

	// Display the BITMAPV4HEADER
	// (Haved: BITMAPV4HEADER & BITMAPV5HEADER)
	Printf(hwnd, TEXT("\t.b%sRedMask   = %08X\r\n"), szVerAdd, pbmih->bV5RedMask);
	Printf(hwnd, TEXT("\t.b%sGreenMask = %08X\r\n"), szVerAdd, pbmih->bV5GreenMask);
	Printf(hwnd, TEXT("\t.b%sBlueMask  = %08X\r\n"), szVerAdd, pbmih->bV5BlueMask);
	Printf(hwnd, TEXT("\t.b%sAlphaMask = %08X\r\n"), szVerAdd, pbmih->bV5AlphaMask);
	Printf(hwnd, TEXT("\t.b%sCSType = %u\r\n"), szVerAdd, pbmih->bV5CSType);
	Printf(hwnd, TEXT("\t.b%sEndpoints.ciexyzRed.ciexyzX   = %08X\r\n"), szVerAdd, pbmih->bV5Endpoints.ciexyzRed.ciexyzX);
	Printf(hwnd, TEXT("\t.b%sEndpoints.ciexyzRed.ciexyzY   = %08X\r\n"), szVerAdd, pbmih->bV5Endpoints.ciexyzRed.ciexyzY);
	Printf(hwnd, TEXT("\t.b%sEndpoints.ciexyzRed.ciexyzZ   = %08X\r\n"), szVerAdd, pbmih->bV5Endpoints.ciexyzRed.ciexyzZ);
	Printf(hwnd, TEXT("\t.b%sEndpoints.ciexyzGreen.ciexyzX = %08X\r\n"), szVerAdd, pbmih->bV5Endpoints.ciexyzGreen.ciexyzX);
	Printf(hwnd, TEXT("\t.b%sEndpoints.ciexyzGreen.ciexyzY = %08X\r\n"), szVerAdd, pbmih->bV5Endpoints.ciexyzGreen.ciexyzY);
	Printf(hwnd, TEXT("\t.b%sEndpoints.ciexyzGreen.ciexyzZ = %08X\r\n"), szVerAdd, pbmih->bV5Endpoints.ciexyzGreen.ciexyzZ);
	Printf(hwnd, TEXT("\t.b%sEndpoints.ciexyzBlue.ciexyzX  = %08X\r\n"), szVerAdd, pbmih->bV5Endpoints.ciexyzBlue.ciexyzX);
	Printf(hwnd, TEXT("\t.b%sEndpoints.ciexyzBlue.ciexyzY  = %08X\r\n"), szVerAdd, pbmih->bV5Endpoints.ciexyzBlue.ciexyzY);
	Printf(hwnd, TEXT("\t.b%sEndpoints.ciexyzBlue.ciexyzZ  = %08X\r\n"), szVerAdd, pbmih->bV5Endpoints.ciexyzBlue.ciexyzZ);
	Printf(hwnd, TEXT("\t.b%sGammaRed   = %08X\r\n"),     szVerAdd, pbmih->bV5GammaRed);
	Printf(hwnd, TEXT("\t.b%sGammaGreen = %08X\r\n"),     szVerAdd, pbmih->bV5GammaGreen);
	Printf(hwnd, TEXT("\t.b%sGammaBlue  = %08X\r\n\r\n"), szVerAdd, pbmih->bV5GammaBlue);
	if (pbmih->bV5Size == sizeof(BITMAPV4HEADER))
	{
		return pFile;
	}

	// Display the BITMAPV5HEADER
	// (Haved: BITMAPV5HEADER)
	Printf(hwnd, TEXT("\t.b%sIntent = %u\r\n"),       szVerAdd, pbmih->bV5Intent);
	Printf(hwnd, TEXT("\t.b%sProfileData = %u\r\n"),  szVerAdd, pbmih->bV5ProfileData);
	Printf(hwnd, TEXT("\t.b%sProfileSize = %u\r\n"),  szVerAdd, pbmih->bV5ProfileSize);
	Printf(hwnd, TEXT("\t.b%sReserved = %u\r\n\r\n"), szVerAdd, pbmih->bV5Reserved);
	return pFile;
}