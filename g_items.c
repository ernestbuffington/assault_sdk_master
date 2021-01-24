
#include "g_local.h"


qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Use_Weapon2 (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);

// JOSEPH 12-OCT-98
void Weapon_Blackjack (edict_t *ent);
// JOSEPH 19-JAN-99
void Weapon_Crowbar (edict_t *ent);

// RAFAEL
void Weapon_SPistol (edict_t *ent);
void Weapon_Pistol (edict_t *ent);
void Weapon_Tommygun (edict_t *ent);

void Weapon_Blaster (edict_t *ent);
void Weapon_Msg90(edict_t *ent); //assault2
void Weapon_Mortar(edict_t *ent); //assault2
void Weapon_Shotgun (edict_t *ent);
void Weapon_Spaz (edict_t *ent); // assault2 virgin16 and virgin17
void Weapon_Ak47 (edict_t *ent); // assault2 virgin23
void Weapon_Machinegun (edict_t *ent);
void Weapon_Chaingun (edict_t *ent);
void Weapon_HyperBlaster (edict_t *ent);
void Weapon_RocketLauncher (edict_t *ent);
void Weapon_Grenade (edict_t *ent);
void Weapon_GrenadeLauncher (edict_t *ent);
void Weapon_Railgun (edict_t *ent);
void Weapon_BFG (edict_t *ent);
// RAFAEL
void Weapon_Ionripper (edict_t *ent);
void Weapon_Phalanx (edict_t *ent);
void Weapon_Trap (edict_t *ent);

void Weapon_FlameThrower (edict_t *ent);

void Weapon_Barmachinegun (edict_t *ent);

gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};

static int	jacket_armor_index;
static int	combat_armor_index;
static int	body_armor_index;
static int	power_screen_index;
static int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

void Use_Quad (edict_t *ent, gitem_t *item);
// RAFAEL
void Use_QuadFire (edict_t *ent, gitem_t *item);

static int	quad_drop_timeout_hack;
// RAFAEL
static int	quad_fire_drop_timeout_hack;

//======================================================================

/*
===============
GetItemByIndex
===============
*/
gitem_t	*GetItemByIndex (int index)
{
	if (index == 0 || index >= game.num_items)
		return NULL;

	return &itemlist[index];
}


/*
===============
FindItemByClassname

===============
*/
gitem_t	*FindItemByClassname (char *classname)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->classname)
			continue;
		if (!Q_stricmp(it->classname, classname))
			return it;
	}

	return NULL;
}

/*
===============
FindItem

===============
*/
gitem_t	*FindItem (char *pickup_name)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->pickup_name)
			continue;
		if (!Q_stricmp(it->pickup_name, pickup_name))
			return it;
	}

	return NULL;
}

//======================================================================

void DoRespawn (edict_t *ent)
{
	if (ent->team)
	{
		edict_t	*master;
		int	count;
		int choice;

		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->chain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
			;
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	gi.linkentity (ent);

	// send an effect
	ent->s.event = EV_ITEM_RESPAWN;
}

void SetRespawn (edict_t *ent, float delay)
{
	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;
	gi.linkentity (ent);
}


//======================================================================

qboolean Pickup_Powerup (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	if ((skill->value == 1 && quantity >= 2) || (skill->value >= 2 && quantity >= 1))
		return false;

	// Rafael no coop
	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);

// Ridah, 27-may-99, not used
/*
		if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}
		// RAFAEL
		else if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_QuadFire) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_QuadFire) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_fire_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}
*/
	}

	return true;
}

void Drop_General (edict_t *ent, gitem_t *item)
{
	Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
}


//======================================================================

// JOSEPH 23-APR-99
qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other)
{
	//if (!deathmatch->value)
	//	other->max_health += 1;

	if(other->health == other->max_health) // ASSAULT2 frank wont allow pickup if you already are fully health, annoying
		return false;
	if (other->health < other->max_health)
		other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}
// END JOSEPH

qboolean Pickup_AncientHead (edict_t *ent, edict_t *other)
{
	other->max_health += 2;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Bandolier (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (other->client->pers.max_bullets < 250)
		other->client->pers.max_bullets = 250;
	if (other->client->pers.max_shells < 150)
		other->client->pers.max_shells = 150;
	if (other->client->pers.max_cells < 250)
		other->client->pers.max_cells = 250;
	if (other->client->pers.max_slugs < 120)
		other->client->pers.max_slugs = 120;
	// RAFAEL
	if (other->client->pers.max_magslug < 75)
		other->client->pers.max_magslug = 75;

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (other->client->pers.max_bullets < 300)
		other->client->pers.max_bullets = 300;
	if (other->client->pers.max_shells < 200)
		other->client->pers.max_shells = 200;
	if (other->client->pers.max_rockets < 100)
		other->client->pers.max_rockets = 100;
	if (other->client->pers.max_grenades < 15)
		other->client->pers.max_grenades = 15;
	if (other->client->pers.max_cells < 300)
		other->client->pers.max_cells = 300;
	if (other->client->pers.max_slugs < 120)
		other->client->pers.max_slugs = 120;
	// RAFAEL
	if (other->client->pers.max_magslug < 100)
		other->client->pers.max_magslug = 100;

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	item = FindItem("Gas");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_cells)
			other->client->pers.inventory[index] = other->client->pers.max_cells;
	}

	item = FindItem("Grenades");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
			other->client->pers.inventory[index] = other->client->pers.max_grenades;
	}

	item = FindItem("Rockets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
			other->client->pers.inventory[index] = other->client->pers.max_rockets;
	}

	// JOSEPH 11-APR-99
	item = FindItem("308cal");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_slugs)
			other->client->pers.inventory[index] = other->client->pers.max_slugs;
	}
	// END JOSEPH

	// RAFAEL
	item = FindItem ("Mag Slug");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_magslug)
			other->client->pers.inventory[index] = other->client->pers.max_magslug;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

//======================================================================

