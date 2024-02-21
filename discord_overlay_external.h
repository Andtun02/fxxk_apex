/*
 * https://github.com/SamuelTulach/OverlayCord
 */

#ifndef OVERLAYCORD_H
#define OVERLAYCORD_H

#include <Windows.h>
#include <iostream>
#include <string>
#include <TlHelp32.h>
#include <vector>

#include "skCrypter.h"
 // Skia库
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SKColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFont.h"

#pragma comment(lib, "skia.lib")
#include <d3d12.h>
#pragma comment(lib, "D3D12.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "Opengl32.lib")

#define MAX_CLASSNAME 255
#define MAX_WNDNAME 255

using namespace std;
namespace Discord
{
	namespace Communication
	{
		typedef struct _Header
		{
			UINT Magic;
			UINT FrameCount;
			UINT NoClue;
			UINT Width;
			UINT Height;
			BYTE Buffer[1];
		} Header;

		typedef struct _ConnectedProcessInfo
		{
			UINT ProcessId;
			HANDLE File;
			Header* MappedAddress;
		} ConnectedProcessInfo;

		inline bool ConnectToProcess(ConnectedProcessInfo& processInfo)
		{
			std::string mappedFilename = "DiscordOverlay_Framebuffer_Memory_" + std::to_string(processInfo.ProcessId);
			processInfo.File = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, mappedFilename.c_str());
			if (!processInfo.File || processInfo.File == INVALID_HANDLE_VALUE)
				return false;

			processInfo.MappedAddress = static_cast<Header*>(MapViewOfFile(processInfo.File, FILE_MAP_ALL_ACCESS, 0, 0, 0));
			return processInfo.MappedAddress;
		}

		inline void DisconnectFromProcess(ConnectedProcessInfo& processInfo)
		{
			UnmapViewOfFile(processInfo.MappedAddress);
			processInfo.MappedAddress = nullptr;

			CloseHandle(processInfo.File);
			processInfo.File = nullptr;
		}

