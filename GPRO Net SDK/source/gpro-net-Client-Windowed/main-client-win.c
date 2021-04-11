/*
   Copyright 2021 Daniel S. Buckstein

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	GPRO Net SDK: Networking framework.
	By Daniel S. Buckstein

	main-client-win.c/.cpp
	Main source for client windowed.
*/

#if (defined _WINDOWS || defined _WIN32)


#include <Windows.h>


#include "a3_app_utils/Win32/a3_app_application.h"
#include "a3_app_utils/Win32/a3_app_window.h"


#include "gpro-net/gpro-net/gpro-net-util/gpro-net-console.h"


//-----------------------------------------------------------------------------
// windowed entry point

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	// window data
	a3_WindowInterface wnd = { 0 };
	a3_WindowClass wndClass = { 0 };
	a3_PlatformEnvironment env = { 0 };
	a3_RenderingContext renderContext = 0;
	const a3byte* wndClassName = "A3_DEMO_PLAYER_CLASS";
	const a3byte* wndName = "animal3D Demo Player";

	// some widescreen resolutions
//	const a3ui32 winWidth = 1280, winHeight = 720;
//	const a3ui32 winWidth = 1024, winHeight = 576;
	const a3ui32 winWidth = 960, winHeight = 540;

	// standard resolution
//	const a3ui32 winWidth =  480, winHeight = 360;

	// result of startup tasks
	a3i32 status = 0;

	// initialize console
	gpro_console console = { 0 };
	status = gpro_consoleCreateMain(&console);
//	status = gpro_consoleDrawTestPatch();

	// initialize app
	//status = a3appStartSingleInstanceSwitchExisting(wndClassName, wndName);

	// register window class
	status = a3windowCreateDefaultRenderingClass(&wndClass, hInstance, wndClassName, sizeof(void*), 0, 0);
	if (status > 0)
	{
		// create rendering context
		status = a3rendererCreateDefaultContext(&renderContext, &wndClass);
		if (status > 0)
		{
			// init platform environment for debugging
			status = a3windowInitPlatformEnvironment(&env, 0, 0, 0, 0);

			// create window
			status = a3windowCreate(&wnd, &wndClass, &env, &renderContext, wndName, winWidth, winHeight, 1, 1);
			if (status > 0)
			{
				// main loop
				status = a3windowBeginMainLoop(&wnd);
			}

			// kill rendering context
			status = a3rendererReleaseContext(&renderContext);
		}

		// kill window class
		status = a3windowReleaseClass(&wndClass, hInstance);
	}

	// release console
	status = gpro_consoleReleaseMain(&console);

	// the end
	return 0;
}


#endif	// (defined _WINDOWS || defined _WIN32)