void Use_Quad (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (quad_drop_timeout_hack)
	{
		timeout = quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += timeout;
	else
		ent->client->quad_framenum = level.framenum + timeout;

	// JOSEPH 29-MAR-99
	//gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
	// END JOSEPH
}


// =====================================================================

// RAFAEL
void Use_QuadFire (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (quad_fire_drop_timeout_hack)
	{
		timeout = quad_fire_drop_timeout_hack;
		quad_fire_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quadfire_framenum > level.framenum)
		ent->client->quadfire_framenum += timeout;
	else
		ent->client->quadfire_framenum = level.framenum + timeout;

	// JOSEPH 29-MAR-99
	//gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
	// END JOSEPH
}


//======================================================================

void Use_Breather (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->breather_framenum > level.framenum)
		ent->client->breather_framenum += 300;
	else
		ent->client->breather_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Envirosuit (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->enviro_framenum > level.framenum)
		ent->client->enviro_framenum += 300;
	else
		ent->client->enviro_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void	Use_Invulnerability (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += 300;
	else
		ent->client->invincible_framenum = level.framenum + 300;

	// JOSEPH 29-MAR-99 
	//gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
	// END JOSEPH
}

//======================================================================

void	Use_Silencer (edict_t *ent, gitem_t *item)
{
	// do not use anymore
	return;

//	ent->client->pers.inventory[ITEM_INDEX(item)]--;
//	ValidateSelectedItem (ent);
//	ent->client->pers.silencer_shots += 30;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

qboolean Pickup_Key (edict_t *ent, edict_t *other)
{
	// JOSEPH 26-JAN-99
	//ent->item->pickup_sound = "world/pickups/keys.wav";
	// END JOSEPH
	
	// Rafael no coop
	if (coop->value)
	{
		if (strcmp(ent->classname, "key_fuse") == 0)
		{
			if (other->client->pers.power_cubes & ((ent->spawnflags & 0x0000ff00)>> 8))
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
			other->client->pers.power_cubes |= ((ent->spawnflags & 0x0000ff00) >> 8);
		}
		else
		{
			if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
		}
		return true;
	}
// ASSAULT2 Frank 6/21/00 HOGIE //////////////////////////////////////////////////////////////////
	if( ( (strcmp(ent->classname, "key_key1") == 0) 
	   || (strcmp(ent->classname, "key_key2") == 0) 
	   || (strcmp(ent->classname, "key_key3") == 0) ) )
	{
		int count;

		if ( !ent->style ) ent->style = 2; // If a key has no style make it style 2 for Runts

		if(other->client->pers.team != ent->style) // Not our team's key.
		{
			if (other->client->last_wave > (level.time - 5) && (other->client->last_wave <= level.time)) // message delay
				return false;
			gi.centerprintf(other, "\n\n\n%s can't touch\nother team's %s!\n", team_names[other->client->pers.team] , ent->item->pickup_name);
			other->client->last_wave = level.time;
			gi.positioned_sound(ent->s.origin, ent, CHAN_ITEM, gi.soundindex("scenaric/fingers_open1.wav"), 1, 3, 0);
			return false;
		}

		if(!strcmp(ent->classname, "key_key2") && (team_cash[other->client->pers.team] != 1 ))
		{
			if (other->client->last_wave > (level.time - 5) && (other->client->last_wave <= level.time)) // message delay
				return false;
			gi.centerprintf(other, "\n\n\n%s must deposit Key_One before\n%s can have %s!\n", team_names[other->client->pers.team] , other->client->pers.netname, ent->item->pickup_name );
			other->client->last_wave = level.time;
			gi.positioned_sound(ent->s.origin, ent, CHAN_ITEM, gi.soundindex("scenaric/fingers_open1.wav"), 1, 3, 0);
			return false;
		}

		if(!strcmp(ent->classname, "key_key3") && (team_cash[other->client->pers.team] != 2 ))
		{
			if (other->client->last_wave > (level.time - 5) && (other->client->last_wave <= level.time)) // message delay
				return false;
			gi.centerprintf(other, "\n\n\n%s must deposit Key_Two before\n%s can have %s.\n", team_names[other->client->pers.team] , other->client->pers.netname, ent->item->pickup_name );
			other->client->last_wave = level.time;
			gi.positioned_sound(ent->s.origin, ent, CHAN_ITEM, gi.soundindex("scenaric/fingers_open1.wav"), 1, 3, 0);
			return false;
		}

		// we have successfully picked up the key by this point so tell the world.

		//copy any target name to a buffer (other->deathtarget) so the target isnt triggered until key is deposited
		// copy key->wait to client->wait
		// copy key->option or key->order into client->option or client->order

		switch (other->client->pers.team)
		{
		case 1:
		if      (!strcmp(ent->classname, "key_key1") && (level.thugs_key1_target) ) other->deathtarget = G_CopyString( level.thugs_key1_target );
		else if (!strcmp(ent->classname, "key_key2") && (level.thugs_key2_target) ) other->deathtarget = G_CopyString( level.thugs_key2_target );
		else if (!strcmp(ent->classname, "key_key3") && (level.thugs_key3_target) ) other->deathtarget = G_CopyString( level.thugs_key3_target );
		break;

		case 2:
		if      (!strcmp(ent->classname, "key_key1") && (level.runts_key1_target) ) other->deathtarget = G_CopyString( level.runts_key1_target );
		else if (!strcmp(ent->classname, "key_key2") && (level.runts_key2_target) ) other->deathtarget = G_CopyString( level.runts_key2_target );
		else if (!strcmp(ent->classname, "key_key3") && (level.runts_key3_target) ) other->deathtarget = G_CopyString( level.runts_key3_target );
		break;
		}

		if (ent->wait)	other->wait = ent->wait;
		else			other->wait = 15.0;
		
		if (ent->option) other->option = ent->option;
		if (ent->order)	other->count = ent->order;

		// now tell everyone except this player
		for (count=1; count<=(int)maxclients->value; count++)
		{
			if (!g_edicts[count].client)
				continue;
			if (!g_edicts[count].inuse)
				continue;
			if (&g_edicts[count] == other)
			{	gi.cprintf( &g_edicts[count], PRINT_CHAT, ">> %s has %s %s!\n", other->client->pers.netname, team_names[other->client->pers.team] , ent->item->pickup_name );
				continue;
			}
			if (other->client->pers.team == g_edicts[count].client->pers.team)// These guys are on our team.
				gi.sound(&g_edicts[count], CHAN_SPECIAL, gi.soundindex("world/alarm.wav"), 1, ATTN_NORM, 0);

			gi.cprintf( &g_edicts[count], PRINT_CHAT, ">> %s has %s %s!\n", other->client->pers.netname, team_names[other->client->pers.team] , ent->item->pickup_name );
		}
		other->client->pers.bagcash += 1;
		gi.sound(ent, CHAN_ITEM, gi.soundindex("world/alarm.wav"), 1, ATTN_NORM, 0);
	}
	// End ASSAULT2 Frank 6/21/00 HOGIE///////////////////////////////////////////////////////////////////
	
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}

//======================================================================

qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count)
{
	int			index;
	int			max;

	if (!ent->client)
		return false;

	if (item->tag == AMMO_BULLETS)
		max = ent->client->pers.max_bullets;
	else if (item->tag == AMMO_SHELLS)
		max = ent->client->pers.max_shells;
	else if (item->tag == AMMO_ROCKETS)
		max = ent->client->pers.max_rockets;
	else if (item->tag == AMMO_GRENADES)
		max = ent->client->pers.max_grenades;
	else if (item->tag == AMMO_CELLS)
		max = ent->client->pers.max_cells;
	else if (item->tag == AMMO_SLUGS)
		max = ent->client->pers.max_slugs;
	else if (item->tag == AMMO_MAGSLUG)
		max = ent->client->pers.max_magslug;
	else if (item->tag == AMMO_TRAP)
		max = ent->client->pers.max_trap;
	else
		return false;

	index = ITEM_INDEX(item);

	if (ent->client->pers.inventory[index] == max)
		return false;

	ent->client->pers.inventory[index] += count;

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

	return true;
}

qboolean Pickup_Ammo_Cylinder (edict_t *ent, edict_t *other)
{
	gitem_t	*it;
	int		index;

	it = FindItem("Bullets");
	if (it)
	{
		index = ITEM_INDEX(it);
		if (other->client->pers.inventory[index] >= other->client->pers.max_bullets)
			return false;

		other->client->pers.inventory[index] += ent->item->quantity; // get the cylinder quantity
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);
	
	return true;
}

qboolean Pickup_Ammo (edict_t *ent, edict_t *other)
{
	int			oldcount;
	int			count;
	qboolean	weapon;

	weapon = (ent->item->flags & IT_WEAPON);
	if ( (weapon) && deathmatch->value && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;

	oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (!Add_Ammo (other, ent->item, count))
		return false;

	if (weapon && !oldcount)
	{
		if (other->client->pers.weapon != ent->item && ( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
			other->client->newweapon = ent->item;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);
	return true;
}

void Drop_Ammo (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	int		index;

	index = ITEM_INDEX(item);
	
	if (ent->client->pers.inventory[index] <= 0)
		return;
	
	dropped = Drop_Item (ent, item);
	if (ent->client->pers.inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
		dropped->count = ent->client->pers.inventory[index];

	if (ent->client->pers.weapon && 
		ent->client->pers.weapon->tag == AMMO_GRENADES &&
		item->tag == AMMO_GRENADES &&
		ent->client->pers.inventory[index] - dropped->count <= 0) {
		cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		G_FreeEdict(dropped);
		return;
	}

	ent->client->pers.inventory[index] -= dropped->count;
	ValidateSelectedItem (ent);
}


// JOSEPH 18-MAR-99
qboolean Pickup_Cash (edict_t *ent, edict_t *other)
{

	if (!ent->currentcash)
	//	cprintf (other, PRINT_HIGH, "%i dollars found\n", ent->currentcash);
	//else
		cprintf (other, PRINT_HIGH, "Counterfeit money! 0 dollars found\n", ent->currentcash);


	if (ent->currentcash > 0)
	{
		if (teamplay->value && (teamplay_mode == TM_GRABDALOOT))
		{
			if (other->client->pers.currentcash > MAX_CASH_PLAYER)
				return false;
		}

		other->client->ps.stats[STAT_CASH_PICKUP] = ent->currentcash;
		other->client->pers.currentcash += ent->currentcash;

		if (teamplay->value && (teamplay_mode == TM_GRABDALOOT))
		{
			if (other->client->pers.currentcash > MAX_CASH_PLAYER)
				other->client->pers.currentcash = MAX_CASH_PLAYER;
		}
	}
	else	// negative money is bagged money (used in Teamplay)
	{
		if (teamplay->value && (teamplay_mode == TM_GRABDALOOT))
		{
			if (other->client->pers.bagcash > MAX_BAGCASH_PLAYER)
				other->client->pers.bagcash = MAX_BAGCASH_PLAYER;
		}

		ent->currentcash = -ent->currentcash;
		other->client->pers.bagcash += ent->currentcash;

		if (teamplay->value && (teamplay_mode == TM_GRABDALOOT))
		{
			if (other->client->pers.bagcash > MAX_BAGCASH_PLAYER)
			{
				edict_t *cash;

				// leave the left-over money here in a smaller bag
				if ((other->client->pers.bagcash - MAX_BAGCASH_PLAYER) > 100)
					cash = SpawnTheWeapon( other, "item_cashbaglarge" );
				else
					cash = SpawnTheWeapon( other, "item_cashbagsmall" );

				cash->currentcash = -(other->client->pers.bagcash - MAX_BAGCASH_PLAYER);

				other->client->pers.bagcash = MAX_BAGCASH_PLAYER;
			}
		}
	}

	// flash the screen green
	other->client->bonus_alpha_color = 2;	

	return true;
}
// END JOSEPH

//======================================================================

void MegaHealth_think (edict_t *self)
{
	if (self->owner->health > self->owner->max_health)
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}


qboolean Pickup_Pistol_Mods (edict_t *ent, edict_t *other)
{

	if (ent->count == 1)
	{
		if(other->client->pers.pistol_mods == WEAPON_MOD_ROF) // Frank wont allow pickup if you have it already. annoying PUREASSAULT
			return false;
		other->client->pers.pistol_mods |= WEAPON_MOD_ROF;
		//gi.dprintf ("Pistol Mod: Faster ROF\n");
	}
	else if (ent->count == 2)
	{
		if(other->client->pers.pistol_mods == WEAPON_MOD_RELOAD) // Frank wont allow pickup if you already have one. PUREASSAULT
			return false;
		other->client->pers.pistol_mods |= WEAPON_MOD_RELOAD;
		//gi.dprintf ("Pistol Mod: Faster Reload\n");
	}
	else if (ent->count == 3)
	{
		if(other->client->pers.pistol_mods == WEAPON_MOD_DAMAGE) // Frank wont allow pickup if you already have one. PUREASSAULT
			return false;
		other->client->pers.pistol_mods |= WEAPON_MOD_DAMAGE;
		//gi.dprintf ("Pistol Mod: Damage x 2\n");
	
	}
	else if (ent->count == 4)
	{
		static int i; //FRANK ASSAULT2

		if (other->client->pers.pistol_mods & WEAPON_MOD_COOLING_JACKET) // Frank wont allow pickup if you have it PUREASSAULT
			return false;
		other->client->pers.pistol_mods |= WEAPON_MOD_COOLING_JACKET;
		other->client->pers.hmg_shots = 30;
///////// ASSAULT2
		i = (i + 1) % 4;

		switch(i)
		{
		case 0:
		gi.sound(ent, CHAN_AUTO, gi.soundindex("world/pawnomatic/mod_hmg1.wav"), 1, ATTN_NORM, 0); //FRANK
		break;

		case 1:
		gi.sound(ent, CHAN_AUTO, gi.soundindex("world/pawnomatic/weapon7.wav"), 1, ATTN_NORM, 0); //FRANK
		break;

		case 2:
		gi.sound(ent, CHAN_AUTO, gi.soundindex("world/pawnomatic/mod_hmg2.wav"), 1, ATTN_NORM, 0); //FRANK
		break;

		case 3:
		default:
		gi.sound(ent, CHAN_AUTO, gi.soundindex("world/pawnomatic/weapon6.wav"), 1, ATTN_NORM, 0); //FRANK
		}
///////// END ASSAULT2
	}
	else
	{
		gi.dprintf ("error in pickup pistol mods\n");
		return false;
	}

	if (ent->count <= 3)
	{
	if (other->client->pers.weapon == FindItem ("SPistol") || other->client->pers.weapon == FindItem("Pistol"))
	{
		other->client->newweapon = other->client->pers.weapon;
		ChangeWeapon (other);
	}
	}
	else if (ent->count == 4)
	{
		if (other->client->pers.weapon == FindItem ("Heavy machinegun") )
		{
			other->client->newweapon = other->client->pers.weapon;
			ChangeWeapon (other);
		}
	}

	// ent->item->pickup_sound = "items/pistolmod1.wav";

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
			SetRespawn (ent, 30);

	return true;
}

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
			return false;

	other->health += ent->count;

	if (ent->count == 2)
		ent->item->pickup_sound = "items/s_health.wav";
	else if (ent->count == 10)
		ent->item->pickup_sound = "items/n_health.wav";
	else if (ent->count == 25)
		ent->item->pickup_sound = "items/l_health.wav";
	else // (ent->count == 100)
		ent->item->pickup_sound = "items/m_health.wav";

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

	if (ent->style & HEALTH_TIMED)
	{
		ent->think = MegaHealth_think;
		ent->nextthink = level.time + 5;
		ent->owner = other;
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	else
	{
		if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
			SetRespawn (ent, 30);
	}

	return true;
}

//======================================================================

int ArmorIndex (edict_t *ent)
{
	// JOSEPH 1-APR-99
	/*if (!ent->client)
		return 0;

	if (ent->client->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (ent->client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (ent->client->pers.inventory[body_armor_index] > 0)
		return body_armor_index;*/
	// END JOSEPH

	return 0;
}

// JOSEPH 2-APR-99
qboolean Pickup_Armor (edict_t *ent, edict_t *other)
{
	if (strcmp(ent->classname, "item_armor_helmet") == 0)
	{
		gitem_t *itemh = FindItem("Helmet Armor");
		gitem_t *itemhh = FindItem("Helmet Armor Heavy");
		
		if ((other->client->pers.inventory[ITEM_INDEX(itemh)] == 100) ||
		   (other->client->pers.inventory[ITEM_INDEX(itemhh)] == 100))
			return false;
		
		if (other->client->pers.inventory[ITEM_INDEX(itemhh)])
		{
			other->client->pers.inventory[ITEM_INDEX(itemhh)] += 25;
			if (other->client->pers.inventory[ITEM_INDEX(itemhh)] > 100)
				other->client->pers.inventory[ITEM_INDEX(itemhh)] = 100;
		}
		else
		{
			other->client->pers.inventory[ITEM_INDEX(itemh)] += 50;
			if (other->client->pers.inventory[ITEM_INDEX(itemh)] > 100)
				other->client->pers.inventory[ITEM_INDEX(itemh)] = 100;
		}
	}
	else if (strcmp(ent->classname, "item_armor_jacket") == 0)
	{
		gitem_t *itemj = FindItem("Jacket Armor");
		gitem_t *itemjh = FindItem("Jacket Armor Heavy");
		
		if ((other->client->pers.inventory[ITEM_INDEX(itemj)] == 100) ||
		   (other->client->pers.inventory[ITEM_INDEX(itemjh)] == 100))
			return false;

		if (other->client->pers.inventory[ITEM_INDEX(itemjh)])
		{
			other->client->pers.inventory[ITEM_INDEX(itemjh)] += 25;
			if (other->client->pers.inventory[ITEM_INDEX(itemjh)] > 100)
				other->client->pers.inventory[ITEM_INDEX(itemjh)] = 100;
		}
		else
		{
			other->client->pers.inventory[ITEM_INDEX(itemj)] += 50;
			if (other->client->pers.inventory[ITEM_INDEX(itemj)] > 100)
				other->client->pers.inventory[ITEM_INDEX(itemj)] = 100;
		}
	}
	else if (strcmp(ent->classname, "item_armor_legs") == 0)
	{
		gitem_t *iteml = FindItem("Legs Armor");
		gitem_t *itemlh = FindItem("Legs Armor Heavy");	
		
		if ((other->client->pers.inventory[ITEM_INDEX(iteml)] == 100) ||
		   (other->client->pers.inventory[ITEM_INDEX(itemlh)] == 100))
			return false;

		if (other->client->pers.inventory[ITEM_INDEX(itemlh)])
		{
			other->client->pers.inventory[ITEM_INDEX(itemlh)] += 25;
			if (other->client->pers.inventory[ITEM_INDEX(itemlh)] > 100)
				other->client->pers.inventory[ITEM_INDEX(itemlh)] = 100;
		}
		else
		{
			other->client->pers.inventory[ITEM_INDEX(iteml)] += 50;
			if (other->client->pers.inventory[ITEM_INDEX(iteml)] > 100)
				other->client->pers.inventory[ITEM_INDEX(iteml)] = 100;
		}
	}
	else if (strcmp(ent->classname, "item_armor_helmet_heavy") == 0)
	{
		gitem_t *itemh = FindItem("Helmet Armor");
		gitem_t *itemhh = FindItem("Helmet Armor Heavy");
		
		if (other->client->pers.inventory[ITEM_INDEX(itemhh)] == 100)
			return false;
		
		if (other->client->pers.inventory[ITEM_INDEX(itemh)])
			other->client->pers.inventory[ITEM_INDEX(itemh)] = 0;			

		other->client->pers.inventory[ITEM_INDEX(itemhh)] = 100;
	}
	else if (strcmp(ent->classname, "item_armor_jacket_heavy") == 0)
	{
		gitem_t *itemj = FindItem("Jacket Armor");
		gitem_t *itemjh = FindItem("Jacket Armor Heavy");
		
		if (other->client->pers.inventory[ITEM_INDEX(itemjh)] == 100)
			return false;		

		if (other->client->pers.inventory[ITEM_INDEX(itemj)])
			other->client->pers.inventory[ITEM_INDEX(itemj)] = 0;			

		other->client->pers.inventory[ITEM_INDEX(itemjh)] = 100;
	}
	else if (strcmp(ent->classname, "item_armor_legs_heavy") == 0)
	{
		gitem_t *iteml = FindItem("Legs Armor");
		gitem_t *itemlh = FindItem("Legs Armor Heavy");	
		
		if (other->client->pers.inventory[ITEM_INDEX(itemlh)] == 100)
			return false;

		if (other->client->pers.inventory[ITEM_INDEX(iteml)])
			other->client->pers.inventory[ITEM_INDEX(iteml)] = 0;			

		other->client->pers.inventory[ITEM_INDEX(itemlh)] = 100;
	}
	
	
	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 45);	// Ridah, armor shouldn't respawn so often

/*	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex (other);

	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
			other->client->pers.inventory[jacket_armor_index] = 2;
		else
			other->client->pers.inventory[old_armor_index] += 2;
	}

	// if player has no armor, just use it
	else if (!old_armor_index)
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;
	}

	// use the better armor
	else
	{
		// get info on old armor
		if (old_armor_index == jacket_armor_index)
			oldinfo = &jacketarmor_info;
		else if (old_armor_index == combat_armor_index)
			oldinfo = &combatarmor_info;
		else // (old_armor_index == body_armor_index)
			oldinfo = &bodyarmor_info;

		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			// calc new armor values
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = salvage * other->client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;
			if (newcount > newinfo->max_count)
				newcount = newinfo->max_count;

			// zero count of old armor so it goes away
			other->client->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
		}
		else
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount = other->client->pers.inventory[old_armor_index] + salvagecount;
			if (newcount > oldinfo->max_count)
				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if (other->client->pers.inventory[old_armor_index] >= newcount)
				return false;

			// update current armor value
			other->client->pers.inventory[old_armor_index] = newcount;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 20);*/

	return true;
}
// END JOSEPH

//======================================================================

int PowerArmorType (edict_t *ent)
{
	if (!ent->client)
		return POWER_ARMOR_NONE;

	if (!(ent->flags & FL_POWER_ARMOR))
		return POWER_ARMOR_NONE;

	if (ent->client->pers.inventory[power_shield_index] > 0)
		return POWER_ARMOR_SHIELD;

	if (ent->client->pers.inventory[power_screen_index] > 0)
		return POWER_ARMOR_SCREEN;

	return POWER_ARMOR_NONE;
}

void Use_PowerArmor (edict_t *ent, gitem_t *item)
{
	int		index;

	if (ent->flags & FL_POWER_ARMOR)
	{
		ent->flags &= ~FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = ITEM_INDEX(FindItem("gas"));
		if (!ent->client->pers.inventory[index])
		{
			cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
			return;
		}
		ent->flags |= FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
	}
}

qboolean Pickup_PowerArmor (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		// auto-use for DM only if we didn't already have one
		if (!quantity)
			ent->item->use (other, ent->item);
	}

	return true;
}

void Drop_PowerArmor (edict_t *ent, gitem_t *item)
{
	if ((ent->flags & FL_POWER_ARMOR) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
		Use_PowerArmor (ent, item);
	Drop_General (ent, item);
}

//======================================================================

void Use_JetPack (edict_t *ent, gitem_t *item)
{
	if (ent->flags & FL_JETPACK)
	{
		ent->flags &= ~FL_JETPACK;

		gi.dprintf( "SOUND TODO: JetPack disabled\n" );
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/jetpack_off.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		ent->flags |= FL_JETPACK;

		gi.dprintf( "SOUND TODO: JetPack enabled\n" );
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/jetpack_on.wav"), 1, ATTN_NORM, 0);
	}
}

qboolean Pickup_JetPack (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		// auto-use for DM only if we didn't already have one
		if (!quantity)
			ent->item->use (other, ent->item);
	}

	other->client->jetpack_power = 2.5;

	return true;
}

// JOSEPH 12-MAR-99-B
qboolean Pickup_Health_Large (edict_t *ent, edict_t *other)
{
	
	if (other->health >= other->max_health)
		return false;

	other->health += 25;

	if (other->health > other->max_health)
			other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 30);

	return true;
}

qboolean Pickup_Health_Small (edict_t *ent, edict_t *other)
{
	if (other->health >= other->max_health)
		return false;

	other->health += 15;

	if (other->health > other->max_health)
			other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 30);

	return true;
}
// END JOSEPH


//======================================================================

/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;

	if (!(other->client))
		return;
	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->item->pickup)
		return;		// not a grabbable item?

	taken = ent->item->pickup(ent, other);

	// Do Episode specific handling
	EP_ItemPickup( ent, other );


	if (taken)
	{
		// flash the screen
		other->client->bonus_alpha = 0.25;	

		// JOSEPH 2-MAR-99
		if (!Q_stricmp(ent->item->pickup_name, "Pistol_Mods"))
		{
			if (ent->count == 1)
			{
				ent->item->icon = "/pics/h_pistol_rof.tga";
				ent->item->pickup_sound = "world/pawnomatic/mod_reload1.wav";
			}
			else if (ent->count == 2)
			{
				ent->item->icon = "/pics/h_pistol_rl.tga";
			   ent->item->pickup_sound = "world/pawnomatic/mod_reload2.wav";
			}
			else if (ent->count == 3)
			{
				ent->item->icon = "/pics/h_pistol_mag.tga";
			    ent->item->pickup_sound = "world/pawnomatic/mod_magnum2.wav";
			}	
		    gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
		}
		// END JOSEPH

		// show icon and name on status bar
		other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
		other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
		// JOSEPH 25-JAN-99
		other->client->pickup_msg_time = level.time + 5.5;
		// END JOSEPH
		// change selected item
		if (ent->item->use)
			other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);

		
	}

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets (ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

	if (!taken)
		return;

	if (!((coop->value) &&  (ent->item->flags & IT_STAY_COOP)) || (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
	{
		if (ent->flags & FL_RESPAWN)
			ent->flags &= ~FL_RESPAWN;
		else
			G_FreeEdict (ent);
	}
	
}

//======================================================================
//======================================================================

static void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

//	gi.bprintf(PRINT_HIGH,"calling drop_temp_touch\n");
	Touch_Item (ent, other, plane, surf);
}


//======================================================================
//======================================================================
void AssaultKeyThink (edict_t *ent) // ASSAULT2
{
	ent->touch = Touch_Item;
	ent->nextthink = level.time + 1;

	if(ent->misstime >= 60)
	{
		//if(!(ent->misstime%10))
		//	gi.bprintf (PRINT_HIGH, "Key will respawn to original position in %i seconds.\n",ent->misstime);
		if(ent->misstime%2)
		{
			//ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
	gi.positioned_sound(ent->s.origin, ent, CHAN_ITEM, gi.soundindex("misc/menu3.wav"), 1, 1, 0);

		}
		else
		{
			//ent->s.effects &= ~EF_COLOR_SHELL;
			ent->s.renderfx &= ~RF_SHELL_GREEN;
		}
	}

	else if(ent->misstime >= 10)
	{
		//if(!(ent->misstime%5))
		//	gi.bprintf (PRINT_HIGH, "Key will respawn to original position in %i seconds.\n",ent->misstime);
		if(ent->misstime%2)
		{
			//ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_BLUE;
			if(rand()%2) gi.positioned_sound(ent->s.origin, ent, CHAN_ITEM, gi.soundindex("world/arc.wav"), 1, 1, 0);
			else  gi.positioned_sound(ent->s.origin, ent, CHAN_ITEM, gi.soundindex("world/arc3.wav"), 1, 1, 0);

		}
		else
		{
			//ent->s.effects &= ~EF_COLOR_SHELL;
			ent->s.renderfx &= ~RF_SHELL_BLUE;
		}
	}

	else 
	{
		//gi.bprintf (PRINT_HIGH, "Key will respawn to original position in %i seconds.\n",ent->misstime);
		if(ent->misstime%2)
		{
			//ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_RED;
			if(rand()%2) gi.positioned_sound(ent->s.origin, ent, CHAN_ITEM, gi.soundindex("world/arc2.wav"), 1, 1, 0);
			else  gi.positioned_sound(ent->s.origin, ent, CHAN_ITEM, gi.soundindex("world/arc4.wav"), 1, 1, 0);
		}
		else
		{
			//ent->s.effects &= ~EF_COLOR_SHELL;
			ent->s.renderfx &= ~RF_SHELL_RED;
		}
	}

	ent->s.angles[YAW] += 6;

	///nobody picked me up so im going away back to my original coordinates
	if(!ent->misstime--) BecomeExplosion1(ent);
}


static void drop_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	if (deathmatch->value)
	{
		if( ent->style && 
			(   (strcmp (ent->classname, "key_key1")==0)
			  ||(strcmp (ent->classname, "key_key2")==0)
			  ||(strcmp (ent->classname, "key_key3")==0)      
			)
		  ) //only keys will get a style when dropped assault2
		{
			edict_t *guy;
			int q;
			ent->nextthink = level.time + 1;
			ent->think = AssaultKeyThink; //divert the key thinking
			for_everyone(guy,q)
				gi.cprintf( guy, PRINT_CHAT, ">> %s %s will respawn in %i secs.\n",team_names[ent->style],ent->classname,ent->misstime);
				//gi.bprintf (PRINT_HIGH, "%s %s will respawn to original position in %i seconds.\n",team_names[ent->style],ent->classname,ent->misstime);
		}
		else
		{
			ent->nextthink = level.time + 30;
			ent->think = G_FreeEdict;
		}
	}
	// Xatrix/Ridah/Navigator/17-mar-1998
	ent->nav_data.flags |= ND_STATIC;
	// END:		Xatrix/Ridah/Navigator/17-mar-1998

}