		inline void SendFrame(ConnectedProcessInfo& processInfo, UINT width, UINT height, void* frame, UINT size)
		{
			// frame is in B8G8R8A8 format
			// size can be nearly anything since it will get resized
			// for the screen appropriately, although the maximum size is
			// game window width * height * 4 (BGRA)
			processInfo.MappedAddress->Width = width;
			processInfo.MappedAddress->Height = height;

			memcpy(processInfo.MappedAddress->Buffer, frame, size);

			processInfo.MappedAddress->FrameCount++; // this will cause the internal module to copy over the framebuffer
		}
	}

	namespace Drawing
	{
		// 定义结构体
		typedef struct _Frame
		{
			UINT Width;
			UINT Height;
			UINT Size;
			void* Buffer;

			// 重载==运算符
			bool operator==(_Frame& other) {
				return(Width == other.Width && Height == other.Height && Size == other.Size && Buffer == other.Buffer); // 注意：这里仅比较指针地址
			}

		} Frame;

		typedef struct _StrList
		{
			char16_t* text;
			SkColor color;

		}StrList;

		typedef struct _StrList2
		{
			wstring text;
			SkColor color;

		}StrList2;

		// 定义静态变量
		static Frame _currentFrame;				// 当前绘制帧
		static SkCanvas* _canvas = nullptr;		// Skia 画布
		static SkSurface* _surface = nullptr;	// Skia 绘图表面
		static SkPaint paint;					// 画笔
		static SkFont font;						// 字体
		static SkPaint shadowPaint;
		static SkPaint textPaint;

		// 创建内存画布
		inline Frame CreateFrame(UINT width, UINT height)
		{
			// HarmonyOS_Sans_SC_Regular.ttf
			//font.setTypeface(SkTypeface::MakeFromName("HarmonyOS Sans SC Regular", SkFontStyle::Bold()));  //设置字体
			font.setTypeface(SkTypeface::MakeFromFile(skCrypt("C:\\Windows\\Fonts\\HarmonyOS_Sans_SC_Regular.ttf")));
			font.setEmbolden(true);
			// 获得字体宽高
			SkRect rect2;  font.measureText(L"T", wcslen(L"T") * 2, SkTextEncoding::kUTF16, &rect2);
			paint.setAntiAlias(true);		//开启抗锯齿

			

			//创建画布
			Frame output;
			output.Width = width;
			output.Height = height;
			output.Size = width * height * 4;

			output.Buffer = malloc(output.Size);
			memset(output.Buffer, 0, output.Size);

			return output;
		}

		// 清理内存画布
		inline void CleanFrame(Frame& frame)
		{
			memset(frame.Buffer, 0, frame.Size);
		}

		// 初始化交换
		inline void EnsureSkiaInitialized(Frame& frame)
		{
			if (_currentFrame == frame) return;
			// 释放之前资源
			delete _canvas;
			delete _surface;
			// 创建 Skia 绘图表面和画布
			SkImageInfo info = SkImageInfo::Make(
				frame.Width,
				frame.Height,
				kBGRA_8888_SkColorType,
				kPremul_SkAlphaType
			);
			// 将像素数据指定为绘图表面的像素缓冲区
			_surface = SkSurface::MakeRasterDirect(info, frame.Buffer, frame.Width * 4).release();
			_canvas = _surface->getCanvas();
			_currentFrame = frame;
		}

		// 绘制背景色
		inline static void DrawBackground(Frame frame, SkColor color)
		{
			EnsureSkiaInitialized(frame);	//初始化画布
			// 在 Skia 画布上绘制背景
			_canvas->clear(color);
		}

		// 绘制线 | 起点XY，终点XY，粗细，颜色
		inline static void DrawLine(Frame frame, int x1, int y1, int x2, int y2, int size, SkColor color)
		{
			EnsureSkiaInitialized(frame);	//初始化画布
			paint.setColor(color);			//设置画笔颜色
			paint.setStrokeWidth(size);		//设置画笔粗细
			_canvas->drawLine(x1, y1, x2, y2, paint);
		}

		// 绘制框 | SK矩形，粗细，颜色，填充
		inline static void DrawRectangle(Frame frame, SkRect rect, int size, SkColor color, bool fill)
		{
			EnsureSkiaInitialized(frame);	//初始化画布
			paint.setColor(color);			//设置画笔颜色
			paint.setStrokeWidth(size);		//设置画笔粗细
			paint.setStyle(fill ? SkPaint::kFill_Style : SkPaint::kStroke_Style);	//设置画笔填充
			_canvas->drawRect(rect, paint);
		}

		// 绘制文本 | 位置XY，颜色，文本
		/*
		inline static void DrawStr(Frame frame, int x1, int y1, int size, SkColor color, wstring text)
		{
			EnsureSkiaInitialized(frame);	//初始化画布
			SkPaint paint;
			font.setSize(size);				//设置字体大小
			// 获得字体宽高
			SkRect rect; font.measureText(text.data(), wcslen(text.data()) * 2, SkTextEncoding::kUTF16, &rect);
			// 在 Skia 画布上绘制文本
			paint.setColor(SK_ColorBLACK);
			_canvas->drawSimpleText(text.data(), wcslen(text.data()) * 2, SkTextEncoding::kUTF16, x1 - rect.centerX(), y1, font, paint);
			paint.setColor(color);			//设置画笔颜色
			_canvas->drawSimpleText(text.data(), wcslen(text.data()) * 2, SkTextEncoding::kUTF16, x1 - rect.centerX(), y1, font, paint);
			
		}
		*/
		/*
		void DrawStr(Frame frame, int x, int y, int size, SkColor textColor, const wchar_t* text) {
			EnsureSkiaInitialized(frame);	//初始化画布

			SkPaint paint;
			font.setSize(size);				//设置字体大小

			// 设置阴影画笔
			//shadowPaint.setColor(SK_ColorBLACK);
			//shadowPaint.setAntiAlias(true);

			// 设置文本画笔
			textPaint.setColor(textColor);
			textPaint.setAntiAlias(true);

			// 获取文本的宽度和高度
			SkRect textBounds;
			font.measureText(text, wcslen(text) * 2, SkTextEncoding::kUTF16, &textBounds);

			// 绘制阴影文本
			//_canvas->drawSimpleText(text, wcslen(text) * 2, SkTextEncoding::kUTF16, x - textBounds.centerX() + 2, y + 2, font, shadowPaint);

			// 绘制正常文本
			_canvas->drawSimpleText(text, wcslen(text) * 2, SkTextEncoding::kUTF16, x - textBounds.centerX(), y, font, textPaint);
		}
		*/
		void DrawStr2(Frame frame, int x, int y, int size, SkColor textColor, const char16_t* text) {
			EnsureSkiaInitialized(frame);	//初始化画布

			SkPaint paint;
			font.setSize(size);				//设置字体大小

			// 设置阴影画笔
			//shadowPaint.setColor(SK_ColorBLACK);
			//shadowPaint.setAntiAlias(true);

			// 设置文本画笔
			textPaint.setColor(textColor);
			textPaint.setAntiAlias(true);

			// 获取文本的宽度和高度
			SkRect textBounds;
			font.measureText(text, std::char_traits<char16_t>::length(text) * 2, SkTextEncoding::kUTF16, &textBounds);
			// 绘制阴影文本
			//_canvas->drawSimpleText(text, std::char_traits<char16_t>::length(text) * 2, SkTextEncoding::kUTF16, x - textBounds.centerX() + 2, y + 2, font, shadowPaint);

			// 绘制正常文本
			_canvas->drawSimpleText(text, std::char_traits<char16_t>::length(text) * 2, SkTextEncoding::kUTF16, x - textBounds.centerX(), y, font, textPaint);
		}

		// 绘制圆
		inline static void DrawCircle(Frame frame, int x, int y, int size, int radius, SkColor color, bool fill)
		{
			EnsureSkiaInitialized(frame);	//初始化画布
			paint.setColor(color);			//设置画笔颜色
			paint.setStrokeWidth(size);		//设置画笔粗细
			paint.setStyle(fill ? SkPaint::kFill_Style : SkPaint::kStroke_Style);	//设置画笔填充
			_canvas->drawCircle(x, y, radius, paint);
		}

		// 绘制血条
		inline static void DrawSHBar(Frame frame, int x, int y, int height, int shield, int max_shield, int health, bool shield_bar, bool health_bar) {
			int pos_offset = 0;										// 位置偏离
			SkColor shield_color = SK_ColorWHITE;					// 护甲颜色
			int barline = height/5 + 1;								// 线段间距
			int shieldbar_height = height;							// 护甲高度
			if (shield_bar && health_bar )  pos_offset = -9;
			// 护甲
			if (shield_bar) {
				DrawRectangle(frame, SkRect::MakeXYWH(x + pos_offset, y, 8, height), 1, SK_ColorGRAY, true);
				DrawRectangle(frame, SkRect::MakeXYWH(x + pos_offset, y, 8, height), 1, SK_ColorBLACK, false);					//背景
				switch (max_shield)
				{
				case 50://white
					shield_color = SK_ColorWHITE;
					shieldbar_height -= barline * 3;
					break;
				case 75://blue
					shield_color = SkColorSetRGB(39, 178, 255);
					shieldbar_height -= barline * 2;
					break;
				case 100://purple gold
					shield_color = SkColorSetRGB(206, 59, 255);
					shieldbar_height -= barline * 1;
					break;
				case 125://red
					shield_color = SK_ColorRED;
					break;
				default:
					shield_color = SK_ColorWHITE;
					break;
				}
				DrawLine(frame, x + 4 + pos_offset, y + (height - int((static_cast<double>(shieldbar_height) / max_shield) * shield)), x + 4 + pos_offset, y + height, 6, shield_color);
				DrawLine(frame, x + pos_offset, y + barline, x + 8 + pos_offset, y + barline, 3, SK_ColorDKGRAY);
				DrawLine(frame, x + pos_offset, y + barline * 2, x + 8 + pos_offset, y + barline * 2, 2, SK_ColorDKGRAY);
				DrawLine(frame, x + pos_offset, y + barline * 3, x + 8 + pos_offset, y + barline * 3, 2, SK_ColorDKGRAY);
				DrawLine(frame, x + pos_offset, y + barline * 4, x + 8 + pos_offset, y + barline * 4, 2, SK_ColorDKGRAY);
			}
			// 血量
			if (health_bar) {

				SkColor health_color = SK_ColorWHITE;
				if (health <= 15) { health_color = SkColorSetRGB(255, 69, 0); }
				else if (health <= 30) { health_color = SkColorSetRGB(250, 128, 114); }
				else if (health <= 50) { health_color = SkColorSetRGB(255, 160, 122); }
				else if (health <= 70) { health_color = SkColorSetRGB(255, 218, 185); }

				DrawRectangle(frame, SkRect::MakeXYWH(x, y, 6, height), 1, SK_ColorGRAY, true);
				DrawRectangle(frame, SkRect::MakeXYWH(x, y, 6, height), 1, SK_ColorBLACK, false);	//背景
				DrawLine(frame, x + 2, y + (height - int((static_cast<double>(height) / 100) * health)), x + 2, y + height, 4, health_color);
			}
		}

		// 绘制文本序列
		inline void DrawStrList(Frame frame, int x, int y, int size, std::vector<StrList> strlist) {
			EnsureSkiaInitialized(frame);	//初始化画布

			SkPaint paint;
			font.setSize(size);				//设置字体大小

			// 设置阴影画笔
			//shadowPaint.setColor(SK_ColorBLACK);
			//shadowPaint.setAntiAlias(true);

			// 设置文本画笔
			textPaint.setAntiAlias(true);

			// 获取文本的宽度和高度
			SkRect textBounds;

			for (const auto& str : strlist)
			{
				const size_t len = std::char_traits<char16_t>::length(str.text) * 2;
				font.measureText(str.text, len, SkTextEncoding::kUTF16, &textBounds);

				// 在 Skia 画布上绘制文本
				y += textBounds.height();
				// 绘制阴影文本
				//_canvas->drawSimpleText(str.text.data(), len, SkTextEncoding::kUTF16, x + 2, y + 2 , font, shadowPaint);
				textPaint.setColor(str.color);		//设置画笔颜色
				// 绘制正常文本
				_canvas->drawSimpleText(str.text, len, SkTextEncoding::kUTF16, x, y , font, textPaint);
			}
		}
		inline void DrawStrList2(Frame frame, int x, int y, int size, std::vector<StrList2> strlist) {
			EnsureSkiaInitialized(frame);	//初始化画布

			SkPaint paint;
			font.setSize(size);				//设置字体大小

			// 设置阴影画笔
			//shadowPaint.setColor(SK_ColorBLACK);
			//shadowPaint.setAntiAlias(true);

			// 设置文本画笔
			textPaint.setAntiAlias(true);

			// 获取文本的宽度和高度
			SkRect textBounds;

			for (const auto& str : strlist)
			{
				const size_t len = wcslen(str.text.data()) * 2;
				font.measureText(str.text.data(), len, SkTextEncoding::kUTF16, &textBounds);

				// 在 Skia 画布上绘制文本
				y += textBounds.height();
				// 绘制阴影文本
				//_canvas->drawSimpleText(str.text.data(), len, SkTextEncoding::kUTF16, x + 2, y + 2 , font, shadowPaint);
				textPaint.setColor(str.color);		//设置画笔颜色
				// 绘制正常文本
				_canvas->drawSimpleText(str.text.data(), len, SkTextEncoding::kUTF16, x, y, font, textPaint);
			}
		}

		// 绘制横向血条
		SkColor shield_color = SK_ColorWHITE;					// 护甲颜色
		inline static  void DrawLtsBar(Frame frame, int x, int y, int height, int shield, int max_shield, int health, const wchar_t* dist) {
			EnsureSkiaInitialized(frame);	//初始化画布
			paint.setStrokeWidth(1);		//设置画笔粗细
			paint.setStyle(SkPaint::kFill_Style);	//设置画笔填充


			switch (max_shield)
			{
			case 50://white
				shield_color = SK_ColorWHITE;
				break;
			case 75://blue
				shield_color = SkColorSetRGB(39, 178, 255);
				break;
			case 100://purple gold
				shield_color = SkColorSetRGB(206, 59, 255);
				break;
			case 125://red
				shield_color = SK_ColorRED;
				break;
			default:
				shield_color = SK_ColorWHITE;
				break;
			}

			paint.setColor(SK_ColorBLACK);
			_canvas->drawRect(SkRect::MakeXYWH(x, y, height, 5), paint);
			_canvas->drawRect(SkRect::MakeXYWH(x, y + 5, height, 5), paint);
			paint.setColor(shield_color);
			_canvas->drawRect(SkRect::MakeXYWH(x, y, int((static_cast<double>(height) / max_shield) * shield), 5), paint);
			paint.setColor(SkColorSetRGB(50, 205, 50));
			_canvas->drawRect(SkRect::MakeXYWH(x, y + 5, int((static_cast<double>(height) / 100) * health), 5), paint);
			paint.setColor(SK_ColorWHITE);

			// 获取文本的宽度和高度
			font.setSize(14);
			textPaint.setColor(SK_ColorWHITE);
			//textPaint.setColor(SkColorSetRGB(255, 165, 0));
			// 绘制阴影文本
			_canvas->drawSimpleText(dist, wcslen(dist) * 2, SkTextEncoding::kUTF16, x, y + 11, font, shadowPaint);
			_canvas->drawSimpleText(dist, wcslen(dist) * 2, SkTextEncoding::kUTF16, x, y + 10, font, textPaint);

		}
	}
}

