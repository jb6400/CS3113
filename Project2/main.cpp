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

const float BG_RED = 0.62f, BG_BLUE = 0.929f, BG_GREEN = 0.804f;
const float BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
	       F_SHADER_PATH[] = "shaders/fragment_textured.glsl";


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

const char ICE_SPRITE[] = "Large_Frost_Sprite_0.png";
const char PLAYER_1_SPRITE[] = "player-1-sprite.png";
const char PLAYER_2_SPRITE[] = "player-2-sprite.png";
const char SIDE_BORDER_SPRITE[] = "side-border.png";
const char BOT_BORDER_SPRITE[] = "top-bot-border.png";
const char TOP_BORDER_SPRITE[] = "top-border.png";
const char INSTRUCTION_SPRITE[] = "instruction-screen.png";

const char NUM_0[] = "numbers-sprite-0.png";
const char NUM_1[] = "numbers-sprite-1.png";
const char NUM_2[] = "numbers-sprite-2.png";
const char NUM_3[] = "numbers-sprite-3.png";
const char NUM_4[] = "numbers-sprite-4.png";
const char NUM_5[] = "numbers-sprite-5.png";
const char NUM_6[] = "numbers-sprite-6.png";
const char NUM_7[] = "numbers-sprite-7.png";
const char NUM_8[] = "numbers-sprite-8.png";
const char NUM_9[] = "numbers-sprite-9.png";

GLuint ice_texture_id, player_1_texture_id, player_2_texture_id,
	   side_border_texture_id, bot_border_texture_id, top_border_texture_id,
	   num_0_texture_id, num_1_texture_id, num_2_texture_id, num_3_texture_id,
	   num_4_texture_id, num_5_texture_id, num_6_texture_id, num_7_texture_id,
	   num_8_texture_id, num_9_texture_id, instruction_texture_id;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

int SCORE_P1 = 0;
int SCORE_P2 = 0;

bool game_start = false;

