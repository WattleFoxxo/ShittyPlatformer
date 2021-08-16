//#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include "json.hpp"
//#include "funcs.hpp"
#include "raylib.h"
#include "raymath.h"
#include "tweeny.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

using json = nlohmann::json;

bool DEBUG = false;

class Colliderdata {
    public:
        std::vector<Rectangle> colliders;
        std::vector<Rectangle> dangercolliders;
        std::vector<Rectangle> bouncecolliders;
        std::vector<Rectangle> disappearingcolliders;
        std::vector<Rectangle> endcolliders;
        std::vector<int> dptimer;
        Vector2 spawnpint;
};

json loadlevel(std::string level, bool external);
Colliderdata drawlevel(json worlddata, std::vector<Texture> textures, Vector2 pos, float frame);
std::vector<std::string> getlevels();

bool isCollideing(Rectangle body, std::vector<Rectangle> colliders){
    for(int i = 0; i < colliders.size(); i++){
        if(CheckCollisionRecs(body, colliders[i])){
            return true;
        }
    }
    return false;
}

int isCollideingIndex(Rectangle body, std::vector<Rectangle> colliders){
    for(int i = 0; i < colliders.size(); i++){
        if(CheckCollisionRecs(body, colliders[i])){
            return i;
        }
    }
    return -1;
}

