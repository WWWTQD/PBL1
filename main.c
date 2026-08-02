#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <string.h>

#include "SDL.h"
#include "SDL_ttf.h"

#define V 10 // Số đỉnh tối đa trong đồ thị




// ************************************************************ SDL ************************************************************ //


typedef struct {
    int x;
    int y;
} SDL_Pos;




typedef struct {
    int id;
    SDL_Pos pos;

    // for connecting 2 node
    bool isSelected;

    bool isSource;
    bool isSink;
} node;

node nodeList[V];

int currentPageId = 1;

bool isSelectingSource = false,
    isSelectingSink = false,
    isSourceSelected = false,
    isSinkSelected = false;

int selectedSource = -1,
    selectedSink = -1;

bool isConnectingNodes = false;


bool running = true;   // Có đang chạy hay không
bool paused = false;   // Đang pause?
bool restart = false;  // Reset

// Hàm xử lý sự kiện pause, resume
void handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_SPACE:  // Nhấn SPACE để dừng/tiếp tục
                    paused = !paused;
                    break;
                case SDLK_ESCAPE: // ESC để thoát luôn
                    running = false;
                    break;
            }
        }
    }
}

void delayWithPause(Uint32 ms) {
    Uint32 start = SDL_GetTicks();
    while (SDL_GetTicks() - start < ms) {
        handleEvents();  // Kiểm tra phím nhấn
        if (!running) break;
        while (paused) {
            handleEvents();  // Cho phép nhấn tiếp tục
            SDL_Delay(100);
        }
        SDL_Delay(10);
    }
}



void SDL_ErrorHandler(char *message,...)
{
    SDL_Log("%s failed: %s\n", message);
    SDL_Quit();
    TTF_Quit();
    exit(1);
}


void ShowMessageBox(const char *message, SDL_Window *window) {
    SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Ok" }
    };

    SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION,
        window,
        "Caution",
        message,
        SDL_arraysize(buttons),
        buttons,
        NULL
    };

    int buttonId;
    if (SDL_ShowMessageBox(&messageboxdata, &buttonId) < 0) {
        SDL_ErrorHandler("SDL_ShowMessageBox", SDL_GetError());
    }
}


// *********************************************** SDL_TTF *********************************************** //


TTF_Font *Font(const int size)
{
    TTF_Font *font = TTF_OpenFont("Roboto.ttf", size);
    if (!font)
    {
        SDL_ErrorHandler("TTF_OpenFont", TTF_GetError());
    }
    return font;
}


SDL_Texture *getTextTexture(SDL_Renderer *renderer, const char *text, int fontSize, SDL_Color fgColor)
{

    TTF_Font *font = Font(fontSize);

    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, fgColor);
    if (!textSurface)
    {
        SDL_ErrorHandler("TTF_RenderText", TTF_GetError());
    }

    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    if (!textTexture)
    {
        SDL_ErrorHandler("SDL_CreateTextureFromSurface", SDL_GetError());
    }

    return textTexture;
}

void SDL_RenderText(SDL_Renderer *renderer, const char *text, int fontSize, SDL_Color fgColor, SDL_Rect textRect)
{
    TTF_Font *font = Font(fontSize);

    SDL_Texture *textTexture = getTextTexture(renderer, text, fontSize, fgColor);

    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);

}


// ve hinh tron
int SDL_RenderDrawCircle(SDL_Renderer *renderer,const int x,const int y,const int radius)
{
    int offsetX = 0;
    int offsetY = radius;
    int d = radius - 1;
    int status = 0;
    while (offsetY >= offsetX)
    {
        status += SDL_RenderDrawPoint(renderer, x + offsetX, y + offsetY);
        status += SDL_RenderDrawPoint(renderer, x + offsetY, y + offsetX);
        status += SDL_RenderDrawPoint(renderer, x - offsetX, y + offsetY);
        status += SDL_RenderDrawPoint(renderer, x - offsetY, y + offsetX);
        status += SDL_RenderDrawPoint(renderer, x + offsetX, y - offsetY);
        status += SDL_RenderDrawPoint(renderer, x + offsetY, y - offsetX);
        status += SDL_RenderDrawPoint(renderer, x - offsetX, y - offsetY);
        status += SDL_RenderDrawPoint(renderer, x - offsetY, y - offsetX);
        if (status < 0)
        {
            status = -1;
            break;
        }
        if (d >= 2*offsetX)
        {
            d -= 2*offsetX + 1;
            offsetX +=1;
        }
        else if (d < 2 * (radius - offsetY))
        {
            d += 2 * offsetY - 1;
            offsetY -= 1;
        }
        else {
            d += 2 * (offsetY - offsetX - 1);
            offsetY -= 1;
            offsetX += 1;
        }
    }
    return status;
};

// fill hinh tron
int SDL_RenderFillCircle(SDL_Renderer *renderer, const int x, const int y, const int radius) {
    int offsetX = 0;
    int offsetY = radius;
    int d = radius - 1;
    int status = 0;
    while (offsetY >= offsetX) {
        status += SDL_RenderDrawLine(renderer, x - offsetY, y + offsetX, x + offsetY, y + offsetX);
        status += SDL_RenderDrawLine(renderer, x - offsetX, y + offsetY, x + offsetX, y + offsetY);
        status += SDL_RenderDrawLine(renderer, x - offsetX, y - offsetY, x + offsetX, y - offsetY);
        status += SDL_RenderDrawLine(renderer, x - offsetY, y - offsetX, x + offsetY, y - offsetX);
        if (status < 0) {
            status = -1;
            break;
        }
        if (d >= 2*offsetX) {
            d -= 2*offsetX + 1;
            offsetX +=1;
        }
        else if (d < 2 * (radius - offsetY)) {
            d += 2 * offsetY - 1;
            offsetY -= 1;
        }
        else {
            d += 2 * (offsetY - offsetX - 1);
            offsetY -= 1;
            offsetX += 1;
        }
    }
    return status;
};


bool SDLNode_isContain(const SDL_Pos _p, const node _n) {
    int dx = _p.x - _n.pos.x;
    int dy = _p.y - _n.pos.y;
    return (dx * dx + dy * dy <= 20 * 20);  // 20 is radius
}