edict_t *Drop_Item (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	vec3_t	forward, right;
	vec3_t	offset;

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	// JOSEPH 15-MAR-99	
	dropped->s.renderfx2 = RF2_NOSHADOW; 	
	// END JOSEPH
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

	// JOSEPH 28-APR-99
	if (!(ent->svflags & SVF_PROP))
	{
	VectorScale (forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;
	}
	else
	{
		dropped->s.origin[2] += 8;
		dropped->velocity[2] = 200;		
	}
	// END JOSEPH

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

void Use_Item (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}

	gi.linkentity (ent);
}

//======================================================================

// JOSEPH 7-DEC-98
edict_t *Shot_Drop_Item (edict_t *ent, gitem_t *item, char* modeloverride)
{
	edict_t	*dropped;
	vec3_t	forward, right;
	vec3_t	offset;

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	
	if (modeloverride)
	{
		gi.setmodel (dropped, modeloverride);
	}
	else
	{	
		gi.setmodel (dropped, dropped->item->world_model);
    }

	//// ASSAULT2 Key thinkin
	if(((item == FindItem("Key_One"))||(item == FindItem("Key_Two"))||(item == FindItem("Key_Three"))) && (teamplay->value) && (ent->client->pers.team) )
	{
		dropped->style = ent->client->pers.team;
		dropped->option = 420; //This lets Pickup_Key know that this is not the original key or coordinates

		if(!skill->value)
			skill->value = 1;
		dropped->misstime = (15*skill->value);
	}
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

	VectorScale (forward, 250, dropped->velocity);
	
	if ((rand()&1) == 1)
	{
	    VectorScale (right, 150, dropped->velocity);
	}
	else
	{
	    VectorScale (right, -150, dropped->velocity);
	}

	dropped->velocity[2] = 350;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

/*
================
droptofloor
================
*/
void droptofloor (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		// RAFAEL
		if (strcmp (ent->classname, "foodcube") == 0)
		{
			VectorCopy (ent->s.origin, tr.endpos);
			ent->velocity[2] = 0;
		}
		// JOSEPH 7-MAY-99
		else if (strcmp(ent->classname, "key_fuse") == 0)
		{
			v = tv(-7,-7,-16);
			VectorCopy (v, ent->mins);
			v = tv(7, 7, 16);
			VectorCopy (v, ent->maxs);
			ent->movetype = MOVETYPE_NONE; 
			ent->velocity[2] = 0;
			ent->s.origin[2] += 1;
			goto SkipFall;
		}
		// END JOSEPH
		else
		{
			gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			G_FreeEdict (ent);
			return;
		}
	}

	VectorCopy (tr.endpos, ent->s.origin);

// JOSEPH 7-MAY-99
SkipFall:
// END JOSEPH

	if (ent->team)
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;

		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		if (ent == ent->teammaster)
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if (ent->spawnflags & ITEM_TRIGGER_SPAWN)
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->use = Use_Item;
	}

	gi.linkentity (ent);
}


