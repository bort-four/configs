#!/usr/bin/python3

## import
from curses import wrapper
import curses
import random
import time
import datetime

## defenitions
## variables
## types

class ScreenPoint:
	def __init__(self, y0 = 0, x0 = 0):
		self.y = int(y0)
		self.x = int(x0)


class WorldPoint:
	def __init__(self, i0 = 0, j0 = 0):
		self.i = int(i0)
		self.j = int(j0)
		
	#~ def toScreenPoint(self, worldPos = ScreenPoint(0, 0), scale = ScreenPoint(1, 1)):
		#~ return ScreenPoint( int(worldPos.y + self.i * scale.y),
							#~ int(worldPos.x + self.j	* scale.x))


class DrawProporties:
	def __init__(self,
				 worldDispl = ScreenPoint(0, 0),
				 worldScale = ScreenPoint(1, 1)):
		self.worldDispl = worldDispl
		self.worldScale = worldScale
	
	def toScreenPoint(self, point):
		return ScreenPoint( int(self.worldDispl.y + point.i * self.worldScale.y),
							int(self.worldDispl.x + point.j * self.worldScale.x))


class Figure:
	def __init__(self,
				position = WorldPoint(0, 0),
				points = [],
				format = curses.A_NORMAL):
		self.position = position
		self.points = points
		self.format = format
		
		if len(points) == 0:
			points += [WorldPoint(0, 0)]
		

	def rotate(self):
		for point in self.points:
			(point.i, point.j) = (point.j, 1 - point.i)
	
	
	def draw(self, window,
			 drawProp,
			 isGhost = False, position = None):
		
		if position is None:
			position = self.position
		
		text = " "
		format = self.format

		if isGhost:
			text = "#"
			format = format & ~curses.A_REVERSE

		for cell in self.points:
			point = WorldPoint(	cell.i + position.i,
								cell.j + position.j)
			screenPt = drawProp.toScreenPoint(point)
			
			if World.isInWorld(point):
				if (isGhost and drawProp.worldScale.y == 1 and drawProp.worldScale.x == 2):
					window.addstr(screenPt.y, screenPt.x, "[]", format)
				else:
					for y in range(0, drawProp.worldScale.y):
						for x in range(0, drawProp.worldScale.x):
							window.addstr(screenPt.y + y, screenPt.x + x, text, format)



class World:
	WORLD_HEIGHT = 20
	WORLD_WIDTH  = 10
	EMPTY_CELL   = 0

	
	def __init__(self, height = WORLD_HEIGHT, width = WORLD_WIDTH):
		self.data = [[World.EMPTY_CELL
						for i in range (0, World.WORLD_WIDTH)]
							for i in range (0, World.WORLD_HEIGHT)]


	def draw(self, window, drawProp):
		
		for i in range(0, World.WORLD_HEIGHT):
			for j in range(0, World.WORLD_WIDTH):
				screenPt = drawProp.toScreenPoint(WorldPoint(i, j))
			
				format = curses.A_NORMAL
				if self.data[i][j] != World.EMPTY_CELL:
					format = self.data[i][j]
			
				for y in range(0, drawProp.worldScale.y):
					for x in range(0, drawProp.worldScale.x):
						window.addstr(screenPt.y + y, screenPt.x + x, " ", format)
				
				#~ if self.data[i][j] == World.EMPTY_CELL:
					#~ window.addstr(screenPt.y, screenPt.x, "  ")
				#~ else:
					#~ window.addstr(screenPt.y, screenPt.x, "  ", self.data[i][j])
	
	
	def removeFullLines(self):
		i0 = World.WORLD_HEIGHT - 1
		
		for i in range(World.WORLD_HEIGHT - 1, -1, -1):
			if self.data[i].count(World.EMPTY_CELL) != 0:
				for j in range (0, World.WORLD_WIDTH):
					self.data[i0][j] = self.data[i][j]
				i0 -= 1
			
		for i in range(i0, -1, -1):
			for j in range (0, World.WORLD_WIDTH):
				self.data[i][j] = World.EMPTY_CELL
	
	
	@classmethod
	def isInWorld(self, point):
		return (point.i >= 0
			and point.j >= 0
			and point.i < World.WORLD_HEIGHT
			and point.j < World.WORLD_WIDTH)
	
	
	def hasContact(self, figure, figurePos = None):
		if figurePos is None:
			figurePos = figure.position
		
		for cell in figure.points:
			point = WorldPoint(	cell.i + figurePos.i,
								cell.j + figurePos.j)
			
			if point.i >= World.WORLD_HEIGHT:
				return True
			elif (World.isInWorld(point)
				and self.data[point.i][point.j] != World.EMPTY_CELL):
				return True
		
		return False
	
	
	@classmethod	
	def isInBorders(self, figure, figurePos = None):
		if figurePos is None:
			figurePos = figure.position
		
		for cell in figure.points:
			point = WorldPoint(	cell.i + figurePos.i,
								cell.j + figurePos.j)
			
			if (point.j < 0 or point.j >= World.WORLD_WIDTH):
				return False
			
		return True