void SDL_DrawNode(SDL_Renderer *renderer, const SDL_Pos _p, int _id, bool flag)
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderFillCircle(renderer, _p.x, _p.y, 20);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderDrawCircle(renderer, _p.x, _p.y, 20);
    int textWidth = 16, textHeight = 16;
    char buffer[20];
    sprintf(buffer, "%d", _id);
    SDL_Texture *textTexture = getTextTexture(renderer, buffer, 25, (SDL_Color){0xFF, 0xFF, 0x00, 0xFF});
    SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);
    SDL_Rect textRect = {_p.x - textWidth / 2, _p.y - textHeight / 2, textWidth, textHeight};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
    if (flag) SDL_RenderPresent(renderer);
}

void SDLGraphic_ConnectNode(SDL_Renderer *renderer, const SDL_Pos _p_node1, const SDL_Pos _p_node2, const int m, const bool selected, int id1, int id2) {
    if (m < 0 && !selected) return;

    // Node radius
    const int nodeRadius = 20;
    const int arrowLength = 10; // Length of the arrowhead

    // Set draw color
    (selected ? SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF) : SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF));

    int offset = 10; // Offset to differentiate overlapping lines
    int x_start = _p_node1.x + (id1 < id2 ? -10 : 10) + (id1 * offset) % 10;
    int y_start = _p_node1.y + (id1 < id2 ? -10 : 10) + (id1 * offset) % 10;
    int x_dest = _p_node2.x + (id1 < id2 ? -10 : 10) + (id2 * offset) % 10;
    int y_dest = _p_node2.y + (id1 < id2 ? -10 : 10) + (id2 * offset) % 10;

    // Calculate direction and back off by radius
    double angle = atan2(y_dest - y_start, x_dest - x_start);
    x_dest -= nodeRadius * cos(angle);
    y_dest -= nodeRadius * sin(angle);

    // === VẼ ĐƯỜNG THẲNG VÀ MŨI TÊN THEO MÀU ===
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

    // Draw the main line
    SDL_RenderDrawLine(renderer, x_start, y_start, x_dest, y_dest);

    // Calculate arrowhead points
    int arrow_x1 = x_dest - arrowLength * cos(angle - M_PI / 6);
    int arrow_y1 = y_dest - arrowLength * sin(angle - M_PI / 6);
    int arrow_x2 = x_dest - arrowLength * cos(angle + M_PI / 6);
    int arrow_y2 = y_dest - arrowLength * sin(angle + M_PI / 6);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);

    // Draw the arrowhead
    SDL_RenderDrawLine(renderer, x_dest, y_dest, arrow_x1, arrow_y1);
    SDL_RenderDrawLine(renderer, x_dest, y_dest, arrow_x2, arrow_y2);

    int textWidth = 60, textHeight = 60;

    if (m < 0) return;
    // int to string
    char buffer[20];
    sprintf(buffer, "%d", m);
    SDL_Texture *textTexture = getTextTexture(renderer, buffer, 20, (SDL_Color){0x00, 0x00, 0xFF, 0xFF});

    // Find midpoint for cost display
    double x_midPoint = x_start + (x_dest - x_start) * 0.5;
    double y_midPoint = y_start + (y_dest - y_start) * 0.5;

    double offsetNum = 30.0;
    // Shift the label slightly forward or backward along the edge line
    double label_offset = ((id1 + id2) % 3 - 1) * offsetNum;  // -15, 0, or 15
    x_midPoint += label_offset * cos(angle);
    y_midPoint += label_offset * sin(angle);

    SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);

    SDL_Rect textRect = {x_midPoint - textWidth / 2, y_midPoint - textHeight / 2, textWidth, textHeight};
    SDL_SetRenderDrawColor(renderer, 0xB8, 0xB8, 0xB8, 0xFF);
    SDL_RenderFillRect(renderer, &textRect);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
}

void SDLGraphic_ConnectNodeWithText(SDL_Renderer *renderer, const SDL_Pos _p_node1, const SDL_Pos _p_node2, const char *m, int id1, int id2, SDL_Color color) {
    // Node radius
    const int nodeRadius = 20;
    const int arrowLength = 10; // Length of the arrowhead

    // Set draw color
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

    int offset = 10; // Offset to differentiate overlapping lines
    int x_start = _p_node1.x + (id1 < id2 ? -10 : 10) + (id1 * offset) % 10;
    int y_start = _p_node1.y + (id1 < id2 ? -10 : 10) + (id1 * offset) % 10;
    int x_dest = _p_node2.x + (id1 < id2 ? -10 : 10) + (id2 * offset) % 10;
    int y_dest = _p_node2.y + (id1 < id2 ? -10 : 10) + (id2 * offset) % 10;

    // Calculate direction and back off by radius
    double angle = atan2(y_dest - y_start, x_dest - x_start);

    x_dest -= nodeRadius * cos(angle);
    y_dest -= nodeRadius * sin(angle);

    // Draw the main line
    SDL_RenderDrawLine(renderer, x_start, y_start, x_dest, y_dest);

    // Calculate arrowhead points
    int arrow_x1 = x_dest - arrowLength * cos(angle - M_PI / 6);
    int arrow_y1 = y_dest - arrowLength * sin(angle - M_PI / 6);
    int arrow_x2 = x_dest - arrowLength * cos(angle + M_PI / 6);
    int arrow_y2 = y_dest - arrowLength * sin(angle + M_PI / 6);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);

    // Draw the arrowhead
    SDL_RenderDrawLine(renderer, x_dest, y_dest, arrow_x1, arrow_y1);
    SDL_RenderDrawLine(renderer, x_dest, y_dest, arrow_x2, arrow_y2);

    int textWidth = 60, textHeight = 60;

    SDL_Texture *textTexture = getTextTexture(renderer, m, 20, (SDL_Color){color.r, color.g, color.b, color.a});

    // Find midpoint for cost display
    double x_midPoint = x_start + (x_dest - x_start) * 0.5;
    double y_midPoint = y_start + (y_dest - y_start) * 0.5;

    double offsetNum = 30.0;
    // Shift the label slightly forward or backward along the edge line
    double label_offset = ((id1 + id2) % 3 - 1) * offsetNum;  // -15, 0, or 15
    x_midPoint += label_offset * cos(angle);
    y_midPoint += label_offset * sin(angle);

    SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);

    SDL_Rect textRect = {x_midPoint - textWidth / 2, y_midPoint - textHeight / 2, textWidth, textHeight};
    SDL_SetRenderDrawColor(renderer, 0xB8, 0xB8, 0xB8, 0xFF);
    SDL_RenderFillRect(renderer, &textRect);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
}

