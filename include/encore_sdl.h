/* SDL native ABI v1. Canonical SDK header; consumers vendor this public contract. */
#ifndef ENCORE_SDL_API_H
#define ENCORE_SDL_API_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#ifndef ENCORE_NATIVE_STRING_ABI
#define ENCORE_NATIVE_STRING_ABI
typedef struct { size_t ref_count; size_t len; char data[]; } encore_str_object;
typedef struct { encore_str_object *object; } encore_str;
#endif
typedef struct UiSdlWindow UiSdlWindow;
typedef struct UiSdlRenderer UiSdlRenderer;
typedef struct UiSdlTexture UiSdlTexture;
typedef struct UiSdlCursor UiSdlCursor;
typedef uint32_t UiSdlPropertiesID;
typedef struct UiSdlSurface {
    uint32_t flags;
    uint32_t format;
    int width;
    int height;
    int pitch;
    void *pixels;
    int refcount;
    void *reserved;
} UiSdlSurface;
typedef struct UiTtfFont UiTtfFont;
typedef struct { float x, y, w, h; } UiSdlFRect;
typedef struct { float x, y; } UiSdlFPoint;
typedef struct { int x, y, w, h; } UiSdlRect;
typedef struct { uint8_t r, g, b, a; } UiSdlColor;
typedef struct { float r, g, b, a; } UiSdlFColor;
typedef struct { UiSdlFPoint position; UiSdlFColor color; UiSdlFPoint tex_coord; } UiSdlVertex;
typedef struct UiTtfTextEngine UiTtfTextEngine;
typedef struct UiTtfText {
    char *text;
    int num_lines;
    int refcount;
    void *internal;
} UiTtfText;
typedef struct {
    uint32_t flags;
    int offset;
    int length;
    int line_index;
    int cluster_index;
    UiSdlRect rect;
} UiTtfSubString;

typedef struct {
    uint32_t type;
    uint32_t reserved;
    uint64_t timestamp;
} UiSdlCommonEvent;

typedef union {
    uint32_t type;
    UiSdlCommonEvent common;
    uint8_t padding[128];
} UiSdlEvent;

typedef struct {
    uint32_t type;
    uint32_t reserved;
    uint64_t timestamp;
    uint32_t window_id;
    uint32_t which;
    uint32_t state;
    float x;
    float y;
    float xrel;
    float yrel;
} UiSdlMouseMotionEvent;

typedef struct {
    uint32_t type;
    uint32_t reserved;
    uint64_t timestamp;
    uint32_t window_id;
    uint32_t which;
    uint8_t button;
    bool down;
    uint8_t clicks;
    uint8_t padding;
    float x;
    float y;
} UiSdlMouseButtonEvent;

typedef struct {
    uint32_t type;
    uint32_t reserved;
    uint64_t timestamp;
    uint32_t window_id;
    uint32_t which;
    float x;
    float y;
    uint32_t direction;
    float mouse_x;
    float mouse_y;
} UiSdlMouseWheelEvent;

typedef struct {
    uint32_t type;
    uint32_t reserved;
    uint64_t timestamp;
    uint32_t window_id;
    uint32_t which;
    uint32_t scancode;
    uint32_t key;
    uint16_t mod;
    uint16_t raw;
    bool down;
    bool repeat;
} UiSdlKeyboardEvent;

typedef struct {
    uint32_t type;
    uint32_t reserved;
    uint64_t timestamp;
    uint32_t window_id;
    const char *text;
} UiSdlTextInputEvent;

typedef struct {
    uint32_t type;
    uint32_t reserved;
    uint64_t timestamp;
    uint32_t window_id;
    int32_t data1;
    int32_t data2;
} UiSdlWindowEvent;

enum {
    UI_SDL_INIT_VIDEO = 0x00000020u,
    UI_SDL_EVENT_QUIT = 0x100,
    UI_SDL_EVENT_WINDOW_CLOSE = 0x210,
    UI_SDL_EVENT_WINDOW_EXPOSED = 0x204,
    UI_SDL_EVENT_WINDOW_RESIZED = 0x206,
    UI_SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED = 0x207,
    UI_SDL_EVENT_WINDOW_FOCUS_LOST = 0x20F,
    UI_SDL_EVENT_KEY_DOWN = 0x300,
    UI_SDL_EVENT_KEY_UP = 0x301,
    UI_SDL_EVENT_TEXT_INPUT = 0x303,
    UI_SDL_EVENT_MOUSE_MOTION = 0x400,
    UI_SDL_EVENT_MOUSE_BUTTON_DOWN = 0x401,
    UI_SDL_EVENT_MOUSE_BUTTON_UP = 0x402,
    UI_SDL_EVENT_MOUSE_WHEEL = 0x403
};

