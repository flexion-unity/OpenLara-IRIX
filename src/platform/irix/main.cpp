// OpenLara IRIX/SGI big endian platform layer

#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <SDL2/SDL.h>
#include "game.h"

#define WND_TITLE "OpenLara"

// Timing
unsigned int startTime;

int osGetTimeMS() {
    timeval t;
    gettimeofday(&t, NULL);
    return int((t.tv_sec - startTime) * 1000 + t.tv_usec / 1000);
}

// Sound  (SDL2 callback, 16-bit stereo 44100 Hz)
#define SND_FRAME_SIZE  4       // int16 L + int16 R
#define SND_FRAMES      2048

Sound::Frame      *sndData;
SDL_AudioDeviceID  sdl_audiodev;

void sndFill(void * /*udata*/, Uint8 *stream, int /*len*/) {
    Sound::fill(sndData, SND_FRAMES);
    memcpy(stream, sndData, SND_FRAMES * SND_FRAME_SIZE);
}

bool sndInit() {
    SDL_AudioSpec desired, obtained;
    desired.freq     = 44100;
    desired.format   = AUDIO_S16SYS;
    desired.channels = 2;
    desired.samples  = SND_FRAMES;
    desired.callback = sndFill;
    desired.userdata = NULL;

    sdl_audiodev = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    if (sdl_audiodev == 0) {
        LOG("IRIX: SDL audio error: %s\n", SDL_GetError());
        return false;
    }
    if (desired.samples != obtained.samples) {
        LOG("IRIX: audio sample count mismatch, audio may be buggy\n");
    }

    sndData = new Sound::Frame[SND_FRAMES];
    memset(sndData, 0, SND_FRAMES * SND_FRAME_SIZE);
    SDL_PauseAudioDevice(sdl_audiodev, 0);
    return true;
}

void sndFree() {
    SDL_PauseAudioDevice(sdl_audiodev, 1);
    SDL_CloseAudioDevice(sdl_audiodev);
    delete[] sndData;
}

// Input
#define MAX_JOYS 4
#define JOY_DEAD_ZONE_STICK  8192
#define WIN_W 1024
#define WIN_H 768

int   sdl_numjoysticks;
int   sdl_numcontrollers;
SDL_Joystick     *sdl_joysticks[MAX_JOYS];
SDL_GameController *sdl_controllers[MAX_JOYS];
SDL_Haptic        *sdl_haptics[MAX_JOYS];
SDL_Window        *sdl_window;
SDL_DisplayMode    sdl_displaymode;
bool               fullscreen;
vec2               joyL, joyR;

bool osJoyReady(int index) { return index == 0; }

void osJoyVibrate(int index, float L, float R) {
    if (index >= sdl_numcontrollers) return;
    if (SDL_IsGameController(index))
        SDL_GameControllerRumble(sdl_controllers[index], (Uint16)(L * 0xFFFF), (Uint16)(R * 0xFFFF), 500);
    else
        SDL_HapticRumblePlay(sdl_haptics[index], L + R, 500);
}

static bool isKeyPressed(SDL_Scancode sc) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    return state[sc] != 0;
}

void toggleFullscreen() {
    fullscreen = !fullscreen;
    Uint32 flags = fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
    SDL_SetWindowFullscreen(sdl_window, flags);
    Core::width  = fullscreen ? sdl_displaymode.w : WIN_W;
    Core::height = fullscreen ? sdl_displaymode.h : WIN_H;
}

