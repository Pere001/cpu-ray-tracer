/*

 This is a simple multithreaded CPU raytracer.

* Controls: WASD to move, Space to ascend, Shift to descend, left click to orient camera,
  Escape to exit.

* Uses WINAPI for input, threads, and window stuff.

* Worker threads render groups of rows of pixels into a common frame buffer which is     
  then sent to the GPU, and each frame rendered via OpenGL.

* It only supports spheres and axis-aligned planes.

* Each pixel that hits a shape shoots one light ray and one reflection ray. The reflection
  doesn't bounce and isn't shaded. Pixels are shaded using the Blinn-Phong reflectivity
  model. The shading could easily and cheaply be improved to make more different materials
  and allow lights of different colors.

* Soft shadows are computed via a hack I came up with, which only works for spherical
  lights and spherical blockers. You project each blocker sphere into the plane
  perpendicular to the light ray, which contains the sphere's center. Imagine a "cone of
  vision", which is a truncated cone extending from the pixel position to the light
  position, defining the space where objects would block the pixel's light. So, compute
  the radius of the section of the "cone of vision" that's on the plane we projected
  the sphere to. Now that we have the cone's projected circle and the sphere's projected
  circle, to find out how much light is blocked we just need to find how much of the
  area of the cone's circle intersects the sphere's circle. To do that, we use a cheap
  approximation using the distance that the sphere's circle penetrates the cone's circle.
  Basically we take this distance and we square it.
  If multiple spheres block some light, the final value of light for the pixel will be a
  mix of 3 different ways of accumulating that blocked light: the maximum light blocked
  by a single sphere, the sum (clamped to 0), and the sum divided by the number of
  spheres that blocked any light. I just experimented a bit and came up with these values
  and their weights to reduce some artifacts that ocurred when only using one value.

* The coordinate system is left-handed: +X is right, +Z is forward, and +Y is up. This
  means that the cross product follows the left hand rule. Angles are counterclockwise and
  follow the right hand rule (thumb points to the direction of the axis of rotation).

* Six spheres and one plane render at 60 FPS, at a 640x480 resolution, in my not so good
  2019 laptop with 8 logical cores.

 */




#define CREATE_CONSOLE false
#define NUM_WORKER_THREADS 7
#define FRAME_BUFFER_WIDTH 640
#define FRAME_BUFFER_HEIGHT 480





#include <windows.h>
#include <stdio.h>
#include <malloc.h>

#include "base.h"
#include "math.h"

#include <Windows.h>
#include <intrin.h>

#include <wingdi.h>
#include <GL/gl.h>


//
// Globals
//
static b32 globalRunning = true;

static LARGE_INTEGER globalPerformanceFrequency;// counts per second
static HANDLE globalStdHandle = {};

//
// OpenGL Declarations
//
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_ARRAY_BUFFER                   0x8892
#define GL_READ_ONLY                      0x88B8
#define GL_WRITE_ONLY                     0x88B9
#define GL_READ_WRITE                     0x88BA
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_DELETE_STATUS                  0x8B80
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_MAX_VERTEX_ATTRIBS             0x8869

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

