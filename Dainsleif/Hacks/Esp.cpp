#include "Esp.h"
#include <optional>

std::optional<Vector2> WorldToScreen(Vector3 entPos, WindowSize& windowSize) {
    float viewMatrix[4][4];
    memcpy(&viewMatrix, (PBYTE*)(Modules::client + dwViewMatrix), sizeof(viewMatrix));
    Vector4 clipCoords{};
    clipCoords.x = entPos.x * viewMatrix[0][0] + entPos.y * viewMatrix[0][1] + entPos.z * viewMatrix[0][2] + viewMatrix[0][3];
    clipCoords.y = entPos.x * viewMatrix[1][0] + entPos.y * viewMatrix[1][1] + entPos.z * viewMatrix[1][2] + viewMatrix[1][3];
    clipCoords.z = entPos.x * viewMatrix[2][0] + entPos.y * viewMatrix[2][1] + entPos.z * viewMatrix[2][2] + viewMatrix[2][3];
    clipCoords.w = entPos.x * viewMatrix[3][0] + entPos.y * viewMatrix[3][1] + entPos.z * viewMatrix[3][2] + viewMatrix[3][3];

    if (clipCoords.w < 0.1f)
        return {};

    Vector3 NDC{};
    NDC.x = clipCoords.x / clipCoords.w;
    NDC.y = clipCoords.y / clipCoords.w;
    NDC.z = clipCoords.z / clipCoords.w;
    std::optional<Vector2> screen = Vector2{};
    screen->x = (windowSize.w / 2 * NDC.x) + (NDC.x + windowSize.w / 2);
    screen->y = -(windowSize.h / 2 * NDC.y) + (NDC.y + windowSize.h / 2);
    return screen;
}

void DrawLine(IDirect3DDevice9& pDevice, int x1, int y1, int x2, int y2, int thickness, D3DCOLOR color) {
    ID3DXLine* LineL;
    D3DXCreateLine(&pDevice, &LineL);

    D3DXVECTOR2 Line[2];
    Line[0] = D3DXVECTOR2(x1, y1);
    Line[1] = D3DXVECTOR2(x2, y2);
    LineL->SetWidth(thickness);
    LineL->Draw(Line, 2, color);
    LineL->Release();
}

void DrawOutLineRect(IDirect3DDevice9& pDevice, Vector2 top, Vector2 bottom, int thickness, D3DCOLOR color) {
    ID3DXLine* LineL;
    D3DXCreateLine(&pDevice, &LineL);

    float height = top.y - bottom.y;

    // apparently, height / 2 is the horizontal length of the rectangle.
    // top.x and bottom.x is located at the center of the entity
    // so x coordinate -(+) height / 4 should be the left and right end of the rectangle.
    D3DXVECTOR2 topLine[2] = {D3DXVECTOR2(top.x - height / 4, top.y), D3DXVECTOR2(top.x + height / 4, top.y)};
    D3DXVECTOR2 bottomLine[2] = {D3DXVECTOR2(bottom.x - height / 4, bottom.y), D3DXVECTOR2(bottom.x + height / 4, bottom.y)};
    D3DXVECTOR2 rightLine[2] = {D3DXVECTOR2(top.x + height / 4, top.y), D3DXVECTOR2(bottom.x + height / 4, bottom.y)};
    D3DXVECTOR2 leftLine[2] = {D3DXVECTOR2(top.x - height / 4, top.y), D3DXVECTOR2(bottom.x - height / 4, bottom.y)};

    LineL->SetWidth(thickness);
    LineL->Draw(topLine, 2, color);
    LineL->Draw(bottomLine, 2, color);
    LineL->Draw(rightLine, 2, color);
    LineL->Draw(leftLine, 2, color);
    LineL->Release();
}

void Esp::Run(IDirect3DDevice9& pDevice, WindowSize windowSize) {
    Player* localPlayer = Player::GetLocalPlayer();
    std::vector<Player*> playerList = Player::GetAll();
    for (auto& player : playerList) {
        if (player->GetHealth() && !player->IsDormant()) {
            std::optional<Vector2> entPos2D = WorldToScreen(player->GetBodyPosition(), windowSize);
            std::optional<Vector2> entHeadPos2D = WorldToScreen(player->GetBonePosition(), windowSize);
            if (entPos2D && entHeadPos2D) {
                D3DCOLOR color;
                if (player->GetTeam() == localPlayer->GetTeam())
                    color = D3DCOLOR_ARGB(255, 0, 255, 0);
                else
                    color = D3DCOLOR_ARGB(255, 255, 0, 0);
                DrawLine(pDevice, entPos2D->x, entPos2D->y, windowSize.w / 2, windowSize.h, 2, color);
                DrawOutLineRect(pDevice, *entHeadPos2D, *entPos2D, 1, color);
            }
        }
    }
}