class WorldWindow:
	WORLD_DISPL_Y = 1
	WORLD_DISPL_X = 1
	
	def __init__(self, stdscr, position = ScreenPoint(1, 2), world = World()):
		self.stdscr = stdscr
		self.position = position
		self.world = world
		worldDispl = ScreenPoint(self.WORLD_DISPL_Y, self.WORLD_DISPL_X)
		worldScale = ScreenPoint(0, 0)
		self.drawProp = DrawProporties(worldDispl, worldScale)
		#~ self.window = stdscr.subwin(World.WORLD_HEIGHT * worldScale.y + worldDispl.y * 2,
									#~ World.WORLD_WIDTH  * worldScale.x + worldDispl.x * 2,
									#~ position.y, position.x)
		#~ self.moveToCenter()
		self.updateDrawProperties()
		
	
	def refresh(self):
		self.window.refresh()
	
	def clear(self):
		self.window.clear()
		
	def draw(self):
		self.window.box()
		self.world.draw(self.window, self.drawProp)
		#~ self.window.refresh()
		
		
	def updateDrawProperties(self):
		maxScaleY = 1 #int(curses.LINES / World.WORLD_HEIGHT)
		maxScaleX = 2 #int(min(curses.COLS / World.WORLD_WIDTH, maxScaleY * 2))
		
		scaleChanged = (self.drawProp.worldScale.y != maxScaleY or
						self.drawProp.worldScale.x != maxScaleX)
		
		if scaleChanged:
			self.window = self.stdscr.subwin(
								World.WORLD_HEIGHT * maxScaleY + self.drawProp.worldDispl.y * 2,
								World.WORLD_WIDTH  * maxScaleX + self.drawProp.worldDispl.x * 2,
								self.position.y, self.position.x)
		
		self.drawProp.worldScale.y = maxScaleY
		self.drawProp.worldScale.x = maxScaleX
		
		#~ if scaleChanged:
		self.moveToCenter()
		
		
		
	def moveToCenter(self):
		# move world window to center
		windowY = int((curses.LINES	- World.WORLD_HEIGHT * self.drawProp.worldScale.y) / 2) - self.WORLD_DISPL_Y
		windowX = int((curses.COLS	- World.WORLD_WIDTH  * self.drawProp.worldScale.x) / 2) - self.WORLD_DISPL_X
		self.window.mvwin(windowY, windowX)
		self.window.refresh()
		

