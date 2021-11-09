#include "apps.h"
#include "gui.h"
#include "buttons.h"

int selectedApp = 0;

typedef struct {
    const char *name;
    int mode;
} app;

const app apps[] = {
    {"Mandelbrot", m_mandelbrot}
};

int apps_needsRedraw = 1;

void applist_updateScreen() {
    if (!apps_needsRedraw) return;
    for (int i = 0; i < sizeof(apps) / sizeof(app); i++) {
        gui_draw_string(apps[i].name, 0, i * (fonts[FNT_SM][FNT_HEIGHT] + 1), FNT_SM, (selectedApp == i));
    }
    apps_needsRedraw = 0;
}

int applist_buttonPress(int btn, int *mode) {
    if (btn == btn_up) {
        if (selectedApp > 0) {
            selectedApp--;
            apps_needsRedraw = 1;
        }
    }
    if (btn == btn_down) {
        if (selectedApp + 1 < sizeof(apps) / sizeof(app)) {
            selectedApp++;
            apps_needsRedraw = 1;
        }
    }
    if (btn == enter) {
        *mode = apps[selectedApp].mode;
        disp_clear();
        apps_needsRedraw = 1;
    }
}