InputKey codeToInputKey(int code) {
    switch (code) {
        case SDL_SCANCODE_LEFT   : return ikLeft;
        case SDL_SCANCODE_RIGHT  : return ikRight;
        case SDL_SCANCODE_UP     : return ikUp;
        case SDL_SCANCODE_DOWN   : return ikDown;
        case SDL_SCANCODE_SPACE  : return ikSpace;
        case SDL_SCANCODE_TAB    : return ikTab;
        case SDL_SCANCODE_RETURN : return ikEnter;
        case SDL_SCANCODE_ESCAPE : return ikEscape;
        case SDL_SCANCODE_LSHIFT :
        case SDL_SCANCODE_RSHIFT : return ikShift;
        case SDL_SCANCODE_LCTRL  :
        case SDL_SCANCODE_RCTRL  : return ikCtrl;
        case SDL_SCANCODE_LALT   :
        case SDL_SCANCODE_RALT   : return ikAlt;
        case SDL_SCANCODE_0      : return ik0;
        case SDL_SCANCODE_1      : return ik1;
        case SDL_SCANCODE_2      : return ik2;
        case SDL_SCANCODE_3      : return ik3;
        case SDL_SCANCODE_4      : return ik4;
        case SDL_SCANCODE_5      : return ik5;
        case SDL_SCANCODE_6      : return ik6;
        case SDL_SCANCODE_7      : return ik7;
        case SDL_SCANCODE_8      : return ik8;
        case SDL_SCANCODE_9      : return ik9;
        case SDL_SCANCODE_A      : return ikA;
        case SDL_SCANCODE_B      : return ikB;
        case SDL_SCANCODE_C      : return ikC;
        case SDL_SCANCODE_D      : return ikD;
        case SDL_SCANCODE_E      : return ikE;
        case SDL_SCANCODE_F      : return ikF;
        case SDL_SCANCODE_G      : return ikG;
        case SDL_SCANCODE_H      : return ikH;
        case SDL_SCANCODE_I      : return ikI;
        case SDL_SCANCODE_J      : return ikJ;
        case SDL_SCANCODE_K      : return ikK;
        case SDL_SCANCODE_L      : return ikL;
        case SDL_SCANCODE_M      : return ikM;
        case SDL_SCANCODE_N      : return ikN;
        case SDL_SCANCODE_O      : return ikO;
        case SDL_SCANCODE_P      : return ikP;
        case SDL_SCANCODE_Q      : return ikQ;
        case SDL_SCANCODE_R      : return ikR;
        case SDL_SCANCODE_S      : return ikS;
        case SDL_SCANCODE_T      : return ikT;
        case SDL_SCANCODE_U      : return ikU;
        case SDL_SCANCODE_V      : return ikV;
        case SDL_SCANCODE_W      : return ikW;
        case SDL_SCANCODE_X      : return ikX;
        case SDL_SCANCODE_Y      : return ikY;
        case SDL_SCANCODE_Z      : return ikZ;
        case SDL_SCANCODE_AC_HOME: return ikEscape;
    }
    return ikNone;
}

JoyKey controllerCodeToJoyKey(int code) {
    switch (code) {
        case SDL_CONTROLLER_BUTTON_A             : return jkA;
        case SDL_CONTROLLER_BUTTON_B             : return jkB;
        case SDL_CONTROLLER_BUTTON_X             : return jkX;
        case SDL_CONTROLLER_BUTTON_Y             : return jkY;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER  : return jkLB;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER : return jkRB;
        case SDL_CONTROLLER_BUTTON_BACK          : return jkSelect;
        case SDL_CONTROLLER_BUTTON_START         : return jkStart;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK     : return jkL;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK    : return jkR;
        case SDL_CONTROLLER_BUTTON_DPAD_UP       : return jkUp;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN     : return jkDown;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT     : return jkLeft;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT    : return jkRight;
    }
    return jkNone;
}

JoyKey joyCodeToJoyKey(int btn) {
    switch (btn) {
        case 0: return jkY;  case 1: return jkB;  case 2: return jkA;
        case 3: return jkX;  case 4: return jkL;  case 5: return jkR;
        case 6: return jkLB; case 7: return jkRB; case 8: return jkSelect;
        case 9: return jkStart;
    }
    return jkNone;
}

int joyGetIndex(SDL_JoystickID id) {
    for (int i = 0; i < sdl_numjoysticks; i++)
        if (SDL_JoystickInstanceID(sdl_joysticks[i]) == id)
            return i;
    return -1;
}

bool joyIsController(Sint32 instanceID) {
    for (int i = 0; i < sdl_numcontrollers; i++)
        if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(sdl_controllers[i])) == instanceID)
            return true;
    return false;
}

void joyAdd(int index) {
    if (SDL_IsGameController(index)) {
        SDL_GameController *c = SDL_GameControllerOpen(index);
        sdl_controllers[sdl_numcontrollers++] = c;
        sdl_joysticks[index] = SDL_GameControllerGetJoystick(c);
    } else {
        sdl_joysticks[index] = SDL_JoystickOpen(index);
        sdl_haptics[index] = SDL_HapticOpenFromJoystick(sdl_joysticks[index]);
        SDL_HapticRumbleInit(sdl_haptics[index]);
    }
    sdl_numjoysticks = SDL_NumJoysticks();
    if (sdl_numjoysticks > MAX_JOYS) sdl_numjoysticks = MAX_JOYS;
}

