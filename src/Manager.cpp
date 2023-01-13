#include "Manager.h"

namespace ISCDNPC
{
	struct ExecuteHandler
	{
		static bool thunk(RE::VoiceSpellFireHandler* a_this, RE::Actor& a_actor, const RE::BSFixedString& a_tag)
		{
			const auto result = func(a_this, a_actor, a_tag);

			if (result && !a_actor.IsPlayerRef()) {
				if (const auto shout = detail::GetShout(&a_actor)) {
					const Shout::Input input{ a_actor.GetFormID(), shout->GetFormID() };
					auto voiceRecoveryTime = detail::GetVoiceRecoveryTime(&a_actor);

					Manager::GetSingleton()->SetCooldown(input, { RE::ProcessLists::GetSingleton()->GetSystemTimeClock(), voiceRecoveryTime });
					logger::info("[{}] [{}] Cooldown started [{}]", a_actor.GetName(), shout->GetName(), voiceRecoveryTime);
				}
			}

			return result;
		}
		static inline REL::Relocation<decltype(thunk)> func;
		static inline std::size_t idx{ 0x1 };
	};

	void Install()
	{
		CheckShouldEquip<RE::CombatMagicCasterBoundItem>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterBoundItem_[0]);
		CheckShouldEquip<RE::CombatMagicCasterCloak>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterCloak_[0]);
		CheckShouldEquip<RE::CombatMagicCasterDisarm>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterDisarm_[0]);
		CheckShouldEquip<RE::CombatMagicCasterInvisibility>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterInvisibility_[0]);
		CheckShouldEquip<RE::CombatMagicCasterLight>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterLight_[0]);
		CheckShouldEquip<RE::CombatMagicCasterOffensive>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterOffensive_[0]);
		CheckShouldEquip<RE::CombatMagicCasterParalyze>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterParalyze_[0]);
		CheckShouldEquip<RE::CombatMagicCasterReanimate>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterReanimate_[0]);
		CheckShouldEquip<RE::CombatMagicCasterRestore>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterRestore_[0]);
		CheckShouldEquip<RE::CombatMagicCasterScript>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterScript_[0]);
		CheckShouldEquip<RE::CombatMagicCasterStagger>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterStagger_[0]);
		CheckShouldEquip<RE::CombatMagicCasterSummon>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterSummon_[0]);
		CheckShouldEquip<RE::CombatMagicCasterTargetEffect>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterTargetEffect_[0]);
		CheckShouldEquip<RE::CombatMagicCasterWard>::Install(RE::VTABLE_CombatInventoryItemMagicT_CombatInventoryItemShout_CombatMagicCasterWard_[0]);

		stl::write_vfunc<RE::VoiceSpellFireHandler, ExecuteHandler>();

		if (const auto scriptEventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton()) {
			scriptEventSourceHolder->AddEventSink(Manager::GetSingleton());
		}
	}
}