// Prototypes
namespace Hjack {

	struct WindowsFinderParams {
		DWORD pidOwner = NULL;
		string wndClassName = "";
		string wndName = "";
		RECT pos = { 0, 0, 0, 0 };
		POINT res = { 0, 0 };
		float percentAllScreens = 0.0f;
		float percentMainScreen = 0.0f;
		DWORD style = NULL;
		DWORD styleEx = NULL;
		bool satisfyAllCriteria = false;
		vector<HWND> hwnds;
	};

	inline vector<DWORD> GetPIDs(wstring targetProcessName) {
		vector<DWORD> pids;
		if (targetProcessName == L"")
			return pids;
		HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32W entry;
		entry.dwSize = sizeof entry;
		if (!Process32FirstW(snap, &entry)) {
			CloseHandle(snap);
			return pids;
		}
		do {
			if (wstring(entry.szExeFile) == targetProcessName) {
				pids.emplace_back(entry.th32ProcessID);
			}
		} while (Process32NextW(snap, &entry));
		CloseHandle(snap);
		return pids;
	}
	inline BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
		WindowsFinderParams& params = *(WindowsFinderParams*)lParam;

		unsigned char satisfiedCriteria = 0, unSatisfiedCriteria = 0;

		// If looking for windows of a specific PDI
		DWORD pid = 0;
		GetWindowThreadProcessId(hwnd, &pid);
		if (params.pidOwner != NULL)
			if (params.pidOwner == pid)
				++satisfiedCriteria; // Doesn't belong to the process targeted
			else
				++unSatisfiedCriteria;