void joyRemove(Sint32 instanceID) {
    if (joyIsController(instanceID)) {
        for (int i = 0; i < sdl_numcontrollers; i++) {
            if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(sdl_controllers[i])) == instanceID) {
                SDL_GameControllerClose(sdl_controllers[i]);
                sdl_controllers[i] = NULL;
                sdl_numcontrollers--;
                sdl_numjoysticks--;
            }
        }
    } else {
        int i = joyGetIndex(instanceID);
        if (i >= 0) {
            SDL_JoystickClose(sdl_joysticks[i]);
            SDL_HapticClose(sdl_haptics[i]);
            sdl_haptics[i] = NULL;
            sdl_numjoysticks--;
        }
    }
}

bool inputInit() {
    joyL = joyR = vec2(0);
    sdl_numjoysticks = SDL_NumJoysticks();
    if (sdl_numjoysticks > MAX_JOYS) sdl_numjoysticks = MAX_JOYS;

    for (int i = 0; i < MAX_JOYS; i++) sdl_joysticks[i] = NULL;
    for (int i = 0; i < sdl_numjoysticks; i++) joyAdd(i);
    return true;
}

void inputFree() {
    for (int i = 0; i < sdl_numjoysticks; i++)
        joyRemove(SDL_JoystickInstanceID(sdl_joysticks[i]));
}

float joyAxisValue(int value) {
    if (value > -JOY_DEAD_ZONE_STICK && value < JOY_DEAD_ZONE_STICK) return 0.0f;
    return value / 32767.0f;
}

float joyTrigger(int value) { return min(1.0f, value / 255.0f); }

vec2 joyDir(const vec2 &v) {
    float d = min(1.0f, v.length());
    return v.normal() * d;
}

void inputUpdate() {
    int joyIndex;
    SDL_Event event;

    while (SDL_PollEvent(&event) == 1) {
        switch (event.type) {
            case SDL_QUIT:
                Core::isQuit = true;
                break;

            case SDL_KEYDOWN: {
                int sc = event.key.keysym.scancode;
                InputKey key = codeToInputKey(sc);
                if (key != ikNone) Input::setDown(key, 1);
                if (sc == SDL_SCANCODE_RETURN &&
                    isKeyPressed(SDL_SCANCODE_LALT) && isKeyPressed(SDL_SCANCODE_RETURN))
                    toggleFullscreen();
                break;
            }
            case SDL_KEYUP: {
                InputKey key = codeToInputKey(event.key.keysym.scancode);
                if (key != ikNone) Input::setDown(key, 0);
                break;
            }

            case SDL_CONTROLLERBUTTONDOWN: {
                joyIndex = joyGetIndex(event.cbutton.which);
                Input::setJoyDown(joyIndex, controllerCodeToJoyKey(event.cbutton.button), 1);
                break;
            }
            case SDL_CONTROLLERBUTTONUP: {
                joyIndex = joyGetIndex(event.cbutton.which);
                Input::setJoyDown(joyIndex, controllerCodeToJoyKey(event.cbutton.button), 0);
                break;
            }
            case SDL_CONTROLLERAXISMOTION: {
                joyIndex = joyGetIndex(event.caxis.which);
                switch (event.caxis.axis) {
                    case SDL_CONTROLLER_AXIS_LEFTX:  joyL.x = joyAxisValue(event.caxis.value); break;
                    case SDL_CONTROLLER_AXIS_LEFTY:  joyL.y = joyAxisValue(event.caxis.value); break;
                    case SDL_CONTROLLER_AXIS_RIGHTX: joyR.x = joyAxisValue(event.caxis.value); break;
                    case SDL_CONTROLLER_AXIS_RIGHTY: joyR.y = joyAxisValue(event.caxis.value); break;
                }
                Input::setJoyPos(joyIndex, jkL, joyDir(joyL));
                Input::setJoyPos(joyIndex, jkR, joyDir(joyR));
                break;
            }
            case SDL_CONTROLLERDEVICEADDED:   joyAdd(event.cdevice.which);    break;
            case SDL_CONTROLLERDEVICEREMOVED: joyRemove(event.cdevice.which); break;

            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            case SDL_JOYAXISMOTION:
            case SDL_JOYDEVICEADDED:
            case SDL_JOYDEVICEREMOVED:
                if (SDL_IsGameController(joyGetIndex(event.jbutton.which))) break;
                switch (event.type) {
                    case SDL_JOYBUTTONDOWN: {
                        joyIndex = joyGetIndex(event.jbutton.which);
                        Input::setJoyDown(joyIndex, joyCodeToJoyKey(event.jbutton.button), 1);
                        break;
                    }
                    case SDL_JOYBUTTONUP: {
                        joyIndex = joyGetIndex(event.jbutton.which);
                        Input::setJoyDown(joyIndex, joyCodeToJoyKey(event.jbutton.button), 0);
                        break;
                    }
                    case SDL_JOYAXISMOTION: {
                        joyIndex = joyGetIndex(event.jaxis.which);
                        switch (event.jaxis.axis) {
                            case 0: joyL.x = joyAxisValue(event.jaxis.value); break;
                            case 1: joyL.y = joyAxisValue(event.jaxis.value); break;
                            case 2: joyR.x = joyAxisValue(event.jaxis.value); break;
                            case 3: joyR.y = joyAxisValue(event.jaxis.value); break;
                        }
                        Input::setJoyPos(joyIndex, jkL, joyDir(joyL));
                        Input::setJoyPos(joyIndex, jkR, joyDir(joyR));
                        break;
                    }
                    case SDL_JOYDEVICEADDED:   joyAdd(event.jdevice.which);    break;
                    case SDL_JOYDEVICEREMOVED: joyRemove(event.jdevice.which); break;
                }
                break;
        }
    }
}

