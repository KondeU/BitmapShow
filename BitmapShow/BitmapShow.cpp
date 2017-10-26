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
File:        BitmapShow.cpp
Description: Could only run on Windows. Windows execute.
Date:        2017-10-26
Version:     1.1
Authors:     Deyou Kong <370242479@qq.com>
History:     01, 17-10-26, Deyou Kong, Create file and implement it.

************************************************************/

#include <Windows.h>
#pragma comment(linker,                                      \
				"/manifestdependency:\""                     \
				"type='win32' "                              \
				"name='Microsoft.Windows.Common-Controls' "  \
				"version='6.0.0.0' "                         \
				"processorArchitecture='*' "                 \
				"publicKeyToken='6595b64144ccf1df' "         \
				"language='*'\"")

#include "resource.h"

#include "CmnDlg.h"
using namespace COMMONDIALOG;

static const TCHAR szAppNameEng[] = TEXT("BitmapShow");
static const TCHAR szAppNameChn[] = TEXT("位图显示器");

TCHAR g_szBmpDir[MAX_PATH] = { 0 };
PBYTE g_pBitmap = nullptr;
DWORD g_dwBmpWidth  = 0;
DWORD g_dwBmpHeight = 0;

namespace
{
	HINSTANCE g_hInst;
	HWND g_hwndMain;
	HWND g_hDlgLoad;
	HWND g_hwndInfo;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcInfo(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcLoad(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Functions from BitmapInfoCore.cpp
void Printf(HWND hwnd, TCHAR * szFormat, ...);
PBYTE BitmapInfo(HWND hwnd, TCHAR * szFileName, DWORD & dwWidth, DWORD & dwHeight);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, INT iCmdShow)
{
	g_hInst = hInstance;

	HWND     hwnd;
	MSG      msg;
	WNDCLASS wndclass;
	memset(&hwnd,     0, sizeof(HWND));
	memset(&msg,      0, sizeof(MSG));
	memset(&wndclass, 0, sizeof(WNDCLASS));

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szAppNameEng;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("错误：主窗口类注册失败，该应用程序最低版本限制为WindowsNT！"),
			szAppNameChn, MB_ICONERROR | MB_OK);
		return 0;
	}

	hwnd = CreateWindow(
		szAppNameEng, szAppNameChn, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		GetDesktopWindow(), NULL, hInstance, NULL);

	g_hwndMain = hwnd;

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	//--- Program init. ---//

	//Accel.

	//HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	//--- Message loop. ---//

	while (GetMessage(&msg, NULL, 0, 0))
	{
		//if (TranslateAccelerator(hwnd, hAccel, &msg))
		//{
		//	continue;
		//}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//--- Program end. ---//

	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HINSTANCE hInst;
	static HWND hDlgLoad; // Dialog
	static HWND hwndInfo; // Window

	HDC hdc;
	PAINTSTRUCT ps;

	switch (message)
	{
	case WM_CREATE:
		{
			hInst = ((LPCREATESTRUCT)lParam)->hInstance;

			hDlgLoad = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DLG_LOAD), hwnd, DlgProcLoad);

			g_hDlgLoad = hDlgLoad;

			WNDCLASS wndclassInfo;
			memset(&wndclassInfo, 0, sizeof(WNDCLASS));
			wndclassInfo.style         = CS_HREDRAW | CS_VREDRAW;
			wndclassInfo.lpfnWndProc   = WndProcInfo;
			wndclassInfo.cbClsExtra    = 0;
			wndclassInfo.cbWndExtra    = 0;
			wndclassInfo.hInstance     = hInst;
			wndclassInfo.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));
			wndclassInfo.hCursor       = LoadCursor(NULL, IDC_ARROW);
			wndclassInfo.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
			wndclassInfo.lpszMenuName  = NULL;
			wndclassInfo.lpszClassName = TEXT("BmpShowInfo");

			if (!RegisterClass(&wndclassInfo))
			{
				MessageBox(hwnd, TEXT("创建位图详细信息窗口失败！"),
					szAppNameChn, MB_ICONERROR | MB_OK);
				return 0;
			}

			RECT rectMainClientRect;
			BOOL bGetRectRet = GetClientRect(hwnd, &rectMainClientRect);

			hwndInfo = CreateWindow(
				TEXT("BmpShowInfo"), TEXT("位图详细信息"),
				WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_CHILD | WS_VISIBLE,
				bGetRectRet ? (rectMainClientRect.right - 320) : CW_USEDEFAULT,
				bGetRectRet ? 0 : CW_USEDEFAULT,
				320, 480,
				hwnd, NULL, hInst, NULL);

			g_hwndInfo = hwndInfo;

			return 0;
		}

	case WM_PAINT:
		{
			hdc = BeginPaint(hwnd, &ps);
			if (g_pBitmap)
			{
				SetDIBitsToDevice(
					hdc, 0, 0,                         // DeviceContext, xDst, yDst
					g_dwBmpWidth, g_dwBmpHeight, 0, 0, // width, height, xSrc, ySrc
					0, g_dwBmpHeight, // first scan line, number of scan lines
					g_pBitmap + ((BITMAPFILEHEADER *)g_pBitmap)->bfOffBits,
					(BITMAPINFO *)(((BITMAPFILEHEADER *)g_pBitmap) + 1),
					DIB_RGB_COLORS);
			}
			EndPaint(hwnd, &ps);
			return 0;
		}

	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


BOOL CALLBACK DlgProcLoad(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static COSDlg cOpenDlg(hDlg);

	switch (message)
	{
	case WM_INITDIALOG:
		{
			cOpenDlg.SetFilter(2, TEXT("位图文件（*.bmp）"), TEXT("*.bmp"), TEXT("所有文件（*.*）"), TEXT("*.*"));
			cOpenDlg.SetDefExt(TEXT("bmp"));
			return TRUE;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_OK:
			{
				if (!GetDlgItemText(hDlg, IDC_EDIT_DIR, g_szBmpDir, MAX_PATH))
				{
					MessageBox(hDlg, TEXT("获取位图路径信息失败！"), szAppNameChn, MB_ICONERROR | MB_OK);
					break;
				}
				g_pBitmap = BitmapInfo(GetDlgItem(g_hwndInfo, 1), g_szBmpDir,
					g_dwBmpWidth, g_dwBmpHeight); // Need global params: hwnd, w, h
				Printf(GetDlgItem(g_hwndInfo, 1),
					TEXT("%s\r\n\r\n\r\n"),
					TEXT("-----------------------------------")
					TEXT("-----------------------------------"));
				InvalidateRect(GetParent(hDlg), NULL, TRUE);
				break;
			}

		case IDC_BTN_BROWSE:
			if (cOpenDlg.CmnDlgOpenFile())
			{
				SetDlgItemText(hDlg, IDC_EDIT_DIR, cOpenDlg.GetFilePath());
			}
			break;
		}
	}

	return FALSE;
}

LRESULT CALLBACK WndProcInfo(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndEdit;
	
	switch (message)
	{
	case WM_CREATE:
		hwndEdit = CreateWindow(TEXT("edit"), NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER |
			WS_VSCROLL | WS_HSCROLL |
			ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
			0, 0, 0, 0, hwnd, (HMENU)1, // ID is set to 1
			((LPCREATESTRUCT)lParam)->hInstance, NULL);
		return 0;

	case WM_SIZE:
		MoveWindow(hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

