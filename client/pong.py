import socket
import pygame, sys
from pygame.locals import *
import threading
import atexit
import re
import time

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
ball_vel = [1.5,1.5]
local_paddle_pos = [HALF_PAD_WIDTH - 1, HEIGHT/2]
enemy_paddle_pos = [WIDTH +1 - HALF_PAD_WIDTH,HEIGHT/2]
local_paddle_vel = 0
enemy_paddle_vel = 0
player_score = -1
opponent_score = 0
point_was_made = False
KEYDOWN_WAS_ENTER = False
THREADS_ARE_RUNNING = True
FONT = pygame.font.SysFont("Consolas", int(WIDTH/20))
player_name = ''
opponent_name = ''

#canvas declaration
window = pygame.display.set_mode((WIDTH, HEIGHT), 0, 32)
pygame.display.set_caption('Pong Game')

server_response_event = threading.Event()
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('54.174.122.196', 8080)
#server_address = ('localhost', 8080)

def close_threads():
    global THREADS_ARE_RUNNING
    THREADS_ARE_RUNNING = False
    client_socket.close()

atexit.register(close_threads)

def ball_init(ball_vel_init):
    global ball_pos, ball_vel # these are vectors stored as lists
    ball_pos = [WIDTH/2,HEIGHT/2]
    ball_vel = ball_vel_init


def receive_data_from_server():
    global enemy_paddle_pos, THREADS_ARE_RUNNING, player_score, point_was_made,player_name, opponent_name, FONT
    while THREADS_ARE_RUNNING:
        response = client_socket.recv(24)
        if not response:
            print('Server closed connection')
            close_threads()
            break
        dataDecoded = response.decode()
        print(f'Data recieved: {dataDecoded}')
        dataDecoded = dataDecoded.split('|')
        for data in dataDecoded:
            print(f'Data: {data}')
            try:    
                type, x, y = data.split(';')
                if type == 'bs': #ball_start
                    if not point_was_made:
                        player_score += 1
                    point_was_made = False
                    print(f'X: {x}')
                    print(f'Y: {y}')
                    ball_init([float(x), float(y)])  # horizontal, vertical
                    server_response_event.set()
                elif type == 'pmv': #paddle_move
                    y = re.sub(r'[a-zA-Z]', '', y)
                    print(f'Enemy Y: {y}')
                    enemy_paddle_pos = [float(x), float(y)]  # x, y
                elif type == 'fg': # finish game
                    print('Game finished')
                    close_threads()
                    winner_name = player_name
                    loser_name = opponent_name
                    if(player_score < opponent_score):
                        winner_name = opponent_name
                        loser_name = player_name
                    print(f'Winner: {winner_name}')
                    print(f'Loser: {loser_name}')
                    window.fill(BLACK)
                    window.blit(FONT.render(f'El ganador es {winner_name}.', True, "green"), (WIDTH/2-300, HEIGHT/2-50))
                    window.blit(FONT.render(f'El perdedor es {loser_name}.', True, "red"), (WIDTH/2-300, HEIGHT/2))
                    pygame.display.update()
                    time.sleep(10)
                    break
            except Exception as e:
                print(f'[ERROR] -> {e}')

