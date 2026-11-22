#pragma once
#include <vector>
#include <algorithm>
#include <cmath>

// Uniform grid to help with phase collision detection.
// The world is split into equal square cells stored in one flat vector, so finding a cell is plain arithmetic. 
// Each cell holds the ids of objects whose bounds overlap it.
class UniformGrid
{
public:
	void init(float worldMinX, float worldMinY, float worldMaxX, float worldMaxY, float size)
	{
		minX = worldMinX;
		minY = worldMinY;
		cellSize = size;
		cols = (int)std::ceil((worldMaxX - worldMinX) / cellSize);
		rows = (int)std::ceil((worldMaxY - worldMinY) / cellSize);
		cells.assign(cols * rows, std::vector<int>());
		stamps.clear();
		currentStamp = 0;
	}

	// Adds an object to every cell its bounding box overlaps
	void insert(int id, float left, float bottom, float right, float top)
	{
		if (id >= (int)stamps.size()) stamps.resize(id + 1, 0);
		forEachCell(left, bottom, right, top, [&](std::vector<int>& cell) {
			cell.push_back(id);
			});
	}

	// Removes an object from every cell its bounding box overlaps
	void remove(int id, float left, float bottom, float right, float top)
	{
		forEachCell(left, bottom, right, top, [&](std::vector<int>& cell) {
			cell.erase(std::remove(cell.begin(), cell.end(), id), cell.end());
			});
	}

	// Fills 'out' with every object in the cells the box overlaps, each one only once
	void query(float left, float bottom, float right, float top, std::vector<int>& out)
	{
		out.clear();
		currentStamp++;
		forEachCell(left, bottom, right, top, [&](std::vector<int>& cell) {
			for (int id : cell) {
				if (stamps[id] != currentStamp) {   // skip objects already returned
					stamps[id] = currentStamp;
					out.push_back(id);
				}
			}
			});
	}

private:
	float minX = 0.0f, minY = 0.0f, cellSize = 1.0f;
	int cols = 0, rows = 0;
	std::vector<std::vector<int>> cells;   // flat: index = row * cols + col
	std::vector<int> stamps;               // last query each object was returned in
	int currentStamp = 0;

	// Converts a world coordinate to a cell coordinate, clamped to the grid.
	// Clamped by hand because <windows.h> defines min and max macros.
	int cellCoord(float value, float origin, int count) const
	{
		int c = (int)std::floor((value - origin) / cellSize);
		if (c < 0) c = 0;
		if (c > count - 1) c = count - 1;
		return c;
	}

	template <typename Fn>
	void forEachCell(float left, float bottom, float right, float top, Fn fn)
	{
		int c0 = cellCoord(left, minX, cols);
		int c1 = cellCoord(right, minX, cols);
		int r0 = cellCoord(bottom, minY, rows);
		int r1 = cellCoord(top, minY, rows);
		for (int r = r0; r <= r1; r++)
			for (int c = c0; c <= c1; c++)
				fn(cells[r * cols + c]);
	}
};