#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>
#include <chrono>
#include <algorithm>
#include <cmath>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

// Half the world's width in world units. Set from the window's aspect ratio in main() so circles draw round instead of stretched.
float worldHalfWidth = 1.0f;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick
{
public:
	float red, green, blue;
	float x, y, width, height;
	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float hh, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx; y = yy, width = ww; height = hh; red = rr, green = gg, blue = bb;
		onoff = ON;
	};

	void drawBrick()
	{
		if (onoff == ON)
		{
			//double halfside = width / 2;
			double halfW = width / 2;
			double halfH = height / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfW, y + halfH);
			glVertex2d(x + halfW, y - halfH);
			glVertex2d(x - halfW, y - halfH);
			glVertex2d(x - halfW, y + halfH);

			glEnd();
		}
	}
};


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.01;
	int direction; // 1=down 2=right 3=up 4=left 5=down-right 6=down-left 7=up-right 8=up-left 

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
	}

	// Direction helpers. Diagonals move equally on both axes, so they are 45 degrees.
	bool MovingYUp() { return direction == 3 || direction == 7 || direction == 8; }
	bool MovingYDown() { return direction == 1 || direction == 5 || direction == 6; }
	bool MovingXRight() { return direction == 2 || direction == 5 || direction == 7; }
	bool MovingXLeft() { return direction == 4 || direction == 6 || direction == 8; }

	// Mirror bounce off a horizontal surface (top or bottom): flips the vertical part only
	int FlipVertical(int d)
	{
		switch (d) {
		case 1: return 3; case 3: return 1;
		case 5: return 7; case 7: return 5;
		case 6: return 8; case 8: return 6;
		}
		return d;
	}

	// Mirror bounce off a vertical surface (left or right): flips the horizontal part only
	int FlipHorizontal(int d)
	{
		switch (d) {
		case 2: return 4; case 4: return 2;
		case 5: return 6; case 6: return 5;
		case 7: return 8; case 8: return 7;
		}
		return d;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->onoff == OFF) return; // skip destroyed bricks

		float halfW = brk->width / 2.0f;
		float halfH = brk->height / 2.0f;

		// Exact circle vs rectangle test (closest-point method).
		// Clamp the circle's center to the brick to find the nearest point on it, then compare squared distances so no square root is needed.
		float closestX = fmaxf(brk->x - halfW, fminf(x, brk->x + halfW));
		float closestY = fmaxf(brk->y - halfH, fminf(y, brk->y + halfH));
		float dx = x - closestX;
		float dy = y - closestY;
		if (dx * dx + dy * dy > radius * radius) return;

		// The axis with the smaller overlap is the face the circle came through
		float overlapX = (halfW + radius) - fabs(x - brk->x);
		float overlapY = (halfH + radius) - fabs(y - brk->y);

		if (overlapY < overlapX) {
			// Hit the top or bottom face: mirror vertically and move outside the brick
			if (y < brk->y) {
				if (MovingYUp()) direction = FlipVertical(direction);
				y = brk->y - halfH - radius;
			}
			else {
				if (MovingYDown()) direction = FlipVertical(direction);
				y = brk->y + halfH + radius;
			}
		}
		else {
			// Hit the left or right face: mirror horizontally and move outside the brick
			if (x < brk->x) {
				if (MovingXRight()) direction = FlipHorizontal(direction);
				x = brk->x - halfW - radius;
			}
			else {
				if (MovingXLeft()) direction = FlipHorizontal(direction);
				x = brk->x + halfW + radius;
			}
		}

		// Both brick types reflect; destructible bricks also disappear
		if (brk->brick_type == DESTRUCTABLE) brk->onoff = OFF;
	}

	// Mirror bounce off a window wall, using the same rule as bricks, so a circle never keeps pushing into a wall
	int GetDirectionAwayFrom(int wall)
	{
		// wall: 0 = bottom, 1 = right, 2 = top, 3 = left
		return (wall == 0 || wall == 2) ? FlipVertical(direction) : FlipHorizontal(direction);
	}

	void MoveOneStep()
	{
		if (direction == 1 || direction == 5 || direction == 6)  // y decreasing (down on screen)
		{
			if (y > -1 + radius) y -= speed;
			else direction = GetDirectionAwayFrom(0);
		}

		if (direction == 2 || direction == 5 || direction == 7)  // x increasing (right)
		{
			if (x < worldHalfWidth - radius) x += speed;
			else direction = GetDirectionAwayFrom(1);
		}

		if (direction == 3 || direction == 7 || direction == 8)  // y increasing (up on screen)
		{
			if (y < 1 - radius) y += speed;
			else direction = GetDirectionAwayFrom(2);
		}

		if (direction == 4 || direction == 6 || direction == 8)  // x decreasing (left)
		{
			if (x > -worldHalfWidth + radius) x -= speed;
			else direction = GetDirectionAwayFrom(3);
		}
	}

	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};


vector<Circle> world;
bool spacePressedLastFrame = false;

vector<Brick> bricks;
bool resetKeyDown = false;

