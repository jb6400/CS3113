#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0, BG_BLUE = 0.302f, BG_GREEN = 0;
const float BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex.glsl",
F_SHADER_PATH[] = "shaders/fragment.glsl";

float SCALE_FACTOR = 1.001f;
const glm::mat4 MIN_SIZE = glm::mat4(1.0f);
const glm::mat4 MAX_SIZE = glm::mat4(4.0f);
glm::mat4 GOAL_SIZE = MAX_SIZE;

const float ROT_ANGLE = glm::radians(1.5f);
const float INIT_TRIANGLE_ANGLE = glm::radians(45.0); //always radians

SDL_Window* display_window;
bool game_is_running = true;
bool ball_moving = false;
bool is_game_over = false;

ShaderProgram program;
glm::mat4 view_matrix, model_matrix_p1, model_matrix_p2, 
		  model_matrix_b, model_matrix_border, projection_matrix;

glm::vec3 player1_position = glm::vec3(0, 0, 0);
glm::vec3 player2_position = glm::vec3(0, 0, 0);
glm::vec3 ball_position = glm::vec3(0, 0, 0);

glm::vec3 player1_movement = glm::vec3(0, 0, 0);
glm::vec3 player2_movement = glm::vec3(0, 0, 0);
glm::vec3 ball_movement = glm::vec3(0, 0, 0);

float previous_ticks = 0.f;

const float player_speed = 2.5f;
const float MINIMUM_COLLISION_DISTANCE_BALL = .4f;
const float MINIMUM_COLLISION_DISTANCE_PLAYER = 1.f;

float BALL_X_TRANS = .5f;
float BALL_Y_TRANS = 1.f;

