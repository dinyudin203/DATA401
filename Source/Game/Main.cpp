/*!
 * Copyright (c) IIIXR LAB. All rights reserved.
 */

#include <Common.h>
#include <Game.h>

INT main() {
	wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWDEFAULT);
	return 0;
}

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
        return 0;

    if (FAILED(InitDevice()))
    {
        CleanupDevice();
        return 0;
    }

    /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
    TODO: Declare elapse time variable
    -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    LARGE_INTEGER frequency;
    LARGE_INTEGER elapsedMsc;

    QueryPerformanceCounter(&startTime);
    QueryPerformanceFrequency(&frequency);

    // Main message loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            /*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
            TODO: Calculate elapse time at each frame
            -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

            QueryPerformanceCounter(&endTime);
            elapsedMsc.QuadPart = endTime.QuadPart - startTime.QuadPart;
            elapsedMsc.QuadPart *= 1000000;

            elapsedMsc.QuadPart /= frequency.QuadPart;
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&startTime);

            FLOAT deltaTime = static_cast<FLOAT>(elapsedMsc.QuadPart) / 1000000.0f;
            HandleInput(deltaTime);
            Update(deltaTime);

            Render();
        }
    }

    CleanupDevice();

    return (int)msg.wParam;
}