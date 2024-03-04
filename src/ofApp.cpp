//
//		SpoutSlideshow
//
//		A Slideshow application with Spout output
//
//		Copyright(C) 2017 - 2024 Lynn Jarvis.
//
//		04.10.16 - First working version 1.0
//		06.10.16 - Added random slide - version 1.001
//		13.10.16 - Added verdana font - version 1.002
//		16.10.16 - started application menu and dialog resources
//		17.10.16 - implemented resource dialog and menu - version 1.003
//		18.10.16 - introduced Pause
//				   Implemented using pThis to use class variables in callbacks
//		19.10.16 - Cleanup - Version 1.004
//		20.10.16 - Check for image files directly after browse for folder
//				 - Fix fade save to registry
//				 - Prevent exit with ESC key
//				 - Full screen option 'f'
//				 - Duration delay fix
//		25.10.16 - Version 1.005
//		31.10.16 - fixed choppy fade progress - Version 1.006
//		09.11.16 - Set title bar version number from resources
//		19.11.16 - Dynamic change for add or delete images from the selected folder
//		25.11.16 - Latest release - Version 1.007
//		23.01.17 - Rebuild for 2.006 - OF84 VS2012 /MD - Version 1.008
//		04.08.17 - Add left-right arrow keys for forwards or back while paused
//				 - Add image number and total count to status display
//				 - Add CountFiles function
//		09.08.17 - Add fit to window option
//				 - Version 1.009
//		21.09.18 - Set framerate to 30 to reduce CPU load
//				 - Support animated png images
//				   Alter slide duration to animated png duration if greater
//				   Change duration to msec
//				   Change menu to ofxWinMenu
//				   Rebuild for 2.006 - OF100 VS2017 /MD
//				   Update documentation and zip to SpoutSlideShow_1010.zip
//				 - Version 1.010
//		21.12.18 - Add per monitor DPI aware using Manifest tool
//		03.01.19 - Changed to revised registry functions in SpoutUtils
//			       Rebuild Spout SDK 2.007 /MD (Openframeworks)
//				   Version 1.011
//		29.05.19 - Rebuild Win32 Spout SDK 2.007 /MD (Openframeworks 10)
//				   Version 1.012 - not released
//		25.02.20 - Add Check for update
//		06.04.20 - Change update to WinInet
//					Save downloaded file to user appdata
//				   Rebuild Win32 Spout SDK 2.007 / MD(Openframeworks 10)
//		24.05.20 - Change update to use static website
//				   Rebuild Win32 Spout SDK 2.007 / MD(Openframeworks 10)
//				   Version 1.013 - release
//		06.07.20 - Remove Check for Update
//				   Update solution file to copy x64 dlls to \bin
//		25.08.20 - Update to Openframeworks 11
//				   Change from Spout SDK to SpoutLibrary
//				   Release private project as open source
//				   TODO : Use Openframeworks for xml read
//				   Rebuild Win32 SpoutLibrary 2.007 / MD(Openframeworks 11)
//				   Version 1.014 - GitHub release
//		11.02.21 - Transition between images on start again (https://github.com/leadedge/SpoutSlideshow/issues/1)
//				   Clear background to alpha transparent (https://github.com/leadedge/SpoutSlideshow/issues/2)
//				   Rebuild Win32 (no other changes) - Version 1.015
//		27.02.24 - Update to latest SpoutLibrary and openframeworks 12
//				   SendFbo in DrawToFbo to allow minimized
//				   _HAS_STD_BYTE=0 in preprocessor defines for OF12 and C++17
//				   Change dialog font size from 8 to 9
//				   Create Data/Fonts folder for on-screen font
//				   SpoutMessageBox for help and about dialogs
//				   Save fit-to-window to registry settings
//				   Increase delay options to 30 seconds
//				   Build x64 /MT - Version 1.016
//
#include "ofApp.h"
#include "resource.h"

INT_PTR CALLBACK SlideshowDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg,LPARAM lp, LPARAM pData);
static ofApp *pThis; // Pointer for callbacks to access the ofApp class

// Resolution tables
//  512 x 512   0
//	640 x 360	1
//	640 x 480	2
//	800 x 600	3
//	1024 x 768	4
//  1024 x 1024 5
//	1280 x 720	6 (default)
//	1600 x 960	7
//	1920 x 1080	8
//	2560 x 1440	9
//	3840 x 2160	10
unsigned int resolutionX[11] =  {512, 640, 640, 800, 1024, 1024, 1280, 1600, 1920, 2560, 3840};
unsigned int resolutionY[11] =  {512, 360, 480, 600,  768, 1024,  720,  960, 1080, 1440, 2160};


