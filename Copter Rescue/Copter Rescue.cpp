#include "framework.h"
#include "Copter Rescue.h"
#include <mmsystem.h>
#include "HeapIt.h"
#include <d2d1.h>
#include <dwrite.h>
#include "D2BMPLOADER.h"
#include "ErrH.h"
#include "gifresizer.h"
#include "FCheck.h"
#include "gaming.h"
#include <chrono>
#include <vector>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2bmploader.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "gifresizer.lib")
#pragma comment(lib, "fcheck.lib")
#pragma comment(lib, "gaming.lib")

constexpr wchar_t bWinClassName[]{ L"RescueTeam" };

constexpr char temp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltemp_file[] { L".\\res\\data\\temp.dat" };
constexpr wchar_t sound_file[]{ L".\\res\\snd\\main.wav" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };
constexpr wchar_t record_file[]{ L".\\res\\data\\record.dat" };

constexpr int mNew{ 1001 };
constexpr int mLvl{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int no_record{ 2001 };
constexpr int first_record{ 2002 };
constexpr int record{ 2003 };


WNDCLASS bWin{};
HINSTANCE bIns{ nullptr };
HICON bIcon{ nullptr };
HCURSOR mainCur{ nullptr };
HCURSOR outCur{ nullptr };
POINT cur_pos{};
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
HWND bHwnd{ nullptr };
HDC PaintDC{ nullptr };
PAINTSTRUCT bPaint{};
MSG bMsg{};
BOOL bRet{ 0 };
UINT bTimer{ 0 };

wchar_t current_player[16]{ L"One Player" };

dll::RandIt Rand{};

bool pause = false;
bool in_client = true;
bool show_help = false;
bool name_set = false;
bool sound = true;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;

int field_frame = 0;
int field_delay = 5;

struct EXPLOSION
{
    FPOINT where{};
    int frame = 0;
    int frame_delay = 3;
};

int level = 1;
int score = 0;
int mins = 0;
int secs = 180;

D2D1_RECT_F b1Rect{ 20.0f, 5.0f, scr_width / 3 - 30.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_width / 3 + 20.0f, 5.0f, scr_width * 2 / 3 - 30.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_width * 2 / 3 + 20.0f, 5.0f, scr_width - 30.0f, 50.0f };

D2D1_RECT_F b1TxtRect{ 40.0f, 10.0f, scr_width / 3 - 30.0f, 50.0f };
D2D1_RECT_F b2TxtRect{ scr_width / 3 + 40.0f, 10.0f, scr_width * 2 / 3 - 30.0f, 50.0f };
D2D1_RECT_F b3TxtRect{ scr_width * 2 / 3 + 40.0f, 10.0f, scr_width - 30.0f, 50.0f };

///////////////////////////////////////////

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };

ID2D1RadialGradientBrush* b1BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b2BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b3BckgBrush{ nullptr };

ID2D1SolidColorBrush* textBrush{ nullptr };
ID2D1SolidColorBrush* hgltBrush{ nullptr };
ID2D1SolidColorBrush* inactBrush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* txtFormat{ nullptr };
IDWriteTextFormat* midFormat{ nullptr };
IDWriteTextFormat* bigFormat{ nullptr };

ID2D1Bitmap* bmpBullet{ nullptr };
ID2D1Bitmap* bmpCivil[46]{ nullptr };
ID2D1Bitmap* bmpEvil1[12]{ nullptr };
ID2D1Bitmap* bmpEvil2[4]{ nullptr };
ID2D1Bitmap* bmpEvil3[4]{ nullptr };
ID2D1Bitmap* bmpExplosion[24]{ nullptr };
ID2D1Bitmap* bmpField[32]{ nullptr };
ID2D1Bitmap* bmpHeroL[3]{ nullptr };
ID2D1Bitmap* bmpHeroR[3]{ nullptr };
ID2D1Bitmap* bmpIntro[8]{ nullptr };
ID2D1Bitmap* bmpSupply[27]{ nullptr };