GLuint load_texture(const char* filepath) {
	int width, height, number_of_components;
	unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

	if (image == NULL) {
		//as of right now, can't print
		assert(false);
	}

	GLuint textureID;
	glGenTextures(NUMBER_OF_TEXTURES, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(image);

	return textureID;
}
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

	//enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ice_texture_id = load_texture(ICE_SPRITE);
	player_1_texture_id = load_texture(PLAYER_1_SPRITE);
	player_2_texture_id = load_texture(PLAYER_2_SPRITE);
	side_border_texture_id = load_texture(SIDE_BORDER_SPRITE);
	bot_border_texture_id = load_texture(BOT_BORDER_SPRITE);
	top_border_texture_id = load_texture(TOP_BORDER_SPRITE);
	instruction_texture_id = load_texture(INSTRUCTION_SPRITE);

	num_0_texture_id = load_texture(NUM_0);
	num_1_texture_id = load_texture(NUM_1);
	num_2_texture_id = load_texture(NUM_2);
	num_3_texture_id = load_texture(NUM_3);
	num_4_texture_id = load_texture(NUM_4);
	num_5_texture_id = load_texture(NUM_5);
	num_6_texture_id = load_texture(NUM_6);
	num_7_texture_id = load_texture(NUM_7);
	num_8_texture_id = load_texture(NUM_8);
	num_9_texture_id = load_texture(NUM_9);

	// Init view, model, and proj. matricies
	view_matrix = glm::mat4(1.f); // Identify 4x matrix

	model_matrix_p1 = glm::mat4(1.f); // Identify 4x matrix
	model_matrix_p2 = glm::mat4(1.f); // Identify 4x matrix
	model_matrix_b = glm::mat4(1.f); // Identify 4x matrix
	model_matrix_border = glm::mat4(1.f); // Identify 4x matrix

	projection_matrix = glm::ortho(-5.f, 5.f, -3.75f, 3.75f, -1.f, 1.f);
	//Camera looks perp on triangle

	//Load them onto OpenGL
	program.SetViewMatrix(view_matrix);
	program.SetProjectionMatrix(projection_matrix);

	//Set color of the triangle
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
					if (!game_start) { game_start = true; }
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
void update() {
	//for delta time
	float ticks = (float)SDL_GetTicks() / 1000.f; // get the current number of ticks
	float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
	previous_ticks = ticks;

	glm::vec3 top_border_vec = glm::vec3(0, 3.425f, 0);
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

		bool collision_right = check_collision(b_m_x, right_border_vec, MINIMUM_COLLISION_DISTANCE_BALL);
		bool collision_left = check_collision(b_m_x, left_border_vec, MINIMUM_COLLISION_DISTANCE_BALL);

		if (collision_right || collision_left)
		{
			//add lose condition
			//BALL_X_TRANS = -1.f * BALL_X_TRANS;
			BALL_X_TRANS = 0.f;
			BALL_Y_TRANS = 0.f;
			is_game_over = true;
			if (collision_right) {
				SCORE_P1++;
			}
			else {
				SCORE_P2++;
			}
			ball_moving = false;
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
void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
	program.SetModelMatrix(object_model_matrix);
	glBindTexture(GL_TEXTURE_2D, object_texture_id);
	glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}
GLuint sprite_calc(int expression) {
	switch (expression) {
	case 0:
		return num_0_texture_id;
		break;
	case 1:
		return num_1_texture_id;
		break;
	case 2:
		return num_2_texture_id;
		break;
	case 3:
		return num_3_texture_id;
		break;
	case 4:
		return num_4_texture_id;
		break;
	case 5:
		return num_5_texture_id;
		break;
	case 6:
		return num_6_texture_id;
		break;
	case 7:
		return num_7_texture_id;
		break;
	case 8:
		return num_8_texture_id;
		break;
	case 9:
		return num_9_texture_id;
		break;
	default:
		return num_0_texture_id;
		break;
	}
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
		-4.25f, -1.f, -3.4f, -1.f, -3.4f, 1.f, //triangle1
		-4.25f, -1.f, -3.4f, 1.f, -4.25f, 1.f   //triangle2
	};

	float texture_coordinates[] =
	{
		0.f, 1.f, 1.f, 1.f, 1.f, 0.f, //triangle 1
		0.f, 1.f, 1.f, 0.f, 0.f, 0.f //triangle 2
	};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_p1);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_p1, player_1_texture_id);

	program.SetModelMatrix(model_matrix_p2);

	//player 2 sprite
	float vertices_p2[] =
	{
		4.25f, -1.f, 3.4f, -1.f, 3.4f, 1.f, //triangle1
		4.25f, -1.f, 3.4f, 1.f, 4.25f, 1.f   //triangle2
	};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_p2);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_p2, player_2_texture_id);
	
	//borders
	program.SetModelMatrix(model_matrix_border);

	float vertices_b_top[] = 
	{
		-5.f, 2.65f, 5.f, 2.65f, 5.f, 3.75f, //triangle1
		-5.f, 2.65f, 5.f, 3.75f, -5.f, 3.75f   //triangle2
	};
	float vertices_b_right[] = 
	{
		4.5f, -3.5f, 5.f, -3.5f, 5.f, 3.5f, //triangle1
		4.5f, -3.5f, 5.f, 3.5f, 4.5f, 3.5f   //triangle2
	};
	float vertices_b_bot[] =
	{
		-5.f, -2.85f, 5.f, -2.85f, 5.f, -3.75f, //triangle1
		-5.f, -2.85f, 5.f, -3.75f, -5.f, -3.75f   //triangle2
	};
	float vertices_b_left[] =
	{
		-4.5f, -3.5f, -5.f, -3.5f, -5.f, 3.5f, //triangle1
		-4.5f, -3.5f, -5.f, 3.5f, -4.5f, 3.5f   //triangle2
	};
	float texture_inverse_coordinates[] =
	{
		0.f, 1.f, -1.f, 1.f, -1.f, 0.f, //triangle 1
		0.f, 1.f, -1.f, 0.f, 0.f, 0.f //triangle 2
	};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_b_top);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_border, top_border_texture_id);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_b_right);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_inverse_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_border, side_border_texture_id);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_b_bot);
	glEnableVertexAttribArray(program.positionAttribute);
	
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_border, bot_border_texture_id);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_b_left);
	glEnableVertexAttribArray(program.positionAttribute);
	
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_inverse_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_border, side_border_texture_id);

	//ball

	program.SetModelMatrix(model_matrix_b);

	float vertices_b[] =
	{
		-.375f, -.375f, .375f, -.375f, .375f, .375f,  //triangle1
		-.375f, -.375f, .375f, .375f, -.375f, .375f   //triangle2
	};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_b);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_b, ice_texture_id);

	//score 
	program.SetModelMatrix(model_matrix_border);

	//player 1 score

	GLuint sprite_num_sp1_0, sprite_num_sp1_1, sprite_num_sp1_2;

	sprite_num_sp1_0 = sprite_calc(SCORE_P1 % 10);
	sprite_num_sp1_1 = sprite_calc((SCORE_P1 / 10) % 10);
	sprite_num_sp1_2 = sprite_calc(SCORE_P1 / 100);

	float vertices_sp1_0[] =
	{
		-2.2f, 3.35f, -1.95f, 3.35f, -1.95f, 3.65f,  //triangle1
		-2.2f, 3.35, -1.95f, 3.65f, -2.2f, 3.65f  //triangle2
	};

	float vertices_sp1_1[] =
	{
		-1.85f, 3.35f, -1.6f, 3.35f, -1.6f, 3.65f,  //triangle1
		-1.85f, 3.35, -1.6f, 3.65f, -1.85f, 3.65f  //triangle2
	};

	float vertices_sp1_2[] =
	{
		-1.5f, 3.35f, -1.25f, 3.35f, -1.25f, 3.65f,  //triangle1
		-1.5f, 3.35, -1.25f, 3.65f, -1.5f, 3.65f  //triangle2
	};

	//first number
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_sp1_0);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_border, sprite_num_sp1_2);

	//second number
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_sp1_1);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_border, sprite_num_sp1_1);

	//third number
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_sp1_2);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_border, sprite_num_sp1_0);

	//player 2 score 

	GLuint sprite_num_sp2_0, sprite_num_sp2_1, sprite_num_sp2_2;

	sprite_num_sp2_0 = sprite_calc(SCORE_P2 % 10);
	sprite_num_sp2_1 = sprite_calc((SCORE_P2 / 10) % 10);
	sprite_num_sp2_2 = sprite_calc(SCORE_P2 / 100);
	
	float vertices_sp2_0[] =
	{
		3.4f, 3.35f, 3.65f, 3.35f, 3.65f, 3.65f,  //triangle1
		3.4f, 3.35, 3.65f, 3.65f, 3.4f, 3.65f  //triangle2
	};

	float vertices_sp2_1[] =
	{
		3.75f, 3.35f, 4.f, 3.35f, 4.f, 3.65f,  //triangle1
		3.75f, 3.35, 4.f, 3.65f, 3.75f, 3.65f  //triangle2
	};

	float vertices_sp2_2[] =
	{
		4.1f, 3.35f, 4.35f, 3.35f, 4.35f, 3.65f,  //triangle1
		4.1f, 3.35, 4.35f, 3.65f, 4.1f, 3.65f  //triangle2
	};

	//first number
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_sp2_0);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_border, sprite_num_sp2_2);

	//second number
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_sp2_1);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_border, sprite_num_sp2_1);

	//third number
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_sp2_2);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	draw_object(model_matrix_border, sprite_num_sp2_0);

	//instructions
	if (!game_start) {
		float vertices_inst[] =
		{
			-4.25f, -2.65f, 4.25f, -2.65f, 4.25f, 2.65f,  //triangle1
			-4.25f, -2.65f, 4.25f, 2.65f, -4.25f, 2.65f  //triangle2
		};

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_inst);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
		glEnableVertexAttribArray(program.texCoordAttribute);

		draw_object(model_matrix_border, instruction_texture_id);
	}
	
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
