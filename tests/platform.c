#include <assert.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "../runtime.c"
#define ABI_SIZE(a,b) _Static_assert(sizeof(a) == sizeof(b), #a " size")
#define ABI_FIELD(a,b,x,y) _Static_assert(offsetof(a,x) == offsetof(b,y), #a "." #x)
ABI_SIZE(UiSdlEvent, SDL_Event);
ABI_SIZE(UiSdlSurface, SDL_Surface);
ABI_SIZE(UiSdlKeyboardEvent, SDL_KeyboardEvent);
ABI_SIZE(UiSdlMouseMotionEvent, SDL_MouseMotionEvent);
ABI_SIZE(UiSdlMouseButtonEvent, SDL_MouseButtonEvent);
ABI_SIZE(UiSdlTextInputEvent, SDL_TextInputEvent);
ABI_SIZE(UiSdlWindowEvent, SDL_WindowEvent);
ABI_SIZE(UiTtfText, TTF_Text);
ABI_SIZE(UiTtfSubString, TTF_SubString);
ABI_FIELD(UiSdlMouseWheelEvent, SDL_MouseWheelEvent, mouse_x, mouse_x);
ABI_FIELD(UiSdlKeyboardEvent, SDL_KeyboardEvent, key, key);
static bool main_thread = true;
static bool is_main(void) { return main_thread; }
static uint32_t window_id(UiSdlWindow *w) { return (uint32_t)(uintptr_t)w; }
static UiSdlEvent events[8];
static unsigned at, count, quits;
static bool poll_event(UiSdlEvent *out) {
    if (at == count) return false;
    *out = events[at++];
    return true;
}
static void quit(void) { ++quits; }
static void event(uint32_t window, uint32_t kind) {
    UiSdlWindowEvent *e = (void *)&events[count++];
    e->type = kind; e->window_id = window;
}
int main(void) {
    g_sdl.ready = true; g_sdl.attempted = true;
    g_sdl.IsMainThread = is_main; g_sdl.GetWindowID = window_id;
    g_sdl.PollEvent = poll_event; g_sdl.Quit = quit;
    UiSdlWindow *one = (void *)1, *two = (void *)2;
    assert(encore_sdl_acquire() && encore_sdl_acquire());
    assert(encore_sdl_track_window(one) && encore_sdl_track_window(two));
    event(2, UI_SDL_EVENT_MOUSE_BUTTON_DOWN);
    char unicode[] = "Привет 🌍";
    UiSdlTextInputEvent *text = (void *)&events[count++];
    text->type = UI_SDL_EVENT_TEXT_INPUT; text->window_id = 2; text->text = unicode;
    event(1, UI_SDL_EVENT_WINDOW_RESIZED);
    event(0, UI_SDL_EVENT_QUIT);
    UiSdlEvent e;
    assert(encore_sdl_wait_any(0));
    assert(at == 1); /* The event for window two is queued, not delivered. */
    assert(encore_sdl_wait_any(0) && at == 1); /* Never block on window one. */
    assert(encore_sdl_poll_raw(one, &e) && e.type == UI_SDL_EVENT_WINDOW_RESIZED);
    memset(unicode, 'x', sizeof unicode - 1);
    assert(encore_sdl_poll_raw(two, &e) && e.type == UI_SDL_EVENT_MOUSE_BUTTON_DOWN);
    assert(encore_sdl_poll_raw(two, &e) && e.type == UI_SDL_EVENT_TEXT_INPUT);
    assert(strcmp(((UiSdlTextInputEvent *)&e)->text, "Привет 🌍") == 0);
    assert(encore_sdl_poll_raw(one, &e) && e.type == UI_SDL_EVENT_QUIT);
    assert(encore_sdl_poll_raw(two, &e) && e.type == UI_SDL_EVENT_QUIT);
    assert(!encore_sdl_poll_raw(one, &e));
    main_thread = false;
    assert(!encore_sdl_release() && !encore_sdl_poll_raw(one, &e));
    main_thread = true;
    encore_sdl_forget_events(one); encore_sdl_forget_events(two);
    assert(!sdl_windows);
    assert(encore_sdl_release() && quits == 0);
    assert(encore_sdl_release() && quits == 1 && !g_sdl.ready);
    return 0;
}