// Entry point
static void print_help(int argc, char **argv) {
    printf("%s [OPTION]\nOpenLara\n",
           argc ? argv[0] : "openlara");
    puts("-d [DIR]   directory where data files are");
    puts("-l [FILE]  load a specific level file");
    puts("-f         start in fullscreen mode");
    puts("-h         print this help");
}

int main(int argc, char **argv) {
    cacheDir[0] = saveDir[0] = contentDir[0] = 0;
    char *lvlName = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "hl:d:f")) != -1) {
        switch (opt) {
            case 'h': print_help(argc, argv); return 0;
            case 'l': lvlName = optarg; break;
            case 'd':
                strncpy(contentDir, optarg, 254);
                break;
            case 'f': fullscreen = true; break;
            default : print_help(argc, argv); return -1;
        }
    }

    {
        size_t n = strlen(contentDir);
        if (n > 0 && contentDir[n-1] != '/' && n < 254) {
            contentDir[n]   = '/';
            contentDir[n+1] = '\0';
        }
    }

    SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS |
             SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);

    SDL_GetCurrentDisplayMode(0, &sdl_displaymode);

    // Request a plain OpenGL 1.x context
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (fullscreen) {
        sdl_window = SDL_CreateWindow(WND_TITLE,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            sdl_displaymode.w, sdl_displaymode.h,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        sdl_window = SDL_CreateWindow(WND_TITLE,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WIN_W, WIN_H,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    }

    if (!sdl_window) {
        LOG("IRIX: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    int w, h;
    SDL_GetWindowSize(sdl_window, &w, &h);
    Core::width  = w;
    Core::height = h;

    SDL_GLContext ctx = SDL_GL_CreateContext(sdl_window);
    if (!ctx) {
        LOG("IRIX: SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_ShowCursor(SDL_DISABLE);

    // Build cache/save paths
    const char *home = getenv("HOME");
    if (!home) home = getpwuid(getuid())->pw_dir;
    strncat(cacheDir, home, sizeof(cacheDir) - 2);
    strcat(cacheDir, "/.openlara/");

    struct stat st = {0};
    if (stat(cacheDir, &st) == -1 && mkdir(cacheDir, 0777) == -1)
        cacheDir[0] = 0;
    strcpy(saveDir, cacheDir);

    timeval t;
    gettimeofday(&t, NULL);
    startTime = t.tv_sec;

    sndInit();
    inputInit();
    Game::init(lvlName);

    while (!Core::isQuit) {
        inputUpdate();
        if (Game::update()) {
            Game::render();
            Core::waitVBlank();
            SDL_GL_SwapWindow(sdl_window);
        }
    }

    sndFree();
    Game::deinit();
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}
