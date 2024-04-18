// VideoDemo.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "VideoDemo.h"

#define MAX_LOADSTRING 100
// TODO:
#define IMAGE_WIDTH    352
#define IMAGE_HIGHT    288
typedef struct COLOR
{
    BYTE b;
    BYTE g;
    BYTE r;
}RGBCOLOR;

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// TODO: 视频1
static FILE* ifp;  //file pointer
static char* filename = NULL;
static BYTE      mybuf[45621248]; //arry,store the video file
static BYTE* pBity, * pBitu, * pBitv;
static int       y[288][352], u[144][176], v[144][176];
// 视频2
static FILE* ifp2;
static char* filename2 = NULL;
static BYTE mybuf2[45621248];
static BYTE* pBity2, * pBitu2, * pBitv2;
static int y2[288][352], u2[144][176], v2[144][176];
// 背景图像
static FILE* ifpback;
static char* filenameback = NULL;
static unsigned char mybufback[IMAGE_WIDTH * IMAGE_HIGHT * 3 + 100];
static BITMAPFILEHEADER* pbmfh;
static BITMAPINFO* pbmi;
static BYTE* pbits;
static int cxDib, cyDib;
// 融合图像
static COLOR det_image[IMAGE_HIGHT][IMAGE_WIDTH];//要显示的目标图像
static COLOR det_image2[IMAGE_HIGHT][IMAGE_WIDTH];//要显示的目标图像(视频拼接)
static int n = 0;
// 水波纹
static int buffer_1[IMAGE_HIGHT][IMAGE_WIDTH], buffer_2[IMAGE_HIGHT][IMAGE_WIDTH];//添加代码，用于计算波能
static int tmp[IMAGE_HIGHT][IMAGE_WIDTH]; // 添加代码，用于交换波能矩阵
// 任务编号
int task = 1;


// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL OpenFile(char** strPath);
void NextFrameWaveEnerge();
void RenderRipple();
void disturb();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
    //波能初始化
    for (int y = 0; y < IMAGE_HIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++)
        {
            buffer_1[y][x] = 0;
            buffer_2[y][x] = 0;
        }
    }

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VIDEODEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VIDEODEMO));

    MSG msg;

    // 主消息循环:
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



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIDEODEMO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VIDEODEMO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 800, 400, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   //// TODO: 设置定时器
   //SetTimer(hWnd, ID_TIMER, 50, NULL);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int xy = 2 * n;
    HDC hdc;
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 分析菜单选择:
        switch (wmId)
        {
        case ID_VIDEO_1:
            if (OpenFile(&filename)) {
                //打开第1个视频文件
                fopen_s(&ifp, filename, "r");
                fread(mybuf, 45621248, 1, ifp);
                pBity = mybuf;
                pBitu = mybuf + 352 * 288;
                pBitv = mybuf + 352 * 288 + 176 * 144;
            }
            break;
        case ID_VIDEO_2:
            if (OpenFile(&filename2)) {
                //打开第2个视频文件
                fopen_s(&ifp2, filename2, "r");
                fread(mybuf2, 45621248, 1, ifp2);
                pBity2 = mybuf2;
                pBitu2 = mybuf2 + 352 * 288;
                pBitv2 = mybuf2 + 352 * 288 + 176 * 144;
            }
            break;
        case ID_BMP:
            if (OpenFile(&filenameback)) {
                // 打开图像文件
                fopen_s(&ifpback, filenameback, "r");
                fread(mybufback, 307200, 1, ifpback);
                pbmfh = (BITMAPFILEHEADER*)mybufback;
                pbmi = (BITMAPINFO*)(pbmfh + 1);
                pbits = (BYTE*)pbmfh + pbmfh->bfOffBits;
                cxDib = pbmi->bmiHeader.biWidth;
                cyDib = pbmi->bmiHeader.biHeight;
            }
            break;
        case ID_TASK_1:
            n = 0;
            task = 1;
            SetTimer(hWnd, ID_TIMER, 50, NULL);
            break;
        case ID_TASK_2:
            n = 0;
            task = 2;
            SetTimer(hWnd, ID_TIMER, 50, NULL);
            break;
        case ID_TASK_3:
            n = 0;
            task = 3;
            SetTimer(hWnd, ID_TIMER, 50, NULL);
            break;
        case ID_TASK_4:
            n = 0;
            task = 4;
            SetTimer(hWnd, ID_TIMER, 50, NULL);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_TIMER:
        switch (task) {
        case 1:
            hdc = GetDC(hWnd);
            n = n + 1;
            if (n > 299) n = 0;
            pBity = pBity + (352 * 288 + 2 * (176 * 144)) * n;
            pBitu = pBity + 352 * 288;
            pBitv = pBitu + 176 * 144;
            //read a new frame of the yuv file
            for (int i = 0; i < 144; i++)
                for (int j = 0; j < 176; j++)
                {
                    u[i][j] = *(pBitu + j + 176 * (i));
                    v[i][j] = *(pBitv + j + 176 * (i));
                }
            //read y，and translate yuv int rgb and display the pixel
            for (int i = 0; i < 288; i++)
                for (int j = 0; j < 352; j++)
                {
                    //read y
                    y[i][j] = *(pBity + j + (i) * 352);
                    //translate
                    int r = (298 * (y[i][j] - 16) + 409 * (v[i / 2][j / 2] - 128) + 128) >> 8;
                    if (r < 0) r = 0;
                    if (r > 255) r = 255;
                    int g = (298 * (y[i][j] - 16) - 100 * (u[i / 2][j / 2] - 128) - 208 * (v[i / 2][j / 2] - 128) + 128) >> 8;
                    if (g < 0) g = 0;
                    if (g > 255) g = 255;
                    int b = (298 * (y[i][j] - 16) + 516 * (u[i / 2][j / 2] - 128) + 128) >> 8;
                    if (b < 0) b = 0;
                    if (b > 255) b = 255;
                    // 取字幕图标图像的像素值
                    int rback = *(pbits + 2 + j * 3 + (cyDib - i - 1) * cxDib * 3);
                    int gback = *(pbits + 1 + j * 3 + (cyDib - i - 1) * cxDib * 3);
                    int bback = *(pbits + 0 + j * 3 + (cyDib - i - 1) * cxDib * 3);
                    // 如果当前字幕图标图像像素值是黑色，就传送视频像素值到目标图像
                    if (rback == 0 && gback == 0 && bback == 0)
                    {
                        det_image[288 - i - 1][j].r = r;
                        det_image[288 - i - 1][j].g = g;
                        det_image[288 - i - 1][j].b = b;
                    }
                    else if (j< xy)
                    {
                        det_image[288 - i - 1][j].r = 100;
                        det_image[288 - i - 1][j].g = 150;
                        det_image[288 - i - 1][j].b = 200;
                    }
                    else//否则，就传送字幕图标图像的当前像素值到目标图像
                    {
                        det_image[288 - i - 1][j].r = rback;
                        det_image[288 - i - 1][j].g = gback;
                        det_image[288 - i - 1][j].b = bback;
                    }
                }
            SetDIBitsToDevice(hdc,
                30,
                20,
                352,
                288,
                0,
                0,
                0,
                288,
                det_image,
                pbmi,
                DIB_RGB_COLORS);
            pBity = mybuf; // let pBity to point at the first place of the file
            ReleaseDC(hWnd, hdc);
            break;
        case 2:
            hdc = GetDC(hWnd);
            n = n + 1;
            if (n > 299) n = 0;
            //利用节拍计数器n控制每一帧图像的指针
            pBity = pBity + (352 * 288 + 2 * (176 * 144)) * n;
            pBitu = pBity + 352 * 288;
            pBitv = pBitu + 176 * 144;
            pBity2 = pBity2 + (352 * 288 + 2 * (176 * 144)) * n;
            pBitu2 = pBity2 + 352 * 288;
            pBitv2 = pBitu2 + 176 * 144;
            //先读取UV数据
            for (int i = 0; i < 144; i++)
                for (int j = 0; j < 176; j++)
                {
                    u[i][j] = *(pBitu + j + 176 * (i));
                    v[i][j] = *(pBitv + j + 176 * (i));
                    u2[i][j] = *(pBitu2 + j + 176 * (i));
                    v2[i][j] = *(pBitv2 + j + 176 * (i));
                }
            //再读取Y值，并立即进行转码、融合处理
            for (int i = 0; i < 288; i++)
                for (int j = 0; j < 352; j++)
                {
                    //读取Y值
                    y[i][j] = *(pBity + j + (i) * 352);
                    y2[i][j] = *(pBity2 + j + (i) * 352);
                    //把当前像素的YUV值转码为RGB值
                    int r = (298 * (y[i][j] - 16) + 409 * (v[i / 2][j / 2] - 128) + 128) >> 8;
                    if (r < 0) r = 0;
                    if (r > 255) r = 255;
                    int g = (298 * (y[i][j] - 16) - 100 * (u[i / 2][j / 2] - 128) - 208 * (v[i / 2][j / 2] - 128) + 128) >> 8;
                    if (g < 0) g = 0;
                    if (g > 255) g = 255;
                    int b = (298 * (y[i][j] - 16) + 516 * (u[i / 2][j / 2] - 128) + 128) >> 8;
                    if (b < 0) b = 0;
                    if (b > 255) b = 255;
                    //处理第2个视频
                    int r2 = (298 * (y2[i][j] - 16) + 409 * (v2[i / 2][j / 2] - 128) + 128) >> 8;
                    if (r2 < 0) r2 = 0;
                    if (r2 > 255) r2 = 255;
                    int g2 = (298 * (y2[i][j] - 16) - 100 * (u2[i / 2][j / 2] - 128) - 208 * (v2[i / 2][j / 2] - 128) + 128) >> 8;
                    if (g2 < 0) g2 = 0;
                    if (g2 > 255) g2 = 255;
                    int b2 = (298 * (y2[i][j] - 16) + 516 * (u2[i / 2][j / 2] - 128) + 128) >> 8;
                    if (b2 < 0) b2 = 0;
                    if (b2 > 255) b2 = 255;
                    //定义透明度参数
                    double para = n / 300.0;
                    //两帧图像的当前像素的融合，结果放入目标图像矩阵中
                    det_image[288 - i - 1][j].r = r * (1 - para) + r2 * para;
                    det_image[288 - i - 1][j].g = g * (1 - para) + g2 * para;
                    det_image[288 - i - 1][j].b = b * (1 - para) + b2 * para;
                }
            SetDIBitsToDevice(hdc,
                30,
                20,
                352,
                288,
                0,
                0,
                0,
                288,
                det_image,
                pbmi,
                DIB_RGB_COLORS);
            // 指针pBity回到视频起始地址
            pBity = mybuf;
            pBity2 = mybuf2;
            ReleaseDC(hWnd, hdc);
            break;
        case 3:
            hdc = GetDC(hWnd);
            n = n + 1;
            if (n > 299) n = 0;
            //利用节拍计数器n控制每一帧图像的指针
            pBity = pBity + (352 * 288 + 2 * (176 * 144)) * n;
            pBitu = pBity + 352 * 288;
            pBitv = pBitu + 176 * 144;
            pBity2 = pBity2 + (352 * 288 + 2 * (176 * 144)) * n;
            pBitu2 = pBity2 + 352 * 288;
            pBitv2 = pBitu2 + 176 * 144;
            //先读取UV数据
            for (int i = 0; i < 144; i++)
                for (int j = 0; j < 176; j++)
                {
                    u[i][j] = *(pBitu + j + 176 * (i));
                    v[i][j] = *(pBitv + j + 176 * (i));
                    u2[i][j] = *(pBitu2 + j + 176 * (i));
                    v2[i][j] = *(pBitv2 + j + 176 * (i));
                }
            //再读取Y值，并立即进行转码、拼接处理
            for (int i = 0; i < 288; i++)
                for (int j = 0; j < 352; j++)
                {
                    //读取Y值
                    y[i][j] = *(pBity + j + (i) * 352);
                    y2[i][j] = *(pBity2 + j + (i) * 352);
                    //把当前像素的YUV值转码为RGB值
                    int r = (298 * (y[i][j] - 16) + 409 * (v[i / 2][j / 2] - 128) + 128) >> 8;
                    if (r < 0) r = 0;
                    if (r > 255) r = 255;
                    int g = (298 * (y[i][j] - 16) - 100 * (u[i / 2][j / 2] - 128) - 208 * (v[i / 2][j / 2] - 128) + 128) >> 8;
                    if (g < 0) g = 0;
                    if (g > 255) g = 255;
                    int b = (298 * (y[i][j] - 16) + 516 * (u[i / 2][j / 2] - 128) + 128) >> 8;
                    if (b < 0) b = 0;
                    if (b > 255) b = 255;
                    //处理第2个视频
                    int r2 = (298 * (y2[i][j] - 16) + 409 * (v2[i / 2][j / 2] - 128) + 128) >> 8;
                    if (r2 < 0) r2 = 0;
                    if (r2 > 255) r2 = 255;
                    int g2 = (298 * (y2[i][j] - 16) - 100 * (u2[i / 2][j / 2] - 128) - 208 * (v2[i / 2][j / 2] - 128) + 128) >> 8;
                    if (g2 < 0) g2 = 0;
                    if (g2 > 255) g2 = 255;
                    int b2 = (298 * (y2[i][j] - 16) + 516 * (u2[i / 2][j / 2] - 128) + 128) >> 8;
                    if (b2 < 0) b2 = 0;
                    if (b2 > 255) b2 = 255;
                    //两帧图像的当前像素的拼接，结果放入目标图像矩阵中
                    det_image[288 - i - 1][j].r = r;
                    det_image[288 - i - 1][j].g = g;
                    det_image[288 - i - 1][j].b = b;
                    // TODO: 因为SetDIBitsToDevice的参数pbmi设定了尺寸，所以不能放到det_image2这1个数组里
                    det_image2[288 - i - 1][j].r = r2;
                    det_image2[288 - i - 1][j].g = g2;
                    det_image2[288 - i - 1][j].b = b2;
                }
            SetDIBitsToDevice(hdc,
                30,
                20,
                352,
                288,
                0,
                0,
                0,
                288,
                det_image,
                pbmi,
                DIB_RGB_COLORS);
            SetDIBitsToDevice(hdc,
                30 + 352,
                20,
                352,
                288,
                0,
                0,
                0,
                288,
                det_image2,
                pbmi,
                DIB_RGB_COLORS);
            // 指针pBity回到视频起始地址
            pBity = mybuf;
            pBity2 = mybuf2;
            ReleaseDC(hWnd, hdc);
            break;
        case 4:
            hdc = GetDC(hWnd);
            n = n + 1;
            if (n > 299) n = 0;
            pBity = pBity + (352 * 288 + 2 * (176 * 144)) * n;
            pBitu = pBity + 352 * 288;
            pBitv = pBitu + 176 * 144;
            //read a new frame of the yuv file
            for (int i = 0; i < 144; i++)
                for (int j = 0; j < 176; j++)
                {
                    u[i][j] = *(pBitu + j + 176 * (i));
                    v[i][j] = *(pBitv + j + 176 * (i));
                }
            //read y，and translate yuv int rgb and display the pixel
            for (int i = 0; i < 288; i++)
                for (int j = 0; j < 352; j++)
                {
                    //read y
                    y[i][j] = *(pBity + j + (i) * 352);
                    //translate
                    int r = (298 * (y[i][j] - 16) + 409 * (v[i / 2][j / 2] - 128) + 128) >> 8;
                    if (r < 0) r = 0;
                    if (r > 255) r = 255;
                    int g = (298 * (y[i][j] - 16) - 100 * (u[i / 2][j / 2] - 128) - 208 * (v[i / 2][j / 2] - 128) + 128) >> 8;
                    if (g < 0) g = 0;
                    if (g > 255) g = 255;
                    int b = (298 * (y[i][j] - 16) + 516 * (u[i / 2][j / 2] - 128) + 128) >> 8;
                    if (b < 0) b = 0;
                    if (b > 255) b = 255;

                    det_image[288 - i - 1][j].r = r;
                    det_image[288 - i - 1][j].g = g;
                    det_image[288 - i - 1][j].b = b;
                }
            disturb();//投石入水
            NextFrameWaveEnerge(); //计算波能传递与衰减
            RenderRipple();//渲染目标图像
            SetDIBitsToDevice(hdc,
                30,
                20,
                352,
                288,
                0,
                0,
                0,
                288,
                det_image,
                pbmi,
                DIB_RGB_COLORS);
            pBity = mybuf; // let pBity to point at the first place of the file
            ReleaseDC(hWnd, hdc);

            break;
        }

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
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

BOOL OpenFile(char** strPath)
{
    OPENFILENAME ofn;
    WCHAR szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (GetOpenFileName(&ofn) == TRUE) {
        // TODO: WCHAR 转 CHAR
        int bufSize = WideCharToMultiByte(CP_ACP, NULL, szFile, -1, NULL, 0, NULL, FALSE);
        *strPath = new char[bufSize];
        WideCharToMultiByte(CP_ACP, NULL, szFile, -1, *strPath, bufSize, NULL, FALSE);
        return true;
    }
    else {
        return false;
    }
}


void disturb()
{
    int x, y, stonesize, stoneweight;
    x = rand();
    y = 200;
    // TODO: 修改参数
    stonesize = 20;
    stoneweight = 200;
    // 突破边界不处理
    if ((x >= cxDib - stonesize) ||
        (x < stonesize) ||
        (y >= cyDib - stonesize) ||
        (y < stonesize))
        return;
    for (int posy = y - stonesize; posy < y + stonesize; posy++)
        for (int posx = x - stonesize; posx < x + stonesize; posx++)
        {
            if ((posx - x) * (posx - x) + (posy - y) * (posy - y) < stonesize * stonesize)
            {
                buffer_1[posy][posx] += stoneweight;
            }
        }
}

void NextFrameWaveEnerge()
{
    for (int y = 1; y < cyDib - 1; y++)
        for (int x = 1; x < cxDib - 1; x++)
        {
            // 公式：7-6
            buffer_2[y][x] = ((buffer_1[y][x - 1] + buffer_1[y][x + 1] + buffer_1[y - 1][x] + buffer_1[y + 1][x]) >> 1) - buffer_2[y][x];
            // 波能衰减，衰减因子为1/32
            buffer_2[y][x] -= buffer_2[y][x] >> 5;
        }
    //exchange the buffer_1 and buffer_2
    for (int y = 1; y < cyDib; y++)
        for (int x = 1; x < cxDib; x++)
            tmp[y][x] = buffer_1[y][x];
    for (int y = 1; y < cyDib; y++)
        for (int x = 1; x < cxDib; x++)
            buffer_1[y][x] = buffer_2[y][x];
    for (int y = 1; y < cyDib; y++)
        for (int x = 1; x < cxDib; x++)
            buffer_2[y][x] = tmp[y][x];
}

void RenderRipple()
{
    for (int y = 0; y < cyDib - 1; y++)
        for (int x = 0; x < cxDib - 1; x++)
        {
            // 计算偏移
            int xoff = buffer_2[y][x - 1] - buffer_2[y][x + 1];
            int yoff = buffer_2[y - 1][x] - buffer_2[y + 1][x];
            // 边界处理
            if (xoff >= cxDib) xoff = cxDib - 1;
            if (xoff < 0) xoff = 0;
            if (yoff >= cyDib) yoff = cyDib - 1;
            if (yoff < 0) yoff = 0;
            // TODO: 处理图像偏移,和实验二不一样
            det_image[y][x].b = det_image[y + yoff][x + xoff].b;
            det_image[y][x].g = det_image[y + yoff][x + xoff].g;
            det_image[y][x].r = det_image[y + yoff][x + xoff].r;
        }
}