int main(void) {
    //init
    int screenWidth = 1280;
    int screenHeight = 720;
    //const int screenWidth = 1920;
    //const int screenHeight = 1080;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "shittyplatfomer");

    Rectangle player = {200,1600,50,50};

    Colliderdata colliderdata;
    std::vector<Rectangle> colliders;
    std::vector<Rectangle> dangercolliders;
    std::vector<Rectangle> bouncecolliders;
    std::vector<Rectangle> disappearingcolliders;

    Vector2 velocity = {0, 0};
    Vector2 mousepos = {0, 0};
    Vector2 spawnpint = {0, 0};
    Vector2 menuscroll1 = {0, 0};

    bool isGrounded = false;
    bool dead = false;
    bool end = false;
    bool fullscreen = false;
    bool selectlevel = true;
    bool external = false;

    json level;

    int state = 2;
    int framesCounter = 0;
    int screentint = 0;
    int levelWidth = 0;
    int levelHeight = 0;

    float currentFrame = 0;
    float deltatime = 0;
    float timer = 0;
    float levelWidthFull = 0;
    float levelHeightFull = 0;

    std::string currentlevel = "level";

    Camera2D camera = { 0 };
    camera.target = {player.x, player.y};
    camera.offset = { screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    //camera.zoom = 1.5f;

    std::vector<Texture> textures;

    for(int i = 0; i < 25; i++){
        textures.push_back(LoadTexture(("resources/tiles/tile" + std::to_string(i) + ".png").c_str()));
    }

    textures.push_back(LoadTexture("resources/tiles/end.png"));
    textures.push_back(LoadTexture("resources/tiles/hologram.png"));

    Texture2D button = LoadTexture("resources/gui/button3.png");

    Font SourceSansPro = LoadFont("resources/gui/SourceSansPro-Regular.ttf");
    Font SourceSansPro_lage = LoadFontEx("resources/gui/SourceSansPro.ttf", 128, 0, 925);
    Font SourceSansPro_smol = LoadFontEx("resources/gui/SourceSansPro-Regular.ttf", 18, 0, 925);

    GuiLoadStyle("resources/gui/default.rgs");

    auto deathtweenstep = [&](int i){
        screentint = i;
        if(i == 255){
            player.x = spawnpint.x;
            player.y = spawnpint.y;
        }
        if(i == 0){
            dead = false;
            return true;
        }
        return false;
    };

    auto endtweenstep = [&](int i){
        screentint = i;
        if(i == 255){
            state = 2;
        }
        if(i == 0){
            end = false;
            return true;
        }
        return false;
    };

    auto deathtween = tweeny::from(1).to(255).during(1000).to(0).during(1000).onStep(deathtweenstep);
    auto endtween = tweeny::from(1).to(255).during(1000).to(0).during(1000).onStep(endtweenstep);

    std::vector<std::string> levels = getlevels();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        deltatime = GetFrameTime();

        screenWidth = GetScreenWidth(); // Get current screen width
        screenHeight = GetScreenHeight();  
        camera.zoom = screenHeight/720.0f; 

        if (IsKeyPressed(KEY_F1)){
            DEBUG = !DEBUG;
        }

        if (IsKeyPressed(KEY_J)){
            state = 2;
        }

        if (IsKeyPressed(KEY_R)){

        }

        if(dead){
            deathtween.step(deltatime);
        } else {
            deathtween = tweeny::from(1).to(255).during(1000).to(0).during(1000).onStep(deathtweenstep);
        }

        if(end){
            endtween.step(deltatime);
        } else {
            endtween = tweeny::from(1).to(255).during(1000).to(0).during(1000).onStep(endtweenstep);
        }

        if(state == 0){
            level = loadlevel(currentlevel, external);
            colliderdata = drawlevel(level, textures, {0, 0}, 0);
            player.x = colliderdata.spawnpint.x;
            player.y = colliderdata.spawnpint.y;
            levelWidth = (int)level["width"];
            levelHeight = (int)level["height"];
            levelWidthFull = ((int)level["width"])*(1024-(1024*(1.0-0.1)));
            levelHeightFull = ((int)level["height"])*(1024-(1024*(1.0-0.1)));
            camera.target = {player.x, player.y};
            state = 1;
        } else if(state == 1){

            framesCounter++;
            timer+= 1*deltatime;
            if(framesCounter > 1){
                currentFrame -= 0.02;
                if(currentFrame < -1){
                    currentFrame = 0;
                }
                framesCounter = 0;
            }
            //--update--//
            if (IsKeyDown(KEY_A)){
                velocity.x += 180.0f*deltatime;
            }
            
            if (IsKeyDown(KEY_D)){
                velocity.x -= 180.0f*deltatime;
            }

            if (IsKeyDown(KEY_SPACE) && isGrounded){
                velocity.y = 14.0f;//*deltatime*92;
                isGrounded = false;
            }

            if(!isGrounded){
                velocity.y -= 36.0f*deltatime;
            } else {
                velocity.y = 0;
            }

            velocity.x /= 1.3f;//*deltatime;

            if(!dead) player.x -= velocity.x;
            for(int i = 0; i < colliderdata.colliders.size(); i++){
                if(CheckCollisionRecs(player, colliderdata.colliders[i])){
                    if(velocity.x < 0){
                        player.x = colliderdata.colliders[i].x-player.width;
                    } else if(velocity.x > 0){
                        player.x = colliderdata.colliders[i].x+colliderdata.colliders[i].width+0.1;
                    }
                }
            }

            isGrounded = false;
            player.y -= velocity.y;
            for(int i = 0; i < colliderdata.colliders.size(); i++){
                if(CheckCollisionRecs(player, colliderdata.colliders[i])){
                    if(velocity.y < 0){
                        player.y = colliderdata.colliders[i].y-player.height;
                        isGrounded = true;
                    } else if(velocity.y > 0){
                        player.y = colliderdata.colliders[i].y+colliderdata.colliders[i].height+0.1;
                    }
                    velocity.y = 0;
                }
            }

            camera.offset = { (float)screenWidth/2-(25*camera.zoom), (float)screenHeight/2-(25*camera.zoom) };
            camera.target = { Clamp(Lerp(player.x, camera.target.x, 0.8f), (screenWidth/camera.zoom)/2-(25*camera.zoom), levelWidthFull-(screenWidth/camera.zoom)/2-(25*camera.zoom)), Clamp(Lerp(player.y, camera.target.y, 0.8f), (screenHeight/camera.zoom)/2-(25*camera.zoom), levelHeightFull-(screenHeight/camera.zoom)/2-(25*camera.zoom))};

            //--logic--//
            if(isCollideing(player, colliderdata.dangercolliders)){
                dead = true;
            }

            if(isCollideing(player, colliderdata.bouncecolliders)){
                velocity.y = 18.0f;
            }

            if(isCollideing(player, colliderdata.endcolliders)){
                end = true;
            }

            BeginDrawing();

                ClearBackground({57, 58, 86, 255});

                BeginMode2D(camera);

                    //playersprite.x = lerp(player.x, playersprite.x, 0.4f);
                    //playersprite.y = lerp(player.y, playersprite.y, 0.4f);



                    DrawRectangleRec(player, RAYWHITE);
                    DrawRectangleRec({player.x+3, player.y+3, player.width-6, player.height-6}, {20, 16, 32, 255});
                    
                    colliderdata = drawlevel(level, textures, {0, 0}, currentFrame);

                    spawnpint = colliderdata.spawnpint;

                    if(DEBUG){
                        for(int i = 0; i < colliderdata.colliders.size(); i++){
                            DrawRectangleLinesEx(colliderdata.colliders[i], 1, {20,20,255,255});
                        }

                        for(int i = 0; i < colliderdata.dangercolliders.size(); i++){
                            DrawRectangleLinesEx(colliderdata.dangercolliders[i], 1, {255,20,20,255});
                        }

                        for(int i = 0; i < colliderdata.bouncecolliders.size(); i++){
                            DrawRectangleLinesEx(colliderdata.bouncecolliders[i], 1, {255,255,20,255});
                        }

                        DrawRectangleLinesEx(player, 1, {20,255,20,255});
                    }

                EndMode2D();

                //DrawTextEx(nunito_lage, std::to_string(timer).c_str(), {(screenWidth/2) - (MeasureTextEx(nunito_lage, std::to_string(timer).c_str(), 69, 1).x/2),0}, 69, 1, BLACK);
                
                DrawRectangleRec({0, 0, (float)screenWidth, (float)screenHeight}, {0,0,0,(unsigned char)screentint});

                if(DEBUG){
                    DrawTextEx(SourceSansPro, ("FPS: "+std::to_string(GetFPS())).c_str(), {5,0}, 32, 1, BLACK);
                    DrawTextEx(SourceSansPro, ("X: "+std::to_string((int)std::round(player.x))+" Y: "+std::to_string((int)std::round(player.y))).c_str(), {5,32}, 32, 1, BLACK);
                }

            EndDrawing();
        } else if(state == 2) {
            mousepos = GetMousePosition();
            BeginDrawing();

                ClearBackground({57, 58, 86, 255});
                GuiSetFont(SourceSansPro);

                DrawTextEx(SourceSansPro_lage, "shitty platfomer", {(screenWidth/2) - (MeasureTextEx(SourceSansPro_lage, "shitty platfomer", 128, 1).x/2),0}, 128, 1, RAYWHITE);
                
                if (GuiButton({(float)screenWidth/2-300, (float)screenHeight/2-75, 600, 100}, "Play!")) {
                    state = 3;
                }

                if(GuiButton({(float)screenWidth/2-300, (float)screenHeight/2+31, 297, 100}, "exit")) {
                    CloseWindow();
                }

                GuiLock();
                GuiSetState(GUI_STATE_DISABLED);
                if(GuiButton({((float)screenWidth/2-300)+303, (float)screenHeight/2+31, 297, 100}, "options")) {
                }
                GuiSetState(GUI_STATE_NORMAL);
                GuiUnlock();

                if(DEBUG){
                    DrawTextEx(SourceSansPro, ("FPS: "+std::to_string(GetFPS())).c_str(), {5,0}, 32, 1, BLACK);
                }

            EndDrawing();
        } else if(state == 3){
             mousepos = GetMousePosition();
            BeginDrawing();

                ClearBackground({57, 58, 86, 255});
                GuiSetFont(SourceSansPro);

                DrawTextEx(SourceSansPro_lage, "shitty platfomer", {(screenWidth/2) - (MeasureTextEx(SourceSansPro_lage, "shitty platfomer", 128, 1).x/2),0}, 128, 1, RAYWHITE);
                
                if (GuiButton({10, 10, 60, 60}, "back")) {
                    state = 2;
                }
                
                int buttons = 20;

                //Rectangle view = GuiScrollPanel({(float)screenWidth/2-505, (float)screenHeight/2-100-5, 1000, 120}, {0,0,buttons*105.0f+5, 100}, &menuscroll1);

                int y = 0;
                int x = 0;
                DrawTextEx(SourceSansPro, "built-in levels", {10,(float)screenHeight/2-132}, 32, 1, RAYWHITE);
                for(int i = 0; i < 1; i++){
                    if(x+110 > screenWidth){
                        y-=105;
                        x=0;
                    }
                    if(GuiButton({(float)10+x, (float)screenHeight/2-100-y, 100, 100}, "demo")) {
                        external = false;
                        currentlevel = "demo";
                        state = 0;
                    }
                    x+=105;
                }
                y-=142;
                x=0;
                
                levels = getlevels();
                DrawTextEx(SourceSansPro, "custom levels", {10,(float)screenHeight/2-132-y}, 32, 1, RAYWHITE);
                for(int i = 0; i < levels.size(); i++){
                    if(x+110 > screenWidth){
                        y-=105;
                        x=0;
                    }
                    GuiSetFont(SourceSansPro_smol);
                    if(GuiButton({(float)10+x, (float)screenHeight/2-100-y, 100, 100}, GetFileNameWithoutExt(levels[i].c_str()))) {
                        external = true;
                        currentlevel = GetFileNameWithoutExt(levels[i].c_str());
                        state = 0;
                    }
                    GuiSetFont(SourceSansPro);
                    x+=105;
                }
                //EndScissorMode();

                if(DEBUG){
                    DrawTextEx(SourceSansPro, ("FPS: "+std::to_string(GetFPS())).c_str(), {5,0}, 32, 1, BLACK);
                }

            EndDrawing();
        }
    }
    CloseWindow();

    return 0;
}

