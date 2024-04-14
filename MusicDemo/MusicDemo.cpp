// MusicDemo.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "MusicDemo.h"

#define MAX_LOADSTRING 100

using namespace std;

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
// TODO: 全局变量
IMFSourceResolver* m_pSourceResolver = NULL;
IUnknown* pSource = NULL;
IMFMediaSource* m_pSource;
IMFTopology* m_pTopology = NULL;
IMFPresentationDescriptor* m_pSourcePD = NULL;
DWORD cSourceStreams = 0;
IMFTopologyNode* pSourceNode = NULL;
IMFTopologyNode* pOutputNode = NULL;
IMFActivate* pSinkActivate = NULL;
IMFStreamDescriptor* pSD = NULL;
IMFMediaTypeHandler* pHandler = NULL;
IMFActivate* pActivate = NULL;
PROPVARIANT var;
WCHAR* wszSourceFile1 = NULL;
WCHAR* wszSourceFile2 = NULL;
WCHAR* wszSourceFile3 = NULL;
IMFMediaSession* m_pSession1;
IMFMediaSession* m_pSession2;
IMFMediaSession* m_pSession3;
HRESULT hr1;
HRESULT hr2;
HRESULT hr3;

IMFSourceReader* pReader = NULL;
IMFSourceReader* pReaderAnother = NULL;
HANDLE hFile = INVALID_HANDLE_VALUE;
const WCHAR* wszTargetFile = L"out.wav";
const LONG MAX_AUDIO_DURATION_MSEC = 246000; // 90 seconds


// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
// TODO: 函数声明
void CreateMediaSession(HWND, LPCWSTR, HRESULT, IMFMediaSession**);
void OnFileOpen(HWND, WCHAR**);
void TransCode(int type);
HRESULT WriteWaveFile(IMFSourceReader*, IMFSourceReader*, HANDLE, LONG, int);
HRESULT ConfigureAudioStream(IMFSourceReader*, IMFMediaType**);
HRESULT WriteWaveHeader(HANDLE, IMFMediaType*, DWORD*);
HRESULT WriteToFile(HANDLE, void*, DWORD);
DWORD   CalculateMaxAudioDataSize(IMFMediaType*, DWORD, DWORD);
HRESULT WriteWaveData(HANDLE, IMFSourceReader*, IMFSourceReader*, DWORD, DWORD, DWORD*, DWORD*, int);
HRESULT FixUpChunkSizes(HANDLE, DWORD, DWORD);

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MUSICDEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MUSICDEMO));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MUSICDEMO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MUSICDEMO);
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 0, 0, 500, 500, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

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
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            // TODO: 任务1
            case ID_FILE_INPUT_1:
                OnFileOpen(hWnd, &wszSourceFile1);
                break;
            case ID_FILE_INPUT_2:
                OnFileOpen(hWnd, &wszSourceFile2);
                break;
            case ID_FILE_INPUT_3:
                OnFileOpen(hWnd, &wszSourceFile3);
                break;
            case ID_PLAY_1:
                CreateMediaSession(hWnd, wszSourceFile1, hr1, &m_pSession1);
                PropVariantInit(&var);
                hr1 = m_pSession1->Start(NULL, &var);
                PropVariantClear(&var);
                break;
            case ID_PLAY_2:
                CreateMediaSession(hWnd, wszSourceFile2, hr2, &m_pSession2);
                PropVariantInit(&var);
                hr2 = m_pSession2->Start(NULL, &var);
                PropVariantClear(&var);
                break;
            case ID_PLAY_3:
                CreateMediaSession(hWnd, wszSourceFile3, hr3, &m_pSession3);
                PropVariantInit(&var);
                hr3 = m_pSession3->Start(NULL, &var);
                PropVariantClear(&var);
                break;
            case ID_PAUSE_1:
                hr1 = m_pSession1->Pause();
                break;
            case ID_PAUSE_2:
                hr2 = m_pSession2->Pause();
                break;
            case ID_PAUSE_3:
                hr3 = m_pSession3->Pause();
                break;

            // TODO: 任务2
            case ID_INPUT_1:
                OnFileOpen(hWnd, &wszSourceFile1);
                break;
            case ID_INPUT_2:
                OnFileOpen(hWnd, &wszSourceFile2);
                break;
            case ID_TRANSCODE:
                TransCode(1);
                PlaySound(L"out.wav", NULL, SND_ASYNC);
                break;
            
            // TODO: 任务3
            case ID_MUTE:
                TransCode(2);
                PlaySound(L"out.wav", NULL, SND_ASYNC);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
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

