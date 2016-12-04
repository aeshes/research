#include <windows.h>
#include <string.h>
#include "define.h"


#pragma comment(lib, "user32.lib")
#pragma comment(lib, "msimg32.lib")

#define ID_KEY_FIELD	3000
#define ID_KEY_ENTER	3001
#define ID_KEY_GROUP	3002
#define ID_BTN_ZERO		3003
#define ID_BTN_ONE		3004
#define ID_BTN_TWO		3005
#define ID_BTN_THREE	3006
#define ID_BTN_FOUR		3007
#define ID_BTN_FIVE		3008
#define ID_BTN_SIX		3009
#define ID_BTN_SEVEN	3010
#define ID_BTN_EIGHT	3011
#define ID_BTN_NINE		3012
#define ID_BTN_A		3013
#define ID_BTN_B		3014
#define ID_BTN_C		3015
#define ID_BTN_D		3016
#define ID_BTN_E		3017
#define ID_BTN_F		3018
#define ID_BTN_CLEAR	3019

const char *KeyboardButtonNames[16] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };
const int KeyboardButtonIDs[16] = { 3003, 3004, 3005, 3006, 3007, 3008, 3009, 3010,
							   3011, 3012, 3013, 3014, 3015, 3016, 3017, 3018 };



void TrackChildProcess(LPVOID param)
{
	while (1)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		CHAR szExePath[MAX_PATH];
		CHAR szCmdLine[MAX_PATH];
		DWORD pid = GetCurrentProcessId();

		GetModuleFileName(GetModuleHandle(NULL), szExePath, MAX_PATH);
		wsprintf(szCmdLine, "%s %d\0", szExePath, pid);

		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		CreateProcess(NULL,
			szCmdLine,
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE,
			NULL,
			NULL,
			&si,
			&pi);

		WaitForSingleObject(pi.hProcess, INFINITE);
	}
}

void TrackParentProcess(LPVOID hParentProcess)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	CHAR szExePath[MAX_PATH];

	WaitForSingleObject((HANDLE)hParentProcess, INFINITE);

	GetModuleFileName(GetModuleHandle(NULL), szExePath, MAX_PATH);

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	CreateProcess(NULL,
		szExePath,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi);
	ExitProcess(0);
}

void ShowMainWindow(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	/*HANDLE hParentProcess = NULL;
	DWORD  ProcessID = 0;

	if (lpCmdLine != NULL)
		ProcessID = atoi(lpCmdLine);

	if (ProcessID == 0)
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TrackChildProcess, NULL, 0, NULL);
	}
	else
	{
		hParentProcess = OpenProcess(SYNCHRONIZE, FALSE, ProcessID);
		if (hParentProcess)
		{
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TrackParentProcess, (LPVOID)hParentProcess, 0, NULL);
			while (1)
			{
				Sleep(500);
			}
		}
	}*/

	// Main logic goes here...
	ShowMainWindow();
}

void RegisterWndClass(WNDPROC Proc, LPCTSTR szName, UINT brBackground)
{
	WNDCLASSEX wc;

	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = Proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(0x14, 0x14, 0x14));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, TEXT("Cannot register class"), TEXT("Error"), MB_OK);
	}
}

