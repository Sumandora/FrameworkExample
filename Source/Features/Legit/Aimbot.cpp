#include "Aimbot.hpp"

#include "../../Interfaces.hpp"
#include "../../SDK/GameClass/CBasePlayer.hpp"

#include "imgui.h"

#include "../../Utils/Recoil.hpp"

#include <algorithm>
#include <math.h>

#define DEG2RAD(deg) (deg * M_PI / 180.0)
#define RAD2DEG(rad) (rad * 180.0 / M_PI)

// Settings
bool	Features::Legit::Aimbot::enabled	= false;
float	Features::Legit::Aimbot::fov		= 10.0f;
float	Features::Legit::Aimbot::smoothness	= 4.0f;
int		Features::Legit::Aimbot::clamp		= 1;

// Thanks 2 Mathlib (https://github.com/SwagSoftware/Kisak-Strike/blob/7df358a4599ba02a4e072f8167a65007c9d8d89c/mathlib/mathlib_base.cpp#L1108)
void VectorAngles( const Vector& forward, Vector &angles )
{
	float	tmp, yaw, pitch;
	
	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrt (forward[0]*forward[0] + forward[1]*forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}
	
	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

Vector CalculateView(Vector a, Vector b) {
	Vector delta = b - a;
	Vector rotation = {};
	VectorAngles(delta, rotation);
	return rotation;
}

void Features::Legit::Aimbot::PollEvent(SDL_Event* event) {
	if(!enabled)
		return;
	
	if(event->type != SDL_MOUSEMOTION)
		return;
	
	if(!Interfaces::engine->IsInGame())
		return;
	
	int localPlayerIndex = Interfaces::engine->GetLocalPlayer();
	CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(Interfaces::entityList->GetClientEntity(localPlayerIndex));

	if(!localPlayer)
		return;

	TeamID localTeam = *localPlayer->Team();
	if(
		localTeam == TeamID::TEAM_UNASSIGNED ||
		localTeam == TeamID::TEAM_SPECTATOR
	) return;

	Vector playerEye = localPlayer->GetEyePosition();
	
	Vector viewAngles;
	Interfaces::engine->GetViewAngles(&viewAngles);
	
	CBasePlayer* target = nullptr;
	float bestDistance;
	Vector bestRotation;
	
	// The first object is always the WorldObj
	for(int i = 1; i < Interfaces::engine->GetMaxClients(); i++) {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(Interfaces::entityList->GetClientEntity(i));
		if(!player ||
			player == localPlayer ||
			player->GetDormant() ||
			*player->LifeState() != LIFE_ALIVE ||
			*player->GunGameImmunity() ||
			*player->Team() == localTeam ||
			*player->SpottedByMask() & (1 << localPlayerIndex)
		) continue;

		Vector rotation = CalculateView(playerEye, player->GetBonePosition(8));
		
		Vector recoil = Utils::CalcRecoilKickBack(localPlayer) * 2;
		rotation -= recoil;
		
		rotation -= viewAngles;
		
		rotation = rotation.Wrap();
		
		float delta = rotation.Length();
		if(!target || bestDistance > delta) {
			target = player;
			bestDistance = delta;
			bestRotation = rotation;
		}
	}

	if(!target)
		return;

	if(bestRotation.Length() > fov)
		return;

	bestRotation /= smoothness;

	Vector before	= Vector(event->motion.xrel, event->motion.yrel, 0);
	Vector goal		= Vector(-round(bestRotation.y), round(bestRotation.x), 0);

	float dir = before.Normalize().Dot(goal.Normalize());
	if(dir < 0)
		return; // We are trying to aim away

	event->motion.xrel += std::clamp(static_cast<int>(goal.x), -clamp, clamp);
	event->motion.yrel += std::clamp(static_cast<int>(goal.y), -clamp, clamp);
}

void Features::Legit::Aimbot::SetupGUI() {
	ImGui::Checkbox(xorstr_("Enabled##Aimbot"), &enabled);
	ImGui::SliderFloat(xorstr_("FOV##Aimbot"), &fov, 0.0f, 45.0f, "%.2f");
	ImGui::SliderFloat(xorstr_("Smoothness##Aimbot"), &smoothness, 1.0f, 5.0f, "%f");
	ImGui::SliderInt(xorstr_("Clamp##Aimbot"), &clamp, 1, 5, "%d");
}
