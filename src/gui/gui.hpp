#pragma once

namespace gui {
    inline const char* GLSL_VERSION = "#version 430";


    int guiInit();
    int guiRender();
    int guiTerminate();

    int guiDraw();
}