#keydown handler
def keydown(event):
    global local_paddle_vel, enemy_paddle_vel
    if event.key == K_w:
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
    global ball_pos, ball_vel, KEYDOWN_WAS_ENTER, THREADS_ARE_RUNNING, player_score, opponent_score, point_was_made, player_name, opponent_name

    receive_data_thread = threading.Thread(target=receive_data_from_server)

    client_socket.connect(server_address)
    player_name = input("Introduce un nombre de usuario de hasta 24 caracteres: ")
    if len(player_name) > 24:
        player_name = player_name[:24]
    dataEncoded = f'{player_name}'.encode()
    client_socket.send(dataEncoded)
    opponent_name = client_socket.recv(24).decode()
    receive_data_thread.start()
    server_response_event.wait()
    server_response_event.clear()

    while THREADS_ARE_RUNNING:

        window.fill(BLACK)
        # update paddle's vertical position, keep paddle on the screen
        if local_paddle_pos[1] > HALF_PAD_HEIGHT and local_paddle_pos[1] < HEIGHT - HALF_PAD_HEIGHT:
            local_paddle_pos[1] += local_paddle_vel
        elif local_paddle_pos[1] == HALF_PAD_HEIGHT and local_paddle_vel > 0:
            local_paddle_pos[1] += local_paddle_vel
        elif local_paddle_pos[1] == HEIGHT - HALF_PAD_HEIGHT and local_paddle_vel < 0:
            local_paddle_pos[1] += local_paddle_vel
        else:
            if KEYDOWN_WAS_ENTER:
                dataEncoded = f'pmv;{enemy_paddle_pos[0]};{local_paddle_pos[1]}|'.encode()
                client_socket.send(dataEncoded)
            KEYDOWN_WAS_ENTER = False
 
    
        #update ball
        ball_pos[0] += int(ball_vel[0])
        ball_pos[1] += int(ball_vel[1])

        #draw paddles and ball
        pygame.draw.circle(window, WHITE, ball_pos, 20, 0)
        pygame.draw.polygon(window, WHITE, [[local_paddle_pos[0] - HALF_PAD_WIDTH, local_paddle_pos[1] - HALF_PAD_HEIGHT], [local_paddle_pos[0] - HALF_PAD_WIDTH, local_paddle_pos[1] + HALF_PAD_HEIGHT], [local_paddle_pos[0] + HALF_PAD_WIDTH, local_paddle_pos[1] + HALF_PAD_HEIGHT], [local_paddle_pos[0] + HALF_PAD_WIDTH, local_paddle_pos[1] - HALF_PAD_HEIGHT]], 0)
        pygame.draw.polygon(window, WHITE, [[enemy_paddle_pos[0] - HALF_PAD_WIDTH, enemy_paddle_pos[1] - HALF_PAD_HEIGHT], [enemy_paddle_pos[0] - HALF_PAD_WIDTH, enemy_paddle_pos[1] + HALF_PAD_HEIGHT], [enemy_paddle_pos[0] + HALF_PAD_WIDTH, enemy_paddle_pos[1] + HALF_PAD_HEIGHT], [enemy_paddle_pos[0] + HALF_PAD_WIDTH, enemy_paddle_pos[1] - HALF_PAD_HEIGHT]], 0)
        player_score_text = FONT.render(str(player_score), True, "white")
        opponent_score_text = FONT.render(str(opponent_score), True, "white")
        window.blit(player_score_text, (WIDTH/2-50, 50))
        window.blit(opponent_score_text, (WIDTH/2+50, 50))
        
        #ball collision check on top and bottom walls
        if int(ball_pos[1]) <= BALL_RADIUS:
            ball_vel[1] = - ball_vel[1]
        if int(ball_pos[1]) >= HEIGHT + 1 - BALL_RADIUS:
            ball_vel[1] = -ball_vel[1]

        
        #ball collison check on gutters or paddles
        if int(ball_pos[0]) <= BALL_RADIUS + PAD_WIDTH and int(ball_pos[1]) in range(int(local_paddle_pos[1] - HALF_PAD_HEIGHT),int(local_paddle_pos[1] + HALF_PAD_HEIGHT + 1),1):
            ball_vel[0] = -ball_vel[0]
            ball_vel[0] *= 1.1
            ball_vel[1] *= 1.1
        elif int(ball_pos[0]) <= BALL_RADIUS + PAD_WIDTH:
            dataEncoded = b'pmd'
            opponent_score += 1
            point_was_made = True
            ball_pos = [WIDTH/2,HEIGHT/2]
            client_socket.send(dataEncoded)
            server_response_event.wait()
            server_response_event.clear()
            KEYDOWN_WAS_ENTER = False
            
        if int(ball_pos[0]) >= WIDTH + 1 - BALL_RADIUS - PAD_WIDTH and int(ball_pos[1]) in range(int(enemy_paddle_pos[1] - HALF_PAD_HEIGHT), int(enemy_paddle_pos[1] + HALF_PAD_HEIGHT + 1),1):
            ball_vel[0] = -ball_vel[0]
            ball_vel[0] *= 1.1
            ball_vel[1] *= 1.1
        elif int(ball_pos[0]) >= WIDTH + 1 - BALL_RADIUS - PAD_WIDTH:
            ball_vel[0] *= 1.1
            ball_vel[1] *= 1.1
            ball_vel[0] = -ball_vel[0]

        if KEYDOWN_WAS_ENTER:
            dataEncoded = f'pmv;{enemy_paddle_pos[0]};{local_paddle_pos[1]}|'.encode()
            client_socket.send(dataEncoded)

        for event in pygame.event.get():

            if event.type == KEYDOWN:
                KEYDOWN_WAS_ENTER = True
                keydown(event)
            elif event.type == KEYUP:
                KEYDOWN_WAS_ENTER = False
                keyup(event)
            elif event.type == QUIT:
                pygame.quit()
                sys.exit()
                
        pygame.display.update()
        fps.tick(30)

if __name__ == '__main__':
    try:    
        game()
    except ConnectionRefusedError as e:
        print(f'Connection to Server {server_address} didn\'t work')
        close_threads()
    except Exception as e:
        print(f'Client error ocurred.\n{e}')
        close_threads()
