#include "ESP.hpp"

#include "xorstr.hpp"

#include "../../Interfaces.hpp"
#include "../../SDK/GameClass/CBasePlayer.hpp"

#include "../../Hooks/FrameStageNotify/FrameStageNotifyHook.hpp"

// Settings
bool Features::Legit::Esp::enabled = false;

bool WorldToScreen(Matrix4x4& matrix, Vector worldPosition, ImVec2& screenPosition)
{
    float w = matrix[3][0] * worldPosition.x + matrix[3][1] * worldPosition.y + matrix[3][2] * worldPosition.z + matrix[3][3];
    if (w < 0.001f)
        return false;

    screenPosition = ImGui::GetIO().DisplaySize;
    screenPosition.x /= 2.0f;
	screenPosition.y /= 2.0f;
    
    screenPosition.x *= 1.0f + (matrix[0][0] * worldPosition.x + matrix[0][1] * worldPosition.y + matrix[0][2] * worldPosition.z + matrix[0][3]) / w;
    screenPosition.y *= 1.0f - (matrix[1][0] * worldPosition.x + matrix[1][1] * worldPosition.y + matrix[1][2] * worldPosition.z + matrix[1][3]) / w;
    return true;
}

void Features::Legit::Esp::ImGuiRender(ImDrawList* drawList) {
	if(!enabled)
		return;
		
	if(!Interfaces::engine->IsInGame())
		return;
		
	Matrix4x4 matrix = Hooks::FrameStageNotify::worldToScreenMatrix;

	if(!matrix.Base())
		return;
	
	int localPlayerIndex = Interfaces::engine->GetLocalPlayer();
	CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(Interfaces::entityList->GetClientEntity(localPlayerIndex));
	
	// The first object is always the WorldObj
	for(int i = 1; i < Interfaces::engine->GetMaxClients(); i++) {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(Interfaces::entityList->GetClientEntity(i));
		if(!player ||
			player == localPlayer ||
			player->GetDormant() ||
			*player->LifeState() != LIFE_ALIVE
		) continue;

		CCollideable* collideable = player->Collision();

		Vector min = *player->VecOrigin() + *collideable->ObbMins();
		Vector max = *player->VecOrigin() + *collideable->ObbMaxs();

		Vector points[] = {
			// Lower
			Vector(min.x, min.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, min.y, max.z),
			Vector(min.x, min.y, max.z),
			// Higher
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z)
		};
		
		ImVec2 topLeft = ImGui::GetIO().DisplaySize; // hacky but hey, it works
		ImVec2 bottomRight;

		for(auto point : points) {
			ImVec2 point2D;
			
			if(!WorldToScreen(matrix, point, point2D))
				continue;
				
			if(point2D.x < topLeft.x)
				topLeft.x = point2D.x;

			if(point2D.y < topLeft.y)
				topLeft.y = point2D.y;
			
			if(point2D.x > bottomRight.x)
				bottomRight.x = point2D.x;

			if(point2D.y > bottomRight.y)
				bottomRight.y = point2D.y;
		}

		drawList->AddRect(topLeft, bottomRight, ImColor(255.f, 255.f, 255.f, 255.f));
	}
}

void Features::Legit::Esp::SetupGUI() {
	ImGui::Checkbox(xorstr_("Enabled##ESP"), &enabled);
}
