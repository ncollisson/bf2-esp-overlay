#pragma once
#include "GameInfo.h"
#include "d2d1.h"
#include "d3dx9math.h"

class ESP
{
public:
	ESP(HWND hwnd);
	~ESP();

	int Init();
	int Render();

private:
	int InitD2D();
	int InitIPC();
	int InjectDLL();
	int RotateRelativeToMe(const D3DXVECTOR2& originalVector, D3DXVECTOR2& rotatedVector);
	int ConvertGameToRadar(const D3DXVECTOR2& gameVector, D3DXVECTOR2& radarVector);
	int ConvertRelativeToMe(const D3DXVECTOR3& otherGameVector, D3DXVECTOR3& relativeVector);

	HWND hwnd = NULL;

	ID2D1HwndRenderTarget* pRT = NULL;

	ID2D1SolidColorBrush* pRedBrush = NULL,
		*pOrangeBrush = NULL,
		*pYellowBrush = NULL,
		*pBlackBrush = NULL,
		*pCrosshairBrush = NULL,
		*pLandBrush = NULL,
		*pSeaBrush = NULL,
		*pAirBrush = NULL,
		*pHeliBrush = NULL,
		*pOtherBrush = NULL,
		*pRadarBackgroundBrush = NULL;

	GameInfo* gameInfo = nullptr;
	GameInfo currentGameInfo;

	float xCenter, yCenter;
	float radarSize = 90;
	float xSize, ySize;
	float xRadarCenter = 100;
	float yRadarCenter = 532;
	float viewRange = 1000;
};

