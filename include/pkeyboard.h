#ifndef PTSD_KEYBOARD_HEADER
#define PTSD_KEYBOARD_HEADER

#include "general.h"
#include "pplatform.h"
#include "pwindow.h"

enum pmodifier_t PTSD_IN_VERSION(PTSD_C23, : u8) {
    PTSD_MOD_SHIFT     = 0b000001,
    PTSD_MOD_CONTROL   = 0b000010,
    PTSD_MOD_ALT       = 0b000100,
    PTSD_MOD_SUPER     = 0b001000,
    PTSD_MOD_CAPS_LOCK = 0b010000,
    PTSD_MOD_NUM_LOCK  = 0b100000,
};
PTSD_BEFORE_VERSION(PTSD_C23, typedef u8 pmodifier_t);
PTSD_IN_VERSION(    PTSD_C23, typedef enum pmodifier_t pmodifier_t);

typedef struct pkey_state_t pkey_state_t;
struct pkey_state_t {
    enum pkey_action_state_t PTSD_IN_VERSION(PTSD_C23, : u8) {
        PKB_RELEASED, PKB_PRESSED, PKB_HELD
    } action;
    usize       code;
    pmodifier_t modifiers;
    bool held, pressed, released;
};
PTSD_BEFORE_VERSION(PTSD_C23, typedef u8 pkey_action_state_t);
PTSD_IN_VERSION(    PTSD_C23, typedef enum pkey_action_state_t pkey_action_state_t);

enum pkeycode_t PTSD_IN_VERSION(PTSD_C23, : s16) {
    PTSD_KEY_UNKNOWN      = -1,

    PTSD_KEY_SPACE         = ' ',
    PTSD_KEY_APOSTROPHE    = '\'',
    PTSD_KEY_COMMA         = ',',
    PTSD_KEY_MINUS         = '-',
    PTSD_KEY_PERIOD        = '.',
    PTSD_KEY_SLASH         = '/',
    PTSD_KEY_0             = '0',
    PTSD_KEY_1             = '1',
    PTSD_KEY_2             = '2',
    PTSD_KEY_3             = '3',
    PTSD_KEY_4             = '4',
    PTSD_KEY_5             = '5',
    PTSD_KEY_6             = '6',
    PTSD_KEY_7             = '7',
    PTSD_KEY_8             = '8',
    PTSD_KEY_9             = '9',
    PTSD_KEY_SEMICOLON     = ';',
    PTSD_KEY_EQUAL         = '=',
    PTSD_KEY_A             = 'A', /* note: 'a' also works */
    PTSD_KEY_B             = 'B', /* note: 'b' also works */
    PTSD_KEY_C             = 'C', /* note: 'c' also works */
    PTSD_KEY_D             = 'D', /* note: 'd' also works */
    PTSD_KEY_E             = 'E', /* note: 'e' also works */
    PTSD_KEY_F             = 'F', /* note: 'f' also works */
    PTSD_KEY_G             = 'G', /* note: 'g' also works */
    PTSD_KEY_H             = 'H', /* note: 'h' also works */
    PTSD_KEY_I             = 'I', /* note: 'i' also works */
    PTSD_KEY_J             = 'J', /* note: 'j' also works */
    PTSD_KEY_K             = 'K', /* note: 'k' also works */
    PTSD_KEY_L             = 'L', /* note: 'l' also works */
    PTSD_KEY_M             = 'M', /* note: 'm' also works */
    PTSD_KEY_N             = 'N', /* note: 'n' also works */
    PTSD_KEY_O             = 'O', /* note: 'o' also works */
    PTSD_KEY_P             = 'P', /* note: 'p' also works */
    PTSD_KEY_Q             = 'Q', /* note: 'q' also works */
    PTSD_KEY_R             = 'R', /* note: 'r' also works */
    PTSD_KEY_S             = 'S', /* note: 's' also works */
    PTSD_KEY_T             = 'T', /* note: 't' also works */
    PTSD_KEY_U             = 'U', /* note: 'u' also works */
    PTSD_KEY_V             = 'V', /* note: 'v' also works */
    PTSD_KEY_W             = 'W', /* note: 'w' also works */
    PTSD_KEY_X             = 'X', /* note: 'x' also works */
    PTSD_KEY_Y             = 'Y', /* note: 'y' also works */
    PTSD_KEY_Z             = 'Z', /* note: 'z' also works */
    PTSD_KEY_LEFT_BRACKET  = '[',
    PTSD_KEY_BACKSLASH     = '\\',
    PTSD_KEY_RIGHT_BRACKET = ']',
    PTSD_KEY_GRAVE_ACCENT  = '`',
    PTSD_KEY_WORLD_1       = 161, /* non-US #1 */
    PTSD_KEY_WORLD_2       = 162, /* non-US #2 */

