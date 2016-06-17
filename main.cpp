/*

Noise agent for We Are Data #wearedata http://wearedata.nl.
Plays fill sounds when instructed so from main application.
Also shuts up when asked to.
Hens Zimmerman, henszimmerman@gmail.com
May 11, 2016.
Code::Blocks 16.01
TDM-GCC64 gcc compiler.

*/


#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

/*

C# calling:

// Imports:

[DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
public static extern uint RegisterWindowMessage(string lpString);

[DllImport("user32.dll", SetLastError = true)]
public static extern IntPtr SendMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

// Registration (hoeft maar een keer bij program start)
// Maar uint variabelen moeten beschikbaar zijn in scope van code
// die de films start en stopt.

uint noiseStart = RegisterWindowMessage("WAD start message");
uint noiseStop = RegisterWindowMessage("WAD stop message");

// Geluid aan:

SendMessage(HWND_BROADCAST, noiseStart, (IntPtr)0, (IntPtr)0);

// Geluid uit:

SendMessage(HWND_BROADCAST, noiseStop, (IntPtr)0, (IntPtr)0);

// Mogelijk krijg je een compiler error over HWND_BROADCAST.
// In dat geval deze regel toevoegen:

IntPtr HWND_BROADCAST = (IntPtr) 0xffff;

*/

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("WAD_NOISE");

// Two UINT variables to store custom Windows messages.

UINT WADStartMessage = 0;
UINT WADStopMessage = 0;


// Main window function.

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    // Run single instance only.

    HANDLE mutexHandle = CreateMutex
    (
        0,
        true,
        "{32D55740-F018-11D3-8E0F-ECA10903AD33}"
    );

    if(GetLastError() == ERROR_ALREADY_EXISTS)
    {
        if(mutexHandle)
        {
            CloseHandle(mutexHandle);
        }

        return 0;
    }

    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* Register our custom send/receive messages. */

    WADStartMessage = RegisterWindowMessage("WAD start message");
    WADStopMessage = RegisterWindowMessage("WAD stop message");


    /* Initialize random seed. */

    srand (time(NULL));


    /* The Window structure */

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */

    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    wincl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

    /* Register the window class, and if it fails quit the program */

    if (!RegisterClassEx (&wincl))
    {
        ReleaseMutex(mutexHandle);
        CloseHandle(mutexHandle);

        return 0;
    }

    /* The class is registered, let's create the program. */

    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("WAD - init"),     /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           160,                 /* The programs width */
           80,                  /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Minimize the window. */

    ShowWindow (hwnd, SW_MINIMIZE /*nCmdShow*/);


    /* Run the message loop. It will run until GetMessage() returns 0 */

    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */

        TranslateMessage(&messages);

        /* Send message to WindowProcedure */

        DispatchMessage(&messages);
    }

    // Give mutex back to OS so we can run again if so desired.

    ReleaseMutex(mutexHandle);
    CloseHandle(mutexHandle);

    /* The program return-value is 0 - The value that PostQuitMessage() gave */

    return messages.wParam;
}


/*

This function is called by the Windows function DispatchMessage()
Adjusted from switch to if/else on purpose because we react to
custom messages stored in non consts. HZ, 2016-05-07.

*/

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    if(message == WADStartMessage)
    {
        /* WAD request to play filler noise. */
        /* We'll play a random sound out of 4 from the program resource data (see .rc). */

        // char soundName[8];

        // sprintf(soundName, "NOISE%d", rand() % 4);

        PlaySound
        (
            "NOISE0",
            GetModuleHandle(0),
            SND_RESOURCE | SND_ASYNC | SND_LOOP
        );

        SetWindowText(hwnd, "WAD - playing");
    }
    else if(message == WADStopMessage)
    {
        /* WAD request to stop filler noise. */

        PlaySound(0, 0, 0);
        SetWindowText(hwnd, "WAD - stopped");
    }
    else if (message == WM_DESTROY)
    {
        /* send a WM_QUIT to the message queue */

        PostQuitMessage (0);
    }
    else if (message == WM_PAINT)
    {
        /* Display some info on who made this. */

        PAINTSTRUCT ps;
        RECT rect;

        HDC hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rect);

        DrawText
        (
            hdc,
            "WAD noise agent\nhenszimmerman@gmail.com",
            -1,
            &rect,
            DT_LEFT | DT_VCENTER
        );

        EndPaint(hwnd, &ps);
    }
    else
    {
        /* for messages that we don't deal with */

        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