// *********************************************** SDL_Button *********************************************** //

typedef struct
{
    int x;
    int y;
    int w;
    int h;

    char *text;
    int fontSize;

    SDL_Color bgColor;
    SDL_Color fgColor;

    int pageId;

} SDL_Button;


bool SDLButton_isContain(const SDL_Pos _p, SDL_Button _button)
{
    return (
        _p.x >= _button.x &&
        _p.x <= _button.x + _button.w &&
        _p.y >= _button.y &&
        _p.y <= _button.y + _button.h
        );
};

void SDLButton_draw(SDL_Renderer *renderer, const SDL_Button _button)
{
    SDL_SetRenderDrawColor(renderer, _button.bgColor.r, _button.bgColor.g, _button.bgColor.b, _button.bgColor.a);
    const SDL_Rect rect = {_button.x, _button.y, _button.w, _button.h};
    SDL_RenderFillRect(renderer, &rect);
    // text
    SDL_SetRenderDrawColor(renderer, _button.fgColor.r, _button.fgColor.g, _button.fgColor.b, _button.fgColor.a);
    SDL_Texture *textTexture = getTextTexture(renderer, _button.text, _button.fontSize, _button.fgColor);
    int textRectW = rect.w - 6, textRectH = rect.h - 6;
    SDL_QueryTexture(textTexture, NULL, NULL, &textRectW, &textRectH);
    SDL_Rect textRect = {rect.x + (rect.w - textRectW) / 2,rect.y + (rect.h - textRectH) / 2, textRectW, textRectH};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
};

SDL_Button buttons[8] = {
    {
        .x = 590,
        .y = 630,
        .w = 100,
        .h = 35,

        .text = "START",
        .fontSize = 30,

        .bgColor = {0xA2, 0xA5, 0xB1, 0xFF},
        .fgColor = {0x00, 0x00, 0x00, 0xFF},

        .pageId = 1
    },
    {
        .x = 1195,
        .y = 665,
        .w = 65,
        .h = 35,

        .text = "RUN",
        .fontSize = 30,

        .bgColor = {0xA2, 0xA5, 0xB1, 0xFF},
        .fgColor = {0x00, 0x00, 0x00, 0xFF},

        .pageId = 3

    },
    {
        .x = -10000,
        .y = -10000,
        .w = 100,
        .h = 35,

        .text = "Mo file",
        .fontSize = 30,

        .bgColor = {0xA2, 0xA5, 0xB1, 0xFF},
        .fgColor = {0x00, 0x00, 0x00, 0xFF},

        .pageId = 3
    },
    {
        .x = 20,
        .y = 665,
        .w = 100,
        .h = 35,

        .text = "Source",
        .fontSize = 30,

        .bgColor = {0xFE, 0x99, 0x00, 0xFF},
        .fgColor = {0x00, 0x00, 0x00, 0xFF},

        .pageId = 3
    },
    {
        .x = 140,
        .y = 665,
        .w = 65,
        .h = 35,

        .text = "Sink",
        .fontSize = 30,

        .bgColor = {0xFE, 0x99, 0x00, 0xFF},
        .fgColor = {0x00, 0x00, 0x00, 0xFF},

        .pageId = 3
    },
    {
        .x = 500,
        .y = 250,
        .w = 280,
        .h = 60,

        .text = "Ford-Fulkerson",
        .fontSize = 28,

        .bgColor = {0xFE, 0x99, 0x00, 0xFF},
        .fgColor = {0x00, 0x00, 0x00, 0xFF},

        .pageId = 2
    },
    {
        .x = 500,
        .y = 350,
        .w = 280,
        .h = 60,

        .text = "Edmonds-Karp",
        .fontSize = 28,

        .bgColor = {0xFE, 0x99, 0x00, 0xFF},
        .fgColor = {0x00, 0x00, 0x00, 0xFF},

        .pageId = 2
    },
    {
        .x = 500,
        .y = 450, // Vị trí thấp hơn nút "Dinic"
        .w = 280,
        .h = 60,

        .text = "Dinic",
        .fontSize = 28,

        .bgColor = {0xFE, 0x99, 0x00, 0xFF},
        .fgColor = {0x00, 0x00, 0x00, 0xFF},

        .pageId = 2
    },
};

SDL_Color originButtonColor = {0xA2, 0xA5, 0xB1, 0xFF};