// NOTE: The 'WINAPI' thing specifies the calling convention and will only be necessary when compiling for 32-bit x86.
typedef BOOL   WINAPI wgl_swap_interval_ext(int interval);
typedef void   WINAPI gl_attach_shader(GLuint program, GLuint shader);
typedef void   WINAPI gl_compile_shader(GLuint shader);
typedef GLuint WINAPI gl_create_program(void);
typedef GLuint WINAPI gl_create_shader(GLenum type);
typedef void   WINAPI gl_delete_program(GLuint program);
typedef void   WINAPI gl_delete_shader(GLuint shader);
typedef void   WINAPI gl_detach_shader(GLuint program, GLuint shader);
typedef void   WINAPI gl_link_program(GLuint program);
typedef void   WINAPI gl_shader_source(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void   WINAPI gl_use_program(GLuint program);
typedef void   WINAPI gl_gen_buffers(GLsizei n, GLuint *buffers);
typedef void   WINAPI gl_bind_buffer(GLenum target, GLuint buffer);
typedef void   WINAPI gl_delete_buffers (GLsizei n, const GLuint *buffers);
typedef void   WINAPI gl_buffer_data(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void   WINAPI gl_get_programiv(GLuint program, GLenum pname, GLint *params);
typedef void   WINAPI gl_get_program_info_log(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void   WINAPI gl_get_shaderiv(GLuint shader, GLenum pname, GLint *params);
typedef void   WINAPI gl_get_shader_info_log(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLint  WINAPI gl_get_attrib_location(GLuint program, const GLchar *name);
typedef void   WINAPI gl_vertex_attrib_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void   WINAPI gl_enable_vertex_attrib_array(GLuint index);
typedef void   WINAPI gl_bind_vertex_array(GLuint array);
typedef void   WINAPI gl_delete_vertex_arrays(GLsizei n, const GLuint *arrays);
typedef void   WINAPI gl_gen_vertex_arrays(GLsizei n, GLuint *arrays);
typedef void   WINAPI gl_uniform_1f(GLint location, GLfloat v0);
typedef void   WINAPI gl_uniform_2f(GLint location, GLfloat v0, GLfloat v1);
typedef void   WINAPI gl_uniform_3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void   WINAPI gl_uniform_4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void   WINAPI gl_uniform_1i(GLint location, GLint v0);
typedef void   WINAPI gl_uniform_2i(GLint location, GLint v0, GLint v1);
typedef void   WINAPI gl_uniform_3i(GLint location, GLint v0, GLint v1, GLint v2);
typedef void   WINAPI gl_uniform_4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void   WINAPI gl_uniform_1fv(GLint location, GLsizei count, const GLfloat *value);
typedef void   WINAPI gl_uniform_2fv(GLint location, GLsizei count, const GLfloat *value);
typedef void   WINAPI gl_uniform_3fv(GLint location, GLsizei count, const GLfloat *value);
typedef void   WINAPI gl_uniform_4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void   WINAPI gl_uniform_1iv(GLint location, GLsizei count, const GLint *value);
typedef void   WINAPI gl_uniform_2iv(GLint location, GLsizei count, const GLint *value);
typedef void   WINAPI gl_uniform_3iv(GLint location, GLsizei count, const GLint *value);
typedef void   WINAPI gl_uniform_4iv(GLint location, GLsizei count, const GLint *value);
typedef void   WINAPI gl_uniform_matrix_2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void   WINAPI gl_uniform_matrix_3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void   WINAPI gl_uniform_matrix_4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef GLint  WINAPI gl_get_uniform_location(GLuint program, const GLchar *name);
typedef void   WINAPI gl_generate_mipmap(GLenum target);
typedef void   WINAPI gl_active_texture(GLenum texture);

global wgl_swap_interval_ext         *wglSwapInterval;
global gl_attach_shader              *glAttachShader;
global gl_compile_shader             *glCompileShader;
global gl_create_program             *glCreateProgram;
global gl_create_shader              *glCreateShader;
global gl_delete_program             *glDeleteProgram;
global gl_delete_shader              *glDeleteShader;
global gl_detach_shader              *glDetachShader;
global gl_link_program               *glLinkProgram;
global gl_shader_source              *glShaderSource;
global gl_use_program                *glUseProgram;
global gl_gen_buffers                *glGenBuffers;
global gl_bind_buffer                *glBindBuffer;
global gl_delete_buffers             *glDeleteBuffers; 
global gl_buffer_data                *glBufferData;
global gl_get_programiv              *glGetProgramiv;
global gl_get_program_info_log       *glGetProgramInfoLog;
global gl_get_shaderiv               *glGetShaderiv;
global gl_get_shader_info_log        *glGetShaderInfoLog;
global gl_get_attrib_location        *glGetAttribLocation;
global gl_vertex_attrib_pointer      *glVertexAttribPointer;
global gl_enable_vertex_attrib_array *glEnableVertexAttribArray;
global gl_bind_vertex_array          *glBindVertexArray;
global gl_delete_vertex_arrays       *glDeleteVertexArrays;
global gl_gen_vertex_arrays          *glGenVertexArrays;
global gl_uniform_1f                 *glUniform1f;
global gl_uniform_2f                 *glUniform2f;
global gl_uniform_3f                 *glUniform3f;
global gl_uniform_4f                 *glUniform4f;
global gl_uniform_1i                 *glUniform1i;
global gl_uniform_2i                 *glUniform2i;
global gl_uniform_3i                 *glUniform3i;
global gl_uniform_4i                 *glUniform4i;
global gl_uniform_1fv                *glUniform1fv;
global gl_uniform_2fv                *glUniform2fv;
global gl_uniform_3fv                *glUniform3fv;
global gl_uniform_4fv                *glUniform4fv;
global gl_uniform_1iv                *glUniform1iv;
global gl_uniform_2iv                *glUniform2iv;
global gl_uniform_3iv                *glUniform3iv;
global gl_uniform_4iv                *glUniform4iv;
global gl_uniform_matrix_2fv         *glUniformMatrix2fv;
global gl_uniform_matrix_3fv         *glUniformMatrix3fv;
global gl_uniform_matrix_4fv         *glUniformMatrix4fv;
global gl_get_uniform_location       *glGetUniformLocation;
global gl_generate_mipmap            *glGenerateMipmap;
global gl_active_texture             *glActiveTexture;

//
// Input
//
struct keyboard_input{
    union{
        button_state asArray[48];
        struct{
            button_state letters[26];
            button_state numbers[10];
            button_state escape;
            button_state enter;
            button_state space;
            button_state shift;
            button_state control;
            button_state backspace;
            button_state alt;
            button_state tab;
            button_state arrowLeft;
            button_state arrowRight;
            button_state arrowUp;
            button_state arrowDown;
        };
    };
};
struct input_state {
    keyboard_input keyboard;
    button_state mouseButtons[5];
    v2 mousePos;
    v2 windowDim;
 
};

void UpdateButtonState(button_state *b, b32 wentDown){
    if (b->isDown && !wentDown){
        b->isDown = false;
        b->transitionCount++;
    }else if (!b->isDown && wentDown){
        b->isDown = true;
        b->transitionCount++;
    }
}

static input_state globalInput = {};




//
// Some WINAPI stuff
//

LARGE_INTEGER GetCurrentTimeCounter(){
    LARGE_INTEGER result = {};
    QueryPerformanceCounter(&result);
    return result;
}

f32 GetSecondsElapsed(LARGE_INTEGER t0, LARGE_INTEGER t1){
    f32 result = (f32)(t1.QuadPart - t0.QuadPart) / (f32)globalPerformanceFrequency.QuadPart;
    return result;
}

inline void Print(char *str){
    WriteFile(globalStdHandle, str, (DWORD)strlen(str), 0, 0);
}
inline void Printf(char *format, ...){
    va_list args;
    va_start(args, format);

    char str[2048];
    vsprintf_s(str, format, args);
    str[ArrayCount(str)-1] = 0; // Null-Terminate
    Print(str);

    va_end(args);
}
inline void DebugPrint(char *str){
    OutputDebugStringA(str);
}
inline void DebugPrintf(char *format, ...){
    va_list args;
    va_start(args, format);

    char str[2048];
    vsprintf_s(str, format, args);
    str[ArrayCount(str)-1] = 0; // Null-Terminate
    DebugPrint(str);

    va_end(args);
}


v2 GetWindowDimension(HWND window){
    RECT rect = {};
    GetClientRect(window, &rect);

    v2 result = {(f32)(rect.right - rect.left), (f32)(-rect.top + rect.bottom)};
    return result;
}

u8 *AllocateMemory(size_t size){
    return (u8 *)malloc(size);
}
void DeallocateMemory(void * ptr){
    free(ptr);
}


static void Win32ProcessPendingMessages(){
    MSG message;   
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message){
            case WM_QUIT:
            {
                globalRunning = false;
            }break;

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 vkCode = (u32)message.wParam;
                bool wasDown = ((message.lParam & (1 << 30)) != 0);
                bool isDown = ((message.lParam & (1 << 31)) == 0);

                if (wasDown != isDown){
                    
                    switch(vkCode){
                    case VK_ESCAPE:
                    case VK_RETURN:
                    case VK_SPACE:
                    {
                        if (isDown){
                            globalRunning = false;
                        }
                    }
                    }

                    // Alt + F4: Close
                    if (isDown){
                        s32 altKeyIsDown = (message.lParam & (1 << 29));
                        if ((vkCode == VK_F4) && altKeyIsDown){
                            globalRunning = false;
                        }
                    }
                }
            }break;

            default:
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }break;
        }
    }
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT   message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch(message)
    {
    case WM_CLOSE:
    case WM_DESTROY:
    {
        globalRunning = false;
    } break;

    default:
    {
        result = DefWindowProcA(window, message, wParam, lParam);
    } break;
    }
    
    return result;
}






//
// World data ,,
//

struct work_entry{
    s32 firstRowY;
    s32 numRows;
};
struct global_state{
    u8 *frameBuffer;
    v2s frameDim;