// Draw custom button
void DrawButton(DRAWITEMSTRUCT *Item, HWND hwnd)
{

	SelectObject(Item->hDC, CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial Black"));
	FillRect(Item->hDC, &Item->rcItem, CreateSolidBrush(RGB(0x14, 0x14, 0x14)));
	SelectObject(Item->hDC, CreateSolidBrush(0));
	if (Item->itemState & ODS_SELECTED)
	{
		SetTextColor(Item->hDC, 0);
		SelectObject(Item->hDC, CreateSolidBrush(0xFF00));
		SelectObject(Item->hDC, CreatePen(PS_SOLID, 2, 0xFF00));
	}
	else
	{
		SetTextColor(Item->hDC, 0x00FF00);
		SelectObject(Item->hDC, CreatePen(PS_SOLID, 2, 0x00FF00));

	}
	SetBkMode(Item->hDC, TRANSPARENT);
	//RoundRect(Item->hDC, Item->rcItem.left, Item->rcItem.top, Item->rcItem.right, Item->rcItem.bottom, 20, 20);
	Rectangle(Item->hDC, Item->rcItem.left, Item->rcItem.top, Item->rcItem.right, Item->rcItem.bottom);
	int len = GetWindowTextLength(Item->hwndItem);
	LPSTR lpBuff[255];
	GetWindowTextA(Item->hwndItem, lpBuff, len + 1);
	DrawTextA(Item->hDC, lpBuff, len, &Item->rcItem, DT_CENTER | DT_VCENTER);
}

// Trying to draw customgroupbox
void DrawKeyGroup(DRAWITEMSTRUCT *Item, HWND hwnd)
{
	SelectObject(Item->hDC, CreateSolidBrush(RGB(0, 255, 0)));
	SelectObject(Item->hDC, CreateSolidBrush(0));

	SetTextColor(Item->hDC, 0x00FF00);
	SelectObject(Item->hDC, CreatePen(PS_SOLID, 2, 0x00FF00));
	MessageBox(NULL, "", "", MB_OK);

	//SetBkMode(Item->hDC, TRANSPARENT);
	RoundRect(Item->hDC, Item->rcItem.left, Item->rcItem.top, Item->rcItem.right, Item->rcItem.bottom, 200, 200);
}

void DrawGroup(HWND hWnd, DRAWITEMSTRUCT *item)
{
	POINT pt;
	SIZE size;
	TCHAR buf[128];

	SelectObject(item->hDC, GetStockObject(DC_PEN));
	SetDCPenColor(item->hDC, RGB(0, 200, 0));

	//FillRect(item->hDC, &item->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));

	GetWindowText(item->hwndItem, buf, 128);
	GetTextExtentPoint32(item->hDC, buf, lstrlen(buf), &size);

	POINT border[] = { { item->rcItem.left + size.cx + 14, item->rcItem.top + size.cy - 1 }, // Starting point
	{ item->rcItem.right - 1, item->rcItem.top + size.cy - 1 },         // Top line
	{ item->rcItem.right - 1, item->rcItem.bottom - 1 },                // Right line
	{ item->rcItem.left, item->rcItem.bottom - 1 },                     // Bottom line
	{ item->rcItem.left, item->rcItem.top + size.cy },                  // Left line
	{ item->rcItem.left + 9, item->rcItem.top + size.cy } };            // Short top line

	MoveToEx(item->hDC, border[0].x, border[0].y, NULL);
	for (int i = 0; i < 6; ++i)
	{
		LineTo(item->hDC, border[i].x, border[i].y);
	}

	SetBkMode(item->hDC, TRANSPARENT);
	SetTextColor(item->hDC, RGB(0, 200, 0));
	TextOut(item->hDC, item->rcItem.left + 11, item->rcItem.top + size.cy / 2, buf, lstrlen(buf));

	DeleteObject(SelectObject(item->hDC, GetStockObject(BLACK_PEN)));

}

void HandleVirtualKeyboardMessage(HWND hWnd, WPARAM wParam);

//Main windows proc
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;

	switch (uMsg)
	{
	case WM_CREATE:	// Init interface
		// Text field
		CreateWindow(TEXT("EDIT"),
			NULL,
			WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL,
			KEYFIELD_POS_X, KEYFIELD_POS_Y, KEYFIELD_WIDTH, KEYFIELD_HEIGHT,
			hWnd,
			(HMENU)ID_KEY_FIELD,
			NULL,
			NULL);
		// Create keyboard controls
		{
			int startX = KEYBOARD_POS_X, startY = KEYBOARD_POS_Y;
			for (int i = 0; i < 16; i++)
			{
				CreateWindowEx(0, TEXT("BUTTON"),
					TEXT(KeyboardButtonNames[i]),
					WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
					startX, startY, NUM_KEY_WIDTH, NUM_KEY_HEIGHT,
					hWnd,
					(HMENU)KeyboardButtonIDs[i],
					GetModuleHandle(NULL),
					NULL);
				startX += NUM_KEY_WIDTH + NUM_KEY_PADDING;
			}
			// Clear
			CreateWindowEx(0, TEXT("BUTTON"),
				TEXT("Clear"),
				WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
				CLEAR_BUTTON_POS_X, CLEAR_BUTTON_POS_Y, CLEAR_BUTTON_WIDTH, CLEAR_BUTTON_HEIGHT,
				hWnd,
				(HMENU)ID_BTN_CLEAR,
				GetModuleHandle(NULL),
				NULL);
		}
		// Groupbox
		/*CreateWindowEx(WS_EX_TRANSPARENT, TEXT("BUTTON"), TEXT("Key"), WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
			5, 230, 523, 80, hWnd, (HMENU)ID_KEY_GROUP, GetModuleHandle(NULL),  NULL);*/
		// Button to enter the text
		CreateWindowEx(0, TEXT("BUTTON"), TEXT("Unlock"), WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
			UNLOCK_BUTTON_POS_X, UNLOCK_BUTTON_POS_Y, UNLOCK_BUTTON_WIDTH, UNLOCK_BUTTON_HEIGHT,
			hWnd, (HMENU)ID_KEY_ENTER, GetModuleHandle(NULL), NULL);
		break;
	case WM_COMMAND:
		HandleVirtualKeyboardMessage(hWnd, wParam);
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rect);
		EndPaint(hWnd, &ps);
		break;
	// Color text field
	case WM_CTLCOLOREDIT:
	{
		HDC hdc = (HDC)wParam;
		HWND hwnd = (HWND)lParam;
		if (GetDlgCtrlID(hwnd) == ID_KEY_FIELD)
		{
			SetTextColor(hdc, 0xFF00);
			SetBkColor(hdc, RGB(0x29, 0x29, 0x29));
			return (LRESULT)CreateSolidBrush(RGB(0x29, 0x29, 0x29));
		}
		break;
	}
	// Color statie elements (group box caption)
	/*case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, RGB(0x00, 0x8C, 0xBA));

		SetBkMode((HDC)wParam, TRANSPARENT);
		SetBkColor((HDC)wParam, RGB(0x14, 0x14, 0x14));

		return (LRESULT)CreateSolidBrush(GetBkColor((HDC)wParam));*/
	// Draw custom controls
	case WM_DRAWITEM:
	{
		DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT*)lParam;
		switch (dis->CtlID)
		{
		case ID_KEY_ENTER:
		case ID_BTN_ZERO:
		case ID_BTN_ONE:
		case ID_BTN_TWO:
		case ID_BTN_THREE:
		case ID_BTN_FOUR:
		case ID_BTN_FIVE:
		case ID_BTN_SIX:
		case ID_BTN_SEVEN:
		case ID_BTN_EIGHT:
		case ID_BTN_NINE:
		case ID_BTN_A:
		case ID_BTN_B:
		case ID_BTN_C:
		case ID_BTN_D:
		case ID_BTN_E:
		case ID_BTN_F:
		case ID_BTN_CLEAR:
			DrawButton(dis, GetDlgItem(hWnd, dis->CtlID));
			break;
		case ID_KEY_GROUP:
			DrawGroup(GetDlgItem(hWnd, dis->CtlID), dis);
			break;
		}
	}
	case WM_SIZE:
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void HandleVirtualKeyboardMessage(HWND hWnd, WPARAM wParam)
{
	char buffer[255];
	char currentText[255];

	switch (wParam)
	{
	case ID_BTN_ZERO:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%d", currentText, 0);
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_ONE:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%d", currentText, 1);
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_TWO:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%d", currentText, 2);
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_THREE:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%d", currentText, 3);
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_FOUR:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%d", currentText, 4);
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_FIVE:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%d", currentText, 5);
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_SIX:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%d", currentText, 6);
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_SEVEN:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%d", currentText, 7);
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_EIGHT:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%d", currentText, 8);
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_NINE:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%d", currentText, 9);
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_A:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%s", currentText, TEXT("A"));
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_B:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%s", currentText, TEXT("B"));
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_C:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%s", currentText, TEXT("C"));
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_D:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%s", currentText, TEXT("D"));
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_E:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%s", currentText, TEXT("E"));
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_F:
		GetDlgItemText(hWnd, ID_KEY_FIELD, currentText, 255);
		wsprintf(buffer, "%s%s", currentText, TEXT("F"));
		SetDlgItemText(hWnd, ID_KEY_FIELD, buffer);
		break;
	case ID_BTN_CLEAR:
		SetDlgItemText(hWnd, ID_KEY_FIELD, NULL);
	default:
		break;
	}
}

void ShowMainWindow(void)
{
	HWND hMainWnd;
	MSG msg;

	RegisterWndClass(WndProc, TEXT("Info"), COLOR_WINDOW);

	hMainWnd = CreateWindowEx(WS_EX_LAYERED, TEXT("Info"), TEXT("Info"),
		WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, 0, WIN_WIDTH, WIN_HEIGHT, (HWND)NULL, (HMENU)NULL,
		GetModuleHandle(NULL), NULL);
	SetWindowLong(hMainWnd, GWL_EXSTYLE, GetWindowLong(hMainWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hMainWnd, 0, (255 * 70) / 100, LWA_ALPHA);

	if (!hMainWnd)
	{
		MessageBox(NULL, TEXT("Can\'t create main window."), TEXT("Error"), MB_OK);
		return;
	}

	ShowWindow(hMainWnd, SW_SHOW);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}