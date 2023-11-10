#include "framework.h"
#include "GRAB.h"
#include "resource.h"
#include <ctime>
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include <vector>
#include <fstream>
#include "ErrH.h"
#include "D2BMPLOADER.h"
#include "FCheck.h"
#include "ofactory.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "d2bmploader.lib")
#pragma comment(lib, "fcheck.lib")
#pragma comment(lib, "ofactory.lib")

#define bWinClassName L"VeronikaGrabIt"

#define scr_width 816
#define scr_height 639

#define temp_file ".\\res\\data\\temp.dat"
#define Ltemp_file L".\\res\\data\\temp.dat"
#define record_file L".\\res\\data\\record.dat"
#define save_file L".\\res\\data\\save.dat"
#define help_file L".\\res\\data\\help.dat"
#define sound_file L".\\res\\snd\\main.wav"

#define mNew 1001
#define mExit 1002
#define mSave 1003
#define mLoad 1004
#define mHoF 1005

#define record 2001
#define no_record 2002
#define first_record 2003
////////////////////////////////////////////////////////////////////

WNDCLASS bWinClass;
HINSTANCE bIns = nullptr;
HWND bHwnd = nullptr;
HMENU bBar = nullptr;
HMENU bMain = nullptr;
HMENU bStore = nullptr;
HCURSOR mainCur = nullptr;
HCURSOR outCur = nullptr;
HICON bIcon = nullptr;
HDC PaintDC = nullptr;

MSG bMsg;
BOOL bRet = 0;

PAINTSTRUCT bPaint;

wchar_t current_player[16] = L"BAGERIST";
int name_size = 9;

bool sound = true;
bool pause = false;
bool show_help = false;
bool in_client = true;
bool set_name = false;
bool b1_hglt = false;
bool b2_hglt = false;
bool b3_hglt = false;

RECT but1Rect = { 0,0,0,0 };
RECT but2Rect = { 0,0,0,0 };
RECT but3Rect = { 0,0,0,0 };

POINT cur_pos = { 0,0 };
UINT bTimer = -1;

int clWidth = 0;
int clHeight = 0;

int level = 1;
int score = 0;
int seconds = 0;
int minutes = 0;

/////////////////////////////////////////////////////////

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;

ID2D1RadialGradientBrush* ButBckgBrush = nullptr;
ID2D1SolidColorBrush* BackBrush = nullptr;
ID2D1SolidColorBrush* TxtBrush = nullptr;
ID2D1SolidColorBrush* HgltTxtBrush = nullptr;
ID2D1SolidColorBrush* InactiveTxtBrush = nullptr;

IDWriteFactory* iWriteFactory = nullptr;
IDWriteTextFormat* nrmTextFormat = nullptr;
IDWriteTextFormat* bigTextFormat = nullptr;

ID2D1Bitmap* bmpField = nullptr;
ID2D1Bitmap* bmpPlatform = nullptr;
ID2D1Bitmap* bmpBase = nullptr;
ID2D1Bitmap* bmpBigG = nullptr;
ID2D1Bitmap* bmpMidG = nullptr;
ID2D1Bitmap* bmpSmallG = nullptr;
ID2D1Bitmap* bmpBigS = nullptr;
ID2D1Bitmap* bmpMidS = nullptr;
ID2D1Bitmap* bmpSmallS = nullptr;
ID2D1Bitmap* bmpBag = nullptr;
ID2D1Bitmap* bmpChain = nullptr;
ID2D1Bitmap* bmpHead[76];
//////////////////////////////////////////////////////////////

template <typename COM> void SafeRelease(COM** which_COM)
{
    if ((*which_COM))
    {
        (*which_COM)->Release();
        (*which_COM) = nullptr;
    }
}

void ReleaseCOM()
{
    SafeRelease(&iFactory);
    SafeRelease(&Draw);
    SafeRelease(&ButBckgBrush);
    SafeRelease(&BackBrush);
    SafeRelease(&TxtBrush);
    SafeRelease(&HgltTxtBrush);
    SafeRelease(&InactiveTxtBrush);
    SafeRelease(&iWriteFactory);
    SafeRelease(&nrmTextFormat);
    SafeRelease(&bigTextFormat);
    SafeRelease(&bmpBag);
    SafeRelease(&bmpField);
    SafeRelease(&bmpBase);
    SafeRelease(&bmpBigG);
    SafeRelease(&bmpMidG);
    SafeRelease(&bmpSmallG);
    SafeRelease(&bmpBigS);
    SafeRelease(&bmpMidS);
    SafeRelease(&bmpSmallS);
    SafeRelease(&bmpChain);
    SafeRelease(&bmpPlatform);
    for (int i = 0; i < 76; i++)
        SafeRelease(&bmpHead[i]);

}
void InitGame()
{
    wcscpy_s(current_player, L"BAGERIST");
    name_size = 9;
    set_name = false;
    
    score = 0;
    level = 1;
    seconds = 60;
}
void ErrExit(int which_error)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(which_error), L"Критична грешка !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    std::remove(temp_file);
    ReleaseCOM();
    exit(1);
}

