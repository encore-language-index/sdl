#include "include/encore_sdl.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef CreateWindow
#undef CreateWindow
#endif
#else
#include <dlfcn.h>
#endif
extern void *encore_str_from_cstr(const char *);
static UiSdlApi g_sdl;
static UiTtfApi g_ttf;
static char g_error[256];
static size_t g_references;
static size_t g_vulkan_references;
static void ui_set_error(const char *message) {
    snprintf(g_error, sizeof g_error, "%s", message ? message : "SDL error");
}
#if defined(_WIN32)
static void *ui_open_library(void) {
    const char *names[] = {"SDL3.dll", NULL};
    for (size_t i = 0; names[i] != NULL; ++i) {
        HMODULE lib = LoadLibraryA(names[i]);
        if (lib != NULL) return (void *)lib;
    }
    return NULL;
}
static void *ui_open_ttf_library(void) {
    const char *names[] = {"SDL3_ttf.dll", NULL};
    for (size_t i = 0; names[i] != NULL; ++i) {
        HMODULE lib = LoadLibraryA(names[i]);
        if (lib != NULL) return (void *)lib;
    }
    return NULL;
}
static void *ui_symbol(void *library, const char *name) {
    return (void *)GetProcAddress((HMODULE)library, name);
}
#else
static void *ui_open_library(void) {
#if defined(__APPLE__)
    const char *names[] = {"libSDL3.0.dylib", "libSDL3.dylib", "/opt/homebrew/lib/libSDL3.dylib", NULL};
#else
    const char *names[] = {"libSDL3.so.0", "libSDL3.so", NULL};
#endif
    for (size_t i = 0; names[i] != NULL; ++i) {
        void *lib = dlopen(names[i], RTLD_NOW | RTLD_LOCAL);
        if (lib != NULL) return lib;
    }
    return NULL;
}
static void *ui_open_ttf_library(void) {
#if defined(__APPLE__)
    const char *names[] = {"libSDL3_ttf.0.dylib", "libSDL3_ttf.dylib", "/opt/homebrew/lib/libSDL3_ttf.dylib", NULL};
#else
    const char *names[] = {"libSDL3_ttf.so.0", "libSDL3_ttf.so", NULL};
#endif
    for (size_t i = 0; names[i] != NULL; ++i) {
        void *lib = dlopen(names[i], RTLD_NOW | RTLD_LOCAL);
        if (lib != NULL) return lib;
    }
    return NULL;
}
static void *ui_symbol(void *library, const char *name) { return dlsym(library, name); }
#endif



#define UI_LOAD(field, symbol) do { \
    *(void **)(&g_sdl.field) = ui_symbol(g_sdl.library, symbol); \
    if (g_sdl.field == NULL) { ui_set_error("SDL3 is missing required rendering symbols"); return false; } \
} while (0)

