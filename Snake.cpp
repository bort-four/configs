// snake.c

#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <queue>
#include <vector>

#include "Snake.hpp"

using namespace SnakeCurses;

const char CELL_STR[][3] = {"  ", "  ", "@@"};
const int  CELL_ATTR[]   =
					{A_NORMAL, A_BOLD | A_REVERSE | COLOR_PAIR(CL_BLUE),
					 A_BOLD | COLOR_PAIR(CL_RED)};
const char PART_STR[][3] = {"[]", "()", "[]",  "|/", "/|", "<{", "}>"};
const int  PART_ATTR[]   =
		{A_NORMAL, A_BOLD, A_BOLD, A_NORMAL, A_NORMAL, A_NORMAL, A_NORMAL};


void initColors();
void mainLoop(Game &game);



int main(int argc, char** argv)
{
	srand(time(NULL));
	initscr();
	initColors();
	//~ //////signal(SIGWINCH, resize);
	noecho(); //cbreak();
	nodelay(stdscr, true);
	keypad(stdscr, true);
	curs_set(0);
	clear();
	
	try
	{
		Game game;
		mainLoop(game);
	}
	catch (SnakeException exc)
	{
		printw("SnakeException: %s\n", exc.message.c_str());
	}
	
	clear(); endwin();
	return 0;
}


void initColors()
{
	start_color();
	use_default_colors();
	
	init_pair(CL_BLACK,		COLOR_BLACK, 	-1);
	init_pair(CL_RED,		COLOR_RED, 		-1);
	init_pair(CL_GREEN, 	COLOR_GREEN, 	-1);
	init_pair(CL_YELLOW, 	COLOR_YELLOW, 	-1);
	init_pair(CL_BLUE, 		COLOR_BLUE, 	-1);
	init_pair(CL_MAGENTA,	COLOR_MAGENTA, 	-1);
	init_pair(CL_CYAN, 		COLOR_CYAN, 	-1);
	init_pair(CL_WHITE, 	COLOR_WHITE, 	-1);
	//~ init_pair(CL_BUSY, COLOR_BLUE, -1);
	//~ init_pair(CL_SNAKE, COLOR_YELLOW, -1);
	//~ init_pair(CL_FOOD, COLOR_GREEN, -1);
}

//~ unsigned int abs(int x) {return (x < 0) ? -x : x;}


void mainLoop(Game &game)
{
	int ch, comNum;
	int mvCommands[8] = {'w', 's', 'd','a',
			KEY_UP, KEY_DOWN, KEY_RIGHT, KEY_LEFT};
	bool stopMainLoop = false;
	
	game.play();
	
	while (!stopMainLoop)
	{
		//printf("1\n");
		
		while ((ch = getch()) != ERR)
		{
			// if ch is move command
			for (comNum = 0; comNum < 8 && ch != mvCommands[comNum]; comNum++);
			if (comNum < 8)
			{
				// then save it in comands_g
				Direction dir = (Direction)(comNum % 4);
				game.addCommand(dir);
			}
			else if (ch == 'q')
			{
				game.stop();
				stopMainLoop = true;
			}
			else if (ch == ' ')
			{
				if (game.getState() == GM_RUNING)
					game.pause();
				else if (game.getState() == GM_IN_PAUSE)
					game.play();
			}
			else if (ch == 'r')
			{
				game.reset();
				game.play();
			}
		}
		
		if (game.getState() == GM_RUNING)
		{
			game.step();
			game.draw();
		}
		
		usleep(game.getFrameDelay());
	}
}