//--------------------------------------------------------------
void ofApp::setup() {

	
	// Console window so printf works
	// FILE * pCout = NULL;
	// AllocConsole();
	// freopen_s(&pCout, "CONOUT$", "w", stdout);
	// printf("SpoutSlideShow 1.016\n");


	// Get instance
	g_hInstance = GetModuleHandle(NULL);

	// Get product version number
	char title[256];
	char temp[256];
	DWORD dummy; // , dwSize;
	strcpy_s(title, "Spout Slideshow");
	if(GetModuleFileNameA(g_hInstance, temp, 256)) {
		DWORD dwSize = GetFileVersionInfoSizeA(temp, &dummy);
		if(dwSize > 0) {
			vector<BYTE> data(dwSize);
			if(GetFileVersionInfoA(temp, NULL, dwSize, &data[0])) {
				LPVOID pvProductVersion = NULL;
				unsigned int iProductVersionLen = 0;
				if(VerQueryValueA(&data[0], ("\\StringFileInfo\\080904E4\\ProductVersion"), &pvProductVersion, &iProductVersionLen)) {
					strcat_s(title, 256, " - v");
					strcat_s(title, 256, (char *)pvProductVersion);
					g_version = title;
				}
			}
		}
	}
	ofSetWindowTitle(g_version.c_str()); // show it on the title bar

	strcpy_s(sendername, "Spout Slideshow");	// Set the sender name

	// Load a font rather than the default
	GetModuleFileNameA(NULL, slideshowpath, MAX_PATH);
	PathRemoveFileSpecA(slideshowpath); // Application folder
	string fontName = "\\Data\\Fonts\\verdana.ttf";
	string fontPath = slideshowpath + fontName;
	myFont.load(fontPath, 12, true, true);

	//
	// ================ WINDOW MESSAGES AND DIALOGS =======================
	//

	// Get window handles for dialog functions to use
	g_hwnd = WindowFromDC(wglGetCurrentDC());
	g_hwndForeground = GetForegroundWindow();
	g_DialogHwnd = NULL; // Settings modeless dialog window

	// Get a pointer to this class so callbacks can use it
	pThis = this;

	// Set a custom window icon
	SetClassLongPtr(g_hwnd, GCLP_HICON, (LONG_PTR)LoadIconA(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_ICON1)));
	
	// Disable escape key exit so we can exit fullscreen with Escape (see keyPressed)
	ofSetEscapeQuitsApp(false);

	//
	// Create a menu using ofxWinMenu
	//

	// A new menu object with a pointer to this class
	menu = new ofxWinMenu(this, g_hwnd);

	// Register an ofApp function that is called when a menu item is selected.
	// The function can be called anything but must exist. 
	// See the example "appMenuFunction".
	menu->CreateMenuFunction(&ofApp::appMenuFunction);

	// Create a window menu
	HMENU hMenu = menu->CreateWindowMenu();

	//
	// Create a "File" popup menu
	//
	HMENU hPopup = menu->AddPopupMenu(hMenu, "File");
	// Add popup items to the File menu
	menu->AddPopupItem(hPopup, "Settings", false, false); // Not checked and not auto-checked
	menu->AddPopupSeparator(hPopup);
	menu->AddPopupItem(hPopup, "Exit", false, false);

	// Variables used by the menu
	bShowInfo = true;  // screen info display
	bTopmost = false; // app is topmost
	bPause = false; // pause the slideshow
	bNewSettings = false; // Settings dialog return flag if user clicked OK or Cancel

	//
	// Window popup menu
	//
	hPopup = menu->AddPopupMenu(hMenu, "Window");
	bShowInfo = true;
	menu->AddPopupItem(hPopup, "Show info ' '", true); // Checked and auto-check
	bPause = false;
	menu->AddPopupItem(hPopup, "Pause 'p'"); // Not checked and auto-check
	bTopmost = false; // app is not topmost yet
	menu->AddPopupItem(hPopup, "Show on top 't'"); // Not checked (default)
	bFullscreen = false; // not fullscreen yet
	menu->AddPopupItem(hPopup, "Full screen 'f'", false, false); // Not checked and not auto-check
	bFitToWindow = false; // not fitted yet
	menu->AddPopupItem(hPopup, "Fit to window 'w'");

	//
	// Help popup menu
	//
	hPopup = menu->AddPopupMenu(hMenu, "Help");
	menu->AddPopupItem(hPopup, "Keys", false, false); // No auto check
	menu->AddPopupItem(hPopup, "Documentation", false, false); // No auto check
	// menu->AddPopupItem(hPopup, "Check for update", false, false); // No auto check
	menu->AddPopupItem(hPopup, "About", false, false); // No auto check

	// Set the menu to the window
	menu->SetWindowMenu();

	//
	// ================== SETTINGS ==================
	//

	// Default values for the Settings dialog
	tIntervalMsec = 4000; // slideshow interval in msecs
	bTransition = true; // transition
	bFadeToBlack = true; // fade out to black
	bRandom = false; // random image selection
	bWhiteBackground = false; // black background by default
	dwDurationIndex = 2; // 4 seconds
	dwResolutionIndex = 6; // 1280x720

	nCurrentImage = 0;
	nImageFiles = 0;

	// Initialize SpoutLibrary now for registry functions in ReadSettings
	spoutsender = GetSpout();				// Initialize SpoutLibrary
	bInitialized = false;		            // Spout sender initialization
	spoutsender->DisableFrameCount();		// Disable frame counting due to variable load and fps 

	// Read saved starting values if they are in the registry
	ReadSettings();
	
	// Set sender resolution
	SenderWidth  = resolutionX[dwResolutionIndex];
	SenderHeight = resolutionY[dwResolutionIndex];

	// Set window shape
	if(SenderWidth == SenderHeight)
		ofSetWindowShape(512, 512 + GetSystemMetrics(SM_CYMENU)); 
	else
		ofSetWindowShape(640, 360 + GetSystemMetrics(SM_CYMENU)); 

	// Centre on the screen
	ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);

	// FBO setup
	fbo.allocate(SenderWidth, SenderHeight);
	fbo.begin();
	ofClear(0,0,0,255); // clear to black
	fbo.end();
	ofBackground(0, 0, 0, 0);

	// Start values for transition timing
	float msecs = (float)(ofGetElapsedTimeMicros() / 1000);
	tMsecs = (int)msecs; // start msecs
	start_msec = 0;
	progress_time = 2.0f; // total transition time
	progress = 0; // Transition progress 0 - 2.0

	// Set framerate to 30 to reduce CPU load for normal pictures
	ofSetFrameRate(30);

	// Get the monitor vertical refresh freqency in Hz e.g. 60 or 30
	m_DisplayFrequency = GetRefreshRate();
	m_FrameRate = (1.0f/m_DisplayFrequency)*1000.0f; // 16.67 for 60fps
	m_time = m_lastTime = 0.0;
	StartCounter();

	// Get the first image in the folder
	FileHandle = GetFirstFile(slideshowpath, filename, MAX_PATH);

	// Count the files in the folder for status display
	nImageFiles = CountFiles(FileHandle);
	FileHandle = GetFirstFile(slideshowpath, filename, MAX_PATH);

	if(FileHandle) {
		// construct the full path of the first image file
		char imagefile[MAX_PATH];
		sprintf_s(imagefile, "%s\\%s", slideshowpath, filename);
		if(LoadSlide(imagefile)) {
			last_image = current_image; // set the last image for fading
		}
		else {
			char temp[MAX_PATH];
			sprintf_s(temp, MAX_PATH, "Error loading image %s\n", imagefile);
			MessageBoxA(NULL, temp, "Spout Slideshow Error", MB_OK);
			FileHandle = NULL;
		}
	}

} // end Setup


