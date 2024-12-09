#include "pch.h"
#include <windows.h>
#include "ShowInformation.h"
#include <string>

std::wstring g_text;
std::wstring g_title;

// ���ڹ��̺���������  
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ShowInformation(const wchar_t* text, const wchar_t* title/* = L"QJS"*/, int width/* = 0*/, int height/* = 0*/)
{
    if (text)
        g_text = text;
    if (title)
        g_title = title;

    // ע�ᴰ����  
    const wchar_t CLASS_NAME[] = L"ShowInformationClass";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    // ��������  
    HWND hwnd = CreateWindowExW(
        0,                              // ��չ��ʽ  
        CLASS_NAME,                     // ��������  
        g_title.c_str(),                // ���ڱ���  
        WS_OVERLAPPEDWINDOW,            // ���ڷ��  
        CW_USEDEFAULT, CW_USEDEFAULT,   // λ��
		(width ? width : CW_USEDEFAULT), (height ? height : CW_USEDEFAULT),   // ��С  
        NULL,                           // �����ھ��  
        NULL,                           // �˵����  
        NULL,                           // ʵ�����  
        NULL                            // ���Ӳ���  
    );

    if (hwnd == NULL)
    {
        return;
    }

    ShowWindow(hwnd, SW_SHOWNORMAL);

    // ��Ϣѭ��  
    MSG msg;
    while (::GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

}

// ���ڹ��̺���  
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hwndEdit = NULL;

    switch (uMsg) {
    case WM_CREATE:
        // ����һ����������򣨱༭�ؼ���  
        hwndEdit = CreateWindowExW(
            0, L"EDIT", g_text.c_str(),  // �༭�ؼ�  
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL, // ��ʽ
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // λ�úʹ�С  
            hwnd,            // ������  
            NULL,           // �˵����  
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), // ʵ�����  
            NULL            // ���Ӳ���  
        );
        break;
    case WM_SIZE: {
        // �����ڴ�С�ı�  
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        // ���µ����༭��Ĵ�С  
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

