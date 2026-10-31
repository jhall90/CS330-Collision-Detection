#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

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
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left

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

	void CheckCollision(Brick* brk)
	{
		/*if (brk->brick_type == REFLECTIVE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				direction = GetRandomDirection();
				x = x + 0.03;
				y = y + 0.04;
			}
		}
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				brk->onoff = OFF;
			}
		}*/
		if (brk->onoff == OFF) return; // skip inactive bricks

		float halfW = brk->width / 2.0f;
		float halfH = brk->height / 2.0f;

		bool collisionX = x + radius > brk->x - halfW && x - radius < brk->x + halfW;
		bool collisionY = y + radius > brk->y - halfH && y - radius < brk->y + halfH;

		if (collisionX && collisionY)
		{
			if (brk->brick_type == REFLECTIVE)
			{
				// Simple bounce: reverse direction vertically
				direction = (direction == 3) ? 1 : (direction == 1 ? 3 : GetRandomDirection());
				y -= 0.02f; // nudge slightly to avoid getting stuck
			}
			else if (brk->brick_type == DESTRUCTABLE)
			{
				brk->onoff = OFF;
				// reverse direction slightly when destroying a brick
				direction = (direction == 3) ? 1 : (direction == 1 ? 3 : GetRandomDirection());
			}
		}
	}

	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	void MoveOneStep()
	{
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y > -1 + radius)
			{
				y -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius)
			{
				x += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius) {
				y += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
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

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(1200, 900, "8-2 Assignment - Josh Hall", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	/*Brick brick(REFLECTIVE, 0.5, -0.33, 0.2, 1, 1, 0);
	Brick brick2(DESTRUCTABLE, -0.5, 0.33, 0.2, 0, 1, 0);
	Brick brick3(DESTRUCTABLE, -0.5, -0.33, 0.2, 0, 1, 1);
	Brick brick4(REFLECTIVE, 0, 0, 0.2, 1, 0.5, 0.5);*/

	// Create a dynamic amount of rows of randomly colored bricks
	// no gap between bricks, but a little height gap so the ball can bounce until it hits another destructive
	
	vector<Brick> bricks;
	float startX = -1.0f;   // starting left side position
	float startY = 0.8f;    
	int brickCount = 8;
	int rowCount = 4;
	float rowSpacing = 0.35f;
	float brickWidth = 2.0f / brickCount;
	float brickHeight = brickWidth / 2.5f;
	//float brickWidth = 0.2f;
	//float totalWidth = brickCount * brickWidth;


	// build the rows
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

			// 2 destructible bricks per row
			BRICKTYPE type = (find(destructibleIndices.begin(), destructibleIndices.end(), i) != destructibleIndices.end())
				? DESTRUCTABLE : REFLECTIVE;

			// no horizontal gap between bricks
			/*float x = startX + offsetX + (i * brickWidth) + (brickWidth / 2.0f);*/
			float x = startX + (i * brickWidth) + (brickWidth / 2.0f);
			float y = startY - (j * rowSpacing);

			bricks.push_back(Brick(type, x, y, brickWidth, brickHeight, red, green, blue));
		}
	}

	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		// Movement and collisions with vector Bricks
		for (int i = 0; i < world.size(); i++) {
			// Check collisions with all bricks in the vector
			for (int j = 0; j < bricks.size(); j++) {
				world[i].CheckCollision(&bricks[j]);
			}

			world[i].MoveOneStep();
			world[i].DrawCircle();
		}

		// Draw all bricks in the vector
		for (int j = 0; j < bricks.size(); j++) {
			bricks[j].drawBrick();
		}

		////Movement
		//for (int i = 0; i < world.size(); i++)
		//{
		//	world[i].CheckCollision(&brick);
		//	world[i].CheckCollision(&brick2);
		//	world[i].CheckCollision(&brick3);
		//	world[i].CheckCollision(&brick4);
		//	world[i].MoveOneStep();
		//	world[i].DrawCircle();

		//}

		//brick.drawBrick();
		//brick2.drawBrick();
		//brick3.drawBrick();
		//brick4.drawBrick();

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
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;
		Circle B(0, 0, 02, 2, 0.05, r, g, b);
		world.push_back(B);
	}
}