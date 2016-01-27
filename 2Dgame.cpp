#include <bits/stdc++.h>
#include <sys/time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

int width = 1280;
int height = 720;
float zoom = 1;
float pan = 0;
float g = 0.12;
int lives = 8;
bool mouse_drag = false;
bool drag_start = false;
int score = 0;
float drag_x;
float coin_location[] = { 700, 200, 700, 400, 700, 600, 700, 500, 700, 300, 1200, 200, 1200, 600};
float obstacle_location[] = { 1000, 200, 1000, 400, 1000, 600, 1000, 500, 1000, 300};
float block_location[] = { 500, 100, 500, 350, 500, 600};
float block2_location[] = { 800, 200, 800, 400, 800, 600};
float sl[10][2] = {{0,0},{60,400},{60,440},{100,400},{100,440},{60,400},{60,440},{60,480}};
int a[] = { 123457, 34, 14567, 34567, 2346, 23567, 12356, 347, 1234567, 234567};
long long tim;
long long rblock_rot = 0;
typedef struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
}VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;

// Struct for circle object
typedef struct Circle{
    VAO *obj;
    float xc, yc, vx, vy, r;
    bool display;
}Circle;

// Struct for rect object
typedef struct Rect{
    VAO *obj;
    float x1,x2,y1,y2;
    bool display;
}Rect;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
            0,                  // attribute 0. Vertices
            3,                  // size (x,y,z)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
            1,                  // attribute 1. Color
            3,                  // size (r,g,b)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

VAO *ground, *cannon_base, *cannon_rectangle, *circle, *speed_rect, *outerammo , *ammo , *sc1, *sc2;
Circle ball;
Circle coin[10];
Circle bcoin;
Circle obstacle[10];
Rect block[10];
Rect block2[10];
Rect rblock[2];
float camera_rotation_angle = 90;

float speed = 0.0f;
bool ball_translate_status = false;
bool display_ball;
bool fire = false;

