#include <stdio.h>
#include <raylib.h>
#include "solids.h"


const int gravDir = 1;
const int runSpeed = 3;
const double maxVSpeed = 9.0;
const double jumpForce = 8.5;
const double djumpForce = 7.0;
const double gravityCoef = 0.4;
int djump = 1;
const int maxDJump = 1;


int main(void)
{
	InitWindow(800, 608, "Collisions demo");

	iwemu::Hitbox solids[] = {
		iwemu::Hitbox({320, 320, 64, 64, 0, 0}),
		iwemu::Hitbox({256, 336, 32, 32, 0, 0}),
		iwemu::Hitbox({200, 320, 32, 32, 0, 1})
	};
	size_t solidsC = sizeof(solids) / sizeof(solids[0]);
	iwemu::Segment segments[] = {
		iwemu::Segment({224, 224, 32, false, true, false, 0, 1}),
		iwemu::Segment({288, 212, 32, true, true, false, 0, 0})
	};
	size_t segmentsC = sizeof(segments) / sizeof(segments[0]);
	iwemu::BBox collidables[] = {
		iwemu::BBox({256.0, 298.0, 11, 21, 0.0, 0.0})
	};
	size_t collidablesC = sizeof(collidables) / sizeof(collidables[0]);
	iwemu::SolidScene scene(1, solids, solidsC, segments, segmentsC, collidables, collidablesC);

	SetTargetFPS(50);
	while (!WindowShouldClose())
	{
		iwemu::BBox& player = collidables[0];
		if (IsKeyPressed(KEY_W))
		{
			scene.alive[0] = true;
			player.x = GetMouseX();
			player.y = GetMouseY();
		}
		char h = 0;
		if (IsKeyDown(KEY_RIGHT))
			h = 1;
		else if (IsKeyDown(KEY_LEFT))
			h = -1;
		bool standing = scene.project_free_down(player) <= 1.0;
		if (h)
		{
			player.dx = h * runSpeed;
		}
		else
		{
			player.dx = 0.0;
		}

		if (gravDir * player.dy > maxVSpeed)
		{
			player.dy = gravDir * maxVSpeed;
		}
		if (standing)
		{
			djump = maxDJump;
		}

		if (IsKeyPressed(KEY_LEFT_SHIFT))
		{
			if (standing)
			{
				printf("Ground jump\n");
				player.dy = -jumpForce;
			}
			else if (djump > 0)
			{
				printf("Air jump\n");
				player.dy = -djumpForce;
				djump--;
			}

		}
		if (IsKeyReleased(KEY_LEFT_SHIFT))
		{
			if (player.dy * gravDir < 0.0)
			{
				player.dy *= 0.45;
			}
		}
		player.dy += gravDir * gravityCoef;
		scene.update();

		char txt[64];
		snprintf(txt, 64, "%f %f", collidables[0].x + 5.0, collidables[0].y + 12.0);
		BeginDrawing();
			ClearBackground(PURPLE);
			DrawText(txt, 64, 64, 18, BLACK);
			for (unsigned int i = 0; i < solidsC; i++)
			{
				DrawRectangle(solids[i].x, solids[i].y, solids[i].width, solids[i].height, VIOLET);
			}
			for (unsigned int i = 0; i < segmentsC; i++)
			{
				if (segments[i].vertical)
				{
					if (segments[i].block_lt)
						DrawLine(segments[i].x, segments[i].y, segments[i].x, segments[i].y + segments[i].length, BLACK);
					else 
						DrawLine(segments[i].x, segments[i].y, segments[i].x, segments[i].y + segments[i].length, GRAY);
					if (segments[i].block_rb)
						DrawLine(segments[i].x + 1, segments[i].y, segments[i].x + 1, segments[i].y + segments[i].length, BLACK);
					else 
						DrawLine(segments[i].x + 1, segments[i].y, segments[i].x + 1, segments[i].y + segments[i].length, GRAY);
				}
				else
				{
					if (segments[i].block_lt)
						DrawLine(segments[i].x, segments[i].y, segments[i].x + segments[i].length, segments[i].y, BLACK);
					else 
						DrawLine(segments[i].x, segments[i].y, segments[i].x + segments[i].length, segments[i].y, GRAY);
					if (segments[i].block_rb)
						DrawLine(segments[i].x, segments[i].y + 1, segments[i].x + segments[i].length, segments[i].y + 1, BLACK);
					else
						DrawLine(segments[i].x, segments[i].y + 1, segments[i].x + segments[i].length, segments[i].y + 1, GRAY);
				}
			}
			if (!scene.alive[0])
			{
				DrawRectangle(collidables[0].x, collidables[0].y, collidables[0].width, collidables[0].height, RED);
			}
			else
			{
				DrawRectangle(collidables[0].x, collidables[0].y, collidables[0].width, collidables[0].height, MAGENTA);
			}
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