//--------------------------------------------------------------
void ofApp::update() {
	
	char imagefile[MAX_PATH]{};
	char temp[MAX_PATH]{};

	// Calculate frame time for slide duration
	float msecs = (float)(ofGetElapsedTimeMicros()/1000);
	float frame_time = msecs-start_msec;
	if (frame_time < 0) frame_time = 0;
	start_msec = msecs;

	// If the Settings dialog has been opened and user clicked OK
	if(CheckSettings()) {

		// Save the changed settings to the registry
		WriteSettings();

		// Set timing variables from indices
		tIntervalMsec = (int)(dwDurationIndex + 1) * 2000; // msecs
		if(tIntervalMsec == 0) tIntervalMsec = 1;
		if (bFadeToBlack) tIntervalMsec += 1000; // allow for half the transition time

		// Re-set sender resolution
		SenderWidth  = resolutionX[dwResolutionIndex];
		SenderHeight = resolutionY[dwResolutionIndex];

		// Reallocate fbo
		fbo.allocate(SenderWidth, SenderHeight);

		// Start values
		tMsecs = (int)msecs; // start msecs
		progress = 0; // Transition progress 0 - 2.0

		if(nCurrentImage == 0) {
			// Get the first image in the folder
			FileHandle = NULL;
			FileHandle = GetFirstFile(slideshowpath, filename, MAX_PATH);
			if(!FileHandle) {
				MessageBoxA(NULL, "No image files in the folder", "Spout Slideshow Error", MB_OK);
				return;
			}
		}

		// construct the full path of the first image file
		sprintf_s(imagefile, "%s\\%s", slideshowpath, filename);

		if(!LoadSlide(imagefile)) {
			sprintf_s(temp, MAX_PATH, "Error loading image %s\n", imagefile);
			MessageBoxA(NULL, temp, "Spout Slideshow Error", MB_OK);
			return;
		}

		// resize the image to fit the window height
		ResizeImageToFbo(current_image);
		last_image = current_image; // set the last image for fading

		return;

	} // end Settings dialog

	if(!bPause) {

		if(FileHandle) {

			// Progress for transition should be 1 second out and 1 second in
			if(progress <= progress_time) { // 2.0
				if(frame_time > 0.0) {
					progress += frame_time/1000.0;
					if(progress > progress_time) progress = progress_time;
				}
			}

			// animated png
			if (!m_Frames.empty()) {

				// Aninmation speed is multiple of frame rate at 60fps = 16.667 msec
				// m_FrameSpeed is the multiple of 16.667 required
				ofTime time = ofGetCurrentTime();
				m_time = time.getAsMilliseconds();
				m_elapsedTime = m_time - m_lastTime;

				if (m_elapsedTime >= (double)m_FrameDelay) {
					if (m_FrameCounter < (int)m_Frames.size()) {
						m_lastTime = m_time;
						m_Pixels.setFromExternalPixels(m_Frames[m_FrameCounter].p, m_Frames[m_FrameCounter].w, m_Frames[m_FrameCounter].h, 4);
						current_image.setFromPixels(m_Pixels);
						ResizeImageToFbo(current_image);
						m_FrameCounter++;
					}
					else {
						m_FrameCounter = 0;
					}
				}
			}

			if (((int)msecs - tMsecs) > tIntervalMsec) {
				// m_Frames.size() will be zero if the image is not animated png
				// Otherwise wait for the animation to end
				if (m_FrameCounter > (int)m_Frames.size() - 1) {
					last_image = current_image;
					OnTimer(); // Load a new current image
					start_msec = (float)(ofGetSystemTimeMicros() / 1000);
					tMsecs = (int)msecs;
					progress = 0.0;
					m_FrameCounter = 0;
				}
			}
		}
	}
	else {
		// Set to start of fade
		msecs = (float)tIntervalMsec;
		tMsecs = tIntervalMsec;
		progress = 0.0;
	}

} // end Update


//--------------------------------------------------------------
void ofApp::draw() {

	char str[256]{};
	int alpha = 255;
	ofBackground(0, 0, 0, 255);
	ofSetColor(255, 255, 255, alpha);

	ofEnableAlphaBlending();

	DrawImageToFbo();
	fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	if(spoutsender->IsInitialized()
		&& bShowInfo && !bFullscreen
		&& ofGetWidth() > 0 && ofGetHeight() > 0) {
			
		// Show what it is sending
		ofSetColor(255);
		sprintf_s(str, "Sending as : [%s] (%d x %d)", sendername, SenderWidth, SenderHeight);
		myFont.drawString(str, 20, 20);

		// Warn if no images
		if(!FileHandle) {
		     myFont.drawString("No image files in the folder", 20, 42);
		}
		else {
			sprintf_s(str, "%s (%d of %d)", filename, nCurrentImage, nImageFiles);
			myFont.drawString(str, 20, 42);
		}

		// The current folder
		// sprintf_s(str, "Folder : %s", slideshowpath);
		// myFont.drawString(str, 20, ofGetHeight()-42);

		// How to open settings
		sprintf_s(str, "' ' hide info  :  ?  help  :  Right mouse click - Settings");
		myFont.drawString(str, 20, ofGetHeight()-20);

	}

} // end Draw


void ofApp::DrawImageToFbo()
{
	int alpha = 255;

	if(FileHandle) {

		fbo.begin();
		// Clear backgound to alpha transparent
		if(bWhiteBackground)
			ofClear(255, 255, 255, 0);
		else
			ofClear(0, 0, 0, 0);
		
		// If paused, just draw and send the current image
		if(bPause) {
			current_image.draw((fbo.getWidth()-current_image.getWidth())/2,
								(fbo.getHeight()-current_image.getHeight())/2, 
								current_image.getWidth(),
								current_image.getHeight());

			spoutsender->SendFbo(fbo.getId(), SenderWidth, SenderHeight, false);

			fbo.end();
			return;
		}

		// Transition between two images
		if (bTransition) {
			ofEnableAlphaBlending();
			if(bFadeToBlack) {
				if(progress < 1.0) { // Fade out to black
					alpha = 255-(int)(progress*255.0);
					ofSetColor(255, 255, 255, alpha);
					last_image.draw((fbo.getWidth()-last_image.getWidth())/2,
									(fbo.getHeight()-last_image.getHeight())/2, 
									last_image.getWidth(),
									last_image.getHeight());
				}
				else { // Fade in to full colour
					alpha = (int)((progress-1.0)*255.0);
					ofSetColor(255, 255, 255, alpha);
					current_image.draw((fbo.getWidth()-current_image.getWidth())/2,
										(fbo.getHeight()-current_image.getHeight())/2, 
										current_image.getWidth(),
										current_image.getHeight());
				}
			}
			else {

				// Fade from the last image to the next
				if (progress < 1.0) {
					alpha = 255-(int)(progress*255.0);
					ofSetColor(255, 255, 255, alpha);
					last_image.draw((fbo.getWidth()-last_image.getWidth())/2,
									(fbo.getHeight()-last_image.getHeight())/2, 
									last_image.getWidth(),
									last_image.getHeight());
					alpha = (int)(progress*255.0);
					ofSetColor(255, 255, 255, alpha);
					current_image.draw((fbo.getWidth()-current_image.getWidth())/2,
										(fbo.getHeight()-current_image.getHeight())/2, 
										current_image.getWidth(),
										current_image.getHeight());
				}
				else {
					current_image.draw((fbo.getWidth()-current_image.getWidth())/2,
										(fbo.getHeight()-current_image.getHeight())/2, 
										current_image.getWidth(),
										current_image.getHeight());
				}
			}
			ofDisableAlphaBlending();
		}
		else { // no transition
			current_image.draw((fbo.getWidth()-current_image.getWidth())/2,
								(fbo.getHeight()-current_image.getHeight())/2, 
								current_image.getWidth(),
								current_image.getHeight());
		}

		// Send the contents of the fbo
		spoutsender->SendFbo(fbo.getId(), SenderWidth, SenderHeight, false);

		fbo.end();
	}
} // end DrawImageToFbo


