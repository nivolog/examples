import numpy as np
from numpy.linalg import norm
import pygame, math
from pygame import *
from math import *

import matplotlib.pyplot as plt
import random
import time


HOR_AMOUNT = 6
VER_AMOUNT = 6
dt = 0.01
SIZE = 3
E = 4
O = 5

class molecula():
	def __init__(self, x, y, vx, vy):
		self.x = x
		self.y = y
		self.vx = vx
		self.vy = vy
		self.ax = 0
		self.ay = 0
		self.new_ax = 0
		self.new_ay = 0
	
	#calculation functions
	
	#position
	def calculate(self, dt):
		self.x += self.vx*dt + self.ax*0.5*dt*dt
		self.y += self.vy*dt + self.ay*0.5*dt*dt
	
	#velocity and changing acceleration
	def calculate_v(self):
		self.vx += (self.ax+self.new_ax)*dt*0.5
		self.vy += (self.ay+self.new_ay)*dt*0.5
		self.ax = self.new_ax
		self.ay = self.new_ay
		self.new_ax = 0
		self.new_ay = 0
		
	#acceleration
	def calculate_ac(self, other):
		r = self.R(other)
		if r < 2*SIZE:
			r = 2*SIZE
		dx = other.x-self.x
		dy = other.y-self.y
		self.new_ax += E*(dx/r)*(6*((O/r)**6)/r - 12*((O/r)**12)/r)
		self.new_ay += E*(dy/r)*(6*((O/r)**6)/r - 12*((O/r)**12)/r)
		
	#range between two moleculas
	def R(self, other):
		return sqrt((self.x-other.x)**2 + (self.y-other.y)**2)
	
	def potential(self, other):
		r = self.R(other)
		if r < 2*SIZE:
			r = 2*SIZE
		dx = other.x-self.x
		dy = other.y-self.y
		return E*((O/r)**12  - (O/r)**6)
		
	
	def collision_react(self, second):
		v1 = np.array([self.vx, self.vy])
		v2 = np.array([second.vx, second.vy])
		r = np.array([self.x-second.x, self.y-second.y])
		modr = norm(r, ord=2)
		modv1 = norm(v1, ord=2)
		modv2 = norm(v2, ord=2)
		v1r =np.dot(v1,r)*r/modr 
		v2r =np.dot(v2,r)*r/modr 
		v1l = v1-v1r
		v2l = v2-v2r
		a = v2r
		v2r = v1r
		v1r = a
		v1 = v1r+v1l
		v2 = v2r+v2l
		self.vx = v1[0]
		self.vy = v1[1]
		second.vx = v2[0]
		second.vy = v2[1]

SCREEN_WIDTH = 800
SCREEN_HEIGHT = 800
DISPLAY = (SCREEN_WIDTH, SCREEN_HEIGHT)
VMAX = 800

