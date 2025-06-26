#pragma once
#include "inputMap.h"

namespace Dog
{
    /*
    inline std::unordered_map<Key, int> keyMap = {
        {Key::UNKNOWN, ultralight::KeyCodes::GK_UNKNOWN},

        {Key::SPACE, ultralight::KeyCodes::GK_SPACE},
        {Key::APOSTROPHE, ultralight::KeyCodes::GK_OEM_7},
        {Key::COMMA, ultralight::KeyCodes::GK_OEM_COMMA},
        {Key::MINUS, ultralight::KeyCodes::GK_OEM_MINUS},
        {Key::PERIOD, ultralight::KeyCodes::GK_OEM_PERIOD},
        {Key::SLASH, ultralight::KeyCodes::GK_OEM_2},

        {Key::NUM0, ultralight::KeyCodes::GK_0},
        {Key::NUM1, ultralight::KeyCodes::GK_1},
        {Key::NUM2, ultralight::KeyCodes::GK_2},
        {Key::NUM3, ultralight::KeyCodes::GK_3},
        {Key::NUM4, ultralight::KeyCodes::GK_4},
        {Key::NUM5, ultralight::KeyCodes::GK_5},
        {Key::NUM6, ultralight::KeyCodes::GK_6},
        {Key::NUM7, ultralight::KeyCodes::GK_7},
        {Key::NUM8, ultralight::KeyCodes::GK_8},
        {Key::NUM9, ultralight::KeyCodes::GK_9},

        {Key::SEMICOLON, ultralight::KeyCodes::GK_OEM_1},
        {Key::EQUAL, ultralight::KeyCodes::GK_OEM_PLUS},

        {Key::A, ultralight::KeyCodes::GK_A},
        {Key::B, ultralight::KeyCodes::GK_B},
        {Key::C, ultralight::KeyCodes::GK_C},
        {Key::D, ultralight::KeyCodes::GK_D},
        {Key::E, ultralight::KeyCodes::GK_E},
        {Key::F, ultralight::KeyCodes::GK_F},
        {Key::G, ultralight::KeyCodes::GK_G},
        {Key::H, ultralight::KeyCodes::GK_H},
        {Key::I, ultralight::KeyCodes::GK_I},
        {Key::J, ultralight::KeyCodes::GK_J},
        {Key::K, ultralight::KeyCodes::GK_K},
        {Key::L, ultralight::KeyCodes::GK_L},
        {Key::M, ultralight::KeyCodes::GK_M},
        {Key::N, ultralight::KeyCodes::GK_N},
        {Key::O, ultralight::KeyCodes::GK_O},
        {Key::P, ultralight::KeyCodes::GK_P},
        {Key::Q, ultralight::KeyCodes::GK_Q},
        {Key::R, ultralight::KeyCodes::GK_R},
        {Key::S, ultralight::KeyCodes::GK_S},
        {Key::T, ultralight::KeyCodes::GK_T},
        {Key::U, ultralight::KeyCodes::GK_U},
        {Key::V, ultralight::KeyCodes::GK_V},
        {Key::W, ultralight::KeyCodes::GK_W},
        {Key::X, ultralight::KeyCodes::GK_X},
        {Key::Y, ultralight::KeyCodes::GK_Y},
        {Key::Z, ultralight::KeyCodes::GK_Z},

        {Key::LEFTBRACKET, ultralight::KeyCodes::GK_OEM_4},
        {Key::BACKSLASH, ultralight::KeyCodes::GK_OEM_5},
        {Key::RIGHTBRACKET, ultralight::KeyCodes::GK_OEM_6},
        {Key::GRAVEACCENT, ultralight::KeyCodes::GK_OEM_3},

        {Key::ESCAPE, ultralight::KeyCodes::GK_ESCAPE},
        {Key::ENTER, ultralight::KeyCodes::GK_RETURN},
        {Key::TAB, ultralight::KeyCodes::GK_TAB},
        {Key::BACKSPACE, ultralight::KeyCodes::GK_BACK},
        {Key::INSERT, ultralight::KeyCodes::GK_INSERT},
        {Key::DEL, ultralight::KeyCodes::GK_DELETE},

        {Key::RIGHT, ultralight::KeyCodes::GK_RIGHT},
        {Key::LEFT, ultralight::KeyCodes::GK_LEFT},
        {Key::DOWN, ultralight::KeyCodes::GK_DOWN},
        {Key::UP, ultralight::KeyCodes::GK_UP},

        {Key::PAGEUP, ultralight::KeyCodes::GK_PRIOR},
        {Key::PAGEDOWN, ultralight::KeyCodes::GK_NEXT},
        {Key::HOME, ultralight::KeyCodes::GK_HOME},
        {Key::END, ultralight::KeyCodes::GK_END},

        {Key::CAPSLOCK, ultralight::KeyCodes::GK_CAPITAL},
        {Key::NUMLOCK, ultralight::KeyCodes::GK_NUMLOCK},
        {Key::SCROLLLOCK, ultralight::KeyCodes::GK_SCROLL},
        {Key::PRINTSCREEN, ultralight::KeyCodes::GK_SNAPSHOT},
        {Key::BREAK, ultralight::KeyCodes::GK_PAUSE},

        {Key::F1, ultralight::KeyCodes::GK_F1},
        {Key::F2, ultralight::KeyCodes::GK_F2},
        {Key::F3, ultralight::KeyCodes::GK_F3},
        {Key::F4, ultralight::KeyCodes::GK_F4},
        {Key::F5, ultralight::KeyCodes::GK_F5},
        {Key::F6, ultralight::KeyCodes::GK_F6},
        {Key::F7, ultralight::KeyCodes::GK_F7},
        {Key::F8, ultralight::KeyCodes::GK_F8},
        {Key::F9, ultralight::KeyCodes::GK_F9},
        {Key::F10, ultralight::KeyCodes::GK_F10},
        {Key::F11, ultralight::KeyCodes::GK_F11},
        {Key::F12, ultralight::KeyCodes::GK_F12},

        {Key::KEYPAD0, ultralight::KeyCodes::GK_NUMPAD0},
        {Key::KEYPAD1, ultralight::KeyCodes::GK_NUMPAD1},
        {Key::KEYPAD2, ultralight::KeyCodes::GK_NUMPAD2},
        {Key::KEYPAD3, ultralight::KeyCodes::GK_NUMPAD3},
        {Key::KEYPAD4, ultralight::KeyCodes::GK_NUMPAD4},
        {Key::KEYPAD5, ultralight::KeyCodes::GK_NUMPAD5},
        {Key::KEYPAD6, ultralight::KeyCodes::GK_NUMPAD6},
        {Key::KEYPAD7, ultralight::KeyCodes::GK_NUMPAD7},
        {Key::KEYPAD8, ultralight::KeyCodes::GK_NUMPAD8},
        {Key::KEYPAD9, ultralight::KeyCodes::GK_NUMPAD9},

        {Key::KEYPADDECIMAL, ultralight::KeyCodes::GK_DECIMAL},
        {Key::KEYPADMULTIPLY, ultralight::KeyCodes::GK_MULTIPLY},
        {Key::KEYPADDIVIDE, ultralight::KeyCodes::GK_DIVIDE},
        {Key::KEYPADMINUS, ultralight::KeyCodes::GK_SUBTRACT},
        {Key::KEYPADPLUS, ultralight::KeyCodes::GK_ADD},
        {Key::KEYPADENTER, ultralight::KeyCodes::GK_RETURN},
        {Key::KEYPADEQUAL, ultralight::KeyCodes::GK_OEM_PLUS},

        {Key::LEFTSHIFT, ultralight::KeyCodes::GK_LSHIFT},
        {Key::LEFTCONTROL, ultralight::KeyCodes::GK_LCONTROL},
        {Key::LEFTALT, ultralight::KeyCodes::GK_LMENU},
        {Key::LEFTSUPER, ultralight::KeyCodes::GK_LWIN},

        {Key::RIGHTSHIFT, ultralight::KeyCodes::GK_RSHIFT},
        {Key::RIGHTCONTROL, ultralight::KeyCodes::GK_RCONTROL},
        {Key::RIGHTALT, ultralight::KeyCodes::GK_RMENU},
        {Key::RIGHTSUPER, ultralight::KeyCodes::GK_RWIN},

        {Key::MENU, ultralight::KeyCodes::GK_APPS}
    };
    */
}
