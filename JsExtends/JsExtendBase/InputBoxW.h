/**************************************/
/*       InputBox.h                   */
/**************************************/

#ifndef WKE_INPUT_BOX
#define WKE_INPUT_BOX

#include <windows.h>
#include <string>

#define INPUT_BOX_WIDTH 365
#define INPUT_BOX_HEIGHT 130

BOOL _InitInputBoxW(HWND hParent);      // ��ѡ�������ڵ���InputBox����֮ǰ��õ���һ�¸ú������ú����ǳ�ʼ��InputBox,��Ҫ���Ǵ���һ�������ھ����
void _SetNumberW(BOOL fIsNumber);       // ��ѡ���ú���������Editֻ����������
void _SetLowerCaseW(BOOL fIsLowerCase); // ��ѡ���ú���������Editֻ������Сд��ĸ
void _SetUpperCaseW(BOOL fIsUpperCase); // ��ѡ���ú���������Editֻ�������д��ĸ

/*********************************************************/
/*                InputBox����                           */
/*  lpWndMsg     ; ��ѡ������������ʾ�û�                */
/*  lpWndTitle   ; ��ѡ����, ���öԻ���Ĭ�ϱ���          */
/*  lpDefValue   ; ��ѡ����������EditĬ��ֵ              */
/*  xPos         ; ��ѡ���������Ϊ������д���          */
/*  yPos         ; ��ѡ���������Ϊ������д���          */
/*  lpHelpFile   ; ��ѡ��������*.hlp�����ļ�           */
/*  nHelpIndex   ; ��ѡ�����������������lpHelpFile������*/
/*               ; �����������Help�ļ�������            */
/*  ��������ɹ�������һ��char *ָ�룬�����û���������� */
/*********************************************************/
const std::wstring& _InputBoxW(const wchar_t *lpWndMsg,
                const wchar_t *lpWndTitle = L"����!",
                const wchar_t *lpDefValue = L"",
                int xPos = 0,
                int yPos = 0,
                const wchar_t *lpHelpFile = L"",
                int nHelpIndex = 0);

#endif