#define window Tigr
#define window_open(w,h,title) tigrWindow(w, h, title, TIGR_2X)
#define window_bitmap(w,h) tigrBitmap(w,h)
#define window_update(win) (tigrUpdate(win))
#define window_alive(win) (!tigrClosed(win))
#define window_close(win) (win = (win ? tigrFree(win), NULL : NULL))
#define window_print(win, text) window_printxy(win, text, 1, 1)
#define window_printxy(win, text, x,y) window_printxycol(win, text, x,y, tigrRGB(0xff, 0xff, 0xff))
#define window_printxycol(win, text, x,y, col) tigrPrint(win, tfont, (x)*11,(y)*11, col, "%s", text)
#define window_pressed(win, keycode) (!!(tigrKeyDown(win, keycode) || tigrKeyHeld(win, keycode)))
#define window_trigger(win, keycode) (!!tigrKeyDown(win, keycode))
void    window_override_icons();
char*   window_title(window *win, const char *title);


int window_keyrepeat(window *app, unsigned char vk) {
    static int table[256] = {0}; // @fixme: table[num_windows][256];
    table[vk] *= !!window_pressed(app, vk);
    table[vk] += !!window_pressed(app, vk);
    return table[vk] == 1 || table[vk] > 32;
}

char*   prompt(const char *title, const char *caption, const char *defaults );
#define alert(body) alert("Warning", body)
void    die(const char *msg);



#ifdef __APPLE__

char* prompt(const char *title, const char *body, const char *defaults ) {
    static char buffer[256]; buffer[0] = '\0';
    char *cmd = va("osascript -e 'text returned of (display dialog \"%s - %s\" default answer \"%s\")'", title, body, defaults);
    for( FILE *fp = popen(cmd, "r"); fp; pclose(fp), fp = 0 ) {
        fgets(buffer, 256, fp);
    }
    puts(buffer);
    return buffer;
}
void window_override_icons() {
    
}

#elif defined _WIN32

// Set the window icon for every window in your app (including MessageBox() calls and assertion failures) instead of just your primary window.
static HICON appIcon; // = (HICON)GetClassLong(hWnd, GCL_HICON);
static LRESULT window_create_callback(int type, WPARAM wparam, LPARAM lparam) {
    if (type == HCBT_CREATEWND) {
        SendMessage((HWND)wparam, WM_SETICON, ICON_SMALL, (LPARAM)appIcon);
        SendMessage((HWND)wparam, WM_SETICON, ICON_BIG, (LPARAM)appIcon);
    }
    return CallNextHookEx(NULL, type, wparam, lparam);
}
void window_override_icons() {
    do_once {
        HANDLE hInstance = GetModuleHandleA(NULL);
        appIcon = ExtractIconA(hInstance, __argv[0], 0 );
        if(!appIcon) appIcon = ExtractIconA(hInstance, va("%s.exe", __argv[0]), 0 );
    }
    SetWindowsHookEx(WH_CBT, window_create_callback, NULL, GetCurrentThreadId());
}

#else

char* prompt(const char *title, const char *body, const char *defaults ) {
    // order should be: kdialog, then zenity, then Xdialog
    // kdialog --title "title" --inputbox "" "body"
    // zenity --title "title" --entry --text "body"
    // Xdialog
    static char buffer[256]; buffer[0] = '\0';
    char *cmdk = va("kdialog --title \"%s\" --inputbox \"%s\" \"%s\"", title, body, defaults);
    char *cmdz = va("zenity --title \"%s\" --entry --text \"%s\" --entry-text \"%s\"", title, body, defaults);
    for( FILE *fp = popen(va("%s || %s", cmdk, cmdz), "r"); fp; pclose(fp), fp = 0 ) {
        fgets(buffer, 256, fp);
    }
    puts(buffer);
    return buffer;
}

#define window_override_icons()

#endif


char* window_title(window *win, const char *title) {
    static char copy[128] = {0};
    if( title ) {
#ifdef __APPLE__

#elif defined _WIN32
        SetWindowTextA((HWND)(((Tigr*)win)->handle), title);
#else
        XTextProperty prop;
        int result = Xutf8TextListToTextProperty(dpy, (char**)&title, 1, XUTF8StringStyle, &prop);
        if (result == Success) {
            Atom wmName = XInternAtom(dpy, "_NET_WM_NAME", 0);
            XSetTextProperty(tigrInternal(win)->dpy, tigrInternal(win)->win, &prop, wmName);
            XFree(prop.value);
        }
#endif
        snprintf(copy, 128, "%s", title);
    }
    return copy;
}

int (alert)(const char *title, const char *body) {
#ifdef _WIN32
    MessageBoxA(0, body, title, MB_OK);
//#elif is(ems)
//    emscripten_run_script(va("alert('%s')", body));
#elif defined __linux__ // is(linux)
    for(FILE *fp = fopen("/tmp/spectral.warning","wb");fp;fp=0)
    fputs(body,fp), fclose(fp), system("xmessage -center -file /tmp/spectral.warning");
#else // if is(osx)
    system(va("osascript -e 'display alert \"%s\" message \"%s\"'", title, body));
#endif
    return 1;
}

void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    alert(msg);
#ifdef _WIN32
    if(IsDebuggerPresent()) DebugBreak();
#endif
    exit(-1);
}