void SDLRenderPreset(SDL_Renderer *renderer, int n, int graph[V][V], int max_flow, int flow[V][V]) {
    SDL_SetRenderDrawColor(renderer, 0xB8, 0xB8, 0xB8, 0xFF);
    SDL_RenderClear(renderer);


    for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
        if (buttons[i].pageId == currentPageId) {
            SDLButton_draw(renderer, buttons[i]);
        }
    }


    if (currentPageId == 1) {
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_Rect containerRect = { 150, 50, 980, 620};

        SDL_RenderDrawLine(renderer, containerRect.x + containerRect.w / 2, 210, containerRect.x + containerRect.w / 2, containerRect.y + containerRect.h - 50);

        SDL_Rect PBL_TITLE_rect = { 307, containerRect.y + 50, 665, 50};
        SDL_RenderText(renderer, "PBL1: DO AN LAP TRINH TINH TOAN", 40, (SDL_Color){0x00, 0x00, 0x00, 0xFF}, PBL_TITLE_rect);

        SDL_Rect PBL_PROJECT_NAME_rect = { 161, PBL_TITLE_rect.y + PBL_TITLE_rect.h, 957, 40};
        SDL_RenderText(renderer, "DE TAI: XAY DUNG CHUONG TRINH TIM LUONG CUC DAI TREN MANG G", 30, (SDL_Color){0x00, 0x00, 0x00, 0xFF}, PBL_PROJECT_NAME_rect);

        SDL_Rect SV_THUC_HIEN_rect = { containerRect.x + 95, PBL_PROJECT_NAME_rect.y + PBL_PROJECT_NAME_rect.h + 50, 300, 35};
        SDL_RenderText(renderer, "SINH VIEN THUC HIEN", 30, (SDL_Color){0x00, 0x00, 0x00, 0xFF}, SV_THUC_HIEN_rect);

        SDL_Rect SV_1_rect = { containerRect.x + 42, SV_THUC_HIEN_rect.y + SV_THUC_HIEN_rect.h + 83, 355, 30};
        SDL_RenderText(renderer, "Truong Quang Dat", 25, (SDL_Color){0x00, 0x00, 0x00, 0xFF}, SV_1_rect);
        SDL_Rect SV_1_id_rect = { 483, SV_THUC_HIEN_rect.y + SV_THUC_HIEN_rect.h + 83, 125, 30};
        SDL_RenderText(renderer, "102240304", 25, (SDL_Color){0x00, 0x00, 0x00, 0xFF}, SV_1_id_rect);

        SDL_Rect SV_2_rect = { containerRect.x + 42, SV_1_rect.y + SV_1_rect.h + 83, 260, 30};
        SDL_RenderText(renderer, "Nguyen Hai Long", 25, (SDL_Color){0x00, 0x00, 0x00, 0xFF}, SV_2_rect);
        SDL_Rect SV_2_id_rect = { SV_2_rect.x + SV_2_rect.w + 31, SV_1_rect.y + SV_1_rect.h + 83, 125, 30};
        SDL_RenderText(renderer, "102240318", 25, (SDL_Color){0x00, 0x00, 0x00, 0xFF}, SV_2_id_rect);

        SDL_Rect GV_HUONG_DAN_rect = { containerRect.x + containerRect.w - 75 - 340, PBL_PROJECT_NAME_rect.y + PBL_PROJECT_NAME_rect.h + 50, 340, 35};
        SDL_RenderText(renderer, "GIANG VIEN HUONG DAN", 30, (SDL_Color){0x00, 0x00, 0x00, 0xFF}, GV_HUONG_DAN_rect);

        SDL_Rect GV_rect = { containerRect.x + containerRect.w - 127 - 235, GV_HUONG_DAN_rect.y + GV_HUONG_DAN_rect.h + 83, 235, 30};
        SDL_RenderText(renderer, "Ts. Pham Cong Thang", 25, (SDL_Color){0x00, 0x00, 0x00, 0xFF}, GV_rect);
    } else if (currentPageId == 2) {
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_Rect OPTION_rect = { 300, 120, 665, 50};
        SDL_RenderText(renderer, "CAC THUAT TOAN LUA CHON",  50, (SDL_Color){0x00, 0x00, 0x00, 0xFF}, OPTION_rect);
    } else if (currentPageId == 3) {

        if (max_flow < 0){
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    if (graph[i][j] > 0) SDLGraphic_ConnectNode(renderer, nodeList[i].pos, nodeList[j].pos, graph[i][j], false, i, j);
                }
            }
        } else {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    if (graph[i][j] > 0) {
                        char m[300];
                        sprintf(m, "%d/%d", flow[i][j], graph[i][j]);

                        // Calculate the ratio of flow to capacity
                        float ratio = (float)flow[i][j] / graph[i][j];

                        // Interpolate color from green to red
                        SDL_Color color = {
                            .r = (Uint8)(255 * ratio),          // Red increases with ratio
                            .g = (Uint8)(255 * (1.0f - ratio)), // Green decreases with ratio
                            .b = 0,                             // Blue remains 0
                            .a = 255                            // Fully opaque
                        };

                        SDLGraphic_ConnectNodeWithText(renderer, nodeList[i].pos, nodeList[j].pos, m, i, j, color);
                    }
                }
            }

        }

        for (int i = 0; i < n; i++) {
            SDL_DrawNode(renderer, nodeList[i].pos, nodeList[i].id, false);
            if (nodeList[i].isSource) {
                SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
                SDL_RenderDrawCircle(renderer, nodeList[i].pos.x, nodeList[i].pos.y, 20);

                // render text
                int textWidth = 60, textHeight = 60;

                char buffer[20];
                sprintf(buffer, "%d", nodeList[i].id);
                SDL_Texture *textTexture = getTextTexture(renderer, buffer, 25, (SDL_Color){0x00, 0xFF, 0x00, 0xFF});
                SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);
                SDL_Rect textRect = {nodeList[i].pos.x - textWidth / 2, nodeList[i].pos.y - textHeight / 2, textWidth, textHeight};
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);

            }
            else if (nodeList[i].isSink) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
                SDL_RenderDrawCircle(renderer, nodeList[i].pos.x, nodeList[i].pos.y, 20);

                // render text
                int textWidth = 60, textHeight = 60;

                char buffer[20];
                sprintf(buffer, "%d", nodeList[i].id);
                SDL_Texture *textTexture = getTextTexture(renderer, buffer, 25, (SDL_Color){0xFF, 0x00, 0x00, 0xFF});
                SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);
                SDL_Rect textRect = {nodeList[i].pos.x - textWidth / 2, nodeList[i].pos.y - textHeight / 2, textWidth, textHeight};
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
        }

        // show result
        if (max_flow >= 0) {
            SDL_Rect maxFlowRect = {225, 665, 100, 35};
            char b[20];
            sprintf(b, "%d", max_flow);
            char tt[300] = "Max flow = ";
            strcat(tt, b);
            SDL_RenderText(renderer, tt, 30, (SDL_Color){0x00, 0x00, 0x00, 0xFF}, maxFlowRect);
        }

    }


    SDL_RenderPresent(renderer);

}

// *********************************************** Data input *********************************************** //


int numInputProcess(const int keysym)
{
    if (keysym >= 48 && keysym <= 57)
    {
        return keysym - 48;
    }
    if (keysym >= 1073741913 && keysym <= 1073741922) return (keysym == 1073741922 ? 0 : keysym - 1073741912);
    if (keysym == 1073741910 || keysym == 45) return -1;
    if (keysym == 1073741911 || keysym == 61) return -2;
    return -3;
}