// resize the image to fit the screen height
void ofApp::ResizeImageToFbo(ofImage &image)
{
	float width, height;

	if(!fbo.isAllocated())
		return;

	if(bFitToWindow) {
		image.resize((int)fbo.getWidth(), (int)fbo.getHeight());
		return;
	}

	width  = (float)image.getWidth();
	height = (float)image.getHeight();

	// Fbo width is always greater than the height
	// according to the selections in Settings dialog
	if(image.getHeight() > image.getWidth()) {
		height = fbo.getHeight();
		width  = fbo.getHeight()*image.getWidth()/image.getHeight(); // OK
	}
	else {
		width  = fbo.getWidth();
		height = fbo.getWidth()*image.getHeight()/image.getWidth();
		if(height > fbo.getHeight()) {
			width  = (float)width*fbo.getHeight()/(float)height;
			height = fbo.getHeight();
		}
	}

	if(width > 0 && height > 0)
		image.resize((int)width, (int)height);

} // end ResizeImageToFbo


// Load the next image - slideshowpath is global
bool ofApp::OnTimer () 
{
	char imagefile[MAX_PATH];
	int nThisImage = 0;
	int nLastImage = 0;
	int nImage = 0;

	nImageFiles = 0;

	if(!FileHandle) { // No file yet
		FileHandle = GetFirstFile(slideshowpath, filename, MAX_PATH);
		if(!FileHandle) return false;
	}
	else { 
		if(bRandom) {
			// Count the files
			nLastImage = nCurrentImage;
			FileHandle = GetFirstFile(slideshowpath, filename, MAX_PATH);
			if(!FileHandle) return false;
			nImageFiles = CountFiles(FileHandle);
			// Random number between 0 and nImageFiles
			nThisImage = rand() % nImageFiles+1;
			if(nThisImage == nLastImage) { // skip the same one
				nThisImage++;
				if(nThisImage > nImageFiles)
					nThisImage = 0;
			}
			if(nThisImage > nImageFiles) {
				nThisImage = nImageFiles; // safety
				if(nThisImage < 0) nThisImage = 0;
			}
			// Get the random image starting from the first
			FileHandle = GetFirstFile(slideshowpath, filename, MAX_PATH);
			while(nImage < nThisImage) {
				FileHandle = GetNextFile(FileHandle, filename, MAX_PATH);
				nImage++;
			}
		}
		else {
			// Count the files
			nLastImage = nCurrentImage;
			FileHandle = GetFirstFile(slideshowpath, filename, MAX_PATH);
			if(!FileHandle) return false;
			nImageFiles = CountFiles(FileHandle);

			// Get the next image number
			nThisImage = nLastImage+1;
			if(nThisImage > nImageFiles)
					nThisImage = 0;

			// Get the next image file starting from the first
			FileHandle = GetFirstFile(slideshowpath, filename, MAX_PATH);
			nImage = 0;
			nCurrentImage = 0;
			while(nImage < nThisImage) {
				FileHandle = GetNextFile(FileHandle, filename, MAX_PATH);
				nImage++;
			}
			// If no more, get the first again
			if(!FileHandle) FileHandle = GetFirstFile(slideshowpath, filename, MAX_PATH);
		}
	}
	
	if(!FileHandle) return false;

	// construct the full path image file name for processing
	sprintf_s(imagefile, "%s\\%s", slideshowpath, filename);

	// load the image
	return(LoadSlide(imagefile));

} // end OnTimer


int ofApp::CountFiles(HANDLE FileHandle)
{
	char fname[MAX_PATH];
	int nFiles = 0;
	while(FileHandle) {
		FileHandle = GetNextFile(FileHandle, fname, MAX_PATH);
		if(FileHandle) {
			nFiles++;
		}
	}
	return nFiles;
}


HANDLE ofApp::GetFirstFile(const char *spath, char *filename, int maxchars)
{
    char tmp[MAX_PATH];
	HANDLE filehandle = NULL;
	WIN32_FIND_DATAA filedata;

	sprintf_s(tmp, MAX_PATH, "%s\\*.*", spath);
    filehandle = FindFirstFileA((LPCSTR)tmp, (LPWIN32_FIND_DATAA)&filedata);
    if(PtrToUint(filehandle) > 0) {
		// Check extension for allowed image types
		while (strstr((char *)filedata.cFileName, ".jpg") == 0
			&& strstr((char *)filedata.cFileName, ".png") == 0
			&& strstr((char *)filedata.cFileName, ".gif") == 0
			&& strstr((char *)filedata.cFileName, ".bmp") == 0
			&& strstr((char *)filedata.cFileName, ".ppm") == 0
			&& strstr((char *)filedata.cFileName, ".psd") == 0
			&& strstr((char *)filedata.cFileName, ".tga") == 0
			&& strstr((char *)filedata.cFileName, ".tif") == 0) {
			if(!FindNextFileA(filehandle, (LPWIN32_FIND_DATAA)&filedata)) {
				filehandle = NULL;
				break;
			}
		}
    }

	if(filehandle) {
		strcpy_s(filename, maxchars, (const char *)filedata.cFileName);
		nCurrentImage = 0;
	}
  
	return filehandle;

} // end GetFirstFile