// TODO: 自定义函数
void CreateMediaSession(HWND hWnd, LPCWSTR pwszURL, HRESULT hr, IMFMediaSession** m_pSession)
{
    //HRESULT hr;
    hr = MFStartup(MF_VERSION);
    hr = MFCreateMediaSession(NULL, m_pSession);
    hr = MFCreateSourceResolver(&m_pSourceResolver);
    MF_OBJECT_TYPE  ObjectType = MF_OBJECT_INVALID;

    hr = m_pSourceResolver->CreateObjectFromURL(
        pwszURL,                    // URL of the source.
        MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
        NULL,                       // Optional property store.
        &ObjectType,                // Receives the created object type. 
        &pSource                    // Receives a pointer to the media source.
    );
    hr = pSource->QueryInterface(IID_PPV_ARGS(&m_pSource));
    // Create the presentation descriptor for the media source.
    hr = m_pSource->CreatePresentationDescriptor(&m_pSourcePD);

    // Create a new topology.
    hr = MFCreateTopology(&m_pTopology);

    // Get the number of streams in the media source.
    hr = m_pSourcePD->GetStreamDescriptorCount(&cSourceStreams);

    // For each stream, create the topology nodes and add them to the topology.
    for (DWORD i = 0; i < cSourceStreams; i++)
    {
        BOOL fSelected = FALSE;

        hr = m_pSourcePD->GetStreamDescriptorByIndex(i, &fSelected, &pSD);
        hr = pSD->GetMediaTypeHandler(&pHandler);

        GUID guidMajorType;
        hr = pHandler->GetMajorType(&guidMajorType);
        if (MFMediaType_Audio == guidMajorType)
        {
            // Create the audio renderer.
            hr = MFCreateAudioRendererActivate(&pActivate);
        }
        else if (MFMediaType_Video == guidMajorType)
        {
            // Create the video renderer.
            hr = MFCreateVideoRendererActivate(hWnd, &pActivate);
        }

        hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pSourceNode);
        hr = pSourceNode->SetUnknown(MF_TOPONODE_SOURCE, m_pSource);
        hr = pSourceNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, m_pSourcePD);
        hr = pSourceNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
        hr = m_pTopology->AddNode(pSourceNode);

        hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pOutputNode);
        hr = pOutputNode->SetObject(pActivate);
        hr = pOutputNode->SetUINT32(MF_TOPONODE_STREAMID, 0);
        hr = pOutputNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
        hr = m_pTopology->AddNode(pOutputNode);

        // Connect the source node to the output node.
        hr = pSourceNode->ConnectOutput(0, pOutputNode, 0);

    }

    (*m_pSession)->SetTopology(0, m_pTopology);
}

void OnFileOpen(HWND hwnd, WCHAR** wszSourceFile)
{
    HRESULT hr = S_OK;

    IFileOpenDialog* pFileOpen = NULL;
    IShellItem* pItem = NULL;

    // Create the FileOpenDialog object.
    hr = CoCreateInstance(
        __uuidof(FileOpenDialog),
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFileOpen)
    );
    if (FAILED(hr)) { goto done; }


    hr = pFileOpen->SetTitle(L"please select a source media file");
    if (FAILED(hr)) { goto done; }


    // Show the file-open dialog.
    hr = pFileOpen->Show(hwnd);

    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        // User cancelled.
        hr = S_OK;
        goto done;
    }
    if (FAILED(hr)) { goto done; }

    // Get the file name from the dialog.
    hr = pFileOpen->GetResult(&pItem);
    if (FAILED(hr)) { goto done; }

    hr = pItem->GetDisplayName(SIGDN_URL, wszSourceFile);
    if (FAILED(hr)) { goto done; }