/*
===============
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
===============
*/
void PrecacheItem (gitem_t *it)
{
	char	*s, *start;
	char	data[MAX_QPATH];
	int		len;
	gitem_t	*ammo;

	if (!it)
		return;

	if (it->pickup_sound)
		gi.soundindex (it->pickup_sound);
	if (it->world_model)
		gi.modelindex (it->world_model);
	if (it->view_model)
		gi.modelindex (it->view_model);
	if (it->icon)
		gi.imageindex (it->icon);

	// parse everything for its ammo
	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem (it->ammo);
		if (ammo != it)
			PrecacheItem (ammo);
	}

	// parse the space seperated precache string for other items
	s = it->precaches;
	if (!s || !s[0])
		return;

	while (*s)
	{
		start = s;
		while (*s && *s != ' ')
			s++;

		len = s-start;
		if (len >= MAX_QPATH || len < 5)
			gi.error ("PrecacheItem: %s has bad precache string", it->classname);
		memcpy (data, start, len);
		data[len] = 0;
		if (*s)
			s++;

		// determine type based on extension
		if (!strcmp(data+len-3, "md2"))
			gi.modelindex (data);
		
		// RAFAEL
		// weapon mdx
		else if (!strcmp (data+len-3, "mdx"))
		{
			gi.modelindex (data);
		}
		else if (!strcmp(data+len-3, "sp2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "wav"))
			gi.soundindex (data);
		if (!strcmp(data+len-3, "pcx"))
			gi.imageindex (data);
	}
}

/*
============
SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void SpawnItem (edict_t *ent, gitem_t *item)
{
	if (!item)
		return;

	PrecacheItem (item);

	if (ent->spawnflags)
	{
		if (strcmp(ent->classname, "key_fuse") != 0)
		{
			ent->spawnflags = 0;
			gi.dprintf("%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
		}
	}

	// some items will be prevented in deathmatch
	if (deathmatch->value)
	{
		if ( (int)dmflags->value & DF_NO_ARMOR )
		{
			if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor)
			{
				G_FreeEdict (ent);
				return;
			}
		}
/*
		if ( (int)dmflags->value & DF_NO_ITEMS )
		{
			if (item->pickup == Pickup_Powerup)
			{
				G_FreeEdict (ent);
				return;
			}
		}
*/
		if ( (int)dmflags->value & DF_NO_HEALTH )
		{
			if (item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline || item->pickup == Pickup_AncientHead 
				|| item->pickup == Pickup_Health_Large || item->pickup == Pickup_Health_Small)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value )
		{
			if ( (item->flags == IT_AMMO) || (strcmp(ent->classname, "ammo_cylinder") == 0) )
			{
				G_FreeEdict (ent);
				return;
			}
		}

		// Ridah, realmode only has pistol, tommy and shotgun
		if (deathmatch->value && dm_realmode->value && item->classname)
		{
			if (	(item->pickup == Pickup_Weapon)
//				&&	(strstr(item->classname, "weapon") == item->classname)
				&&	(item->ammo && strcmp(item->ammo, "Bullets") && strcmp(item->ammo, "Shells")))
			{
				int	y;

				y = ((int)item->classname[7]) % 3;

				// select another valid weapon to use here
				if (y == 0)
				{
					item = FindItem("Shotgun");
				}
				else if (y == 1)
				{
					item = FindItem("Tommygun");
				}
				else
				{
					item = FindItem("Pistol_Mods");
					ent->model = "models/pu_icon/magmod/tris.md2";
					ent->count = 3;	//magnum mod
				}

			}
			else if (	(item->pickup == Pickup_Ammo)
					&&	((item->tag != AMMO_BULLETS) && (item->tag != AMMO_SHELLS)))
			{
				// select another valid weapon to use here
				if (((int)item->classname[5]) % 2)
				{
					item = FindItem("Bullets");
				}
				else
				{
					item = FindItem("Shells");
				}
			}


			// no armor
			if (item->classname && strstr(item->classname, "item_armor") == item->classname)
			{
				G_FreeEdict (ent);
				return;
			}
/*
			// no health
			if (strstr(item->classname, "item_health") == item->classname)
			{
				G_FreeEdict (ent);
				return;
			}
*/
		}
	}
			
