// Make sure you add the SDL2 library to the library list when compiling: -lSDL2

// TO COMPILE!!!! gcc -o tute1 tute1.c -F/Library/Frameworks -framework Carbon -framework OpenGL -framework SDL2 -Wno-deprecated

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#if _WIN32
#  include <Windows.h>
#endif
#if __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#endif

#include <SDL2/SDL.h>
#include "gl.h"
#include "util.h"
#include "state.h"
#include "player.h"
#include "level.h"
#include "counters.h"
#include "OSD.h"


Globals globals;
SDL_Window *mainWindow = 0;
int num_objects;
int num_car_triangles;
extern int num_triangles;

static void cleanup() {
    destroyPlayer(&globals.player);
    destroyLevel(&globals.level);
}


static void
updateKeyStatus(SDL_Keysym *key, bool state)
{
    switch (key->sym) {
        case 'w': // Move up
        globals.controls.up = state;
        break;
        case 's': // Move down
        globals.controls.down = state;
        break;
        case 'a': // Move left
        globals.controls.left = state;
        break;
        case 'd': // Move right
        globals.controls.right = state;
        break;
        case ' ': // Junp
        globals.controls.jump = state;
        case SDLK_LEFT: // Turn camera left
        globals.controls.turnLeft = state;
        break;
        case SDLK_RIGHT: // Turn camera right
        globals.controls.turnRight = state;
        break;
        default:
        break;
    }
}

bool handleKeyDown(SDL_Keysym *key){
    switch (key->sym){
        case SDLK_ESCAPE:
        case 'q':
        return true;
        break;
        case 'h': // Pauses game time
        globals.halt = !globals.halt;
        if (globals.halt)
        printf("Stopping time\n");
        else
        printf("Resuming time\n");
        break;
        case 'c': // Toggle OSD
        printf("Toggling OSD\n");
        globals.drawingFlags.osd = !globals.drawingFlags.osd;
        break;
        case 'l': // Toggle lighting
        globals.drawingFlags.lighting = !globals.drawingFlags.lighting;
        printf("Toggling lighting\n");
        break;
        case 't': // Toggle textures
        globals.drawingFlags.textures = !globals.drawingFlags.textures;
        printf("Toggling textures\n");
        break;
        case 'n':
        globals.drawingFlags.normals = !globals.drawingFlags.normals;
        printf("Toggling normals\n");
        break;
        case 'o': // Toggle axis
        globals.drawingFlags.axes = !globals.drawingFlags.axes;
        printf("Toggling axes\n");
        break;
        case 'm': // Change render mode
        globals.drawingFlags.rm++;
        if (globals.drawingFlags.rm >= nrms)
        globals.drawingFlags.rm = im;
        printf("Changing RM (%i)\n", globals.drawingFlags.rm);
        break;
        case 'p': // Toggle wireframe
        printf("Wireframe triggered\n");
        globals.drawingFlags.wireframe = !globals.drawingFlags.wireframe;
        if (globals.drawingFlags.wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            printf("Using wireframe rendering\n");
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            printf("Using filled rendering\n");
        }
        break;
        case '+': // Increase Tesselation
        case '=':
        globals.drawingFlags.tess[0] =
        clamp(globals.drawingFlags.tess[0] * 2, 8, 1024);
        globals.drawingFlags.tess[1] =
        clamp(globals.drawingFlags.tess[1] * 2, 8, 1024);
        generatePlayerGeometry(&globals.player, globals.drawingFlags.tess);
        generateLevelGeometry(&globals.level, globals.drawingFlags.tess);
        printf("Tesselation: %zu %zu\n",
        globals.drawingFlags.tess[0], globals.drawingFlags.tess[1]);
        globals.counters.num_triangles = num_objects * globals.drawingFlags.tess[1] * globals.drawingFlags.tess[0] * 2 + num_car_triangles;
        break;
        case '-': // Decrease tesselation
        globals.drawingFlags.tess[0] =
        clamp(globals.drawingFlags.tess[0] / 2, 8, 1024);
        globals.drawingFlags.tess[1] =
        clamp(globals.drawingFlags.tess[1] / 2, 8, 1024);
        generatePlayerGeometry(&globals.player, globals.drawingFlags.tess);
        generateLevelGeometry(&globals.level, globals.drawingFlags.tess);
        printf("Tesselation: %zu %zu\n",
        globals.drawingFlags.tess[0], globals.drawingFlags.tess[1]);
        globals.counters.num_triangles = num_objects * globals.drawingFlags.tess[1] * globals.drawingFlags.tess[0] * 2 + num_car_triangles;
        break;
        default:
        // NOTE This may break was originally updateKeyChar(key, true);
        updateKeyStatus(key, true); // player controls are updated here and processed on each frame
        break;
    }
    return false;
}

// Quick control method to change pressed status of certain buttons
// Look at updateKeyStatus()
bool handleKeyUp(SDL_Keysym *key){
    updateKeyStatus(key, false);
    return false;
}

// Handle mouse motion
static void
mouseMotion(int x, int y)
{
    //printf("motion x: %i\ty:%i\n", x, y);
    int dX = x - globals.camera.lastX;
    int dY = y - globals.camera.lastY;

    if (globals.controls.lmb) {
        globals.camera.xRot += dX * 0.1;
        globals.camera.yRot += dY * 0.1;
        globals.camera.yRot = clamp(globals.camera.yRot, 0, 90);
    }

    if (globals.controls.rmb) {
        globals.camera.zoom += dY * 0.01;
        globals.camera.zoom = max(globals.camera.zoom, 0.5);
    }

    globals.camera.lastX = x;
    globals.camera.lastY = y;
}