done:
    if (FAILED(hr))
    {
        MessageBox(hwnd, L"you can not open any file", L"message", NULL);
    }

    if (pItem)
    {
        pItem->Release();
    }
    if (pFileOpen)
    {
        pFileOpen->Release();
    }
}




void TransCode(int type)
{
    // TODO: 不能等于S_OK
    HRESULT hr = MFStartup(MF_VERSION);
    HRESULT hr2 = MFStartup(MF_VERSION);

    // Create the source reader to read the input file.
    if (SUCCEEDED(hr))
    {
        hr = MFStartup(MF_VERSION);
        hr = MFCreateSourceReaderFromURL(
            wszSourceFile1,
            NULL,
            &pReader
        );

        if (FAILED(hr))
        {
            return;
        }
    }

    // Create another source reader to read another input file which will be the background sound.
    if (SUCCEEDED(hr2))
    {
        hr2 = MFCreateSourceReaderFromURL(
            wszSourceFile2,
            NULL,
            &pReaderAnother
        );

        if (FAILED(hr2))
        {
            return;
        }
    }

    // open the output wave file to which we write decoded data
    if (SUCCEEDED(hr))
    {
        hFile = CreateFile(
            wszTargetFile,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            0,
            NULL
        );

        if (hFile == INVALID_HANDLE_VALUE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            return;
        }
    }

    // write wave file
    if (SUCCEEDED(hr))
    {
        hr = WriteWaveFile(pReader, pReaderAnother, hFile, MAX_AUDIO_DURATION_MSEC, type);
    }

    if (FAILED(hr))
    {
        return;
    }
    // close file
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }

    SafeRelease(&pReader);
    SafeRelease(&pReaderAnother);
}

HRESULT WriteWaveFile(
    IMFSourceReader* pReader,   // Pointer to the source reader.
    IMFSourceReader* pReaderAnother,   // Pointer to another source reader.
    HANDLE hFile,               // Handle to the output file.
    LONG msecAudioData,          // Maximum amount of audio data to write, in msec.
    int type                     // 数据处理的类型（声道替换 or 静音）
)
{
    HRESULT hr = S_OK;
    HRESULT hr2 = S_OK;

    DWORD cbHeader = 0;         // Size of the WAVE file header, in bytes.
    DWORD cbAudioData = 0;      // Total bytes of PCM audio data written to the file.
    DWORD cbMaxAudioData = 0;
    IMFMediaType* pAudioType = NULL;    // Represents the PCM audio format.
    ////////////////////////////////////////////////////
    //
    DWORD cbHeaderAnother = 0;         // Size of the WAVE file header, in bytes.
    DWORD cbAudioDataAnother = 0;      // Total bytes of PCM audio data written to the file.
    DWORD cbMaxAudioDataAnother = 0;
    IMFMediaType* pAudioTypeAnother = NULL;    // Represents the PCM audio format.
    //
    ////////////////////////////////////////////////////

    // Configure the source reader to get uncompressed PCM audio from the source file.

    hr = ConfigureAudioStream(pReader, &pAudioType);

    hr2 = ConfigureAudioStream(pReaderAnother, &pAudioTypeAnother);

    // Write the WAVE file header.
    if (SUCCEEDED(hr))
    {
        hr = WriteWaveHeader(hFile, pAudioType, &cbHeader);
        cbMaxAudioData = CalculateMaxAudioDataSize(pAudioType, cbHeader, msecAudioData);
        cbMaxAudioDataAnother = CalculateMaxAudioDataSize(pAudioTypeAnother, cbHeaderAnother, msecAudioData);
        // Decode audio data to the file.
        hr = WriteWaveData(hFile, pReader, pReaderAnother, cbMaxAudioData, cbMaxAudioDataAnother, &cbAudioData, &cbAudioDataAnother, type);
    }

    // Fix up the RIFF headers with the correct sizes.
    if (SUCCEEDED(hr))
    {
        hr = FixUpChunkSizes(hFile, cbHeader, cbAudioData);
    }

    SafeRelease(&pAudioType);
    SafeRelease(&pAudioTypeAnother);
    return hr;
}