///////////////////////////////////////////

template<typename T>concept HasRelease = requires(T var)
{
    var.Release();
};
template<HasRelease U>bool ClearMem(U** var)
{
    if (*var)
    {
        (*var)->Release();
        (*var) = nullptr;
        return true;
    }

    return false;
}
void LogErr(LPCWSTR what)
{
    std::wofstream err(L".\\res\\data\\error.log", std::ios::app);
    err << what << L", time of occurence: " << std::chrono::system_clock::now() << std::endl;
    err.close();
}
void ClearResources()
{
    if (!ClearMem(&iFactory))LogErr(L"Error clearing iFactory");
    if (!ClearMem(&Draw))LogErr(L"Error clearing HwndRenderTarget");
    if (!ClearMem(&b1BckgBrush))LogErr(L"Error clearing b1BckgBrush");
    if (!ClearMem(&b2BckgBrush))LogErr(L"Error clearing b2BckgBrush");
    if (!ClearMem(&b3BckgBrush))LogErr(L"Error clearing b3BckgBrush");
    if (!ClearMem(&textBrush))LogErr(L"Error clearing textBrush");
    if (!ClearMem(&hgltBrush))LogErr(L"Error clearing hgltBrush");
    if (!ClearMem(&inactBrush))LogErr(L"Error clearing inatBrush");
    if (!ClearMem(&iWriteFactory))LogErr(L"Error clearing iWriteFactory");
    if (!ClearMem(&txtFormat))LogErr(L"Error clearing txtFormat");
    if (!ClearMem(&midFormat))LogErr(L"Error clearing midFormat");
    if (!ClearMem(&bigFormat))LogErr(L"Error clearing bigFormat");

    if (!ClearMem(&bmpBullet))LogErr(L"Error clearing bmpBullet");
    for (int i = 0; i < 46; ++i)if (!ClearMem(&bmpCivil[i]))LogErr(L"Error clearing bmpCivil");
    for (int i = 0; i < 12; ++i)if (!ClearMem(&bmpEvil1[i]))LogErr(L"Error clearing bmpEvil1");
    for (int i = 0; i < 4; ++i)if (!ClearMem(&bmpEvil2[i]))LogErr(L"Error clearing bmpEvil2");
    for (int i = 0; i < 4; ++i)if (!ClearMem(&bmpEvil3[i]))LogErr(L"Error clearing bmpEvil3");
    for (int i = 0; i < 3; ++i)if (!ClearMem(&bmpHeroL[i]))LogErr(L"Error clearing bmpHeroL");
    for (int i = 0; i < 3; ++i)if (!ClearMem(&bmpHeroR[i]))LogErr(L"Error clearing bmpHeroR");
    for (int i = 0; i < 24; ++i)if (!ClearMem(&bmpExplosion[i]))LogErr(L"Error clearing bmpExplosion");
    for (int i = 0; i < 32; ++i)if (!ClearMem(&bmpField[i]))LogErr(L"Error clearing bmpField");
    for (int i = 0; i < 8; ++i)if (!ClearMem(&bmpIntro[i]))LogErr(L"Error clearing bmpIntro");
    for (int i = 0; i < 27; ++i)if (!ClearMem(&bmpSupply[i]))LogErr(L"Error clearing bmpSupply");
}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"КРИТИЧНА ГРЕШКА !!!", MB_OK | MB_APPLMODAL | MB_ICONERROR);
    
    ClearResources();
    std::remove(temp_file);
    exit(1);
}
void InitGame()
{
    level = 1;
    score = 0;
    mins = 0;
    secs = 180;

    wcscpy_s(current_player, L"One Player");
    name_set = false;
    ///////////////////////////////////////




}

void GameOver()
{
    PlaySound(NULL, NULL, NULL);
    KillTimer(bHwnd, bTimer);


    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}