    PTSD_KEY_ESCAPE        = 256,
    PTSD_KEY_ENTER         = 257,
    PTSD_KEY_TAB           = 258,
    PTSD_KEY_BACKSPACE     = 259,
    PTSD_KEY_INSERT        = 260,
    PTSD_KEY_DELETE        = 261,
    PTSD_KEY_RIGHT         = 262,
    PTSD_KEY_LEFT          = 263,
    PTSD_KEY_DOWN          = 264,
    PTSD_KEY_UP            = 265,
    PTSD_KEY_PAGE_UP       = 266,
    PTSD_KEY_PAGE_DOWN     = 267,
    PTSD_KEY_HOME          = 268,
    PTSD_KEY_END           = 269,
    PTSD_KEY_CAPS_LOCK     = 280,
    PTSD_KEY_SCROLL_LOCK   = 281,
    PTSD_KEY_NUM_LOCK      = 282,
    PTSD_KEY_PRINT_SCREEN  = 283,
    PTSD_KEY_PAUSE         = 284,
    PTSD_KEY_F1            = 290,
    PTSD_KEY_F2            = 291,
    PTSD_KEY_F3            = 292,
    PTSD_KEY_F4            = 293,
    PTSD_KEY_F5            = 294,
    PTSD_KEY_F6            = 295,
    PTSD_KEY_F7            = 296,
    PTSD_KEY_F8            = 297,
    PTSD_KEY_F9            = 298,
    PTSD_KEY_F10           = 299,
    PTSD_KEY_F11           = 300,
    PTSD_KEY_F12           = 301,
    PTSD_KEY_F13           = 302,
    PTSD_KEY_F14           = 303,
    PTSD_KEY_F15           = 304,
    PTSD_KEY_F16           = 305,
    PTSD_KEY_F17           = 306,
    PTSD_KEY_F18           = 307,
    PTSD_KEY_F19           = 308,
    PTSD_KEY_F20           = 309,
    PTSD_KEY_F21           = 310,
    PTSD_KEY_F22           = 311,
    PTSD_KEY_F23           = 312,
    PTSD_KEY_F24           = 313,
    PTSD_KEY_F25           = 314,
    PTSD_KEY_KP_0          = 320,
    PTSD_KEY_KP_1          = 321,
    PTSD_KEY_KP_2          = 322,
    PTSD_KEY_KP_3          = 323,
    PTSD_KEY_KP_4          = 324,
    PTSD_KEY_KP_5          = 325,
    PTSD_KEY_KP_6          = 326,
    PTSD_KEY_KP_7          = 327,
    PTSD_KEY_KP_8          = 328,
    PTSD_KEY_KP_9          = 329,
    PTSD_KEY_KP_DECIMAL    = 330,
    PTSD_KEY_KP_DIVIDE     = 331,
    PTSD_KEY_KP_MULTIPLY   = 332,
    PTSD_KEY_KP_SUBTRACT   = 333,
    PTSD_KEY_KP_ADD        = 334,
    PTSD_KEY_KP_ENTER      = 335,
    PTSD_KEY_KP_EQUAL      = 336,
    PTSD_KEY_LEFT_SHIFT    = 340,
    PTSD_KEY_LEFT_CONTROL  = 341,
    PTSD_KEY_LEFT_ALT      = 342,
    PTSD_KEY_LEFT_SUPER    = 343,
    PTSD_KEY_RIGHT_SHIFT   = 344,
    PTSD_KEY_RIGHT_CONTROL = 345,
    PTSD_KEY_RIGHT_ALT     = 346,
    PTSD_KEY_RIGHT_SUPER   = 347,
    PTSD_KEY_MENU          = 348,
    PTSD_KEY_COUNT         = 512,
};
PTSD_BEFORE_VERSION(PTSD_C23, typedef s16 pkeycode_t);
PTSD_IN_VERSION(    PTSD_C23, typedef enum pkeycode_t pkeycode_t);



typedef struct pkeyboard_t pkeyboard_t;
struct pkeyboard_t {
    pdevice_t       device;
    pkeycode_t      keycodes[PTSD_KEY_COUNT];
    usize           scancodes[PTSD_KEY_COUNT];
    pkey_state_t    keys[PTSD_KEY_COUNT];
    pkeycode_t last_key;
};

PTSD_UNUSED
static inline pkeyboard_t pkeyboard(void) {
    void                  pkeyboard_init(pdevice_t *const);
    void                  pkeyboard_shutdown(pdevice_t *const);
    void                  pkeyboard_update(pdevice_t *const);
    pdevice_proc_result_t pkeyboard_device_proc(pdevice_t *const,
            const pdevice_parameter_pack_t *const);

    return (pkeyboard_t) {
        .device = {
            .init     = pkeyboard_init,
            .shutdown = pkeyboard_shutdown,
            .update   = pkeyboard_update,
            .device_proc = pkeyboard_device_proc,
        },
        .last_key = PTSD_KEY_UNKNOWN
    };
}

bool         pkb_is_pressed(const pkeyboard_t *const kb, pkeycode_t);
bool         pkb_is_released(const pkeyboard_t *const kb, pkeycode_t);
bool         pkb_is_held(const pkeyboard_t *const kb, pkeycode_t);
pkey_state_t pkb_get_key(const pkeyboard_t *const kb, pkeycode_t);
pkey_state_t pkb_get_last_key(const pkeyboard_t *const kb);
bool         pkb_any_key_pressed(const pkeyboard_t *kb);

pmodifier_t  pkb_get_modifiers(const pkeyboard_t *const kb);


#endif