static bool ui_load_sdl(void) {
    if (g_sdl.attempted) return g_sdl.ready;
    g_sdl.attempted = true;
#if !defined(_WIN32) && !defined(__APPLE__)
    if (getenv("SDL_VIDEODRIVER") == NULL && getenv("WAYLAND_DISPLAY") != NULL) {
        setenv("SDL_VIDEODRIVER", "wayland", 0);
    }
#endif
    if (!g_sdl.library) g_sdl.library = ui_open_library();
    if (g_sdl.library == NULL) {
        ui_set_error("SDL3 runtime was not found");
        return false;
    }
    UI_LOAD(Init, "SDL_Init");
    UI_LOAD(IsMainThread, "SDL_IsMainThread");
    UI_LOAD(GetWindowID, "SDL_GetWindowID");
    UI_LOAD(Quit, "SDL_Quit");
    UI_LOAD(CreateWindow, "SDL_CreateWindow");
    UI_LOAD(CreateWindowAndRenderer, "SDL_CreateWindowAndRenderer");
    UI_LOAD(CreateRenderer, "SDL_CreateRenderer");
    UI_LOAD(CreateProperties, "SDL_CreateProperties");
    UI_LOAD(DestroyProperties, "SDL_DestroyProperties");
    UI_LOAD(SetNumberProperty, "SDL_SetNumberProperty");
    UI_LOAD(GetPointerProperty, "SDL_GetPointerProperty");
    UI_LOAD(GetNumberProperty, "SDL_GetNumberProperty");
    UI_LOAD(GetRendererProperties, "SDL_GetRendererProperties");
    UI_LOAD(CreateSurface, "SDL_CreateSurface");
    UI_LOAD(CreateSoftwareRenderer, "SDL_CreateSoftwareRenderer");
    UI_LOAD(GetRendererName, "SDL_GetRendererName");
    UI_LOAD(ShowWindow, "SDL_ShowWindow");
    UI_LOAD(SetWindowOpacity, "SDL_SetWindowOpacity");
    UI_LOAD(SyncWindow, "SDL_SyncWindow");
    UI_LOAD(GetWindowSize, "SDL_GetWindowSize");
    UI_LOAD(SetWindowMinimumSize, "SDL_SetWindowMinimumSize");
    UI_LOAD(GetWindowPixelDensity, "SDL_GetWindowPixelDensity");
    UI_LOAD(GetDisplayForWindow, "SDL_GetDisplayForWindow");
    UI_LOAD(GetDisplayContentScale, "SDL_GetDisplayContentScale");
    UI_LOAD(GetWindowSizeInPixels, "SDL_GetWindowSizeInPixels");
    UI_LOAD(DestroyRenderer, "SDL_DestroyRenderer");
    UI_LOAD(DestroyWindow, "SDL_DestroyWindow");
    UI_LOAD(PollEvent, "SDL_PollEvent");
    UI_LOAD(WaitEventTimeout, "SDL_WaitEventTimeout");
    /* Relative mouse mode was added to SDL3's window API. Keep it optional
       so the renderer remains usable with older SDL3 runtimes. */
    *(void **)(&g_sdl.SetWindowRelativeMouseMode) =
        ui_symbol(g_sdl.library, "SDL_SetWindowRelativeMouseMode");
    UI_LOAD(StartTextInput, "SDL_StartTextInput");
    UI_LOAD(StopTextInput, "SDL_StopTextInput");
    UI_LOAD(CreateSystemCursor, "SDL_CreateSystemCursor");
    UI_LOAD(SetCursor, "SDL_SetCursor");
    UI_LOAD(DestroyCursor, "SDL_DestroyCursor");
    UI_LOAD(SetClipboardText, "SDL_SetClipboardText");
    UI_LOAD(GetClipboardText, "SDL_GetClipboardText");
    UI_LOAD(Free, "SDL_free");
    UI_LOAD(SetRenderDrawColor, "SDL_SetRenderDrawColor");
    UI_LOAD(SetRenderDrawBlendMode, "SDL_SetRenderDrawBlendMode");
    UI_LOAD(SetRenderClipRect, "SDL_SetRenderClipRect");
    UI_LOAD(SetRenderScale, "SDL_SetRenderScale");
    UI_LOAD(RenderClear, "SDL_RenderClear");
    UI_LOAD(RenderFillRect, "SDL_RenderFillRect");
    UI_LOAD(RenderRect, "SDL_RenderRect");
    UI_LOAD(RenderLine, "SDL_RenderLine");
    UI_LOAD(RenderGeometry, "SDL_RenderGeometry");
    UI_LOAD(RenderDebugText, "SDL_RenderDebugText");
    UI_LOAD(CreateTextureFromSurface, "SDL_CreateTextureFromSurface");
    UI_LOAD(CreateTexture, "SDL_CreateTexture");
    UI_LOAD(CreateTextureWithProperties, "SDL_CreateTextureWithProperties");
    UI_LOAD(UpdateTexture, "SDL_UpdateTexture");
    UI_LOAD(SetTextureBlendMode, "SDL_SetTextureBlendMode");
    UI_LOAD(SetRenderTarget, "SDL_SetRenderTarget");
    UI_LOAD(RenderTexture, "SDL_RenderTexture");
    UI_LOAD(RenderReadPixels, "SDL_RenderReadPixels");
    UI_LOAD(ReadSurfacePixel, "SDL_ReadSurfacePixel");
    UI_LOAD(WriteSurfacePixel, "SDL_WriteSurfacePixel");
    UI_LOAD(DestroyTexture, "SDL_DestroyTexture");
    UI_LOAD(DestroySurface, "SDL_DestroySurface");
    UI_LOAD(RenderPresent, "SDL_RenderPresent");
    UI_LOAD(SetRenderVSync, "SDL_SetRenderVSync");
    UI_LOAD(GetError, "SDL_GetError");
    UI_LOAD(GetTicksNS, "SDL_GetTicksNS");
    UI_LOAD(IOFromConstMem, "SDL_IOFromConstMem");
    UI_LOAD(CloseIO, "SDL_CloseIO");
    if (!g_sdl.IsMainThread()) { ui_set_error("SDL video operations require the main thread"); return false; }
    if (!g_sdl.Init(UI_SDL_INIT_VIDEO)) {
        ui_set_error(g_sdl.GetError());
        return false;
    }
    g_sdl.ready = true;
    return true;
}