HRESULT ConfigureAudioStream(
    IMFSourceReader* pReader,   // Pointer to the source reader.
    IMFMediaType** ppPCMAudio   // Receives the audio format.
)
{
    HRESULT hr = S_OK;

    IMFMediaType* pUncompressedAudioType = NULL;
    IMFMediaType* pPartialType = NULL;

    // Create a partial media type that specifies uncompressed PCM audio.

    hr = MFCreateMediaType(&pPartialType);

    if (SUCCEEDED(hr))
    {
        hr = pPartialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    }

    if (SUCCEEDED(hr))
    {
        hr = pPartialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    }

    // Set this type on the source reader. The source reader will
    // load the necessary decoder.
    if (SUCCEEDED(hr))
    {
        hr = pReader->SetCurrentMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            NULL,
            pPartialType
        );
    }

    // Get the complete uncompressed format.
    if (SUCCEEDED(hr))
    {
        hr = pReader->GetCurrentMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            &pUncompressedAudioType
        );
    }

    // Ensure the stream is selected.
    if (SUCCEEDED(hr))
    {
        hr = pReader->SetStreamSelection(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            TRUE
        );
    }

    // Return the PCM format to the caller.
    if (SUCCEEDED(hr))
    {
        *ppPCMAudio = pUncompressedAudioType;
        (*ppPCMAudio)->AddRef();
    }

    SafeRelease(&pUncompressedAudioType);
    SafeRelease(&pPartialType);
    return hr;
}

HRESULT WriteWaveHeader(
    HANDLE hFile,               // Output file.
    IMFMediaType* pMediaType,   // PCM audio format.
    DWORD* pcbWritten           // Receives the size of the header.    
)
{
    HRESULT hr = S_OK;
    UINT32 cbFormat = 0;

    WAVEFORMATEX* pWav = NULL;

    *pcbWritten = 0;

    // Convert the PCM audio format into a WAVEFORMATEX structure.
    hr = MFCreateWaveFormatExFromMFMediaType(
        pMediaType,
        &pWav,
        &cbFormat
    );


    // Write the 'RIFF' header and the start of the 'fmt ' chunk.

    if (SUCCEEDED(hr))
    {
        DWORD header[] = {
            // RIFF header
            FCC('RIFF'),
            0,
            FCC('WAVE'),
            // Start of 'fmt ' chunk
            FCC('fmt '),
            cbFormat
        };

        DWORD dataHeader[] = { FCC('data'), 0 };

        hr = WriteToFile(hFile, header, sizeof(header));//先写入一部分数据
        //有些置为零，待后面FixUp

// Write the WAVEFORMATEX structure.
        if (SUCCEEDED(hr))
        {
            hr = WriteToFile(hFile, pWav, cbFormat);//注意，接着前面继续写入，所以是紧跟
            //‘fmt ’和cbFormat后继续写入    
        }

        // Write the start of the 'data' chunk

        if (SUCCEEDED(hr))
        {
            hr = WriteToFile(hFile, dataHeader, sizeof(dataHeader));//再接着写入音频数据头部
        }

        if (SUCCEEDED(hr))
        {
            *pcbWritten = sizeof(header) + cbFormat + sizeof(dataHeader);//记录已经写入的字节数
        }
    }


    CoTaskMemFree(pWav);
    return hr;
}

HRESULT WriteToFile(HANDLE hFile, void* p, DWORD cb)
{
    DWORD cbWritten = 0;
    HRESULT hr = S_OK;

    BOOL bResult = WriteFile(hFile, p, cb, &cbWritten, NULL);
    if (!bResult)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}

