// WinAPI.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WinAPI.h"
#include "windowsx.h"
#include "commdlg.h"
#include <time.h>
#include <string>
#include <thread>
#include <vector>
using namespace std;

#define MAX_LOADSTRING 100


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hSquare;
HMENU file_menu;                                //this is for the tick in pause in menu
int square_count;                               //max is 100
int width = GetSystemMetrics(SM_CXSCREEN) / 2; 
int height = GetSystemMetrics(SM_CYSCREEN) / 2;
static vector<HWND> squaresHWND;                 //vector holding handlers of child windows
static bool active = 1;                                 
static vector<char> characters;                  //vector holding letters from squares
int wrong = 0;
int missed = 0;
const int buffer_size = 256;
TCHAR buffer[buffer_size];

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	WndProcSquare(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINAPI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPI));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

BOOL CALLBACK DestroyChildCallback(
    HWND   hwnd,
    LPARAM lParam
)
{
    if (hwnd != NULL) {
        DestroyWindow(hwnd);
    }

    return TRUE;
}

//
//  FUNCTION: MyRegisterClass()
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex, square;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINAPI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    square.cbSize = sizeof(WNDCLASSEX);
    square.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    square.lpfnWndProc = WndProcSquare;
    square.cbClsExtra = 0;
    square.cbWndExtra = 0;
    square.hInstance = hInstance;
    square.hIcon = NULL;
    square.hCursor = LoadCursor(NULL, IDC_ARROW);
    square.hbrBackground = (HBRUSH)(COLOR_WINDOW + 3);
    square.lpszMenuName = NULL;
    square.lpszClassName = _T("Square");
    square.hIconSm = NULL;

    RegisterClassEx(&square);
    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//   PURPOSE: Saves instance handle and creates main window
//   COMMENTS:
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//

// WS_CLICKCHILDREN == Excludes the area occupied by child windows when drawing occurs within the parent window.

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, L"KeyboardMaster_WinAPI2021, Missed: 0, Wrong keys: 0",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME | WS_CLIPCHILDREN, (GetSystemMetrics(SM_CXSCREEN) - width) / 2,
        ((GetSystemMetrics(SM_CYSCREEN) - height) / 2), width, height, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return FALSE;

    file_menu = GetMenu(hWnd);

    SetWindowPos(hWnd, HWND_TOPMOST, (GetSystemMetrics(SM_CXSCREEN) - width) / 2,
        ((GetSystemMetrics(SM_CYSCREEN) - height) / 2), width, height, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//  PURPOSE: Processes messages for the main window.
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    srand(rand());
    RECT rc;
    int min = 0;            //height of lowest square
    HWND minHWND = nullptr; //handler of the lowest square

    switch (message)
    {
    //Pause at focus change, if on == 0 then the focus is outside the window and on pause
    case WM_ACTIVATEAPP:
    {
        if (active == 1) active = 0;
        else active = 1;
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case ID_FILE_PAUSE: {
            if (active == 1) {
                CheckMenuItem(file_menu, ID_FILE_PAUSE, MF_UNCHECKED);
                active = 0;
            }
            else {
                CheckMenuItem(file_menu, ID_FILE_PAUSE, MF_CHECKED);
                active = 1;
            }
            break;
        }
        case ID_FILE_NEWGAME:
            EnumChildWindows(hWnd, DestroyChildCallback, NULL); //enumerates child windows and destroys then
            wrong = 0;
            missed = 0;
            _stprintf_s(buffer, buffer_size, _T("KeyboardMaster_WinAPI2021, Missed: %d, Wrong keys: %d"), missed, wrong);
            SetWindowText(hWnd, buffer);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_CREATE:
        SetTimer(hWnd, 1, (300 + rand() % 701), nullptr); //time-out value is in milliseconds.

        break;
    case WM_CHAR:
    {
        for (int i = 0; i < characters.size(); i++)
        {
            if (characters.at(i) == (TCHAR)wParam)
            {
                GetWindowRect(squaresHWND.at(i), &rc); //Retrieves the dimensions of the bounding rectangle of the specified window.
                if (min < rc.bottom)
                {
                    minHWND = squaresHWND.at(i);
                    min = rc.bottom;
                }
            }
            if (minHWND != nullptr) DestroyWindow(minHWND);
        }

        if (min == 0) {
            wrong++;
            _stprintf_s(buffer, buffer_size, _T("KeyboardMaster_WinAPI2021, Missed: %d, Wrong keys: %d"), missed, wrong);
            SetWindowText(hWnd, buffer);
        }
    }
    break;
    case WM_TIMER:
    {
        if (square_count < MAX_LOADSTRING) {
            if (active == 0)
                hSquare = CreateWindow(_T("Square"), nullptr, WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,
                    rand() % (width - 50), 0, 25, 25, hWnd, NULL, hInst, NULL);
        }
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProcSquare(HWND hWndSquare, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rc;
    GetClientRect(hWndSquare, &rc);
    static TCHAR literki[2] = L"d";
    srand(rand());

    switch (message)
    {

    case WM_CREATE:
    {
        int v = 10 + rand() % 40;
        squaresHWND.push_back(hWndSquare);
        _stprintf_s(literki, 2, _T("%c"), 'a' + rand() % 26);
        characters.push_back(literki[0]);
        SetTimer(hWndSquare, 2, v, nullptr);
    }
    break;
    case WM_TIMER:
        if (wParam == 2)
        {
            GetWindowRect(hWndSquare, &rc);
            MapWindowPoints(HWND_DESKTOP, GetParent(hWndSquare), (LPPOINT)(&rc), 2); //squares only in main window not outside it 

            if (active == 0)
                MoveWindow(hWndSquare, rc.left, rc.top + 1, rc.right - rc.left, rc.bottom - rc.top, TRUE); //they fall if the game is on focus

            if (height < rc.bottom + 56) { //when the square hits the bottom
                missed++;
                _stprintf_s(buffer, buffer_size, _T("KeyboardMaster_WinAPI2021, Missed: %d, Wrong keys: %d"), missed, wrong);
                SetWindowText(GetParent(hWndSquare), buffer);
                DestroyWindow(hWndSquare);
            }
        }
        break;
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        break;
    case WM_PAINT: {
        HDC hdc = BeginPaint(hWndSquare, &ps);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, 0x00FFFFFF); //text has to be white
        DrawText(hdc, literki, (int)_tcslen(literki), &rc,DT_VCENTER | DT_CENTER | DT_SINGLELINE); 
        EndPaint(hWndSquare, &ps);
    }
    default:
        return DefWindowProc(hWndSquare, message, wParam, lParam);
    }
    return 0;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