// Builds (or rebuilds) the rows of bricks with new random colors and destructible picks
void buildBricks()
{
	bricks.clear();

	float startX = -worldHalfWidth;   // starting left side position
	float startY = 0.8f;
	int brickCount = 8;
	int rowCount = 4;
	float rowSpacing = 0.35f;
	float brickWidth = (2.0f * worldHalfWidth) / brickCount;
	float brickHeight = (2.0f / brickCount) / 2.5f;

	for (int j = 0; j < rowCount; j++) {

		// randomly decide which 2 bricks in the row will be destructible
		vector<int> destructibleIndices;
		while (destructibleIndices.size() < 2) {
			int randIndex = rand() % brickCount;
			if (find(destructibleIndices.begin(), destructibleIndices.end(), randIndex) == destructibleIndices.end()) {
				destructibleIndices.push_back(randIndex);
			}
		}

		for (int i = 0; i < brickCount; i++) {
			float red = (float)rand() / RAND_MAX;
			float green = (float)rand() / RAND_MAX;
			float blue = (float)rand() / RAND_MAX;

			BRICKTYPE type = (find(destructibleIndices.begin(), destructibleIndices.end(), i) != destructibleIndices.end())
				? DESTRUCTABLE : REFLECTIVE;

			// no horizontal gap between bricks
			float x = startX + (i * brickWidth) + (brickWidth / 2.0f);
			float y = startY - (j * rowSpacing);

			bricks.push_back(Brick(type, x, y, brickWidth, brickHeight, red, green, blue));
		}
	}
}

// Benchmark instrumentation 
long long checksThisFrame = 0;
long long totalChecks = 0;
double totalCollisionMs = 0.0;
int framesMeasured = 0;
const int REPORT_EVERY = 120;
bool benchKeyDown[3] = { false, false, false };
bool clearKeyDown = false;

// Spawns a batch of circles at random positions for benchmarking
void spawnBatch(int count)
{
	for (int i = 0; i < count; i++) {
		float x = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * (worldHalfWidth - 0.1f);
		float y = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 0.9f;
		int dir = (rand() % 8) + 1;
		float r = (float)rand() / RAND_MAX;
		float g = (float)rand() / RAND_MAX;
		float b = (float)rand() / RAND_MAX;
		world.push_back(Circle(x, y, 0.05, dir, 0.05f, r, g, b));
	}
}

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // world size is set once from the window
	GLFWwindow* window = glfwCreateWindow(1200, 900, "8-2 Assignment - Josh Hall", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Original computed an aspect ratio and never used it, so the 1200x900 window stretched circles into ovals. Widen the world to match.
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	worldHalfWidth = fbWidth / (float)fbHeight;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-worldHalfWidth, worldHalfWidth, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

	buildBricks();

	while (!glfwWindowShouldClose(window)) {
		//Setup View
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		// Collision pass, timed separately from drawing
		checksThisFrame = 0;
		auto collisionStart = chrono::high_resolution_clock::now();
		for (int i = 0; i < world.size(); i++) {
			for (int j = 0; j < bricks.size(); j++) {
				world[i].CheckCollision(&bricks[j]);
				checksThisFrame++;
			}
		}
		auto collisionEnd = chrono::high_resolution_clock::now();

		// Movement and drawing pass
		for (int i = 0; i < world.size(); i++) {
			world[i].MoveOneStep();
			world[i].DrawCircle();
		}

		// Print averages to the console every REPORT_EVERY frames
		totalChecks += checksThisFrame;
		totalCollisionMs += chrono::duration<double, milli>(collisionEnd - collisionStart).count();
		framesMeasured++;
		if (framesMeasured == REPORT_EVERY) {
			cout << "Brute force | circles: " << world.size()
				<< " | avg checks/frame: " << totalChecks / framesMeasured
				<< " | avg collision time: " << totalCollisionMs / framesMeasured << " ms" << endl;
			totalChecks = 0;
			totalCollisionMs = 0.0;
			framesMeasured = 0;
		}

		// Draw all bricks in the vector
		for (int j = 0; j < bricks.size(); j++) {
			bricks[j].drawBrick();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// I wanted to have 1 cicrlce not many thrown in for now
	//if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
	//	if (!spacePressedLastFrame) {
	//		// only runs once per press
	//		double r = (double)rand() / RAND_MAX;
	//		double g = (double)rand() / RAND_MAX;
	//		double b = (double)rand() / RAND_MAX;
	//		Circle B(0, -0.8, 0.02, 2, 0.05, r, g, b);
	//		world.push_back(B);
	//	}
	//	spacePressedLastFrame = true;  
	//}
	//else {
	//	spacePressedLastFrame = false; 
	//}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		double r, g, b;
		r = (double)rand() / RAND_MAX;
		g = (double)rand() / RAND_MAX;
		b = (double)rand() / RAND_MAX;
		// Launch from the bottom center at a random 45 degree angle toward the bricks
		int launchDir = (rand() % 2 == 0) ? 7 : 8;  // up-right or up-left
		Circle B(0, -0.8, 0.05, launchDir, 0.05, r, g, b);
		world.push_back(B);
	}

	// Benchmark keys: 1 = 100 circles, 2 = 1,000, 3 = 5,000 (once per press)
	int benchKeys[3] = { GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3 };
	int benchCounts[3] = { 100, 1000, 5000 };
	for (int k = 0; k < 3; k++) {
		bool down = glfwGetKey(window, benchKeys[k]) == GLFW_PRESS;
		if (down && !benchKeyDown[k]) spawnBatch(benchCounts[k]);
		benchKeyDown[k] = down;
	}

	// C clears all circles so each benchmark run starts fresh
	bool cDown = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
	if (cDown && !clearKeyDown) world.clear();

	clearKeyDown = cDown;


	// R rebuilds the bricks with a new random layout
	bool rDown = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
	if (rDown && !resetKeyDown) buildBricks();
	resetKeyDown = rDown;
}