void keyInputWait(SDL_Renderer *renderer, SDL_Event _event, const SDL_Pos _p_node1, const SDL_Pos _p_node2, int *cost, const int id1, const int id2, int n, int graph[V][V], int flow[10][10])
{
    SDLGraphic_ConnectNode(renderer, _p_node1, _p_node2, -1, true, id1, id2);
    SDL_RenderPresent(renderer);

    bool isNegative = false;

    bool awaitInput = true;
    *cost = 0;
    while (awaitInput)
    {
        while (SDL_PollEvent(&_event))
        {
            switch (_event.type)
            {
            case SDL_QUIT:
                break;
            case SDL_KEYDOWN:
                switch (_event.key.keysym.sym)
                {
                    case SDLK_KP_ENTER:
                        awaitInput = false;
                        break;
                    case SDLK_RETURN:
                        awaitInput = false;
                        break;
                    case SDLK_BACKSPACE:
                        *cost = trunc(*cost / 10);
                        break;
                    case SDLK_ESCAPE:
                        *cost = -1;
                        awaitInput = false;
                        break;
                    default:
                        int keysym = numInputProcess(_event.key.keysym.sym);
                        if (keysym == -1)
                        {
                            isNegative = true;
                            *cost *= -1;
                        }
                        else if (keysym == -2)
                        {
                            isNegative = false;
                            *cost *= -1;
                        }
                        else if (keysym != -3) *cost = *cost * 10 + (isNegative ? -1 * keysym : keysym);
                        break;
                }
                break;
            default:
                SDLRenderPreset(renderer, n, graph, -1, flow);
                SDLGraphic_ConnectNode(renderer, _p_node1, _p_node2, *cost, true, id1, id2);
                SDL_RenderPresent(renderer);
                break;
            }
        }
    }
}


void addNode(SDL_Renderer *renderer, SDL_Pos _p, int *n, int graph[V][V]) {
    graph[*n][*n] = 0;
    for (int i = 0; i < *n; i++) {
        graph[*n][i] = 0;
        graph[i][*n] = 0;
    }
    nodeList[*n] = (node){
        .pos = _p,
        .id = *n,
        .isSelected = false,
        .isSource = false,
        .isSink = false,
    };
    SDL_DrawNode(renderer, _p, *n, true);
    (*n)++;
}

typedef enum {
    FORD_FULKERSON,
    EDMONDS_KARP,
    DINIC,
} AlgorithmType;

AlgorithmType selectedAlgorithm = FORD_FULKERSON;

int level[V];

// Tạo cây mức bằng BFS
bool dinicbfs(int graph[V][V], int s, int t, int n) {
    memset(level, -1, sizeof(level));
    int queue[V], front = 0, rear = 0;
    queue[rear++] = s;
    level[s] = 0;

    while (front < rear) {
        int u = queue[front++];
        for (int v = 0; v < n; v++) {
            if (level[v] < 0 && graph[u][v] > 0) {
                level[v] = level[u] + 1;
                queue[rear++] = v;
            }
        }
    }
    return level[t] >= 0;
}

// DFS tìm đường tăng và đẩy luồng
int Dinicdfs(int rGraph[V][V], int u, int t, int flow, int n, int path[], int *pathLen) {
    if (u == t) return flow;

    for (int v = 0; v < n; v++) {
        if (rGraph[u][v] > 0 && level[v] == level[u] + 1) {
            path[*pathLen] = v;
            (*pathLen)++;
            int pushed = Dinicdfs(rGraph, v, t, flow < rGraph[u][v] ? flow : rGraph[u][v], n, path, pathLen);
            if (pushed > 0) {
                rGraph[u][v] -= pushed;
                rGraph[v][u] += pushed;
                return pushed;
            }
            (*pathLen)--;
        }
    }
    return 0;
}

// Hàm chính tính Max Flow bằng thuật toán Dinic có hiển thị SDL
int Dinic(int graph[V][V], int s, int t, int flow[V][V], int n, SDL_Renderer *renderer) {
    int rGraph[V][V];  // Đồ thị dư
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            rGraph[i][j] = graph[i][j];
            flow[i][j] = 0;
        }

    int max_flow = 0;
    running = true;
    paused = false;

    while (running && dinicbfs(rGraph, s, t, n)) {
        while (true) {
            int path[V], pathLen = 1;
            path[0] = s;

            int pushed = Dinicdfs(rGraph, s, t, INT_MAX, n, path, &pathLen);
            if (pushed <= 0) break;
            max_flow += pushed;

            // Đánh dấu đường đi hiện tại
            bool forwardEdge[V][V] = { false };
            bool backwardEdge[V][V] = { false };

            for (int i = 0; i < pathLen - 1; i++) {
                int u = path[i], v = path[i+1];
                if (graph[u][v] > 0) forwardEdge[u][v] = true;
                else backwardEdge[v][u] = true;
            }


            // === GIAI ĐOẠN 1: Hiển thị flow cũ + tô màu đường tăng ===
            SDL_SetRenderDrawColor(renderer, 184, 184, 184, 255);
            SDL_RenderClear(renderer);

            for (int i = 0; i < n; i++)
                SDL_DrawNode(renderer, nodeList[i].pos, nodeList[i].id, false);

            for (int u = 0; u < n; u++) {
                for (int v = 0; v < n; v++) {
                    if (graph[u][v] > 0) {
                        SDL_Color color = {0x00, 0x00, 0x00, 0xFF}; // mặc định
                        if (forwardEdge[u][v]) color = (SDL_Color){0x32, 0xCD, 0x32, 0xFF};
                        else if (backwardEdge[u][v]) color = (SDL_Color){0xFF, 0x00, 0x00, 0xFF};

                        char text[20];
                        sprintf(text, "%d/%d", flow[u][v], graph[u][v]);
                        SDLGraphic_ConnectNodeWithText(renderer, nodeList[u].pos, nodeList[v].pos, text, u, v, color);
                    }
                }
            }

            SDL_RenderPresent(renderer);
            delayWithPause(1500);
            if (!running) break;

            // Cập nhật luồng chính thức
            for (int i = 0; i < pathLen - 1; i++) {
                int u = path[i], v = path[i+1];
                flow[u][v] += pushed;
            }

            // === GIAI ĐOẠN 2: Hiển thị flow mới + reset màu đen ===
            SDL_SetRenderDrawColor(renderer, 184, 184, 184, 255);
            SDL_RenderClear(renderer);

            for (int i = 0; i < n; i++)
                SDL_DrawNode(renderer, nodeList[i].pos, nodeList[i].id, false);

            for (int u = 0; u < n; u++) {
                for (int v = 0; v < n; v++) {
                    if (graph[u][v] > 0) {
                        SDL_Color color = {0x00, 0x00, 0x00, 0xFF};

                        if (forwardEdge[u][v]) color = (SDL_Color){0x32, 0xCD, 0x32, 0xFF};
                        else if (backwardEdge[u][v]) color = (SDL_Color){0xFF, 0x00, 0x00, 0xFF};
                        char text[20];
                        sprintf(text, "%d/%d", flow[u][v], graph[u][v]);
                        SDLGraphic_ConnectNodeWithText(renderer, nodeList[u].pos, nodeList[v].pos, text, u, v, color);
                    }
                }
            }

            SDL_RenderPresent(renderer);
            delayWithPause(1500);
            if (!running) break;
        }
    }

    // Tính lại flow từ rGraph và graph
    for (int u = 0; u < n; u++)
        for (int v = 0; v < n; v++)
            flow[u][v] = graph[u][v] - rGraph[u][v];

    return max_flow;
}