DWORD CalculateMaxAudioDataSize(
    IMFMediaType* pAudioType,    // The PCM audio format.
    DWORD cbHeader,              // The size of the WAVE file header.
    DWORD msecAudioData          // Maximum duration, in milliseconds.
)
{
    UINT32 cbBlockSize = 0;         // Audio frame size, in bytes.
    UINT32 cbBytesPerSecond = 0;    // Bytes per second.

    // Get the audio block size and number of bytes/second from the audio format.

    cbBlockSize = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_BLOCK_ALIGNMENT, 0);
    cbBytesPerSecond = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 0);

    // Calculate the maximum amount of audio data to write. 
    // This value equals (duration in seconds x bytes/second), but cannot
    // exceed the maximum size of the data chunk in the WAVE file.

    // Size of the desired audio clip in bytes:
    DWORD cbAudioClipSize = (DWORD)MulDiv(cbBytesPerSecond, msecAudioData, 1000);

    // Largest possible size of the data chunk:
    DWORD cbMaxSize = MAXDWORD - cbHeader;

    // Maximum size altogether.
    cbAudioClipSize = min(cbAudioClipSize, cbMaxSize);

    // Round to the audio block size, so that we do not write a partial audio frame.
    cbAudioClipSize = (cbAudioClipSize / cbBlockSize) * cbBlockSize;

    return cbAudioClipSize;
}