typedef struct {
    void *library;
    bool attempted;
    bool ready;
    bool (*Init)(uint32_t);
    bool (*IsMainThread)(void);
    uint32_t (*GetWindowID)(UiSdlWindow *);
    void (*Quit)(void);
    UiSdlWindow *(*CreateWindow)(const char *, int, int, uint64_t);
    bool (*CreateWindowAndRenderer)(const char *, int, int, uint64_t, UiSdlWindow **, UiSdlRenderer **);
    UiSdlRenderer *(*CreateRenderer)(UiSdlWindow *, const char *);
    UiSdlPropertiesID (*CreateProperties)(void);
    void (*DestroyProperties)(UiSdlPropertiesID);
    bool (*SetNumberProperty)(UiSdlPropertiesID, const char *, int64_t);
    void *(*GetPointerProperty)(UiSdlPropertiesID, const char *, void *);
    int64_t (*GetNumberProperty)(UiSdlPropertiesID, const char *, int64_t);
    UiSdlPropertiesID (*GetRendererProperties)(UiSdlRenderer *);
    UiSdlSurface *(*CreateSurface)(int, int, uint32_t);
    UiSdlRenderer *(*CreateSoftwareRenderer)(UiSdlSurface *);
    const char *(*GetRendererName)(UiSdlRenderer *);
    bool (*ShowWindow)(UiSdlWindow *);
    bool (*SetWindowOpacity)(UiSdlWindow *, float);
    bool (*SyncWindow)(UiSdlWindow *);
    bool (*GetWindowSize)(UiSdlWindow *, int *, int *);
    bool (*SetWindowMinimumSize)(UiSdlWindow *, int, int);
    float (*GetWindowPixelDensity)(UiSdlWindow *);
    uint32_t (*GetDisplayForWindow)(UiSdlWindow *);
    float (*GetDisplayContentScale)(uint32_t);
    bool (*GetWindowSizeInPixels)(UiSdlWindow *, int *, int *);
    void (*DestroyRenderer)(UiSdlRenderer *);
    void (*DestroyWindow)(UiSdlWindow *);
    bool (*PollEvent)(UiSdlEvent *);
    bool (*WaitEventTimeout)(UiSdlEvent *, int);
    bool (*SetWindowRelativeMouseMode)(UiSdlWindow *, bool);
    bool (*StartTextInput)(UiSdlWindow *);
    bool (*StopTextInput)(UiSdlWindow *);
    UiSdlCursor *(*CreateSystemCursor)(uint32_t);
    bool (*SetCursor)(UiSdlCursor *);
    void (*DestroyCursor)(UiSdlCursor *);
    bool (*SetClipboardText)(const char *);
    char *(*GetClipboardText)(void);
    void (*Free)(void *);
    void *(*IOFromConstMem)(const void *, size_t);
    bool (*CloseIO)(void *);
    bool (*SetRenderDrawColor)(UiSdlRenderer *, uint8_t, uint8_t, uint8_t, uint8_t);
    bool (*SetRenderDrawBlendMode)(UiSdlRenderer *, uint32_t);
    bool (*SetRenderClipRect)(UiSdlRenderer *, const UiSdlRect *);
    bool (*SetRenderScale)(UiSdlRenderer *, float, float);
    bool (*RenderClear)(UiSdlRenderer *);
    bool (*RenderFillRect)(UiSdlRenderer *, const UiSdlFRect *);
    bool (*RenderRect)(UiSdlRenderer *, const UiSdlFRect *);
    bool (*RenderLine)(UiSdlRenderer *, float, float, float, float);
    bool (*RenderGeometry)(UiSdlRenderer *, UiSdlTexture *, const UiSdlVertex *, int, const int *, int);
    bool (*RenderDebugText)(UiSdlRenderer *, float, float, const char *);
    UiSdlTexture *(*CreateTextureFromSurface)(UiSdlRenderer *, UiSdlSurface *);
    UiSdlTexture *(*CreateTexture)(UiSdlRenderer *, uint32_t, int, int, int);
    UiSdlTexture *(*CreateTextureWithProperties)(UiSdlRenderer *, UiSdlPropertiesID);
    bool (*UpdateTexture)(UiSdlTexture *, const UiSdlRect *, const void *, int);
    bool (*SetTextureBlendMode)(UiSdlTexture *, uint32_t);
    bool (*SetRenderTarget)(UiSdlRenderer *, UiSdlTexture *);
    bool (*RenderTexture)(UiSdlRenderer *, UiSdlTexture *, const UiSdlFRect *, const UiSdlFRect *);
    UiSdlSurface *(*RenderReadPixels)(UiSdlRenderer *, const UiSdlRect *);
    bool (*ReadSurfacePixel)(UiSdlSurface *, int, int, uint8_t *, uint8_t *, uint8_t *, uint8_t *);
    bool (*WriteSurfacePixel)(UiSdlSurface *, int, int, uint8_t, uint8_t, uint8_t, uint8_t);
    void (*DestroyTexture)(UiSdlTexture *);
    void (*DestroySurface)(UiSdlSurface *);
    bool (*RenderPresent)(UiSdlRenderer *);
    bool (*SetRenderVSync)(UiSdlRenderer *, int);
    const char *(*GetError)(void);
    uint64_t (*GetTicksNS)(void);
} UiSdlApi;