// Tìm đường tăng luồng bằng BFS
bool EdmondsKarpbfs(int rGraph[V][V], int s, int t, int parent[], int n) {
    bool visited[V] = {false};
    int queue[V], front = 0, rear = 0;
    queue[rear++] = s;
    visited[s] = true;
    parent[s] = -1;

    while (front < rear) {
        int u = queue[front++];
        for (int v = 0; v < n; v++) {
            if (!visited[v] && rGraph[u][v] > 0) {
                queue[rear++] = v;
                parent[v] = u;
                visited[v] = true;
            }
        }
    }
    return visited[t];
}


// Thuật toán Edmonds-Karp
int EdmondsKarp(int graph[V][V], int s, int t, int flow[V][V], int n, SDL_Renderer *renderer) {

    int rGraph[V][V];
    for (int u = 0; u < n; u++)
        for (int v = 0; v < n; v++) {
            rGraph[u][v] = graph[u][v];
            flow[u][v] = 0;
        }

    int parent[V];
    int max_flow = 0;

    running = true;
    paused = false;

    while (running && EdmondsKarpbfs(rGraph, s, t, parent, n)) {
    int path_flow = INT_MAX;
    for (int v = t; v != s; v = parent[v]) {
        int u = parent[v];
        path_flow = path_flow < rGraph[u][v] ? path_flow : rGraph[u][v];
    }

    bool forwardEdge[V][V] = { false };
    bool backwardEdge[V][V] = { false };

    for (int v = t; v != s; v = parent[v]) {
        int u = parent[v];
        if (rGraph[u][v] > 0) {
            forwardEdge[u][v] = true;
        } else {
            backwardEdge[v][u] = true;
        }
    }

    // === GIAI ĐOẠN 1: Vẽ màu sắc cạnh mới, flow cũ ===
    SDL_SetRenderDrawColor(renderer, 0xB8, 0xB8, 0xB8, 0xFF);
    SDL_RenderClear(renderer);

    for (int i = 0; i < n; i++) {
        SDL_DrawNode(renderer, nodeList[i].pos, nodeList[i].id, false);
    }

    for (int u = 0; u < n; u++) {
        for (int v = 0; v < n; v++) {
            if (graph[u][v] > 0) {
                SDL_Color color = {0x00, 0x00, 0x00, 0xFF};
                if (forwardEdge[u][v]) {
                    color = (SDL_Color){0x32, 0xCD, 0x32, 0xFF};
                } else if (backwardEdge[u][v]) {
                    color = (SDL_Color){0xFF, 0x00, 0x00, 0xFF};
                }

                // Ở giai đoạn 1 vẫn hiển thị flow cũ, chưa cập nhật
                char text[20];
                sprintf(text, "%d/%d", flow[u][v], graph[u][v]);
                SDLGraphic_ConnectNodeWithText(renderer, nodeList[u].pos, nodeList[v].pos, text, u, v, color);
            }
        }
    }

    SDL_RenderPresent(renderer);
    delayWithPause(1500); // delay 1.5s có pause

    if (!running) break;

    // Cập nhật luồng
    for (int v = t; v != s; v = parent[v]) {
        int u = parent[v];
        rGraph[u][v] -= path_flow;
        rGraph[v][u] += path_flow;
        flow[u][v] += path_flow;
    }
    max_flow += path_flow;

    // === GIAI ĐOẠN 2: Vẽ màu sắc cạnh giữ nguyên, flow mới ===
    SDL_SetRenderDrawColor(renderer, 0xB8, 0xB8, 0xB8, 0xFF);
    SDL_RenderClear(renderer);

    for (int i = 0; i < n; i++) {
        SDL_DrawNode(renderer, nodeList[i].pos, nodeList[i].id, false);
    }

    for (int u = 0; u < n; u++) {
        for (int v = 0; v < n; v++) {
            if (graph[u][v] > 0) {
                SDL_Color color = {0x00, 0x00, 0x00, 0xFF};
                if (forwardEdge[u][v]) {
                    color = (SDL_Color){0x32, 0xCD, 0x32, 0xFF};
                } else if (backwardEdge[u][v]) {
                    color = (SDL_Color){0xFF, 0x00, 0x00, 0xFF};
                }

                // Ở giai đoạn 2 hiển thị flow mới đã cập nhật
                char text[20];
                sprintf(text, "%d/%d", flow[u][v], graph[u][v]);
                SDLGraphic_ConnectNodeWithText(renderer, nodeList[u].pos, nodeList[v].pos, text, u, v, color);
            }
        }
    }

    SDL_RenderPresent(renderer);
    delayWithPause(1500);

    if (!running) break;
}
        return max_flow;
}


bool FordFulkersondfs(int rGraph[V][V], int s, int t, int parent[V], int n, bool visited[V]) {
    visited[s] = true;
    if (s == t) return true;

    for (int v = 0; v < n; v++) {
        if (!visited[v] && rGraph[s][v] > 0) {
            parent[v] = s;
            if (FordFulkersondfs(rGraph, v, t, parent, n, visited)) return true;
        }
    }
    return false;
}