//////////////////////////////////////////

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(bIcon));
        return true;

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDOK:
            if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
            {
                wcscpy_s(current_player, L"One Player");

                if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
                MessageBox(bHwnd, L"Ха, ха, ха ! Забрави си името", L"Забраватор", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
                EndDialog(hwnd, IDCANCEL);
                break;
            }
            EndDialog(hwnd, IDOK);
            break;
        }
        break;
    }

    return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
        bBar = CreateMenu();
        if (bBar)
        {
            bMain = CreateMenu();
            bStore = CreateMenu();

            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

            AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
            AppendMenu(bMain, MF_STRING, mLvl, L"Следващо ниво");
            AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bMain, MF_STRING, mExit, L"Изход");

            AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
            AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
            AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");
            SetMenu(hwnd, bBar);
            SetTimer(hwnd, bTimer, 1000, NULL);
            InitGame();
        }
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(hwnd, L"Ако излезеш, губиш прогреса по тази игра !\n\nНаистина ли излизаш ?", L"Изход",
            MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(10, 10, 10)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cur_pos);
        ScreenToClient(hwnd, &cur_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                pause = false;
                in_client = true;
            }

            if (cur_pos.y <= 50)
            {
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = true;
                        b2Hglt = false;
                        b3Hglt = false;
                    }
                }
                else if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = true;
                        b3Hglt = false;
                    }
                }
                else if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = false;
                        b3Hglt = true;
                    }
                }

                SetCursor(outCur);
                return true;
            }
            else if (b1Hglt||b2Hglt|| b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }

            SetCursor(mainCur);
            return true;
        }
        else
        {
            if (in_client)
            {
                pause = true;
                in_client = false;
            }

            if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }

            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return true;
        }
        break;

    case WM_TIMER:
        if (pause)break;
        secs--;
        mins = secs / 60;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case mNew:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако рестартираш, губиш прогреса по тази игра !\n\nНаистина ли рестартираш ?", L"Рестарт",
                MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            break;

        case mLvl:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ще загубиш прогреса по тази игра !\n\nНаистина ли минаваш на горно ниво ?", L"Прескочи ниво",
                MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            //LevelUp();
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;


        }
        break;





    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void CreateResources()
{
    int win_x = (int)(GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2.0f));
    if (GetSystemMetrics(SM_CXSCREEN) < win_x + (int)(scr_width) || GetSystemMetrics(SM_CYSCREEN) < (int)(scr_height) + 50)
        ErrExit(eScreen);

    bIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 255, LR_LOADFROMFILE));
    if (!bIcon)ErrExit(eIcon);
    mainCur = LoadCursorFromFile(L".\\res\\main.ani");
    outCur = LoadCursorFromFile(L".\\res\\out.ani");
    if (!mainCur || !outCur)ErrExit(eCursor);

    bWin.lpszClassName = bWinClassName;
    bWin.hInstance = bIns;
    bWin.lpfnWndProc = &WinProc;
    bWin.hbrBackground = CreateSolidBrush(RGB(10, 10, 10));
    bWin.hIcon = bIcon;
    bWin.hCursor = mainCur;
    bWin.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWin))ErrExit(eClass);

    bHwnd = CreateWindow(bWinClassName, L"МИСИЯ - СПАСЯВАНЕ", WS_CAPTION | WS_SYSMENU, win_x, 50, (int)(scr_width),
        (int)(scr_height), NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else
    {
        ShowWindow(bHwnd, SW_SHOWDEFAULT);

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
        if (hr != S_OK)
        {
            LogErr(L"Error creating D2D1Factory !");
            ErrExit(eD2D);
        }

        if (iFactory)
            hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
                D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
        if (hr != S_OK)
        {
            LogErr(L"Error creating D2D1HwndRenderTarget !");
            ErrExit(eD2D);
        }
        
        if (Draw)
        {
            D2D1_GRADIENT_STOP gStops[2]{};
            ID2D1GradientStopCollection* gColl = nullptr;
            gStops[0].position = 0;
            gStops[0].color = D2D1::ColorF(D2D1::ColorF::MediumSpringGreen);
            gStops[1].position = 1.0f;
            gStops[1].color = D2D1::ColorF(D2D1::ColorF::DarkOliveGreen);

            hr = Draw->CreateGradientStopCollection(gStops, 2, &gColl);

            if (hr != S_OK)
            {
                LogErr(L"Error creating D2D1GradientStopCollection !");
                ErrExit(eD2D);
            }

            if (gColl)
            {
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b1Rect.left
                    + (b1Rect.right - b1Rect.left) / 2), D2D1::Point2F(0, 0), (b1Rect.right - b1Rect.left) / 2, 22.5f), gColl,
                    &b1BckgBrush);
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b2Rect.left
                    + (b2Rect.right - b2Rect.left) / 2), D2D1::Point2F(0, 0), (b2Rect.right - b2Rect.left) / 2, 22.5f), gColl,
                    &b2BckgBrush);
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b3Rect.left
                    + (b3Rect.right - b3Rect.left) / 2), D2D1::Point2F(0, 0), (b3Rect.right - b3Rect.left) / 2, 22.5f), gColl,
                    &b3BckgBrush);
                if (hr != S_OK)
                {
                    LogErr(L"Error creating D2D1RadialGradientBrushes !");
                    ClearMem(&gColl);
                    ErrExit(eD2D);
                }

                ClearMem(&gColl);
            }

            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Maroon), &textBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &hgltBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSalmon), &inactBrush);
            
            if (hr != S_OK)
            {
                LogErr(L"Error creating D2D1SolidColorBrushes !");
                ErrExit(eD2D);
            }

            bmpBullet = Load(L".\\res\\img\\bullet.png", Draw);
            if (!bmpBullet)
            {
                LogErr(L"Error loading bmpBullet !");
                ErrExit(eD2D);
            }
            for (int i = 0; i < 46; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\civil\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                
                bmpCivil[i] = Load(name, Draw);
                if (!bmpCivil[i])
                {
                    LogErr(L"Error loading bmpCivil !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 12; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil1\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil1[i] = Load(name, Draw);
                if (!bmpEvil1[i])
                {
                    LogErr(L"Error loading bmpEvil1 !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 4; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil2\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil2[i] = Load(name, Draw);
                if (!bmpEvil2[i])
                {
                    LogErr(L"Error loading bmpEvil2 !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 4; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil3\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil3[i] = Load(name, Draw);
                if (!bmpEvil3[i])
                {
                    LogErr(L"Error loading bmpEvil3 !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\explosion\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpExplosion[i] = Load(name, Draw);
                if (!bmpExplosion[i])
                {
                    LogErr(L"Error loading bmpExplosion !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 32; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\field\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpField[i] = Load(name, Draw);
                if (!bmpField[i])
                {
                    LogErr(L"Error loading bmpField !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 3; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\hero\\l\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpHeroL[i] = Load(name, Draw);
                if (!bmpHeroL[i])
                {
                    LogErr(L"Error loading bmpHeroL !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 3; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\hero\\r\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpHeroR[i] = Load(name, Draw);
                if (!bmpHeroR[i])
                {
                    LogErr(L"Error loading bmpHeroR !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 8; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\intro\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpIntro[i] = Load(name, Draw);
                if (!bmpIntro[i])
                {
                    LogErr(L"Error loading bmpIntro !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 27; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\supply\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpSupply[i] = Load(name, Draw);
                if (!bmpSupply[i])
                {
                    LogErr(L"Error loading bmpSupply !");
                    ErrExit(eD2D);
                }
            }
        }

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), 
            reinterpret_cast<IUnknown**>(&iWriteFactory));
        if (hr != S_OK)
        {
            LogErr(L"Error creating D2D1WriteFactory !");
            ErrExit(eD2D);
        }

        if (iWriteFactory)
        {
            hr = iWriteFactory->CreateTextFormat(L"SEGOE PRINT", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"", &txtFormat);
            hr = iWriteFactory->CreateTextFormat(L"SEGOE PRINT", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 32.0f, L"", &midFormat);
            hr = iWriteFactory->CreateTextFormat(L"SEGOE PRINT", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 72.0f, L"", &bigFormat);

            if (hr != S_OK)
            {
                LogErr(L"Error creating D2D1 text formats !");
                ErrExit(eD2D);
            }
        }
    }

    int intro_frame = 0;
    wchar_t init_txt[40]{ L"СПАСЯВАНЕ НА УЧЕНИ !\n\n    dev. Daniel !" };
    wchar_t show_txt[40]{ L"\0" };

    if (Draw && hgltBrush && bigFormat)
    {
        mciSendString(L"play .\\res\\snd\\morse.wav", NULL, NULL, NULL);
        
        for (int i = 0; i < 40; ++i)
        {
            Draw->BeginDraw();
            Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
            ++intro_frame;
            if (intro_frame > 7)intro_frame = 0;
            show_txt[i] = init_txt[i];
            Draw->DrawTextW(show_txt, i, bigFormat, D2D1::RectF(100.0f, 50.0f, scr_width, scr_height), hgltBrush);
            Draw->EndDraw();
            Sleep(120);
        }
        PlaySound(L".\\res\\snd\\boom.wav", NULL, SND_SYNC);
    }
}

/////////////////////////////////////////


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)
    {
        LogErr(L"Error in wWinMain hInstance parameter !");
        ErrExit(eClass);
    }

    CreateResources();

    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);
            TranslateMessage(&bMsg);
            DispatchMessageW(&bMsg);
        }

        if (pause)
        {
            if (show_help)continue;
            Draw->BeginDraw();
            Draw->DrawBitmap(bmpIntro[0], D2D1::RectF(0, 0, scr_width, scr_height));
            if (bigFormat && hgltBrush)
                Draw->DrawTextW(L"ПАУЗА", 6, bigFormat, D2D1::RectF(scr_width / 2 - 100.0f, scr_height / 2 - 50.0f, scr_width,
                    scr_height), hgltBrush);
            Draw->EndDraw();
            continue;
        }
    ///////////////////////////////////////////////////////////////














    // DRAW THINGS *************************************************
        Draw->BeginDraw();
        Draw->DrawBitmap(bmpField[field_frame], D2D1::RectF(0, 0, scr_width, scr_height));
        --field_delay;
        if (field_delay <= 0)
        {
            field_delay = 5;
            ++field_frame;
            if (field_frame > 31)field_frame = 0;
        }
        if (textBrush && hgltBrush && inactBrush && txtFormat && b1BckgBrush && b2BckgBrush && b3BckgBrush)
        {
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b1Rect, 10.0f, 15.0f), b1BckgBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b2Rect, 10.0f, 15.0f), b2BckgBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b3Rect, 10.0f, 15.0f), b3BckgBrush);

            if (name_set)
                Draw->DrawTextW(L"ИМЕ НА ПИЛОТ", 13, txtFormat, b1TxtRect, inactBrush);
            else
            {
                if (!b1Hglt)Draw->DrawTextW(L"ИМЕ НА ПИЛОТ", 13, txtFormat, b1TxtRect, textBrush);
                else Draw->DrawTextW(L"ИМЕ НА ПИЛОТ", 13, txtFormat, b1TxtRect, hgltBrush);
            }
            if (!b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, txtFormat, b2TxtRect, textBrush);
            else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, txtFormat, b2TxtRect, hgltBrush);
            if (!b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, txtFormat, b3TxtRect, textBrush);
            else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, txtFormat, b3TxtRect, hgltBrush);

        }
        //////////////////////////////////////////////////////////////








    /////////////////////////////////////////////////////////////////
        Draw->EndDraw();
    }

    ClearResources();
    std::remove(temp_file);

    return (int) bMsg.wParam;
}