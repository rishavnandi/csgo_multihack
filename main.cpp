#include "memory.h"
#include "vector.h"

#include <thread>
#include <array>
#include <iostream>

namespace offset
{
	// client
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDEA98C;
	constexpr ::std::ptrdiff_t dwEntityList = 0x4DFFF7C;
	constexpr ::std::ptrdiff_t dwGlowObjectManager = 0x535AA08;
	constexpr ::std::ptrdiff_t dwForceAttack = 0x322DDE8;

	// player
	constexpr ::std::ptrdiff_t m_hMyWeapons = 0x2E08;

	// base attributable
	constexpr ::std::ptrdiff_t m_flFallbackWear = 0x31E0;
	constexpr ::std::ptrdiff_t m_nFallbackPaintKit = 0x31D8;
	constexpr ::std::ptrdiff_t m_nFallbackSeed = 0x31DC;
	constexpr ::std::ptrdiff_t m_nFallbackStatTrak = 0x31E4;
	constexpr ::std::ptrdiff_t m_iItemDefinitionIndex = 0x2FBA;
	constexpr ::std::ptrdiff_t m_iItemIDHigh = 0x2FD0;
	constexpr ::std::ptrdiff_t m_iEntityQuality = 0x2FBC;
	constexpr ::std::ptrdiff_t m_iAccountID = 0x2FD8;
	constexpr ::std::ptrdiff_t m_OriginalOwnerXuidLow = 0x31D0;

	// engine
	constexpr ::std::ptrdiff_t dwClientState = 0x59F19C;
	constexpr ::std::ptrdiff_t dwClientState_ViewAngles = 0x4D90;

	// entity
	constexpr ::std::ptrdiff_t m_dwBoneMatrix = 0x26A8;
	constexpr ::std::ptrdiff_t m_bDormant = 0xED;
	constexpr ::std::ptrdiff_t m_iTeamNum = 0xF4;
	constexpr ::std::ptrdiff_t m_iHealth = 0x100;
	constexpr ::std::ptrdiff_t m_vecOrigin = 0x138;
	constexpr ::std::ptrdiff_t m_vecViewOffset = 0x108;
	constexpr ::std::ptrdiff_t m_aimPunchAngle = 0x303C;
	constexpr ::std::ptrdiff_t m_bSpottedByMask = 0x980;
	constexpr ::std::ptrdiff_t m_iGlowIndex = 0x10488;
	constexpr ::std::ptrdiff_t m_iShotsFired = 0x103E0;
	constexpr ::std::ptrdiff_t m_iCrosshairId = 0x11838;
}

constexpr Vector3 CalculateAngle(
	const Vector3& localPosition,
	const Vector3& enemyPosition,
	const Vector3& viewAngles) noexcept
{
	return ((enemyPosition - localPosition).ToAngle() - viewAngles);
}

struct Color
{
	constexpr Color(float r, float g, float b, float a = 1.f) noexcept :
		r(r), g(g), b(b), a(a) { }

	float r, g, b, a;
};

struct Vector2
{
	float x = { }, y = {};
};

// set skins to apply here
constexpr const int GetWeaponPaint(const short& itemDefinition)
{
	switch (itemDefinition)
	{
	case 1: return 764; // deagle
	case 4: return 10063; // glock
	case 61: return 504; // usps
	case 7: return 1141; // ak47
	case 8: return 455; // aug
	case 39: return 750; // sg553
	case 9: return 344; // awp
	case 14: return 1148; // m249
	case 16: return 1001; // m4a1
	case 60: return 1001; // m4a1s
	case 507: return 409; // karambit
	default: return 0;
	}
}