//Hàm chính của thuật toán Ford-Fulkerson
int FordFulkerson(int graph[V][V], int s, int t, int flow[V][V], int n, SDL_Renderer *renderer) {
    int rGraph[V][V];
    for (int u = 0; u < n; u++)
        for (int v = 0; v < n; v++) {
            rGraph[u][v] = graph[u][v];
            flow[u][v] = 0;
        }

    int parent[V];
    int max_flow = 0;

    running = true;
    paused = false;

    while (running) {
        bool visited[V] = { false };
        for (int i = 0; i < n; i++) parent[i] = -1;

        if (!FordFulkersondfs(rGraph, s, t, parent, n, visited)) break;

        int path_flow = INT_MAX;
        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            if (rGraph[u][v] < path_flow)
                path_flow = rGraph[u][v];
        }

        bool forwardEdge[V][V] = { false };
        bool backwardEdge[V][V] = { false };

        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            if (rGraph[u][v] > 0)
                forwardEdge[u][v] = true;
            else
                backwardEdge[v][u] = true;
        }

        SDL_SetRenderDrawColor(renderer, 0xB8, 0xB8, 0xB8, 0xFF);
        SDL_RenderClear(renderer);

        for (int i = 0; i < n; i++)
            SDL_DrawNode(renderer, nodeList[i].pos, nodeList[i].id, false);

        for (int u = 0; u < n; u++) {
            for (int v = 0; v < n; v++) {
                if (graph[u][v] > 0) {
                    SDL_Color color = {0x00, 0x00, 0x00, 0xFF};
                    if (forwardEdge[u][v])
                        color = (SDL_Color){0x32, 0xCD, 0x32, 0xFF};
                    else if (backwardEdge[u][v])
                        color = (SDL_Color){0xFF, 0x00, 0x00, 0xFF};

                    char text[20];
                    sprintf(text, "%d/%d", flow[u][v], graph[u][v]);
                    SDLGraphic_ConnectNodeWithText(renderer, nodeList[u].pos, nodeList[v].pos, text, u, v, color);
                }
            }
        }

        SDL_RenderPresent(renderer);
        delayWithPause(1500);

        if (!running) break;

        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            rGraph[u][v] -= path_flow;
            rGraph[v][u] += path_flow;
            flow[u][v] += path_flow;
        }

        max_flow += path_flow;

        SDL_SetRenderDrawColor(renderer, 0xB8, 0xB8, 0xB8, 0xFF);
        SDL_RenderClear(renderer);

        for (int i = 0; i < n; i++)
            SDL_DrawNode(renderer, nodeList[i].pos, nodeList[i].id, false);

        for (int u = 0; u < n; u++) {
            for (int v = 0; v < n; v++) {
                if (graph[u][v] > 0) {
                    SDL_Color color = {0x00, 0x00, 0x00, 0xFF};
                    if (forwardEdge[u][v])
                        color = (SDL_Color){0x32, 0xCD, 0x32, 0xFF};
                    else if (backwardEdge[u][v])
                        color = (SDL_Color){0xFF, 0x00, 0x00, 0xFF};

                    char text[20];
                    sprintf(text, "%d/%d", flow[u][v], graph[u][v]);
                    SDLGraphic_ConnectNodeWithText(renderer, nodeList[u].pos, nodeList[v].pos, text, u, v, color);
                }
            }
        }

        SDL_RenderPresent(renderer);
        delayWithPause(1500);

        if (!running) break;
    }

    return max_flow;
}


// Nhập đồ thị từ bàn phím
void inputGraph(int graph[V][V], int *n, int *m, int *source, int *sink) {
    printf("Nhap so dinh (n <= %d): ", V);
    scanf("%d", n);
    if (*n > V) *n = V;
    printf("Nhap so cung: ");
    scanf("%d", m);
    printf("Nhap nguon (source): ");
    scanf("%d", source);
    printf("Nhap dich (sink): ");
    scanf("%d", sink);

    // Khởi tạo đồ thị
    for (int i = 0; i < V; i++)
        for (int j = 0; j < V; j++)
            graph[i][j] = 0;

    printf("Nhap %d cung (u v w):\n", *m);
    for (int i = 0; i < *m; i++) {
        int u, v, w;
        scanf("%d %d %d", &u, &v, &w);
        if (u >= 0 && u < *n && v >= 0 && v < *n)
            graph[u][v] = w;
    }
}