    // Current frame camera position (doesn't change till the current frame is finished)
    v3 frameCamPos;
    v3 frameCamForward;
    v3 frameCamRight;
    v3 frameCamUp;

    // Current logical camera position (can change more often than we draw frames)
    v3 camPos; // Eye pos.
    f32 camAngleY; // Camera direction along the y axis (horizontal plane direction).
    f32 camAngleX; // Camera direction along the X axis (up/down rotation).

    // Constant camera state
    f32 camNear; // Near clip plane
    f32 camFar; // Far clip plane
    f32 fovY;
    
    // Work queue
    s32 numEntries;
    work_entry entries[100];
    HANDLE semaphoreEntriesToDo;
    volatile s32 nextEntry;
    volatile s32 completedEntriesCount;
};

static global_state globalState;

#define INITIAL_CAM_POS V3(0, 10, -15.f)
#define INITIAL_CAM_ANGLE_Y 0
#define INITIAL_CAM_ANGLE_X -.5f


void BeginFrame(){
    auto gs = &globalState;

    gs->frameCamPos = gs->camPos;
    mat3 rotation = YRotation3(gs->camAngleY)*XRotation3(gs->camAngleX);
    gs->frameCamForward = MatrixMultiply(V3(0, 0, 1.f), rotation);
    gs->frameCamUp      = MatrixMultiply(V3(0, 1.f, 0), rotation);
    gs->frameCamRight   = -Cross(gs->frameCamForward, gs->frameCamUp);

    // Fill work queue
    s32 rowsPerEntry = 10;
    gs->numEntries = 0;

    gs->nextEntry = 0;
    gs->completedEntriesCount = 0;
    _ReadWriteBarrier();

    Assert(ArrayCount(gs->entries) > gs->frameDim.y/rowsPerEntry);
    for(s32 y = 0; y < gs->frameDim.y; y += rowsPerEntry){
        work_entry *entry = &gs->entries[gs->numEntries];
        gs->numEntries++;
        entry->firstRowY = y;
        entry->numRows = MinS32(rowsPerEntry, gs->frameDim.y - y);
    }

    LONG prevCount = 0;
    _ReadWriteBarrier();
    ReleaseSemaphore(gs->semaphoreEntriesToDo, gs->numEntries, &prevCount);
    Assert(prevCount == 0);
}


struct ray_intersection{
    f32 t; // distance. 0 for no intersection.
    s32 material;
    v3 normal;
};

struct sphere{
    v3 c;
    f32 r;
};

f32 IntersectSphere(sphere sphere, v3 ro, v3 rd){
    f32 t = -1.f;
    ro -= sphere.c; // Make ro relative to sphere center, so that sphere is centered at 0,0,0.

    // sphere at 0,0,0 equation:   sqrt(dot(p)) = r        (by dot(p) I mean dot(p, p))
    // ray equation:               p = ro + t*rd
    // substitution:               sqrt(dot(ro + t*rd)) = r
    //                             dot(ro + t*rd) = r^2
    //                             dot(ro + t*rd) - r^2 = 0
    // (expand binomial squared)   dot(ro) + dot(t*rd) + 2*t*dot(ro, rd) - r^2 = 0
    // (rd is unitary)             dot(ro) + t*t + 2*dot(ro, rd)*t - r^2 = 0
    // (reorder)                   t^2  +  2*dot(ro, rd)*t  +  dot(ro) - r^2 = 0
    // Now we have a quadratic equation on t.

    // a = 1
    f32 b = 2.f*Dot(ro, rd);
    f32 c = Dot(ro, ro) - SQUARE(sphere.r);
    f32 d = b*b - 4.f*c;
    if (d >= 0){
        t = (-b - SquareRoot(d))/2.f; // We only care about the lowest solution, i.e. the closest to the camera.
    }
    return t;
}
inline v3 NormalSphere(sphere sphere, v3 pos){
    v3 n = (pos - sphere.c)/sphere.r;
    return n;
}

inline f32 IntersectPlane(f32 y, v3 ro, v3 rd){
    f32 t = -1.f;
    // plane equation: p.y = y
    // ray equation:   p = ro + t*rd
    //                 p.y = ro.y + t*rd.y
    // substitution:   y = ro.y + t*rd.y
    //                 y - ro.y = t*rd.y
    //                 (y - ro.y)/rd.y = t
    if (rd.y){
        t = (y - ro.y)/rd.y;
    }
    return t;
}
inline v3 NormalPlane(){
    v3 n = {0, 1.f, 0};
    return n;
}

struct shape_material{
    v3 color;
    f32 reflectivity;
};
inline shape_material ShapeMaterial(v3 color, f32 reflectivity){
    shape_material result = {color, reflectivity};
    return result;
}