#undef UI_LOAD

#define UI_TTF_LOAD(field, symbol) do { \
    *(void **)(&g_ttf.field) = ui_symbol(g_ttf.library, symbol); \
    if (g_ttf.field == NULL) { ui_set_error("SDL3_ttf is missing required font symbols"); return false; } \
} while (0)

static bool ui_load_ttf(void) {
    if (g_ttf.attempted) return g_ttf.ready;
    g_ttf.attempted = true;
    if (!g_ttf.library) g_ttf.library = ui_open_ttf_library();
    if (g_ttf.library == NULL) {
        ui_set_error("SDL3_ttf runtime was not found");
        return false;
    }
    UI_TTF_LOAD(Init, "TTF_Init");
    UI_TTF_LOAD(Quit, "TTF_Quit");
    UI_TTF_LOAD(OpenFont, "TTF_OpenFont");
    UI_TTF_LOAD(OpenFontIO, "TTF_OpenFontIO");
    UI_TTF_LOAD(CloseFont, "TTF_CloseFont");
    UI_TTF_LOAD(SetFontStyle, "TTF_SetFontStyle");
    UI_TTF_LOAD(SetFontSize, "TTF_SetFontSize");
    UI_TTF_LOAD(GetStringSize, "TTF_GetStringSize");
    UI_TTF_LOAD(GetStringSizeWrapped, "TTF_GetStringSizeWrapped");
    UI_TTF_LOAD(MeasureString, "TTF_MeasureString");
    UI_TTF_LOAD(RenderTextBlended, "TTF_RenderText_Blended");
    UI_TTF_LOAD(RenderTextBlendedWrapped, "TTF_RenderText_Blended_Wrapped");
    UI_TTF_LOAD(CreateSurfaceTextEngine, "TTF_CreateSurfaceTextEngine");
    UI_TTF_LOAD(DestroySurfaceTextEngine, "TTF_DestroySurfaceTextEngine");
    UI_TTF_LOAD(CreateText, "TTF_CreateText");
    UI_TTF_LOAD(DestroyText, "TTF_DestroyText");
    UI_TTF_LOAD(GetTextSize, "TTF_GetTextSize");
    UI_TTF_LOAD(GetTextSubStringsForRange, "TTF_GetTextSubStringsForRange");
    if (!g_ttf.Init()) {
        ui_set_error(g_sdl.GetError());
        return false;
    }
    g_ttf.ready = true;
    return true;
}

#undef UI_TTF_LOAD

/* All platform objects are main-thread confined. Raw dispatch is an unsafe
 * escape hatch: callers retain the runtime until their final object is gone. */
