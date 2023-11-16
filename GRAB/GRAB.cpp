#include "D2BMPLOADER.h"
#include "ErrH.h"
#include "FCheck.h"
#include "framework.h"
#include "GRAB.h"
#include "ofactory.h"
#include "resource.h"
#include <ctime>
#include <d2d1.h>
#include <dwrite.h>
#include <fstream>
#include <mmsystem.h>
#include <vector>

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

bool level_up = false;

bool target_down = true;

RECT but1Rect = { 0,0,0,0 };
RECT but2Rect = { 0,0,0,0 };
RECT but3Rect = { 0,0,0,0 };


RECT store_timeRect = { 0,0,0,0 };
RECT store_speedRect = { 0,0,0,0 };
RECT store_exitRect = { 0,0,0,0 };

POINT cur_pos = { 0,0 };
UINT bTimer = -1;

int clWidth = 0;
int clHeight = 0;

int level = 1;
int score = 0;
int seconds = 0;
int minutes = 0;
int max_benefits_for_level = 5;
int target_points = 0;

/////////////////////////////////////////////////////////

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;

ID2D1RadialGradientBrush* ButBckgBrush = nullptr;
ID2D1SolidColorBrush* BackBrush = nullptr;
ID2D1SolidColorBrush* LevelBackBrush = nullptr;
ID2D1SolidColorBrush* TxtBrush = nullptr;
ID2D1SolidColorBrush* HgltTxtBrush = nullptr;
ID2D1SolidColorBrush* InactiveTxtBrush = nullptr;
ID2D1SolidColorBrush* RedTxtBrush = nullptr;
ID2D1SolidColorBrush* GreenTxtBrush = nullptr;

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
ID2D1Bitmap* bmpTarget = nullptr;
ID2D1Bitmap* bmpHead[76];
//////////////////////////////////////////////////////////////

head_ptr Head = nullptr;
std::vector<object_ptr> vChain;
std::vector<benefit_ptr> vBenefits;

object_ptr Target = nullptr;