json loadlevel(std::string level, bool external){
    char* rawdata;
    if(external){
        rawdata = LoadFileText(("levels/"+level+".json").c_str());
    } else {
        rawdata = LoadFileText(("resources/levels/"+level+".json").c_str());
    }
    auto worlddata = json::parse(rawdata);
    return worlddata;
}

Colliderdata drawlevel(json worlddata, std::vector<Texture> textures, Vector2 pos, float frame){
    Colliderdata colliderdata;
    float scale = 0.1;
    for(int i = 0; i < (int)worlddata["layers"].size(); i++){
        for(int x = 0; x < (int)worlddata["width"]; x++){
            for(int y = 0; y < (int)worlddata["height"]; y++){
                int tileid = (int)worlddata["layers"][i]["data"][y*20+x];
                if(tileid != 0){
                    if(tileid == 3 || tileid == 13){
                        DrawTextureQuad(textures[tileid-1], {1,1}, {(float)frame,0}, {x*(1024*scale)+pos.x, y*(1024*scale)+pos.y, 1024*scale, 1024*scale}, WHITE); 
                    } else if (tileid == 25) {
                        colliderdata.spawnpint.x = (x*(1024*scale)+pos.x/2)+25;
                        colliderdata.spawnpint.y = (y*(1024*scale)+pos.y/2)+0.1;
                        DrawTextureQuad(textures[26], {1,1}, {0,(float)frame*-2}, {x*(1024*scale)+pos.x, y*(1024*scale)+pos.y, (1024*scale), (1024*scale)}, WHITE);
                        DrawTextureEx(textures[25], {x*(1024*scale)+pos.x, y*(1024*scale)+pos.y}, 0, scale, WHITE);
                    } else if (tileid == 24) {
                        DrawTextureQuad(textures[26], {1,1}, {0,(float)frame*2}, {x*(1024*scale)+pos.x, y*(1024*scale)+pos.y, (1024*scale), (1024*scale)}, WHITE);
                        DrawTextureEx(textures[25], {x*(1024*scale)+pos.x, y*(1024*scale)+pos.y}, 0, scale, WHITE);
                    } else {
                        DrawTextureEx(textures[tileid-1], {x*(1024*scale)+pos.x, y*(1024*scale)+pos.y}, 0, scale, WHITE);
                    }
                    for(int j = 0; j < worlddata["tilesets"][0]["tiles"].size(); j++){
                        if((int)worlddata["tilesets"][0]["tiles"][j]["id"] == tileid-1){
                            for(int e = 0; e < worlddata["tilesets"][0]["tiles"][j]["objectgroup"]["objects"].size(); e++){
                                json collider = worlddata["tilesets"][0]["tiles"][j]["objectgroup"]["objects"][e];
                                float xpos = (float)collider["x"]-((float)collider["x"]*(1.0-scale))+x*(1024*scale)+pos.x;
                                float ypos = (float)collider["y"]-((float)collider["y"]*(1.0-scale))+y*(1024*scale)+pos.y;
                                float width = (float)((float)collider["width"]-((float)collider["width"]*(1.0-scale)));
                                float height = (float)((float)collider["height"]-((float)collider["height"]*(1.0-scale)));
                                if(tileid == 3 || tileid == 4 || tileid == 5 || tileid == 8 || tileid == 13 || tileid == 18){
                                    colliderdata.dangercolliders.push_back({xpos, ypos, width, height});
                                } else if(tileid == 9){
                                    colliderdata.bouncecolliders.push_back({xpos, ypos, width, height});
                                } else if(tileid == 24){
                                    colliderdata.colliders.push_back({xpos, ypos, width, height});
                                    colliderdata.endcolliders.push_back({xpos, ypos-0.1f, width, height});
                                } else {
                                    colliderdata.colliders.push_back({xpos, ypos, width, height});
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    return colliderdata;
}

std::vector<std::string> getlevels(){
    std::vector<std::string> levelparths;
    int fileCount;
    int count = 0;
    char **levels = GetDirectoryFiles("levels/",&fileCount);
    for(int i = 0 ; i < fileCount; i ++){
        //char *file = levels[i];
        if(IsFileExtension(levels[i], ".json")){
            levelparths.push_back(levels[i]);
        }
    }
    return levelparths;
}