		// If looking for windows of a specific class
		char className[MAX_CLASSNAME] = "";
		GetClassNameA(hwnd, (LPSTR)className, MAX_CLASSNAME);
		string classNameWstr = className;
		if (params.wndClassName != "")
			if (params.wndClassName == classNameWstr)
				++satisfiedCriteria; // Not the class targeted
			else
				++unSatisfiedCriteria;

		// If looking for windows with a specific name
		char windowName[MAX_WNDNAME] = "";
		GetWindowTextA(hwnd, (LPSTR)windowName, MAX_CLASSNAME);
		string windowNameWstr = windowName;
		if (params.wndName != "")
			if (params.wndName == windowNameWstr)
				++satisfiedCriteria; // Not the class targeted
			else
				++unSatisfiedCriteria;

		// If looking for window at a specific position
		RECT pos;
		GetWindowRect(hwnd, &pos);
		if (params.pos.left || params.pos.top || params.pos.right || params.pos.bottom)
			if (params.pos.left == pos.left && params.pos.top == pos.top && params.pos.right == pos.right && params.pos.bottom == pos.bottom)
				++satisfiedCriteria;
			else
				++unSatisfiedCriteria;

		// If looking for window of a specific size
		POINT res = { pos.right - pos.left, pos.bottom - pos.top };
		if (params.res.x || params.res.y)
			if (res.x == params.res.x && res.y == params.res.y)
				++satisfiedCriteria;
			else
				++unSatisfiedCriteria;

