import random
import socket
import pygame, sys
import ball as b
from pygame.locals import *
import struct
import json
import threading
import atexit

pygame.init()
fps = pygame.time.Clock()

#colors
WHITE = (255,255,255)
RED = (255,0,0)
GREEN = (0,255,0)
BLACK = (0,0,0)

#globals
WIDTH = 600
HEIGHT = 400       
BALL_RADIUS = 20
PAD_WIDTH = 8
PAD_HEIGHT = 80
HALF_PAD_WIDTH = PAD_WIDTH / 2
HALF_PAD_HEIGHT = PAD_HEIGHT / 2
ball_pos = [0,0]
ball_vel = [0,0]
local_paddle_vel = 0
enemy_paddle_vel = 0
KEYDOWN_WAS_ENTER = False
THREADS_ARE_RUNNING = True

#canvas declaration
window = pygame.display.set_mode((WIDTH, HEIGHT), 0, 32)
pygame.display.set_caption('Hello World')


server_response_event = threading.Event()
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('localhost', 8080)


def close_threads():
    global THREADS_ARE_RUNNING
    THREADS_ARE_RUNNING = False
    client_socket.close()
    print('cerrando threads')
    sys.exit()

atexit.register(close_threads)

def ball_init(ball_vel_init):
    global ball_pos, ball_vel # these are vectors stored as lists
    ball_pos = [WIDTH/2,HEIGHT/2]
    ball_vel = ball_vel_init

def receive_data_from_server():
    global enemy_paddle_pos, THREADS_ARE_RUNNING
    while THREADS_ARE_RUNNING:
        response = client_socket.recv(1024)
        decoded_response = response.decode('utf-8')  # Decodificar a cadena UTF-8
        response_dict = json.loads(decoded_response)
        print('receive_Data_from_Server - response:', response)
        print('receive_Data_from_Server - type:', response_dict['type'])
        
        if response_dict['type'] == 'ball_start':
            ball_init([response_dict["data"][0], response_dict["data"][1]])  # horizontal, vertical
        elif response_dict['type'] == 'paddle_move':
            enemy_paddle_pos = [response_dict["data"][0], response_dict["data"][1]]  # x, y
        elif response_dict['type'] == 'point_made':
            ball_init([response_dict["data"][0], response_dict["data"][1]])  # horizontal, vertical
        server_response_event.set()


# define event handlers
def init():
    global local_paddle_pos, enemy_paddle_pos, local_paddle_vel, enemy_paddle_vel
    local_paddle_pos = [HALF_PAD_WIDTH - 1,HEIGHT/2]
    enemy_paddle_pos = [WIDTH +1 - HALF_PAD_WIDTH,HEIGHT/2]
    

#keydown handler
def keydown(event):
    global local_paddle_vel, enemy_paddle_vel
    
    if event.key == K_UP:
        enemy_paddle_vel = -8
    elif event.key == K_DOWN:
        enemy_paddle_vel = 8
    elif event.key == K_w:
        local_paddle_vel = -8
    elif event.key == K_s:
        local_paddle_vel = 8

#keyup handler
def keyup(event):
    global local_paddle_vel, enemy_paddle_vel
    
    if event.key in (K_w, K_s):
        local_paddle_vel = 0
    elif event.key in (K_UP, K_DOWN):
        enemy_paddle_vel = 0

