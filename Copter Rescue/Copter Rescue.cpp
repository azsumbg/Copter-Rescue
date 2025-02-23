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

bool hero_killed = false;

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
ID2D1Bitmap* bmpCivilL[46]{ nullptr };
ID2D1Bitmap* bmpCivilR[46]{ nullptr };
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

dll::Creature Copter;
dirs copter_draw_dir = dirs::right;

int good_ammo = 0;
int civils_needed = 0;
int civils_saved = 0;

std::vector<dll::Asset> vCivilians;
std::vector<dll::Asset> vGoodShots;
std::vector<dll::Asset> vBadShots;
std::vector<dll::Asset> vSupplies;

std::vector<dll::Creature> vEvils;

std::vector<EXPLOSION> vExplosions;

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
    for (int i = 0; i < 46; ++i)if (!ClearMem(&bmpCivilR[i]))LogErr(L"Error clearing bmpCivil");
    for (int i = 0; i < 46; ++i)if (!ClearMem(&bmpCivilL[i]))LogErr(L"Error clearing bmpCivil");
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
    good_ammo = 2;
    civils_needed = 10;
    civils_saved = 0;

    wcscpy_s(current_player, L"One Player");
    name_set = false;
    ///////////////////////////////////////

    ClearMem(&Copter);
    Copter = dll::CreatureFactory(hero, static_cast<float>(Rand(20, 700)), 70.0f);

    if (!vCivilians.empty())
        for (int i = 0; i < vCivilians.size(); ++i)ClearMem(&vCivilians[i]);
    vCivilians.clear();

    if (!vGoodShots.empty())
        for (int i = 0; i < vGoodShots.size(); ++i)ClearMem(&vGoodShots[i]);
    vGoodShots.clear();

    if (!vBadShots.empty())
        for (int i = 0; i < vBadShots.size(); ++i)ClearMem(&vBadShots[i]);
    vBadShots.clear();

    if (!vSupplies.empty())
        for (int i = 0; i < vSupplies.size(); ++i)ClearMem(&vSupplies[i]);
    vSupplies.clear();

    if (!vEvils.empty())
        for (int i = 0; i < vEvils.size(); ++i)ClearMem(&vEvils[i]);
    vEvils.clear();
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

    case WM_KEYDOWN:
        if(Copter)
            switch (wParam)
            {
            case VK_LEFT:
                Copter->dir = dirs::left;
                copter_draw_dir = dirs::left;
                break;

            case VK_RIGHT:
                Copter->dir = dirs::right;
                copter_draw_dir = dirs::right;
                break;

            case VK_UP:
                Copter->dir = dirs::up;
                break;

            case VK_DOWN:
                Copter->dir = dirs::down;
                break;

            case VK_SPACE:
                Copter->dir = dirs::stop;
                break;

            case VK_SHIFT:
                if (good_ammo > 0)
                {
                    --good_ammo;
                    switch (Copter->dir)
                    {
                    case dirs::right:
                        vGoodShots.push_back(dll::ObjectFactory(bullet, Copter->center.x, Copter->center.y,
                            Copter->center.x - 30.0f, ground));
                        break;

                    case dirs::left:
                        vGoodShots.push_back(dll::ObjectFactory(bullet, Copter->center.x, Copter->center.y,
                            Copter->center.x + 30.0f, ground));
                        break;

                    case dirs::stop:
                        vGoodShots.push_back(dll::ObjectFactory(bullet, Copter->center.x, Copter->center.y,
                            Copter->center.x, ground));
                        break;
                    }
                    if (sound)mciSendString(L"play .\\res\\snd\\shot.wav", NULL, NULL, NULL);
                }
                else if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
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
                wchar_t name[100] = L".\\res\\img\\civil\\l\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                
                bmpCivilL[i] = Load(name, Draw);
                if (!bmpCivilL[i])
                {
                    LogErr(L"Error loading bmpCivilL !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 46; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\civil\\r\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpCivilR[i] = Load(name, Draw);
                if (!bmpCivilR[i])
                {
                    LogErr(L"Error loading bmpCivilR !");
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

        if (Copter)Copter->Move((float)(level));

        if (vCivilians.size() < level + 2 && Rand(0, 500) == 66)vCivilians.push_back(dll::ObjectFactory(civilian,
            static_cast<float>(Rand(0, 750)), static_cast<float>(Rand((int)(ground - 150.0f), (int)(ground - 60.0f)))));

        if (!vCivilians.empty() && Rand(0, 10) == 6)
        {
            int which = Rand(0, (int)(vCivilians.size()) - 1);
            vCivilians[which]->Move((float)(level));
        }

        if (vEvils.size() < level + 2 && Rand(0, 500) == 66)
        {
            switch (Rand(0, 2))
            {
            case 0:
                vEvils.push_back(dll::CreatureFactory(evil1,
                    static_cast<float>(Rand(0, 650)), static_cast<float>(Rand((int)(ground - 150.0f), (int)(ground - 48.0f)))));
                break;

            case 1:
                vEvils.push_back(dll::CreatureFactory(evil2,
                    static_cast<float>(Rand(0, 700)), static_cast<float>(Rand((int)(ground - 150.0f), (int)(ground - 90.0f)))));
                break;

            case 2:
                vEvils.push_back(dll::CreatureFactory(evil3,
                    static_cast<float>(Rand(0, 700)), static_cast<float>(Rand((int)(ground - 150.0f), (int)(ground - 90.0f)))));
                break;
            }
        }

        if (Rand(0, 5000) == 666 && vSupplies.size() < level + 2)
            vSupplies.push_back(dll::ObjectFactory(supply, (float)(Rand(0, (int)(scr_width - 50.0f))), 0));       
        
        if (!vSupplies.empty())
        {
            for (std::vector<dll::Asset>::iterator sup = vSupplies.begin(); sup < vSupplies.end(); ++sup)
            {
                if (!(*sup)->Move((float)(level)))
                {
                    (*sup)->Release();
                    vSupplies.erase(sup);
                    break;
                }
            }
        }

        if (!vEvils.empty() && Copter && !vCivilians.empty())
        {
            for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                FPOINT CopterPos{ Copter->start.x,Copter->start.y };
                cont::CONTAINER<FPOINT>CivsPos(vCivilians.size());
                for (int i = 0; i < vCivilians.size(); i++)CivsPos.push_back(FPOINT(vCivilians[i]->start.x, 
                    vCivilians[i]->start.y));

                if ((*evil)->AIShoot(CivsPos, FPOINT(Copter->start.x, Copter->start.y)))
                {
                    vBadShots.push_back(dll::ObjectFactory(bullet, (*evil)->center.x, (*evil)->center.y, Copter->center.x,
                        Copter->center.y));
                    if (sound)mciSendString(L"play .\\res\\snd\\shot.wav", NULL, NULL, NULL);

                }
                else (*evil)->Move((float)(level));
            }
        }
        else if (!vEvils.empty() && Copter)
        {
            for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                if (Rand(0, 500) == 66)
                {
                    vBadShots.push_back(dll::ObjectFactory(bullet, (*evil)->center.x, (*evil)->center.y, Copter->center.x,
                        Copter->center.y));
                    if (sound)mciSendString(L"play .\\res\\snd\\shot.wav", NULL, NULL, NULL);
                }
            }
        }

        if (!vBadShots.empty())
        {
            for (std::vector<dll::Asset>::iterator shot = vBadShots.begin(); shot < vBadShots.end(); shot++)
            {
                if (!(*shot)->Move((float)(level)))
                {
                    (*shot)->Release();
                    vBadShots.erase(shot);
                    break;
                }
            }
        }

        if (!vGoodShots.empty())
        {
            for (std::vector<dll::Asset>::iterator shot = vGoodShots.begin(); shot < vGoodShots.end(); shot++)
            {
                if (!(*shot)->Move((float)(level)))
                {
                    (*shot)->Release();
                    vGoodShots.erase(shot);
                    break;
                }
            }
        }

        if (!vCivilians.empty() && Copter)
        {
            for (std::vector<dll::Asset>::iterator civ = vCivilians.begin(); civ < vCivilians.end(); ++civ)
            {
                if (!(Copter->start.x >= (*civ)->end.x || Copter->end.x <= (*civ)->start.x
                    || Copter->start.y >= (*civ)->end.y || Copter->end.y <= (*civ)->start.y))
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\civsaved.wav", NULL, NULL, NULL);
                    (*civ)->Release();
                    vCivilians.erase(civ);
                    ++civils_saved;
                    break;
                }
            }
        }

        if (!vCivilians.empty() && !vEvils.empty())
        {
            bool killed = false;

            for (std::vector<dll::Asset>::iterator civ = vCivilians.begin(); civ < vCivilians.end(); ++civ)
            {
                for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
                {
                    if (!((*evil)->start.x >= (*civ)->end.x || (*evil)->end.x <= (*civ)->start.x
                        || (*evil)->start.y >= (*civ)->end.y || (*evil)->end.y <= (*civ)->start.y))
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\civkilled.wav", NULL, NULL, NULL);
                        (*civ)->Release();
                        vCivilians.erase(civ);
                        (*evil)->lifes -= 2;
                        if ((*evil)->lifes <= 0)
                        {
                            if (sound)mciSendString(L"play .\\res\\snd\\explosion.wav", NULL, NULL, NULL);
                            score += 10 + level;
                            vExplosions.push_back(EXPLOSION{ (*evil)->center.x, (*evil)->center.y });
                            (*evil)->Release();
                            vEvils.erase(evil);
                            killed = true;
                        }
                        killed = true;
                        break;
                    }
                }
                if (killed) break;
            }
        }

        // SHOOT TO KILL ******
        
        if (!vGoodShots.empty() && !vEvils.empty())
        {
            bool killed = false;

            for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                for (std::vector<dll::Asset>::iterator shot = vGoodShots.begin(); shot < vGoodShots.end(); ++shot)
                {
                    if (!((*evil)->start.x >= (*shot)->end.x || (*evil)->end.x <= (*shot)->start.x
                        || (*evil)->start.y >= (*shot)->end.y || (*evil)->end.y <= (*shot)->start.y))
                    {
                        (*shot)->Release();
                        vGoodShots.erase(shot);
                        (*evil)->lifes -= 20;
                        if ((*evil)->lifes <= 0)
                        {
                            if (sound)mciSendString(L"play .\\res\\snd\\explosion.wav", NULL, NULL, NULL);
                            score += 10 + level;
                            vExplosions.push_back(EXPLOSION{ (*evil)->center.x, (*evil)->center.y });
                            (*evil)->Release();
                            vEvils.erase(evil);
                            killed = true;
                        }
                        break;
                    }
                }

                if (killed)break;
            }
        }

        if (!vEvils.empty() && Copter)
        {
            for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                if (!((*evil)->start.x >= Copter->end.x || (*evil)->end.x <= Copter->start.x
                    || (*evil)->start.y >= Copter->end.y || (*evil)->end.y <= Copter->start.y))
                {
                    vExplosions.push_back(EXPLOSION{ (*evil)->start.x,(*evil)->start.y });
                    vExplosions.push_back(EXPLOSION{ Copter->start.x,Copter->start.y });
                    if (sound)mciSendString(L"play .\\res\\snd\\explosion.wav", NULL, NULL, NULL);
                    (*evil)->Release();
                    vEvils.erase(evil);
                    ClearMem(&Copter);
                    hero_killed = true;
                    break;
                }
            }
        }

        if (!vBadShots.empty() && Copter)
        {
            for (std::vector<dll::Asset>::iterator shot = vBadShots.begin(); shot < vBadShots.end(); ++shot)
            {
                if (!((*shot)->start.x <= Copter->end.x || (*shot)->end.x <= Copter->start.x
                    || (*shot)->start.y <= Copter->end.y || (*shot)->end.y <= Copter->start.y))
                {
                    Copter->lifes -= 10;
                    (*shot)->Release();
                    vBadShots.erase(shot);
                    if (Copter->lifes <= 0)
                    {
                        vExplosions.push_back(EXPLOSION{ Copter->start.x,Copter->start.y });
                        if (sound)mciSendString(L"play .\\res\\snd\\explosion.wav", NULL, NULL, NULL);
                        ClearMem(&Copter);
                        hero_killed = true;
                    }
                    break;
                }
            }
        }


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

        if (Copter)
        {
            int current_frame = Copter->GetFrame();

            switch (copter_draw_dir)
            {
            case dirs::right:
                Draw->DrawBitmap(bmpHeroR[current_frame], Resizer(bmpHeroR[current_frame], Copter->start.x, Copter->start.y));
                break;

            case dirs::left:
                Draw->DrawBitmap(bmpHeroL[current_frame], Resizer(bmpHeroL[current_frame], Copter->start.x, Copter->start.y));
                break;
            }

            Draw->DrawLine(D2D1::Point2F(Copter->start.x + 10.0f, Copter->end.y + 10.0f),
                D2D1::Point2F(Copter->start.x + (float)(Copter->lifes) + 15.0f, Copter->end.y + 12.0f), hgltBrush, 8.0f);
        }

        if (!vCivilians.empty())
        {
            for (int i = 0; i < vCivilians.size(); ++i)
            {
                int aframe = vCivilians[i]->GetFrame();

                if (vCivilians[i]->dir == dirs::left)
                    Draw->DrawBitmap(bmpCivilR[aframe], Resizer(bmpCivilR[aframe], vCivilians[i]->start.x,
                        vCivilians[i]->start.y));
                else
                    Draw->DrawBitmap(bmpCivilL[aframe], Resizer(bmpCivilL[aframe], vCivilians[i]->start.x,
                        vCivilians[i]->start.y));
            }
        }

        if (!vEvils.empty())
        {
            for (int i = 0; i < vEvils.size(); ++i)
            {
                int aframe = vEvils[i]->GetFrame();

                switch (vEvils[i]->GetType())
                {
                case evil1:
                    Draw->DrawBitmap(bmpEvil1[aframe], Resizer(bmpEvil1[aframe], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case evil2:
                    Draw->DrawBitmap(bmpEvil2[aframe], Resizer(bmpEvil2[aframe], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case evil3:
                    Draw->DrawBitmap(bmpEvil3[aframe], Resizer(bmpEvil3[aframe], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;
                }

                Draw->DrawLine(D2D1::Point2F(vEvils[i]->start.x, vEvils[i]->end.y + 10.0f),
                    D2D1::Point2F(vEvils[i]->start.x + (float)(vEvils[i]->lifes) + 40.0f, vEvils[i]->end.y + 10.0f), 
                    hgltBrush, 8.0f);
            }

           
        }

        if (!vSupplies.empty())
        {
            for (int i = 0; i < vSupplies.size(); ++i)
            {
                int aframe = vSupplies[i]->GetFrame();

                Draw->DrawBitmap(bmpSupply[aframe], Resizer(bmpSupply[aframe], vSupplies[i]->start.x, vSupplies[i]->start.y));
                
            }
        }

        if (!vBadShots.empty())
            for (int i = 0; i < vBadShots.size(); ++i)Draw->DrawBitmap(bmpBullet, D2D1::RectF(vBadShots[i]->start.x,
                vBadShots[i]->start.y, vBadShots[i]->end.x, vBadShots[i]->end.y));

        if (!vGoodShots.empty())
            for (int i = 0; i < vGoodShots.size(); ++i)Draw->DrawBitmap(bmpBullet, D2D1::RectF(vGoodShots[i]->start.x,
                vGoodShots[i]->start.y, vGoodShots[i]->end.x, vGoodShots[i]->end.y));

        if (!vExplosions.empty())
        {
            for (std::vector<EXPLOSION>::iterator expl = vExplosions.begin(); expl < vExplosions.end(); expl++)
            {
                expl->frame_delay--;
                if (expl->frame_delay < 0)
                {
                    expl->frame_delay = 3;
                    ++expl->frame;
                    if (expl->frame > 23)
                    {
                        Draw->DrawBitmap(bmpExplosion[23], Resizer(bmpExplosion[23], 
                            expl->where.x, expl->where.y));
                        Draw->EndDraw();
                        vExplosions.erase(expl);
                        if (hero_killed)
                        {
                            if (sound)
                            {
                                PlaySound(NULL, NULL, NULL);
                                PlaySound(L".\\res\\snd\\killed.wav", NULL, SND_SYNC);
                            }
                            GameOver();
                        }
                        break;
                    }
                }
                Draw->DrawBitmap(bmpExplosion[expl->frame], Resizer(bmpExplosion[expl->frame], expl->where.x, expl->where.y));
            }
        }


    /////////////////////////////////////////////////////////////////
        Draw->EndDraw();
    }

    ClearResources();
    std::remove(temp_file);

    return (int) bMsg.wParam;
}