		// If looking for windows taking more than a specific percentage of all the screens
		float ratioAllScreensX = res.x / GetSystemMetrics(SM_CXSCREEN);
		float ratioAllScreensY = res.y / GetSystemMetrics(SM_CYSCREEN);
		float percentAllScreens = ratioAllScreensX * ratioAllScreensY * 100;
		if (params.percentAllScreens != 0.0f)
			if (percentAllScreens >= params.percentAllScreens)
				++satisfiedCriteria;
			else
				++unSatisfiedCriteria;

		// If looking for windows taking more than a specific percentage or the main screen
		RECT desktopRect;
		GetWindowRect(GetDesktopWindow(), &desktopRect);
		POINT desktopRes = { desktopRect.right - desktopRect.left, desktopRect.bottom - desktopRect.top };
		float ratioMainScreenX = res.x / desktopRes.x;
		float ratioMainScreenY = res.y / desktopRes.y;
		float percentMainScreen = ratioMainScreenX * ratioMainScreenY * 100;
		if (params.percentMainScreen != 0.0f)
			if (percentAllScreens >= params.percentMainScreen)
				++satisfiedCriteria;
			else
				++unSatisfiedCriteria;

		// Looking for windows with specific styles
		LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (params.style)
			if (params.style & style)
				++satisfiedCriteria;
			else
				++unSatisfiedCriteria;

