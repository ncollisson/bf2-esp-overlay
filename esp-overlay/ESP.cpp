#include "stdafx.h"
#include "ESP.h"
#include <tlhelp32.h>
#include "d2d1.h"
#include "d3dx9math.h"
#include "GameInfo.h"
#include <d3dx9.h>
#include <cmath>

#pragma comment(lib, "d3dx9.lib")

ESP::ESP(HWND hWnd)
	: hwnd(hWnd)
{
}

ESP::~ESP()
{
}

int ESP::Init()
{
	InitD2D();
	InitIPC();
	InjectDLL();

	return 0;
}

int ESP::InitD2D()
{
	ID2D1Factory* pD2DFactory = NULL;
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);

	RECT rect;
	GetClientRect(hwnd, &rect);

	xCenter = (rect.right + rect.left) / 2.0f;
	yCenter = (rect.top + rect.bottom) / 2.0f;

	xSize = rect.right - rect.left;
	ySize = rect.bottom - rect.top;

	pD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM,
				D2D1_ALPHA_MODE_PREMULTIPLIED)),
		D2D1::HwndRenderTargetProperties(
			hwnd,
			D2D1::SizeU(
				rect.right - rect.left,
				rect.bottom - rect.top)
		),
		&pRT
	);

	pRT->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Red),
		&pRedBrush
	);

	pRT->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Orange),
		&pOrangeBrush
	);

	pRT->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Yellow),
		&pYellowBrush
	);

	pRT->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black),
		&pBlackBrush
	);

	pRT->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::GhostWhite),
		&pCrosshairBrush
	);

	pRT->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Goldenrod),
		&pLandBrush
	);

	pRT->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::DarkTurquoise),
		&pSeaBrush
	);

	pRT->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::DeepPink),
		&pAirBrush
	);

	pRT->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Red),
		&pHeliBrush
	);

	pRT->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::WhiteSmoke),
		&pOtherBrush
	);

	pRT->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.7f)),
		&pRadarBackgroundBrush
	);

	return 0;
}

int ESP::InitIPC()
{
	HANDLE hMapObject = CreateFileMapping(
		INVALID_HANDLE_VALUE,   // use paging file
		NULL,                   // default security attributes
		PAGE_READWRITE,         // read/write access
		0,                      // size: high 32-bits
		sizeof(GameInfo),       // size: low 32-bits
		TEXT("bf2-game-info")); // name of map object

	gameInfo = (GameInfo*)MapViewOfFile(
		hMapObject,				// object to map view of
		FILE_MAP_ALL_ACCESS,	// read/write access
		0,						// high offset:  map from
		0,						// low offset:   beginning
		0);						// default: map entire file

	memset(gameInfo, '\0', sizeof(GameInfo));

	return 0;
}

int ESP::InjectDLL()
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	DWORD bf2_pid = 0;
	TCHAR bf2exe[] = _T("BF2.exe");

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (_tcscmp(entry.szExeFile, bf2exe) == 0)
			{
				bf2_pid = entry.th32ProcessID;
				break;
			}
		}
	}

	HANDLE bf2_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, bf2_pid);

	HMODULE k32_handle = GetModuleHandle(L"kernel32.dll");

	char dll_name[] = "C:/Users/nick/Documents/Visual Studio 2015/Projects/bf2-game-info/Debug/bf2-game-info.dll";
	LPVOID remote_ptr2_dll_name = VirtualAllocEx(bf2_handle, NULL, sizeof(dll_name), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	WriteProcessMemory(bf2_handle, remote_ptr2_dll_name, &dll_name, sizeof(dll_name), NULL);

	FARPROC ptr2_Load_Lib_A = GetProcAddress(k32_handle, "LoadLibraryA");

	HANDLE dll_thread_handle = CreateRemoteThread(bf2_handle, NULL, 0, (LPTHREAD_START_ROUTINE)ptr2_Load_Lib_A, remote_ptr2_dll_name, 0, NULL);

	CloseHandle(bf2_handle);

	return 0;
}

int ESP::ConvertRelativeToMe(const D3DXVECTOR3& otherGameVector, D3DXVECTOR3& relativeVector)
{
	D3DXVECTOR3 myGameVector = currentGameInfo.playerInfo[0].position;

	relativeVector = otherGameVector - myGameVector;

	return 0;
}

int ESP::ConvertGameToRadar(const D3DXVECTOR2& gameVector, D3DXVECTOR2& radarVector)
{
	radarVector = gameVector * (radarSize / viewRange);

	radarVector.y *= -1;

	radarVector.x += xRadarCenter;
	radarVector.y += yRadarCenter;

	return 0;
}

int ESP::RotateRelativeToMe(const D3DXVECTOR2& originalVector, D3DXVECTOR2& rotatedVector)
{
	// need to get right camera vector
	D3DXVECTOR3 cameraVector = currentGameInfo.myCamera;

	float theta = std::atan2(0, 1) - std::atan2(cameraVector.z, cameraVector.x);

	float xOriginal = originalVector.x;
	float yOriginal = originalVector.y;

	float xRotated = xOriginal * std::cos(theta) - yOriginal * std::sin(theta);
	float yRotated = xOriginal * std::sin(theta) + yOriginal * std::cos(theta);

	rotatedVector.x = xRotated;
	rotatedVector.y = yRotated;

	return 0;
}

