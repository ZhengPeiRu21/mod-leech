/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Leech.h"

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cstdlib>
#include <cctype>

static std::vector<uint32> const& Leech_GetRequiredItemIds()
{
    static std::vector<uint32> ids;
    static bool inited = false;
    if (!inited)
    {
        std::string raw = sConfigMgr->GetOption<std::string>("Leech.RequiredItemId", "");
        if (!raw.empty())
        {
            std::stringstream ss(raw);
            std::string tok;
            while (std::getline(ss, tok, ','))
            {
                tok.erase(std::remove_if(tok.begin(), tok.end(), ::isspace), tok.end());
                if (!tok.empty())
                {
                    char* end = nullptr;
                    unsigned long v = std::strtoul(tok.c_str(), &end, 10);
                    if (end != tok.c_str() && *end == '\0' && v <= std::numeric_limits<uint32>::max())
                        ids.push_back(static_cast<uint32>(v));
                }
            }
        }
        inited = true;
    }
    return ids;
}

class Leech_UnitScript : public UnitScript
{
public:
    Leech_UnitScript() : UnitScript("Leech_UnitScript") { }

    void OnDamage(Unit* attacker, Unit* victim, uint32& damage) override
    {
        if (!sConfigMgr->GetOption<bool>("Leech.Enable", true) || !attacker)
        {
            return;
        }

        bool isPet = attacker->GetOwner() && attacker->GetOwner()->GetTypeId() == TYPEID_PLAYER;
        if (!isPet && attacker->GetTypeId() != TYPEID_PLAYER)
        {
            return;
        }
        Player* player = isPet ? attacker->GetOwner()->ToPlayer() : attacker->ToPlayer();
		if (!player)
			return;

        if (sConfigMgr->GetOption<bool>("Leech.DungeonsOnly", true) && !(player->GetMap()->IsDungeon()))
        {
            return;
        }

		auto const& reqIds = Leech_GetRequiredItemIds();
		// Skip item check if only "0" is specified (means "no item required")
		if (!reqIds.empty() && !(reqIds.size() == 1 && reqIds[0] == 0))
		{
			bool hasAny = false;
			for (uint32 id : reqIds)
			{
				if (player->HasItemCount(id, 1, false)) { hasAny = true; break; }
			}
			if (!hasAny)
				return;
		}

        auto leechAmount = sConfigMgr->GetOption<float>("Leech.Amount", 0.05f);
		auto bp1 = static_cast<int32>(leechAmount * float(damage));
		
		Unit* caster = attacker;
		Unit* target = attacker;
		
		std::string petMode = sConfigMgr->GetOption<std::string>("Leech.PetDamage", "pet");
		if (isPet)
		{
			if (petMode == "none")
				return;
			else if (petMode == "owner")
				caster = target = player;
			else
				caster = target = attacker;
		}
		else
		{
			caster = target = player;
		}
		
		caster->CastCustomSpell(target, SPELL_HEAL, &bp1, nullptr, nullptr, true);
    }
};


// Add all scripts in one
void AddSC_mod_leech()
{
    new Leech_UnitScript();
}