// Worker thread entry point
DWORD WINAPI ThreadProc(void *param){
    auto gs = &globalState;
    while(1){
        WaitForSingleObject(gs->semaphoreEntriesToDo, INFINITE);

        while(1){
            s32 entryIndex = gs->nextEntry;
            if (InterlockedCompareExchange((volatile LONG *)&gs->nextEntry, (LONG)entryIndex + 1, (LONG)entryIndex) == entryIndex){
                work_entry *entry = &gs->entries[entryIndex];

                //
                // Render our xframe rows
                //
                v2 worldFrameDim;
                worldFrameDim.y = Tan(gs->fovY/2);
                worldFrameDim.x = worldFrameDim.y*(gs->frameDim.x/(f32)gs->frameDim.y);

                for(s32 y = entry->firstRowY; y < entry->firstRowY + entry->numRows; y++){
                    for(s32 x = 0; x < gs->frameDim.x; x++){
                        v2 uv = {(f32)x/gs->frameDim.x, (f32)y/gs->frameDim.y}; // [0, 1]
                        u8 *pixel = &gs->frameBuffer[3*(y*gs->frameDim.x + x)];

                        // Ray
                        v3 ro = gs->frameCamPos;//V3(0, 2, -15.f);
                        //v3 rd = NormalizeNonZero(V3((-1.f + 2.f*uv.x)*worldFrameDim.x, (-1.f + 2.f*uv.y)*worldFrameDim.y, 1.f));
                        v3 rd = NormalizeNonZero(gs->frameCamForward + (-1.f + 2.f*uv.x)*gs->frameCamRight*worldFrameDim.x/2 + (-1.f + 2.f*uv.y)*gs->frameCamUp*worldFrameDim.y/2);

                        // Intersection with all objects
                        sphere spheres[6] = { { V3(0), 5.f },
                                              { V3(0, 6.f, 0), 3.f },
                                              { V3(8.f, 0, 0), 2.f },
                                              { V3(9.2f, 4.f, 1.f), 1.8f },
                                              { V3(0, 15.f, 0), 2.5f }, // light source
                                              { gs->frameCamPos, 1.5f }}; // Camera
                                              
                        v3 pointLightPos = spheres[4].c;

                        shape_material materials[12]; // Subscript is the shape index
                        materials[1] = ShapeMaterial(V3(.5f), 1.f); // Sphere 1
                        materials[2] = ShapeMaterial(V3(1.f, .3f, .3f), 1.f); // Sphere 2
                        materials[3] = ShapeMaterial(V3(.3f, 1.f, .5f), 1.f); // Sphere 3
                        materials[4] = ShapeMaterial(V3(.3f, .3f, .9f), .5f); // Sphere 4
                        materials[5] = ShapeMaterial(V3(1.f, 1.f, 1.f), 0); // Sphere 5
                        materials[6] = ShapeMaterial(V3(.3f, .3f, .3f), 0); // Sphere 6 (Camera)
                        materials[10] = ShapeMaterial(V3(.5f, .8f, .4f), 0); // Plane

                        f32 tSphere[ArrayCount(spheres)];
                        for(s32 i = 0; i < ArrayCount(spheres); i++){
                            tSphere[i] = IntersectSphere(spheres[i], ro, rd);
                        }

                        f32 tPlane   = IntersectPlane(0, ro, rd);
                        s32 shapeIndex = 0;

                        f32 t = gs->camFar;
                        for(s32 i = 0; i < ArrayCount(spheres); i++){
                            if (tSphere[i] > gs->camNear && tSphere[i] < t){
                                t = tSphere[i];
                                shapeIndex = 1 + i;
                            }
                        }
                        if (tPlane > gs->camNear && tPlane < t){
                            t = tPlane;
                            shapeIndex = 10;
                        }

                        //
                        // Color
                        //
                        v3 col = {0};
                        if (shapeIndex){
                            v3 n = {};
                            v3 p = ro + t*rd;
                            f32 emit = 0;
                            v3 shapeCol = materials[shapeIndex].color;
                            f32 reflectivity = materials[shapeIndex].reflectivity;
                            if (shapeIndex >= 1 && shapeIndex <= ArrayCount(spheres)){ // Spheres
                                n = NormalSphere(spheres[shapeIndex - 1], p);
                                emit = (shapeIndex == 5);
                            }else if (shapeIndex == 10){ // Plane
                                n = NormalPlane();
                            }
                            
                            // NOTE: The "pointLight" is actually spherical now. I just didn't bother to change the variable names hehe.
                            f32 pointLightLength = Length(pointLightPos - p);
                            f32 pointLight = 10.f/SQUARE(pointLightLength) + 5.f/pointLightLength; // Light strength based on distance
                            v3 pointLightDir = Normalize(pointLightPos - p);
                            pointLight *= Max(0, Dot(n, pointLightDir)); // Reduce strength based on angle.
                            pointLight *= Clamp01(MapRangeTo01(pointLight, .002f, .01f)); // Unnoticeable falloff for performance.
                            if (pointLight){
                                f32 pointLightRadius = spheres[4].r;

                                // Hard shadows: just one ray.
#if 0
                                f32 shadowT = pointLightLength;
                                for(s32 i = 0; i < ArrayCount(spheres); i++){
                                    if (i == 4) continue;
                                    f32 t = IntersectSphere(spheres[i], p, pointLightDir);
                                    if (t > .001f && t < shadowT){
                                        shadowT = t;
                                    }
                                }
                                f32 l = (shadowT < pointLightLength ? 0 : 1.f);
#endif

                                // Old way: Average of multiple rays.
#if 0
                                s32 numRays = 25;
                                s32 occludedRaysCount = 0;
                                for(s32 rayIndex = 0; rayIndex < numRays; rayIndex++){
                                    v3 rayDir = pointLightDir;
                                    if (rayIndex){
                                        // Shoot rays in different directions
                                        v3 px = Perpendicular(pointLightDir);
                                        v3 py = Cross(px, pointLightDir);
                                        u32 hash = SimpleHash((u32)rayIndex);
                                        f32 pr = SafeDivide0(pointLightRadius, pointLightLength)*((f32)(hash & 0xffff)/65535.f);
                                        f32 angle = ((f32)((hash >> 16) & 0xffff)/65535.f)*2*PI;
                                        rayDir = Normalize(pointLightDir + pr*(px*Cos(angle) + py*Sin(angle)));
                                    }
                                    f32 shadowT = pointLightLength;
                                    for(s32 i = 0; i < ArrayCount(spheres); i++){
                                        if (i == 4) continue;
                                        f32 t = IntersectSphere(spheres[i], p, rayDir);
                                        if (t > .001f && t < shadowT){
                                            shadowT = t;
                                        }
                                    }
                                    if (shadowT < pointLightLength){
                                        occludedRaysCount++;
                                    }
                                }
                                f32 l = (f32)(numRays - occludedRaysCount)/(f32)numRays;
#endif
                                
                                // New method: Project each sphere to 2d, circle intersection is the blocked area...
                                // r0 and r1 are the radius of light that will affect the pixel. r0 is the radius at 'p' and r1 is the radius at the light source.
#if 1
                                f32 pixelArea = (worldFrameDim.x/gs->frameDim.x)*(worldFrameDim.y/gs->frameDim.y);
                                f32 r0 = SquareRoot(pixelArea/(PI*t));
                                f32 r1 = spheres[4].r;
                                v3 perpX = Perpendicular(pointLightDir);
                                v3 perpY = Cross(perpX, pointLightDir);

                                v3 test1 = Cross(perpX, perpY);
                                v3 test2 = Cross(perpX, pointLightDir);
                                v3 test3 = Cross(perpY, pointLightDir);

                                f32 test1Length = Length(test1);
                                f32 test2Length = Length(test2);
                                f32 test3Length = Length(test3);
                                f32 pointLightDirLength = Length(pointLightDir);
                                f32 perpXLength = Length(perpX);
                                f32 perpYLength = Length(perpY);

                                f32 l = 1.f;
                                f32 lMin = 1.f;
                                f32 lSum = 1.f;
                                f32 blockCount = 0;
                                f32 blockedSum = 0;
                                for(s32 i = 0; i < ArrayCount(spheres); i++){
                                    if (i == 4) continue;

                                    // 'd' is the distance from 'p' to the point in the ray closest to the sphere. Maybe we could use distance to sphere as approximation.
                                    f32 d = Dot(spheres[i].c - p, pointLightDir);
                                    if (d < 0 || d > pointLightLength) // Outside blocking range.
                                        continue;

                                    v2 sphereProj = {Dot(spheres[i].c - p, perpX), Dot(spheres[i].c - p, perpY)};

                                    // 'r' is the radius of vision at the projected slice (where the sphere covers more area).
                                    f32 r = LerpClamp(r0, r1, d/pointLightLength);
                                    //f32 blockedArea = IntersectionAreaOfTwoCircles(V2(0), r, sphereProj, spheres[i].r);
                                    //f32 blockedAmount = blockedArea/(PI*r*r);

                                    f32 dis = Length(sphereProj);
                                    f32 len = Min(r, dis + spheres[i].r) - Max(-r, dis - spheres[i].r);
                                    f32 blockedAmount = Map01ToReverseSquare(Clamp01(len/r));
                                    if (blockedAmount){
                                        blockCount++;
                                        blockedSum += blockedAmount;
                                        lMin = Min(lMin, 1.f - blockedAmount);
                                        lSum = Max(0, lSum - blockedAmount);
                                    }
                                }
                                if (blockCount){
                                    l = Min(lMin, Lerp(lMin, Lerp(lSum, 1.f - blockedSum/blockCount, .5f), .5f));
                                }
#endif

                                pointLight *= l;
                            }
                            
                            //
                            // Secondary rays
                            //
                            v3 reflectionCol = {};
                            if (reflectivity && shapeIndex <= ArrayCount(spheres)){
                                v3 ro2 = p;
                                v3 rd2 = rd -2.f*Dot(rd, n)*n; // Reflect ray by the normal
                                f32 tSphere2[ArrayCount(spheres)];
                                for(s32 i = 0; i < ArrayCount(spheres); i++){
                                    tSphere2[i] = IntersectSphere(spheres[i], ro2, rd2);
                                }
                                f32 tPlane2 = IntersectPlane(0, ro2, rd2);
                                s32 shapeIndex2 = 0;

                                f32 t2 = gs->camFar;
                                for(s32 i = 0; i < ArrayCount(spheres); i++){
                                    if (tSphere2[i] > gs->camNear && tSphere2[i] < t2){
                                        t2 = tSphere2[i];
                                        shapeIndex2 = 1 + i;
                                    }
                                }
                                if (tPlane2 > gs->camNear && tPlane2 < t2){
                                    t2 = tPlane2;
                                    shapeIndex2 = 10;
                                }

                                //
                                // Color
                                //
                                v3 col2 = {0};
                                if (shapeIndex2){
                                    v3 p2 = ro2 + rd2*t2;
                                    v3 shapeCol2 = materials[shapeIndex2].color;
                                    //v3 n2;
                                    //if (shapeIndex2 >= 1 && shapeIndex2 <= ArrayCount(spheres)){ // Spheres
                                        //n2 = NormalSphere(spheres[shapeIndex2 - 1], p2); // BUG: Why does this mess up the plane's shading?
                                    //}else
                                    //if (shapeIndex2 == 10){ // Plane
                                    //	n2 = NormalPlane();
                                    //}
                                    col2 = shapeCol2;
                                }
                                reflectionCol = col2*(.06f*Square(Clamp01(1.f - Dot(n, -rd))) + .01f); // Fresnel kinda thing
                            }

                            
                            v3 specular = {};
                            if (pointLight){
                                // Blinn-Phong
                                v3 l = pointLightDir;
                                v3 v = -rd;
                                v3 h = Normalize(l + v);
                                f32 intensity = 3.f*Pow(Dot(n, h), 50.f);
                                specular = pointLight*intensity*V3(1.f, 1.f, 1.f)/pointLightLength;
                            }

                            //      emited light | ambient |  directional         |        spherical light  | specular  |  reflection
                            col = shapeCol*(emit + .03f + .12f*Max(0, n.y)/*(.5f + .5f*n.y)*/ + pointLight) + specular + reflectionCol*reflectivity;
                        }

                        pixel[0] = (u8)(Clamp01(LinearToSrgb(col.r))*255);
                        pixel[1] = (u8)(Clamp01(LinearToSrgb(col.g))*255);
                        pixel[2] = (u8)(Clamp01(LinearToSrgb(col.b))*255);
                    }
                }

                InterlockedIncrement((volatile LONG *)&gs->completedEntriesCount);
                break;
            }// Else another thread changed incremented entryIndex. We'll need to try again.
        }
    }
}

