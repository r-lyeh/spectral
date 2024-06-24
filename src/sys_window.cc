#include <windows.h>
#pragma comment(lib, "gdi32")
#pragma comment(lib, "user32")

extern "C" int   tinyfd_assumeGraphicDisplay;
extern "C" char* tinyfd_inputBox(const char *, const char *, const char *);
extern "C" char* osdialog_prompt(int, const char*, const char*);

extern "C"
char* prompt( const char *title, const char *caption, const char *defaults ) {

    void *hwndParentWindow = (void*)GetActiveWindow(); // = (void*)GetForegroundWindow();

#if 1

    class InputBox
    {
        public:

        HWND                hwndParent,
                            hwndInputBox,
                            hwndQuery,
                            hwndEditBox;
        LPSTR               szInputText;
        WORD                wInputMaxLength, wInputLength;
        bool                bRegistered,
                            bResult;

        HINSTANCE           hThisInstance;

        enum
        {
            CIB_SPAN = 10,
            CIB_LEFT_OFFSET = 6,
            CIB_TOP_OFFSET = 4,
            CIB_WIDTH = 300,
            CIB_HEIGHT = 130,
            CIB_BTN_WIDTH = 60,
            CIB_BTN_HEIGHT = 20
        };

        public:

#       define CIB_CLASS_NAME   "CInputBoxA"

        static LRESULT CALLBACK CIB_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            InputBox *self;
            self = (InputBox *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

            switch (msg)
            {
                case WM_CREATE:
                    self = (InputBox *) ((CREATESTRUCT *)lParam)->lpCreateParams;
                    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)self);
                    self->create(hWnd);
                break;
                case WM_COMMAND:
                    switch(LOWORD(wParam)) {
                        case IDOK:
                            self->submit();
                        case IDCANCEL:
                            self->close();
                        break;
                    }
                    break;
                case WM_CLOSE:
                    self->close();
                    return 0;
                case WM_DESTROY:
                    self->destroy();
                    break;
            }
            return(DefWindowProc (hWnd, msg, wParam, lParam));
        }

        InputBox( HINSTANCE hInst, HWND hWndParent_ ) :
            hwndParent(hWndParent_),
            hwndInputBox(0),
            hwndQuery(0),
            hwndEditBox(0),
            szInputText(0),
            wInputMaxLength(0), wInputLength(0),
            bRegistered(false),
            bResult(false),
            hThisInstance(hInst)
        {
            WNDCLASSEXA wndInputBox;
            RECT rect;

            memset(&wndInputBox, 0, sizeof(WNDCLASSEXA));

            hThisInstance = hInst;

            wndInputBox.cbSize                  = sizeof(wndInputBox);
            wndInputBox.lpszClassName           = CIB_CLASS_NAME;
            wndInputBox.style                   = CS_HREDRAW | CS_VREDRAW;
            wndInputBox.lpfnWndProc             = CIB_WndProc;
            wndInputBox.lpszMenuName            = NULL;
            wndInputBox.hIconSm                 = NULL;
            wndInputBox.cbClsExtra              = 0;
            wndInputBox.cbWndExtra              = 0;
            wndInputBox.hInstance               = hInst;
            wndInputBox.hIcon                   = LoadIcon(NULL, IDI_WINLOGO);
            wndInputBox.hCursor                 = LoadCursor(NULL, IDC_ARROW);
            wndInputBox.hbrBackground           = (HBRUSH)(COLOR_WINDOW);

            RegisterClassExA(&wndInputBox);

            if (hwndParent)
                GetWindowRect(hwndParent, &rect); //always false?
            else
                GetWindowRect(GetDesktopWindow(), &rect);

            hwndInputBox = CreateWindowExA( WS_EX_NOPARENTNOTIFY,
                            CIB_CLASS_NAME, "",
 WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_CENTER | DS_SHELLFONT,
//                            (WS_BORDER | WS_CAPTION),
                            rect.left+(rect.right-rect.left-CIB_WIDTH)/2,
                            rect.top+(rect.bottom-rect.top-CIB_HEIGHT)/2,
                            CIB_WIDTH*1.05, CIB_HEIGHT, hwndParent, NULL,
                            hThisInstance, this);
        }

        void destroy()
        {
            EnableWindow(hwndParent, true);
            SendMessage(hwndInputBox, WM_CLOSE/*WM_DESTROY*/, 0, 0);
        }

        ~InputBox()
        {
            UnregisterClassA(CIB_CLASS_NAME, hThisInstance);
        }

        void submit()
        {
            wInputLength = (int)SendMessage(hwndEditBox, EM_LINELENGTH, 0, 0);
            if (wInputLength) {
                *((LPWORD)szInputText) = wInputMaxLength;
                wInputLength = (WORD)SendMessage(hwndEditBox, EM_GETLINE, 0, (LPARAM)szInputText);
            }
            szInputText[wInputLength] = '\0';
            bResult = true;
        }

        void create(HWND hwndNew)
        {
            static HFONT myFont = NULL;

            if( myFont != NULL )
            {
                DeleteObject( myFont );
                myFont = NULL;
            }

            hwndInputBox = hwndNew;

            NONCLIENTMETRICS ncm;
            ncm.cbSize = sizeof(NONCLIENTMETRICS);

            if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
            {
#if 0
                LOGFONT lf;
                memset(&lf,0,sizeof(LOGFONT));

                lf.lfWeight= FW_NORMAL;
                lf.lfCharSet= ANSI_CHARSET;
                //lf.lfPitchAndFamily = 35;
                lf.lfHeight= 10;
                strcpy(lf.lfFaceName, "Tahoma");
                myFont=CreateFontIndirect(&lf);
#else
                myFont = CreateFontIndirect(&ncm.lfMessageFont);
#endif
            }
            else
            {
                myFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            }

SetWindowPos(hwndInputBox, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

            hwndQuery = CreateWindowExA(WS_EX_NOPARENTNOTIFY, "Static", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                    CIB_LEFT_OFFSET, CIB_TOP_OFFSET,
                                    CIB_WIDTH-CIB_LEFT_OFFSET*2, CIB_BTN_HEIGHT*2,
                                    hwndInputBox, NULL,
                                    hThisInstance, NULL);
            hwndEditBox = CreateWindowExA(WS_EX_NOPARENTNOTIFY, "Edit", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL | ES_LEFT, CIB_LEFT_OFFSET,
                                    CIB_TOP_OFFSET + CIB_BTN_HEIGHT*2, CIB_WIDTH-CIB_LEFT_OFFSET*3, CIB_BTN_HEIGHT,
                                    hwndInputBox,   NULL,
                                    hThisInstance, NULL);

        //  SendMessage(hwndInputBox,WM_SETFONT,(WPARAM)myFont,FALSE);
            SendMessage(hwndQuery,WM_SETFONT,(WPARAM)myFont,FALSE);
            SendMessage(hwndEditBox,WM_SETFONT,(WPARAM)myFont,FALSE);

            SendMessage(hwndInputBox, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)(HICON)(UINT_PTR)GetClassLong(hwndParent, -14/*GCL_HICON*/)); // setting the icon
            SendMessage(hwndQuery, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)(HICON)(UINT_PTR)GetClassLong(hwndParent, -14/*GCL_HICON*/)); // setting the icon
            SendMessage(hwndEditBox, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)(HICON)(UINT_PTR)GetClassLong(hwndParent, -14/*GCL_HICON*/)); // setting the icon
        }

        void close()
        {
            PostMessage(hwndInputBox, WM_CLOSE, 0, 0);
        }

        void hide()
        {
            ShowWindow(hwndInputBox, SW_HIDE);
        }

        void show(LPCSTR lpszTitle, LPCSTR  lpszQuery)
        {
            SetWindowTextA(hwndInputBox, lpszTitle);
            SetWindowTextA(hwndEditBox, szInputText);
            SetWindowTextA(hwndQuery, lpszQuery);
            SendMessage(hwndEditBox, EM_LIMITTEXT, wInputMaxLength, 0);
            SendMessage(hwndEditBox, EM_SETSEL, 0, -1);
            SetFocus(hwndEditBox);
            ShowWindow(hwndInputBox, SW_NORMAL);
        }

        int pump() {
            MSG msg;
            BOOL    bRet;

            if( (bRet = GetMessageA( &msg, NULL, 0, 0 )) != 0 )
            {
                if (bRet == -1)
                {
                    // handle the error and possibly exit
                    return 0;
                }
                else
                {

                    if (msg.message==WM_KEYDOWN) {
                        switch (msg.wParam) {
                        case VK_RETURN:
                            submit();
                        case VK_ESCAPE:
                            close();
                            break;
                        default:
                            TranslateMessage(&msg);
                            break;
                        }
                    } else
                    //if (!IsDialogMessage(hwndInputBox, &msg)) {
                        TranslateMessage(&msg);
                    //}

                    DispatchMessage(&msg); 
                }

                if (msg.message == WM_CLOSE)
                    return 0;

                return 1;
            }

            return 0;
        }

        void show(LPCSTR lpszTitle, LPCSTR lpszQuery, LPSTR szResult, WORD wMax)
        {

        EnableWindow(hwndParent, FALSE);

            szInputText = szResult;
            wInputMaxLength = wMax;
            show(lpszTitle, lpszQuery);

            while( pump() );

        // EnableWindow(hwndParent, TRUE);

        }

    #   undef CIB_CLASS_NAME
    };

    static char result[2048+1];
    strncpy( result, defaults, 2048 );

    {
        InputBox myinp( GetModuleHandleA(NULL), (HWND)hwndParentWindow );
        myinp.show(title, caption ? caption : "", result, 2048);
        myinp.destroy();

        DestroyWindow(myinp.hwndInputBox);
    }

    return result;

#else

    HWND hwndParent = (HWND)hwndParentWindow;
    EnableWindow(hwndParent, false);
#ifdef TFD_IMPLEMENTATION
    char *ret = (tinyfd_assumeGraphicDisplay = 1, tinyfd_inputBox(title, caption, defaults));
#else
    char *ret = osdialog_prompt(0/*OSDIALOG_INFO*/, caption, defaults);
#endif
    EnableWindow(hwndParent, true);
    return ret;

#endif

}

#if 0
#include <stdio.h>
int main() {
    puts(prompt("abc","def","ghi"));
}
#endif
