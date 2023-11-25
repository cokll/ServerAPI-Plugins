#include "Hooks.h"

#include "Main.h"

namespace Permissions::Hooks
{
	DECLARE_HOOK(AShooterGameMode_HandleNewPlayer, bool, AShooterGameMode*, AShooterPlayerController*, UPrimalPlayerData*, AShooterCharacter*, bool);
	DECLARE_HOOK(AShooterPlayerController_ClientNotifyAdmin, void, AShooterPlayerController*);
	DECLARE_HOOK(AShooterGameMode_PostLogin, void, AShooterGameMode*, APlayerController*);

	bool Hook_AShooterGameMode_HandleNewPlayer(AShooterGameMode* _this, AShooterPlayerController* new_player, UPrimalPlayerData* player_data, AShooterCharacter* player_character, bool is_from_login)
	{
		FString eos_id;
		new_player->GetUniqueNetIdAsString(&eos_id);
		
		if (!database->IsPlayerExists(*eos_id))
		{
			const bool res = database->AddPlayer(*eos_id);
			if (!res)
			{
				Log::GetLog()->error("({} {}) Couldn't add player", __FILE__, __FUNCTION__);
			}
		}

		if (new_player->bIsAdmin()())
			if (!IsPlayerInGroup(eos_id, "Admins"))
				database->AddPlayerToGroup(*eos_id, "Admins");

		return AShooterGameMode_HandleNewPlayer_original(_this, new_player, player_data, player_character,
			is_from_login);
	}

	void Hook_AShooterGameMode_PostLogin(AShooterGameMode* _this, APlayerController* NewPlayer)
	{
		Log::GetLog()->info("Hook_AShooterGameMode_PostLogin Fired");

		AShooterGameMode_PostLogin_original(_this, NewPlayer);

		FString eos_id;
		auto APC = static_cast<AShooterPlayerController*>(NewPlayer);
		if (APC)
		{
			APC->GetUniqueNetIdAsString(&eos_id);
			if (!eos_id.IsEmpty())
			{
				Log::GetLog()->info("({} {}) Player {} logged in", __FILE__, __FUNCTION__, eos_id.ToString());

				if (APC->bIsAdmin()())
				{
					if (!IsPlayerInGroup(eos_id, "Admins"))
					{
						database->AddPlayerToGroup(*eos_id, "Admins");
					}
					else
						Log::GetLog()->info("({} {}) Player {} is already admin", __FILE__, __FUNCTION__, eos_id.ToString());
				}
				else
					Log::GetLog()->info("({} {}) Player {} is not admin", __FILE__, __FUNCTION__, eos_id.ToString());
			}
			else
				Log::GetLog()->error("({} {}) Couldn't get UniqueNetIdAsString", __FILE__, __FUNCTION__);
		}
		else
			Log::GetLog()->error("({} {}) Couldn't cast APlayerController to AShooterPlayerController", __FILE__, __FUNCTION__);
	}

	void Hook_AShooterPlayerController_ClientNotifyAdmin(AShooterPlayerController* player_controller)
	{
		FString eos_id;
		player_controller->GetUniqueNetIdAsString(&eos_id);

			if (!IsPlayerInGroup(eos_id, "Admins"))
				database->AddPlayerToGroup(*eos_id, "Admins");

		AShooterPlayerController_ClientNotifyAdmin_original(player_controller);
	}

	void Init()
	{
		AsaApi::GetHooks().SetHook("AShooterGameMode.HandleNewPlayer_Implementation(AShooterPlayerController*,UPrimalPlayerData*,AShooterCharacter*,bool)",&Hook_AShooterGameMode_HandleNewPlayer, &AShooterGameMode_HandleNewPlayer_original);
		AsaApi::GetHooks().SetHook("AShooterPlayerController.ClientNotifyAdmin()", &Hook_AShooterPlayerController_ClientNotifyAdmin, &AShooterPlayerController_ClientNotifyAdmin_original);
		AsaApi::GetHooks().SetHook("AShooterGameMode.PostLogin(APlayerController*)", &Hook_AShooterGameMode_PostLogin, &AShooterGameMode_PostLogin_original);
	}
}