extern int CALLBACK 
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode){
    auto gi = &globalInput;
    auto gs = &globalState;

    //
    // Initialization
    //
    QueryPerformanceFrequency(&globalPerformanceFrequency);
    
    // Set the Windows scheduler granularity to 1ms so that our Sleep() can be more granular.
    UINT desiredSchedulerMS = 1;
    b32 sleepIsGranular = (timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR);
    
    // Create Console
    if (CREATE_CONSOLE){
        if(AttachConsole((DWORD)-1) == 0){ // wasn't launched from console
            AllocConsole(); // alloc your own instead
        }
    }
    globalStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);


    
    //
    //
    //
    
    WNDCLASSA windowClass = {};
    
    // NOTE: CS_OWNDC would allow to ask for the DeviceContext only once and pass it everywhere
    windowClass.style = CS_HREDRAW | CS_VREDRAW;// | CS_OWNDC; 
    windowClass.lpfnWndProc = Win32MainWindowCallback;
    windowClass.hInstance = instance; //GetModuleHandle(0);
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.lpszClassName = "MyWindowClass";


    HWND window = 0;
    if (RegisterClassA(&windowClass)){
        window = CreateWindowEx(0,
                                windowClass.lpszClassName, "Window title",
                                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                CW_USEDEFAULT, CW_USEDEFAULT, 658, 527, // 658x527 for the window dim gives 640x480 client area at least in my machine.
                                0, 0, instance, 0);
        if (!window){
            OutputDebugStringA("window error :)\n");
            return 1;
        }
    }else{
        OutputDebugStringA("ERROR CREATING WINDOW!\n");
    }
    
    HDC dc = GetDC(window);

    //
    // OpenGl
    //
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32;
    int pf = ChoosePixelFormat(dc, &pfd);

    if (!pf)
        return 0;
    
    if (!SetPixelFormat(dc, pf, &pfd))
        return 0;

    DescribePixelFormat(dc, pf, sizeof(pfd), &pfd);

    // Device context
    HGLRC rc = wglCreateContext(dc);
    wglMakeCurrent(dc, rc);

    s32 timeInFrames = 0;
    
    //
    // GL Get Procedures...
    //
    wglSwapInterval = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapInterval){
        wglSwapInterval(1); // Enables Vsync
    }
    glAttachShader            = (gl_attach_shader              *)wglGetProcAddress("glAttachShader");
    glCompileShader           = (gl_compile_shader             *)wglGetProcAddress("glCompileShader");
    glCreateProgram           = (gl_create_program             *)wglGetProcAddress("glCreateProgram");
    glCreateShader            = (gl_create_shader              *)wglGetProcAddress("glCreateShader");
    glDeleteProgram           = (gl_delete_program             *)wglGetProcAddress("glDeleteProgram");
    glDeleteShader            = (gl_delete_shader              *)wglGetProcAddress("glDeleteShader");
    glDetachShader            = (gl_detach_shader              *)wglGetProcAddress("glDetachShader");
    glLinkProgram             = (gl_link_program               *)wglGetProcAddress("glLinkProgram");
    glShaderSource            = (gl_shader_source              *)wglGetProcAddress("glShaderSource");
    glUseProgram              = (gl_use_program                *)wglGetProcAddress("glUseProgram");
    glGenBuffers              = (gl_gen_buffers                *)wglGetProcAddress("glGenBuffers");
    glBindBuffer              = (gl_bind_buffer                *)wglGetProcAddress("glBindBuffer");
    glDeleteBuffers           = (gl_delete_buffers             *)wglGetProcAddress("glDeleteBuffers"); 
    glBufferData              = (gl_buffer_data                *)wglGetProcAddress("glBufferData");
    glGetProgramiv            = (gl_get_programiv              *)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog       = (gl_get_program_info_log       *)wglGetProcAddress("glGetProgramInfoLog");
    glGetShaderiv             = (gl_get_shaderiv               *)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog        = (gl_get_shader_info_log        *)wglGetProcAddress("glGetShaderInfoLog");
    glGetAttribLocation       = (gl_get_attrib_location        *)wglGetProcAddress("glGetAttribLocation");
    glVertexAttribPointer     = (gl_vertex_attrib_pointer      *)wglGetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (gl_enable_vertex_attrib_array *)wglGetProcAddress("glEnableVertexAttribArray");
    glBindVertexArray         = (gl_bind_vertex_array          *)wglGetProcAddress("glBindVertexArray");
    glDeleteVertexArrays      = (gl_delete_vertex_arrays       *)wglGetProcAddress("glDeleteVertexArrays");
    glGenVertexArrays         = (gl_gen_vertex_arrays          *)wglGetProcAddress("glGenVertexArrays");
    glUniform1f               = (gl_uniform_1f                 *)wglGetProcAddress("glUniform1f");
    glUniform2f               = (gl_uniform_2f                 *)wglGetProcAddress("glUniform2f");
    glUniform3f               = (gl_uniform_3f                 *)wglGetProcAddress("glUniform3f");
    glUniform4f               = (gl_uniform_4f                 *)wglGetProcAddress("glUniform4f");
    glUniform1i               = (gl_uniform_1i                 *)wglGetProcAddress("glUniform1i");
    glUniform2i               = (gl_uniform_2i                 *)wglGetProcAddress("glUniform2i");
    glUniform3i               = (gl_uniform_3i                 *)wglGetProcAddress("glUniform3i");
    glUniform4i               = (gl_uniform_4i                 *)wglGetProcAddress("glUniform4i");
    glUniform1fv              = (gl_uniform_1fv                *)wglGetProcAddress("glUniform1fv");
    glUniform2fv              = (gl_uniform_2fv                *)wglGetProcAddress("glUniform2fv");
    glUniform3fv              = (gl_uniform_3fv                *)wglGetProcAddress("glUniform3fv");
    glUniform4fv              = (gl_uniform_4fv                *)wglGetProcAddress("glUniform4fv");
    glUniform1iv              = (gl_uniform_1iv                *)wglGetProcAddress("glUniform1iv");
    glUniform2iv              = (gl_uniform_2iv                *)wglGetProcAddress("glUniform2iv");
    glUniform3iv              = (gl_uniform_3iv                *)wglGetProcAddress("glUniform3iv");
    glUniform4iv              = (gl_uniform_4iv                *)wglGetProcAddress("glUniform4iv");
    glUniformMatrix2fv        = (gl_uniform_matrix_2fv         *)wglGetProcAddress("glUniformMatrix2fv");
    glUniformMatrix3fv        = (gl_uniform_matrix_3fv         *)wglGetProcAddress("glUniformMatrix3fv");
    glUniformMatrix4fv        = (gl_uniform_matrix_4fv         *)wglGetProcAddress("glUniformMatrix4fv");
    glGetUniformLocation      = (gl_get_uniform_location       *)wglGetProcAddress("glGetUniformLocation");
    glGenerateMipmap          = (gl_generate_mipmap            *)wglGetProcAddress("glGenerateMipmap");
    glActiveTexture           = (gl_active_texture             *)wglGetProcAddress("glActiveTexture");


    //
    // Init game state
    //
    gs->camPos = INITIAL_CAM_POS;
    gs->camAngleY = INITIAL_CAM_ANGLE_Y; // Rotation around Y axis (hand rule).
    gs->camAngleX = INITIAL_CAM_ANGLE_X; // Rotation around X axis (hand rule).
    gs->camNear = .001f;
    gs->camFar = MAX_F32;
    gs->fovY = DegreesToRadians(95.f);
    
    // Make test image
    gs->frameDim = V2S(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
    gs->frameBuffer = (u8 *)malloc(gs->frameDim.x*gs->frameDim.y*3); // 3 bytes per pixel

    // Init image memory
    //for(s32 y = 0; y < gs->frameDim.y; y++){
    //	for(s32 x = 0; x < gs->frameDim.x; x++){
    //		v4 color = ColorFromHSV(((x + 200) % gs->frameDim.x)/(f32)(gs->frameDim.x - 1), y/(f32)(gs->frameDim.y - 1), y/(f32)(gs->frameDim.y - 1));
    //		u8 *dest = &gs->imageData[(y*gs->frameDim.x + x)*3];
    //		dest[0] = (u8)(color.r*255);
    //		dest[1] = (u8)(color.g*255);
    //		dest[2] = (u8)(color.b*255);
    //	}
    //}

    gs->semaphoreEntriesToDo = CreateSemaphore(NULL, 0, gs->frameDim.y, NULL);

    _mm_sfence();

    // Create worker threads
    for(s32 i = 0; i < NUM_WORKER_THREADS; i++){
        DWORD threadId = 0;
        HANDLE thread = CreateThread(NULL, 0, ThreadProc, NULL, 0, &threadId);
        if (!thread){
            Printf("Error creating thread %i.\n", i);
        }
    }
    

    BeginFrame();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    u32 vbo;
    glGenBuffers(1, &vbo); // Create vertex buffer object.
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind the buffer so the next calls apply to it.
    
    //
    // Textures
    //
    u32 sceneTextureHandler;
    glGenTextures(1, &sceneTextureHandler);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTextureHandler);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gs->frameDim.x, gs->frameDim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, gs->frameBuffer);

    //
    // Shaders
    //

    
    struct vertex_data{
        v2 pos;
        v2 texPos;
    };
    
    char *vertexShaderStr = R"END(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexPos;

