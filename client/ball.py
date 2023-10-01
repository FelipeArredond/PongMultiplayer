class Ball:
    
    def __init__(self) -> None:
        pass
    
    @staticmethod
    def ball_update(ball_pos, ball_vel):
        return int(ball_pos) + int(ball_vel)
    
    @staticmethod
    def ball_collision(ball_pos, ball_vel, BALL_RADIUS, HEIGHT):
        if int(ball_pos[1]) <= BALL_RADIUS:
            return  - ball_vel[1]
        if int(ball_pos[1]) >= HEIGHT + 1 - BALL_RADIUS:
            return  -ball_vel[1]