HANDLE ofApp::GetNextFile(HANDLE &filehandle, char *filename, int maxchars)
{
	HANDLE nexthandle = NULL;
	WIN32_FIND_DATAA filedata;

	if(!filehandle) return false;

	if(FindNextFileA(filehandle, (LPWIN32_FIND_DATAA)&filedata)) {
		// Check extension for allowed image types
		while (strstr((char *)filedata.cFileName, ".jpg") == 0
			&& strstr((char *)filedata.cFileName, ".png") == 0
			&& strstr((char *)filedata.cFileName, ".gif") == 0
			&& strstr((char *)filedata.cFileName, ".bmp") == 0
			&& strstr((char *)filedata.cFileName, ".ppm") == 0
			&& strstr((char *)filedata.cFileName, ".psd") == 0
			&& strstr((char *)filedata.cFileName, ".tga") == 0
			&& strstr((char *)filedata.cFileName, ".tif") == 0) {
			if(!FindNextFileA(filehandle, (LPWIN32_FIND_DATAA)&filedata)) {
				filehandle = NULL;
				break;
			}
		}
	}
	else {
		filehandle = NULL;
	}
    
	if(filehandle) {
		strcpy_s(filename, maxchars, (const char *)filedata.cFileName);
		nCurrentImage++;
	}
    
	return filehandle;

} // end GetNextFile

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	ResizeImageToFbo(current_image);
	if(nCurrentImage > 0) ResizeImageToFbo(last_image);
}

//--------------------------------------------------------------
void ofApp::exit() {
	if(bInitialized) spoutsender->ReleaseSender(); // Release the sender
	// Save settings to the registry
	WriteSettings();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	
	// Escape key exit has been disabled but it can still be checked here
	if (key == VK_ESCAPE) {
		// Disable fullscreen set, otherwise quit the application as usual
		if (bFullscreen) {
			bFullscreen = false;
			doFullScreen(false);
		}
		else {
			if (MessageBoxA(NULL, "Exit - are your sure?", "Warning", MB_YESNO) == IDYES) {
				if (bInitialized)
					spoutsender->ReleaseSender(); // Release the sender
				ofExit();
			}
		}
	}

	// Show keyboard shortcuts
	if (key == '?') {
		string showinfo;
		showinfo = "'  '  :  Hide onscreen text\n";
		showinfo += "'p'  :  Pause slideshow\n";
		showinfo += "'>' :  Next slide\n";
		showinfo += "'<' :  Previous slide\n";
		showinfo += "'f'  :  Full screen\n";
		showinfo += "'t'  :  Topmost window\n";
		showinfo += "'w' :  Fit to window\n";
		spoutsender->SpoutMessageBox(NULL, (LPSTR)showinfo.c_str(), "SpoutSlideShow", MB_ICONINFORMATION | MB_OK);
	}

	// Remove or show screen info
	if(key == ' ') {
		bShowInfo = !bShowInfo;
	}

	// Pause on the current image
	if(key == 'p') {
		bPause = !bPause;
		if(bPause) {
			// If fade out and in again, use the image it is fading from if progress
			// is less than half way, otherwise use the image it is fading to.
			if(bFadeToBlack && progress > 0 && progress < 1.0)
				current_image = last_image;
			else
				last_image = current_image;
		}
	}

	// Right arrow - next image when paused
	if(key == OF_KEY_RIGHT || key == '>') {
		bPause = true;
		OnTimer(); // Next image
	}

	// Left arrow - previous image when paused
	if(key == OF_KEY_LEFT || key == '<') {
		bPause = true;
		// Needs tracing - but it works
		if(nCurrentImage > 0) {
			if(nCurrentImage == 1) {
				// Get the first file
				nCurrentImage = 0;
				FileHandle = GetFirstFile(slideshowpath, filename, MAX_PATH);
				nImageFiles = CountFiles(FileHandle);
			}
			else {
				nCurrentImage -= 2;
				if(nCurrentImage < 0) 
					nCurrentImage = 0;
			}
			OnTimer();
		}
	}

	if(key == 'f'){
		bFullscreen = !bFullscreen;
		doFullScreen(bFullscreen);
	}

	if (key == 't') {
		bTopmost = !bTopmost;
		doTopmost(bTopmost);
	}

	if (key == 'w') {
		bFitToWindow = !bFitToWindow;
	}
}


void ofApp::doFullScreen(bool bFull)
{
	// Enter full screen
	if(bFull) {
		// Remove the menu but don't destroy it
		menu->RemoveWindowMenu();
		// hide the cursor
		ofHideCursor();
		ofSetFullscreen(true);
	}
	else { 
		// return from full screen
		ofSetFullscreen(false);
		// Centre on the screen
		ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);
		// Restore the menu
		menu->SetWindowMenu();
		// Set window shape
		if(SenderWidth == SenderHeight)
			ofSetWindowShape(512, 512 + GetSystemMetrics(SM_CYMENU)); 
		else
			ofSetWindowShape(640, 360 + GetSystemMetrics(SM_CYMENU)); 

		// Centre on the screen
		ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);
		bFullscreen = false;
		bFitToWindow = false;

		// Repaint the entire desktop
		RECT rect;
		GetClientRect(GetDesktopWindow(), &rect);
		RedrawWindow(GetDesktopWindow(), &rect, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
		// Show the cursor again
		 ofShowCursor();
		// Restore topmost state
		if(bTopmost) doTopmost(true);
	}

}