void GameOver()
{
    PlaySound(NULL, NULL, NULL);
    KillTimer(bHwnd, bTimer);


   
    std::remove(temp_file);
    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
        case WM_INITDIALOG:
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)bIcon);
            return (INT_PTR)TRUE;
            break;

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
                    name_size = GetDlgItemText(hwnd, IDC_NAME, current_player, 15);
                    if (name_size < 1)
                    {
                        wcscpy_s(current_player, L"BAGERIST");
                        name_size = 9;
                        if (sound)MessageBeep(MB_ICONASTERISK);
                        MessageBox(bHwnd, L"Ха, ха, ха ! Забрави си името !", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONASTERISK);
                        EndDialog(hwnd, IDCANCEL);
                        break;
                    }
                    EndDialog(hwnd, IDOK);
            }
            break;
    }

    return (INT_PTR)FALSE;
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
        case WM_CREATE:
            {
                RECT clr = { 0,0,0,0 };
                GetClientRect(hwnd, &clr);
                clWidth = clr.right;
                clHeight = clr.bottom;

                but1Rect = { 0,0,200,50 };
                but2Rect = { 275,0,475,50 };
                but3Rect = { 550,0,800,50 };

                srand((unsigned int)time(0));
                SetTimer(hwnd, bTimer, 1000, NULL);

                bBar = CreateMenu();
                bMain = CreateMenu();
                bStore = CreateMenu();

                AppendMenu(bBar, MF_POPUP, (UINT_PTR)bMain, L"Основно меню");
                AppendMenu(bBar, MF_POPUP, (UINT_PTR)bStore, L"Меню за данни");

                AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
                AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
                AppendMenu(bMain, MF_STRING, mExit, L"Изход");

                AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
                AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
                AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
                AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");

                SetMenu(hwnd, bBar);
                InitGame();
            }
            break;


        case WM_CLOSE:
            pause = true;
            if (sound)MessageBeep(MB_ICONEXCLAMATION);
            if (MessageBox(hwnd, L"Ако излезеш, ще загубиш тази игра !\n\nНаистина ли излизаш ?",
                L"Изход", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            GameOver();
            break;

        case WM_PAINT:
            PaintDC = BeginPaint(hwnd, &bPaint);
            FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(80, 80, 80)));
            EndPaint(hwnd, &bPaint);
            break;

        case WM_SETCURSOR:
            GetCursorPos(&cur_pos);
            ScreenToClient(hwnd, &cur_pos);
            if (LOWORD(lParam) == HTCLIENT)
            {
                if (!in_client)
                {
                    in_client = true;
                    pause = false;
                }

                if (cur_pos.y <= 50)
                {
                    SetCursor(outCur);
                    if (cur_pos.x >= but1Rect.left && cur_pos.x <= but1Rect.right)
                    {
                        if (!b1_hglt)
                        {
                            if(sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                            b1_hglt = true;
                        }
                    }
                    else
                    {
                        if (b1_hglt)
                        {
                            if(sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                            b1_hglt = false;
                        }
                        
                    }

                    if (cur_pos.x >= but2Rect.left && cur_pos.x <= but2Rect.right)
                    {
                        if (!b2_hglt)
                        {
                            if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                            b2_hglt = true;
                        }
                    }
                    else
                    {
                        if (b2_hglt && sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b2_hglt = false;
                        
                    }
                    if (cur_pos.x >= but3Rect.left && cur_pos.x <= but3Rect.right)
                    {
                        if (!b3_hglt)
                        {
                            if(sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                            b3_hglt = true;
                        }
                    }
                    else
                    {
                        if (b3_hglt && sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b3_hglt = false;
                    }
                    return true;
                }
                else
                {
                    if (b1_hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1_hglt = false;
                    }

                    if (b2_hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b2_hglt = false;
                    }

                    if (b3_hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b3_hglt = false;
                    }
                }
                SetCursor(mainCur);
                return true;
            }
            else
            {
                if (in_client)
                {
                    in_client = false;
                    pause = true;
                }

                if (b1_hglt)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    b1_hglt = false;
                }

                if (b2_hglt)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    b2_hglt = false;
                }

                if (b3_hglt)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    b3_hglt = false;
                }
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return true;
            }
            break;

        case WM_TIMER:
            if (pause)break;
            minutes = (int)(floor(seconds / 60));
            seconds--;
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case mNew:
                    pause = true;
                    if (sound)MessageBeep(MB_ICONEXCLAMATION);
                    if (MessageBox(hwnd, L"Ако рестартираш, ще загубиш тази игра !\n\nНаистина ли рестартираш ?",
                        L"Рестарт", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
                    {
                        pause = false;
                        break;
                    }
                    InitGame();
                    pause = false;
                    break;

                case mExit:
                    SendMessage(hwnd, WM_CLOSE, NULL, NULL);
                    break;


            }
            break;

        case WM_LBUTTONDOWN:
            if (LOWORD(lParam) >= but1Rect.left && LOWORD(lParam) <= but1Rect.right
                && HIWORD(lParam) >= but1Rect.top && HIWORD(lParam) <= but1Rect.bottom)
            {
                if (set_name)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                    break;
                }
                if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &DlgProc) == IDOK)set_name = true;
                break;
            }
            if (LOWORD(lParam) >= but2Rect.left && LOWORD(lParam) <= but2Rect.right
                && HIWORD(lParam) >= but2Rect.top && HIWORD(lParam) <= but2Rect.bottom)
            {
                if (sound)
                {
                    PlaySound(NULL, NULL, NULL);
                    sound = false;
                    break;
                }
                else
                {
                    PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
                    sound = true;
                    break;
                }
            }


            break;


        default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)FALSE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    for (int i = 0; i < 76; ++i)bmpHead[i] = nullptr;
    
    if (!bIns)ErrExit(eClass);
    int check_started = 0;
    CheckFile(Ltemp_file, &check_started);
    if (check_started == FILE_EXIST)ErrExit(eStarted);
    else
    {
        std::wofstream tmp(temp_file);
        tmp << L"Играта е стартирана !";
        tmp.close();
    }

    if (GetSystemMetrics(SM_CXSCREEN) + 100 < scr_width || GetSystemMetrics(SM_CYSCREEN) + 50 < scr_height)ErrExit(eScreen);

    bIcon = (HICON)LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 119, LR_LOADFROMFILE);
    if (!bIcon)ErrExit(eIcon);

    mainCur = LoadCursorFromFile(L".\\res\\bcursor.ani");
    outCur = LoadCursorFromFile(L".\\res\\out.ani");
    if (!mainCur || !outCur)ErrExit(eCursor);

    ZeroMemory(&bWinClass, sizeof(WNDCLASS));

    bWinClass.lpszClassName = bWinClassName;
    bWinClass.hInstance = bIns;
    bWinClass.lpfnWndProc = &WinProc;
    bWinClass.hbrBackground = CreateSolidBrush(RGB(80, 80, 80));
    bWinClass.hIcon = bIcon;
    bWinClass.hCursor = mainCur;
    bWinClass.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWinClass))ErrExit(eClass);

    bHwnd = CreateWindow(bWinClassName, L"СЪБЕРИ ПЛЯЧКАТА !", WS_CAPTION | WS_SYSMENU, 100, 50, scr_width, scr_height,
        NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else ShowWindow(bHwnd, SW_SHOWDEFAULT);
    /////////////////////////////////////////////////////////////

    HRESULT hr = S_OK;

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 Factory !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    if (iFactory && bHwnd)
        hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
            D2D1::SizeU(clWidth, clHeight)), &Draw);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 HWND Render Target !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    D2D1_GRADIENT_STOP gStops[2] = { 0,0,0,0 };
    ID2D1GradientStopCollection* gStopCollection = nullptr;
    D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES BrushProperties;

    //CREATE RADIAL GRADIENT BRUSH **************************
    gStops[0].position = 0.0f;
    gStops[0].color = D2D1::ColorF(D2D1::ColorF::DarkOrchid);
    gStops[1].position = 1.0f;
    gStops[1].color = D2D1::ColorF(D2D1::ColorF::DarkViolet);

    hr = Draw->CreateGradientStopCollection(gStops, 2, &gStopCollection);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 GradientStopCollection !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    BrushProperties.center = D2D1::Point2F((float)clWidth, 25.0f);
    BrushProperties.gradientOriginOffset = D2D1::Point2F(0, 0);
    BrushProperties.radiusX = (float)(clWidth / 2);
    BrushProperties.radiusY = 25.0f;

    if (gStopCollection)
        hr = Draw->CreateRadialGradientBrush(BrushProperties, gStopCollection, &ButBckgBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 RadialGradientBrush !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }
    //********************************************************************

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BurlyWood), &BackBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 SolidColorBrush !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::GreenYellow), &TxtBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 SolidColorBrush !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Beige), &HgltTxtBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 SolidColorBrush !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &InactiveTxtBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 SolidColorBrush !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }
    ///////////////////////////////////////////////////////////////////////

    //WRITE FACTORY AND TEXT **********************************************

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&iWriteFactory));
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 WriteFactory !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = iWriteFactory->CreateTextFormat(L"Gabriola", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
        DWRITE_FONT_STRETCH_EXTRA_EXPANDED, 24.0f, L"", &nrmTextFormat);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 nrmTextFormat !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = iWriteFactory->CreateTextFormat(L"Gabriola", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
        DWRITE_FONT_STRETCH_EXTRA_EXPANDED, 64.0f, L"", &bigTextFormat);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 nrmTextFormat !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }
    /////////////////////////////////////////////////////////////////////

    bmpField = Load(L".\\res\\img\\field.png", Draw);
    if (!bmpField)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpField !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpPlatform = Load(L".\\res\\img\\platform.png", Draw);
    if (!bmpPlatform)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpPlatform !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpBase = Load(L".\\res\\img\\base.png", Draw);
    if (!bmpBase)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpBase !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpBigG = Load(L".\\res\\img\\biggold.png", Draw);
    if (!bmpBigG)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpBigGold !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpMidG = Load(L".\\res\\img\\midgold.png", Draw);
    if (!bmpMidG)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpMidGold !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpSmallG = Load(L".\\res\\img\\smgold.png", Draw);
    if (!bmpSmallG)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpSmallGold !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpBigS = Load(L".\\res\\img\\bigsilver.png", Draw);
    if (!bmpBigS)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpBigSilver !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpMidS = Load(L".\\res\\img\\midsilver.png", Draw);
    if (!bmpMidS)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpMidSilver !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpSmallS = Load(L".\\res\\img\\smsilver.png", Draw);
    if (!bmpSmallS)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpSmallSilver !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpBag = Load(L".\\res\\img\\bag.png", Draw);
    if (!bmpBag)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpBag !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpChain = Load(L".\\res\\img\\chain.png", Draw);
    if (!bmpChain)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpChain !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }
    
    for (int i = 0; i < 76; i++)
    {
        wchar_t name[75] = L".\\res\\img\\head\\";
        wchar_t add[5] = L"\0";
        wsprintf(add, L"%d", i);
        wcscat_s(name, add);
        wcscat_s(name, L".png");

        bmpHead[i] = Load(name, Draw);
        if (!bmpHead[i])
        {
            std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
            log << L"Error creating D2D1 bmpHead !" << std::endl;
            log.close();
            ErrExit(eD2D);
        }
    }

    //////////////////////////////////////////////////////////////////////

    //MAIN LOOP *******************************************************
    
    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);
            TranslateMessage(&bMsg);
            DispatchMessage(&bMsg);
        }

        if (pause)
        {
            if (show_help)continue;
            Draw->BeginDraw();
            if (BackBrush)
                Draw->FillRectangle(D2D1::RectF(0.0f, 50.0f, 150.0f, (float)(clHeight)), BackBrush);
            if (ButBckgBrush)
                Draw->FillRectangle(D2D1::RectF(0.0f, 0.0f, (float)(clWidth), 50.0f), ButBckgBrush);
            if (nrmTextFormat && TxtBrush)
            {
                if(set_name && InactiveTxtBrush)
                    Draw->DrawText(L"Име на играч", 13, nrmTextFormat, D2D1::RectF(20.0f, 0.0f, (float)(but1Rect.right), 50.0f),
                        InactiveTxtBrush);
                if (!b1_hglt)
                {
                    if (!set_name)
                        Draw->DrawText(L"Име на играч", 13, nrmTextFormat, D2D1::RectF(20.0f, 0.0f, (float)(but1Rect.right), 50.0f),
                            TxtBrush);
                }
                else
                {
                    if (!set_name)
                        Draw->DrawText(L"Име на играч", 13, nrmTextFormat, D2D1::RectF(20.0f, 0.0f, (float)(but1Rect.right), 50.0f),
                            HgltTxtBrush);
                }
                if (!b2_hglt)
                    Draw->DrawText(L"Звуци ON / OFF", 15, nrmTextFormat, 
                        D2D1::RectF((float)(but2Rect.left + 10.0f), 0.0f, (float)(but2Rect.right), 50.0f), TxtBrush);
                else
                    Draw->DrawText(L"Звуци ON / OFF", 15, nrmTextFormat, 
                        D2D1::RectF((float)(but2Rect.left + 10.0f), 0.0f, (float)(but2Rect.right), 50.0f), HgltTxtBrush);
                if (!b3_hglt)
                    Draw->DrawText(L"Помощ за играта", 16, nrmTextFormat,
                        D2D1::RectF((float)(but3Rect.left + 10.0f), 0.0f, (float)(but3Rect.right)), TxtBrush);
                else
                    Draw->DrawText(L"Помощ за играта", 16, nrmTextFormat, 
                        D2D1::RectF((float)(but3Rect.left + 10.0f), 0.0f, (float)(but3Rect.right)), HgltTxtBrush);
            }
            Draw->DrawBitmap(bmpField, D2D1::RectF(150.0f, 50.0f, (float)(clWidth), (float)(clHeight)));
            if (TxtBrush && bigTextFormat)
                Draw->DrawText(L"ПАУЗА", 6, bigTextFormat, D2D1::RectF((float)(clWidth / 2 - 50), (float)(clHeight / 2 - 50),
                    (float)(clWidth), (float)(clHeight)), TxtBrush);
            Draw->EndDraw();
            continue;
        }


        //DRAW THINGS *************************************************

        Draw->BeginDraw();
        if (BackBrush)
            Draw->FillRectangle(D2D1::RectF(0.0f, 50.0f, 150.0f, (float)(clHeight)), BackBrush);
        if (ButBckgBrush)
            Draw->FillRectangle(D2D1::RectF(0.0f, 0.0f, (float)(clWidth), 50.0f), ButBckgBrush);
        Draw->DrawBitmap(bmpField, D2D1::RectF(150.0f, 50.0f, (float)(clWidth), (float)(clHeight)));
        if (nrmTextFormat && TxtBrush)
        {
            if (set_name && InactiveTxtBrush)
                Draw->DrawText(L"Име на играч", 13, nrmTextFormat, D2D1::RectF(20.0f, 0.0f, (float)(but1Rect.right), 50.0f),
                    InactiveTxtBrush);
            if (!b1_hglt)
            {
                if (!set_name)
                    Draw->DrawText(L"Име на играч", 13, nrmTextFormat, D2D1::RectF(20.0f, 0.0f, (float)(but1Rect.right), 50.0f),
                        TxtBrush);
            }
            else
            {
                if (!set_name)
                    Draw->DrawText(L"Име на играч", 13, nrmTextFormat, D2D1::RectF(20.0f, 0.0f, (float)(but1Rect.right), 50.0f),
                        HgltTxtBrush);
            }
            if (!b2_hglt)
                Draw->DrawText(L"Звуци ON / OFF", 15, nrmTextFormat,
                    D2D1::RectF((float)(but2Rect.left + 10.0f), 0.0f, (float)(but2Rect.right), 50.0f), TxtBrush);
            else
                Draw->DrawText(L"Звуци ON / OFF", 15, nrmTextFormat,
                    D2D1::RectF((float)(but2Rect.left + 10.0f), 0.0f, (float)(but2Rect.right), 50.0f), HgltTxtBrush);
            if (!b3_hglt)
                Draw->DrawText(L"Помощ за играта", 16, nrmTextFormat,
                    D2D1::RectF((float)(but3Rect.left + 10.0f), 0.0f, (float)(but3Rect.right)), TxtBrush);
            else
                Draw->DrawText(L"Помощ за играта", 16, nrmTextFormat,
                    D2D1::RectF((float)(but3Rect.left + 10.0f), 0.0f, (float)(but3Rect.right)), HgltTxtBrush);
        }
        /////////////////////////////////////////////////////////////////////////

        Draw->DrawBitmap(bmpBase, D2D1::RectF(0.0f, (float)(clHeight / 2), 130.0f, (float)(clHeight / 2 + 61)));





        Draw->EndDraw();

    }
    /////////////////////////////////////////////////////////////////////

    ReleaseCOM();
    return (int) bMsg.wParam;
}