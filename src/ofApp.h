#pragma once

#include "ofMain.h"
#include "ofFileUtils.h"
#include <shlobj.h> 
#include <Shellapi.h> // for shellexecute
#include <shlwapi.h>  // for path functions
#include "SpoutLibrary.h" // Spout SDK
#include "apngdis.h" // animated png
#include "ofxWinMenu.h" // Addon for a windows style menu
#include "tinyxml.h" // XML parser

#pragma comment(lib, "shlwapi.lib")  // for path functions#include "resource.h" // for custom icon, dialogs, menu etc.
#pragma comment(lib, "Version.lib") // for GetFileVersionInfo

// For xml file download
// URL
struct ComInit
{
	HRESULT hr;
	ComInit() : hr(::CoInitialize(nullptr)) {}
	~ComInit() { if (SUCCEEDED(hr)) ::CoUninitialize(); }
};


class ofApp : public ofBaseApp {

	public:

		void setup();
		void update();
		void draw();
		void windowResized(int w, int h);
		void exit();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

        HANDLE GetFirstFile(const char *filepath, char *filename, int maxchars);
		HANDLE GetNextFile(HANDLE &filehandle, char *filename, int maxchars);
		int CountFiles(HANDLE FileHandle);

		void ResizeImageToFbo(ofImage &image);
		void DrawImageToFbo();
        bool OnTimer ();
		bool CheckSettings();
		void ReadSettings();
		void WriteSettings();
		bool OpenFolder(char *foldername, int maxchars); // used by settings dialog
		void doFullScreen(bool bFull);
		void doTopmost(bool bTop);

        ofImage current_image;
		ofImage last_image;
        HANDLE FileHandle;
		char filename[MAX_PATH];
		bool bFullscreen;
		bool bFitToWindow;

		// Variables for the slideshow
        char slideshowpath[MAX_PATH]; // Slideshow folder
		char folderselect[MAX_PATH]; // working folder for OpenFolder to use
		char startingfolder[MAX_PATH]; // initial folder for the browse dialog
		DWORD dwDurationIndex;
		DWORD dwResolutionIndex;
		bool bTransition;
		bool bFadeToBlack;
		bool bRandom;
		bool bWhiteBackground;

		// Variables used by dialogs and callbacks
		HWND g_hwnd; // Application window
		HWND g_hwndForeground; // current foreground window
		HINSTANCE g_hInstance; // Application instance
		HWND g_DialogHwnd; // Settings dialog window
		std::string g_version; // version number;

		// Variables used by WNDPROC
		bool bShowInfo;
		bool bTopmost;
		bool bPause;

		// Menu
		ofxWinMenu * menu; // Menu object
		void appMenuFunction(string title, bool bChecked); // Menu callback function

		// For xml parsing
		TiXmlDocument _xmlDoc;

		bool bNewSettings; // Dialog return flag if user clicked OK or Cancel

		// Timing for transition
		int tMsecs, tIntervalMsec;
		float progress, progress_time, start_msec;

		double startTime, elapsedTime, frameTime, lastTime;
		int nCurrentImage;
		int nImageFiles;

		ofFbo fbo; // used for draw
		ofTrueTypeFont myFont;

		// Spout
		SPOUTLIBRARY *spoutsender; // A sender object
		char sendername[256];     // Sender name
		GLuint sendertexture;     // Local OpenGL texture used for sharing
		bool bInitialized;        // Initialization result
		unsigned int SenderWidth, SenderHeight;

		ofPixels m_Pixels;
		ofImage m_Image;
		std::vector<APNGFrame> m_Frames;
		int m_FrameCounter;
		std::string m_Path;
		DWORD m_dwDuration;
		float m_DisplayFrequency;
		float m_FrameRate;
		float m_FrameDelay;
		float m_FrameSpeed;
		float m_Speed;
		double m_elapsedTime, m_lastTime, m_time;
		double PCFreq;
		__int64 CounterStart;
		bool LoadPng(const char *filepath);
		bool LoadSlide(const char *imagefile);
		float GetRefreshRate();
		void StartCounter();
		double GetCounter();


};