def main():
	
	#init moleculas array
	moleculas = []
	i = 0
	j = 0
	POSITION = np.empty((VER_AMOUNT, HOR_AMOUNT,2))
	
	#grid positioning 
	for i in range(VER_AMOUNT):
		for j in range(HOR_AMOUNT):
			POSITION[i,j,0] = ((j+1)*SCREEN_WIDTH/(2*HOR_AMOUNT+1))
			POSITION[i,j,1] = ((i+1)*SCREEN_HEIGHT/(VER_AMOUNT+1))
	for i in range(VER_AMOUNT):
		for j in range(HOR_AMOUNT):
			moleculas.append(molecula(POSITION[i,j,0], POSITION[i,j,1] ,VMAX*(random.random()-0.5), VMAX*(random.random()-0.5)))
	
	#PyGame init
	pygame.init()
	screen = pygame.display.set_mode(DISPLAY)
	pygame.display.set_caption("Gas")
	
	#bg init
	bg = Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
	bg.fill(Color("black"))
	screen_rect = pygame.Rect((0,0), (SCREEN_WIDTH, SCREEN_HEIGHT))
	
	#timer init
	timer = pygame.time.Clock()
	
	#main bool
	done = False
	
	#Surface with molecula
	Size_of_surf = (2*SIZE, 2*SIZE)
	MOLSURF = Surface(Size_of_surf)			#TODO: WHY IT DOESN'T WORK WITH GLOBAL VARIABLES
	MOLSURF.fill(Color("black"))
	draw.circle(MOLSURF, Color("yellow"), (SIZE, SIZE), SIZE)
	
	#init lists with data
	
	#list with nums of moleculas
	Dots_on_left = []
	Dots_on_left.append(VER_AMOUNT*HOR_AMOUNT)
	
	#list with kinetic energy
	K_Energy = []
	Cur_K_Energy = 0
	for i in range(len(moleculas)):
		Cur_K_Energy += ((moleculas[i].vx)**2 + (moleculas[i].vy)**2)*5*(10**(-6))/3
	K_Energy.append(Cur_K_Energy)
	
	#list with potential energy
	P_Energy = []
	Cur_P_Energy = 0
	for i in range(len(moleculas)):
		for j in range(len(moleculas)):
			#if i != j:
			Cur_P_Energy = moleculas[i].potential(moleculas[j])/1000000
	P_Energy.append(Cur_P_Energy)
	
	
	while not done:
		timer.tick(dt*1000)
		
		for event in pygame.event.get():
			if event.type == QUIT:
				done = True
				break
			if event.type == KEYDOWN:
				if event.key == K_ESCAPE:
					done = True
					break
		#clearing window
		screen.blit(bg, (0,0))
		
		#calculation, drawing and check collisions
		for i in range(len(moleculas)):
			
			#calculation all the staff
			moleculas[i].calculate(dt)
			moleculas[i].calculate_v()
			
			
			
			#drawing
			screen.blit(MOLSURF, (int(moleculas[i].x - SIZE), int(moleculas[i].y - SIZE)))
			
			#checking collisions
			
			#with walls
			actuall_rect = pygame.Rect((moleculas[i].x - SIZE, moleculas[i].y-SIZE),(2*SIZE, 2*SIZE))
			if not screen_rect.contains(actuall_rect):
				if actuall_rect.x < 0 or actuall_rect.x+2*SIZE > SCREEN_WIDTH:
					moleculas[i].vx = -moleculas[i].vx
				if actuall_rect.y < 0 or actuall_rect.y+2*SIZE > SCREEN_HEIGHT:
					moleculas[i].vy = -moleculas[i].vy
			
			#with other moleculas
			#for j in range(len(moleculas)):
				
				#hit_rect = pygame.Rect((moleculas[i].x - 2*SIZE, moleculas[i].y-2*SIZE),(4*SIZE, 4*SIZE))
				#if i != j and hit_rect.collidepoint(moleculas[j].x, moleculas[j].y):		#moleculas[i].gonna_collide(hit_rect, moleculas[j].x, moleculas[j].y):
					#if moleculas[i].R(moleculas[j]) < 2*SIZE:
						#moleculas[i].collision_react(moleculas[j])
		#calculation acceleration
		for i in range(len(moleculas)):
			for j in range(len(moleculas)):
				if i != j:
					moleculas[i].calculate_ac(moleculas[j])
		#different lists
		
		#list with numbers of moleculas on the left side of the box
		left_num = 0
		for i in range(len(moleculas)):
			if moleculas[i].x < 400:
				left_num += 1
		Dots_on_left.append(left_num)
		
		#list with kinetic energy
		Cur_K_Energy = 0
		for i in range(len(moleculas)):
			Cur_K_Energy += ((moleculas[i].vx)**2 + (moleculas[i].vy)**2)*5*(10**(-6))/3
		K_Energy.append(Cur_K_Energy)
		
		#list with potential energy
		Cur_P_Energy = 0
		for i in range(len(moleculas)):
			for j in range(len(moleculas)):
				#if j > i:
				Cur_P_Energy += moleculas[i].potential(moleculas[j])/1000000
		P_Energy.append(Cur_P_Energy)
		
		#END OF DISPLAYING
		pygame.display.update()
		
		
		
	x = np.linspace(0, (len(Dots_on_left)-1)*dt, len(Dots_on_left)) #.reshape((1, len(Dots_on_left)))
	
	fig = plt.figure()
	
	plt.subplot(221)
	plt.plot(x, K_Energy)
	
	plt.subplot(222)
	plt.plot(x,Dots_on_left)
	
	plt.subplot(223)
	plt.plot(x, P_Energy)
	
	K = np.array(K_Energy)
	P = np.array(P_Energy)
	F_Energy = P+K
	
	plt.subplot(224)
	plt.plot(x, F_Energy)
	plt.show()
	











if __name__ == "__main__":
    main()
