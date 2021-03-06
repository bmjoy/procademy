#pragma once
#include <windows.h>
#include <windowsx.h>
#include <queue>

#define dfMAXCHILD		100
#define MAXDATA 100

class GraphWindow
{
public:
	static const LPCWSTR className;

	enum TYPE
	{
		LINE_SINGLE,
		LINE_MULTI,
		NUMBER,
		ONOFF,
		PIE
	};
	

	typedef struct ST_COLUMN_INFO
	{
		ULONG64 u64ServerID;
		int iType;
		WCHAR szName[64];

		// 여기에 Queue 를 추가하여도 됨.

	} stColumnInfo;

	typedef struct ST_HWNDtoTHIS
	{
		HWND			hWnd[dfMAXCHILD];
		GraphWindow	*pThis[dfMAXCHILD];

	} stHWNDtoTHIS;

	GraphWindow(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int posX, int posY, int iWidth, int iHeight);
	~GraphWindow();

	

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	BOOL	InsertData(ULONG64 u64ServerID, int iType, int iData);
	BOOL SetDataColumnInfo(int iColIndex, ULONG64 u64ServerID, int iType, LPCWSTR szName);

protected:

	//------------------------------------------------------
	// 윈도우 핸들, this 포인터 매칭 테이블 관리.
	//------------------------------------------------------
	BOOL				PutThis(void);
	static GraphWindow	*GetThis(HWND hWnd);

private:
	static bool flag;

	//------------------------------------------------------
	// 부모 윈도우 핸들, 내 윈도우 핸들, 인스턴스 핸들
	//------------------------------------------------------
	HWND _hWndParent;
	HWND _hWnd;
	HINSTANCE _hInstance;
	//------------------------------------------------------
	// 윈도우 위치,크기,색상, 그래프 타입 등.. 자료
	//------------------------------------------------------
	TYPE		_enGraphType;
	int _iWindowPosX;
	int _iWindowPosY;
	int _iWindowPosWidth;
	int _iWindowPosHeight;

	//------------------------------------------------------
	// 더블 버퍼링용 메모리 DC, 메모리 비트맵
	//------------------------------------------------------
	HDC			_hMemDC;
	HBITMAP		_hBitmap;

	//------------------------------------------------------
	// 데이터
	//------------------------------------------------------
	stColumnInfo _dataColumn;
	std::queue<int> *DataQueue;

	BOOL paint(HWND hWnd);

		// static 맴버 함수의 프로시저에서 This 포인터를 찾기 위한
		// HWND + Class Ptr 의 테이블
};