out vec2 texPos;

void main(){
    texPos = aTexPos;
    gl_Position = vec4(aPos.xy, -1.f, 1.0);
}
)END";

    char *fragmentShaderStr = R"END(
#version 330 core
in vec2 texPos;

uniform sampler2D textureSampler;

out vec4 resultColor;

void main(){
    resultColor = texture(textureSampler, texPos);
}
)END";

    // Vertex Shader
    u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderStr, 0);
    glCompileShader(vertexShader);

    int compiledVertex;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiledVertex);
    if (!compiledVertex) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, ArrayCount(infoLog), 0, infoLog);
        Printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n\n", infoLog);
    }

    // Fragment Shader
    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderStr, 0);
    glCompileShader(fragmentShader);
    
    int compiledFragment;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiledFragment);
    if (!compiledFragment) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, ArrayCount(infoLog), 0, infoLog);
        Printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n\n", infoLog);
    }

    // Shader Program
    u32 shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int linkedProgram;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkedProgram);
    if(!linkedProgram) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, ArrayCount(infoLog), 0, infoLog);
        Printf("ERROR LINKING THE SHADER PROGRAM:\n%s\n\n", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    s32 locationAPos = glGetAttribLocation(shaderProgram, "aPos");
    s32 locationATexPos = glGetAttribLocation(shaderProgram, "aTexPos");
    //Printf("aPos: %i   aTexPos: %i\n", locationAPos, locationATexPos);
    
    // Link vertex attributes

    // aPos attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data), (void *)OffsetOf(vertex_data, pos));
    glEnableVertexAttribArray(0);
    // aTexPos attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data), (void *)OffsetOf(vertex_data, texPos));
    glEnableVertexAttribArray(1);

    //int vertexUniformLocationTransform = glGetUniformLocation(shaderProgram, "transform");
    int vertexUniformLocationTexture = glGetUniformLocation(shaderProgram, "textureSampler");

    glUseProgram(shaderProgram);
    glUniform1i(vertexUniformLocationTexture, 0); // Set the texture sampler uniform. This won't change.


    LARGE_INTEGER lastFrameTime = GetCurrentTimeCounter();
    LARGE_INTEGER lastFpsUpdateTime = GetCurrentTimeCounter();
    globalRunning = true;
    b32 firstFrame = true;
    s32 frameCount = -1;
    s32 prevFrameCount = frameCount;
    s32 renderedFrameCountSinceFpsUpdate = 0;
    while(globalRunning){
        // Update FPS (aproximation)
        LARGE_INTEGER fpsUpdateTime = GetCurrentTimeCounter();
        f32 timeSinceLastFpsUpdate = GetSecondsElapsed(lastFpsUpdateTime, fpsUpdateTime);
        if (timeSinceLastFpsUpdate > 1.f){
            lastFpsUpdateTime = fpsUpdateTime;
            f32 fps = renderedFrameCountSinceFpsUpdate/timeSinceLastFpsUpdate;
            f32 steps = (frameCount - prevFrameCount)/timeSinceLastFpsUpdate;
            prevFrameCount = frameCount;
            renderedFrameCountSinceFpsUpdate = 0;
            
            char title[256];
            sprintf_s(title, "FPS: %.0f   StepsPS: %.0f", fps, steps);
            SetWindowTextA(window, title);
        }


        frameCount++;

        //
        // Message Loop
        //
        MSG msg = { };
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
            switch(msg.message){
            case WM_KEYUP:
            case WM_KEYDOWN:
            {
                //
                // Keyboard Input
                //
                b32 wentDown = (msg.message == WM_KEYDOWN);
                auto k = &globalInput.keyboard;
                if (msg.wParam == VK_ESCAPE){
                    UpdateButtonState(&k->escape, wentDown);
                }else if (msg.wParam == VK_RETURN){
                    UpdateButtonState(&k->enter, wentDown);
                }else if (msg.wParam == VK_SPACE){
                    UpdateButtonState(&k->space, wentDown);
                }else if (msg.wParam == VK_SHIFT){
                    UpdateButtonState(&k->shift, wentDown);
                }else if (msg.wParam == VK_CONTROL){
                    UpdateButtonState(&k->control, wentDown);
                }else if (msg.wParam == VK_BACK){
                    UpdateButtonState(&k->backspace, wentDown);
                }else if (msg.wParam == VK_MENU){
                    UpdateButtonState(&k->alt, wentDown);
                }else if (msg.wParam == VK_TAB){
                    UpdateButtonState(&k->tab, wentDown);
                }else if (msg.wParam == VK_LEFT){
                    UpdateButtonState(&k->arrowLeft, wentDown);
                }else if (msg.wParam == VK_RIGHT){
                    UpdateButtonState(&k->arrowRight, wentDown);
                }else if (msg.wParam == VK_UP){
                    UpdateButtonState(&k->arrowUp, wentDown);
                }else if (msg.wParam == VK_DOWN){
                    UpdateButtonState(&k->arrowDown, wentDown);
                }else if (msg.wParam >= 'A' && msg.wParam <= 'Z'){
                    UpdateButtonState(&k->letters[msg.wParam - 'A'], wentDown);
                }else if (msg.wParam >= '0' && msg.wParam <= '9'){
                    UpdateButtonState(&k->numbers[msg.wParam - '0'], wentDown);
                }

                break;
            }
            
            default:
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            }
        }


        //
        // Mouse Input
        //
        globalInput.windowDim = GetWindowDimension(window);

        v2 prevMousePos = gi->mousePos;

        POINT mousePoint;
        GetCursorPos(&mousePoint);
        ScreenToClient(window, &mousePoint);
        gi->mousePos.x = (f32)(s32)mousePoint.x;
        gi->mousePos.y = (f32)(s32)(globalInput.windowDim.y - mousePoint.y);
        
        if (firstFrame){
            prevMousePos = gi->mousePos;
        }

        if (PointInRectangle(globalInput.mousePos, V2(0), globalInput.windowDim)){
            UpdateButtonState(&globalInput.mouseButtons[0], GetKeyState(VK_LBUTTON)  & (1 << 15));
            UpdateButtonState(&globalInput.mouseButtons[1], GetKeyState(VK_MBUTTON)  & (1 << 15));
            UpdateButtonState(&globalInput.mouseButtons[2], GetKeyState(VK_RBUTTON)  & (1 << 15));
            UpdateButtonState(&globalInput.mouseButtons[3], GetKeyState(VK_XBUTTON1) & (1 << 15));
            UpdateButtonState(&globalInput.mouseButtons[4], GetKeyState(VK_XBUTTON2) & (1 << 15));
        }

        //
        // Program Code
        //

        
        v2 winDim = gi->windowDim;


        //
        // Input
        //
        if (gi->keyboard.escape.isDown)
            globalRunning = false;


        v2 prevAngles = {gs->camAngleX, gs->camAngleY};
        f32 camSpeed = .1f;
        v3 prevCamPos = gs->camPos;

        // Rotate movement
        v2 groundMove = { camSpeed*((gi->keyboard.letters['D' - 'A'].isDown ? 1.f : 0) + (gi->keyboard.letters['A' - 'A'].isDown ? -1.f : 0)),
                          camSpeed*((gi->keyboard.letters['W' - 'A'].isDown ? 1.f : 0) + (gi->keyboard.letters['S' - 'A'].isDown ? -1.f : 0)) };
        groundMove = RotateV2(groundMove, gs->camAngleY);

        v3 camMove = { groundMove.x,
                       camSpeed*((gi->keyboard.space.isDown ?  1.f : 0) + (gi->keyboard.shift.isDown ? -1.f : 0)),
                       groundMove.y };
        gs->camPos += camMove;

        // Mouse input
        v2 mouseDelta = {};
        if (gi->mouseButtons[0].isDown){ // TODO: Only if has focus.
            mouseDelta = gi->mousePos - prevMousePos;
        }
        f32 mouseSensitivityX = .01f;
        f32 mouseSensitivityY = .01f;
        if (mouseDelta.x){ // Rotate left/right
            gs->camAngleY = NormalizeAngle(gs->camAngleY - mouseDelta.x*mouseSensitivityX);
        }
        if (mouseDelta.y){ // Rotate up/down
            gs->camAngleX = Clamp(gs->camAngleX + mouseDelta.y*mouseSensitivityY, -PI/2, PI/2);
        }

        if (ButtonWentDown(&gi->keyboard.letters['R' - 'A'])){ // Reset
            gs->camPos = INITIAL_CAM_POS;
            gs->camAngleY = INITIAL_CAM_ANGLE_Y;
            gs->camAngleX = INITIAL_CAM_ANGLE_X;
        }

        //if (V2(gs->camAngleX, gs->camAngleY) != prevAngles){
        //	Printf("Camera angle Y=%.3f, X=%.3f\n", gs->camAngleY, gs->camAngleX);
        //}


        _mm_lfence();
        if (gs->completedEntriesCount == gs->numEntries){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gs->frameDim.x, gs->frameDim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, gs->frameBuffer);
            renderedFrameCountSinceFpsUpdate++;

            _ReadWriteBarrier(); // (maybe I overdo these but just in case...)
            _mm_sfence();
            _mm_lfence();

            BeginFrame();
        }

        //
        // Render
        //
        glViewport(0,0, (GLsizei)gi->windowDim.x, (GLsizei)gi->windowDim.y); // change draw viewport size.
    
        glClearColor(0.12f, .06f, .2f, 1.f);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindTexture(GL_TEXTURE_2D, sceneTextureHandler);
        
