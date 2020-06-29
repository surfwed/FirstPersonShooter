#include<iostream>
#include<Windows.h>
#include<chrono>
#include<vector>
#include<algorithm>
using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;
// to run smoothly we can't use integer to store location 
float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
// angle player is looking at
float fPlayerA = 0.0f;

// field of view 
float fFOV = 3.14159 / 4.0;


// we need a map
int nMapHeight = 16;
int nMapWidth = 16;
float fDepth = 16.0f;

int main()
{
	// Try everything with unicode 16bit
	// Create Screen Buffer
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	// It's just regular text mode buffer
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	// Tell this is our console, target to our console
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#......##......#";
	map += L"#....#.........#";
	map += L"#............###";
	map += L"#..............#";
	map += L"#.........#....#";
	map += L"#.........#....#";
	map += L"#.........#....#";
	map += L"#..#......#....#";
	map += L"#..#......#....#";
	map += L"#.........#....#";
	map += L"#.........#....#";
	map += L"#.........#....#";
	map += L"#..............#";	
	map += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	// As a game engine we need a game loop
	while (1)
	{
		// grab the current system
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
		{
			fPlayerA -= (0.8f * fElapsedTime);
		}
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
		{
			fPlayerA += (0.8f * fElapsedTime);
		}
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}
		// For each column, calculate the projected ray angle into world space 
		for (int x = 0; x < nScreenWidth; x++)
		{
			
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / nScreenWidth) * fFOV;

			// Track what is the distance from the player to the wall for that given angle
			float fDistanceToWall = 0.0f;
			bool bHitWall = false;
			bool bBoundary = false;

			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth )
			{
				fDistanceToWall += 0.1f;
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);				
				if (nTestY < 0 || nTestX < 0 || nTestX >= nMapWidth || nTestY >= nMapHeight)
				{
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else
				{
					if (map[nTestY * nMapWidth + nTestX] == '#')
					{
						bHitWall = true;
						vector<pair<float, float>> p;
						for (int tx = 0; tx < 2; tx++)
						{
							for (int ty = 0; ty < 2; ty++)
							{
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx*vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}
						}
						sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> & right) {return left.first < right.first; });
						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
					}
				}
			}
			int nCeiling = (float)(nScreenHeight / 2.0) - (nScreenHeight / (float) fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;
			short nShade = ' ';
			if (fDistanceToWall <= fDepth / 4.0f) nShade = 0x2588;
			else if (fDistanceToWall <= fDepth / 3.0f) nShade = 0x2593;
			else if (fDistanceToWall <= fDepth / 2.0f) nShade = 0x2592;
			else if (fDistanceToWall <= fDepth) nShade = 0x2591;
			else nShade = ' ';
			if (bBoundary) nShade = ' ';
			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y <= nCeiling)
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else
				{									
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25) nShade = '#';
					else if (b < 0.5) nShade = 'x';
					else if (b < 0.75) nShade = '.';
					else if (b < 0.9) nShade = '-';
					else nShade = ' ';					
					screen[y * nScreenWidth + x] = nShade;
				}
			}
		}
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapHeight; ny++)
			{
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';
		// If we want to write to the screen we use the code here
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	}
	
	return 0;
}