// Ridah, no coop
/*
	if (coop->value && (strcmp(ent->classname, "key_power_cube") == 0))
	{
		ent->spawnflags |= (1 << (8 + level.power_cubes));
		level.power_cubes++;
	}

	// don't let them drop items that stay in a coop game
	if ((coop->value) && (item->flags & IT_STAY_COOP))
	{
		item->drop = NULL;
	}
*/
	// JOSEPH 22-JAN-99
	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	ent->s.renderfx2 = RF2_NOSHADOW; 
	// END JOSEPH
	
	
	if (!(strcmp(ent->classname, "item_lizzyhead")))
	{
		int i;
		memset(&(ent->s.model_parts[0]), 0, sizeof(model_part_t) * MAX_MODEL_PARTS);
		ent->s.model_parts[PART_HEAD].modelindex = gi.modelindex("models/pu_icon/head/head.mdx");
		ent->s.num_parts++;
		for (i=0; i<MAX_MODELPART_OBJECTS; i++)
			ent->s.model_parts[PART_HEAD].baseskin = ent->s.model_parts[PART_HEAD].skinnum[i] = 0;
		gi.GetObjectBounds( "models/pu_icon/head/head.mdx", &ent->s.model_parts[PART_HEAD] );
	}	
	else if (ent->model)
		gi.modelindex (ent->model);

	// Xatrix/Ridah/Navigator/17-mar-1998
	ent->nav_data.flags |= ND_STATIC;
	// END:		Xatrix/Ridah/Navigator/17-mar-1998

}

// JOSEPH 13-FEB-99
void Use_Null (edict_t *ent, gitem_t *item)
{

}
// END JOSEPH

//======================================================================

// JOSEPH 21-APR-99
gitem_t	itemlist[] = 
{
	{
		NULL
	},	// leave index 0 alone

	//
	// ARMOR
	//


// JOSEPH 19-APR-99
/*QUAKED item_armor_helmet (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/armor_head/armor_head.md2" 
*/
	{
		"item_armor_helmet", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/armor_head/armor_head.md2", 0,
		NULL,
/* icon */		"/pics/h_helmet_lt.tga",
/* pickup */	"Helmet Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		&bodyarmor_info,
		ARMOR_COMBAT,
/* precache */ ""
	},

/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/armor/tris.md2" 
*/
	{
		"item_armor_jacket", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/armor/tris.md2", 0,
		NULL,
/* icon */		"/pics/h_chest_lt.tga",
/* pickup */	"Jacket Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		&combatarmor_info,
		ARMOR_JACKET,
/* precache */ ""
	},

/*QUAKED item_armor_legs (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/armor_lo/armor_lo.md2" 
*/
	{
		"item_armor_legs", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/armor_lo/armor_lo.md2", 0,
		NULL,
/* icon */		"/pics/h_pants_lt.tga",
/* pickup */	"Legs Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		&jacketarmor_info,
		ARMOR_BODY,
/* precache */ ""
	},


/*QUAKED item_armor_helmet_heavy (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/armor_head/armor_head_hd.md2"
*/
	{
		"item_armor_helmet_heavy", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/armor_head/armor_head_hd.md2", 0,
		NULL,
/* icon */		"/pics/h_helmet_hvy.tga",
/* pickup */	"Helmet Armor Heavy",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		&bodyarmor_info,
		ARMOR_COMBAT,
/* precache */ ""
	},

/*QUAKED item_armor_jacket_heavy (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/armor/armorhdtop.md2"
*/
	{
		"item_armor_jacket_heavy", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/armor/armorhdtop.md2", 0,
		NULL,
/* icon */		"/pics/h_chest_hvy.tga",
/* pickup */	"Jacket Armor Heavy",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		&combatarmor_info,
		ARMOR_JACKET,
/* precache */ ""
	},

/*QUAKED item_armor_legs_heavy (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/armor_lo/armor_lo_hd.md2"
*/
	{
		"item_armor_legs_heavy", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/armor_lo/armor_lo_hd.md2", 0,
		NULL,
/* icon */		"/pics/h_pants_hvy.tga",
/* pickup */	"Legs Armor Heavy",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		&jacketarmor_info,
		ARMOR_BODY,
/* precache */ ""
	},

	//
	// WEAPONS 
	//

/* weapon_blackjack (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blackjack", 
		NULL,
		Use_Weapon2,
		NULL,
		Weapon_Blackjack,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/blakjak/blakjak.mdx",
/* icon */		"h_pipe",
/* pickup */	"Pipe",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/melee/swing.wav" 
	},

// JOSEPH 14-APR-99
/*QUAKED weapon_crowbar (.3 .3 1) (-16 -16 -16) (16 16 16)

model="models/weapons/g_crowbar/tris.md2"  
*/
	{
		"weapon_crowbar", 
		Pickup_Weapon,
		Use_Weapon2,
		Drop_Weapon,
		Weapon_Crowbar,
		"misc/w_pkup.wav",
		"models/weapons/g_crowbar/tris.md2", EF_REFL,
		"models/weapons/crowbar/crowbar.mdx",
/* icon */		"/pics/h_cbar.tga",
/* pickup */	"Crowbar",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/melee/swing.wav" 
	},
// END JOSEPH
	
/*QUAKED weapon_pistol (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/weapons/g_pistol/tris.md2"  
*/
	{
		"weapon_pistol", 
		Pickup_Weapon,
		Use_Weapon2,
		Drop_Weapon,
		Weapon_Pistol,
		"misc/w_pkup.wav",
		// note to Jason 
		// we need a pistol pickup icon
		"models/weapons/g_pistol/tris.md2", EF_REFL,
		"models/weapons/v_colt/pistol.mdx",
/* icon */		"/pics/h_pistol.tga",
/* pickup */	"Pistol",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ ""
	},

