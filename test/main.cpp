//
//  Nanocat engine.
//
//  Game initializer & window manager for Linux, Mac OS, Windows.
//
//  Created by Neko Vision on 22/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "core.h"
#include "system.h"
#include "systemshared.h"



// Window.
#ifdef _WIN32
#define EDIT_ID			100
#define INPUT_ID		101
#define EXTERNALCONSOLE_TEXTLEN 512
typedef struct {
    HWND		hWnd;
    HWND		hwndBuffer;
    
    HFONT		hfBufferFont;
    HWND		hwndInputLine;
    
    char		consoleText[EXTERNALCONSOLE_TEXTLEN];
    int			windowWidth, windowHeight;
    
    WNDPROC		SysInputLineWndProc;
    
} wincon_t;

// Windows window.
typedef struct _window {
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    HINSTANCE hInstance;
} window_t;

extern window_t   _win;
extern wincon_t   _con32;

#endif

void console_logo( void );

#ifdef _WIN32
#define USE_CONTROLCONSOLE

// Main window.
#define WINDOW_NAME     "Nanocat ( Windows, OpenGL 3.2 )"
#define WINDOW_CLASS    "Win32SkyCat"

// Console.
#define WIN32_CONSOLECLASSNAME "SkyCat32Console"
#define WIN32_CONSOLENAME "sky_cat"

wincon_t _con32;

static LONG WINAPI ConWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_CLOSE:
            system_quit("User quit.");
        return 0;
            
        case WM_ACTIVATE:
            if ( LOWORD( wParam ) != WA_INACTIVE ) {
                SetFocus( _con32.hwndInputLine );
            }
		break;

        case WM_CTLCOLORSTATIC:
            if ( (HWND)lParam == _con32.hwndBuffer ) {
                SetBkColor( ( HDC ) wParam, RGB( 0x00, 0x00, 0x00 ) );
                SetTextColor( ( HDC ) wParam, RGB( 0xff, 0xff, 0xff ) );
                return ( long )0;
            }
		break;
    }
    
    
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

LONG WINAPI InputLineWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char inputBuffer[1024];

	switch ( uMsg ) {
        case WM_KILLFOCUS:
            if ( (HWND) wParam == _con32.hWnd ) {
                SetFocus( hWnd );
                return 0;
            }
            break;

        case WM_CHAR:
            if ( wParam == 13 ) {
                GetWindowText( _con32.hwndInputLine, inputBuffer, sizeof( inputBuffer ) );
                
                strncat( _con32.consoleText, inputBuffer, sizeof( _con32.consoleText ) - strlen( _con32.consoleText ) - 5 );
                strcat( _con32.consoleText, "\n" );
                
                SetWindowText( _con32.hwndInputLine, "" );

                console_exec(inputBuffer);

                return 0;
            }
	}

	return CallWindowProc( _con32.SysInputLineWndProc, hWnd, uMsg, wParam, lParam );
}

