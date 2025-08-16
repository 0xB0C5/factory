emcc -D WEB -sEXPORTED_FUNCTIONS=_setup,_loop,_getScreen,_setInputs,_consumeDelay -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,HEAPU8 -o web/factory-wasm.js ^
    web.cpp arduino_web.cpp battery.cpp factory.cpp ^
    fatal_error.cpp game.cpp generated_graphics.cpp input.cpp lcd.cpp recipe.cpp ^
    render.cpp ui.cpp ui_update.cpp fs_web.cpp