/* weapon_pistol_s (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/weapons/silencer_mdx/pistol.mdx"
*/
	{
		"weapon_spistol", 
		Pickup_Weapon,
		Use_Weapon2,
		Drop_Weapon,
		Weapon_SPistol,
		"misc/w_pkup.wav",
		// note to Jason 
		// we need a pistol pickup icon
		"models/weapons/silencer_mdx/pistol.mdx", EF_REFL,
		"models/weapons/silencer_mdx/pistol.mdx",
/* icon */		"/pics/h_silencer.tga",
/* pickup */	"SPistol",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_SILENCER,
		NULL,
		0,
/* precache */ "weapons/pistol/silencer.wav weapons/pistol/silencerattatch.wav"
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/weapons/g_shotgun/tris.md2"  
*/
	{
		"weapon_shotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Shotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotgun/tris.md2", EF_REFL,
		"models/weapons/shotgun/shotgun.mdx",
/* icon */		"/pics/h_shotgun.tga",
/* pickup */	"Shotgun",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/shotgun/shotgf1b.wav weapons/shotgun/shotgr1b.wav"
	},

/*QUAKED weapon_tommygun (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/weapons/g_tomgun/tris.md2"  
*/
	{
		"weapon_tommygun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Tommygun,
		"misc/w_pkup.wav",
		"models/weapons/g_tomgun/tris.md2", EF_REFL,
		"models/weapons/v_tomgun/tomgun.mdx",
/* icon */		"/pics/h_tgun.tga",
/* pickup */	"Tommygun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/machinegun/machgf1b.wav"
	},

// JOSEPH 16-APR-99
/*QUAKED weapon_heavymachinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/weapons/g_hmg/tris.md2"  
*/
{
		"weapon_heavymachinegun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Barmachinegun,
		"misc/w_pkup.wav",
		"models/weapons/g_hmg/tris.md2", EF_REFL,
//		"models/weapons/g_msg90/tris.md2", EF_REFL, //assault2
		"models/weapons/v_hmg/hmg.mdx",
//		"models/weapons/v_msg90/msg90.mdx",
/* icon *///		"/pics/kpl/kpl_icon.tga", //h_hmg
		"/pics/h_hmg.tga",
/* pickup */	"Heavy machinegun",
		0,
		1,
		"308cal",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/weapons/g_grenade_launcher/tris.md2"    
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_GrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_grenade_launcher/tris.md2", EF_REFL,
		"models/weapons/v_grenade_launcher/grenade_launcher.mdx",
/* icon */		"/pics/h_grenade_l.tga",
/* pickup */	"Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ ""
	},

// JOSEPH 28-APR-99
/*QUAKED weapon_bazooka (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/weapons/g_rocket_launcher/tris.md2"  
*/
	{
		"weapon_bazooka",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket_launcher/tris.md2", EF_REFL,
		"models/weapons/v_rocket_launcher/rlaunch.mdx",
/* icon */		"/pics/h_bazooka.tga",
/* pickup */	"Bazooka",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "models/props/glass/glass2.md2"
	},
// END JOSEPH

/*QUAKED weapon_flamethrower (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/flame_shell/tris.md2"    
*/
	{
		"weapon_flamethrower", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_FlameThrower,
		"misc/w_pkup.wav",
		//"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/pu_icon/flame_shell/tris.md2", EF_REFL,
		"models/weapons/v_flamegun/flamegun.mdx",
/* icon */		"/pics/h_flamethrower.tga",
/* pickup */	"FlameThrower",
		0,
		2,
		"Gas",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

	{
		"weapon_mortar_e", //assault2
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Mortar,
		"misc/w_pkup.wav",
		"models/weapons/e_mortar/tris.md2", EF_REFL,
		"models/weapons/v_mortar/detonator.mdx",
/* icon */		"/pics/kpl/kpl_icon.tga",
/* pickup */	"Mortar",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "nam/tank_gun.wav"
	},
	//
	// AMMO ITEMS
	//

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/grenade/tris.md2"  
*/
	{
		"ammo_grenades",
		Pickup_Ammo,
		Use_Null,
		Drop_Ammo,
		NULL,
		"world/pickups/ammo.wav",
		"models/pu_icon/grenade/tris.md2", 0,
        NULL,
/* icon */		"/pics/h_grenade.tga",
/* pickup */	"Grenades",
/* width */		3,
		3,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_GRENADES,
/* precache */ ""
	},

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/shotgun_shell/tris.md2"  
*/
	{
		"ammo_shells",
		Pickup_Ammo,
		Use_Null,
		Drop_Ammo,
		NULL,
		"world/pickups/ammo.wav",
		"models/pu_icon/shotgun_shell/tris.md2", 0,		
		NULL,
/* icon */		"/pics/h_shots.tga",
/* pickup */	"Shells",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

// JOSEPH 3-MAR-99
/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/pclip/tris.md2"  
*/
	{
		"ammo_bullets",
		Pickup_Ammo,
		Use_Null,
		Drop_Ammo,
		NULL,
		"world/pickups/ammo.wav",
		"models/pu_icon/pclip/tris.md2", 0,
		NULL,
/* icon */		"/pics/h_bullet.tga",
/* pickup */	"Bullets",
/* width */		3,
		20,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},
// END JOSEPH


// JOSEPH 1-APR-99
/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/rocket/tris.md2" 
*/
	{
		"ammo_rockets",
		Pickup_Ammo,
		Use_Null,
		Drop_Ammo,
		NULL,
		"world/pickups/ammo.wav",
		"models/pu_icon/rocket/tris.md2", 0,
		NULL,
/* icon */		"/pics/h_missle.tga",
/* pickup */	"Rockets",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},
// END JOSEPH

// JOSEPH 28-APR-99
/*QUAKED ammo_308 (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/hmgclip/tris.md2" 
*/
	{
		"ammo_308",
		Pickup_Ammo,
		Use_Null,
		Drop_Ammo,
		NULL,
		"misc/w_pkup.wav",
		"models/pu_icon/hmgclip/tris.md2", 0,
		NULL,
/* icon */		"/pics/h_hmg_clip.tga",
/* pickup */	"308cal",
/* width */		3,
		30,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SLUGS,
/* precache */ ""
	},
// END JOSEPH

/*QUAKED ammo_cylinder (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/tgclip/tris.md2"  
*/
	{
		"ammo_cylinder",
		Pickup_Ammo_Cylinder,
		Use_Null,
		Drop_Ammo,
		NULL,
		"world/pickups/ammo.wav",
		"models/pu_icon/tgclip/tris.md2", 0,
		NULL,
/* icon */		"/pics/h_tclip.tga",
/* pickup */	"Cylinder",
/* width */		3,
		50,
		NULL,
		IT_AMMO|IT_NOCHEATS,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

/*QUAKED ammo_flametank (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/flametank/tris.md2"  
*/
	{
		"ammo_flametank",
		Pickup_Ammo,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/ammo.wav",
		"models/pu_icon/flametank/tris.md2", 0,
		NULL,
		"/pics/h_ftank.tga",
		"Gas",
		2,
		50,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},		

	//
	// KINGPIN items
	//

	
// JOSEPH 22-FEB-99	
/*QUAKED item_coil (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/coil/tris.md2"  
*/
	{
		"item_coil",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/coil.wav",
		"models/pu_icon/coil/tris.md2", 0,
		NULL,
		"/pics/h_coil.tga",
		"Coil",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

// JOSEPH 5-JUN-99
/*QUAKED item_lizzyhead (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/head/tris.md2"  
*/
	{
		"item_lizzyhead",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/lizzyhead.wav",
		"models/pu_icon/head/head.mdx", 0,
		NULL,
		"/pics/h_lizzyhead.tga",
		"Lizzy Head",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

/*QUAKED item_cashroll (.3 .3 1) (-16 -16 -16) (16 16 16)
currentcash - set amount of cash in roll  
model="models/pu_icon/cash/tris.md2"  
*/
	{
		"item_cashroll",
		Pickup_Cash,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/cash.wav",
		"models/pu_icon/cash/tris.md2", 0,
		NULL,
		// JOSEPH 26-JAN-99-C
		"/pics/hh_money.tga",
		"Cash",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},	

// JOSEPH 2-MAR-99
/*QUAKED item_cashbaglarge (.3 .3 1) (-16 -16 -16) (16 16 16)
currentcash - set amount of cash in the bag  
model="models/pu_icon/money/money_lg.md2"  
*/
	{
		"item_cashbaglarge",
		Pickup_Cash,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/cash.wav",
		"models/pu_icon/money/money_lg.md2", 0,
		NULL,
		// JOSEPH 26-JAN-99-C
		"/pics/hh_money.tga",
		"Large Cash Bag",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},	

/*QUAKED item_cashbagsmall (.3 .3 1) (-16 -16 -16) (16 16 16)
currentcash - set amount of cash in the bag  
model="models/pu_icon/money/money_sm.md2"  
*/
	{
		"item_cashbagsmall",
		Pickup_Cash,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/cash.wav",
		"models/pu_icon/money/money_sm.md2", 0,
		NULL,
		// JOSEPH 26-JAN-99-C
		"/pics/hh_money.tga",
		"Small Cash Bag",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},	
// END JOSEPH
	

/*QUAKED item_battery (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/battery/tris.md2"  
*/
	{
		"item_battery",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/battery/tris.md2", 0,
		NULL,
		"/pics/h_bat.tga",
		"Battery",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},		


// JOSEPH 2-MAR-99
/*QUAKED item_jetpack (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/items/misc/jetpack/jetpack.mdx"
*/
	{
		"item_jetpack",
		Pickup_JetPack,
		Use_JetPack,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/items/misc/jetpack/jetpack.mdx", 0,
		NULL,
/* icon */		"/pics/h_jetpack.tga",
		"Jet Pack",
		0,
		0,
		NULL,
		IT_STAY_COOP|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},			
// END JOSEPH

// JOSEPH 10-FEB-99
/*QUAKED item_health_sm (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/health_s/tris.md2"  
*/
	{
		"item_health_sm",
		Pickup_Health_Small,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/health.wav",
		"models/pu_icon/health_s/tris.md2", 0,
		NULL,
		"/pics/h_smed.tga",
		"Small Health",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},			
	
/*QUAKED item_health_lg (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/health/tris.md2"  
*/
	{
		"item_health_lg",
		Pickup_Health_Large,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/health.wav",
		"models/pu_icon/health/tris.md2", 0,
		NULL,
		"/pics/h_lmed.tga",
		"Large Health",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},		
// END JOSEPH


/*QUAKED item_flashlight (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/f_light/tris.md2"  
*/
	{
		"item_flashlight",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/f_light/tris.md2", 0,
		NULL,
		"/pics/h_flashl.tga",
		"Flashlight",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_FLASHLIGHT,
		NULL,
		0,
/* precache */ ""
	},		

// JOSEPH 12-MAR-99	
/*QUAKED item_watch (.3 .3 1) (-16 -16 -16) (16 16 16)

Watch for Lenny
  
model="models/pu_icon/watch/tris.md2"  
*/
	{
		"item_watch",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/watch/tris.md2", 0,
		NULL,
		"/pics/h_watch.tga",
		"Watch",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

// JOSEPH 24-MAR-99	
/*QUAKED item_whiskey (.3 .3 1) (-16 -16 -16) (16 16 16)

model="models/pu_icon/whiskey/tris.md2"  

Whiskey for the bum
  
*/
	{
		"item_whiskey",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/whiskey/tris.md2", 0,
		NULL,
		"/pics/h_bottle.tga",
		"Whiskey",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

// JOSEPH 30-MAY-99
/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
model="models/pu_icon/backpack/tris.md2"
*/
	{
		"item_pack",
		Pickup_Pack,
		NULL,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/backpack/tris.md2", 0,
		NULL,
/* icon */		"/pics/h_backpack.tga",
/* pickup */	"Ammo Pack",
/* width */		2,
		180,
		NULL,
		IT_NOCHEATS|IT_FLASHLIGHT,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

// JOSEPH 29-MAY-99
/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
gives +1 to maximum health
model="models/pu_icon/adrenaline/tris.md2"
*/
	{
		"item_adrenaline",
		Pickup_Adrenaline,
		NULL,
		NULL,
		NULL,
		"world/pickups/health.wav",
		"models/pu_icon/adrenaline/tris.md2", 0,
		NULL,
		"/pics/h_smed.tga",
/* pickup */	"Adrenaline",
/* width */		2,
		60,
		NULL,
		IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

// JOSEPH 7-MAY-99
/*QUAKED key_fuse (0 .5 .8) (-8 -8 -16) (8 8 16) TRIGGER_SPAWN NO_TOUCH
*/
	{
		"key_fuse",
		Pickup_Key,
		NULL,
		NULL,
		NULL,
		"world/pickups/coil.wav",
		"models/pu_icon/fuse/tris.md2", 0,
		NULL,
		"/pics/h_fuse.tga",
		"Fuse",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

// JOSEPH 4-MAY-99
/*QUAKED item_safedocs (.3 .3 1) (-16 -16 -16) (16 16 16)
Safe docs for the louie in PV
model="models/pu_icon/folder/tris.md2"  
*/
	{
		"item_safedocs",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/folder/tris.md2", 0,
		NULL,
		"/pics/h_safedocs.tga",
		"Safe docs",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

/*QUAKED item_valve (.3 .3 1) (-16 -16 -16) (16 16 16)
A valve dial turn key
model="models/pu_icon/valve/tris.md2" 
*/
	{
		"item_valve",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/valve/tris.md2", 0,
		NULL,
		"/pics/h_valve.tga",
		"Valve",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

// JOSEPH 25-MAY-99
/*QUAKED item_oilcan (.3 .3 1) (-16 -16 -16) (16 16 16)
An oil can
model="models/pu_icon/oilcan/tris.md2" 
*/
	{
		"item_oilcan",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
		"world/pickups/generic.wav",
		"models/pu_icon/oilcan/tris.md2", 0,
		NULL,
		"/pics/h_oilcan.tga",
		"Oil Can",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

	// Kingpin keys


// JOSEPH 16-MAR-99

/*QUAKED key_key1 (0 .5 .8) (-16 -16 -16) (16 16 16)
key 1
model="models/pu_icon/key/key_a.md2"  
*/
	{
		"key_key1",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
        "world/pickups/keys.wav",
		"models/pu_icon/key/key_a.md2", 0,
		NULL,
		"/pics/h_key.tga",
		"Key_One", /* ASSAULT2 "StoreRoomKey" */
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_key2 (0 .5 .8) (-16 -16 -16) (16 16 16)
key 2
model="models/pu_icon/key/key_b.md2" 
*/
	{
		"key_key2",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
        "world/pickups/keys.wav",
		"models/pu_icon/key/key_b.md2", 0,
		NULL,
		"/pics/h_key.tga",
		"Key_Two", /* ASSAULT2 "Electrical_Room" */
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_key3 (0 .5 .8) (-16 -16 -16) (16 16 16)
key 3
model="models/pu_icon/key/key_c.md2" 
*/
	{
		"key_key3",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
        "world/pickups/keys.wav",
		"models/pu_icon/key/key_c.md2", 0,
		NULL,
		"/pics/h_key.tga",
		"Key_Three", /* ASSAULT2 "Chem_Plant_Key" */
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_key4 (0 .5 .8) (-16 -16 -16) (16 16 16)
key 4
model="models/pu_icon/key/key_c.md2" 
*/
	{
		"key_key4",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
        "world/pickups/keys.wav",
		"models/pu_icon/key/key_c.md2", 0,
		NULL,
		"/pics/h_key.tga",
		"Bridge_Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_key5 (0 .5 .8) (-16 -16 -16) (16 16 16)
key 5
model="models/pu_icon/key/key_c.md2" 
*/
	{
		"key_key5",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
        "world/pickups/keys.wav",
		"models/pu_icon/key/key_c.md2", 0,
		NULL,
		"/pics/h_key.tga",
		"Shipyard_Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_key6 (0 .5 .8) (-16 -16 -16) (16 16 16)
key 6
model="models/pu_icon/key/key_c.md2" 
*/
	{
		"key_key6",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
        "world/pickups/keys.wav",
		"models/pu_icon/key/key_c.md2", 0,
		NULL,
		"/pics/h_key.tga",
		"Warehouse_Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_key7 (0 .5 .8) (-16 -16 -16) (16 16 16)
key 7
model="models/pu_icon/key/key_c.md2" 
*/
	{
		"key_key7",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
        "world/pickups/keys.wav",
		"models/pu_icon/key/key_c.md2", 0,
		NULL,
		"/pics/h_key.tga",
		"Shop_Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},

// JOSEPH 6-JUN-99
/*QUAKED key_key8 (0 .5 .8) (-16 -16 -16) (16 16 16)
key 8
model="models/pu_icon/key/key_c.md2" 
*/
	{
		"key_key8",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
        "world/pickups/keys.wav",
		"models/pu_icon/key/key_c.md2", 0,
		NULL,
		"/pics/h_ticket.tga",
		"Ticket",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH

/*QUAKED key_key9 (0 .5 .8) (-16 -16 -16) (16 16 16)
key 9
model="models/pu_icon/key/key_c.md2" 
*/
	{
		"key_key9",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
        "world/pickups/keys.wav",
		"models/pu_icon/key/key_c.md2", 0,
		NULL,
		"/pics/h_key.tga",
		"Office_Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_key10 (0 .5 .8) (-16 -16 -16) (16 16 16)
key 10
model="models/pu_icon/key/key_c.md2" 
*/
	{
		"key_key10",
		Pickup_Key,
		Use_Null,
		NULL,
		NULL,
        "world/pickups/keys.wav",
		"models/pu_icon/key/key_c.md2", 0,
		NULL,
		"/pics/h_key.tga",
		"Key10",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
// END JOSEPH


	{
		NULL,
		Pickup_Pistol_Mods,
		NULL,
		NULL,
		NULL,
		"misc/w_pkup.wav",
		NULL, 0,
		NULL,
/* icon */		NULL,//"i_pistol_mods",
/* pickup */	"Pistol_Mods",
/* width */		3,
		0,
		NULL,
		IT_NOCHEATS,
		NULL,
		0,
/* precache */ ""
	},
////////////////////////////assault2 addons
	{
		"weapon_msg90",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Msg90,
		"misc/w_pkup.wav",
		"models/weapons/g_msg90/tris.md2", EF_REFL,//EF_ROTATE,
		NULL,
		"/pics/h_hmg.tga",	/* icon */
		"Msg90",					/* pickup name */
		3,							/* width */
		1,
		"308cal",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,							/* precache */ 
		"weapons/msg90/msg90_reload3.wav weapons/msg90/msg90_reload4.wav"
	},
	{
		"weapon_mortar", //assault2
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Mortar,
		"misc/w_pkup.wav",
		"models/weapons/g_mortar/mortar.mdx", EF_REFL,//EF_ROTATE,
		NULL,
		"/pics/kpl/kpl_icon.tga",	/* icon */
		"Mortar",					/* pickup name */
		3,							/* width */
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,							/* precache */ 
		""
	},

	{
		"weapon_spaz", /* assault2 virgin16 */
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Spaz,
		"misc/w_pkup.wav",
		"models/weapons/g_shotgun/spaz.md2", EF_REFL,
		"models/weapons/spaz/spaz.mdx",
/* icon */		"/pics/h_spaz.tga",
/* pickup */	"Spaz",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/spaz/spaz.wav"
	},

	{
		"weapon_pump", /* assault2 virgin17 */
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Spaz, /* this does double duty for pump */
		"misc/w_pkup.wav",
		"models/weapons/g_shotgun/pump.md2", EF_REFL,
		"models/weapons/pump/pump.mdx",
/* icon */		"/pics/h_pump.tga",
/* pickup */	"Pump",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/pump/pump.wav"
	},

	{
		"weapon_ak47", /* assault2 virgin23 */
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Ak47,
		"weapons/ak47/ak47pu3.wav",
		"models/weapons/ak47/g_ak47.md2", EF_REFL,
		"models/weapons/ak47/ak47.mdx",
/* icon */		"/pics/h_tgun.tga",
/* pickup */	"Ak47",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/ak47/machgf1b.wav"
	},

	// end of list marker
	{NULL}
};

////assault2
void mortar_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	static float	last_touch_time;
	static edict_t	*last_touch_ent;
	static int		last_touch_count = 0;

	if (!other->client)
		return;

	if ((level.time < last_touch_time) || (last_touch_time && (last_touch_time < (level.time - 2.0))) || (last_touch_ent && (last_touch_ent != other)))
	{	// reset
		last_touch_time = 0;
		last_touch_ent = NULL;
		last_touch_count = 0;
	}
	else if (last_touch_time > (level.time - 0.1))	return;

	else
	{
		last_touch_count++;
		last_touch_time = level.time;
		if (last_touch_count > 10)
		{
			// let them go away on their own terms
			T_Damage( other, other, other, vec3_origin, other->s.origin, vec3_origin, 9999, 0, 0, MOD_SAFECAMPER );
			last_touch_count = 0;
		}
	}

	last_touch_ent = other;
	if (self->timestamp > (level.time - 1.0)) return;
	self->timestamp = level.time;
/*
	// depositing, or withdrawing?
	if (other->client->pers.team == self->style)
	{	// deposit
		
	}
	else if (team_cash[self->style] > 0)
	{	// withdrawal

	}
*/
}


void mortar_think(edict_t *self)
{
	int	i;
	edict_t *trav;
	qboolean	noenemies = true;
	#define	SAFE_CLOSE_DIST		128
	#define	MAX_TIMEATSAFE		8.0

	// first, check if we have any unwanted enemies around, if so, don't count
	for (i=0; i<maxclients->value; i++)
	{
		trav = &g_edicts[i+1];
		if (!trav->inuse || !trav->client)	continue;
		if (trav->health <= 0)	continue;
		if (!trav->client->pers.team || (trav->client->pers.team == self->style)) continue;
		if (VectorDistance( self->s.origin, trav->s.origin ) > 512)	continue;
		if (!gi.inPVS( self->s.origin, trav->s.origin )) continue;
		noenemies = false;
	}

	for (i=0; i<maxclients->value; i++)
	{
		trav = &g_edicts[i+1];
		if (!trav->inuse || !trav->client)	continue;
		if (trav->health <= 0)	continue;
		if (!trav->client->pers.team || (trav->client->pers.team != self->style))	continue;
		if (noenemies)
		{
			if (VectorDistance( self->s.origin, trav->s.origin ) > SAFE_CLOSE_DIST)
			{
				trav->client->pers.timeatsafe -= 0.2;
				if (trav->client->pers.timeatsafe < 0)	trav->client->pers.timeatsafe = 0;
			}
			else
			{
				trav->client->pers.timeatsafe += 0.2;
				if (trav->client->pers.timeatsafe > MAX_TIMEATSAFE)	trav->client->pers.timeatsafe = MAX_TIMEATSAFE;
			}

			if (trav->client->pers.timeatsafe >= MAX_TIMEATSAFE)
				trav->client->pers.friendly_vulnerable = true;
			else
				trav->client->pers.friendly_vulnerable = false;

		}
		else	// turn off vulnerability, there is an enemy in range
			trav->client->pers.friendly_vulnerable = false;
	}
	self->nextthink = level.time + 0.2;
}


void SP_weapon_mortar (edict_t *self)
{
	if (self->style && (self->style < 1 || self->style > 2))
	{
		gi.dprintf( "weapon_msg90 has invalid \"style\" at %s, should be 1 or 2.\n", vtos(self->s.origin));
		G_FreeEdict( self );
		return;
	}

	self->model = "models/weapons/g_mortar/mortar.mdx";
	SpawnItem (self, FindItem ("Mortar"));
	self->item->icon = "/pics/kpl/kpl_icon.tga";
	self->s.modelindex = gi.modelindex("models/weapons/g_mortar/mortar.mdx");
	VectorSet( self->mins, -32, -16, -32 );
	VectorSet( self->maxs,  40,  20, -24 );
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_TRIGGER;
	gi.linkentity( self );
	self->touch = mortar_touch;
	self->think = mortar_think;
	self->nextthink = level.time + 2;
}

void msg90_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	static float	last_touch_time;
	static edict_t	*last_touch_ent;
	static int		last_touch_count = 0;

	if (!other->client)
		return;

	if ((level.time < last_touch_time) || (last_touch_time && (last_touch_time < (level.time - 2.0))) || (last_touch_ent && (last_touch_ent != other)))
	{	// reset
		last_touch_time = 0;
		last_touch_ent = NULL;
		last_touch_count = 0;
	}
	else if (last_touch_time > (level.time - 0.1))	return;

	else
	{
		last_touch_count++;
		last_touch_time = level.time;
		if (last_touch_count > 10)
		{
			// let them go away on their own terms
			T_Damage( other, other, other, vec3_origin, other->s.origin, vec3_origin, 9999, 0, 0, MOD_SAFECAMPER );
			last_touch_count = 0;
		}
	}

	last_touch_ent = other;
	if (self->timestamp > (level.time - 1.0)) return;
	self->timestamp = level.time;
/*
	// depositing, or withdrawing?
	if (other->client->pers.team == self->style)
	{	// deposit
		
	}
	else if (team_cash[self->style] > 0)
	{	// withdrawal

	}
*/
}


void msg90_think(edict_t *self)
{
	int	i;
	edict_t *trav;
	qboolean	noenemies = true;
	#define	SAFE_CLOSE_DIST		128
	#define	MAX_TIMEATSAFE		8.0

	// first, check if we have any unwanted enemies around, if so, don't count
	for (i=0; i<maxclients->value; i++)
	{
		trav = &g_edicts[i+1];
		if (!trav->inuse || !trav->client)	continue;
		if (trav->health <= 0)	continue;
		if (!trav->client->pers.team || (trav->client->pers.team == self->style)) continue;
		if (VectorDistance( self->s.origin, trav->s.origin ) > 512)	continue;
		if (!gi.inPVS( self->s.origin, trav->s.origin )) continue;
		noenemies = false;
	}

	for (i=0; i<maxclients->value; i++)
	{
		trav = &g_edicts[i+1];
		if (!trav->inuse || !trav->client)	continue;
		if (trav->health <= 0)	continue;
		if (!trav->client->pers.team || (trav->client->pers.team != self->style))	continue;
		if (noenemies)
		{
			if (VectorDistance( self->s.origin, trav->s.origin ) > SAFE_CLOSE_DIST)
			{
				trav->client->pers.timeatsafe -= 0.2;
				if (trav->client->pers.timeatsafe < 0)	trav->client->pers.timeatsafe = 0;
			}
			else
			{
				trav->client->pers.timeatsafe += 0.2;
				if (trav->client->pers.timeatsafe > MAX_TIMEATSAFE)	trav->client->pers.timeatsafe = MAX_TIMEATSAFE;
			}

			if (trav->client->pers.timeatsafe >= MAX_TIMEATSAFE)
				trav->client->pers.friendly_vulnerable = true;
			else
				trav->client->pers.friendly_vulnerable = false;

		}
		else	// turn off vulnerability, there is an enemy in range
			trav->client->pers.friendly_vulnerable = false;
	}
	self->nextthink = level.time + 0.2;
}


void SP_weapon_msg90 (edict_t *self)
{
	if (self->style && (self->style < 1 || self->style > 2))
	{
		gi.dprintf( "weapon_msg90 has invalid \"style\" at %s, should be 1 or 2.\n", vtos(self->s.origin));
		G_FreeEdict( self );
		return;
	}

	self->model = "models/weapons/g_msg90/tris.md2";
	SpawnItem (self, FindItem ("Msg90"));
	self->item->icon = "/pics/kpl/kpl_icon.tga";
	self->s.modelindex = gi.modelindex("models/weapons/g_msg90/tris.md2");
	VectorSet( self->mins, -32, -16, -32 );
	VectorSet( self->maxs,  40,  20, -24 );
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_TRIGGER;
	gi.linkentity( self );
	self->touch = msg90_touch;
	self->think = msg90_think;
	self->nextthink = level.time + 2;
}
/////end assault 2
// JOSEPH 13-JUN-99
/*QUAKED hmg_mod_cooling (.7 .3 .4) (-16 -16 -16) (16 16 16)
model="models/pu_icon/modcool/tris.md2" 
*/
void SP_hmg_mod_cooling (edict_t *self)
{
	self->model = "models/pu_icon/coolmod/tris.md2";
//	self->model = "models/pu_icon/msg_90/tris.md2"; //assault2
	self->count = 4;
	SpawnItem (self, FindItem ("Pistol_Mods"));
	self->item->icon = "/pics/h_heavymachinegun_cooling.tga"; //assault2
//	self->item->icon = "/pics/kpl/kpl_icon.tga"; //assault2
	//gi.soundindex ("items/pistol_mods.wav");
}
// END JOSEPH

// JOSEPH 1-APR-99
/*QUAKED pistol_mod_rof (.7 .3 .4) (-16 -16 -16) (16 16 16)
model="models/pu_icon/ro_fire_mod/tris.md2" 
*/
void SP_pistol_mod_rof (edict_t *self)
{
	self->model = "models/pu_icon/ro_fire_mod/tris.md2";
	self->count = 1;
	SpawnItem (self, FindItem ("Pistol_Mods"));
	//gi.soundindex ("items/pistol_mods.wav");
}

/*QUAKED pistol_mod_reload (.7 .3 .4) (-16 -16 -16) (16 16 16)
model="models/pu_icon/reload_mod/tris.md2" 
model="models/pu_icon/reload_mod/tris.md2" 
*/
void SP_pistol_mod_reload (edict_t *self)
{
	self->model = "models/pu_icon/reload_mod/tris.md2";
	self->count = 2;
	SpawnItem (self, FindItem ("Pistol_Mods"));
	//gi.soundindex ("items/pistol_mods.wav");
}

/*QUAKED pistol_mod_damage (.7 .3 .4) (-16 -16 -16) (16 16 16)
model="models/pu_icon/magmod/tris.md2" 
*/
void SP_pistol_mod_damage (edict_t *self)
{
	self->model = "models/pu_icon/magmod/tris.md2";
	self->count = 3;
	SpawnItem (self, FindItem ("Pistol_Mods"));
	//gi.soundindex ("items/pistol_mods.wav");
}
// END JOSEPH

/*UAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/n_health.wav");
}

/*UAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = 2;
	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("items/s_health.wav");
}

/*UAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_large (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/l_health.wav");
}

/*UAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_mega (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
}

// RAFAEL
void SP_item_foodcube (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/objects/trapfx/tris.md2";
	SpawnItem (self, FindItem ("Health"));
	self->spawnflags |= DROPPED_ITEM;
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("items/s_health.wav");
	self->classname = "foodcube";
}

void InitItems (void)
{
	game.num_items = sizeof(itemlist)/sizeof(itemlist[0]) - 1;
}



/*
===============
SetItemNames

Called by worldspawn
===============
*/
void SetItemNames (void)
{
	int		i;
	gitem_t	*it;

	for (i=0 ; i<game.num_items ; i++)
	{
		it = &itemlist[i];
		gi.configstring (CS_ITEMS+i, it->pickup_name);
	}

	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
}
