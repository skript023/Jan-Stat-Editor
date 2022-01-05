#pragma once
#include "common.hpp"

namespace big::features
{
	extern bool PlayerGodmode;
	extern bool InfiniteClip;
	extern bool VehicleGodmode;
	extern bool NoIdleKickBool;
	extern bool AutoHealBool;
	extern bool NeverWantedBool;
	extern bool ExplosiveAmmoBool;
	extern bool FlamingAmmo;
	extern bool SuperJump;
	extern bool ExplosiveFist;
	extern bool NoRagdoll;
	extern bool CrewHeistBool;
	extern bool AutomaticHeistCut;
	extern bool PlayerSeatBelt;
	extern bool PlayerRegen;
	extern bool PlayerTembus;
	extern bool InfiniteAmmoToggle;
	extern bool AllMissionLLivesToggle;
	extern bool RemoveCrew;
	extern bool send_heal;
	extern bool AllTakeHeist;
	extern bool test_frame;
	extern bool RemoveObjects;
	extern const char* PlayerNames[32];
	extern int LevelData(int Level);

	extern void SetVehicleAcceleration(float Activation);
	extern void SetPlayerWaterProof(bool Activation);
	extern void RevealRadar(bool Activation);
	extern void BlindCops(bool Activation);
	extern void GhostOrganization(bool Activation);
	extern void OffTheRadar(bool Activation);

	extern void HeistTakeAll(int Take);

	extern void SetVehicleGodmode(bool Activation);
	extern void SetPlayerRagdoll(int Ragdoll);
	extern void SetPlayerArmour(float armour);

	extern void SessionChanger(int SessionId);

	extern float GetPlayerMaxHealth();
	extern float GetPlayerHealth();
	extern void AutoHeal(bool Activation);
	extern void SetPlayerHealth(float health);
	extern void SetWantedLevel(int Number);

	extern void BigconHeist();
	extern void AggressiveHeist();
	extern void SilentHeist();
	extern void CayoPericoHeist();

	extern void run_tick();
	extern void script_func();
}