float cannon_rectangle_rotation = 0;
float cannon_rectangle_rot_dir = 0;
bool cannon_rectangle_rot_status = false;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) 
    {
        switch (key) 
        {
            case GLFW_KEY_A:
                cannon_rectangle_rot_status = false;
                cannon_rectangle_rot_dir = 0;
                break;
            case GLFW_KEY_B:
                cannon_rectangle_rot_status = false;
                cannon_rectangle_rot_dir = 0;
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS || action == GLFW_REPEAT) 
    {
        switch (key) 
        {
            case GLFW_KEY_UP:
                zoom -= 0.1;
                if( zoom <= 0.1 )
                    zoom = 0.1;
                break;
            case GLFW_KEY_DOWN:
                zoom += 0.1;
                if( zoom >= 1 )
                    zoom = 1;
                if( pan > (1.0f-zoom)*(float)width)
                    pan = (1.0f-zoom)*(float)width;
                break;
            case GLFW_KEY_LEFT:
                pan -= 1;
                if( pan < 0 )
                    pan = 0;
                break;
            case GLFW_KEY_RIGHT:
                pan += 1;
                if( pan > (1.0f-zoom)*(float)width)
                    pan -= 1;
                break;
            case GLFW_KEY_A:
                cannon_rectangle_rot_status = true;
                cannon_rectangle_rot_dir = 1;
                break;
            case GLFW_KEY_B:
                cannon_rectangle_rot_status = true;
                cannon_rectangle_rot_dir = -1;
                break;
            case GLFW_KEY_F:
                speed++;
                if( speed > 20 )
                    speed = 20;
                break;
            case GLFW_KEY_S:
                speed--;
                if( speed < 0 )
                    speed = 0;
                break;
            case GLFW_KEY_R:
                if( lives )
                {
                    fire = true;
                    ball.xc = 90;
                    ball.yc = 150;
                    ball.vy = ball.vx = 0;
                    speed = 0;
                    ball_translate_status = false;
                    display_ball = true;
                    lives--;
                }
                break;
            case GLFW_KEY_SPACE:
                if(fire)
                {
                    ball_translate_status = true;
                    ball.vx = speed*cos((cannon_rectangle_rotation*M_PI)/180);
                    ball.vy = speed*sin((cannon_rectangle_rotation*M_PI)/180);
                    fire = false; 
                    struct timeval tv;
                    gettimeofday(&tv,NULL);
                    tim = tv.tv_usec;
                }
                break;
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
            quit(window);
            break;
        default:
            break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                if(fire)
                {
                    ball_translate_status = true;
                    ball.vx = speed*cos((cannon_rectangle_rotation*M_PI)/180);
                    ball.vy = speed*sin((cannon_rectangle_rotation*M_PI)/180);
                    fire = false;
                    struct timeval tv;
                    gettimeofday(&tv,NULL);
                    tim = tv.tv_usec;
                }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if( action == GLFW_PRESS )
            {
                mouse_drag = true;
                drag_start = true;
            }
            if (action == GLFW_RELEASE) 
            {
                drag_start = false;
                mouse_drag = false;
            }
            break;
        default:
            break;
    }
}

void cursorPos(GLFWwindow *window, double x_position,double y_position)
{
    float y_new = height - y_position;

    if( drag_start )
        drag_x = x_position;

    if( mouse_drag )
    {
        drag_start = false;
        pan -= (float)(x_position - drag_x)/500.0f;
        if( pan < 0 )
            pan = 0;
        if( pan > (1.0f-zoom)*(float)width)
            pan = (1.0f-zoom)*(float)width;
    }
    speed = ((x_position-90)*(x_position-90) + (y_new-150)*(y_new-150))/30000;

    if(speed > 20)
        speed = 20;
    cannon_rectangle_rotation = atan((y_new-150)/(x_position-90))*180.0f/M_PI;

    if( x_position - 90 < 0 )
        cannon_rectangle_rotation += 180;
}

void mouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset > 0) 
    { 
        zoom -= 0.1;
        if( zoom <= 0.1 )
            zoom = 0.1;
    }
    else 
    {
        zoom += 0.1;
        if( zoom >= 1 )
            zoom = 1;
        if( pan > (1.0f-zoom)*(float)width)
            pan = (1.0f-zoom)*(float)width;
    }
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
       is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    //GLfloat fov = 90.0f;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // set the projection matrix as perspective
    /* glMatrixMode (GL_PROJECTION);
       glLoadIdentity ();
       gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    //if( (float)width*zoom + pan > width )
    //  pan -= 1;
    Matrices.projection = glm::ortho(zoom*0.0f + pan, (float)width*zoom + pan, zoom*0.0f, (float)height*zoom, 0.1f, 500.0f);
}

// Creates the ground rect object
void createGround ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        0, 0, 0, // vertex 1
        (float)width, 0, 0, // vertex 2
        0, 25, 0, // vertex 3

        (float)width, 25, 0, // vertex 4
        0, 25, 0, // vertex 3
        (float)width , 0, 0  // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        0,0.4,0, // color 1
        0,0.4,0, // color 2
        0,0.4,0, // color 3

        0,0.4,0, // color 4
        0,0.4,0, // color 3
        0,0.4,0, // color 2

    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    ground = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the rotating rect object
void createRblock ()
{

    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -20,-20, 0, // vertex 1
        20, -20, 0, // vertex 2
        -20, 20, 0, // vertex 3

        20, 20, 0, // vertex 4
        -20, 20, 0, // vertex 3
        20, -20, 0  // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        1,0.4,0, // color 1
        1,0.4,0, // color 2
        1,0.4,0, // color 3

        1,0.4,0, // color 4
        1,0.4,0, // color 3
        1,0.4,0, // color 2

    };

    rblock[0].x1 = 1080;
    rblock[0].x2 = 1120;
    rblock[0].y1 = 480;
    rblock[0].y2 = 520;
    rblock[1].x1 = 1080;
    rblock[1].x2 = 1120;
    rblock[1].y1 = 280;
    rblock[1].y2 = 320;
    // create3DObject creates and returns a handle to a VAO that can be used later
    rblock[0].obj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    rblock[1].obj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the score rect object
void createSC1 ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        0, 0, 0, // vertex 1
        10, 0, 0, // vertex 2
        0, 40, 0, // vertex 3

        10, 40, 0, // vertex 4
        0, 40, 0, // vertex 3
        10 , 0, 0  // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        1,1,1, // color 1
        1,1,1, // color 2
        1,1,1, // color 3

        1,1,1, // color 4
        1,1,1, // color 3
        1,1,1, // color 2

    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    sc1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createSC2 ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        0, 0, 0, // vertex 1
        50, 0, 0, // vertex 2
        0, 10, 0, // vertex 3

        50, 10, 0, // vertex 4
        0, 10, 0, // vertex 3
        50 , 0, 0  // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        1,1,1, // color 1
        1,1,1, // color 2
        1,1,1, // color 3

        1,1,1, // color 4
        1,1,1, // color 3
        1,1,1, // color 2

    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    sc2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the speed rectangle object
void createSpeedRect ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -500, 0, 0, // vertex 1
        0, 0, 0, // vertex 2
        -500, 40, 0, // vertex 3

        0, 40, 0, // vertex 4
        -500, 40, 0, // vertex 3
        0 , 0, 0  // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        1,1,0, // color 1
        1,1,0, // color 2
        1,1,0, // color 3

        1,1,0, // color 4
        1,1,0, // color 3
        1,1,0, // color 2

    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    speed_rect = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the cannon circle object
void createCircle ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
    GLfloat vertex_buffer_data[2000] = {};
    GLfloat color_buffer_data[2000] = {};

    // Center of the circle
    vertex_buffer_data[0] = 0;
    vertex_buffer_data[1] = 0;
    vertex_buffer_data[2] = 0;
    color_buffer_data[0] = 0;
    color_buffer_data[1] = 0;
    color_buffer_data[2] = 0;

    int i,x;
    x=3;

    for( i=0;i<181;i++)
    {
        vertex_buffer_data[x] = 40*cos(i*M_PI/180);
        color_buffer_data[x] = 0;
        x++;
        vertex_buffer_data[x] = 40*sin(i*M_PI/180);
        color_buffer_data[x] = 0;
        x++;
        vertex_buffer_data[x] = 0;
        color_buffer_data[x] = 0;
        x++;
    }

    // create3DObject creates and returns a handle to a VAO that can be used later
    circle = create3DObject(GL_TRIANGLE_FAN, 182, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the cannon base object
void createCannonBase ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        50, 0, 0, // vertex 1
        130, 0, 0, // vertex 2
        50, 150, 0, // vertex 3

        130, 0, 0, // vertex 4
        50, 150, 0, // vertex 3
        130 , 150, 0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0,0,0, // color 1
        0,0,0, // color 2
        0,0,0, // color 3

        0,0,0, // color 4
        0,0,0, // color 3
        0,0,0, // color 1

    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    cannon_base = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the cannon rotating rect object
void createCannonRectangle ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        0, -25, 0, // vertex 1
        100, -25, 0, // vertex 2
        0, 25, 0, // vertex 3

        100, 25, 0, // vertex 4
        0, 25, 0, // vertex 3
        100 , -25, 0  // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        0,0,0, // color 1
        0,0,0, // color 2
        0,0,0, // color 3

        0,0,0, // color 4
        0,0,0, // color 3
        0,0,0, // color 2

    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    cannon_rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the ball object 
void createBall ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
    GLfloat vertex_buffer_data[2000] = {};
    GLfloat color_buffer_data[2000] = {};

    // Center of the circle
    vertex_buffer_data[0] = 0;
    vertex_buffer_data[1] = 0;
    vertex_buffer_data[2] = 0;
    color_buffer_data[0] = 0;
    color_buffer_data[1] = 0;
    color_buffer_data[2] = 0;

    int i,x;
    x=3;

    for( i=0;i<361;i++)
    {
        vertex_buffer_data[x] = 15*cos(i*M_PI/180);
        color_buffer_data[x] = 0;
        x++;
        vertex_buffer_data[x] = 15*sin(i*M_PI/180);
        color_buffer_data[x] = 0;
        x++;
        vertex_buffer_data[x] = 0;
        color_buffer_data[x] = 1;
        x++;
    }

    // create3DObject creates and returns a handle to a VAO that can be used later
    ball.xc = 90 ;
    ball.yc = 150;
    ball.vx = 0 ;
    ball.vy = 0;
    ball.r = 15;

    ball.obj = create3DObject(GL_TRIANGLE_FAN, 362, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the coin object 
void createCoin ()
{
    for( int j = 0 ; j < 7 ; j++ )
    {
        /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
        GLfloat vertex_buffer_data[2000] = {};
        GLfloat color_buffer_data[2000] = {};

        // Center of the circle
        vertex_buffer_data[0] = 0;
        vertex_buffer_data[1] = 0;
        vertex_buffer_data[2] = 0;
        color_buffer_data[0] = 0;
        color_buffer_data[1] = 0;
        color_buffer_data[2] = 0;

        int x = 3;

        for(int i=0;i<361;i++)
        {
            vertex_buffer_data[x] = 15*cos(i*M_PI/180);
            color_buffer_data[x] = 1;
            x++;
            vertex_buffer_data[x] = 15*sin(i*M_PI/180);
            color_buffer_data[x] = 1;
            x++;
            vertex_buffer_data[x] = 0;
            color_buffer_data[x] = 0;
            x++;
        }

        // create3DObject creates and returns a handle to a VAO that can be used later
        coin[j].xc = coin_location[2*j];
        coin[j].yc = coin_location[2*j+1];
        coin[j].vx = 0;
        coin[j].vy = 0;
        coin[j].r = 15;
        coin[j].display = true;
        coin[j].obj = create3DObject(GL_TRIANGLE_FAN, 362, vertex_buffer_data, color_buffer_data, GL_FILL);

    }
}

// Creates the Big coin object 
void createBcoin ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
    GLfloat vertex_buffer_data[2000] = {};
    GLfloat color_buffer_data[2000] = {};

    // Center of the circle
    vertex_buffer_data[0] = 0;
    vertex_buffer_data[1] = 0;
    vertex_buffer_data[2] = 0;
    color_buffer_data[0] = 0;
    color_buffer_data[1] = 0;
    color_buffer_data[2] = 0;

    int x = 3;

    for(int i=0;i<361;i++)
    {
        vertex_buffer_data[x] = 25*cos(i*M_PI/180);
        color_buffer_data[x] = 1;
        x++;
        vertex_buffer_data[x] = 25*sin(i*M_PI/180);
        color_buffer_data[x] = 1;
        x++;
        vertex_buffer_data[x] = 0;
        color_buffer_data[x] = 0;
        x++;
    }

    // create3DObject creates and returns a handle to a VAO that can be used later
    bcoin.xc = 1200;
    bcoin.yc = 400; 
    bcoin.vx = 0;
    bcoin.vy = 0;
    bcoin.r = 15;
    bcoin.display = true;
    bcoin.obj = create3DObject(GL_TRIANGLE_FAN, 362, vertex_buffer_data, color_buffer_data, GL_FILL);

}

// Creates the movable obstacle object 
void createObstacle ()
{
    for( int j = 0 ; j < 5 ; j++ )
    {
        /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
        GLfloat vertex_buffer_data[2000] = {};
        GLfloat color_buffer_data[2000] = {};

        // Center of the circle
        vertex_buffer_data[0] = 0;
        vertex_buffer_data[1] = 0;
        vertex_buffer_data[2] = 0;
        color_buffer_data[0] = 1;
        color_buffer_data[1] = 0;
        color_buffer_data[2] = 0;

        int x = 3;

        for(int i=0;i<361;i++)
        {
            vertex_buffer_data[x] = 15*cos(i*M_PI/180);
            color_buffer_data[x] = 1;
            x++;
            vertex_buffer_data[x] = 15*sin(i*M_PI/180);
            color_buffer_data[x] = 0;
            x++;
            vertex_buffer_data[x] = 0;
            color_buffer_data[x] = 0;
            x++;
        }

        // create3DObject creates and returns a handle to a VAO that can be used later
        obstacle[j].xc = obstacle_location[2*j];
        obstacle[j].yc = obstacle_location[2*j+1];
        obstacle[j].vx = 0;
        obstacle[j].vy = 0;
        obstacle[j].r = 15;
        obstacle[j].display = true;
        obstacle[j].obj = create3DObject(GL_TRIANGLE_FAN, 362, vertex_buffer_data, color_buffer_data, GL_FILL);

    }
}

// Creates the block object 
void createBlock ()
{
    for( int i = 0 ; i < 3 ; i++)
    {
        block[i].x1 = block_location[2*i]; 
        block[i].y1 = block_location[2*i+1]; 
        block[i].x2 = block_location[2*i] + 40; 
        block[i].y2 = block_location[2*i+1] + 100; 
        // GL3 accepts only Triangles. Quads are not supported
        static const GLfloat vertex_buffer_data [] = {
            0, 0, 0, // vertex 1
            40, 0, 0, // vertex 2
            0, 100, 0, // vertex 3

            40, 100, 0, // vertex 4
            0, 100, 0, // vertex 3
            40 , 0, 0  // vertex 2
        };

        static const GLfloat color_buffer_data [] = {
            1,0,0, // color 1
            1,0,0, // color 2
            1,0,0, // color 3

            1,0,0, // color 4
            1,0,0, // color 3
            1,0,0, // color 2

        };

        // create3DObject creates and returns a handle to a VAO that can be used later
        block[i].obj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    }
}

// Creates the block2 object 
void createBlock2 ()
{
    for( int i = 0 ; i < 3 ; i++)
    {
        block2[i].x1 = block2_location[2*i]; 
        block2[i].y1 = block2_location[2*i+1]; 
        block2[i].x2 = block2_location[2*i] + 40; 
        block2[i].y2 = block2_location[2*i+1] + 80; 
        block2[i].display = true;
        // GL3 accepts only Triangles. Quads are not supported
        static const GLfloat vertex_buffer_data [] = {
            0, 0, 0, // vertex 1
            40, 0, 0, // vertex 2
            0, 80, 0, // vertex 3

            40, 80, 0, // vertex 4
            0, 80, 0, // vertex 3
            40 , 0, 0  // vertex 2
        };

        static const GLfloat color_buffer_data [] = {
            1,0,1, // color 1
            1,0,1, // color 2
            1,0,1, // color 3

            1,0,1, // color 4
            1,0,1, // color 3
            1,0,1, // color 2

        };

        // create3DObject creates and returns a handle to a VAO that can be used later
        block2[i].obj = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    }
}

void createOuterAmmo ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
    GLfloat vertex_buffer_data[2000] = {};
    GLfloat color_buffer_data[2000] = {};

    // Center of the circle
    vertex_buffer_data[0] = 0;
    vertex_buffer_data[1] = 0;
    vertex_buffer_data[2] = 0;
    color_buffer_data[0] = 0;
    color_buffer_data[1] = 0;
    color_buffer_data[2] = 0;

    int i,x;
    x=3;

    for( i=0;i<361;i++)
    {
        vertex_buffer_data[x] = 70*cos(i*M_PI/180);
        color_buffer_data[x] = 0;
        x++;
        vertex_buffer_data[x] = 70*sin(i*M_PI/180);
        color_buffer_data[x] = 0;
        x++;
        vertex_buffer_data[x] = 0;
        color_buffer_data[x] = 0;
        x++;
    }

    // create3DObject creates and returns a handle to a VAO that can be used later
    outerammo = create3DObject(GL_TRIANGLE_FAN, 362, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createAmmo ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
    GLfloat vertex_buffer_data[2000] = {};
    GLfloat color_buffer_data[2000] = {};

    // Center of the circle
    vertex_buffer_data[0] = 0;
    vertex_buffer_data[1] = 0;
    vertex_buffer_data[2] = 0;
    color_buffer_data[0] = 1;
    color_buffer_data[1] = 1;
    color_buffer_data[2] = 1;

    int i,x;
    x=3;

    for( i=0;i<361;i++)
    {
        vertex_buffer_data[x] = 15*cos(i*M_PI/180);
        color_buffer_data[x] = 1;
        x++;
        vertex_buffer_data[x] = 15*sin(i*M_PI/180);
        color_buffer_data[x] = 1;
        x++;
        vertex_buffer_data[x] = 0;
        color_buffer_data[x] = 1;
        x++;
    }

    // create3DObject creates and returns a handle to a VAO that can be used later
    ammo = create3DObject(GL_TRIANGLE_FAN, 362, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void calculateBallCoordinates ()
{
    if( ball_translate_status )
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        long long dif= tv.tv_usec - tim;
        tim = tv.tv_usec;
        float te = (float)dif/10000;
        te = 1;
        ball.xc += ball.vx ;
        ball.yc -= ((ball.vy-(g*te))*(ball.vy-(g*te)) - ball.vy*ball.vy)/(2*g);
        ball.vy -= g*(te);
        ball.vx = 0.9999*ball.vx;
    }
}

void calculateObstacleCoordinates ()
{
    for( int i = 0 ; i < 5 ; i++ )
    {
        obstacle[i].xc += obstacle[i].vx;
        obstacle[i].yc -= obstacle[i].vy;
        obstacle[i].vx = 0.9999*obstacle[i].vx;
    }
}

void checkGroundCollision ()
{
    if( ball.yc - ball.r <= 25 )
    {
        ball.vy *= -0.78;
        ball.vx *= 0.9;
        ball.yc += 5;
    }
    if( ball.yc + ball.r >= height )
    {
        ball.vy *= -0.78;
        ball.vx *= 0.9;
        ball.yc -= 5;
    }
    if( ball.xc + ball.r >= width )
    {
        ball.vx *= -0.66;
        ball.vy *= 0.9;
        ball.xc -= 5;
    }
    if( ball.xc - ball.r <= 0)
    {
        ball.vx *= -0.66;
        ball.vy *= 0.9;
        ball.xc += 5;
    }

    for( int i = 0 ; i < 5 ; i ++ )
    {
        if( obstacle[i].yc - obstacle[i].r <= 25 )
        {
            obstacle[i].vy *= -0.78;
            obstacle[i].vx *= 0.9;
            obstacle[i].yc += 5;
        }
        if( obstacle[i].xc + obstacle[i].r >= width )
        {
            obstacle[i].vx *= -0.66;
            obstacle[i].vy *= 0.9;
            obstacle[i].xc -= 5;
        }
        if( obstacle[i].xc - obstacle[i].r <= 0)
        {
            obstacle[i].vx *= -0.66;
            obstacle[i].vy *= 0.9;
            obstacle[i].xc += 5;
        }
    }
}

void checkCoinCollision ()
{
    for( int i = 0 ; i < 7 ; i++ )
    {
        if( coin[i].display )
            if( (coin[i].yc - ball.yc)*(coin[i].yc - ball.yc) + (coin[i].xc - ball.xc)*(coin[i].xc - ball.xc) <= 900 )
            {
                coin[i].display = false;
                score++;
                cout<<score<<endl;
            }
    }
    if( bcoin.display )
        if( (bcoin.yc - ball.yc)*(bcoin.yc - ball.yc) + (bcoin.xc - ball.xc)*(bcoin.xc - ball.xc) <= 1600 )
        {
            bcoin.display = false;
            score++;
            score++;
            cout<<score<<endl;
        }
}

void checkObstacleCollision ()
{
    for( int i = 0 ; i < 5 ; i++ )
    {
        if( (obstacle[i].yc - ball.yc)*(obstacle[i].yc - ball.yc) + (obstacle[i].xc - ball.xc)*(obstacle[i].xc - ball.xc) <= 1600 )
        {
            if( ball.vx >= 0 )
                ball.xc -= 5;
            else
                ball.xc += 5;
            if( ball.vy >= 0 )
                ball.yc -= 5;
            else
                ball.yc += 5;
            if( obstacle[i].vx > 0 )
                obstacle[i].xc -= 3;
            else if( obstacle[i].vx < 0 )
                obstacle[i].xc += 3;
            if( obstacle[i].vy > 0 )
                obstacle[i].yc -= 3;
            else if( obstacle[i].vy < 0 )
                obstacle[i].yc += 3;
            obstacle[i].vx = 0.4*ball.vx;
            obstacle[i].vy = 0.6*ball.vy;
            ball.vx *= -0.999;
            ball.vy *= -0.999;
        }
    }
}

void checkBCollision ()
{
    for( int j = 0 ; j<2 ; j++)
    {
        for( int i = 0 ; i < 5 ; i++ )
        {
            if( (obstacle[i].yc - rblock[j].y1 - 20)*(obstacle[i].yc - rblock[j].y1 - 20) + (obstacle[i].xc - rblock[j].x1 - 20)*(obstacle[i].xc - rblock[j].x1 - 20) <= 44*44 )
            {
                if( obstacle[i].vx > 0 )
                    obstacle[i].xc -= 10;
                else if( obstacle[i].vx < 0 )
                    obstacle[i].xc += 10;
                if( obstacle[i].vy > 0 )
                    obstacle[i].yc -= 10;
                else if( obstacle[i].vy < 0 )
                    obstacle[i].yc += 10;
                obstacle[i].vx *= -1;
                obstacle[i].vy *= -1;
            }
        }
        if( display_ball )
        if( (rblock[j].y1 + 20 - ball.yc)*(rblock[j].y1 + 20 - ball.yc) + (rblock[j].x1 + 20- ball.xc)*(rblock[j].x1 + 20 - ball.xc) <= 1600 )
        {
            if( ball.vx >= 0 )
                ball.xc -= 10;
            else
                ball.xc += 10;
            if( ball.vy >= 0 )
                ball.yc -= 10;
            else
                ball.yc += 10;
            ball.vx *= -1;
            ball.vy *= -1;
        }
    }
}

void checkOOCollision ()
{
    for( int i = 0 ; i < 4 ; i++ )
        for( int j = i+1 ; j < 5 ; j++ )
            if( (obstacle[i].yc - obstacle[j].yc)*(obstacle[i].yc - obstacle[j].yc) + (obstacle[i].xc - obstacle[j].xc)*(obstacle[i].xc - obstacle[j].xc) <= 1600 )
            {
                if( obstacle[i].vx > 0 )
                    obstacle[i].xc -= 3+3;
                else if( obstacle[i].vx < 0)
                    obstacle[i].xc += 3+3;
                if( obstacle[i].vy > 0 )
                    obstacle[i].yc -= 3+3;
                else if( obstacle[i].vy < 0 )
                    obstacle[i].yc += 3+3;

                if( obstacle[j].vx > 0 )
                    obstacle[j].xc -= 3+3;
                else if( obstacle[j].vx < 0 )
                    obstacle[j].xc += 3+3;
                if( obstacle[j].vy > 0 )
                    obstacle[j].yc -= 3+3;
                else if( obstacle[j].vy < 0 )
                    obstacle[j].yc += 3+3;
                float tempx = obstacle[j].vx;
                float tempy = obstacle[j].vy;
                obstacle[j].vx = obstacle[i].vx;
                obstacle[j].vy = obstacle[i].vy;
                obstacle[i].vx = tempx;
                obstacle[i].vy = tempy;
            }
}

void checkBlockCollision ()
{
    for( int i = 0 ; i < 3 ; i++ )
    {
        glm::vec2 center(ball.xc, ball.yc);
        glm::vec2 aabb_half_extents((block[i].x2 - block[i].x1)/2, (block[i].y2 - block[i].y1)/2);
        glm::vec2 aabb_center(  block[i].x1 + aabb_half_extents.x, block[i].y1 + aabb_half_extents.y);
        glm::vec2 difference = center - aabb_center;
        glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
        glm::vec2 closest = aabb_center + clamped;
        difference = closest - center;
        if( glm::length(difference) < ball.r )
        {
            if( closest.x == block[i].x1 || closest.x == block[i].x2 )
            {
                if( closest.y == block[i].y1 || closest.y == block[i].y2 )
                {
                    ball.vx *= -0.98;
                    ball.vy *= -0.98;
                }
                else
                    ball.vx *= -1;
            }
            else if( closest.y == block[i].y1 || closest.y == block[i].y2 )
            {
                if( closest.x == block[i].x1 || closest.x == block[i].x2 )
                {
                    ball.vy *= -0.98;
                    ball.vx *= -0.98;
                }
                else
                    ball.vy *= -1;
            } 
        }

        for( int j = 0 ; j < 5 ; j++ )
        {
            glm::vec2 center(obstacle[j].xc, obstacle[j].yc);
            glm::vec2 difference = center - aabb_center;
            glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
            glm::vec2 closest = aabb_center + clamped;
            difference = closest - center;
            if( glm::length(difference) < obstacle[j].r )
            {
                if( closest.x == block[i].x1 || closest.x == block[i].x2 )
                {
                    if( closest.y == block[i].y1 || closest.y == block[i].y2 )
                    {
                        obstacle[j].vx *= -0.98;
                        obstacle[j].vy *= -0.98;
                    }
                    else
                        obstacle[j].vx *= -1;
                }
                else if( closest.y == block[i].y1 || closest.y == block[i].y2 )
                {
                    if( closest.x == block[i].x1 || closest.x == block[i].x2 )
                    {
                        obstacle[j].vy *= -0.98;
                        obstacle[j].vx *= -0.98;
                    }
                    else
                        obstacle[j].vy *= -1;
                } 
            }
        }
    }
}

void checkBlock2Collision ()
{
    for( int i = 0 ; i < 3 ; i++ )
    {
        if( block2[i].display )
        {
            // Get center point circle first 
            glm::vec2 center(ball.xc, ball.yc);
            // Calculate AABB info (center, half-extents)
            glm::vec2 aabb_half_extents((block2[i].x2 - block2[i].x1)/2, (block2[i].y2 - block2[i].y1)/2);
            glm::vec2 aabb_center(  block2[i].x1 + aabb_half_extents.x, block2[i].y1 + aabb_half_extents.y);
            // Get difference vector between both centers
            glm::vec2 difference = center - aabb_center;
            glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
            // Add clamped value to AABB_center and we get the value of box closest to circle
            glm::vec2 closest = aabb_center + clamped;
            // Retrieve vector between center circle and closest point AABB and check if length <= radius
            difference = closest - center;
            if( glm::length(difference) < ball.r )
            {
                if( closest.x == block2[i].x1 || closest.x == block2[i].x2 )
                {
                    if( closest.y == block2[i].y1 || closest.y == block2[i].y2 )
                    {
                        ball.vx *= -0.98;
                        ball.vy *= -0.98;
                    }
                    else
                        ball.vx *= -1;
                }
                else if( closest.y == block2[i].y1 || closest.y == block2[i].y2 )
                {
                    if( closest.x == block2[i].x1 || closest.x == block2[i].x2 )
                    {
                        ball.vy *= -0.98;
                        ball.vx *= -0.98;
                    }
                    else
                        ball.vy *= -1;
                }
                block2[i].display = false;
            }
            for( int j = 0 ; j < 5 ; j++ )
            {
                glm::vec2 center(obstacle[j].xc, obstacle[j].yc);
                glm::vec2 difference = center - aabb_center;
                glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
                glm::vec2 closest = aabb_center + clamped;
                difference = closest - center;
                if( glm::length(difference) < obstacle[j].r )
                {
                    if( closest.x == block2[i].x1 || closest.x == block2[i].x2 )
                    {
                        if( closest.y == block2[i].y1 || closest.y == block2[i].y2 )
                        {
                            obstacle[j].vx *= -0.98;
                            obstacle[j].vy *= -0.98;
                        }
                        else
                            obstacle[j].vx *= -1;
                    }
                    else if( closest.y == block2[i].y1 || closest.y == block2[i].y2 )
                    {
                        if( closest.x == block2[i].x1 || closest.x == block2[i].x2 )
                        {
                            obstacle[j].vy *= -0.98;
                            obstacle[j].vx *= -0.98;
                        }
                        else
                            obstacle[j].vy *= -1;
                    } 
                }
            }
        }
    }
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ( GLFWwindow* window)
{
    reshapeWindow (window, width, height);
    // clear the color and depth in the frame buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram (programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target (0, 0, 0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (0, 1, 0);

    // Compute Camera matrix (view)
    // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projection * Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;	// MVP = Projection * View * Model

    /* Render your scene */

    // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
    // glPopMatrix ();

    /* Ground */
    Matrices.model = glm::mat4(1.0f);
    MVP = VP;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(ground);

    /* Speed Rect */
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateSpeedRect = glm::translate (glm::vec3((float)speed*20, 700.0f, 0.0f)); // glTranslatef
    Matrices.model *=  translateSpeedRect;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(speed_rect);

    /* Score */
    int yo = a[score];

    while( yo )
    {
        createSC1();
        createSC2();
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateSpeedRect = glm::translate (glm::vec3(sl[yo%10][0], sl[yo%10][1], 0.0f)); // glTranslatef
        Matrices.model *=  translateSpeedRect;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        if( yo%10 < 5 )
            draw3DObject(sc1);
        else
            draw3DObject(sc2);
        yo  /= 10;
    }

    /* Moving Block */
    for( int i=0 ; i<2 ; i++)
    {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateR = glm::translate (glm::vec3(rblock[i].x1+20.0f, rblock[i].y1+20.0f, 0.0f)); // glTranslatef
        glm::mat4 rotateR = glm::rotate((float)(rblock_rot*M_PI/180.0f), glm::vec3(0,0,1)); 
        Matrices.model *= ( translateR * rotateR);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(rblock[i].obj);
    }
    /* Cannon Base */
    Matrices.model = glm::mat4(1.0f);
    MVP = VP;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(cannon_base);

    /* Cannon Rectangle 2 */
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateCannonRectangle = glm::translate (glm::vec3(90.0f, 150.0f, 0.0f)); // glTranslatef
    glm::mat4 rotateCannonRectangle = glm::rotate((float)(cannon_rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); 
    Matrices.model *= ( translateCannonRectangle * rotateCannonRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(cannon_rectangle);

    /* Cannon Circle */
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateCircle = glm::translate (glm::vec3(90.0f, 150.0f, 0.0f)); // glTranslatef
    Matrices.model *= translateCircle ; 
    MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle);

    /* Outer Ammo */
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateOAmmo = glm::translate (glm::vec3(90.0f, 600.0f, 0.0f)); // glTranslatef
    Matrices.model *= translateOAmmo ; 
    MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(outerammo);

    for( int i = 0 ; i < lives ; i++ )
    {
        createAmmo();
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateAmmo = glm::translate (glm::vec3(90.0f + 50*cos(i*(M_PI/4)), 600.0f + 50*sin(i*(M_PI/4)), 0.0f)); // glTranslatef
        Matrices.model *= translateAmmo ; 
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(ammo);
    }

    /* Ball */
    if( ball.vy*ball.vy + ball.vx*ball.vx < 1 && ball.yc < 65)
        display_ball = false;

    if( display_ball )
    {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateBall = glm::translate (glm::vec3(ball.xc, ball.yc, 0.0f)); // glTranslatef
        Matrices.model *= translateBall ; 
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(ball.obj);
    }

    /* Coin */
    for( int i = 0 ; i < 7 ; i++ )
    {
        if( coin[i].display )
        {
            Matrices.model = glm::mat4(1.0f);
            glm::mat4 translateCoin = glm::translate (glm::vec3(coin[i].xc, coin[i].yc, 0.0f)); // glTranslatef
            Matrices.model *= translateCoin ; 
            MVP = VP * Matrices.model; // MVP = p * V * M
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            draw3DObject(coin[i].obj);
        }
    }

    /* Big Coin */
    if( bcoin.display )
    {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateBCoin = glm::translate (glm::vec3(bcoin.xc, bcoin.yc, 0.0f)); // glTranslatef
        Matrices.model *= translateBCoin ; 
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(bcoin.obj);
    }

    /* Movable Obstacle */
    for( int i = 0 ; i < 5 ; i++ )
    {
        if( obstacle[i].display )
        {
            Matrices.model = glm::mat4(1.0f);
            glm::mat4 translateObstacle = glm::translate (glm::vec3(obstacle[i].xc, obstacle[i].yc, 0.0f)); // glTranslatef
            Matrices.model *= translateObstacle ; 
            MVP = VP * Matrices.model; // MVP = p * V * M
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            draw3DObject(obstacle[i].obj);
        }
    }

    /* Block */
    for( int i = 0 ; i < 3 ; i++ )
    {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateBlock = glm::translate (glm::vec3(block[i].x1, block[i].y1, 0.0f)); // glTranslatef
        Matrices.model *=  translateBlock;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(block[i].obj);
    }

    /* Block 2 */
    for( int i = 0 ; i < 3 ; i++ )
    {
        if( block2[i].display )
        {
            Matrices.model = glm::mat4(1.0f);
            glm::mat4 translateBlock2 = glm::translate (glm::vec3(block2[i].x1, block2[i].y1, 0.0f)); // glTranslatef
            Matrices.model *=  translateBlock2;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            draw3DObject(block2[i].obj);
        }
    }

    // Increment angles
    float increments = 1;

    //camera_rotation_angle++; // Simulating camera rotation
    cannon_rectangle_rotation = cannon_rectangle_rotation + increments*cannon_rectangle_rot_dir*cannon_rectangle_rot_status;
    rblock_rot += 3;
    if( cannon_rectangle_rot_status )
        printf("%f\n",cannon_rectangle_rotation);

    if(display_ball)
    {
        checkCoinCollision();
        checkObstacleCollision();
        calculateBallCoordinates();
    }

    checkBlockCollision();
    checkBlock2Collision();
    checkGroundCollision();
    checkOOCollision();
    checkBCollision();
    calculateObstacleCoordinates();
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
       is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    /* Register function to handle cursor position*/
    glfwSetCursorPosCallback(window, cursorPos);

    /* Register function to handle mouse scroll*/
    glfwSetScrollCallback(window,mouseScroll);

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    // Generate the VAO, VBOs, vertices data & copy into the array buffer
    createGround();
    createSpeedRect();
    createCannonBase();
    createCannonRectangle();
    createCircle();
    createObstacle();
    createBall();
    createCoin();
    createRblock();
    createBlock();
    createBlock2();
    createOuterAmmo();
    createAmmo();
    createBcoin();
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor ((float)0/255, (float)31/255, (float)63/255, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
    GLFWwindow* window = initGLFW(width, height);

    initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw(window);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