const UiSdlApi *encore_sdl_api_v1(void) { return &g_sdl; }
const UiTtfApi *encore_sdl_ttf_api_v1(void) { return &g_ttf; }
const char *encore_sdl_error_cstr(void) { return g_error; }
encore_str encore_sdl_error(void) {
    return (encore_str){encore_str_from_cstr(g_error)};
}
bool encore_sdl_main_thread(void) {
    if (g_sdl.ready && g_sdl.IsMainThread()) return true;
    ui_set_error("SDL operation requires the initialized main thread");
    return false;
}
bool encore_sdl_acquire(void) {
    if (!ui_load_sdl() || !encore_sdl_main_thread()) return false;
    ++g_references;
    return true;
}
bool encore_sdl_load_ttf(void) {
    return encore_sdl_main_thread() && ui_load_ttf();
}
bool encore_sdl_release(void) {
    if (!encore_sdl_main_thread() || !g_references) return false;
    if (--g_references) return true;
    if (g_ttf.ready) { g_ttf.Quit(); g_ttf.ready = false; }
    g_ttf.attempted = false;
    g_sdl.Quit();
    g_sdl.ready = false;
    g_sdl.attempted = false;
    return true;
}

/* SDL owns text only until the next pump. Queue our own copy before polling
 * another window, and retain delivered text until that window's next poll. */
typedef struct SdlPending {
    UiSdlEvent event;
    char *text;
    struct SdlPending *next;
} SdlPending;
typedef struct SdlWindowState {
    UiSdlWindow *window;
    uint32_t id;
    size_t token;
    unsigned pins;
    UiSdlCursor *cursor;
    SdlPending *head, *tail;
    char *delivered_text;
    UiSdlEvent last;
    struct SdlWindowState *next;
} SdlWindowState;
static SdlWindowState *sdl_windows;
static size_t next_token = 1;

static SdlWindowState *sdl_find(UiSdlWindow *window) {
    for (SdlWindowState *s = sdl_windows; s; s = s->next)
        if (s->window == window) return s;
    return NULL;
}
static SdlWindowState *sdl_token(size_t token) {
    if (!encore_sdl_main_thread()) return NULL;
    for (SdlWindowState *s = sdl_windows; s; s = s->next)
        if (token && s->token == token) return s;
    ui_set_error("Invalid or closed SDL window");
    return NULL;
}
bool encore_sdl_track_window(UiSdlWindow *window) {
    if (!encore_sdl_main_thread() || !window) return false;
    if (sdl_find(window)) return true;
    SdlWindowState *s = calloc(1, sizeof *s);
    if (!s) { ui_set_error("Unable to allocate SDL event queue"); return false; }
    s->window = window;
    s->id = g_sdl.GetWindowID(window);
    s->next = sdl_windows;
    sdl_windows = s;
    return true;
}
void encore_sdl_forget_events(UiSdlWindow *window) {
    if (!encore_sdl_main_thread()) return;
    SdlWindowState **link = &sdl_windows;
    while (*link && (*link)->window != window) link = &(*link)->next;
    if (!*link) return;
    SdlWindowState *s = *link;
    *link = s->next;
    while (s->head) {
        SdlPending *p = s->head; s->head = p->next;
        free(p->text); free(p);
    }
    free(s->delivered_text);
    if (s->cursor) g_sdl.DestroyCursor(s->cursor);
    free(s);
}
static bool sdl_enqueue(SdlWindowState *s, const UiSdlEvent *event) {
    SdlPending *p = calloc(1, sizeof *p);
    if (!p) { ui_set_error("Unable to queue SDL event"); return false; }
    p->event = *event;
    if (event->type == UI_SDL_EVENT_TEXT_INPUT) {
        const char *text = ((const UiSdlTextInputEvent *)event)->text;
        if (text) {
            p->text = malloc(strlen(text) + 1);
            if (!p->text) { free(p); ui_set_error("Unable to queue SDL text"); return false; }
            strcpy(p->text, text);
        }
        ((UiSdlTextInputEvent *)&p->event)->text = p->text;
    }
    if (s->tail) s->tail->next = p; else s->head = p;
    s->tail = p;
    return true;
}
static void sdl_route(const UiSdlEvent *event) {
    uint32_t id = 0;
    /* SDL window, keyboard and mouse events share the windowID prefix. Other
       global events are broadcast; they are never interpreted as a windowID. */
    if ((event->type >= 0x200 && event->type < 0x400) ||
        (event->type >= 0x400 && event->type <= 0x405))
        memcpy(&id, event->padding + 16, sizeof id);
    for (SdlWindowState *s = sdl_windows; s; s = s->next)
        if (!id || id == s->id) sdl_enqueue(s, event);
}
static bool sdl_pop(SdlWindowState *s, UiSdlEvent *out) {
    if (!s->head) return false;
    SdlPending *p = s->head;
    s->head = p->next;
    if (!s->head) s->tail = NULL;
    free(s->delivered_text);
    s->delivered_text = p->text;
    *out = p->event;
    free(p);
    return true;
}
bool encore_sdl_poll_raw(UiSdlWindow *window, UiSdlEvent *out) {
    if (!encore_sdl_main_thread() || !out) return false;
    SdlWindowState *s = sdl_find(window);
    if (!s) { ui_set_error("Unregistered SDL window"); return false; }
    if (sdl_pop(s, out)) return true;
    UiSdlEvent event;
    while (g_sdl.PollEvent(&event)) {
        sdl_route(&event);
        if (sdl_pop(s, out)) return true;
    }
    return false;
}
bool encore_sdl_wait_raw(UiSdlWindow *window, UiSdlEvent *out, int timeout) {
    if (encore_sdl_poll_raw(window, out)) return true;
    if (!encore_sdl_main_thread()) return false;
    SdlWindowState *s = sdl_find(window);
    if (!s) return false;
    uint64_t start = g_sdl.GetTicksNS();
    for (;;) {
        int remaining = timeout;
        if (timeout >= 0) {
            uint64_t elapsed = (g_sdl.GetTicksNS() - start) / 1000000;
            if (elapsed >= (uint64_t)timeout) return false;
            remaining -= (int)elapsed;
        }
        UiSdlEvent event;
        if (!g_sdl.WaitEventTimeout(&event, remaining)) return false;
        sdl_route(&event);
        if (sdl_pop(s, out)) return true;
    }
}
/* Wake the application for any registered window without consuming that
 * window's event. A per-window wait can otherwise sleep while a satellite
 * already has queued input. Callers drain their windows after this wake. */
