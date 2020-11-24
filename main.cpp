// think about xseticon 
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#include "header.h"

class game : public engine {

	int nScreenWidth;
	int nScreenHeight;
	int nMapWidth = 16;				// World Dimensions
	int nMapHeight = 16;

	float fPlayerX = 14.7f;			// Player Start Position
	float fPlayerY = 5.09f;
	float fPlayerA = 0.0f;			// Player Start Rotation
	float fFOV = 3.14159f / 4.0f;	// Field of View
	float fDepth = 16.0f;			// Maximum rendering distance
	float fSpeed = 0.04f;			// Walking Speed

	std::wstring map;

	
	bool on_create () override
	{

		nScreenWidth = (int)get_screen_width();
		nScreenHeight = (int)get_screen_height();

		map += L"################";
		map += L"#..............#";
		map += L"#..............#";
		map += L"#..............#";
		map += L"#..##..##..##..#";
		map += L"#..............#";
		map += L"#..............#";
		map += L"#..............#";
		map += L"#..............#";
		map += L"######.........#";
		map += L"#..............#";
		map += L"#..............#";
		map += L"#..............#";
		map += L"#..............#";
		map += L"#..............#";
		map += L"################";


		return true;
	}

	bool on_update () override
	{
		std::vector<char>::iterator p;
		p = std::find( keys_pressed.begin(), keys_pressed.end(), 'a');
		if ( p != keys_pressed.end() ) {
			fPlayerA -= (fSpeed);
		}
		p = std::find( keys_pressed.begin(), keys_pressed.end(), 'd');
		if ( p != keys_pressed.end() ) {
			fPlayerA += (fSpeed);
		}
		p = std::find( keys_pressed.begin(), keys_pressed.end(), 'w');
		if ( p!= keys_pressed.end() ) {
			fPlayerX += sinf(fPlayerA) * fSpeed*15;
			fPlayerY += cosf(fPlayerA) * fSpeed*15;
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * fSpeed*15;
				fPlayerY -= cosf(fPlayerA) * fSpeed*15;
			}
		}
		p = std::find( keys_pressed.begin(), keys_pressed.end(), 's');
		if ( p!= keys_pressed.end() ) {
			fPlayerX -= sinf(fPlayerA) * fSpeed*15;
			fPlayerY -= cosf(fPlayerA) * fSpeed*15;
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX += sinf(fPlayerA) * fSpeed*15;
				fPlayerY += cosf(fPlayerA) * fSpeed*15;
			}
		}


		for (int x = 0; x < nScreenWidth; x++)
		{
			// For each column, calculate the projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV/2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			// Find distance to wall
			float fStepSize = 0.1f;		  // Increment size for ray casting, decrease to increase										
			float fDistanceToWall = 0.0f; //                                      resolution

			bool bHitWall = false;		// Set when ray hits wall block
			bool bBoundary = false;		// Set when ray hits boundary between two wall blocks

			float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
			float fEyeY = cosf(fRayAngle);

			// Incrementally cast ray from player, along ray angle, testing for 
			// intersection with a block
			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += fStepSize;
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
				
				// Test if ray is out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;			// Just set distance to maximum depth
					fDistanceToWall = fDepth;
				}
				else
				{
					// Ray is inbounds so test to see if the ray cell is a wall block
					if (map.c_str()[nTestX * nMapWidth + nTestY] == '#')
					{
						// Ray has hit wall
						bHitWall = true;

						// To highlight tile boundaries, cast a ray from each corner
						// of the tile, to the player. The more coincident this ray
						// is to the rendering ray, the closer we are to a tile 
						// boundary, which we'll shade to add detail to the walls
						std::vector<std::pair<float, float>> p;

						// Test each corner of hit tile, storing the distance from
						// the player, and the calculated dot product of the two rays
						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								// Angle of corner to eye
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx*vx + vy*vy); 
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(std::make_pair(d, dot));
							}

						// Sort Pairs from closest to farthest
						std::sort(p.begin(), p.end(), [](const std::pair<float, float> &left, const std::pair<float, float> &right) {return left.first < right.first; });
						
						// First two/three are closest (we will never see all four)
						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}
		
			// Calculate distance to ceiling and floor
			int nCeiling = (float)(nScreenHeight/2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			// Shader walls based on distance
			int colour_value;
			if (fDistanceToWall <= fDepth / 4.0f)			colour_value = 100;
			else if (fDistanceToWall < fDepth / 3.0f)		colour_value = 75;
			else if (fDistanceToWall < fDepth / 2.0f)		colour_value = 50;
			else if (fDistanceToWall < fDepth)				colour_value = 25;
			else											colour_value = 5;
			if (bBoundary)		 colour_value = 0;
			pixel px = pixel(colour_value,colour_value,colour_value);
			pixel px_wall = px;
			
			for (int y = 0; y < nScreenHeight; y++)
			{
				// Each Row
				if(y <= nCeiling) {
					float b = 1.0f + (((float)y - nScreenHeight/2.0f) / ((float)nScreenHeight / 2.0f));
					if (b > 0.9)		colour_value = 0;
					else if (b > 0.825)	colour_value = 12;
					else if (b > 0.75)	colour_value = 25;
					else if (b > 0.625)	colour_value = 37;
					else if (b > 0.5)	colour_value = 50;
					else if (b > 0.375)	colour_value = 62;
					else if (b > 0.25)	colour_value = 75;
					else if (b > 0.125)	colour_value = 87;
					else				colour_value = 100;
					draw( x, y, pixel(0,colour_value,colour_value));
				} else if(y > nCeiling && y <= nFloor) {
					draw( x, y, px_wall);
				} else { // Floor		
					// Shade floor based on distance
					float b = 1.0f - (((float)y -nScreenHeight/2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.125)		colour_value = 100;
					else if (b < 0.25)	colour_value = 87;
					else if (b < 0.375)	colour_value = 75;
					else if (b < 0.5)	colour_value = 62;
					else if (b < 0.625)	colour_value = 50;
					else if (b < 0.75)	colour_value = 37;
					else if (b < 0.825)	colour_value = 25;
					else if (b < 0.9)	colour_value = 12;
					else				colour_value = 0;

					draw( x, y, pixel(0,colour_value,0));
				}
			}
		}

		keys_pressed.clear();

		return true;
	}

	bool on_delete () override
	{
		keys_pressed.clear();
		return true;
	}
};

int main ()
{
	game ins;
	ins.create ( 1366, 768, 1, 1, false );
	ins.start();

	return 0;
}
