#include "pch.h"
#include <windows.h>
#include "ShowInformation.h"
#include <string>

std::wstring g_text;
std::wstring g_title;

// 窗口过程函数的声明  
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ShowInformation(const wchar_t* text, const wchar_t* title/* = L"QJS"*/, int width/* = 0*/, int height/* = 0*/)
{
    if (text)
        g_text = text;
    if (title)
        g_title = title;

    // 注册窗口类  
    const wchar_t CLASS_NAME[] = L"ShowInformationClass";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    // 创建窗口  
    HWND hwnd = CreateWindowExW(
        0,                              // 扩展样式  
        CLASS_NAME,                     // 窗口类名  
        g_title.c_str(),                // 窗口标题  
        WS_OVERLAPPEDWINDOW,            // 窗口风格  
        CW_USEDEFAULT, CW_USEDEFAULT,   // 位置
		(width ? width : CW_USEDEFAULT), (height ? height : CW_USEDEFAULT),   // 大小  
        NULL,                           // 父窗口句柄  
        NULL,                           // 菜单句柄  
        NULL,                           // 实例句柄  
        NULL                            // 附加参数  
    );

    if (hwnd == NULL)
    {
        return;
    }

    ShowWindow(hwnd, SW_SHOWNORMAL);

    // 消息循环  
    MSG msg;
    while (::GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

}

// 窗口过程函数  
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hwndEdit = NULL;

    switch (uMsg) {
    case WM_CREATE:
        // 创建一个多行输入框（编辑控件）  
        hwndEdit = CreateWindowExW(
            0, L"EDIT", g_text.c_str(),  // 编辑控件  
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL, // 样式
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // 位置和大小  
            hwnd,            // 父窗口  
            NULL,           // 菜单句柄  
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), // 实例句柄  
            NULL            // 附加参数  
        );
        break;
    case WM_SIZE: {
        // 处理窗口大小改变  
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        // 重新调整编辑框的大小  
        MoveWindow(hwndEdit, 0, 0, width, height, TRUE);
    }
                break;
    case WM_DESTROY:
        hwndEdit = NULL;
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