class Game:
	FIGURES = [None for i in range (0, 7)]
	FIGURES[0] = [(0, 0),							##
				  (1, 0), (1, 1), (1, 2)]			######
	FIGURES[1] = [				   (0, 1),			    ##
				  (1, -1), (1, 0), (1, 1)]			######
	FIGURES[2] = [		  (0, 1), (0, 2),			  ####
				  (1, 0), (1, 1)]					####
	FIGURES[3] = [(0, -1), (0, 0),					####
						   (1, 0), (1, 1)]			  ####
	FIGURES[4] = [		  (0, 1),					  ##
				  (1, 0), (1, 1), (1, 2)]			######
	FIGURES[5] = [(1, -1), (1, 0), (1, 1), (1, 2)]	########
	FIGURES[6] = [(0, 0), (0, 1),					####
				  (1, 0), (1, 1)]					####
	
	FIGURE_FORMATS = []
	
	FRAME_DELAY = 0.5
	CONTROL_DELAY = 0.01
	
	
	def __init__(self, stdscr):
		self.stdscr = stdscr
		self.worldWin = WorldWindow(stdscr)
		self.stdscr.refresh()
		self.figure = Figure()
		self.world = self.worldWin.world
		self.worldData = self.world.data
		
		if len(Game.FIGURE_FORMATS) == 0:
			Game.FIGURE_FORMATS = [
				#~ color(curses.COLOR_BLACK, -1)	| curses.A_REVERSE | curses.A_BOLD,
				color(curses.COLOR_CYAN, -1)	| curses.A_REVERSE,
				color(curses.COLOR_BLUE, -1)	| curses.A_REVERSE,
				color(curses.COLOR_MAGENTA, -1)	| curses.A_REVERSE,
				color(curses.COLOR_YELLOW, -1)	| curses.A_REVERSE,
				]
		
	
	def createFigure(self):
		format = Game.FIGURE_FORMATS[
					random.randint(0, len(Game.FIGURE_FORMATS) - 1)]
		#~ position = WorldPoint(-2, World.WORLD_WIDTH / 2)
		position = WorldPoint(-2, World.WORLD_WIDTH / 2 - 1)
		figureNum = random.randint(0, len(Game.FIGURES) - 1)
		points = []
		figureBody = self.FIGURES[figureNum]
		
		for (i, j) in figureBody:
			points.append(WorldPoint(i, j))
		
		return Figure(position, points, format)
	
	
	def landFigure(self):
		result = True
	
		for cell in self.figure.points:
			point = WorldPoint(	cell.i + self.figure.position.i,
								cell.j + self.figure.position.j)
			
			if point.i >= 0:
				self.worldData[point.i][point.j] = self.figure.format
			else:
				result = False
		
		return result
	
	
	def canMoveFrom(self, ghostPos):
		result = (True, True)
		
		for displX in range(-1, 2, 2):
			#~ self.figure.position.j += displX
			ghostPos.j += displX
			
			cantMove = (not World.isInBorders(self.figure, ghostPos)
						or self.world.hasContact(self.figure, ghostPos))
			
			ghostPos.j -= displX
			
			if not cantMove:
				if displX == -1:
					result = (False, result[1])
				else:
					result = (result[0], False)
			
			#~ if (not World.isInBorders(self.figure, ghostPos) or
				#~ self.world.hasContact(self.figure, ghostPos)):
				#~ self.figure.position.j -= displX
				#~ ghostPos.j -= displX
				#~ return False
		
		return result
	
	
	
	def mainLoop(self):
		#creare game components
		self.figure = self.createFigure()
		ghostPos = WorldPoint(self.figure.position.i, self.figure.position.j)
		
		# main loop
		play = True
		isInPause = False
		lastTime = datetime.datetime.now()
		
		while play:
			ch = self.stdscr.getch()
			#~ needRedraw = False
			displX = 0
			rotCount = 0
			downIsPresed = False
			
			# process commands
			while ch != -1:
				if ch == ord('q'):
					play = False
				elif ch == ord(' '):
					isInPause = not isInPause
					lastTime = datetime.datetime.now()
				
				if not isInPause:
					if ch == curses.KEY_LEFT:
						displX = -1
					elif ch == curses.KEY_RIGHT:
						displX = 1
					elif ch == curses.KEY_UP:
						rotCount += 1
					elif ch == curses.KEY_DOWN:
						downIsPresed = True
				
				ch = self.stdscr.getch()
			
			figureIsMoved = displX != 0
			figureIsTurned = rotCount % 4 != 0
			figureIsUpdated = figureIsMoved or figureIsTurned
			
			# move right or left and rotate figure			
			if figureIsMoved:
				self.figure.position.j += displX
				ghostPos.j += displX
				
				if (not World.isInBorders(self.figure) or
					self.world.hasContact(self.figure)):
					self.figure.position.j -= displX
					ghostPos.j -= displX
			
			if figureIsTurned:	
				for i in range(0, rotCount):
					self.figure.rotate()
				
				# rotate back in case of collision
				if (self.world.hasContact(self.figure)
					or not World.isInBorders(self.figure)):
					for i in range(0, 4 - rotCount % 4):
						self.figure.rotate()
			
			# update ghost i-position
			if figureIsUpdated or ghostPos.i == self.figure.position.i:
				ghostPos.i = self.figure.position.i
				ghostPos.j = self.figure.position.j
				canMove = self.canMoveFrom(ghostPos)
				lastCanMove = (canMove[0], canMove[1])
				
				while not self.world.hasContact(self.figure, ghostPos):
						#~ and not (canMove[0] and (not lastCanMove[0])
							  #~ or canMove[1] and (not lastCanMove[1]))):
					ghostPos.i += 1
					lastCanMove = (canMove[0], canMove[1])
					canMove = self.canMoveFrom(ghostPos)
					
				ghostPos.i -= 1
			
			if downIsPresed:
				self.figure.position.i = ghostPos.i
			
			currentTime = datetime.datetime.now()
			dtime = currentTime - lastTime
			delay = dtime.total_seconds()
			
			if (delay >= Game.FRAME_DELAY or downIsPresed) and not isInPause:
				# on new frame
				# move figure down if possible
				if self.figure.position.i == ghostPos.i:
					play = self.landFigure()
					self.world.removeFullLines()
					self.figure = self.createFigure()
					ghostPos = WorldPoint(	self.figure.position.i,
											self.figure.position.j)
				else:
					self.figure.position.i += 1
				
				figureIsUpdated = True
				lastTime = datetime.datetime.now()
			
			# redraw
			self.worldWin.updateDrawProperties()
			#~ self.stdscr.addstr(0, 0, "%d %d" % (curses.COLS, curses.LINES))
			
			if figureIsUpdated:
				self.worldWin.draw()
				self.figure.draw(self.worldWin.window, self.worldWin.drawProp,
								 True, ghostPos)
				self.figure.draw(self.worldWin.window, self.worldWin.drawProp)
				self.worldWin.refresh()
			
			# control frame delay
			time.sleep(Game.CONTROL_DELAY)
		##################################


## main
# SVD decomposition
# Piter Norving - AI

def new_main(stdscr):
	# initialize curses
	stdscr.clear()
	curses.use_default_colors()
	createColors()
	stdscr.nodelay(True)
	curses.curs_set(0)
	
	#~ stdscr.addstr(0, 0, "%d %d" % (curses.COLS, curses.LINES))
	
	game = Game(stdscr)
	game.mainLoop()
	
	stdscr.refresh()
	stdscr.nodelay(False)
	#~ stdscr.getch()
	#########################

## functions
def color(i, j):
    return curses.color_pair(colorNum(i, j))


def colorNum(i, j):
    if i == -1 and j == -1:
        return 0
    else:
        return (i + 1) * (curses.COLORS + 1) + (j + 1)


def createColors():
    for i in range(-1, curses.COLORS):
        for j in range(-1, curses.COLORS):
            curses.init_pair(colorNum(i, j), i, j)

## start
wrapper(new_main)