int main()
{
	// initialize memory class
	const auto memory = Memory("csgo.exe");

	// module addresses
	const auto client = memory.GetModuleAddress("client.dll");
	const auto engine = memory.GetModuleAddress("engine.dll");

	std::cout << std::hex << "client.dll -> 0x" << client << std::dec << std::endl;

	const auto color = Color(1.f, 0.f, 0.f);

	auto oldPunch = Vector2{ };

	// infinite hack loop
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		// aimbot key
		if (GetAsyncKeyState(VK_SHIFT))
			continue;

		// get local player
		const auto localPlayer = memory.Read<std::uintptr_t>(client + offset::dwLocalPlayer);
		const auto localTeam = memory.Read<std::int32_t>(localPlayer + offset::m_iTeamNum);

		// eye position = origin + viewOffset
		const auto localEyePosition = memory.Read<Vector3>(localPlayer + offset::m_vecOrigin) + 
			memory.Read<Vector3>(localPlayer + offset::m_vecViewOffset);

		const auto& clientState = memory.Read<std::uintptr_t>(engine + offset::dwClientState);

		const auto& viewAngles = memory.Read<Vector3>(clientState + offset::dwClientState_ViewAngles);
		const auto& aimPunch = memory.Read<Vector3>(localPlayer + offset::m_aimPunchAngle) * 2;

		// aimbot fov
		auto bestFov = 50.f;
		auto bestAngle = Vector3{ };

		for (auto i = 1; i <= 32; ++i)
		{
			const auto& player = memory.Read<std::uintptr_t>(client + offset::dwEntityList + i * 0x10);

			// entity checks
			if (memory.Read<std::int32_t>(player + offset::m_iTeamNum) == localTeam)
				continue;

			if (memory.Read<bool>(player + offset::m_bDormant))
				continue;

			if (!memory.Read<std::int32_t>(player + offset::m_iHealth))
				continue;

			if (!memory.Read<bool>(player + offset::m_bSpottedByMask))
				continue;

			const auto boneMatrix = memory.Read<std::uintptr_t>(player + offset::m_dwBoneMatrix);

			const auto playerHeadPosition = Vector3{
				memory.Read<float>(boneMatrix + 0x30 * 8 + 0x0C),
				memory.Read<float>(boneMatrix + 0x30 * 8 + 0x1C),
				memory.Read<float>(boneMatrix + 0x30 * 8 + 0x2C)
			};

			const auto angle = CalculateAngle(
				localEyePosition,
				playerHeadPosition,
				viewAngles + aimPunch
			);

			const auto fov = std::hypotf(angle.x, angle.y);

			if (fov < bestFov)
			{
				bestFov = fov;
				bestAngle = angle;
			}
		}

		// if we have a best angle
		// do aimbot
		if (!bestAngle.IsZero())
			memory.Write<Vector3>(clientState + offset::dwClientState_ViewAngles, viewAngles + bestAngle);

		// glow
		const auto glowObjectManager = memory.Read<std::uintptr_t>(client + offset::dwGlowObjectManager);

		for (auto i = 0; i < 64; ++i)
		{
			const auto entity = memory.Read<std::uintptr_t>(client + offset::dwEntityList + i * 0x10);

			if (memory.Read<std::uintptr_t>(entity + offset::m_iTeamNum) == memory.Read<std::uintptr_t>(localPlayer + offset::m_iTeamNum))
				continue;

			const auto glowIndex = memory.Read<std::int32_t>(entity + offset::m_iGlowIndex);

			memory.Write<Color>(glowObjectManager + (glowIndex * 0x38) + 0x8, color);

			memory.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x27, true);
			memory.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x28, true);
		}

		// recoil control system
		const auto& shotsFired = memory.Read<std::int32_t>(localPlayer + offset::m_iShotsFired);

		if (shotsFired)
		{
			const auto& clientState = memory.Read<std::uintptr_t>(engine + offset::dwClientState);
			const auto& viewAngles = memory.Read<Vector2>(clientState + offset::dwClientState_ViewAngles);

			const auto& aimPunch = memory.Read<Vector2>(localPlayer + offset::m_aimPunchAngle);

			auto newAngles = Vector2
			{
				viewAngles.x + oldPunch.x - aimPunch.x * 2.f,
				viewAngles.y + oldPunch.y - aimPunch.y * 2.f
			};

			if (newAngles.x > 89.f)
				newAngles.x = 89.f;

			if (newAngles.x < -89.f)
				newAngles.x = -89.f;

			while (newAngles.y > 180.f)
				newAngles.y -= 360.f;

			while (newAngles.y < -180.f)
				newAngles.y += 360.f;

			memory.Write<Vector2>(clientState + offset::dwClientState_ViewAngles, newAngles);

			oldPunch.x = aimPunch.x * 2.f;
			oldPunch.y = aimPunch.y * 2.f;
		}
		else
		{
			oldPunch.x = oldPunch.y = 0.f;
		}

		// triggerbot
		const auto& localHealth = memory.Read<std::int32_t>(localPlayer + offset::m_iHealth);

		// skip if local player is dead
		if (!localHealth)
			continue;

		const auto& crosshairId = memory.Read<std::int32_t>(localPlayer + offset::m_iCrosshairId);

		if (!crosshairId || crosshairId > 64)
			continue;

		const auto& player = memory.Read<std::uintptr_t>(client + offset::dwEntityList + (crosshairId - 1) * 0x10);

		// skip if player is dead
		if (!memory.Read<std::int32_t>(player + offset::m_iHealth))
			continue;

		// skip if player is on our team
		if (memory.Read<std::int32_t>(player + offset::m_iTeamNum) ==
			memory.Read<std::int32_t>(localPlayer + offset::m_iTeamNum))
			continue;

		memory.Write<std::uintptr_t>(client + offset::dwForceAttack, 6);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		memory.Write<std::uintptr_t>(client + offset::dwForceAttack, 4);

		//// skin changer
		//const auto& weapons = memory.Read<std::array<unsigned long, 8>>(localPlayer + offset::m_hMyWeapons);
		//
		//// local player weapon iteration
		//for (const auto& handle : weapons)
		//{
		//	const auto& weapon = memory.Read<std::uintptr_t>((client + offset::dwEntityList + (handle & 0xFFF) * 0x10) - 0x10);

		//	// make sure weapon is valid
		//	if (!weapon)
		//		continue;

		//	// see if we want to apply a skin
		//	if (const auto paint = GetWeaponPaint(memory.Read<short>(weapon + offset::m_iItemDefinitionIndex)))
		//	{
		//		const bool shouldUpdate = memory.Read<std::int32_t>(weapon + offset::m_nFallbackPaintKit) != paint;

		//		// force weapon to use fallback values
		//		memory.Write<std::int32_t>(weapon + offset::m_iItemIDHigh, -1);

		//		memory.Write<std::int32_t>(weapon + offset::m_nFallbackPaintKit, paint);
		//		memory.Write<float>(weapon + offset::m_flFallbackWear, 0.1f);

		//		if (shouldUpdate)
		//			memory.Write<std::int32_t>(memory.Read<std::uintptr_t>(engine + offset::dwClientState) + 0x174, -1);
		//	}
		//}


	}

	return 0;
}