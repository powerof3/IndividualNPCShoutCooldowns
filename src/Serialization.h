#pragma once

namespace ISCDNPC
{
	namespace Shout
	{
		struct Input;
		struct Cooldown;
	}

	namespace Serialization
	{
		enum : std::uint32_t
		{
			kSerializationVersion = 1,
			kIndividualShoutCooldownNPC = 'PSCD',
			kManager = 'MNGR'
		};

        inline std::string DecodeTypeCode(std::uint32_t a_typeCode)
		{
			constexpr std::size_t SIZE = sizeof(std::uint32_t);

			std::string sig;
			sig.resize(SIZE);
			const char* iter = reinterpret_cast<char*>(&a_typeCode);
			for (std::size_t i = 0, j = SIZE - 2; i < SIZE - 1; ++i, --j) {
				sig[j] = iter[i];
			}

			return sig;
		}

		void SaveCallback(SKSE::SerializationInterface* a_intfc);

		void LoadCallback(SKSE::SerializationInterface* a_intfc);

		void RevertCallback(SKSE::SerializationInterface* a_intfc);
	}

	class Manager : public RE::BSTEventSink<RE::TESFormDeleteEvent>
	{
	public:
		[[nodiscard]] static Manager* GetSingleton()
		{
			static Manager singleton;
			return std::addressof(singleton);
		}

		std::optional<Shout::Cooldown> GetCooldown(const Shout::Input& a_shoutInput);
		void SetCooldown(const Shout::Input& a_shoutInput, const Shout::Cooldown& a_cooldown);
		bool EraseCooldown(const Shout::Input& a_shoutInput);

		void Save(SKSE::SerializationInterface* a_intfc, std::uint32_t a_type, std::uint32_t a_version);
        void Save(SKSE::SerializationInterface* a_intfc);
		void Load(SKSE::SerializationInterface* a_intfc);
		void Revert(SKSE::SerializationInterface* a_intfc);

		RE::BSEventNotifyControl ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>*) override;

    private:
		Manager() = default;
		Manager(const Manager&) = delete;
		Manager(Manager&&) = delete;
		~Manager() = default;

		Manager& operator=(const Manager&) = delete;
		Manager& operator=(Manager&&) = delete;

		using Lock = std::recursive_mutex;
		using Locker = std::scoped_lock<Lock>;

		// NPC formID, shout formID, time
		Map<RE::FormID, Map<RE::FormID, Shout::Cooldown>> _map;
		mutable Lock _lock;
	};
}