typedef struct {
    void *library;
    bool attempted;
    bool ready;
    bool (*Init)(void);
    void (*Quit)(void);
    UiTtfFont *(*OpenFont)(const char *, float);
    UiTtfFont *(*OpenFontIO)(void *, bool, float);
    void (*CloseFont)(UiTtfFont *);
    void (*SetFontStyle)(UiTtfFont *, uint32_t);
    bool (*SetFontSize)(UiTtfFont *, float);
    bool (*GetStringSize)(UiTtfFont *, const char *, size_t, int *, int *);
    bool (*GetStringSizeWrapped)(UiTtfFont *, const char *, size_t, int, int *, int *);
    bool (*MeasureString)(UiTtfFont *, const char *, size_t, int, int *, size_t *);
    UiSdlSurface *(*RenderTextBlended)(UiTtfFont *, const char *, size_t, UiSdlColor);
    UiSdlSurface *(*RenderTextBlendedWrapped)(UiTtfFont *, const char *, size_t, UiSdlColor, int);
    UiTtfTextEngine *(*CreateSurfaceTextEngine)(void);
    void (*DestroySurfaceTextEngine)(UiTtfTextEngine *);
    UiTtfText *(*CreateText)(UiTtfTextEngine *, UiTtfFont *, const char *, size_t);
    void (*DestroyText)(UiTtfText *);
    bool (*GetTextSize)(UiTtfText *, int *, int *);
    UiTtfSubString **(*GetTextSubStringsForRange)(UiTtfText *, int, int, int *);
} UiTtfApi;


const UiSdlApi *encore_sdl_api_v1(void);
const UiTtfApi *encore_sdl_ttf_api_v1(void);
bool encore_sdl_acquire(void);
bool encore_sdl_release(void);
bool encore_sdl_load_ttf(void);
const char *encore_sdl_error_cstr(void);
bool encore_sdl_main_thread(void);
bool encore_sdl_poll_raw(UiSdlWindow *, UiSdlEvent *);
bool encore_sdl_wait_raw(UiSdlWindow *, UiSdlEvent *, int);
bool encore_sdl_wait_any(int);
void encore_sdl_forget_events(UiSdlWindow *);
bool encore_sdl_track_window(UiSdlWindow *);
bool encore_sdl_vulkan_acquire(void);
void encore_sdl_vulkan_release(void);
const char *const *encore_sdl_vulkan_extensions(uint32_t *);
bool encore_sdl_vulkan_surface(UiSdlWindow *, void *, uint64_t *);
void encore_sdl_vulkan_surface_destroy(void *, uint64_t);
#endif
