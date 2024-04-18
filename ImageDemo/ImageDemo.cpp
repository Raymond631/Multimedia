// ImageDemo.cpp : 定义应用程序的入口点。
//

// TODO: 忽略调用fopen()的警告
#define _CRT_SECURE_NO_DEPRECATE

#include "framework.h"
#include "ImageDemo.h"

#define MAX_LOADSTRING 100

// TODO: 宏定义
#define IMAGE_WIDTH 640
#define IMAGE_HIGHT 480
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

static int buffer_1[IMAGE_HIGHT][IMAGE_WIDTH], buffer_2[IMAGE_HIGHT][IMAGE_WIDTH];//添加代码，用于计算波能
static int tmp[IMAGE_HIGHT][IMAGE_WIDTH]; // 添加代码，用于交换波能矩阵
static COLOR det_image[IMAGE_HIGHT][IMAGE_WIDTH];// 添加代码，要显示的目标图像
static FILE* ifp; //添加代码，文件指针
static const char* filename = "stone.bmp";//添加代码，源图像文件，本程序仅限处理24位BMP图像
static unsigned char imagebuf[IMAGE_WIDTH * IMAGE_HIGHT * 3 + 100]; //添加代码，内存，用于储源图像文件
static BITMAPFILEHEADER* pbmfh; //添加代码，位图文件头结构
static BITMAPINFO* pbmi; //添加代码，位图信息头结构
static BYTE* pBits; //添加代码，源位图数据指针
static int cxDib, cyDib; //添加代码，存储实际打开的图像的宽度和高度
static TCHAR szFileName[MAX_PATH], szTitleName[MAX_PATH];

// TODO: 用于加载第1个BMP图像的全局变量
BITMAPINFOHEADER bi;
HBITMAP hbmp;
BITMAP bmp;
HANDLE hDib;
BYTE* lpbitmap = NULL;
// TODO: 用于加载第2个BMP图像的全局变量
BITMAPINFOHEADER bi2;
HBITMAP hbmp2;
BITMAP bmp2;
HANDLE hDib2;
BYTE* lpbitmap2 = NULL;


// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
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

    // TODO: 加载第1个BMP图像
    if (LoadImage(hInstance, L"background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE) == NULL)
    {
        MessageBox(NULL, L"加载图像错误", L"message", NULL);
    }
    else
    {
        hbmp =
            (HBITMAP)LoadImage(hInstance, L"background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }
    GetObject(hbmp, sizeof(BITMAP), &bmp);
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = bmp.bmPlanes;
    bi.biBitCount = bmp.bmBitsPixel;
    bi.biCompression = bmp.bmType;
    bi.biSizeImage = bmp.bmWidth * bmp.bmHeight * bmp.bmBitsPixel / 8;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrImportant = 0;
    hDib = GlobalAlloc(GHND, bi.biSizeImage);
    lpbitmap = (BYTE*)GlobalLock(hDib);

    // TODO: 加载第2个BMP图像
    if (LoadImage(hInstance, L"蒙娜丽莎.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE) == NULL)
    {
        MessageBox(NULL, L"加载图像错误", L"message", NULL);
    }
    else
    {
        hbmp2 =
            (HBITMAP)LoadImage(hInstance, L"蒙娜丽莎.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }
    GetObject(hbmp2, sizeof(BITMAP), &bmp2);
    bi2.biSize = sizeof(BITMAPINFOHEADER);
    bi2.biWidth = bmp2.bmWidth;
    bi2.biHeight = bmp2.bmHeight;
    bi2.biPlanes = bmp2.bmPlanes;
    bi2.biBitCount = bmp2.bmBitsPixel;
    bi2.biCompression = bmp2.bmType;
    bi2.biSizeImage = bmp2.bmWidth * bmp2.bmHeight * bmp2.bmBitsPixel / 8;
    bi2.biXPelsPerMeter = 0;
    bi2.biYPelsPerMeter = 0;
    bi2.biClrImportant = 0;
    hDib2 = GlobalAlloc(GHND, bi2.biSizeImage);
    lpbitmap2 = (BYTE*)GlobalLock(hDib2);

    //波能初始化
    for (int y = 0; y < 480; y++)
        for (int x = 0; x < 640; x++)
        {
            buffer_1[y][x] = 0;
            buffer_2[y][x] = 0;
        }
    //进入消息循环之前，加载图像文件
    ifp = fopen(filename, "r");
    fread(imagebuf, 925696, 1, ifp);
    pbmfh = (BITMAPFILEHEADER*)imagebuf;
    pbmi = (BITMAPINFO*)(pbmfh + 1);
    pBits = (BYTE*)pbmfh + pbmfh->bfOffBits;
    if (pbmi->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
    {
        cxDib = ((BITMAPCOREHEADER*)pbmi)->bcWidth;
        cyDib = ((BITMAPCOREHEADER*)pbmi)->bcHeight;
    }
    else
    {
        cxDib = pbmi->bmiHeader.biWidth;
        cyDib = abs(pbmi->bmiHeader.biHeight);
    }



    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_IMAGEDEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IMAGEDEMO));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IMAGEDEMO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_IMAGEDEMO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 0, 0, 1900, 1000, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // TODO: 设置定时器
   SetTimer(hWnd, ID_TIMER, 20, NULL); // 33ms刷新一次，相当于30帧

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
    HDC hdc;
    // 加入显示间隙
    LONG height = bi.biHeight + 2;
    LONG width = bi.biWidth + 2;

    switch (message)
    {
    case WM_TIMER:
        hdc = GetDC(hWnd); //获取窗口设备句柄

        disturb();//投石入水
        NextFrameWaveEnerge(); //计算波能传递与衰减
        RenderRipple();//渲染目标图像
        //显示目标图像
        SetDIBitsToDevice(hdc,
            width * 3, // xDst
            height, // yDst
            cxDib, // cxSrc
            cyDib, // cySrc
            0, // xSrc
            0, // ySrc
            0, // first scan line
            cyDib, // number of scan lines
            det_image, //目标图像数据指针
            pbmi,
            DIB_RGB_COLORS);
        ReleaseDC(hWnd, hdc);
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...

            // 拷贝第1幅位图
            GetDIBits(hdc,
                hbmp,
                0,
                (UINT)bmp.bmHeight,
                lpbitmap,
                (BITMAPINFO*)&bi,
                DIB_RGB_COLORS);
            // 拷贝第2幅位图
            GetDIBits(hdc,
                hbmp2,
                0,
                (UINT)bmp2.bmHeight,
                lpbitmap2,
                (BITMAPINFO*)&bi2,
                DIB_RGB_COLORS);

            
            for (int i = 0; i < bi.biHeight; i++) {
                for (int j = 0; j < bi.biWidth; j++) {
                    BYTE r = *(lpbitmap + 2 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
                    BYTE g = *(lpbitmap + 1 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
                    BYTE b = *(lpbitmap + 0 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
                    // RGB单色显示
                    SetPixel(hdc, j, i, RGB(r, 0, 0));
                    SetPixel(hdc, j + width, i, RGB(0, g, 0));
                    SetPixel(hdc, j + width * 2, i, RGB(0, 0, b));
                    // 灰度显示
                    BYTE average = (r + g + b) / 3;
                    BYTE y = r * 0.299 + g * 0.58 + b * 0.114;
                    SetPixel(hdc, j + width * 3, i, RGB(g, g, g));
                    SetPixel(hdc, j + width * 4, i, RGB(average, average, average));
                    SetPixel(hdc, j + width * 5, i, RGB(y, y, y));
                }
            }
                
            // 倒立显示
            for (int i = 0; i < bi.biHeight; i++) {
                for (int j = 0; j < bi.biWidth; j++) {
                    BYTE r = *(lpbitmap + 2 + j * 4 + i * bi.biWidth * 4);
                    BYTE g = *(lpbitmap + 1 + j * 4 + i * bi.biWidth * 4);
                    BYTE b = *(lpbitmap + 0 + j * 4 + i * bi.biWidth * 4);
                    SetPixel(hdc, j, i + height, RGB(r, g, b));
                }
            }
            
            // 正立显示
            for (int i = 0; i < bi.biHeight; i++) {
                for (int j = 0; j < bi.biWidth; j++) {
                    BYTE r = *(lpbitmap + 2 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
                    BYTE g = *(lpbitmap + 1 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
                    BYTE b = *(lpbitmap + 0 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
                    SetPixel(hdc, j + width, i + height, RGB(r, g, b));
                }
            }

            // 合成两幅图像并显示合成图像
            for (int i = 0; i < bi.biHeight; i++) {
                for (int j = 0; j < bi.biWidth; j++) {
                    // 读取第一幅图像的RGB分量
                    BYTE r1 = *(lpbitmap + 2 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
                    BYTE g1 = *(lpbitmap + 1 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
                    BYTE b1 = *(lpbitmap + 0 + j * 4 + (bi.biHeight - i - 1) * bi.biWidth * 4);
                    // 读取第二幅图像的RGB分量
                    BYTE r2 = *(lpbitmap2 + 2 + j * 4 + (bi2.biHeight - i - 1) * bi2.biWidth * 4);
                    BYTE g2 = *(lpbitmap2 + 1 + j * 4 + (bi2.biHeight - i - 1) * bi2.biWidth * 4);
                    BYTE b2 = *(lpbitmap2 + 0 + j * 4 + (bi2.biHeight - i - 1) * bi2.biWidth * 4);
                    // 两幅图像的对应分量按比例叠加，α=0.5
                    BYTE r = r1 / 2 + r2 / 2;
                    BYTE g = g1 / 2 + g2 / 2;
                    BYTE b = b1 / 2 + b2 / 2;
                    // 显示合成图像
                    SetPixel(hdc, j + width * 2, i + height, RGB(r, g, b));
                }
            }

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

void disturb()
{
    int x, y, stonesize, stoneweight;
    x = rand();
    y = 100;
    // TODO: 修改参数
    stonesize = 20;
    stoneweight = 50;
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
            buffer_2[y][x] = ((buffer_1[y][x - 1] + buffer_1[y][x + 1] + buffer_1[y - 1][x] + buffer_1[y + 1][x]) >>
                1) - buffer_2[y][x];
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
            // 处理图像偏移
            det_image[y][x].b = *(pBits + 0 + (x + xoff) * 3 + (y + yoff) * cxDib * 3);
            det_image[y][x].g = *(pBits + 1 + (x + xoff) * 3 + (y + yoff) * cxDib * 3);
            det_image[y][x].r = *(pBits + 2 + (x + xoff) * 3 + (y + yoff) * cxDib * 3);
        }
}


