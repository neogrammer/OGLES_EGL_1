
#include <stdlib.h>
#include <string.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <glad/glad.h>
#include <App.h>
#include <iostream>
#include <Shader.h>


EGLSurface eglSurface;
EGLNativeWindowType eglNativeWindow;
EGLNativeDisplayType eglNativeDisplay;
EGLDisplay display;
EGLContext context;

int width = 1600;
int height = 900;

void mainLoop();

App app{};

LRESULT WINAPI windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT  lRet = 1;

    switch (uMsg)
    {
    case WM_CREATE:
        break;

    case WM_PAINT:
    {
        if (display)
        {
            if (app.isInitialized())
                app.render((void*)display);
            eglSwapBuffers(display, eglSurface);
        }
       
    }
    break;
    case WM_MOVING:
    {
        if (display && eglSurface)
            app.resize((void*)display, (void*)eglSurface, width, height);
    }
        break;

    case WM_SIZE:
    {
        UINT width_ = LOWORD(lParam);
        UINT height_ = HIWORD(lParam);
        if (app.isInitialized())
        {
            glViewport(0, 0, width, height);
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        width = (int)width_;
        height = (int)height_;
        if (display && eglSurface)
            app.resize((void*)display, (void*)eglSurface, width, height );
    }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_CHAR:
    {
        POINT      point;

        GetCursorPos(&point);

    }
    break;

    default:
        lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;
    }

    return lRet;
}


int main()
{
    WNDCLASSEX wndclass = { 0 };
    DWORD    wStyle = 0;
    HINSTANCE hInstance = GetModuleHandle(NULL);


    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hIcon = NULL;
    wndclass.hCursor = NULL;
    wndclass.lpszMenuName = NULL;
    wndclass.cbSize = sizeof(wndclass);
    wndclass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = (WNDPROC)windowProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = NULL;
    wndclass.lpszClassName = "opengles3.0";
    wndclass.hIconSm = NULL;
    if (!RegisterClassEx(&wndclass))
    {
        return FALSE;
    }

    wStyle = WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

 

    RECT rect = { 0, 0, width, height };
    AdjustWindowRectEx(&rect, wStyle, FALSE, 0);
    int winWidth = rect.right - rect.left;
    int winHeight = rect.bottom - rect.top;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int winX = (screenWidth - winWidth) / 2;
    int winY = (screenHeight - winHeight) / 2;


     eglNativeWindow = CreateWindowEx(
         0,
        "opengles3.0",
        "OGLESEGL",
        wStyle,
        winX,
        winY,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        hInstance,
        NULL);


    if (eglNativeWindow == NULL)
    {
        return FALSE;
    }


    eglNativeDisplay = GetDC((HWND)eglNativeWindow);

     

    display = eglGetDisplay(eglNativeDisplay);
    if (display == EGL_NO_DISPLAY) {
        MessageBox((HWND)eglNativeWindow, "EGL_NO_DISPLAY", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
        return FALSE;
    };
    
    EGLConfig config;
    EGLint minorVersion;
    EGLint majorVersion;
    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE, };

    if (!eglInitialize(display, &majorVersion, &minorVersion))
    {

        MessageBox((HWND)eglNativeWindow, "EGL unable to be initialized", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
        return FALSE;
    }
  
    // now to create opengl context
    {
        EGLint numConfigs = 0;
        EGLint attribList[] =
        {
           EGL_RED_SIZE,       8,
           EGL_GREEN_SIZE,     8,
           EGL_BLUE_SIZE,      8,
           EGL_ALPHA_SIZE,     8,
           EGL_DEPTH_SIZE,     24,
           EGL_STENCIL_SIZE,   8,
           EGL_SAMPLE_BUFFERS,  1,    // <- Enable multisampling
           EGL_SAMPLES,         4,    // <- 4x MSAA
           EGL_CONFORMANT, EGL_OPENGL_ES3_BIT,
           // if EGL_KHR_create_context extension is supported, then we will use
           // EGL_OPENGL_ES3_BIT_KHR instead of EGL_OPENGL_ES2_BIT in the attribute list
           EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
           EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
           EGL_NONE
        };

        if (!eglChooseConfig(display, attribList, &config, 1, &numConfigs))
        {
            MessageBox((HWND)eglNativeWindow, "EGL chooseConfig failed", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
            return FALSE;
        }

        if (numConfigs < 1)
        {
            MessageBox((HWND)eglNativeWindow, "EGL numCongfigs == 0", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
            return FALSE;
        }
    }

    eglSurface = eglCreateWindowSurface(display, config, eglNativeWindow, NULL);

    if (eglSurface == EGL_NO_SURFACE)
    {
        MessageBox((HWND)eglNativeWindow, "EGL_NO_SURFACE", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
        return FALSE;
    }

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);

    if (context == EGL_NO_CONTEXT)
    {
        MessageBox((HWND)eglNativeWindow, "EGL context not created", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
        return FALSE;
    }


    if (!eglMakeCurrent(display, eglSurface, eglSurface, context))
    {
        MessageBox((HWND)eglNativeWindow, "Unable to make context current", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL);
        return FALSE;
    }

    if (!gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress)) {
        std::cerr << "Failed to load GL ES symbols\n";
        return EXIT_FAILURE;
    }



    GLint major;
    GLint minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cout << "GL ES version " << major << "." << minor << '\n';

    
    ShowWindow((HWND)eglNativeWindow, TRUE);
    UpdateWindow((HWND)eglNativeWindow);
    
   
    app.resize((void*)display, (void*)eglSurface, 1600, 900);

    if (!app.init((void*)display))
    {
        std::cout << "App unable to be initialized" << std::endl;
        return -420;
    }

    GLfloat   lineWidthRange[2];
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);

    GLfloat    pointSizeRange[2];
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange);

    std::cout << "PointSize Range: " << pointSizeRange[0] << " - " << pointSizeRange[1] << std::endl;

    glClear(GL_COLOR_BUFFER_BIT);
    app.render((void*)display);
    eglSwapBuffers(display, eglSurface);

    // now load shaders



    mainLoop();

    app.shutdown();
	return 69;
}


void mainLoop()
{


    MSG msg = { 0 };
    int done = 0;
    ULONGLONG lastTime = GetTickCount64();

    while (!done)
    {
        int gotMsg = (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0);
        ULONGLONG curTime = GetTickCount64();
        float deltaTime = (float)(curTime - lastTime) / 1000.0f;
        lastTime = curTime;

        if (gotMsg)
        {
            if (msg.message == WM_QUIT)
            {
                done = 1;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if ((HWND)eglNativeWindow)
                SendMessage((HWND)eglNativeWindow, WM_PAINT, 0, 0);
        }

        app.update(deltaTime, width, height);

    }
}