#include "Serialization.h"
#include "Manager.h"

namespace ISCDNPC
{
	namespace Serialization
	{
		void SaveCallback(SKSE::SerializationInterface* a_intfc)
		{
			Manager::GetSingleton()->Save(a_intfc, kManager, kSerializationVersion);
			logger::info("Finished saving data"sv);
		}

		void LoadCallback(SKSE::SerializationInterface* a_intfc)
		{
			std::uint32_t type;
			std::uint32_t version;
			std::uint32_t length;
			while (a_intfc->GetNextRecordInfo(type, version, length)) {
				if (version < kSerializationVersion) {
					logger::critical("Loaded data is out of date! Read ({}), expected ({}) for type code ({})", version, kSerializationVersion, DecodeTypeCode(type));
					continue;
				}
				if (type == kManager) {
					Manager::GetSingleton()->Load(a_intfc);
				}
			}
			logger::info("Finished loading data"sv);
		}

		void RevertCallback(SKSE::SerializationInterface* a_intfc)
		{
			Manager::GetSingleton()->Revert(a_intfc);
			logger::info("Finished reverting data"sv);
		}
	}

	std::optional<Shout::Cooldown> Manager::GetCooldown(const Shout::Input& a_shoutInput)
	{
		Locker lock(_lock);
		if (const auto npcIt = _map.find(a_shoutInput.npcID); npcIt != _map.end()) {
			if (const auto shoutIt = npcIt->second.find(a_shoutInput.shoutID); shoutIt != npcIt->second.end()) {
				return shoutIt->second;
			}
		}
		return std::nullopt;
	}

	void Manager::SetCooldown(const Shout::Input& a_shoutInput, const Shout::Cooldown& a_cooldown)
	{
		Locker lock(_lock);
		_map[a_shoutInput.npcID].emplace(a_shoutInput.shoutID, a_cooldown);
	}

	bool Manager::EraseCooldown(const Shout::Input& a_shoutInput)
	{
		Locker lock(_lock);
		if (const auto npcIt = _map.find(a_shoutInput.npcID); npcIt != _map.end()) {
			return npcIt->second.erase(a_shoutInput.shoutID);
		}
		return false;
	}

	void Manager::Save(SKSE::SerializationInterface* a_intfc, std::uint32_t a_type, std::uint32_t a_version)
	{
		assert(a_intfc);
		if (!a_intfc->OpenRecord(a_type, a_version)) {
			logger::error("Failed to open record!");
			return;
		}

		return Save(a_intfc);
	}

    void Manager::Save(SKSE::SerializationInterface* a_intfc)
	{
		assert(a_intfc);
		Locker lock(_lock);

		const std::size_t numRegs = _map.size();
		if (!a_intfc->WriteRecordData(numRegs)) {
			logger::error("Failed to save number of regs ({})", numRegs);
			return;
		}

		for (auto& [npcID, shoutDataMap] : _map) {
			if (!a_intfc->WriteRecordData(npcID)) {
				logger::error("Failed to save reg ({})", npcID);
				continue;
			}
			for (auto& [shoutID, cooldown] : shoutDataMap) {
				if (!a_intfc->WriteRecordData(shoutID)) {
					logger::error("Failed to save reg ({})", shoutID);
					continue;
				}
				if (!a_intfc->WriteRecordData(cooldown.startTime)) {
					logger::error("Failed to save reg ({})", cooldown.startTime);
				}
				if (!a_intfc->WriteRecordData(cooldown.timeRemaining)) {
					logger::error("Failed to save reg ({})", cooldown.timeRemaining);
				}
			}
		}
	}

	void Manager::Load(SKSE::SerializationInterface* a_intfc)
	{
		assert(a_intfc);
		std::size_t numRegs;
		a_intfc->ReadRecordData(numRegs);

		Locker lock(_lock);
		_map.clear();

		RE::FormID npcID;

		std::size_t numShoutData;
		RE::FormID shoutID;

		Shout::Cooldown cooldown{};

		for (std::size_t i = 0; i < numRegs; ++i) {
			a_intfc->ReadRecordData(npcID);
			if (!a_intfc->ResolveFormID(npcID, npcID)) {
				logger::warn("Error reading formID ({:X})", npcID);
				continue;
			}
			a_intfc->ReadRecordData(numShoutData);
			for (std::size_t j = 0; j < numShoutData; ++j) {
				a_intfc->ReadRecordData(shoutID);
				if (!a_intfc->ResolveFormID(shoutID, shoutID)) {
					logger::warn("Error reading formID ({:X})", shoutID);
					continue;
				}
				a_intfc->ReadRecordData(cooldown.startTime);
				a_intfc->ReadRecordData(cooldown.timeRemaining);

				_map[npcID][shoutID] = cooldown;
			}
		}
	}

	void Manager::Revert(SKSE::SerializationInterface*)
	{
		Locker lock(_lock);
		_map.clear();
	}

	RE::BSEventNotifyControl Manager::ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>*)
	{
		if (a_event && a_event->formID != 0) {
			Locker lock(_lock);
			_map.erase(a_event->formID);
		}

		return RE::BSEventNotifyControl::kContinue;
	}
}