HRESULT WriteWaveData(
    HANDLE hFile,               // Output file.
    IMFSourceReader* pReader,   // Source reader.
    IMFSourceReader* pReaderAnother,   // Another Source reader.
    DWORD cbMaxAudioData,       // Maximum amount of audio data (bytes).
    DWORD cbMaxAudioDataAnother,       // Maximum amount of audio data (bytes).
    DWORD* pcbDataWritten,       // Receives the amount of data written.
    DWORD* pcbDataWrittenAnother,       // Receives the amount of data written.
    int type                     // 数据处理的类型（声道替换 or 静音）
)
{
    HRESULT hr = S_OK;
    DWORD cbAudioData = 0;
    DWORD cbBuffer = 0;
    BYTE* pAudioData = NULL;

    //declar some other local variables for the second source
    HRESULT hr2 = S_OK;
    DWORD cbAudioDataAnother = 0;
    DWORD cbBufferAnother = 0;
    BYTE* pAudioDataAnother = NULL;

    IMFSample* pSample = NULL;
    IMFMediaBuffer* pBuffer = NULL;

    //declar some other local variables for the second source
    IMFSample* pSampleAnother = NULL;
    IMFMediaBuffer* pBufferAnother = NULL;

    // Get audio samples from the source reader.
    while (true)
    {
        DWORD dwFlags = 0;
        DWORD dwFlags2 = 0;

        // Read the next sample.
        hr = pReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0,
            NULL,
            &dwFlags,
            NULL,
            &pSample
        );
        hr2 = pReaderAnother->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0,
            NULL,
            &dwFlags2,
            NULL,
            &pSampleAnother
        );

        if (FAILED(hr)) { break; }
        if (FAILED(hr2)) { break; }

        if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
        {
            printf("Type change - not supported by WAVE file format.\n");
            break;
        }
        if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            //MessageBox(NULL, L"End of input file", L"message", NULL);
            break;
        }

        if (pSample == NULL)
        {
            printf("No sample\n");
            continue;
        }

        if (pSampleAnother == NULL)
        {
            printf("No sample\n");
            continue;
        }

        // Get a pointer to the audio data in the sample.
        hr = pSample->ConvertToContiguousBuffer(&pBuffer);
        if (FAILED(hr)) { break; }
        hr2 = pSampleAnother->ConvertToContiguousBuffer(&pBufferAnother);
        if (FAILED(hr2)) { break; }

        //锁定内存，得到缓冲地址指针
        hr = pBuffer->Lock(&pAudioData, NULL, &cbBuffer);
        if (FAILED(hr)) { break; }
        hr2 = pBufferAnother->Lock(&pAudioDataAnother, NULL, &cbBufferAnother);
        if (FAILED(hr2)) { break; }

        // Make sure not to exceed the specified maximum size.
        if (cbMaxAudioData - cbAudioData < cbBuffer)
        {
            cbBuffer = cbMaxAudioData - cbAudioData;
        }
        if (cbMaxAudioDataAnother - cbAudioDataAnother < cbBufferAnother)
        {
            cbBufferAnother = cbMaxAudioDataAnother - cbAudioDataAnother;
        }

        // TODO: 音频数据处理模块
        if (type == 1)
        {
            for (int i = 0; i < cbBuffer; i++)
            {
                // 声道替换
                if ((i - 0) % 4 == 0)     *(pAudioData + i) = (*(pAudioDataAnother + i)); //替换左声道的第一个字节
                if ((i - 1) % 4 == 0)     *(pAudioData + i) = (*(pAudioDataAnother + i)); //替换左声道的第二个字节
            }
        }
        else if (type == 2)
        {
            for (int i = 0; i < cbBuffer; i++)
            {
                // 静音
                *(pAudioData + i) = 100;
            }
        }


        // Write data1 and data2 to the output file1 and output file2.
        hr = WriteToFile(hFile, pAudioData, cbBuffer);
        if (FAILED(hr)) { break; }
        if (FAILED(hr2)) { break; }

        // Unlock the buffer.
        hr = pBuffer->Unlock();
        pAudioData = NULL;
        hr2 = pBufferAnother->Unlock();
        pAudioDataAnother = NULL;

        if (FAILED(hr)) { break; }
        if (FAILED(hr2)) { break; }

        // Update running total of audio data.
        cbAudioData += cbBuffer;
        cbAudioDataAnother += cbBufferAnother;

        if (cbAudioData >= cbMaxAudioData)
        {
            break;
        }
        if (cbAudioDataAnother >= cbMaxAudioDataAnother)
        {
            break;
        }

    }

    SafeRelease(&pSample);
    SafeRelease(&pBuffer);
    SafeRelease(&pSampleAnother);
    SafeRelease(&pBufferAnother);

    if (SUCCEEDED(hr))
    {
        //MessageBox(NULL, L"success to write file", L"message", NULL);
        *pcbDataWritten = cbAudioData;
    }

    if (pAudioData)
    {
        pBuffer->Unlock();
    }
    if (pAudioDataAnother)
    {
        pBufferAnother->Unlock();
    }
    SafeRelease(&pBuffer);
    SafeRelease(&pSample);
    SafeRelease(&pBufferAnother);
    SafeRelease(&pSampleAnother);
    return hr;
}

HRESULT FixUpChunkSizes(
    HANDLE hFile,           // Output file.
    DWORD cbHeader,         // Size of the 'fmt ' chuck.
    DWORD cbAudioData       // Size of the 'data' chunk.
)
{
    HRESULT hr = S_OK;

    LARGE_INTEGER ll;
    ll.QuadPart = cbHeader - sizeof(DWORD);

    if (0 == SetFilePointerEx(hFile, ll, NULL, FILE_BEGIN))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    // Write the data size.
    if (SUCCEEDED(hr))
    {
        hr = WriteToFile(hFile, &cbAudioData, sizeof(cbAudioData));
    }

    if (SUCCEEDED(hr))
    {
        // Write the file size.
        ll.QuadPart = sizeof(DWORD);

        if (0 == SetFilePointerEx(hFile, ll, NULL, FILE_BEGIN))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        DWORD cbRiffFileSize = cbHeader + cbAudioData - 8;

        // NOTE: The "size" field in the RIFF header does not include
        // the first 8 bytes of the file. i.e., it is the size of the
        // data that appears _after_ the size field.

        hr = WriteToFile(hFile, &cbRiffFileSize, sizeof(cbRiffFileSize));
    }

    return hr;
}