// Converts from range [0, winDim] to [-1, 1]
#define WINDOW_COORD_TO_GL_X(x_) (((x_)/winDim.x)*2.f - 1.f)
#define WINDOW_COORD_TO_GL_Y(y_) (((y_)/winDim.y)*2.f - 1.f)
#define WINDOW_COORD_TO_GL(vec) V2(WINDOW_COORD_TO_GL_X((vec).x), WINDOW_COORD_TO_GL_Y((vec).y))

        f32 imageScale = Min(winDim.x/gs->frameDim.x, winDim.y/gs->frameDim.y);
        v2 p0 = WINDOW_COORD_TO_GL(winDim/2 - V2(gs->frameDim)*imageScale/2);
        v2 p1 = WINDOW_COORD_TO_GL(winDim/2 + V2(gs->frameDim)*imageScale/2);
        v2 t0 = V2(0);
        v2 t1 = V2(1);
        vertex_data vertices[6] = {{{p0.x, p1.y},  {t0.x, t1.y}},  // Bottom-Left
                                   {{p1.x, p0.y},  {t1.x, t0.y}},  // Top-Right
                                   {{p0.x, p0.y},  {t0.x, t0.y}},  // Top-Left

                                   {{p0.x, p1.y},  {t0.x, t1.y}},  // Bottom-Left
                                   {{p1.x, p1.y},  {t1.x, t1.y}},  // Bottom-Right
                                   {{p1.x, p0.y},  {t1.x, t0.y}}}; // Top-Right
        glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind the buffer so the next calls apply to it.
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
        glBindVertexArray(vao);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        

        glFlush();
        SwapBuffers(dc);


        //
        // Sleep to render at 60 FPS
        //
        f32 fpsTarget = 60.f;
        while(true){
            LARGE_INTEGER newFrameTime = GetCurrentTimeCounter();
            f32 timeElapsed = GetSecondsElapsed(lastFrameTime, newFrameTime);
            if (timeElapsed > 1.f/fpsTarget){
                lastFrameTime = newFrameTime;
                break;
            }
            if (1.f/fpsTarget - timeElapsed > 0.005f){
                Sleep(1);
            }
        }

        // Reset button input.
        for(s32 i = 0; i < ArrayCount(globalInput.keyboard.asArray); i++){
            globalInput.keyboard.asArray[i].transitionCount = 0;
        }
        for(s32 i = 0; i < ArrayCount(globalInput.mouseButtons); i++){
            globalInput.mouseButtons[i].transitionCount = 0;
        }

        timeInFrames++;
        firstFrame = false;
    }

    FreeConsole();

    wglMakeCurrent(NULL, NULL);
    ReleaseDC(window, dc);
    wglDeleteContext(rc);
    DestroyWindow(window);

    return 0;
}
