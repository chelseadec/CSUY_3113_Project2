/**
 
 * Author: Chelsea DeCambre
 * Assignment: Pong Clone
 * Date due: 2023-06-23, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <cmath>

const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0.257f,
            BG_BLUE    = 0.231f,
            BG_GREEN   = 0.656f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X      = 0,
          VIEWPORT_Y      = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char SHELL_SPRITE[] = "pixelshell.png";
const char LUIGI_SPRITE[] = "pixelluigi.png";
const char MARIO_SPRITE[] = "pixelmario.png";

const float ROT_SPEED = 200.0f;

const glm::vec3 SHELL_INIT_POS = glm::vec3(0.0f, 1.0f, 0.0f),
                SHELL_INIT_SCA = glm::vec3(0.5f, 0.5f, 0.0f);

const glm::vec3 LUIGI_INIT_POS = glm::vec3(3.0f, -1.5f, 0.0f),
                LUIGI_INIT_SCA = glm::vec3(0.50f, 1.0f, 0.0f);

const glm::vec3 MARIO_INIT_POS = glm::vec3(-4.75f, -1.5f, 0.0f),
                MARIO_INIT_SCA = glm::vec3(1.25f, 1.0f, 0.0f);

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0,
            TEXTURE_BORDER   = 0;

const float MILLISECONDS_IN_SECOND = 1000.0;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_flower_program;
GLuint        g_flower_texture_id;

ShaderProgram g_luigi_program;
GLuint        g_luigi_texture_id;

ShaderProgram g_mario_program;
GLuint        g_mario_texture_id;

glm::mat4 g_view_matrix,
          g_shell_model_matrix,
          g_luigi_model_matrix,
          g_mario_model_matrix,
          g_projection_matrix;

float g_previous_ticks  = 0.0f;
float g_rot_angle = 0.0f;
float g_speed = 1.0f;

glm::vec3 g_shell_movement = glm::vec3(0.0f, 0.0f, 0.0f),
          g_shell_position = glm::vec3(0.0f, 0.0f, 0.0f),
          g_flower_scale    = glm::vec3(1.0f, 1.0f, 0.0f),
          g_flower_growth   = glm::vec3(0.0f, 0.0f, 0.0f);
// ———————————————— PART 1 ———————————————— //

glm::vec3 g_luigi_movement = glm::vec3(0.0f, 0.0f, 0.0f),
          g_luigi_position = glm::vec3(1.0f, 0.0f, 0.0f),
          g_luigi_scale    = glm::vec3(3.0f, 3.0f, 0.0f),
          g_luigi_growth   = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_mario_movement = glm::vec3(0.0f, 0.0f, 0.0f),
          g_mario_position = glm::vec3(1.0f, 0.0f, 0.0f),
          g_mario_scale    = glm::vec3(3.0f, 3.0f, 0.0f),
          g_mario_growth   = glm::vec3(0.0f, 0.0f, 0.0f);

GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
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


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("User input exercise",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    // ———————————————— FLOWER ———————————————— //
    g_flower_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_shell_model_matrix = glm::mat4(1.0f);
    g_shell_model_matrix = glm::translate(g_shell_model_matrix, SHELL_INIT_POS);
    g_shell_model_matrix = glm::scale(g_shell_model_matrix, SHELL_INIT_SCA);
    
    g_flower_program.SetProjectionMatrix(g_projection_matrix);
    g_flower_program.SetViewMatrix(g_view_matrix);
    
    glUseProgram(g_flower_program.programID);
    g_flower_texture_id = load_texture(SHELL_SPRITE);
    
    // ———————————————— LUIGI ———————————————— //
    g_luigi_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_luigi_model_matrix = glm::mat4(1.0f);
    g_luigi_model_matrix = glm::translate(g_luigi_model_matrix, LUIGI_INIT_POS);
    g_luigi_model_matrix = glm::scale(g_luigi_model_matrix, LUIGI_INIT_SCA);
    
    g_luigi_program.SetProjectionMatrix(g_projection_matrix);
    g_luigi_program.SetViewMatrix(g_view_matrix);
    
    glUseProgram(g_luigi_program.programID);
    g_luigi_texture_id = load_texture(LUIGI_SPRITE);
    
    // ———————————————— MARIO  ———————————————— //
    g_mario_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_mario_model_matrix = glm::mat4(1.0f);
    g_mario_model_matrix = glm::translate(g_mario_model_matrix, MARIO_INIT_POS);
    g_mario_model_matrix = glm::scale(g_mario_model_matrix, MARIO_INIT_SCA);
    
    g_mario_program.SetProjectionMatrix(g_projection_matrix);
    g_mario_program.SetViewMatrix(g_view_matrix);
    
    glUseProgram(g_mario_program.programID);
    g_mario_texture_id = load_texture(MARIO_SPRITE);
    
    
    // ———————————————— GENERAL ———————————————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = !g_game_is_running;
                break;
            
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        g_game_is_running = !g_game_is_running;
                        break;
                        
                    default: break;
                }
        }
    }
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

        
    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_luigi_movement.x = 0.0f;
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_luigi_movement.x = 0.0f;
    }
    
    if (key_state[SDL_SCANCODE_UP])
    {
        g_luigi_movement.y = 1.0f;
        
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        g_luigi_movement.y = -1.0f;
    }
    
    if (key_state[SDL_SCANCODE_A])
    {
        g_mario_movement.x = 0.0f;
    }
    else if (key_state[SDL_SCANCODE_D])
    {
        g_mario_movement.x = 0.0f;
    }
    
    if (key_state[SDL_SCANCODE_W])
    {
        g_mario_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_S])
    {
        g_mario_movement.y = -1.0f;
    }
    
    
    if (glm::length(g_shell_movement) > 1.0f)
    {
        g_shell_movement = glm::normalize(g_shell_movement); // do this to avoid player sprites from moving twice as fast when moving diagonally
    }
}
const int MAX_FRAME = 200;
int g_frame_counter = 0;
bool reverse = false;

void update()
{
    // ———————————————— PART 2 ———————————————— //
    float collision_factor = 0.001f;
    
    float x_distance = fabs(g_shell_position.x - g_luigi_position.x) - ((SHELL_INIT_SCA.x * collision_factor + g_luigi_scale.x * collision_factor) / 2.0f);
    float y_distance = fabs(g_shell_position.y - g_luigi_position.y) - ((SHELL_INIT_SCA.y * collision_factor + g_luigi_scale.y * collision_factor) / 2.0f);
    
    float x_distance2 = fabs(g_shell_position.x - g_mario_position.x) - ((SHELL_INIT_SCA.x * collision_factor + g_mario_scale.x * collision_factor) / 2.0f);
    float y_distance2 = fabs(g_shell_position.y - g_mario_position.y) - ((SHELL_INIT_SCA.y * collision_factor + g_mario_scale.y * collision_factor) / 2.0f);
    
    if ((x_distance < 0 && y_distance < 0) || (x_distance2 < 0 && y_distance2 < 0)) // what to do after the objects collide
    {
        LOG("COLLISION!!!");
        // If collision occurs, tell the flower if and in which direction to scale (in this case, shrinking)
        g_shell_movement = -(g_shell_movement);
        
    }
    // ———————————————— PART 2 ———————————————— //
    
    // ———————————————— DELTA TIME CALCULATIONS ———————————————— //
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    float TRAN_VALUE = 0.010f;
    
    g_frame_counter += 1;
    
    // ———————————————— RESETTING MODEL MATRIX ———————————————— //
    g_shell_model_matrix = glm::mat4(1.0f);
    g_shell_model_matrix = glm::translate(g_shell_model_matrix, SHELL_INIT_POS);
    g_shell_model_matrix = glm::scale(g_shell_model_matrix, SHELL_INIT_SCA);
    
    g_luigi_model_matrix = glm::mat4(1.0f);
    g_luigi_model_matrix = glm::translate(g_luigi_model_matrix, LUIGI_INIT_POS);
    g_luigi_model_matrix = glm::scale(g_luigi_model_matrix, LUIGI_INIT_SCA);
    
    g_mario_model_matrix = glm::mat4(1.0f);
    g_mario_model_matrix = glm::translate(g_mario_model_matrix, MARIO_INIT_POS);
    g_mario_model_matrix = glm::scale(g_mario_model_matrix, MARIO_INIT_SCA);
    // ———————————————— TRANSLATIONS ———————————————— //
    g_shell_position += g_shell_movement * g_speed * delta_time;
    g_shell_model_matrix = glm::translate(g_shell_model_matrix, g_shell_position);
    float direction = 1.0f;
    if (g_shell_position.x < -5.0f || g_shell_position.x > 5.0f) {
        LOG(g_shell_position.x);
        LOG("HERE");
        direction = -(direction);
        g_shell_movement = glm::vec3(direction, 0.0f, 0.0f);
    }
    g_shell_movement = glm::vec3(direction, 0.0f, 0.0f);
    
        
    
    g_luigi_position += g_luigi_movement * g_speed * delta_time;
    g_luigi_model_matrix = glm::translate(g_luigi_model_matrix, g_luigi_position);
    g_luigi_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    
    g_mario_position += g_mario_movement * g_speed * delta_time;
    g_mario_model_matrix = glm::translate(g_mario_model_matrix, g_mario_position);
    g_mario_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // ———————————————— ROTATIONS ———————————————— //
    g_rot_angle += ROT_SPEED * delta_time;
    g_shell_model_matrix = glm::rotate(g_shell_model_matrix, glm::radians(g_rot_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    
    
//    if (g_frame_counter >= MAX_FRAME)
//    {
//        reverse = !reverse;
//        g_frame_counter = 0;
//    }
//
//    g_shell_model_matrix = glm::translate(g_shell_model_matrix , glm::vec3(reverse ? -(TRAN_VALUE) : TRAN_VALUE, 0.0f, 0.0f)); // makes shell travel in a cresent-like fashion.

    
    // ———————————————— PART 3 ———————————————— //
    // Here, I simply use 1.0f as my shrinking speed
    g_flower_scale += g_flower_growth * delta_time;
    
    // Since we'll eventually reach scale factors of 0 (i.e. the flower will have disappeared)
    // we want to make sure that a) we remain at that scale of 0 and b) that we don't "scale
    // negatively.
    g_flower_scale.x = g_flower_scale.x > 0.0 ? g_flower_scale.x : 0.0f;
    g_flower_scale.y = g_flower_scale.y > 0.0 ? g_flower_scale.y : 0.0f;
    g_shell_model_matrix = glm::scale(g_shell_model_matrix, g_flower_scale);
    // ———————————————— PART 3 ———————————————— //
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // ———————————————— FLOWER ———————————————— //
    float flower_vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float flower_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    glVertexAttribPointer(g_flower_program.positionAttribute, 2, GL_FLOAT, false, 0, flower_vertices);
    glEnableVertexAttribArray(g_flower_program.positionAttribute);
    
    glVertexAttribPointer(g_flower_program.texCoordAttribute, 2, GL_FLOAT, false, 0, flower_texture_coordinates);
    glEnableVertexAttribArray(g_flower_program.texCoordAttribute);
    
    g_flower_program.SetModelMatrix(g_shell_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_flower_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(g_flower_program.positionAttribute);
    glDisableVertexAttribArray(g_flower_program.texCoordAttribute);
    
    // ———————————————— LUIGI ———————————————— //
    float luigi_vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float luigi_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    glVertexAttribPointer(g_luigi_program.positionAttribute, 2, GL_FLOAT, false, 0, luigi_vertices);
    glEnableVertexAttribArray(g_luigi_program.positionAttribute);
    
    glVertexAttribPointer(g_luigi_program.texCoordAttribute, 2, GL_FLOAT, false, 0, luigi_texture_coordinates);
    glEnableVertexAttribArray(g_luigi_program.texCoordAttribute);
    
    g_luigi_program.SetModelMatrix(g_luigi_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_luigi_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(g_luigi_program.positionAttribute);
    glDisableVertexAttribArray(g_luigi_program.texCoordAttribute);
    
    // ———————————————— MARIO ———————————————— //
    float mario_vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float mario_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    glVertexAttribPointer(g_mario_program.positionAttribute, 2, GL_FLOAT, false, 0, mario_vertices);
    glEnableVertexAttribArray(g_mario_program.positionAttribute);
    
    glVertexAttribPointer(g_mario_program.texCoordAttribute, 2, GL_FLOAT, false, 0, mario_texture_coordinates);
    glEnableVertexAttribArray(g_mario_program.texCoordAttribute);
    
    g_mario_program.SetModelMatrix(g_mario_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_mario_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(g_mario_program.positionAttribute);
    glDisableVertexAttribArray(g_mario_program.texCoordAttribute);
    // ———————————————— GENERAL ———————————————— //
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}