int ESP::Render()
{
	pRT->BeginDraw();

	pRT->Clear(D2D1::ColorF(0, 0.0f));

	if (gameInfo == nullptr)
	{
		pRT->EndDraw();
		return 0;
	}

	currentGameInfo = *gameInfo; // performance?
	PlayerInfo myPlayer = currentGameInfo.playerInfo[0];

	D3DXVECTOR3 xhair_coords, relative_velocity;
	D3DXVECTOR3 *my_coords, *target_coords, predicted_coords,
		missile_velocity, *target_velocity,
		*camera_right, *camera_up, *camera_ahead,
		target_movement, target_vector, n_target_vector, lag_compensation;
	D3DXMATRIX* my_camera;
	const float missile_speed = 125;
	float target_distance, relative_speed, time_to_impact, amount_to_lead, target_speed;

	for (unsigned int i = 1; i < currentGameInfo.numberOfPlayers; i++)
	{
		my_camera = &(currentGameInfo.myCamera);
		camera_right = (D3DXVECTOR3 *)&(my_camera->_11);
		camera_up = (D3DXVECTOR3 *)&(my_camera->_21);
		camera_ahead = (D3DXVECTOR3 *)&(my_camera->_31);
		my_coords = (D3DXVECTOR3 *)&(my_camera->_41);
		// can get head tilt angle from camera_right angle from up up

		PlayerInfo player = currentGameInfo.playerInfo[i];

		if (myPlayer.team == player.team) continue;
		if (!player.isAlive || player.isManDown) continue;

		target_coords = &(player.position);

		target_vector = *target_coords - *my_coords;

		float target_distance = D3DXVec3Length(&target_vector);
		if (target_distance > viewRange) continue;

		D3DXVec3Normalize(&n_target_vector, &target_vector);

		// why * 600?
		xhair_coords.x = D3DXVec3Dot(camera_right, &n_target_vector) * 600;
		xhair_coords.y = D3DXVec3Dot(camera_up, &n_target_vector) * -600;
		xhair_coords.z = D3DXVec3Dot(camera_ahead, &n_target_vector);
		if (xhair_coords.z < 0) continue;

		// get zoom value from camera + 0x58
		// check if zoom value is less than 1, take from 0x5c? not really necessary
		// the screen size values may be 1180 and 660?k
		float myZoom = currentGameInfo.myZoom;

		xhair_coords.x = xCenter * (1 - (xhair_coords.x / myZoom) / -(1196 / 2 + 0) / xhair_coords.z);
		xhair_coords.y = yCenter * (1 - (xhair_coords.y / myZoom) / -(698 / 2 + 0) / xhair_coords.z);
		
		xhair_coords.y += 10;

		D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(xhair_coords.x, xhair_coords.y), 2.0f, 2.0f);

		switch (player.vehicleClass)
		{
		case 0:
			pRT->DrawEllipse(ellipse, pRedBrush, 0.8f);
			break;
		case 1:
			pRT->DrawEllipse(ellipse, pSeaBrush, 0.8f);
			break;
		case 2:
			ellipse = D2D1::Ellipse(D2D1::Point2F(xhair_coords.x, xhair_coords.y), 4.0f, 4.0f);
			pRT->DrawEllipse(ellipse, pAirBrush, 0.8f);
			//pRT->FillEllipse(ellipse, pAirBrush);
			break;
		case 3:
			ellipse = D2D1::Ellipse(D2D1::Point2F(xhair_coords.x, xhair_coords.y), 6.0f, 6.0f);
			pRT->DrawEllipse(ellipse, pHeliBrush, 0.8f);
			//pRT->FillEllipse(ellipse, pHeliBrush);
			break;
		default:
			pRT->DrawEllipse(ellipse, pOtherBrush, 0.6f);
		}
	}

	D2D1_ELLIPSE radarBackgound = D2D1::Ellipse(D2D1::Point2F(xRadarCenter, yRadarCenter), radarSize, radarSize);
	pRT->FillEllipse(radarBackgound, pRadarBackgroundBrush);

	pRT->DrawLine(D2D1::Point2F(xRadarCenter, yRadarCenter + 20), D2D1::Point2F(xRadarCenter, yRadarCenter - 20), pCrosshairBrush, 0.5f);
	pRT->DrawLine(D2D1::Point2F(xRadarCenter + 20, yRadarCenter), D2D1::Point2F(xRadarCenter - 20, yRadarCenter), pCrosshairBrush, 0.5f);

	for (unsigned int i = 1; i < currentGameInfo.numberOfPlayers; i++)
	{
		PlayerInfo player = currentGameInfo.playerInfo[i];

		if (myPlayer.team == player.team) continue;
		if (!player.isAlive || player.isManDown) continue;

		D3DXVECTOR3 relativePosition;
		ConvertRelativeToMe(player.position, relativePosition);

		float distance = D3DXVec3Length(&relativePosition);
		if (distance > viewRange) continue;

		D3DXVECTOR2 relativePosition2D, rotatedPosition;
		relativePosition2D.x = relativePosition.x;
		relativePosition2D.y = relativePosition.z;

		RotateRelativeToMe(relativePosition2D, rotatedPosition);

		D3DXVECTOR2 radarPosition;
		ConvertGameToRadar(rotatedPosition, radarPosition);

		D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(radarPosition.x, radarPosition.y), 2.0f, 2.0f);

		switch (player.vehicleClass)
		{
		case 0:
			pRT->DrawEllipse(ellipse, pLandBrush, 0.4f);
			break;
		case 1:
			pRT->DrawEllipse(ellipse, pSeaBrush, 0.6f);
			break;
		case 2:
			pRT->DrawEllipse(ellipse, pAirBrush, 0.8f);
			pRT->FillEllipse(ellipse, pAirBrush);
			break;
		case 3:
			pRT->DrawEllipse(ellipse, pHeliBrush, 0.8f);
			pRT->FillEllipse(ellipse, pHeliBrush);
			break;
		default:
			pRT->DrawEllipse(ellipse, pOtherBrush, 0.6f);
		}
	}

	pRT->EndDraw();

	return 0;
}