void Win32_CreateConsole( HINSTANCE hInstance ) {
    
	HDC hDC;
	WNDCLASS wc;
	RECT rect;

	int nHeight;
	int swidth, sheight;
	int DEDSTYLE = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX;

	memset( &wc, 0, sizeof( wc ) );

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC) ConWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = (void *)COLOR_WINDOW;
	wc.lpszMenuName  = 0;
	wc.lpszClassName = WIN32_CONSOLECLASSNAME;

	if ( !RegisterClass (&wc) )
		return;

	rect.left = 0;
	rect.right = 540;
	rect.top = 0;
	rect.bottom = 370;
	AdjustWindowRect( &rect, DEDSTYLE, FALSE );

	hDC = GetDC( GetDesktopWindow() );
	swidth = GetDeviceCaps( hDC, HORZRES );
	sheight = GetDeviceCaps( hDC, VERTRES );
	ReleaseDC( GetDesktopWindow(), hDC );

	_con32.windowWidth = rect.right - rect.left + 1;
	_con32.windowHeight = rect.bottom - rect.top + 1;

	_con32.hWnd = CreateWindowEx( 0,
							   WIN32_CONSOLECLASSNAME,
							   WIN32_CONSOLENAME,
							   DEDSTYLE,
							   ( swidth - 600 ) / 2, ( sheight - 410 ) / 2 , rect.right - rect.left + 1, rect.bottom - rect.top + 1,
							   NULL,
							   NULL,
							   hInstance,
							   NULL );

	if ( _con32.hWnd == NULL ) {
		return;
	}

	hDC = GetDC( _con32.hWnd );
	nHeight = -MulDiv( 8, GetDeviceCaps( hDC, LOGPIXELSY), 12);

	_con32.hfBufferFont = CreateFont( 12,
									  8,
									  0,
									  0,
									  FW_LIGHT,
									  0,
									  0,
									  0,
									  DEFAULT_CHARSET,
									  OUT_DEFAULT_PRECIS,
									  CLIP_DEFAULT_PRECIS,
									  DEFAULT_QUALITY,
									  FF_MODERN | FIXED_PITCH,
									  "Terminal" );

	ReleaseDC( _con32.hWnd, hDC );

	_con32.hwndInputLine = CreateWindow( "edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
												ES_LEFT | ES_AUTOHSCROLL,
												6, 340, 528, 20,
												_con32.hWnd,
												( HMENU ) INPUT_ID,
												hInstance, NULL );

	_con32.hwndBuffer = CreateWindow( "edit", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
												ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
												6, 10, 526, 324,
												_con32.hWnd,
												( HMENU ) EDIT_ID,
												hInstance, NULL );


	SendMessage( _con32.hwndBuffer, WM_SETFONT, ( WPARAM ) _con32.hfBufferFont, 0 );

	_con32.SysInputLineWndProc = ( WNDPROC ) SetWindowLong( _con32.hwndInputLine, GWL_WNDPROC, ( long ) InputLineWndProc );
	SendMessage( _con32.hwndInputLine, WM_SETFONT, ( WPARAM ) _con32.hfBufferFont, 0 );

	ShowWindow( _con32.hWnd, SW_SHOWDEFAULT);
	UpdateWindow( _con32.hWnd );
	SetForegroundWindow( _con32.hWnd );
	SetFocus( _con32.hwndInputLine );
}

window_t    _win;

