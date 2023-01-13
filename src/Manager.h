#pragma once

#include "Manager.h"
#include "Serialization.h"

namespace ISCDNPC
{
	struct detail
	{
		static RE::TESShout* GetShout(const RE::Actor* a_attacker)
		{
			const auto currentProcess = a_attacker->currentProcess;
			const auto high = currentProcess ? currentProcess->high : nullptr;

            return high ? high->currentShout : nullptr;
		}

	    static float GetMaxVoiceRecoveryTime()
		{
			return RE::GameSettingCollection::GetSingleton()->GetSetting("fCombatInventoryShoutMaxRecoveryTime")->GetFloat();
		}

        static float GetVoiceRecoveryTime(const RE::Actor* a_attacker)
		{
			const auto currentProcess = a_attacker->currentProcess;
			return currentProcess ? currentProcess->GetVoiceRecoveryTime() : 0.0f;
		}

        static void SetVoiceRecoveryTime(const RE::NiPointer<RE::Actor>& a_attacker, float a_time)
		{
			const auto currentProcess = a_attacker->currentProcess;
			const auto high = currentProcess ? currentProcess->high : nullptr;

			if (high) {
				high->voiceRecoveryTime = a_time;
			}
		}
	};

	namespace Shout
	{
		struct Input
		{
			RE::FormID npcID;
			RE::FormID shoutID;
		};

		struct Cooldown
		{
			float startTime{ 0.0f };
			float timeRemaining{ 0.0f };
		};
	}

	template <class T>
	class CheckShouldEquip
	{
	public:
		static void Install(REL::ID a_vtable_id)
		{
			REL::Relocation<std::uintptr_t> vtbl{ a_vtable_id };
			func = vtbl.write_vfunc(0xF, thunk);

			logger::info("Installed {} hook", typeid(T).name());
		}
	private:
		static bool thunk(RE::CombatInventoryItemMagicT<RE::CombatInventoryItemShout, T>* a_this, RE::CombatController* a_controller)
		{
			auto result = func(a_this, a_controller);

			const auto attacker = a_controller->handleCount ?
			                          a_controller->cachedAttacker :
			                          a_controller->attackerHandle.get();

			if (!result && a_this->IsValid()) {
				const static auto maxVoiceRecoveryTime = detail::GetMaxVoiceRecoveryTime();

				if (detail::GetVoiceRecoveryTime(attacker.get()) > maxVoiceRecoveryTime) {
					const auto shout = a_this->item->As<RE::TESShout>();

					const Shout::Input input{ attacker->GetFormID(), shout->GetFormID() };
					const auto currentTime = RE::ProcessLists::GetSingleton()->GetSystemTimeClock();

					float newTime;
					if (const auto shoutCooldown = Manager::GetSingleton()->GetCooldown(input)) {
						newTime = shoutCooldown->timeRemaining - (currentTime - shoutCooldown->startTime);
					} else {
						logger::info("[{}] [{}] Resetting cooldown", attacker->GetName(), shout->GetName());
					    newTime = maxVoiceRecoveryTime;
					}

					if (newTime > maxVoiceRecoveryTime) {
						Manager::GetSingleton()->SetCooldown(input, { currentTime, newTime });
						logger::debug("[{}] Cooldown : {} remaining", shout->GetName(), newTime);
					} else if (Manager::GetSingleton()->EraseCooldown(input)) {
						logger::info("[{}] [{}] Cooldown over", attacker->GetName(), shout->GetName());
					}

				    detail::SetVoiceRecoveryTime(attacker, newTime);
					return newTime <= maxVoiceRecoveryTime;
				}
			}

			return result;
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	void Install();
}
