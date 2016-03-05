/* Snake.h */

#ifndef SNAKE_HEADER
#define SNAKE_HEADER

#include <stddef.h>
#include <vector>
#include <queue>
#include <list>
#include <string>

namespace SnakeCurses
{

// types & constants

enum Direction {DR_UP = 0, DR_DOWN, DR_RIGHT, DR_LEFT};
typedef enum {ST_FREE, ST_BUSY, ST_FOOD} CellState;
//~ typedef enum {CL_BUSY = 1, CL_FOOD} Color;
typedef enum {CL_BLACK = 1, CL_RED, CL_GREEN, CL_YELLOW,
			  CL_BLUE, CL_MAGENTA, CL_CYAN, CL_WHITE} Color;
typedef enum {GM_NOT_EXISTS, GM_RUNING, GM_IN_PAUSE, GM_OFF} GameState;


class Point
{
public:
	int x, y;
	Point(int _x = 0, int _y = 0):x(_x), y(_y) {}
	
	void move(Direction dir)
	{
		switch (dir)
		{
			case DR_UP: 	y--; break;
			case DR_DOWN: 	y++; break;
			case DR_RIGHT: 	x++; break;
			case DR_LEFT: 	x--; break;
		}
	}
	
	bool operator==(const Point& pt) const
	{
		return x == pt.x && y == pt.y;
	}
};


class World
{
public:
	World() {}
	World(size_t _height, size_t _width);
	
	void reset(size_t _height, size_t _width);
	CellState getCellState(Point pt) const;
	CellState setCellState(Point pt, CellState state);
	
	size_t getWidth() const;
	size_t getHeight() const;
	Point getMidle() const;
	Point backToWorld(Point pt) const;
	
	void draw() const;
	
	~World() {}

private:
	size_t height, width;
	std::vector<std::vector<CellState> > matrix;
};



class Snake
{
public:
	typedef enum {SN_BODY, SN_HEAD, SN_FOOD,
		SN_TL_UP, SN_TL_DOWN, SN_TLRIGHT, SN_TL_LEFT} PartType;
	typedef enum {SN_ST_NORMAL, SN_ST_EAT, SN_ST_DEAD} SnakeState;
	
	static const size_t START_LEN 	= 4;
	static const size_t START_DISPL = 4;

	Snake() {}
	Snake(Point headPos, Direction startDir,
		  size_t startLen = START_LEN, Color color = CL_YELLOW);
	
	void reset(Point headPos, Direction startDir,
				size_t startLen = START_LEN,
				Color color = CL_YELLOW);
	
	void step(const World &world);
	void setDirection(Direction dir);
	bool isPointOnBody(Point pt) const;
	size_t getBusyTime(Point pt) const;
	SnakeState getState() const;
	void setState(SnakeState st);
	void draw();
	Point getHeadPos() const;
	Direction getDirection() const;
	
	~Snake() {}

private:
	struct SnakePart
	{
		Point pt;
		PartType type;
		Direction dir;
	};
	
	//~ const World &world;
	std::vector<SnakePart> body;
	Color color;
	SnakeState state;
};



class Opponent
{
public:
	Opponent() {}
	Opponent(const World &world);
	
	void reset(const World &world);
	void step(const World &world, const Snake &playerSnake);
	void control(const World &world, const Snake &playerSnake);
	void draw();
	const Snake& getSnake() const {return snake;} 
	~Opponent() {};
	
	bool isPointOnBody(Point pt)
	{
		return snake.isPointOnBody(pt);
	}

private:
	struct SmartPoint
	{
		Point parent;
		size_t distance;
		
		SmartPoint() {}
		SmartPoint(Point par, size_t dist):parent(par), distance(dist) {}
	};

	FILE *logFile;
	Snake snake;
	std::vector<std::vector<SmartPoint> > matrix;
	std::list<Point> way;
	
	void findWay(const World &world, const Snake &playerSnake);
};


class SnakeException
{
public:
	std::string message;
	SnakeException(const char msg[]):message(msg) {}
};


class Game
{
public:
	Game(double busyK = 0.03, int fps = 6);
	
	void reset();
	void step();
	void pause();
	void stop();
	void play();
	void addCommand(Direction comm);
	void draw();
	//~ void testDraw();
	
	GameState getState() const;
	size_t getFrameDelay() const;
	
	~Game() {}

private:
	GameState state;
	World world;
	//~ std::vector<Snake> snakes;
	Snake playerSnake;
	Opponent opponent;
	std::queue<Direction> commands;
	double BUSY_K;
	int FPS;
	
	Point getEmptyCell(size_t startDisplX = 0, size_t startDisplY = 0) const;
	void addFood();
};


}	// SnakeCurses

#endif // SNAKE_HEADER
