#include <algorithm>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <codecvt>
#include <string>
#include <vector>
#include <chrono>


struct screenInfo
{
	int nWidth = 120;
	int nHeight = 40;
} screendata;

struct mapInfo
{
	int nWidth = 16;
	int nHeight = 16;
	std::wstring wslayout;
} map;

struct playerInfo
{
	long double fX = 14.7f;
	long double fY = 5.09f;
	long double fTheta = 0.0f;
	long double fFOV = 3.14159f / 4.0f;
	long double fDepth = 16.0f;
	long double fSpeed = 5.0f;
} player;

wchar_t* screen = new wchar_t[screendata.nWidth * screendata.nHeight];

// Takes in a file name
std::wstring generateMap(std::string sFileName)
{
	std::ifstream inputfile(sFileName);
	std::string line;
	std::wstring wslayout;

	if (inputfile.is_open())
	{
		while (std::getline(inputfile, line))
		{
			wslayout += std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(line);
		}
	}
	return wslayout;
}

void ldFindDistance(int x)
{
	long double ldRayAngle = (player.fTheta - player.fFOV / 2.0f) + ((long double)x / (long double)screendata.nWidth) * player.fFOV;
	long double ldResolution = 0.05f;		  										
	long double ldDistance = 0.0f; 
	long double ldEyeH = sinf(ldRayAngle); 
	long double ldEyeV = cosf(ldRayAngle);
	bool bHit = false;
	bool bInBoundary = false;
	while (!bHit && ldDistance < player.fDepth)
	{
		ldDistance += ldResolution;
		int nRayX = (int)(player.fX + ldEyeH * ldDistance);
		int nRayY = (int)(player.fY + ldEyeV * ldDistance);


		if (nRayX < 0 || nRayX >= map.nWidth || nRayY < 0 || nRayY >= map.nHeight)
		{
			bHit = true;			
			ldDistance = player.fDepth;
		}
		else
		{

			if (map.wslayout.c_str()[nRayX * map.nWidth + nRayY] == '#')
			{
				bHit = true;
				std::vector<std::pair<long double, long double>> p;

				for (int tx = 0; tx < 2; tx++)
					for (int ty = 0; ty < 2; ty++)
					{

						long double vy = (long double)nRayY + ty - player.fY;
						long double vx = (long double)nRayX + tx - player.fX;
						long double d = sqrt(vx * vx + vy * vy);
						long double dot = (ldEyeH * vx / d) + (ldEyeV * vy / d);
						p.push_back(std::make_pair(d, dot));
					}
				sort(p.begin(), p.end(), [](const std::pair<long double, long double>& left, const std::pair<long double, long double>& right) {return left.first < right.first; });


				long double fBound = 0.01;
				if (acos(p.at(0).second) < fBound) 
					bInBoundary = true;
				if (acos(p.at(1).second) < fBound) 
					bInBoundary = true;
				if (acos(p.at(2).second) < fBound) 
					bInBoundary = true;
			}
		}
	}


	int nCeiling = (long double)(screendata.nHeight / 2.0) - screendata.nHeight / ((long double)ldDistance);
	int nFloor = screendata.nHeight - nCeiling;


	short nShade = ' ';
	if (ldDistance <= player.fDepth / 4.0f)	nShade = '#';	
	else if (ldDistance < player.fDepth / 3.0f) nShade = '+';
	else if (ldDistance < player.fDepth / 2.0f) nShade = '=';
	else if (ldDistance < player.fDepth) nShade = '-';
	else nShade = '.';		

	if (bInBoundary)		nShade = ' '; 

	for (int y = 0; y < screendata.nHeight; y++)
	{
		if (y <= nCeiling)
			screen[y * screendata.nWidth + x] = ' ';
		else if (y > nCeiling && y <= nFloor)
			screen[y * screendata.nWidth + x] = nShade;
		else 
		{
			long double b = 1.0f - (((long double)y - screendata.nHeight / 2.0f) / ((long double)screendata.nHeight / 2.0f));
			if (b < 0.25)		nShade = '#';
			else if (b < 0.5)	nShade = '+';
			else if (b < 0.75)	nShade = '=';
			else if (b < 0.9)	nShade = '-';
			else				nShade = ' ';
			screen[y * screendata.nWidth + x] = nShade;
		}
	}


}


void handleRotation(float fElapsedTime)
{
	if (GetAsyncKeyState((unsigned short)'A') & 0x8000) player.fTheta -= (player.fSpeed * 0.75f) * fElapsedTime;
	if (GetAsyncKeyState((unsigned short)'D') & 0x8000) player.fTheta += (player.fSpeed * 0.75f) * fElapsedTime;
}

void handleMovement(float fElapsedTime)
{
	if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
	{
		player.fX += sinf(player.fTheta) * player.fSpeed * fElapsedTime;;
		player.fY += cosf(player.fTheta) * player.fSpeed * fElapsedTime;;
		if (map.wslayout.c_str()[(int)player.fX * map.nWidth + (int)player.fY] == '#')
		{
			player.fX -= sinf(player.fTheta) * player.fSpeed * fElapsedTime;;
			player.fY -= cosf(player.fTheta) * player.fSpeed * fElapsedTime;;
		}
	}

	if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
	{
		player.fX -= sinf(player.fTheta) * player.fSpeed * fElapsedTime;;
		player.fY -= cosf(player.fTheta) * player.fSpeed * fElapsedTime;;
		if (map.wslayout.c_str()[(int)player.fX * map.nWidth + (int)player.fY] == '#')
		{
			player.fX += sinf(player.fTheta) * player.fSpeed * fElapsedTime;;
			player.fY += cosf(player.fTheta) * player.fSpeed * fElapsedTime;;
		}
	}


}


int main()
{
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	SetConsoleTextAttribute(hConsole, 16);
	DWORD dwBytesWritten = 0;
	map.wslayout = generateMap("map.txt");
	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();
	while (1)
	{
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<long double> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();
		handleRotation(fElapsedTime);
		handleMovement(fElapsedTime);
		for (int x = 0; x < screendata.nWidth; x++)
		{
			ldFindDistance(x);
		}
		swprintf_s(screen, 40, L"FPS=%3.2f ",  1.0f / fElapsedTime);
		screen[screendata.nWidth * screendata.nHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, screendata.nWidth * screendata.nHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}