namespace SnakeCurses
{

// ////////////////////////////// OPPONENT


Opponent::Opponent(const World &world)
{
	reset(world);
}

void Opponent::reset(const World &world)
{
	Point snkHeadPos = world.getMidle();
	snkHeadPos.x += Snake::START_DISPL;
	snake.reset(snkHeadPos, DR_LEFT, Snake::START_LEN, CL_GREEN);
	way.clear();
	
	matrix = std::vector<std::vector<SmartPoint> >(world.getHeight(),
						 std::vector<SmartPoint>(world.getWidth(),
									 SmartPoint(Point(), 0)));
}

void Opponent::draw() { snake.draw(); }


void Opponent::control(const World &world, const Snake &playerSnake)
{
	std::list<Point>::iterator it = way.begin();
	
	for (; it != way.end(); ++it)
		if (playerSnake.isPointOnBody(*it))
		{
			way.clear();
			it = way.end();
		}
	
	if (way.empty())
	{
		findWay(world, playerSnake);
		//way.push(Point());
	}
	
	if (way.empty())
	{
		//~ snake.setDirection(DR_DOWN);
	}
	else
	{
		Point currPt = snake.getHeadPos();
		Point nextPt = way.front();
		way.pop_front();
		Direction dir;
		
		int dx = nextPt.x - currPt.x;
		int dy = nextPt.y - currPt.y;
		
			 if (dy == -1 || dy >  1) dir = DR_UP;
		else if (dy ==  1 || dy < -1) dir = DR_DOWN;
		else if (dx == -1 || dx >  1) dir = DR_LEFT;
		else if (dx ==  1 || dx < -1) dir = DR_RIGHT;
		
		snake.setDirection(dir);
	}
	//~ snake.step(world);
}


void Opponent::step(const World &world, const Snake &playerSnake)
{
	snake.step(world);
}


void Opponent::findWay(const World &world, const Snake &playerSnake)
{
	bool isFound = false;
	std::queue<Point> newCells;
	newCells.push(Point(snake.getHeadPos()));
	
	while (!newCells.empty() && !isFound)
	{
		Point pt = newCells.front();
		newCells.pop();
		size_t dist = matrix[pt.y][pt.x].distance;
		
		for (Direction dir = DR_UP; dir <= DR_LEFT; dir = (Direction)(dir + 1))
		{
			Point _pt = pt;
			_pt.move(dir);
			
			//~ if (_pt.x < 0 || _pt.y < 0 || _pt.x >= (long)world.getWidth()
									   //~ || _pt.y >= (long)world.getHeight())
				//~ continue;
			
			_pt = world.backToWorld(_pt);
			
			if (world.getCellState(_pt) != ST_BUSY
				&& playerSnake.getBusyTime(_pt) < dist + 1
				&& 		 snake.getBusyTime(_pt) < dist + 1
				&& matrix[_pt.y][_pt.x].distance == 0
				&& !(_pt == snake.getHeadPos()))
			{
				newCells.push(Point(_pt));
				
				matrix[_pt.y][_pt.x].parent   = pt;
				matrix[_pt.y][_pt.x].distance = dist + 1;
			}
		}
		
		if (world.getCellState(pt) == ST_FOOD)
		{
			isFound = true;
			way.clear();
			
			while (!(pt == snake.getHeadPos()))
			{
				way.push_front(pt);
				pt = matrix[pt.y][pt.x].parent;
			}
		}
	}
	
	for (long y = 0; y < (long)world.getHeight(); y++)
		for (long x = 0; x < (long)world.getWidth(); x++)
			matrix[y][x].distance = 0;
}


// ////////////////////////////// GAME

Game::Game(double busyK, int fps)
{
	state = GM_NOT_EXISTS;
	BUSY_K = busyK;
	FPS = fps;
	
	reset();
}

void Game::reset()
{
	world.reset(LINES, COLS / 2);
	//~ world.reset(10, 20);
	
	// create player's snake
	Point snkHeadPos = world.getMidle();
	snkHeadPos.x -= Snake::START_DISPL;
	playerSnake.reset(snkHeadPos, DR_RIGHT);
	
	// create opponent
	opponent.reset(world);
	
	// generate map
	size_t maxBusyCt = world.getHeight() * world.getWidth() * BUSY_K;
	size_t startDisplX = Snake::START_LEN + Snake::START_DISPL;
	
	for (size_t busyCt = 0; busyCt < maxBusyCt; busyCt++)
		world.setCellState(getEmptyCell(startDisplX, 1), ST_BUSY);

	addFood();
	addFood();
	state = GM_IN_PAUSE;
	
	//testDraw();
}

void Game::step()
{
	if (state != GM_RUNING)
		throw SnakeException("step: invalid game state");
	
	const Snake& oppSnake = opponent.getSnake();
	opponent.control(world, playerSnake);
	
	if (!commands.empty())
	{
		Direction comm = commands.front();
		commands.pop();
		playerSnake.setDirection(comm);
	}
	
	Point nextHeadPosP = playerSnake.getHeadPos();
	nextHeadPosP.move(playerSnake.getDirection());
	CellState nextCellStateP = world.getCellState(nextHeadPosP);
	
	Point nextHeadPosO = oppSnake.getHeadPos();
	nextHeadPosO.move(oppSnake.getDirection());
	CellState nextCellStateO = world.getCellState(nextHeadPosO);
	
	if (nextCellStateP == ST_BUSY || nextCellStateO == ST_BUSY
		|| playerSnake.isPointOnBody(nextHeadPosP)
		|| playerSnake.isPointOnBody(nextHeadPosO)
		|| 	  oppSnake.isPointOnBody(nextHeadPosO)
		|| 	  oppSnake.isPointOnBody(nextHeadPosP)
		|| nextHeadPosP == nextHeadPosO)
		state = GM_OFF;
	else
	{
		playerSnake.step(world);
		opponent.step(world, playerSnake);
		
		if (nextCellStateO == ST_FOOD)
		{
			world.setCellState(nextHeadPosO, ST_FREE);
			addFood();
		}
		if (nextCellStateP == ST_FOOD)
		{
			world.setCellState(nextHeadPosP, ST_FREE);
			addFood();
		}
	}
}

void Game::pause() {state = GM_IN_PAUSE;}
void Game::stop() {state = GM_OFF;}
void Game::play() {if (state == GM_IN_PAUSE) state = GM_RUNING;}

void Game::draw()
{
	world.draw();
	playerSnake.draw();
	opponent.draw();
	refresh();
}

void Game::addCommand(Direction comm)
{
	if (!commands.empty() || comm != playerSnake.getDirection())
		commands.push(comm);
}

GameState Game::getState() const {return state;}
size_t Game::getFrameDelay() const {return 1000000 / FPS;}


Point Game::getEmptyCell(size_t startDisplX, size_t startDisplY) const
{
	Point pt(rand() % world.getWidth(), rand() % world.getHeight());
	Point midle = world.getMidle();
	
	while (world.getCellState(pt) != ST_FREE
		|| playerSnake.isPointOnBody(pt)
		|| opponent.getSnake().isPointOnBody(pt)
		|| ((size_t)abs(pt.x - midle.x) <= startDisplX &&
			(size_t)abs(pt.y - midle.y) <= startDisplY))
		pt = Point(rand() % world.getWidth(), rand() % world.getHeight());
	
	return pt;
}


void Game::addFood()
{
	world.setCellState(getEmptyCell(), ST_FOOD);
}


// //////////////////////////// SNAKE

//~ Snake::Snake(const World &_world, Point headPos, Direction startDir,
Snake::Snake(Point headPos, Direction startDir, size_t startLen, Color cl)
{
	reset(headPos, startDir, startLen, cl);
}
				
	
void Snake::reset(Point headPos, Direction startDir, size_t startLen, Color cl)
{
	color = cl;
	state = SN_ST_NORMAL;
	body.resize(startLen);
	Point currPos = headPos;
	Direction revDir = (Direction)(startDir + 1 - 2*(startDir % 2));
	
	for (size_t parNum = 0; parNum < body.size(); parNum++)
	{
		//~ body.push_back(SnakePart());
		body[parNum].pt = currPos;
		body[parNum].dir = startDir;
		body[parNum].type = SN_BODY;
		currPos.move(revDir);
	}
	body[body.size() - 1].type = (PartType)(SN_TL_UP + startDir);
	body[0].type = SN_HEAD;
}


void Snake::step(const World &world)
{
	if (state == SN_ST_DEAD) return;
	
	Point nextHeadPos = getHeadPos();
	nextHeadPos.move(getDirection());
	nextHeadPos = world.backToWorld(nextHeadPos);
	CellState nextCellState = world.getCellState(nextHeadPos);
	
	if (nextCellState == ST_FOOD) body.push_back(SnakePart());

	for (size_t partNum = body.size() - 1; partNum >= 1 ; --partNum)
		body[partNum] = body[partNum - 1];

	body[1].type = (state == SN_ST_EAT) ? SN_FOOD : SN_BODY;
	body[body.size() - 1].type = (PartType)(SN_TL_UP + body[body.size() - 1].dir);
	body[0].pt = nextHeadPos;
	state = (nextCellState == ST_FOOD) ? SN_ST_EAT : SN_ST_NORMAL;
}


void Snake::setDirection(Direction dir)
{
	Direction revDir = (Direction)(dir + 1 - 2 * (dir % 2));
	if (body[0].dir != revDir) body[0].dir = dir;
}

Direction Snake::getDirection() const {return body[0].dir;}
Point 	  Snake::getHeadPos()   const {return body[0].pt;}

size_t Snake::getBusyTime(Point pt) const
{
	size_t partNum;
	for (partNum = 0; partNum < body.size()
						&& !(body[partNum].pt == pt); partNum++);
	
	if (partNum == body.size()) return 0;
	else return body.size() - 1 - partNum;
}

bool Snake::isPointOnBody(Point pt) const
{
	return getBusyTime(pt) != 0;
}


Snake::SnakeState Snake::getState() const { return state; }
void Snake::setState(SnakeState st) { state = st; }


void Snake::draw()
{
	for (size_t partNum = 0; partNum < body.size(); partNum++)
	{
		SnakePart part = body[partNum];
		attrset(PART_ATTR[part.type] | COLOR_PAIR(color));
		mvprintw(part.pt.y, part.pt.x*2, "%s", PART_STR[part.type]);
	}
	attrset(A_NORMAL);
	//~ refresh();
}

// //////////////////////////////// WORLD


World::World(size_t _height, size_t _width)
{
	reset(_height, _width);
}

void World::reset(size_t _height, size_t _width)
{
	height = _height;
	width = _width;
	
	matrix = std::vector<std::vector<CellState> >(height,
						 std::vector<CellState>(width, ST_FREE));
}

CellState World::getCellState(Point pt) const
{
	pt = backToWorld(pt);
	return matrix[pt.y][pt.x];
}
CellState World::setCellState(Point pt, CellState state)
{
	pt = backToWorld(pt);
	return matrix[pt.y][pt.x] = state;
}

size_t World::getWidth() const {return width;}
size_t World::getHeight() const {return height;}
Point World::getMidle() const {return Point(width / 2, height / 2);}

Point World::backToWorld(Point pt) const
{
	if (pt.x < 0) pt.x = width - 1;
	if (pt.y < 0) pt.y = height - 1;
	if ((size_t)pt.x >= width) pt.x = 0;
	if ((size_t)pt.y >= height) pt.y = 0;
	
	return pt;
}

void World::draw() const
{
	size_t x, y;
	
	for (x = 0; x < width; x++)
		for (y = 0; y < height; y++)
		{
			CellState state = getCellState(Point(x, y));
			attrset(CELL_ATTR[state]); // COLOR_PAIR(CL_BLUE)
			mvprintw(y, x*2, "%s", CELL_STR[state]);
		}
	attrset(A_NORMAL);
	//~ refresh();
}

} // SnakeCurses