void ofApp::doTopmost(bool bTop)
{
	if(bTop) {
		// get the current top window for return
		g_hwndForeground = GetForegroundWindow();
		// Set this window topmost
		SetWindowPos(g_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
		ShowWindow(g_hwnd, SW_SHOW);
	}
	else {
		SetWindowPos(g_hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		ShowWindow(g_hwnd, SW_SHOW);
		// Reset the window that was topmost before
		if(GetWindowLong(g_hwndForeground, GWL_EXSTYLE) & WS_EX_TOPMOST)
			SetWindowPos(g_hwndForeground, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
		else
			SetWindowPos(g_hwndForeground, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); 
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if(key == 32) {
		// Space key pressed - next slide if set for key selection
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

	if(button == 2) { // rh button
		// Open the the Settings dialog window if closed
		if(!g_DialogHwnd) {
			// Set working folder for dialog
			strcpy_s(folderselect, MAX_PATH, slideshowpath);
			// Create a modeless dialog
			g_DialogHwnd = CreateDialogA(g_hInstance, MAKEINTRESOURCEA(IDD_SLIDESHOWBOX), g_hwnd, SlideshowDlgProc);
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


// ======================== SETTINGS FUNCTIONS =======================
void ofApp::ReadSettings()
{
	// For registry read
	DWORD dwWhite = 0;
	DWORD dwRandom = 0;
	DWORD dwTransition = 1;
	DWORD dwFade = 1;
	DWORD dwFit = 0;
	DWORD dwDuration = 2; // 4 secs
	DWORD dwResolution = 6;
	char path[MAX_PATH];
	strcpy_s(path, MAX_PATH, slideshowpath);

	spoutsender->ReadPathFromRegistry (HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "folder", path);
	spoutsender->ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "resolution", &dwResolution);
	spoutsender->ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "duration", &dwDuration);
	spoutsender->ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "white", &dwWhite);
	spoutsender->ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "random", &dwRandom);
	spoutsender->ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "transition", &dwTransition);
	spoutsender->ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "fade", &dwFade);
	spoutsender->ReadDwordFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "fit", &dwFit);
	
	// Set variables
	if(path[0]) strcpy_s(slideshowpath, MAX_PATH, path);
	dwDurationIndex = dwDuration;
	dwResolutionIndex = dwResolution;
	tIntervalMsec = (int)(dwDuration + 1) * 2000;
	if (tIntervalMsec == 0) tIntervalMsec = 1000;

	bRandom = (dwRandom == 1);
	bWhiteBackground = (dwWhite == 1);
	bTransition = (dwTransition == 1); // transition
	bFadeToBlack = (dwFade == 1); // fade out to black
	if (bFadeToBlack) tIntervalMsec += 1000; // allow for half the transition time
	bFitToWindow = (dwFit == 1); // fit to window

}


void ofApp::WriteSettings()
{
	spoutsender->WritePathToRegistry (HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "folder", slideshowpath);
	spoutsender->WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "resolution", dwResolutionIndex);
	spoutsender->WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "duration", dwDurationIndex);
	spoutsender->WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "white", (DWORD)bWhiteBackground);
	spoutsender->WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "random", (DWORD)bRandom);
	spoutsender->WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "transition", (DWORD)bTransition);
	spoutsender->WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "fade", (DWORD)bFadeToBlack);
	spoutsender->WriteDwordToRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutSlideshow", "fit", (DWORD)bFitToWindow);


}


//
//	Check if the Settings dialog has been opened (Modeless hwnd exists)
//
bool ofApp::CheckSettings()
{
		if(g_DialogHwnd) { // Modal dialog window
			if (!IsWindow(g_DialogHwnd)) { // It has now closed
				g_DialogHwnd = NULL;
				// Return true only if the user clicked OK
				if(bNewSettings) {
					// Set window shape
					if(resolutionX[dwResolutionIndex] == resolutionY[dwResolutionIndex])
						ofSetWindowShape(512, 512 + GetSystemMetrics(SM_CYMENU)); 
					else
						ofSetWindowShape(640, 360 + GetSystemMetrics(SM_CYMENU)); 
					// Centre on the screen
					ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);
					return true;
				}
			}
		}
		return false;

}


// Used by the settings dialog to choose a folder
bool ofApp::OpenFolder(char *foldername, int maxchars)
{
	char szDir[MAX_PATH];
	BROWSEINFOA bInfo;
	bInfo.hwndOwner = NULL; // Owner window
	bInfo.pidlRoot = NULL; 
	bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = "Please select a folder"; // Title of the dialog
	bInfo.ulFlags = 0 ;
	bInfo.lpfn = BrowseCallbackProc;
	bInfo.lParam = 0;
	bInfo.iImage = -1;

	// Set starting folder for Browse
	strcpy_s(pThis->startingfolder, MAX_PATH, foldername);

	LPITEMIDLIST lpItem = SHBrowseForFolderA( &bInfo);
	if( lpItem != NULL ) {
		SHGetPathFromIDListA(lpItem, szDir );
		strcpy_s(foldername, maxchars, szDir);
		return true;
	}

	return false;
}


//
// =========================== CALLBACKS =========================
//

//
// Menu function callback
//
// This function is called by ofxWinMenu when an item is selected.
// The the title and state can be checked for required action.
// 
void ofApp::appMenuFunction(string title, bool bChecked) {

	ofFileDialogResult result;
	string filePath;

	//
	// File menu
	//
	if (title == "Settings") {
		// Open the the Settings dialog window if closed
		if (!g_DialogHwnd) {
			// Set working folder for dialog
			strcpy_s(folderselect, MAX_PATH, slideshowpath);
			// Create a modeless dialog
			g_DialogHwnd = CreateDialogA(g_hInstance, MAKEINTRESOURCEA(IDD_SLIDESHOWBOX), g_hwnd, SlideshowDlgProc);
		}
	}

	if (title == "Exit") {
		ofExit(); // Quit the application
	}

	//
	// Window menu
	//

	if (title == "Show info ' '") {
		bShowInfo = bChecked;
	}

	if (title == "Pause 'p'") {
		bPause = bChecked;
		if (bPause) {
			// If fade out and in again, use the image it is fading from if progress
			// is less than half way, otherwise use the image it is fading to.
			if (bFadeToBlack && progress > 0 && progress < 1.0)
				current_image = last_image;
			else
				last_image = current_image;
		}
	}

	if (title == "Show on top 't'") {
		bTopmost = bChecked;
		doTopmost(bTopmost);
	}

	if (title == "Full screen 'f'") {
		bFullscreen = !bFullscreen; // Not auto-checked and also used in the keyPressed function
		doFullScreen(bFullscreen); // But take action immediately
	}

	if (title == "Fit to window 'w'") {
		bFitToWindow = bChecked;
	}

	//
	// Help menu
	//
	if (title == "Keys") {
		string showinfo;
		showinfo = "'  '  :  Hide onscreen text\n";
		showinfo += "'p'  :  Pause slideshow\n";
		showinfo += "'>' :  Next slide\n";
		showinfo += "'<' :  Previous slide\n";
		showinfo += "'f'  :  Full screen\n";
		showinfo += "'t'  :  Topmost window\n";
		showinfo += "'w' :  Fit to window\n";
		spoutsender->SpoutMessageBox(NULL, (LPSTR)showinfo.c_str(), "SpoutSlideShow", MB_ICONINFORMATION | MB_OK);
	}

	if (title == "About") {
		std::string about =  g_version.c_str();
		about += "\n";
		about += "<a href=\"http://spout.zeal.co/\">http://spout.zeal.co/</a>\n";
		// Custom icon for the SpoutMessagBox, activated by MB_USERICON
		spoutsender->SpoutMessageBoxIcon(LoadIconA(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_ICON1)));
		spoutsender->SpoutMessageBox(NULL, (LPSTR)about.c_str(), "SpoutSlideShow", MB_USERICON | MB_OK);
	}

	if (title == "Documentation") {
		char path[MAX_PATH];
		HMODULE hModule = GetModuleHandle(NULL);
		GetModuleFileNameA(hModule, path, MAX_PATH);
		PathRemoveFileSpecA(path);
		strcat_s(path, MAX_PATH, "\\SpoutSlideShow.pdf");
		ShellExecuteA(g_hwnd, "open", path, NULL, NULL, SW_SHOWNORMAL);
	}

} // end appMenuFunction


