/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Leech.h"

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

        if (sConfigMgr->GetOption<bool>("Leech.DungeonsOnly", true) && !(player->GetMap()->IsDungeon()))
        {
            return;
        }

        auto leechAmount = sConfigMgr->GetOption<float>("Leech.Amount", 0.05f);
        auto bp1 = static_cast<int32>(leechAmount * float(damage));
        player->CastCustomSpell(attacker, SPELL_HEAL, &bp1, nullptr, nullptr, true);
    }
};


// Add all scripts in one
void AddSC_mod_leech()
{
    new Leech_UnitScript();
}