// Handle mouse button pressess
static void
mouseButton(int button, int state, int x, int y)
{
    //printf("button(%i): %i\tstate(%i): %i\n", SDL_BUTTON_LEFT, button, SDL_PRESSED, state);

    globals.camera.lastX = x;
    globals.camera.lastY = y;
    if (state == SDL_PRESSED) {
        if (button == SDL_BUTTON_LEFT) {
            globals.controls.lmb = state == SDL_PRESSED;
        } else if (button == SDL_BUTTON_RIGHT) {
            globals.controls.rmb = state == SDL_PRESSED;
        }
    }
    else if (state == SDL_RELEASED){
        if (button == SDL_BUTTON_LEFT) {
            globals.controls.lmb = state;
        } else if (button == SDL_BUTTON_RIGHT) {
            globals.controls.rmb = state;
        }
    }


}

bool handleEvents(){
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEMOTION:
            mouseMotion(event.motion.x, event.motion.y);
            break;
            case SDL_MOUSEBUTTONDOWN:
            mouseButton(event.button.button, event.button.state, event.button.x, event.button.y);
            break;
            case SDL_MOUSEBUTTONUP:
            mouseButton(event.button.button, event.button.state, event.button.x, event.button.y);
            break;
            case SDL_KEYDOWN: // Handle Keyups
            return handleKeyDown(&event.key.keysym);
            case SDL_KEYUP: // Handle key downs
            return handleKeyUp(&event.key.keysym);
            break;
            case SDL_QUIT:
            return true;
            break;
            case SDL_WINDOWEVENT:
            switch(event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                if(event.window.windowID == SDL_GetWindowID(mainWindow)){ // FIXME remove curly
                    SDL_SetWindowSize(mainWindow, event.window.data1, event.window.data2);
                }
                break;
                case SDL_WINDOWEVENT_CLOSE:
                return true;
                break;
                default:
                break;
            }
            default:
            break;
        }
    }
    return false;
}

static void
reshape(int width, int height) // FIXME not working
{
    glViewport(0,0, (GLsizei) width, (GLsizei) height);
    globals.camera.width = width;
    globals.camera.height = height;
    applyProjectionMatrix(&globals.camera);
}

static void checkReshape(){
    int tmp_x, tmp_y;
    SDL_GetWindowSize(mainWindow, &tmp_x, &tmp_y);
    if(tmp_x != globals.windowx || tmp_y != globals.windowy){
        globals.windowx = tmp_x;
        globals.windowy = tmp_y;
        reshape(globals.windowx, globals.windowy);
    }
}

static void
render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    applyViewMatrix(&globals.camera);

    static float lightPos[] = { 1, 1, 1, 0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    renderPlayer(&globals.player, &globals.drawingFlags);
    renderLevel(&globals.level, &globals.drawingFlags);

    displayOSD(&globals.counters, &globals.drawingFlags, globals.windowx, globals.windowy);

    SDL_GL_SwapWindow(mainWindow);

    globals.counters.frameCount++;
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
        printf("display: %s\n", gluErrorString(err));
    }
}

static void
update()
{
    static int tLast = -1;

    if (tLast < 0)
    tLast = SDL_GetTicks();

    int t = SDL_GetTicks();
    int dtMs = t - tLast;
    float dt = (float)dtMs / 1000.0f;
    tLast = t;

    if (!globals.halt) {
        updatePlayer(&globals.player, dt, &globals.controls);
        updateLevel(&globals.level, dt);
        updateCounters(&globals.counters, t);
        globals.camera.pos = globals.player.pos;
    };
}


static void
init()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    globals.drawingFlags.tess[0] = 8;
    globals.drawingFlags.tess[1] = 8;
    globals.drawingFlags.wireframe = false;
    globals.drawingFlags.textures = true;
    globals.drawingFlags.lighting = true;
    globals.drawingFlags.osd = true;
    globals.drawingFlags.rm = im;
    initPlayer(&globals.player, &globals.drawingFlags);
    initLevel(&globals.level, &globals.drawingFlags);
    initCamera(&globals.camera);
    initCounters(&globals.counters);
    globals.camera.pos = globals.player.pos;
    globals.camera.width = 800;
    globals.camera.height = 600;

    //NUmber of objects = num cars + num logs + 1 landscape + 1 player + osd?
    num_objects = globals.level.river.numLanes + 1 + 1;
    num_car_triangles = globals.level.road.numLanes * 6 * 2;
    globals.counters.num_triangles = num_objects * globals.drawingFlags.tess[1] * globals.drawingFlags.tess[0] * 2 + num_car_triangles;
    // VBO
    //glBindBuffer(GL_ARRAY_BUFFER, globals.vbo);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, globals.ibo);m
}

int main(int argc, char const *argv[])
{
    #if __APPLE__
    UNUSED(argc);
    UNUSED(argv);
    #else
    glutInit(&argc, argv);
    #endif
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) { // Returns -1 on error
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    mainWindow = SDL_CreateWindow("Frogger Microbenchmark", 0, 0, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);
    if(!mainWindow) { // Main window still = 0
        fprintf(stderr, "Failed to create a window: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GLContext mainGLContext = SDL_GL_CreateContext(mainWindow);
    if(mainGLContext == 0) {
        fprintf(stderr, "Unable to get OpenGL context: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (SDL_GL_MakeCurrent(mainWindow, mainGLContext) != 0) {
        fprintf(stderr, "Unable to make OpenGL context current: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);


    init();

    SDL_GetWindowSize(mainWindow, &globals.windowx, &globals.windowy);
    reshape(globals.windowx, globals.windowy);

    bool done = false;
    // Main event and display loop goes here
    while(!done) {
        checkReshape();
        done = handleEvents();
        update();
        render();
    }
    cleanup();
    SDL_DestroyWindow(mainWindow);
    SDL_Quit();

    return EXIT_SUCCESS;
}