		// Looking for windows with specific extended styles
		LONG_PTR styleEx = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
		if (params.styleEx)
			if (params.styleEx & styleEx)
				++satisfiedCriteria;
			else
				++unSatisfiedCriteria;

		if (!satisfiedCriteria)
			return TRUE;

		if (params.satisfyAllCriteria && unSatisfiedCriteria)
			return TRUE;

		// If looking for multiple windows
		params.hwnds.push_back(hwnd);
		return TRUE;
	}
	inline vector<HWND> WindowsFinder(WindowsFinderParams params) {
		EnumWindows(EnumWindowsCallback, (LPARAM)&params);
		return params.hwnds;
	}

	inline string static random_string(const int len)
	{
		const string alpha_numeric(skCrypt("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*"));

		default_random_engine generator{ random_device{}() };
		auto distribution = uniform_int_distribution< string::size_type >{ 0, alpha_numeric.size() - 1 };

		string str(len, 0);
		for (auto& it : str) {
			it = alpha_numeric[distribution(generator)];
		}
		return str;
	}
	inline wstring GetWStringByChar(const char* szString)
	{
		std::wstring wstrString;
		if (szString != NULL)
		{
			std::string str(szString);
			wstrString.assign(str.begin(), str.end());
		}

		return wstrString;
	}
}

#endif