// Message handler for Settings dialog
INT_PTR CALLBACK SlideshowDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam); // suppress warning
	// char str1[MAX_PATH];
	HWND hwndList = NULL;
	// int pos, lbItem;
	char name[MAX_PATH];
	int nSenders = 0;
	int index = 0;
	char durationprefs[15][128] =  {"2", "4 (default)", "6", "8", "10", "12", "14", "16", "18", "20", "22", "24", "26", "28", "30"};
	char resolutionprefs[11][128] =  {"512 x 512", "640 x 360", "640 x 480", "800 x 600", "1024 x 768", "1024 x 1024", "1280 x 720 (default)", "1600 x 960", "1920 x 1080",  "2560 x 1440",  "3840 x 2160"};

	switch (message) {

		case WM_INITDIALOG:

			// Slideshow folder
			SetDlgItemTextA(hDlg, IDC_FOLDER, (LPCSTR)pThis->slideshowpath);

			//  Select all text in the edit field
            SendDlgItemMessage (hDlg, IDC_FOLDER, EM_SETSEL, 0, 0x7FFF0000L);

			// Duration list
			hwndList = GetDlgItem(hDlg, IDC_DURATION);
			for (int k = 0; k < 15; k ++) {
				strcpy_s(name, sizeof(name), durationprefs[k]);
				SendMessageA(hwndList, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)name);
			}
			SendMessageA(hwndList, CB_SETCURSEL, (WPARAM)pThis->dwDurationIndex, (LPARAM)0);

			// Resolution list
			hwndList = GetDlgItem(hDlg, IDC_RESOLUTION);
			for (int k = 0; k < 11; k ++) {
				strcpy_s(name, sizeof(name), resolutionprefs[k]);
				SendMessageA(hwndList, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)name);
			}
			// Display an initial item in the selection field  
			SendMessageA(hwndList, CB_SETCURSEL, (WPARAM)pThis->dwResolutionIndex, (LPARAM)0);

			// Set checkboxes
			if(pThis->bWhiteBackground) CheckDlgButton(hDlg, IDC_WHITE, BST_CHECKED);
			else     CheckDlgButton(hDlg, IDC_WHITE, BST_UNCHECKED);

			if(pThis->bRandom) CheckDlgButton(hDlg, IDC_RANDOM, BST_CHECKED);
			else     CheckDlgButton(hDlg, IDC_RANDOM, BST_UNCHECKED);

			if(pThis->bTransition) CheckDlgButton(hDlg, IDC_TRANSITION, BST_CHECKED);
			else     CheckDlgButton(hDlg, IDC_TRANSITION, BST_UNCHECKED);

			if(pThis->bFadeToBlack) CheckDlgButton(hDlg, IDC_FADE, BST_CHECKED);
			else     CheckDlgButton(hDlg, IDC_FADE, BST_UNCHECKED);

			if(IsDlgButtonChecked(hDlg, IDC_TRANSITION) == BST_CHECKED)
				EnableWindow(GetDlgItem(hDlg, IDC_FADE), TRUE);
			else
				EnableWindow(GetDlgItem(hDlg, IDC_FADE), FALSE);

			return (INT_PTR)TRUE;

		case WM_COMMAND:

			switch(LOWORD(wParam)) {

			case IDC_BROWSE :
				if(pThis->OpenFolder(pThis->folderselect, MAX_PATH)) {
					// Check the folder for images
					HANDLE filehandle = pThis->GetFirstFile(pThis->folderselect, name, MAX_PATH);
					if(!filehandle)
						MessageBoxA(NULL, "No image files in the folder", "Spout Slideshow Error", MB_OK);
					// Set the selected folder anyway
					SetDlgItemTextA(hDlg, IDC_FOLDER, (LPCSTR)pThis->folderselect);
				}
				break;

				// Resolution
				case IDC_RESOLUTION :
					if(HIWORD(wParam) == CBN_SELCHANGE) {
						pThis->dwResolutionIndex = SendMessage((HWND)lParam, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
					}
					break;
				
				// Duration
				case IDC_DURATION :
					if(HIWORD(wParam) == CBN_SELCHANGE) {
						pThis->dwDurationIndex = SendMessage((HWND)lParam, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
					}
					break;

				case IDC_TRANSITION :
					if(IsDlgButtonChecked(hDlg, IDC_TRANSITION) == BST_CHECKED)
						EnableWindow(GetDlgItem(hDlg, IDC_FADE), TRUE);
					else
						EnableWindow(GetDlgItem(hDlg, IDC_FADE), FALSE);
					break;

				case IDOK :

					// Selected slideshow folder
					if(strcmp(pThis->slideshowpath, pThis->folderselect) != 0) {
						pThis->nCurrentImage = 0; // Set image number to zero for the new folder
						strcpy_s(pThis->slideshowpath, MAX_PATH, pThis->folderselect);
					}
					
					//
					// Set ofApp variables according to checkboxes
					//

					if(IsDlgButtonChecked(hDlg, IDC_WHITE) == BST_CHECKED)
						pThis->bWhiteBackground = true;
					else
						pThis->bWhiteBackground = false;

					if(IsDlgButtonChecked(hDlg, IDC_RANDOM) == BST_CHECKED)
						pThis->bRandom = true;
					else
						pThis->bRandom = false;

					if(IsDlgButtonChecked(hDlg, IDC_TRANSITION) == BST_CHECKED)
						pThis->bTransition = true;
					else
						pThis->bTransition = false;

					if(IsDlgButtonChecked(hDlg, IDC_FADE) == BST_CHECKED)
						pThis->bFadeToBlack = true;
					else
						pThis->bFadeToBlack = false;

					// Destroy window but not it's handle to check later
					DestroyWindow(hDlg);
					pThis->bNewSettings = true; // flag to indicate OK pressed
					return (INT_PTR)TRUE;

				case IDCANCEL :
                    // User pressed cancel.  Just take down dialog box.
					DestroyWindow(hDlg);
					pThis->bNewSettings = false;
					return (INT_PTR)TRUE;

				default:
					break;

			}

			break;
	}
	return (INT_PTR)FALSE;
}