def game():
    global local_paddle_pos, enemy_paddle_pos, ball_pos, ball_vel, KEYDOWN_WAS_ENTER, THREADS_ARE_RUNNING
    #game loop
    while THREADS_ARE_RUNNING:

        window.fill(BLACK)

        # update paddle's vertical position, keep paddle on the screen
        if local_paddle_pos[1] > HALF_PAD_HEIGHT and local_paddle_pos[1] < HEIGHT - HALF_PAD_HEIGHT:
            local_paddle_pos[1] += local_paddle_vel
        elif local_paddle_pos[1] == HALF_PAD_HEIGHT and local_paddle_vel > 0:
            local_paddle_pos[1] += local_paddle_vel
        elif local_paddle_pos[1] == HEIGHT - HALF_PAD_HEIGHT and local_paddle_vel < 0:
            local_paddle_pos[1] += local_paddle_vel

        if(KEYDOWN_WAS_ENTER):
            message_dict = {
                'type': 'paddle_move',
                'data': [enemy_paddle_pos[0], local_paddle_pos[1]]
            }
            client_socket.send(json.dumps(message_dict).encode('utf-8'))
            KEYDOWN_WAS_ENTER = False
        #update ball
        ball_pos[0] += int(ball_vel[0])
        ball_pos[1] += int(ball_vel[1])

        #draw paddles and ball
        pygame.draw.circle(window, WHITE, ball_pos, 20, 0)
        pygame.draw.polygon(window, WHITE, [[local_paddle_pos[0] - HALF_PAD_WIDTH, local_paddle_pos[1] - HALF_PAD_HEIGHT], [local_paddle_pos[0] - HALF_PAD_WIDTH, local_paddle_pos[1] + HALF_PAD_HEIGHT], [local_paddle_pos[0] + HALF_PAD_WIDTH, local_paddle_pos[1] + HALF_PAD_HEIGHT], [local_paddle_pos[0] + HALF_PAD_WIDTH, local_paddle_pos[1] - HALF_PAD_HEIGHT]], 0)
        pygame.draw.polygon(window, WHITE, [[enemy_paddle_pos[0] - HALF_PAD_WIDTH, enemy_paddle_pos[1] - HALF_PAD_HEIGHT], [enemy_paddle_pos[0] - HALF_PAD_WIDTH, enemy_paddle_pos[1] + HALF_PAD_HEIGHT], [enemy_paddle_pos[0] + HALF_PAD_WIDTH, enemy_paddle_pos[1] + HALF_PAD_HEIGHT], [enemy_paddle_pos[0] + HALF_PAD_WIDTH, enemy_paddle_pos[1] - HALF_PAD_HEIGHT]], 0)

        #ball collision check on top and bottom walls
        if int(ball_pos[1]) <= BALL_RADIUS:
            ball_vel[1] = - ball_vel[1]
        if int(ball_pos[1]) >= HEIGHT + 1 - BALL_RADIUS:
            ball_vel[1] = -ball_vel[1]

        
        #ball collison check on gutters or paddles
        if int(ball_pos[0]) <= BALL_RADIUS + PAD_WIDTH and int(ball_pos[1]) in range(int(local_paddle_pos[1] - HALF_PAD_HEIGHT),int(local_paddle_pos[1] + HALF_PAD_HEIGHT),1):
            ball_vel[0] = -ball_vel[0]
            ball_vel[0] *= 1.1
            ball_vel[1] *= 1.1
        elif int(ball_pos[0]) <= BALL_RADIUS + PAD_WIDTH:
            message_dict = {
            'type': 'point_made',
            }
            client_socket.send(json.dumps(message_dict).encode('utf-8'))
            server_response_event.wait()
            server_response_event.clear() # reset even

        for event in pygame.event.get():

            if event.type == KEYDOWN:
                KEYDOWN_WAS_ENTER = True
                keydown(event)
            elif event.type == KEYUP:
                keyup(event)
            elif event.type == QUIT:
                pygame.quit()
                sys.exit()
                
        pygame.display.update()
        fps.tick(60)
    
init()

try:    

    receive_data_thread = threading.Thread(target=receive_data_from_server)
    receive_data_thread.daemon = True



    client_socket.connect(server_address)
    receive_data_thread.start()
    server_response_event.wait()
    server_response_event.clear() # reset event     
    print('connected')
    game()
except Exception as e:
    print('ERRORRR -> ', e)
    close_threads()


print('termino')