void win_destroy( void ) {
    // Remove OpenGL context and release window.
    wglMakeCurrent( NULL, NULL );
    wglDeleteContext( _win.hRC );
    ReleaseDC( _win.hwnd, _win.hDC );

    DestroyWindow( _win.hwnd );
}

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(	HINSTANCE	hInstance,				// Instance
					HINSTANCE	hPrevInstance,			// Previous
					LPSTR		lpCmdLine,				// Command line
					int			nCmdShow ) {
    DWORD _exstyle, _style;
    PIXELFORMATDESCRIPTOR pfd;

    int     _format;
    bool    _quit;
    char    _cwd[MAX_PATH];

    
    
#ifdef USE_CONTROLCONSOLE
    // Remove Windows console.
    FreeConsole();

    // Create new console.
    HINSTANCE hInstanceForConsole = (HINSTANCE)GetModuleHandle(NULL);
    Win32_CreateConsole( hInstanceForConsole );
#endif

    // Make console look nicer.
    system("COLOR 0F");
    SetConsoleTitle("Sky cat engine");
    console_logo();

    MSG msg;

    srand( time(0) );
    timeBeginPeriod( 1 );
    system_milliseconds();
    
    getcwd( _cwd, MAX_PATH );

    core_preload( _cwd );                    // We need to initialize some core stuff.
    render_loadvars();                           // Load renderer console variables.

    console_exec("readconfig config");            // Load some settings.

    _cmain.use_graphics = Server_DedicatedGetInteger() ? false : true;
    if( _cmain.use_graphics ) {
        // ----------------------------------------------------------------------------------------
        // Register the window class
        _win.hInstance = hInstance;
        _win.wcex.cbSize               = sizeof(WNDCLASSEX);
        _win.wcex.style                =  CS_OWNDC; // CS_HREDRAW | CS_VREDRAW |
        _win.wcex.lpfnWndProc          = WindowProc;
        _win.wcex.cbClsExtra         = 0;
        _win.wcex.cbWndExtra         = 0;
        _win.wcex.hInstance          = hInstance;
        _win.wcex.hIcon              = LoadIcon(NULL, IDI_APPLICATION);
        _win.wcex.hCursor            = LoadCursor(NULL, IDC_ARROW);
        _win.wcex.hbrBackground      = (HBRUSH)GetStockObject(BLACK_BRUSH);
        _win.wcex.lpszMenuName       = NULL;
        _win.wcex.lpszClassName      = WINDOW_CLASS;
        _win.wcex.hIconSm            = LoadIcon(NULL, IDI_APPLICATION);;

        if ( !RegisterClassEx(&_win.wcex) ) {
            _core.Error( ERC_FATAL, "failed to register window class\n" );
            return 0;
        }

        RECT wrect;

        wrect.left = 0;
        wrect.right = render_modeWidthGetInteger();
        wrect.top = 0;
        wrect.bottom = render_modeHeightGetInteger();

        // Window mode.
        if( render_fullscreenGetInteger() )
        {
            DEVMODE dmScreenSettings;

            dmScreenSettings.dmSize			= sizeof(DEVMODE);
            dmScreenSettings.dmPelsWidth	= wrect.right;
            dmScreenSettings.dmPelsHeight	= wrect.bottom;
            dmScreenSettings.dmBitsPerPel	= 32;
            dmScreenSettings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

            if( ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL )
            {
                _core.Print( LOG_WARN, "Couldn't set fullscreen mode ( probably it's not supported by your video card \n" );
                _core.Print( LOG_WARN, "Or wrong resolution set. )\n" );
                _core.Print( LOG_INFO, "Setting windowed mode..\n" );

                consolevar_set( "render_fullscreen", "0", true );              // Lock further changes.
                consolevar_lock( "render_fullscreen" );
            }

        } else                                                      // Windowed.
            _core.Print( LOG_INFO, "Setting windowed mode..\n" );


        // 'render_fullscreen' is still enabled, so we are going to create the full screen window
        if( render_fullscreenGetInteger() )
        {
            _core.Print( LOG_INFO, "Setting fullscreen mode..\n" );
            _exstyle = WS_EX_APPWINDOW;
            _style = WS_POPUP;
        } else {    // Extended style.
            _exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
            _style = WS_OVERLAPPEDWINDOW;
        }

        AdjustWindowRectEx( &wrect, _style, FALSE, _exstyle);

        // Create window now.
        if ( !(_win.hwnd = CreateWindowEx( _exstyle,
                                    WINDOW_CLASS,
                                    WINDOW_NAME,
                                    WS_CLIPSIBLINGS |
                                    WS_CLIPCHILDREN |
                                    _style,
                                    0, 0,
                                    wrect.right,
                                    wrect.bottom,
                                    NULL,
                                    NULL,
                                    hInstance,
                                    NULL )) ) {

                _core.Error( ERC_FATAL, "Couldn't create window." );
                return -1;
        }

        // Get device context.
        _win.hDC = GetDC(_win.hwnd);

        // Pixel format setup.
        ZeroMemory(&pfd, sizeof(pfd));

        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                      PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 8;
        pfd.cDepthBits = 8;
        pfd.iLayerType = PFD_MAIN_PLANE;

        _format = ChoosePixelFormat(_win.hDC, &pfd);

        if ( !SetPixelFormat(_win.hDC, _format, &pfd) ) {
            _core.Error( ERC_FATAL, "Failed to setup window pixel format.\n" );
            return -1;
        }

        _core.Print( LOG_INFO, "Creating window..\n" );
        _core.Print( LOG_INFO, "Successfully created window.\n" );
        _core.Print( LOG_INFO, "Resolution: %ix%i\n", render_modeWidthGetInteger(), render_modeHeightGetInteger() );

        // opengl context from a windows
        if ( !(_win.hRC = wglCreateContext(_win.hDC) ) ) {
            _core.Error( ERC_GL, "Could not create OpenGL context.\n" );
            return -1;
        }

        if ( wglMakeCurrent(_win.hDC, _win.hRC) )
            _core.Print( LOG_DEVELOPER, "Created OpenGL context.\n" );


        // After OpenGL context created, use GLEW library.
        // NOTE: It's only one 3rd party lib even used in this
        // engine for OpenGL 3.X, 4.X versions.
        glewInit();
    }

    // --------------------------------------------------------------------------------

    // System init.
    core_init();

    // OpenGL preferences ( calls renderer load ).
    gl_init();

    // Engine systems were loaded, allow to show the window now.
    _cmain.gl_init = true;

    // Resize window manually.
    _opengl.OnResize( render_modeWidthGetInteger(), render_modeHeightGetInteger() );

    // Show window.
    ShowWindow( _win.hwnd, nCmdShow );
    UpdateWindow( _win.hwnd );
	SetFocus( _win.hwnd );

    _quit = false;

    // Everything got loaded.
    core_loaded();

    while (!_quit)
    {
        // Windows messages.
        if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) // Nao, console goes here too, && !Server_DedicatedGetInteger() )
        {
            // handle and dispatch messages
            if (msg.message == WM_QUIT) {
                system_quit("user quit");
                _quit = TRUE;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else {
            core_frame();
            Sleep(1);
        }
    }

    // Remove window and context after all.
    win_destroy();

	return msg.wParam;								// Exit The Program
}

#endif

/*
        Resize window.
*/
void win_resize( int w, int h ) {
    if( ( w <= 0 || h <= 0 ) ) {
        _core.Print( LOG_WARN, "Given sizes for window resize are too small ( width: %i height: %i )\n" );
        return;
    }

#ifdef _WIN32
    SetWindowPos(_win.hwnd, 0, 0, 0, w, h, SWP_NOMOVE);
#endif // _WIN32
}

#ifdef _WIN32
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int _width, _height;
    int _mx, _my;

    switch (uMsg)
    {
        case WM_MOUSEMOVE:
            _mx = GET_X_LPARAM(lParam);
            _my = GET_Y_LPARAM(lParam);

            input_mousemove( _mx, _my, 0, 0 );
        break;

        // mouse down
        case WM_LBUTTONDOWN:
            _mouse.holding = true;
            input_mousedown( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
        break;

        // mouse up
        case WM_LBUTTONUP:
            _mouse.holding = false;
            input_mouseup( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
        break;

        case WM_SIZE:
            _width = LOWORD(lParam);
            _height = HIWORD(lParam);

            _opengl.OnResize( _width, _height );
        break;

        case WM_CLOSE:
            system_quit("user quit");
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYUP:
            input_keyup( (int)wParam );
            break;

        case WM_KEYDOWN: {
            input_keypress((int)wParam);

            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}
#endif


/*
    Print logo.
    Art.
*/
void console_logo( void ) {
#ifdef _WIN32
    HANDLE hOut;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY );
#endif
    const char *meow[10]={

    "                                                           \n",
    "  .___  ___.  _______   ______   ____    __    ____       \n",
    "  |   \\/   | |   ____| /  __  \\  \\   \\  /  \\  /   /   \n",
    "  |  \\  /  | |  |__   |  |  |  |  \\   \\/    \\/   /   \n",
    "  |  |\\/|  | |   __|  |  |  |  |   \\            /      \n",
    "  |  |  |  | |  |____ |  `--'  |    \\    /\\    /    \n",
    "  |__|  |__| |_______| \\______/      \\__/  \\__/   \n",
    "                         \n",
    "                v0.7 night, by nekocode\n",
    "                   Nanocat engine\n"
};

    int i;
    for( i = 0; i < 10; i++) {
        _core.Print( LOG_NONE, meow[i] );
    }

    _core.Print( LOG_NONE, "\n" );

    printf("--------------------------------------------------------------------------------\n" );
#ifdef _WIN32
    SetConsoleTextAttribute(hOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY );
#endif
    
    _core.Print( LOG_NONE,"Loading...\n");
    _core.Print( LOG_NONE,"Welcome, %s!\n", _system.GetCurrentUsername());
    _core.Print( LOG_NONE,"\n");
}