void initialise() {
	SDL_Init(SDL_INIT_VIDEO);
	display_window = SDL_CreateWindow("Project 2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(display_window);
	SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT); // Init camera
	program.Load(V_SHADER_PATH, F_SHADER_PATH); // Loads up shaders

	// Init view, model, and proj. matricies
	view_matrix = glm::mat4(1.f); // Identify 4x matrix

	model_matrix_p1 = glm::mat4(1.f); // Identify 4x matrix
	model_matrix_p2 = glm::mat4(1.f); // Identify 4x matrix
	model_matrix_b = glm::mat4(1.f); // Identify 4x matrix
	model_matrix_border = glm::mat4(1.f); // Identify 4x matrix

	projection_matrix = glm::ortho(-5.f, 5.f, -3.75f, 3.75f, -1.f, 1.f);
	//projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
	//Camera looks perp on triangle

	//Load them onto OpenGL
	program.SetViewMatrix(view_matrix);
	program.SetProjectionMatrix(projection_matrix);

	//Set color of the triagle
	program.SetColor(1.0f, 0.44f, 0.38f, 1.0f);

	glUseProgram(program.programID);

	glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
}
void process_input() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			game_is_running = false;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
				if (!is_game_over) {
					case SDLK_w:
						player1_movement.y += 1;
						break;
					case SDLK_s:
						player1_movement.y -= 1;
						break;
					case SDLK_UP:
						player2_movement.y += 1;
						break;
					case SDLK_DOWN:
						player2_movement.y -= 1;
						break;
				}
				case SDLK_SPACE:
					ball_moving = true;
					if (is_game_over) {
						ball_position = glm::vec3(0, 0, 0);
						BALL_X_TRANS = .5f;
						BALL_Y_TRANS = 1.f;
						is_game_over = false;
					}
					break;
				default:
					break;
			}

		default:
			break;
		}
	}

	const Uint8* key_state = SDL_GetKeyboardState(NULL);

	if (!is_game_over) {
		if (key_state[SDL_SCANCODE_W]) {
			player1_movement.y = 1.f;
		}
		else if (key_state[SDL_SCANCODE_S]) {
			player1_movement.y = -1.f;
		}

		if (key_state[SDL_SCANCODE_UP]) {
			player2_movement.y = 1.f;
		}
		else if (key_state[SDL_SCANCODE_DOWN]) {
			player2_movement.y = -1.f;
		}
	}
}
bool check_collision(glm::vec3& position_1, glm::vec3& position_2, const float& collsion_dist) {
	return sqrt(pow(position_2[0] - position_1[0], 2) + 
		   pow(position_2[1] - position_1[1], 2)) < collsion_dist;
}
bool check_collision2(glm::vec3& position_1, glm::vec3& position_2, const float& collsion_dist) {
	_RPTF2(_CRT_WARN, "check_collision2 returns x: %f\n", sqrt(pow(position_2[0] - position_1[0], 2)));
	_RPTF2(_CRT_WARN, "check_collision2 returns y: %f\n", sqrt(pow(position_2[1] - position_1[1], 2)));
	return sqrt(pow(position_2[0] - position_1[0], 2) +
		pow(position_2[1] - position_1[1], 2)) < collsion_dist;
	//return (sqrt(pow(position_2[0] - position_1[0], 2)) < MINIMUM_COLLISION_DISTANCE) ||
	//	(sqrt(pow(position_2[1] - position_1[1], 2)) < MINIMUM_COLLISION_DISTANCE);
}
void update() {
	//for delta time
	float ticks = (float)SDL_GetTicks() / 1000.f; // get the current number of ticks
	float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
	previous_ticks = ticks;

	glm::vec3 top_border_vec = glm::vec3(0, 3.625f, 0);
	glm::vec3 bot_border_vec = glm::vec3(0, -3.625f, 0);

	glm::vec3 right_border_vec = glm::vec3(4.875f, 0, 0);
	glm::vec3 left_border_vec = glm::vec3(-4.875f, 0, 0);

	glm::vec3 player1_global_pos = glm::vec3(-3.5, player1_position.y, 0);
	glm::vec3 player2_global_pos = glm::vec3(3.5, player2_position.y, 0);

	if(ball_moving == true){

		glm::vec3 b_m_x(ball_position.x, 0, 0);
		glm::vec3 b_m_y(0, ball_position.y, 0);

		if (check_collision(b_m_y, top_border_vec, MINIMUM_COLLISION_DISTANCE_BALL) ||
			check_collision(b_m_y, bot_border_vec, MINIMUM_COLLISION_DISTANCE_BALL))
		{
			BALL_Y_TRANS = -1.f * BALL_Y_TRANS;
		}

		if (check_collision(b_m_x,right_border_vec, MINIMUM_COLLISION_DISTANCE_BALL) ||
			check_collision(b_m_x, left_border_vec, MINIMUM_COLLISION_DISTANCE_BALL))
		{
			//add lose condition
			//BALL_X_TRANS = -1.f * BALL_X_TRANS;
			BALL_X_TRANS = 0.f;
			BALL_Y_TRANS = 0.f;
			is_game_over = true;
		}

		if (check_collision(ball_position, player1_global_pos, MINIMUM_COLLISION_DISTANCE_BALL) ||
			check_collision(ball_position, player2_global_pos, MINIMUM_COLLISION_DISTANCE_BALL)) {
			BALL_X_TRANS = -1.f * BALL_X_TRANS;
			BALL_Y_TRANS = -1.f * BALL_Y_TRANS;
		}

		ball_movement = glm::vec3(BALL_X_TRANS, BALL_Y_TRANS, 0);

		if (glm::length(ball_movement) > 1.0f)
		{
			ball_movement = glm::normalize(ball_movement);
		}

		ball_position += ball_movement * player_speed * delta_time;
		//_RPTF2(_CRT_WARN, "ball_position: %f, %f\n", ball_position.x, ball_position.y);
		model_matrix_b = glm::mat4(1.0f);
		model_matrix_b = glm::translate(model_matrix_b, ball_position);
	}

	//player

	if (check_collision(player1_position, top_border_vec, MINIMUM_COLLISION_DISTANCE_PLAYER) ||
		check_collision(player1_position, bot_border_vec, MINIMUM_COLLISION_DISTANCE_PLAYER))
	{
		player1_movement.y = -80.f * player1_movement.y;
	}

	if (check_collision(player2_position, top_border_vec, MINIMUM_COLLISION_DISTANCE_PLAYER) ||
		check_collision(player2_position, bot_border_vec, MINIMUM_COLLISION_DISTANCE_PLAYER))
	{
		player2_movement.y = -80.f * player2_movement.y;
	}

	player1_position += player1_movement * player_speed * delta_time;
	player2_position += player2_movement * player_speed * delta_time;

	model_matrix_p1 = glm::mat4(1.0f);
	model_matrix_p1 = glm::translate(model_matrix_p1, player1_position);

	model_matrix_p2 = glm::mat4(1.0f);
	model_matrix_p2 = glm::translate(model_matrix_p2, player2_position);

	player1_movement = glm::vec3(0, 0, 0);
	player2_movement = glm::vec3(0, 0, 0); 

}
void render() {
	//STEP 1
	glClear(GL_COLOR_BUFFER_BIT);
	//STEP 2
	program.SetModelMatrix(model_matrix_p1);
	//STEP 3

	//player 1 sprite
	float vertices_p1[] = 
	{ 
		-4.f, -1.f, -3.5f, -1.f, -3.5f, 1.f, //triangle1
		-4.f, -1.f, -3.5f, 1.f, -4.f, 1.f   //triangle2
	};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_p1);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	program.SetModelMatrix(model_matrix_p2);

	//player 2 sprite
	float vertices_p2[] =
	{
		4.f, -1.f, 3.5f, -1.f, 3.5f, 1.f, //triangle1
		4.f, -1.f, 3.5f, 1.f, 4.f, 1.f   //triangle2
	};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_p2);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	//borders
	program.SetModelMatrix(model_matrix_border);

	float vertices_b_top[] = 
	{
		-5.f, 3.5f, 5.f, 3.5f, 5.f, 3.75f, //triangle1
		-5.f, 3.5f, 5.f, 3.75f, -5.f, 3.75f   //triangle2
	};
	float vertices_b_right[] = 
	{
		4.75f, -3.5f, 5.f, -3.5f, 5.f, 3.5f, //triangle1
		4.75f, -3.5f, 5.f, 3.5f, 4.75f, 3.5f   //triangle2
	};
	float vertices_b_bot[] =
	{
		-5.f, -3.5f, 5.f, -3.5f, 5.f, -3.75f, //triangle1
		-5.f, -3.5f, 5.f, -3.75f, -5.f, -3.75f   //triangle2
	};
	float vertices_b_left[] =
	{
		-4.75f, -3.5f, -5.f, -3.5f, -5.f, 3.5f, //triangle1
		-4.75f, -3.5f, -5.f, 3.5f, -4.75f, 3.5f   //triangle2
	};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_b_top);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_b_right);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_b_bot);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_b_left);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//ball

	program.SetModelMatrix(model_matrix_b);

	float vertices_b[] =
	{
		-.25f, -.25f, .25f, -.25f, .25f, .25f,  //triangle1
		-.25f, -.25f, .25f, .25f, -.25f, .25f   //triangle2
	};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_b);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);


	glDisableVertexAttribArray(program.positionAttribute);
	//STEP 4
	SDL_GL_SwapWindow(display_window);
}
void shutdown() { SDL_Quit(); }

int main(int argc, char* argv[]) {
	//STEP 1: Get everything set up and ready
	initialise();

	//STEP 2: Game loop
	while (game_is_running) {
		//STEP 3: Check for any input from the user
		process_input();
		//STEP 4: Update our scene from user input
		update();
		//STEP 5: Get those updates onto current screen
		render();
	}

	//STEP 6: Shut engine down
	shutdown();
	return 0;
}