// Used by the OpenFolder function from the settings dialog to choose a folder
INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg,LPARAM lp, LPARAM pData) 
{
    TCHAR szDir[MAX_PATH];
	TCHAR initialDir[MAX_PATH];
	
	mbstowcs_s(NULL, &initialDir[0], MAX_PATH, &pThis->startingfolder[0], MAX_PATH);

    switch(uMsg) 
    {
    case BFFM_INITIALIZED: 
        _tcscpy_s(szDir, MAX_PATH, initialDir);
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szDir);
        break;
    }

    return 0;
}


// ============================================================================
//                         Animated png
// ============================================================================
bool ofApp::LoadPng(const char *filepath)
{
	std::string spath;

	m_Frames.clear();
	
	// make sure it is a png file
	if(strstr(filepath, ".png")) {

		spath = filepath;
		// Update the global path for comparison with new entries
		m_Path = spath;

		// Remove leading and trailing double-quote characters in case this is a copy/paste
		if ( spath.front() == '"' ) {
			spath.erase( 0, 1 ); // erase the first character
			spath.erase( spath.size() - 1 ); // erase the last character
		}

		if(load_apng((char *)spath.c_str(), m_Frames) >= 0) {

			if(m_Frames.size() == 1) {
				m_Frames.clear();
				return false;
			}

			m_Pixels.allocate(m_Frames[0].w, m_Frames[0].h, OF_IMAGE_COLOR_ALPHA);
			m_Image.allocate(m_Frames[0].w, m_Frames[0].h, OF_IMAGE_COLOR_ALPHA);
			current_image.allocate((int)fbo.getWidth(), (int)fbo.getHeight(), OF_IMAGE_COLOR_ALPHA);
			m_elapsedTime = 0.0;
			m_lastTime = 0.0;
			m_FrameCounter = 0;

			ofTime time = ofGetCurrentTime();
			m_time = time.getAsMilliseconds();
			m_lastTime = m_time;
			
			// Frame delay
			// e.g. typically
			//     delay_num = 6     Frame delay fraction numerator
			//     delay_den = 100   Frame delay fraction denomintor
			//
			// https://wiki.mozilla.org/APNG_Specification
			//
			// The `delay_num` and `delay_den` parameters together specify a fraction
			// indicating the time to display the current frame, in seconds. 
			// If the denominator is 0, it is to be treated as if it were 100 
			// (that is, `delay_num` then specifies 1/100ths of a second). 
			// If the the value of the numerator is 0 the decoder should render the next frame
			// as quickly as possible, though viewers may impose a reasonable lower bound. 
			unsigned int delay_den = m_Frames[0].delay_den;
			unsigned int delay_num = m_Frames[0].delay_num;
			if(delay_den == 0) 
				delay_den = 100;
			if(delay_num == 0)
				m_FrameDelay = (1.0f/m_DisplayFrequency)*1000.0f; // fast as possible e.g. 16.67msec for 60fps
			else
				m_FrameDelay = ((float)m_Frames[0].delay_num/(float)m_Frames[0].delay_den)*1000.0f; // in msec

			// FrameRate is the current monitor vsync frame time - 16.67 for 60fps
			// FrameDelay is the delay between frames required in msec
			// FrameSpeed is the multiple of frame rate used to control the delay between frames
			m_FrameSpeed = m_FrameDelay/m_FrameRate;
			// set dwDuration to be the total time of the animated png
			float duration = m_FrameDelay * (float)m_Frames.size(); // msecs
			// round up
			m_dwDuration = (DWORD)duration; // msecs

			return true;
		}
	}

	return false;
}

float ofApp::GetRefreshRate()
{
	float frate = 60.0;
	float frequency = 60.0;
	DEVMODE DevMode;
	BOOL bResult = TRUE;
	DWORD dwCurrentSettings = 0;
	DevMode.dmSize = sizeof(DEVMODE);
	while (bResult) {
		bResult = EnumDisplaySettings(NULL, dwCurrentSettings, &DevMode);
		if (bResult)
			frequency = (float)DevMode.dmDisplayFrequency;
		dwCurrentSettings++;
	}
	frate = frequency;
	return frate;
}

void ofApp::StartCounter()
{
    LARGE_INTEGER li;
	// Find frequency
    QueryPerformanceFrequency(&li);
    PCFreq = double(li.QuadPart)/1000.0;
	// Second call needed
    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;

}

double ofApp::GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}

// Load image - can be animated png or normal image
bool ofApp::LoadSlide(const char *imagefile)
{
	bool bRet = false;

	// printf("LoadSlide [%s]\n", imagefile);
	m_Frames.clear();
	m_FrameCounter = 0;

	// Default user selected interval between slides
	tIntervalMsec = (int)(dwDurationIndex + 1) * 2000;
	if (tIntervalMsec == 0) tIntervalMsec = 1;
	if (bFadeToBlack) tIntervalMsec += 1000;

	if(LoadPng(imagefile)) {
		ofSetFrameRate(60);
		// Calculate interval for animated png
		// If the animated png plays longer, use this as the interval
		int interval = tIntervalMsec;
		if ((int)m_dwDuration > interval) {
			tIntervalMsec = (int)m_dwDuration; // duration of the animated png in msecs
			if (tIntervalMsec == 0) tIntervalMsec = 1;
			if (bFadeToBlack) tIntervalMsec += 1000;
			tIntervalMsec += (int)m_FrameDelay;
		}
		bRet = true;
	}
	else if(current_image.load(imagefile)) {
		ofSetFrameRate(30);
		// User set slide interval
		bRet = true;
	}

	if(bRet) {
		// resize the image to fit the window height
		ResizeImageToFbo(current_image);
		return true;
	}

	return false;
}