bool encore_sdl_wait_any(int timeout) {
    if (!encore_sdl_main_thread()) return false;
    for (SdlWindowState *s = sdl_windows; s; s = s->next)
        if (s->head) return true;
    UiSdlEvent event;
    if (g_sdl.PollEvent(&event)) { sdl_route(&event); return true; }
    if (!sdl_windows || timeout == 0) return false;
    if (!g_sdl.WaitEventTimeout(&event, timeout)) return false;
    sdl_route(&event);
    return true;
}
static bool (*sdl_vk_load)(const char *);
static void (*sdl_vk_unload)(void);
static const char *const *(*sdl_vk_extensions)(uint32_t *);
static bool (*sdl_vk_create)(UiSdlWindow *, void *, const void *, uint64_t *);
static void (*sdl_vk_destroy)(void *, uint64_t, const void *);
bool encore_sdl_vulkan_acquire(void) {
    if (!encore_sdl_acquire()) return false;
    if (g_vulkan_references) { ++g_vulkan_references; return true; }
    *(void **)&sdl_vk_load = ui_symbol(g_sdl.library, "SDL_Vulkan_LoadLibrary");
    *(void **)&sdl_vk_unload = ui_symbol(g_sdl.library, "SDL_Vulkan_UnloadLibrary");
    *(void **)&sdl_vk_extensions = ui_symbol(g_sdl.library, "SDL_Vulkan_GetInstanceExtensions");
    *(void **)&sdl_vk_create = ui_symbol(g_sdl.library, "SDL_Vulkan_CreateSurface");
    *(void **)&sdl_vk_destroy = ui_symbol(g_sdl.library, "SDL_Vulkan_DestroySurface");
    if (!sdl_vk_load || !sdl_vk_unload || !sdl_vk_extensions ||
        !sdl_vk_create || !sdl_vk_destroy || !sdl_vk_load(NULL)) {
        ui_set_error("SDL Vulkan integration is unavailable");
        encore_sdl_release(); return false;
    }
    ++g_vulkan_references;
    return true;
}
void encore_sdl_vulkan_release(void) {
    if (!encore_sdl_main_thread() || !g_vulkan_references) return;
    if (!--g_vulkan_references) sdl_vk_unload();
    encore_sdl_release();
}
const char *const *encore_sdl_vulkan_extensions(uint32_t *count) {
    if (!encore_sdl_main_thread() || !g_vulkan_references) return NULL;
    return sdl_vk_extensions(count);
}
bool encore_sdl_vulkan_surface(UiSdlWindow *window, void *instance, uint64_t *surface) {
    return encore_sdl_main_thread() && g_vulkan_references &&
        sdl_vk_create(window, instance, NULL, surface);
}
void encore_sdl_vulkan_surface_destroy(void *instance, uint64_t surface) {
    if (encore_sdl_main_thread() && g_vulkan_references)
        sdl_vk_destroy(instance, surface, NULL);
}
size_t encore_sdl_window_create(encore_str title, uint32_t width, uint32_t height, uint32_t mode) {
    if (!width || !height || width > INT_MAX || height > INT_MAX || mode > 3 || !next_token) {
        ui_set_error("Invalid SDL window dimensions or graphics mode"); return 0;
    }
    if (!encore_sdl_acquire()) return 0;
    uint64_t flags = 0x8u | 0x20u | 0x2000u;
    /* Hidden, resizable, high DPI; graphics capability chosen before creation. */
    if (mode == 1) flags |= 0x10000000ull;
    if (mode == 2) flags |= 0x2ull;
    if (mode == 3) flags |= 0x20000000ull;
    UiSdlWindow *w = g_sdl.CreateWindow(title.object ? title.object->data : "", (int)width, (int)height, flags);
    if (!w) { ui_set_error(g_sdl.GetError()); encore_sdl_release(); return 0; }
    if (!encore_sdl_track_window(w)) {
        g_sdl.DestroyWindow(w); encore_sdl_release(); return 0;
    }
    SdlWindowState *s = sdl_find(w);
    s->token = next_token++;
    return s->token;
}
bool encore_sdl_window_close(size_t token) {
    SdlWindowState *s = sdl_token(token);
    if (!s) return false;
    if (s->pins) { ui_set_error("SDL window still has an attached GPU surface"); return false; }
    UiSdlWindow *w = s->window;
    encore_sdl_forget_events(w);
    g_sdl.DestroyWindow(w);
    encore_sdl_release();
    return true;
}
size_t encore_sdl_window_native(size_t token) {
    SdlWindowState *s = sdl_token(token);
    return s ? (size_t)s->window : 0;
}
bool encore_sdl_window_pin(size_t token, bool pin) {
    SdlWindowState *s = sdl_token(token);
    if (!s) return false;
    if (pin) { if (s->pins == UINT_MAX) return false; ++s->pins; }
    else { if (!s->pins) return false; --s->pins; }
    return true;
}
bool encore_sdl_window_show(size_t token) {
    SdlWindowState *s = sdl_token(token);
    return s && g_sdl.ShowWindow(s->window);
}
uint32_t encore_sdl_window_size(size_t token, uint32_t component) {
    SdlWindowState *s = sdl_token(token);
    int w = 0, h = 0;
    if (!s) return 0;
    if (component < 2) g_sdl.GetWindowSize(s->window, &w, &h);
    else g_sdl.GetWindowSizeInPixels(s->window, &w, &h);
    return (uint32_t)((component & 1) ? h : w);
}
float encore_sdl_window_scale(size_t token) {
    SdlWindowState *s = sdl_token(token);
    return s ? g_sdl.GetWindowPixelDensity(s->window) : 0;
}
bool encore_sdl_text_input(size_t token, bool enabled) {
    SdlWindowState *s = sdl_token(token);
    return s && (enabled ? g_sdl.StartTextInput(s->window) : g_sdl.StopTextInput(s->window));
}
bool encore_sdl_relative_mouse(size_t token, bool enabled) {
    SdlWindowState *s = sdl_token(token);
    return s && g_sdl.SetWindowRelativeMouseMode && g_sdl.SetWindowRelativeMouseMode(s->window, enabled);
}
bool encore_sdl_cursor(size_t token, uint32_t kind) {
    SdlWindowState *s = sdl_token(token);
    if (!s || kind >= 20) return false;
    UiSdlCursor *cursor = g_sdl.CreateSystemCursor(kind);
    if (!cursor) return false;
    if (!g_sdl.SetCursor(cursor)) { g_sdl.DestroyCursor(cursor); return false; }
    if (s->cursor) g_sdl.DestroyCursor(s->cursor);
    s->cursor = cursor;
    return true;
}
bool encore_sdl_clipboard_set(encore_str text) {
    return encore_sdl_main_thread() && g_sdl.SetClipboardText(text.object ? text.object->data : "");
}
encore_str encore_sdl_clipboard_get(void) {
    char *text = encore_sdl_main_thread() ? g_sdl.GetClipboardText() : NULL;
    encore_str result = {encore_str_from_cstr(text ? text : "")};
    if (text) g_sdl.Free(text);
    return result;
}
/* Stable, flat language ABI. Payload access is bounded by the most recent event. */
uint32_t encore_sdl_event(size_t token, int32_t timeout) {
    if (timeout < -1) { ui_set_error("Invalid SDL event timeout"); return 0; }
    SdlWindowState *s = sdl_token(token);
    if (!s) return 0;
    memset(&s->last, 0, sizeof s->last);
    bool found = timeout == 0 ? encore_sdl_poll_raw(s->window, &s->last)
        : encore_sdl_wait_raw(s->window, &s->last, timeout);
    return found ? s->last.type : 0;
}
float encore_sdl_event_number(size_t token, uint32_t field) {
    SdlWindowState *s = sdl_token(token);
    if (!s) return 0;
    UiSdlEvent *e = &s->last;
    if (e->type == UI_SDL_EVENT_MOUSE_MOTION) {
        UiSdlMouseMotionEvent *v = (void *)e;
        return field == 0 ? v->x : field == 1 ? v->y : field == 2 ? v->xrel : v->yrel;
    }
    if (e->type == UI_SDL_EVENT_MOUSE_WHEEL) {
        UiSdlMouseWheelEvent *v = (void *)e;
        float sign = v->direction == 1 ? -1 : 1;
        return field == 0 ? v->mouse_x : field == 1 ? v->mouse_y : field == 2 ? v->x * sign : v->y * sign;
    }
    if (e->type == UI_SDL_EVENT_MOUSE_BUTTON_DOWN || e->type == UI_SDL_EVENT_MOUSE_BUTTON_UP) {
        UiSdlMouseButtonEvent *v = (void *)e;
        return field == 0 ? v->x : v->y;
    }
    return 0;
}
uint32_t encore_sdl_event_integer(size_t token, uint32_t field) {
    SdlWindowState *s = sdl_token(token);
    if (!s) return 0;
    UiSdlEvent *e = &s->last;
    if (e->type == UI_SDL_EVENT_KEY_DOWN || e->type == UI_SDL_EVENT_KEY_UP) {
        UiSdlKeyboardEvent *v = (void *)e;
        return field == 0 ? v->key : field == 1 ? v->scancode : field == 2 ? v->mod : v->repeat;
    }
    if (e->type == UI_SDL_EVENT_MOUSE_BUTTON_DOWN || e->type == UI_SDL_EVENT_MOUSE_BUTTON_UP) {
        UiSdlMouseButtonEvent *v = (void *)e;
        return field == 0 ? v->button : v->clicks;
    }
    if (e->type >= 0x200 && e->type < 0x300) {
        UiSdlWindowEvent *v = (void *)e;
        return (uint32_t)(field == 0 ? v->data1 : v->data2);
    }
    return 0;
}
encore_str encore_sdl_event_text(size_t token) {
    SdlWindowState *s = sdl_token(token);
    const char *text = s && s->last.type == UI_SDL_EVENT_TEXT_INPUT
        ? ((UiSdlTextInputEvent *)&s->last)->text : "";
    return (encore_str){encore_str_from_cstr(text ? text : "")};
}