/////////////////////////////////////////////////////////////

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
    SafeRelease(&LevelBackBrush);
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
    SafeRelease(&bmpTarget);
    SafeRelease(&RedTxtBrush);
    SafeRelease(&GreenTxtBrush);


    for (int i = 0; i < 76; i++)
        SafeRelease(&bmpHead[i]);

}
void InitGame()
{
    wcscpy_s(current_player, L"BAGERIST");
    name_size = 9;
    set_name = false;
    level_up = false;
    
    score = 0;
    level = 1;
    seconds = 90 - 2 * level;
    minutes = (int)(floor(seconds / 60));
    max_benefits_for_level = 11 - level;
    target_points = 50 + 10 * level;
    
    if (max_benefits_for_level < 6)max_benefits_for_level = 6;

    Head = new HEAD(150.0f, (float)(clHeight / 2));
    vChain.clear();

    if (Target)
    {
        Target->Release();
        Target = nullptr;
    }

    Target = new OBJECT(static_cast<float>(clWidth - 50), 50.0f, 40.0f, 46.0f);

    if (!vBenefits.empty())
    {
        for (int i = 0; i < vBenefits.size(); ++i) vBenefits[i]->Release();
    }
   
    vBenefits.clear();

    for (int i = 0; i <= max_benefits_for_level; ++i)
    {
        bool end_init = true;

        while (end_init)
        {
            float tx = 0;
            float ty = 0;
            int ttype = -1;
            bool problem = false;

            ttype = rand() % 7;

            tx = (float)(rand() % 755 + 200);
            ty = (float)(rand() % 480 + 50);


            if (!vBenefits.empty())
            {
                for (int j = 0; j < vBenefits.size(); j++)
                {
                    if (tx >= vBenefits[j]->x && tx <= vBenefits[j]->ex + 40.0f && ty >= vBenefits[j]->y && ty <= vBenefits[j]->ey)
                    {
                        problem = true;
                        break;
                    }
                }
            
            }
            if (!problem)
            {
                vBenefits.push_back(iCreate(static_cast<types>(ttype), tx, ty));
                end_init = false;
            }
        }
    }
}
void ErrExit(int which_error)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(which_error), L"Критична грешка !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    std::remove(temp_file);
    ReleaseCOM();
    exit(1);
}
BOOL CheckRecord()
{
    if (score < 1)return no_record;

    int result = -1;
    CheckFile(record_file, &result);

    if (result == FILE_NOT_EXIST)
    {
        std::wofstream rec(record_file);
        rec << score << std::endl;
        for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
        rec.close();
        return first_record;
    }

    std::wfstream check(record_file);
    int svd = 0;
    check >> svd;
    check.close();

    if (score > svd)
    {
        std::wofstream rec(record_file);
        rec << score << std::endl;
        for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
        rec.close();
        return record;
    }
    return no_record;
}
void GameOver()
{
    PlaySound(NULL, NULL, NULL);
    KillTimer(bHwnd, bTimer);

    wchar_t final_text[100] = L"\0";
    int final_size = 0;
    

    switch (CheckRecord())
    {
        case no_record:
            if (sound)PlaySound(L".\\res\\snd\\loose.wav", NULL, SND_ASYNC);
            wcscpy_s(final_text, L"ИГРАТА СВЪРШИ !");
            break;

        case first_record:
            if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_ASYNC);
            wcscpy_s(final_text, L"ПЪРВИ РЕКОРД !");
            break;

        case record:
            if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_ASYNC);
            wcscpy_s(final_text, L"НОВ СВЕТОВЕН РЕКОРД !");
            break;
    }

    for (int i = 0; i < 100; i++)
    {
        if (final_text[i] != '\0')final_size++;
        else break;
    }
    
    Draw->BeginDraw();
    Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkViolet));
    Draw->DrawText(final_text, final_size, bigTextFormat, D2D1::RectF(100.0f, (float)(clHeight / 2 - 50), (float)(clWidth), (float)(clHeight)),
        TxtBrush);
    Draw->EndDraw();
    Sleep(6500);
    std::remove(temp_file);
    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void HallOfFame()
{
    int result = 0;
    CheckFile(record_file, &result);

    if (result == FILE_NOT_EXIST)
    {
        if (sound)MessageBeep(MB_ICONEXCLAMATION);
        MessageBox(bHwnd, L"Липсва рекорд на играта !\n\nПостарай се повече !", L"Липсва файл !", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    wchar_t a_text[100] = L"Най-добър играч: ";
    wchar_t svd_pl[16] = L"\0";
    wchar_t add[5] = L"\0";

    int svd_score = 0;
    int text_size = 0;

    std::wifstream rec(record_file);

    rec >> svd_score;
    wsprintf(add, L"%d", svd_score);
    
    for (int i = 0; i < 16; i++)
    {
        int a_letter = 0;
        rec >> a_letter;
        svd_pl[i] = static_cast<wchar_t>(a_letter);
    }

    wcscat_s(a_text, svd_pl);
    wcscat_s(a_text, L" !\nрекорд: ");
    wcscat_s(a_text, add);

    for (int i = 0; i < 100; i++)
    {
        if (a_text[i] != '\0')text_size++;
        else break;
    }

    if (sound)mciSendString(L"play .\\res\\snd\\tada.wav", NULL, NULL, NULL);

    Draw->BeginDraw();
    Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkViolet));
    Draw->DrawText(a_text, text_size, bigTextFormat, D2D1::RectF(50.0f, (float)(clHeight / 2 - 50), (float)(clWidth), (float)(clHeight)),
        TxtBrush);
    Draw->EndDraw();
    Sleep(3000);
}
void SaveGame()
{
    int result = -1;
    CheckFile(save_file, &result);
    if (result == FILE_EXIST)
    {
        if (sound)MessageBeep(MB_ICONEXCLAMATION);
        if (MessageBox(bHwnd, L"Има записана игра, която ще загубиш !\n\nПрезаписваш ли я ?", L"Презапис !",
            MB_YESNO | MB_ICONQUESTION) == IDNO)return;
    }

    std::wofstream save(save_file);

    save << score << std::endl;
    for (int i = 0; i < 16; i++)save << static_cast<int>(current_player[i]) << std::endl;
    save << level << std::endl;
    save << name_size << std::endl;
    save << set_name << std::endl;
    save << level_up << std::endl;
    save << seconds << std::endl;
    save << max_benefits_for_level << std::endl;
    save << target_points << std::endl;

    save << vBenefits.size() << std::endl;
    for (int i = 0; i < vBenefits.size(); i++)
    {
        save << static_cast<int>(vBenefits[i]->type) << std::endl;
        save << vBenefits[i]->x << std::endl;
        save << vBenefits[i]->y << std::endl;
    }

    save.close();

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
    MessageBox(bHwnd, L"Играта е записана !", L"Съхранение !", MB_OK | MB_ICONEXCLAMATION);
}
void LoadGame()
{
    int result = -1;
    CheckFile(save_file, &result);
    if (result == FILE_EXIST)
    {
        if (sound)MessageBeep(MB_ICONEXCLAMATION);
        if (MessageBox(bHwnd, L"Настоящата игра ще бъде загубена !\n\nПрезаписваш ли я ?", L"Презапис !",
            MB_YESNO | MB_ICONQUESTION) == IDNO)return;
    }
    else
    {
        if (sound)MessageBeep(MB_ICONEXCLAMATION);
        MessageBox(bHwnd, L"Липсва записана игра !\n\nПостарай се повече !", L"Липсва файл !", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    vChain.clear();

    if (Target)
    {
        Target->Release();
        Target = nullptr;
    }

    Target = new OBJECT(static_cast<float>(clWidth - 50), 50.0f, 40.0f, 46.0f);

    if (!vBenefits.empty())
    {
        for (int i = 0; i < vBenefits.size(); ++i) vBenefits[i]->Release();
    }

    vBenefits.clear();

    std::wifstream save(save_file);

    save >> score;
    for (int i = 0; i < 16; i++)
    {
        int letter = 0;

        save >> letter;
        current_player[i]=static_cast<wchar_t>(letter);
    
    }
    save >> level;
    save >> name_size;
    save >> set_name;
    save >> level_up;
    save >> seconds;
    save >> max_benefits_for_level;
    save >> target_points;

    save >> result;

    if (result > 0)
    {
        for (int i = 0; i < result; i++)
        {
            int ttype = -1;
            float tx = 0;
            float ty = 0;
            
            save >> ttype;
            save >> tx;
            save >> ty;

            vBenefits.push_back(iCreate(static_cast<types>(ttype), tx, ty));
        }
    }
    save.close();

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
    MessageBox(bHwnd, L"Играта е заредена !", L"Съхранение !", MB_OK | MB_ICONEXCLAMATION);
}
void ShowHelp()
{
    int result = -1;
    CheckFile(help_file, &result);

    if (result == FILE_NOT_EXIST)
    {
        if (sound)MessageBeep(MB_ICONERROR);
        MessageBox(bHwnd, L"Липсва помощ за играта !\n\nсвържете се с разработчика !", L"Липсва файл !", MB_OK | MB_ICONERROR);
        return;
    }

    wchar_t help_txt[1000] = L"\0";

    std::wifstream help(help_file);
    help >> result;
    for (int i = 0; i < result; ++i)
    {
        int letter = 0;
        help >> letter;
        help_txt[i] = static_cast<wchar_t>(letter);
    }

    Draw->BeginDraw();
    if (BackBrush)
        Draw->FillRectangle(D2D1::RectF(0.0f, 50.0f, 150.0f, (float)(clHeight)), BackBrush);
    if (ButBckgBrush)
        Draw->FillRectangle(D2D1::RectF(0.0f, 0.0f, (float)(clWidth), 50.0f), ButBckgBrush);
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
    if (RedTxtBrush)
        Draw->FillRectangle(D2D1::RectF(0, 50.0f, (float)(clWidth), (float)(clHeight)), RedTxtBrush);
    if (nrmTextFormat && TxtBrush)
        Draw->DrawText(help_txt, result, nrmTextFormat, D2D1::RectF(100.0f, 70.0f, (float)(clWidth), (float)(clHeight)),
            TxtBrush);
    Draw->EndDraw();
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

                store_timeRect = { 360, 250, 500, 350 };
                store_speedRect = { 360, 360, 500, 450 };
                store_exitRect = { 450, 450, 600, 500 };

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
            if (pause || level_up)break;
            seconds--;
            minutes = (int)(floor(seconds / 60));
            if (seconds < 0)
            {
                if (score < target_points)GameOver();

                if (sound)mciSendString(L"play .\\res\\snd\\levelup.wav", NULL, NULL, NULL);
                pause = true;
                level_up = true;

                level++;
                seconds = 90 - 2 * level;
                max_benefits_for_level = 11 - level;
                target_points = 50 + 10 * level;
               
                vChain.clear();
                
                if (Target)
                {
                    Target->Release();
                    Target = nullptr;
                }

                Target = new OBJECT(static_cast<float>(clWidth - 50), 50.0f, 40.0f, 46.0f);

                if (!vBenefits.empty())
                {
                    for (int i = 0; i < vBenefits.size(); ++i) vBenefits[i]->Release();
                }
                vBenefits.clear();

                for (int i = 0; i <= max_benefits_for_level; ++i)
                {
                    bool end_init = true;

                    while (end_init)
                    {
                        float tx = 0;
                        float ty = 0;
                        int ttype = -1;
                        bool problem = false;

                        ttype = rand() % 7;

                        tx = (float)(rand() % 755 + 200);
                        ty = (float)(rand() % 480 + 50);


                        if (!vBenefits.empty())
                        {
                            for (int j = 0; j < vBenefits.size(); j++)
                            {
                                if (tx >= vBenefits[j]->x && tx <= vBenefits[j]->ex + 40.0f && ty >= vBenefits[j]->y && ty <= vBenefits[j]->ey)
                                {
                                    problem = true;
                                    break;
                                }
                            }

                        }
                        if (!problem)
                        {
                            vBenefits.push_back(iCreate(static_cast<types>(ttype), tx, ty));
                            end_init = false;
                        }
                    }
                }
            }
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

                case mSave:
                    pause = true;
                    SaveGame();
                    pause = false;
                    break;

                case mLoad:
                    pause = true;
                    LoadGame();
                    pause = false;
                    break;

                case mHoF:
                    pause = true;
                    HallOfFame();
                    pause = false;
                    break;

            }
            break;

        case WM_LBUTTONDOWN:

            if (level_up)
            {
                if (LOWORD(lParam) >= store_timeRect.left && LOWORD(lParam) <= store_timeRect.right
                    && HIWORD(lParam) >= store_timeRect.top && HIWORD(lParam) <= store_timeRect.bottom)
                {
                    if (score < 75)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                        break;
                    }
                    else
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                        score -= 75;
                        seconds = 120 - 2 * level;
                        pause = false;
                        level_up = false;
                        break;
                    }
                }

                if (LOWORD(lParam) >= store_speedRect.left && LOWORD(lParam) <= store_speedRect.right
                    && HIWORD(lParam) >= store_speedRect.top && HIWORD(lParam) <= store_speedRect.bottom)
                {
                    if (score < 50)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                        break;
                    }
                    else
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                        max_benefits_for_level += 3;
                        score -= 50;
                        pause = false;
                        level_up = false;
                        break;
                    }
                }

                if (LOWORD(lParam) >= store_exitRect.left && LOWORD(lParam) <= store_exitRect.right
                    && HIWORD(lParam) >= store_exitRect.top && HIWORD(lParam) <= store_exitRect.bottom)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                    pause = false;
                    level_up = false;
                    break;
                }
            }

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
            else if (LOWORD(lParam) >= but2Rect.left && LOWORD(lParam) <= but2Rect.right
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
            else if (LOWORD(lParam) >= but3Rect.left && LOWORD(lParam) <= but3Rect.right
                && HIWORD(lParam) >= but3Rect.top && HIWORD(lParam) <= but3Rect.bottom)
            {
                if (!show_help)
                {
                    show_help = true;
                    pause = true;
                    ShowHelp();
                    break;
                }
                else
                {
                    show_help = false;
                    pause = false;
                    break;
                }
            }
            else
            {
                if (Head && Target)
                {
                    if (Head->moving)break;
                    Head->moving = true;
                    Head->GetLambda(Target->y + 20.0f);
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
    
    PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
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

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &RedTxtBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 SolidColorBrush !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &GreenTxtBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 SolidColorBrush !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkViolet), &LevelBackBrush);
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

    bmpTarget = Load(L".\\res\\img\\target.png", Draw);
    if (!bmpTarget)
    {
        std::wofstream log(L".\\res\\data\\log.err", std::ios::app);
        log << L"Error creating D2D1 bmpTarget !" << std::endl;
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

    wchar_t first_text[32] = L"АЛЧЕН БАГЕРИСТ !\n\ndev. Daniel";
    wchar_t text_to_show[32] = L"\0";
    int fsize = 0;

    for (int i = 0; i < 32; ++i)
    {
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::Red));
        text_to_show[i] = first_text[i];
        mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
        if (bigTextFormat && TxtBrush)
            Draw->DrawText(text_to_show, fsize, bigTextFormat, D2D1::RectF(150.0f, 150.0f, (float)(clWidth), (float)clHeight), TxtBrush);
        fsize++;
        Draw->EndDraw();
        Sleep(50);
    }
    Sleep(3000);
    
    
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

            if (level_up)
            {
                Draw->BeginDraw();
                Draw->FillRectangle(D2D1::RectF(350.0f, 150.0f, 600.0f, 500.0f), LevelBackBrush);
                Draw->DrawText(L"НИВОТО ПРЕМИНАТО !\n         МАГАЗИН", 38, nrmTextFormat,
                    D2D1::RectF(380.0f, 155.0f, 600.0f, 300.0f), TxtBrush);
                Draw->DrawText(L"ВРЕМЕ - 75", 11, nrmTextFormat, D2D1::RectF(360.0f, 250.0f, 500.0f, 350.0f), TxtBrush);
                Draw->DrawText(L"БРОЙ КЮЛЧЕТА - 50", 18, nrmTextFormat, D2D1::RectF(360.0f, 360.0f, 500.0f, 450.0f), TxtBrush);
                Draw->DrawText(L"ИЗХОД", 6, nrmTextFormat, D2D1::RectF(450.0f, 450.0f, 600.0f, 500.0f), TxtBrush);
                Draw->EndDraw();
                continue;
            }


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
        ////////////////////////////////////////////////////////////////

        //GAME PLAY ***************************************************

        //CHAIN ***********************************************************

        if (Head)
        {
            float chain_distance = Head->x - 140.0f;
            int need_number = (int)(chain_distance / 10);
            float ch_lambda = (Head->y + 25.0f - static_cast<float>(clHeight / 2 + 30)) / (Head->x - 140.0f);
            vChain.clear();
            vChain.push_back(new OBJECT(130.0f, static_cast<float>(clHeight / 2 + 30), 10.0f, 11.0f));

            for (int i = 0; i < need_number; ++i)
                vChain.push_back(new OBJECT(vChain.back()->x + 10, vChain.back()->y + 10 * ch_lambda, 10.0f, 11.0f));
        }

        ////////////////////////////////////////////////////////////////////

        //TARGET ***********************************************

        if (Target)
        {
            if (target_down)
            {
                if (Target->ey + 1.0f <= (float)(clHeight))
                {
                    Target->y += 1.0f;
                    Target->SetEdges();
                    
                }
                else target_down = false;
            }
            else
            {
                
                if (Target->y - 1.0f >= 50.0f)
                {
                    Target->y -= 1.0f;
                    Target->SetEdges();
                }
                else target_down = true;
            }
        }

        //MOVE HEAD *****************************************************************

        if (Head && Head->moving)
        {
            if (sound)mciSendString(L"play .\\res\\snd\\chain.wav", NULL, NULL, NULL);
            if (!Head->Move())
            {
                Head->moving = false;
                Head->forward = true;
                if (Head->cargo_attached != types::no_type)
                {
                    switch (Head->cargo_attached)
                    {
                        case types::big_gold:
                            if (sound)mciSendString(L"play .\\res\\snd\\yeah.wav", NULL, NULL, NULL);
                            score += 35;
                            break;

                        case types::big_silver:
                            if (sound)mciSendString(L"play .\\res\\snd\\yeah.wav", NULL, NULL, NULL);
                            score += 25;
                            break; 

                        case types::mid_gold:
                            if (sound)mciSendString(L"play .\\res\\snd\\yeah.wav", NULL, NULL, NULL);
                            score += 30;
                            break;

                        case types::mid_silver:
                            if (sound)mciSendString(L"play .\\res\\snd\\yeah.wav", NULL, NULL, NULL);
                            score += 20;
                            break;

                        case types::sm_gold:
                            if (sound)mciSendString(L"play .\\res\\snd\\yeah.wav", NULL, NULL, NULL);
                            score += 15;
                            break;

                        case types::sm_silver:
                            if (sound)mciSendString(L"play .\\res\\snd\\yeah.wav", NULL, NULL, NULL);
                            score += 10;
                            break;

                        case types::bag:
                            switch (rand() % 4)
                            {
                                case 0:
                                    if (sound)mciSendString(L"play .\\res\\snd\\yeah.wav", NULL, NULL, NULL);
                                    score += 20;
                                    break;

                                case 1:
                                    if (sound)mciSendString(L"play .\\res\\snd\\ouch.wav", NULL, NULL, NULL);
                                    score -= 20;
                                    break;

                                case 2:
                                    if (sound)mciSendString(L"play .\\res\\snd\\yeah.wav", NULL, NULL, NULL);
                                    seconds += 10;
                                    break;

                                case 3:
                                    if (sound)mciSendString(L"play .\\res\\snd\\ouch.wav", NULL, NULL, NULL);
                                    seconds -= 10;
                                    break;
                            }
                            
                            break;
                    }

                    Head->cargo_attached = types::no_type;
                    Head->max_heavy_delay = 0;
                }
            }
        }

        if (Head && !vBenefits.empty())
        {
            for (std::vector<benefit_ptr>::iterator it = vBenefits.begin(); it < vBenefits.end(); ++it)
            {
                if (!(Head->x >= (*it)->ex || Head->ex <= (*it)->x || Head->y >= (*it)->ey || Head->ey <= (*it)->y))
                {
                    if (!Head->forward)break;
                    Head->max_heavy_delay = (*it)->weight;
                    Head->cargo_attached = (*it)->type;
                    Head->forward = false;
                    (*it)->Release();
                    vBenefits.erase(it);
                    break;
                }
            }
        }

        if (vBenefits.size() < max_benefits_for_level && rand() % 30 == 20)
        {
            bool problem = true;
            while (problem)
            {
                float tx = (float)(rand() % 755 + 200);
                float ty = (float)(rand() % 480 + 50);
                int ttype = rand() % 7;

                if (vBenefits.empty()) problem = false;
                else
                {
                    for (int i = 0; i < vBenefits.size(); ++i)
                    {
                        if (tx >= vBenefits[i]->x && tx <= vBenefits[i]->ex + 40.0f
                            && ty >= vBenefits[i]->y && ty <= vBenefits[i]->ey)
                            problem = true;
                        else problem = false;
                    }
                }
                if(!problem)
                    vBenefits.push_back(iCreate(static_cast<types>(ttype), tx, ty));
            }
        }

        //////////////////////////////////////////////////////////////////////////////////////////

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

        Draw->DrawBitmap(bmpBase, D2D1::RectF(10.0f, (float)(clHeight / 2), 130.0f, (float)(clHeight / 2 + 61)));
        Draw->DrawBitmap(bmpPlatform, D2D1::RectF(0.0f, (float)(clHeight / 2 + 61), 150.0f, (float)(clHeight / 2 + 150)));
        if (Head && bmpHead)
            if(Head->forward)
                Draw->DrawBitmap(bmpHead[Head->GetFrame()], D2D1::RectF(Head->x, Head->y, Head->ex, Head->ey));
            else
                Draw->DrawBitmap(bmpHead[75], D2D1::RectF(Head->x, Head->y, Head->ex, Head->ey));
        if (!vChain.empty())
        {
            for (int i = 0; i < vChain.size(); ++i)
                Draw->DrawBitmap(bmpChain, D2D1::RectF(vChain[i]->x, vChain[i]->y, vChain[i]->ex, vChain[i]->ey));
        }
        if (Target)
            Draw->DrawBitmap(bmpTarget, D2D1::RectF(Target->x, Target->y, Target->ex, Target->ey));

        if (!vBenefits.empty())
        {
            for (int i = 0; i < vBenefits.size(); ++i)
            {
                switch (vBenefits[i]->type)
                {
                    case types::big_gold:
                        Draw->DrawBitmap(bmpBigG, D2D1::RectF(vBenefits[i]->x, vBenefits[i]->y, vBenefits[i]->ex, vBenefits[i]->ey));
                        break;

                    case types::big_silver:
                        Draw->DrawBitmap(bmpBigS, D2D1::RectF(vBenefits[i]->x, vBenefits[i]->y, vBenefits[i]->ex, vBenefits[i]->ey));
                        break;

                    case types::mid_gold:
                        Draw->DrawBitmap(bmpMidG, D2D1::RectF(vBenefits[i]->x, vBenefits[i]->y, vBenefits[i]->ex, vBenefits[i]->ey));
                        break;

                    case types::mid_silver:
                        Draw->DrawBitmap(bmpBigS, D2D1::RectF(vBenefits[i]->x, vBenefits[i]->y, vBenefits[i]->ex, vBenefits[i]->ey));
                        break;

                    case types::sm_gold:
                        Draw->DrawBitmap(bmpSmallG, D2D1::RectF(vBenefits[i]->x, vBenefits[i]->y, vBenefits[i]->ex, vBenefits[i]->ey));
                        break;

                    case types::sm_silver:
                        Draw->DrawBitmap(bmpSmallS, D2D1::RectF(vBenefits[i]->x, vBenefits[i]->y, vBenefits[i]->ex, vBenefits[i]->ey));
                        break;

                    case types::bag:
                        Draw->DrawBitmap(bmpBag, D2D1::RectF(vBenefits[i]->x, vBenefits[i]->y, vBenefits[i]->ex, vBenefits[i]->ey));
                        break;
                }
            }
        }

        if (Head && Head->cargo_attached != types::no_type)
        {
            switch (Head->cargo_attached)
            {
            case types::big_gold:
                Draw->DrawBitmap(bmpBigG, D2D1::RectF(Head->ex - 5.0f, Head->y + 10.0f, Head->ex + 35.0f, Head->y + 47.0f));
                break;

            case types::big_silver:
                Draw->DrawBitmap(bmpBigS, D2D1::RectF(Head->ex - 5.0f, Head->y + 10.0f, Head->ex + 35.0f, Head->y + 39.0f));
                break;

            case types::mid_gold:
                Draw->DrawBitmap(bmpMidG, D2D1::RectF(Head->ex - 5.0f, Head->y + 10.0f, Head->ex + 25.0f, Head->y + 44.0f));
                break;

            case types::mid_silver:
                Draw->DrawBitmap(bmpBigS, D2D1::RectF(Head->ex - 5.0f, Head->y + 10.0f, Head->ex + 25.0f, Head->y + 40.0f));
                break;

            case types::sm_gold:
                Draw->DrawBitmap(bmpSmallG, D2D1::RectF(Head->ex - 5.0f, Head->y + 10.0f, Head->ex + 15.0f, Head->y + 42.0f));
                break;

            case types::sm_silver:
                Draw->DrawBitmap(bmpSmallS, D2D1::RectF(Head->ex - 5.0f, Head->y + 10.0f, Head->ex + 15.0f, Head->y + 40.0f));
                break;

            case types::bag:
                Draw->DrawBitmap(bmpBag, D2D1::RectF(Head->ex - 5.0f, Head->y + 10.0f, Head->ex + 25.0f, Head->y + 40.0f));
                break;

            }
        }
        ///////////////////////////////////////////////////////////////

        wchar_t status[100] = L"резултат: ";
        wchar_t add[10] = L"\0";
        wsprintf(add, L"%d", score);
        wcscat_s(status, add);
        
        int text_size = 0;
        for (int i = 0; i < 100; i++)
        {
            if (status[i] != '\0')text_size++;
            else break;
        }

        if (nrmTextFormat && RedTxtBrush && GreenTxtBrush)
        {
            if (score < target_points)
                Draw->DrawText(status, text_size, nrmTextFormat, D2D1::RectF(5.0f, 55.0f, 150.0f, 155.0f), RedTxtBrush);
            else
                Draw->DrawText(status, text_size, nrmTextFormat, D2D1::RectF(5.0f, 55.0f, 150.0f, 155.0f), GreenTxtBrush);
        }

        wcscpy_s(status, L"нужни: ");
        wsprintf(add, L"%d", target_points);
        wcscat_s(status, add);

        wcscat_s(status, L"\n\nниво: ");
        wsprintf(add, L"%d", level);
        wcscat_s(status, add);

        text_size = 0;
        for (int i = 0; i < 100; i++)
        {
            if (status[i] != '\0')text_size++;
            else break;
        }
        if (nrmTextFormat && RedTxtBrush)
            Draw->DrawText(status, text_size, nrmTextFormat, D2D1::RectF(5.0f, 160.0f, 150.0f, 260.0f), RedTxtBrush);

        if (nrmTextFormat && RedTxtBrush)
            Draw->DrawText(current_player, name_size, nrmTextFormat, D2D1::RectF(10.0f, 350.0f, 150.0f, 500.0f), RedTxtBrush);

        wsprintf(add, L"%d", minutes);
        wcscpy_s(status, add);
        
        wsprintf(add, L"%d", seconds - minutes * 60);
        wcscat_s(status, L" : ");
        if (seconds - minutes * 60 < 10)wcscat_s(status, L"0");
        wcscat_s(status, add);
        text_size = 0;
        for (int i = 0; i < 100; i++)
        {
            if (status[i] != '\0')text_size++;
            else break;
        }

        if (nrmTextFormat && RedTxtBrush)
            Draw->DrawText(status, text_size, nrmTextFormat, D2D1::RectF(50.0f, 520.0f, 150.0f, 600.0f), RedTxtBrush);
        Draw->EndDraw();
    }
    /////////////////////////////////////////////////////////////////////

    ReleaseCOM();
    return (int) bMsg.wParam;
}