int main(int argc, char *argv[]) {

    int graph[V][V], flow[V][V];
    int n = 0, m, source = -1, sink = -1, max_flow = -1;

    // Khởi tạo SDL
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        SDL_ErrorHandler("SDL_Init", SDL_GetError());
    }

    if (TTF_Init() == -1) {
        SDL_ErrorHandler("TTF_Init", TTF_GetError());
    }

    SDL_Window *window = SDL_CreateWindow("PBL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_HIDDEN);
    if(window == nullptr){
        SDL_ErrorHandler("SDL_CreateWindow", SDL_GetError());
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        SDL_ErrorHandler("SDL_CreateRenderer", SDL_GetError());
    }

    SDL_ShowWindow(window);

    SDL_Color color;

    // Vòng lặp sự kiện
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    isRunning = false;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT && !isConnectingNodes) {
                        SDL_Pos MousePos = {0, 0};
                        SDL_GetMouseState(&MousePos.x, &MousePos.y);

                        if (SDLButton_isContain(MousePos, buttons[0]) && currentPageId == buttons[0].pageId) {
                            currentPageId = 2;
                            break;
                        }

                        // Chọn thuật toán ở trang OPTION
                        if (currentPageId == 2) {
                            if (SDLButton_isContain(MousePos, buttons[5])) {
                                selectedAlgorithm = FORD_FULKERSON;
                                currentPageId = 3;
                                break;
                            }
                            if (SDLButton_isContain(MousePos, buttons[6])) {
                                selectedAlgorithm = EDMONDS_KARP;
                                currentPageId = 3;
                                break;
                            }
                            if (SDLButton_isContain(MousePos, buttons[7])) {
                                selectedAlgorithm = DINIC;
                                currentPageId = 3;
                                break;
                            }
                        }

                        // select source
                        if (SDLButton_isContain(MousePos, buttons[3]) && currentPageId == buttons[3].pageId) {
                            if (isSourceSelected) {
                                ShowMessageBox("Source already selected.", window);
                                break;
                            }
                            if (n < 2) {
                                ShowMessageBox("Needed more node to select source.", window);
                                break;
                            }
                            buttons[3].bgColor = (SDL_Color){0x00, 0xFF, 0xFF, 0xFF};
                            isSelectingSource = true;
                            break;
                        }
                        if (isSelectingSource && currentPageId == 3) {
                            for (int i = 0; i < n; i++) {
                                if (SDLNode_isContain(MousePos, nodeList[i]) && currentPageId == 3) {
                                    if (i == sink) {
                                        ShowMessageBox("Source can not be the same as sink.", window);
                                        break;
                                    }
                                    source = i;
                                    nodeList[i].isSource = true;
                                    isSelectingSource = false;
                                    isSourceSelected = true;
                                    buttons[3].bgColor = (SDL_Color){0x00, 0xFF, 0x00, 0xFF};
                                    break;
                                }
                            }
                            break;
                        }

                        // select sink
                        if (SDLButton_isContain(MousePos, buttons[4]) && currentPageId == buttons[4].pageId) {
                            if (isSinkSelected) {
                                ShowMessageBox("Sink already selected.", window);
                                break;
                            }
                            if (n < 2) {
                                ShowMessageBox("Needed more node to select sink.", window);
                                break;
                            }
                            isSelectingSink = true;
                            buttons[4].bgColor = (SDL_Color){0x00, 0xFF, 0xFF, 0xFF};
                            break;
                        }
                        if (isSelectingSink && currentPageId == 3) {
                            for (int i = 0; i < n; i++) {
                                if (SDLNode_isContain(MousePos, nodeList[i]) && currentPageId == 3) {
                                    if (i == source) {
                                        ShowMessageBox("Sink can not be the same as source.", window);
                                        break;
                                    }
                                    sink = i;
                                    nodeList[i].isSink = true;
                                    isSelectingSink = false;
                                    isSinkSelected = true;
                                    buttons[4].bgColor = (SDL_Color){0x00, 0xFF, 0x00, 0xFF};
                                    break;
                                }
                            }
                            break;
                        }

                        // Run action
                        if (SDLButton_isContain(MousePos, buttons[1]) && currentPageId == 3) {
                            if (n < 2 || !isSourceSelected || !isSinkSelected) {
                                ShowMessageBox("Neither source or sink not selected or not enough nodes.", window);
                                break;
                            }
                            // Xoá kết quả cũ
                            for (int i = 0; i < n; i++)
                                for (int j = 0; j < n; j++)
                                    flow[i][j] = 0;

                            // Chạy thuật toán tương ứng
                            switch (selectedAlgorithm) {
                                case FORD_FULKERSON:
                                    max_flow = FordFulkerson(graph, source, sink, flow, n, renderer);
                                    break;
                                case EDMONDS_KARP:
                                    max_flow = EdmondsKarp(graph, source, sink, flow, n,renderer);
                                    break;
                                case DINIC:
                                    max_flow = Dinic(graph, source, sink, flow, n, renderer);
                                    break;
                                default:
                                    ShowMessageBox("No algorithm selected!", window);
                                    break;
                            }

                            printf("Max flow = %d\n", max_flow);
                            break;
                        }

                        if (currentPageId == 3 && !isSelectingSink && !isSelectingSource) {
                            if (isSinkSelected) {
                                ShowMessageBox("Node can not be added after sink selected.", window);
                                break;
                            }
                            addNode(renderer, MousePos, &n, graph);
                        }
                        break;
                    }

                    // right mouse to add connection between 2 node
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        SDL_Pos MousePos = {0, 0};
                        SDL_GetMouseState(&MousePos.x, &MousePos.y);
                        if (n < 2) {
                            ShowMessageBox("Needed more node to make connection.", window);
                        }
                        for (int i = 0; i < n; i++) {
                            if (SDLNode_isContain(MousePos, nodeList[i]) && currentPageId == 3) {
                                nodeList[i].isSelected = true;
                                isConnectingNodes = true;
                                break;
                            }
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_RIGHT && isConnectingNodes) {
                        SDL_Pos MousePos = {0, 0};
                        SDL_GetMouseState(&MousePos.x, &MousePos.y);
                        for (int i = 0; i < n; i++) {
                            if (SDLNode_isContain(MousePos, nodeList[i]) && currentPageId == 3) {
                                for (int j = 0; j < n; j++) {
                                    if (nodeList[j].isSelected) {
                                        if (i == j) {
                                            isConnectingNodes = false;
                                            nodeList[j].isSelected = false;
                                            ShowMessageBox("Can not connect node to it-self.", window);
                                            break;
                                        }
                                        int mAB;
                                        keyInputWait(renderer, event, nodeList[j].pos, nodeList[i].pos, &mAB, j, i, n, graph, flow);
                                        graph[j][i] = mAB;
                                        isConnectingNodes = false;
                                        nodeList[j].isSelected = false;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                        break;
                    }
                case SDL_MOUSEMOTION:
                    SDL_Pos MousePos = {0, 0};
                    SDL_GetMouseState(&MousePos.x, &MousePos.y);
                    for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++)
                    {
                        if (SDLButton_isContain(MousePos, buttons[i]) && currentPageId == buttons[i].pageId)
                        {
                            SDL_Cursor *cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
                            SDL_SetCursor(cursor);
                            if (i != 3 && i != 4) buttons[i].bgColor = (SDL_Color){ 0x00, 0xFF, 0xFF, 0xFF};
                            break;
                        }
                        if (i != 3 && i != 4) buttons[i].bgColor = originButtonColor;
                        SDL_Cursor *cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
                        SDL_SetCursor(cursor);
                    }
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_s) {
                        // Quay về trang 1 và reset toàn bộ
                        currentPageId = 1;
                        n = 0;
                        isSourceSelected = false;
                        isSinkSelected = false;
                        isSelectingSource = false;
                        isSelectingSink = false;
                        memset(graph, 0, sizeof(graph));
                        memset(flow, 0, sizeof(flow));
                        memset(nodeList, 0, sizeof(nodeList));
                    } else if (event.key.keysym.sym == SDLK_b && currentPageId == 3) {
                        // Quay lại trang 2, giữ nguyên dữ liệu
                        currentPageId = 2;
                    }
                    break;
                default:
                    break;
            }
        }

        SDLRenderPreset(renderer, n, graph, max_flow, flow);

    }

    // Giải phóng tài nguyên
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}


