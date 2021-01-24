// g_weapon.c

#include "g_local.h"
#include "m_player.h"


static qboolean	is_quad;
// RAFAEL
static qboolean is_quadfire;
static byte		is_silenced;

void weapon_grenade_fire (edict_t *ent, qboolean held);
// RAFAEL
void weapon_trap_fire (edict_t *ent, qboolean held);

static void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}


/*
===============
PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
===============
*/
void PlayerNoise(edict_t *who, vec3_t where, int type)
{

	if (deathmatch->value && type != PNOISE_WEAPON)
		return;

	VectorCopy( who->s.origin, who->noise_pos );
	who->noise_time = level.time + 2;
	who->noise_type = type;
	VectorCopy( who->s.angles, who->noise_angles );

}

#define MAX_PISTOL_ROUNDS		10
#define MAX_TOMMYGUN_ROUNDS		50
#define MAX_SHOTGUN_ROUNDS		8
#define MAX_BARMACHINEGUN_ROUNDS 	30
#define MAX_GRENADELAUNCHER_ROUNDS	3
#define MAX_ROCKETLAUNCHER_ROUNDS	5
#define	MAX_FLAMEGUN_ROUNDS			50

// this is here so we can simplify the auto-reload code. it's in the same order as clip_t
int	auto_rounds[] =
{
	0,
	MAX_PISTOL_ROUNDS,
	MAX_SHOTGUN_ROUNDS,
	MAX_TOMMYGUN_ROUNDS,
	MAX_BARMACHINEGUN_ROUNDS,
	MAX_GRENADELAUNCHER_ROUNDS,
	MAX_ROCKETLAUNCHER_ROUNDS,
	//MAX_FLAMEGUN_ROUNDS,
	0
};

int QweryClipIndex (gitem_t *item);

void AutoLoadWeapon( gclient_t *client, gitem_t *weapon, gitem_t *ammo )
{
	int clip_index;
	int	ammo_index;

	if (((clip_index = QweryClipIndex( weapon )) != CLIP_NONE) && !(client->pers.weapon_clip[clip_index]))
	{
		ammo_index = ITEM_INDEX(ammo);

		if (client->pers.inventory[ammo_index] >= auto_rounds[clip_index])
		{
			client->pers.weapon_clip[clip_index] = auto_rounds[clip_index];
			client->pers.inventory[ammo_index] -= auto_rounds[clip_index];
		}
		else	// not quite enough ammo, so just load all ammo
		{
			client->pers.weapon_clip[clip_index] = client->pers.inventory[ammo_index];
			client->pers.inventory[ammo_index] = 0;
		}
	}
}

qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*ammo;
	int			auto_reload=false;

	index = ITEM_INDEX(ent->item);

	// assault2 virgin23 ... need custom pickup sounds here

if (!(strcmp (ent->classname, "weapon_msg90")))
	{
		if (other->client->last_wave > (level.time - 5) && (other->client->last_wave <= level.time))
			return false;
		gi.centerprintf (other, "Weapon_Msg90 Index #%i Under Construction.",index);
		other->client->last_wave = level.time;
		return false;
	}
else if (!(strcmp (ent->classname, "weapon_mortar")))
	{
		if (other->client->last_wave > (level.time - 5) && (other->client->last_wave <= level.time))
			return false;
		gi.centerprintf (other, "Weapon_Mortar Index #%i Under Construction.",index);
		other->client->last_wave = level.time;
		return false;
	}

	if ( ( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value) 
		&& other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}

	// Ridah, start with gun loaded
	if (!(other->client->pers.inventory[index]))
		auto_reload = true;

	//other->client->pers.inventory[index]++;
	other->client->pers.inventory[index] = 1;

	if (ent->item->ammo && !(ent->spawnflags & DROPPED_ITEM) )
	{
		// give them some ammo with it
		ammo = FindItem (ent->item->ammo);
		if ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value )
			Add_Ammo (other, ammo, 1000);
		else
			Add_Ammo (other, ammo, ammo->quantity);

		if (auto_reload)
			AutoLoadWeapon( other->client, ent->item, ammo );

		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) )
		{
			if (deathmatch->value)
			{
				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn (ent, 30);
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	}
	else if (!(strcmp (ent->classname, "weapon_crowbar")))
	{
		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) )
		{
			if (deathmatch->value)
			{
				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn (ent, 30);
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	}
	
	if (other->client->pers.weapon == FindItem ("SPistol"))
			other->client->pers.silencer_shots = 20;

	/*
	if (other->client->pers.weapon != ent->item && 
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value || other->client->pers.weapon == FindItem("Pipe") ) )
		other->client->newweapon = ent->item;
	*/
	
	if (deathmatch->value)  //Assault2 added other->client->pers.weapon == FindItem ("SPistol") //for the silencer // PUREASSAULT
	{
		if (other->client->pers.weapon != ent->item && auto_reload && (other->client->pers.weapon == FindItem ("Pistol") || other->client->pers.weapon == FindItem ("SPistol") || other->client->pers.weapon == FindItem ("Pipe") ))
			other->client->newweapon = ent->item;
	}
	else
	{
	if (other->client->pers.weapon != ent->item && auto_reload)
		other->client->newweapon = ent->item;
	}
	
	
	return true;
}

/*
Change Clip Index 
*/
void ChangeClipIndex (edict_t *ent)
{
	
	if (!strcmp(ent->client->pers.weapon->pickup_name, "Pipe"))
		ent->client->clip_index = CLIP_NONE;
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "Crowbar"))
		ent->client->clip_index = CLIP_NONE;
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "Pistol"))
		ent->client->clip_index = CLIP_PISTOL;
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "SPistol"))
		ent->client->clip_index = CLIP_PISTOL;
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "Shotgun"))
		ent->client->clip_index = CLIP_SHOTGUN;
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "Tommygun"))
		ent->client->clip_index = CLIP_TOMMYGUN;
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "FlameThrower"))
		ent->client->clip_index = CLIP_FLAMEGUN;
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "Bazooka"))
		ent->client->clip_index = CLIP_ROCKETS;
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "Grenade Launcher"))
		ent->client->clip_index = CLIP_GRENADES;
	// JOSEPH 16-APR-99
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "Heavy machinegun"))
		ent->client->clip_index = CLIP_SLUGS;
	// END JOSEPH
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "Spaz"))
		ent->client->clip_index = CLIP_SPAZ; // virgin16
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "Pump"))
		ent->client->clip_index = CLIP_PUMP; // virgin17
	else if (!strcmp(ent->client->pers.weapon->pickup_name, "Ak47"))
//		ent->client->clip_index = CLIP_AK47; // virgin23
		ent->client->clip_index = CLIP_TOMMYGUN;
}


int QweryClipIndex (gitem_t *item)
{
	
	if (!strcmp(item->pickup_name, "Pipe"))
		return CLIP_NONE;
	else if (!strcmp(item->pickup_name, "Crowbar"))
		return CLIP_NONE;
	else if (!strcmp(item->pickup_name, "Pistol"))
		return CLIP_PISTOL;
	else if (!strcmp(item->pickup_name, "SPistol"))
		return CLIP_PISTOL;
	else if (!strcmp(item->pickup_name, "Shotgun"))
		return CLIP_SHOTGUN;
	else if (!strcmp(item->pickup_name, "Tommygun"))
		return CLIP_TOMMYGUN;
	else if (!strcmp(item->pickup_name, "FlameThrower"))
		return CLIP_FLAMEGUN;
	else if (!strcmp(item->pickup_name, "Bazooka"))
		return CLIP_ROCKETS;
	else if (!strcmp(item->pickup_name, "Grenade Launcher"))
		return CLIP_GRENADES;
	// JOSEPH 16-APR-99
	else if (!strcmp(item->pickup_name, "Heavy machinegun"))
		return CLIP_SLUGS;
	// END JOSEPH
	else if (!strcmp(item->pickup_name, "Spaz")) // assault2 virgin16
		return CLIP_SPAZ;
	else if (!strcmp(item->pickup_name, "Pump")) // assault2 virgin17
		return CLIP_PUMP;
	else if (!strcmp(item->pickup_name, "Ak47")) // assault2 virgin23
//		return CLIP_AK47;
		return CLIP_TOMMYGUN;
	return (0);
}

/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon (edict_t *ent)
{
	int i;

	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire (ent, false);
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	// set visible model
	if (ent->s.modelindex == 255) {
		if (ent->client->pers.weapon)
			i = (((QweryClipIndex(ent->client->pers.weapon)+1) & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	else
		ent->client->ammo_index = 0;

	if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	
	// ent->client->flashlight = false;

	// RAFAEL	28-dec-98
	ent->client->gun_noise = false;
	
	if (ent->client->pers.weapon)
		ChangeClipIndex (ent);
	
	{
		int i;
	
		if (!strcmp (ent->client->pers.weapon->pickup_name , "SPistol"))
		{
			if (ent->client->pers.pistol_mods & WEAPON_MOD_DAMAGE)
				ent->client->ps.gunindex = gi.modelindex("models/weapons/silencer_mdx/magnum.mdx");
			else
				ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		}
		else if (!strcmp (ent->client->pers.weapon->pickup_name , "Pistol"))
		{
			if (ent->client->pers.pistol_mods & WEAPON_MOD_DAMAGE)
				ent->client->ps.gunindex = gi.modelindex("models/weapons/v_colt/magnum.mdx");
			else
				ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		}
		else if (!strcmp (ent->client->pers.weapon->pickup_name, "Shotgun"))
			ent->client->ps.gunindex = gi.modelindex("models/weapons/shotgun/shotgun.mdx");

		else if (!strcmp (ent->client->pers.weapon->pickup_name, "Spaz")) // assault2 virgin16
			ent->client->ps.gunindex = gi.modelindex("models/weapons/spaz/spaz.mdx");

		else if (!strcmp (ent->client->pers.weapon->pickup_name, "Pump")) // assault2 virgin17
			ent->client->ps.gunindex = gi.modelindex("models/weapons/pump/pump.mdx");

		else if (!strcmp (ent->client->pers.weapon->pickup_name, "Ak47")) // assault2 virgin23
			ent->client->ps.gunindex = gi.modelindex("models/weapons/ak47/ak47.mdx");


		else if (!strcmp (ent->client->pers.weapon->pickup_name, "Heavy machinegun"))
		{
			if (ent->client->pers.pistol_mods & WEAPON_MOD_COOLING_JACKET)
				ent->client->ps.gunindex = gi.modelindex("models/weapons/v_hmgcool/hmgcool.mdx");
//				ent->client->ps.gunindex = gi.modelindex("models/weapons/v_msg90/msg90.mdx");//assault2
			else
				ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		}
		else
			ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

		memset(&(ent->client->ps.model_parts[0]), 0, sizeof(model_part_t) * MAX_MODEL_PARTS);
		ent->client->ps.num_parts = 0;

		ent->client->ps.num_parts++;
		// ent->client->ps.model_parts[PART_HEAD].modelindex = gi.modelindex(ent->client->pers.weapon->view_model);
		ent->client->ps.model_parts[PART_HEAD].modelindex = ent->client->ps.gunindex;//gi.modelindex(ent->client->pers.weapon->view_model);
		for (i=0; i<MAX_MODELPART_OBJECTS; i++)
			ent->client->ps.model_parts[PART_HEAD].skinnum[i] = 0; // will we have more than one skin???

		// HACK, set arm model
		if (!strcmp(ent->client->pers.weapon->pickup_name, "Shotgun"))
		{
			// Ridah, HACK: changed this to use PART_LEGS, since PART_BODY sometimes doesn't show up
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/shotgun/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???
		}

		else if (!strcmp(ent->client->pers.weapon->pickup_name, "Spaz")) // assault2 virgin16
		{
			// Ridah, HACK: changed this to use PART_LEGS, since PART_BODY sometimes doesn't show up
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/spaz/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???
		}

		else if (!strcmp(ent->client->pers.weapon->pickup_name, "Pump")) // assault2 virgin17
		{
			// Ridah, HACK: changed this to use PART_LEGS, since PART_BODY sometimes doesn't show up
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/pump/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???
		}

		else if (!strcmp(ent->client->pers.weapon->pickup_name, "Ak47")) // assault2 virgin23
		{
			// Ridah, HACK: changed this to use PART_LEGS, since PART_BODY sometimes doesn't show up
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/ak47/rhand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???
		}

		else if (!strcmp(ent->client->pers.weapon->pickup_name, "Pipe"))
		{
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/blakjak/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		}

		else if (!strcmp(ent->client->pers.weapon->pickup_name, "Crowbar"))
		{
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/crowbar/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		}

		else if (!strcmp(ent->client->pers.weapon->pickup_name, "FlameThrower"))
		{
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_flamegun/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		}
		// END JOSEPH
		// JOSEPH 4-MAR-99
		else if (!strcmp(ent->client->pers.weapon->pickup_name, "Bazooka"))
		{
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_rocket_launcher/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???

			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_rocket_launcher/shell_a.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???		

			// note to self: not using this anymore
			/*
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_GUN].modelindex = gi.modelindex("models/weapons/v_rocket_launcher/shell_b.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_GUN].skinnum[i] = 0; // will we have more than one skin???		
			*/

			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_GUN2].modelindex = gi.modelindex("models/weapons/v_rocket_launcher/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_GUN2].skinnum[i] = 0; // will we have more than one skin???		
		}
		// END JOSEPH
		else if (!strcmp (ent->client->pers.weapon->pickup_name , "SPistol"))
		{
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/silencer_mdx/handl.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
			
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/silencer_mdx/handr.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???
		
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_GUN].modelindex = gi.modelindex("models/weapons/silencer_mdx/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_GUN].skinnum[i] = 0;

			ent->client->ps.model_parts[PART_GUN].invisible_objects = (1<<0 | 1<<1);
		}
		else if (!strcmp (ent->client->pers.weapon->pickup_name , "Pistol"))
		{
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_colt/handl.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
			
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_colt/handr.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???
		
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_GUN].modelindex = gi.modelindex("models/weapons/v_colt/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_GUN].skinnum[i] = 0;
		}
		else if (!strcmp(ent->client->pers.weapon->pickup_name, "Tommygun"))
		{
			// Ridah, hacked this, we're still having disappearing hands after going from the Pistol ?!
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_CIGAR].modelindex = gi.modelindex("models/weapons/v_tomgun/lhand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_CIGAR].skinnum[i] = 0; // will we have more than one skin???

			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_tomgun/rhand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???
		
		}
		// JOSEPH 16-APR-99
		else if (!strcmp (ent->client->pers.weapon->pickup_name , "Heavy machinegun"))
		{
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_hmg/lhand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???

			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_hmg/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
			
		}
		// END JOSEPH
		else if (!strcmp (ent->client->pers.weapon->pickup_name , "Grenade Launcher"))
		{
			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_grenade_launcher/lhand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???

			ent->client->ps.num_parts++;
			ent->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_grenade_launcher/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				ent->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		}
	}

//	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{	
	//ent->client->pers.weapon_clip[ent->client->clip_index]
	// Assault2 ... The PROBLEM with ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] is....
	//				it only returns what ammo is in the clients pockets, not what's left in his gun's clip.
	// Assault2 ... Intimate knowledge or clip_t lets us use, for example 5, as the grenade clip_index number.
	//ent->client->pers.weapon_clip[ent->client->clip_index]

	// return to hmg if there are 308cal still in the clip OR your pocket.
	if ( (ent->client->pers.weapon_clip[4] || ent->client->pers.inventory[ITEM_INDEX(FindItem("308cal"))] )
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Heavy machinegun"))] )
	{
		ent->client->newweapon = FindItem ("Heavy machinegun");
		return;
	}

	// return to bazooka if there are rockets still in the clip OR your pocket.
	if ( (ent->client->pers.weapon_clip[6] || ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] )
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Bazooka"))] )
	{
		ent->client->newweapon = FindItem ("Bazooka");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("gas"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("FlameThrower"))] )
	{
		ent->client->newweapon = FindItem ("flamethrower");
		return;
	}

	//  return to tommy if there are tommyslugs still in the clip OR your pocket.	
	if ( ( ent->client->pers.weapon_clip[10] || ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] )
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Ak47"))] )
	{
		ent->client->newweapon = FindItem ("Ak47"); // virgin23
		return;
	}

	//  return to tommy if there are tommyslugs still in the clip OR your pocket.	
	if ( ( ent->client->pers.weapon_clip[3] || ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] )
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Tommygun"))] )
	{
		ent->client->newweapon = FindItem ("Tommygun");
		return;
	}
	
	//  return to shotty if there are shells still in the clip OR your pocket.
	if ( ( ent->client->pers.weapon_clip[8] || ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] )
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("Spaz"))] )
	{
		ent->client->newweapon = FindItem ("Spaz"); // assault2 virgin16
		return;
	}
	
	//  return to shotty if there are shells still in the clip OR your pocket.
	if ( ( ent->client->pers.weapon_clip[9] || ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] )
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("Pump"))] )
	{
		ent->client->newweapon = FindItem ("Pump"); // assault2 virgin17
		return;
	}	

	if ( ( ent->client->pers.weapon_clip[2] || ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] )
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("Shotgun"))] )
	{
		ent->client->newweapon = FindItem ("Shotgun");
		return;
	}

	//  return to grenlauncher if there are grenades still in the clip OR your pocket.
	if ( ( ent->client->pers.weapon_clip[5] || ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] ) 
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenade Launcher"))] )
	{
		ent->client->newweapon = FindItem ("Grenade Launcher");
		return;
	}	

	// return to pistol if there are bullets still in the clip OR your pocket.
	if ( ( ent->client->pers.weapon_clip[1] || ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] )
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("pistol"))] )
	{
		ent->client->newweapon = FindItem ("pistol");
		return;
	}

	ent->client->newweapon = FindItem ("Crowbar");
	//	ent->client->newweapon = FindItem ("Pipe");
}


/*
Think_FlashLight
*/
void Think_FlashLight (edict_t *ent)
{

	trace_t tr;
	vec3_t	forward;
	vec3_t	end;
	vec3_t	mins, maxs;
	edict_t	*light;
	vec3_t	angles, origin;

	if (ent->health < 1)
		return;

	if (ent->client->flashlight)
	{
		VectorCopy (ent->client->v_angle, angles);
		VectorCopy (ent->s.origin, origin);
		AngleVectors (angles, forward, NULL, NULL);
		VectorCopy (origin, end);
		VectorMA (end, 8194, forward, end);

		VectorSet (mins, -8, -8, -8 );
		VectorSet (maxs, 8, 8, 8);

		tr = gi.trace (ent->s.origin, mins, maxs, end, ent, MASK_SHOT);

		if (((tr.ent) && (tr.ent->s.renderfx2 & RF2_SURF_ALPHA)) || ((tr.contents & MASK_ALPHA))) 
		{
			vec3_t	start;
			VectorCopy (tr.endpos, start);
			VectorMA (start, 8, forward, start);
			tr = gi.trace (start, mins, maxs, end, ent, MASK_SHOT);
		}

		light = G_Spawn();
		VectorCopy (tr.endpos, light->s.origin);
		light->s.renderfx |= RF_BEAM;
		light->s.effects |= EF_FLASHLIGHT;
		light->nextthink = level.time + 0.1;
		light->think = G_FreeEdict;
		gi.linkentity (light);
	}


}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/		
void Think_Weapon (edict_t *ent)
{
	// make sure flamethrower is disabled when it should be
	ent->s.renderfx2 &= ~RF2_FLAMETHROWER;

	// if just died, put the weapon away
	if (ent->health < 1)
	{
		ent->client->newweapon = NULL;
		ChangeWeapon (ent);
	}
	
	// JOSEPH 10-FEB-99
	if (level.bar_lvl)
		return;
	// END JOSEPH

	// Unholster a holstered gun if fired
	if ((ent->onarampage==0)&&(!ent->client->pers.weapon) && ent->client->pers.holsteredweapon && (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)))
	{
		ent->client->newweapon = ent->client->pers.holsteredweapon;
		ChangeWeapon (ent);
		ent->client->pers.holsteredweapon = 0;
	}
	else if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		is_quad = (ent->client->quad_framenum > level.framenum);
		// RAFAEL
		is_quadfire = (ent->client->quadfire_framenum > level.framenum);
		//if (ent->client->pers.silencer_shots)
		//	is_silenced = MZ_SILENCED;
		//else
		is_silenced = 0;
		ent->client->pers.weapon->weaponthink (ent);
	}

}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================ ent->client->pers.weapon->pickup_name //virgin17
*/
void Use_Weapon (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;
	int			clip_index;

		if(!strcmp (item->pickup_name,"Tommygun")) // assault2 virgin23
		{
			int index;

			if(ent->client->pers.weapon == FindItem ("Tommygun") )
			{
				item = FindItem ("Ak47"); // virgin23
				/*
				if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
				{
					ammo_item = FindItem(item->ammo);
					ammo_index = ITEM_INDEX(ammo_item);
					
					clip_index = QweryClipIndex (item);

					if (!ent->client->pers.inventory[ammo_index] && !ent->client->pers.weapon_clip [clip_index])
					{ // No ammo for this weapon
						cprintf (ent, PRINT_HIGH, "No %s for the %s.\n", ammo_item->pickup_name, item->pickup_name);
						return;
					}
				}
				*/
			}

			else if(ent->client->pers.weapon == FindItem ("Ak47") )
			{
				item = FindItem ("Tommygun");
				if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
				{
					ammo_item = FindItem(item->ammo);
					ammo_index = ITEM_INDEX(ammo_item);
					
					clip_index = QweryClipIndex (item);

					if (!ent->client->pers.inventory[ammo_index] && !ent->client->pers.weapon_clip [clip_index])
					{ // No ammo for this weapon
						cprintf (ent, PRINT_HIGH, "No %s for the %s.\n", ammo_item->pickup_name,item->pickup_name);
						return;
					}
				}
			}

			else 
			{
				item = FindItem ("Tommygun"); // virgin23
				if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
				{
					ammo_item = FindItem(item->ammo);
					ammo_index = ITEM_INDEX(ammo_item);
					
					clip_index = QweryClipIndex (item);

					if (!ent->client->pers.inventory[ammo_index] && !ent->client->pers.weapon_clip [clip_index])
					{ // No ammo for this weapon
						cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
						return;
					}
				}
			} // end virgin22

			index = ITEM_INDEX (item);

			if (!ent->client->pers.inventory[index]) 
				item = FindItem ("Tommygun");
		}

// virgin22 puts this outside the toggling weapon key block
		else if (strcmp (item->pickup_name, "Shotgun") == 0 /* assault2 virgin16
			&& ent->client->pers.shotgunfrags > assault_customweaponfrags // virgin18 */
		   )
		{ // virgin19 removes this for now until release :)

			int index;
/*
			switch(ent->client->pers.team)
			{
			case 1:	item = FindItem ("Spaz"); break; // virgin19
//			case 1:	item = FindItem ("Shotgun"); break; // virgin16
			default:
//			case 2:	item = FindItem ("Shotgun"); break; // virgin17
			case 2:	item = FindItem ("Pump"); break; // virgin19
			}
*/
			if(ent->client->pers.weapon == FindItem ("Shotgun") )
			{
				item = FindItem ("Spaz"); // virgin22
				if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
				{
					ammo_item = FindItem(item->ammo);
					ammo_index = ITEM_INDEX(ammo_item);
					
					clip_index = QweryClipIndex (item);

					if (!ent->client->pers.inventory[ammo_index] && !ent->client->pers.weapon_clip [clip_index])
					{ // No ammo for this weapon
						cprintf (ent, PRINT_HIGH, "No %s for the Spaz Gun.\n", ammo_item->pickup_name);
						item = FindItem ("Pump");
					}
				}
			}

			else if(ent->client->pers.weapon == FindItem ("Spaz") )
			{
				item = FindItem ("Pump");
				if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
				{
					ammo_item = FindItem(item->ammo);
					ammo_index = ITEM_INDEX(ammo_item);
					
					clip_index = QweryClipIndex (item);

					if (!ent->client->pers.inventory[ammo_index] && !ent->client->pers.weapon_clip [clip_index])
					{ // No ammo for this weapon
						cprintf (ent, PRINT_HIGH, "No %s for the Pump Gun.\n", ammo_item->pickup_name);
						item = FindItem ("Shotgun");
					}
				}
			}

			else // returning to shotgun from another, non-shell ammo weapon
			{
				item = FindItem ("Shotgun"); // virgin22
				if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
				{
					ammo_item = FindItem(item->ammo);
					ammo_index = ITEM_INDEX(ammo_item);
					
					clip_index = QweryClipIndex (item);

					if (!ent->client->pers.inventory[ammo_index] && !ent->client->pers.weapon_clip [clip_index])
					{ // No ammo for this weapon
						cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
						item = FindItem ("Spaz");
						if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
						{
							ammo_item = FindItem(item->ammo);
							ammo_index = ITEM_INDEX(ammo_item);
							
							clip_index = QweryClipIndex (item);

							if (!ent->client->pers.inventory[ammo_index] && !ent->client->pers.weapon_clip [clip_index])
							{ // No ammo for this weapon
								cprintf (ent, PRINT_HIGH, "No %s for Spaz Gun.\n", ammo_item->pickup_name);
								item = FindItem ("Pump");
							}
						}
					}
				}
			} // end virgin22

			index = ITEM_INDEX (item);

			if (!ent->client->pers.inventory[index]) 
				item = FindItem ("Shotgun");
		} // end virgin22
	
	// see if we're already using it
	if (item == ent->client->pers.weapon) // toggling weapon key?
	{
//		if (strcmp (item->pickup_name, "Shotgun") == 0 /* assault2 virgin16
//			&& ent->client->pers.shotgunfrags > assault_customweaponfrags // virgin18 */
//		   )
//		{ // virgin19 removes this for now until release :)
//
//			int index;
/*
			switch(ent->client->pers.team)
			{
			case 1:	item = FindItem ("Spaz"); break; // virgin19
//			case 1:	item = FindItem ("Shotgun"); break; // virgin16
			default:
//			case 2:	item = FindItem ("Shotgun"); break; // virgin17
			case 2:	item = FindItem ("Pump"); break; // virgin19
			}
*/
//			if(ent->client->pers.weapon == FindItem ("Spaz") )
//				item = FindItem ("Pump");
//			else
//				item = FindItem ("Spaz"); // virgin22
//
//			index = ITEM_INDEX (item);
//
//			if (!ent->client->pers.inventory[index]) 
//				item = FindItem ("Shotgun");
//		}

//		else return;
		return; //virgin22
	}

	
	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		
		clip_index = QweryClipIndex (item);

		if (!ent->client->pers.inventory[ammo_index] && !ent->client->pers.weapon_clip [clip_index])
		{ // No ammo for this weapon
			cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;
}

// RAFAEL 14-APR-98
void Use_Weapon2 (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;
	int			index;
	int			clip_index;

	if (item == ent->client->pers.weapon) // toggling weapon key?
	{

		if (strcmp (item->pickup_name, "Pistol") == 0)
		{
			item = FindItem ("SPistol");
			index = ITEM_INDEX (item);
			if (!ent->client->pers.inventory[index]) item = FindItem ("Pistol");
		}

		else if (strcmp (item->pickup_name, "Pipe") == 0)
		{
			// Ridah, changed this so it defaults to Crowbar if we have it
			if (ent->client->pers.weapon != FindItem ("Crowbar"))
			{
				item = FindItem ("Crowbar");
				index = ITEM_INDEX (item);
				if (!ent->client->pers.inventory[index]) item = FindItem ("Pipe");
			}
		}
	}	
	
	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;
	
	if (item->ammo)
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		clip_index = QweryClipIndex (item);

		if (!ent->client->pers.inventory[ammo_index] && !g_select_empty->value && !ent->client->pers.weapon_clip [clip_index])
		{
			cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;
	
}
// END 14-APR-98

/*
================
Drop_Weapon
================
*/
void Drop_Weapon (edict_t *ent, gitem_t *item)
{
	int		index;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	if ( ((item == ent->client->pers.weapon) || (item == ent->client->newweapon))&& (ent->client->pers.inventory[index] == 1) )
	{
		cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item (ent, item);
	ent->client->pers.inventory[index]--;
}

/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST	(FRAME_IDLE_LAST + 1)

#define FRAME_OFFSET_FOR_SPISTOL	43
#define FRAME_OFFSET_FOR_SPISTOL_WITH_SILENCER 70

void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;
// RAFAEL 01-11-99

	if (ent->client->weaponstate ==	WEAPON_RELOADING_SPISTOL)
	{
		if (ent->client->ps.gunframe == FRAME_OFFSET_FOR_SPISTOL)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate ==	WEAPON_RELOADING)
	{
		if (ent->client->ps.gunframe == FRAME_FIRE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_RELOAD_CYCLE)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->pers.weapon == FindItem("SPistol"))
		{
			if (ent->client->pers.silencer_shots && ent->client->ps.gunframe == FRAME_OFFSET_FOR_SPISTOL_WITH_SILENCER)
			{
				ChangeWeapon (ent);
				return;
			}
			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
			{
				ChangeWeapon (ent);
				return;
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
			{
				ChangeWeapon (ent);
				return;
			}
			else if ((ent->client->weaponstate != WEAPON_FIRING) && (ent->client->ps.gunframe < FRAME_DEACTIVATE_FIRST))
			{
				ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
				return;
			}
		}

		ent->client->ps.gunframe++;

		// Ridah, faster weapon changing in deathmatch
		if (deathmatch->value && ent->client->ps.gunframe < FRAME_DEACTIVATE_LAST)
			ent->client->ps.gunframe++;

		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;

		// Ridah, faster weapon changing in deathmatch
		if (deathmatch->value && ent->client->ps.gunframe < FRAME_ACTIVATE_LAST)
			ent->client->ps.gunframe++;

		return;
	}
	
	if ((ent->client->newweapon) /*&& (ent->client->weaponstate != WEAPON_FIRING)*/)
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			 // virgin23 // chief rewrote this reducing the code bloat
			if (   (   ent->client->clip_index == CLIP_PISTOL
			        || ent->client->clip_index == CLIP_SHOTGUN
					|| ent->client->clip_index == CLIP_SPAZ
					|| ent->client->clip_index == CLIP_PUMP
					|| ent->client->clip_index == CLIP_TOMMYGUN
					|| ent->client->clip_index == CLIP_SLUGS
					|| ent->client->clip_index == CLIP_GRENADES
					|| ent->client->clip_index == CLIP_ROCKETS
			       )
			    && ent->client->pers.weapon_clip[ent->client->clip_index] >= 1
			   )
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
/*			
			if (ent->client->clip_index == CLIP_PISTOL && ent->client->pers.weapon_clip[ent->client->clip_index] >= 1)
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else if (ent->client->clip_index == CLIP_SHOTGUN && ent->client->pers.weapon_clip[ent->client->clip_index] >= 1)
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else if (ent->client->clip_index == CLIP_SPAZ && ent->client->pers.weapon_clip[ent->client->clip_index] >= 1)
			{ // virgin16
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else if (ent->client->clip_index == CLIP_PUMP && ent->client->pers.weapon_clip[ent->client->clip_index] >= 1)
			{ // virgin17
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else if (ent->client->clip_index == CLIP_AK47 && ent->client->pers.weapon_clip[ent->client->clip_index] >= 1)
			{ // virgin23
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else if (ent->client->clip_index == CLIP_TOMMYGUN && ent->client->pers.weapon_clip[ent->client->clip_index] >=1)
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else if (ent->client->clip_index == CLIP_SLUGS && ent->client->pers.weapon_clip[ent->client->clip_index] >=1)
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else if (ent->client->clip_index == CLIP_GRENADES && ent->client->pers.weapon_clip[ent->client->clip_index] >=1)
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else if (ent->client->clip_index == CLIP_ROCKETS && ent->client->pers.weapon_clip[ent->client->clip_index] >=1)
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
*/
			else if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else 
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand()&15)
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		// JOSEPH 5-FEB-99-B
		// Blackjack hack
        if ((ent->client->ps.gunframe == 9) &&
			(!((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)) &&
			(!strcmp(ent->client->pers.weapon->pickup_name, "Pipe")))
		{
			ent->client->ps.gunframe = FRAME_IDLE_FIRST+1;
			ent->client->weaponstate = WEAPON_READY;
		}
		// END JOSEPH

		// JOSEPH 19-JAN-99
		// Crowbar hack
        else if ((ent->client->ps.gunframe == 9) &&
			(!((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)) &&
			(!strcmp(ent->client->pers.weapon->pickup_name, "Crowbar")))
		{
			ent->client->ps.gunframe = FRAME_IDLE_FIRST+1;
			ent->client->weaponstate = WEAPON_READY;
		}
		// END JOSEPH


		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}


/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

void weapon_grenade_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;
/*
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
*/
}

void Weapon_Grenade (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/grenade_launcher/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/grenade_launcher/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_grenade_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_grenade_fire (ent, false);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}



// JOSEPH 18-DEC-98
void Think_Eject_Shell (edict_t *ent)
{
	if (!ent->misstime--)
	{	
		ent->nextthink = 0;
		G_FreeEdict(ent);
	}
	else
	{
		if (ent->misstime <= 20)
		{
			if (ent->misstime == 20)
			{
				ent->s.renderfx2 |= RF2_PASSALPHA;
				ent->s.effects = 1;		// this is full alpha now
			}
			
			ent->s.effects += (255/20);
		
			if (ent->s.effects > 255)
				ent->s.effects = 255;
		}

//		ent->s.angles[0] += ent->avelocity[0];
		ent->s.angles[1] += ent->avelocity[1];
//		ent->s.angles[2] += ent->avelocity[2];
		ent->nextthink = level.time + 0.1;
	}
}
// END JOSEPH

// JOSEPH 7-JAN-99
void Eject_Shotgun_Shell (edict_t *ent)
{
	edict_t	*dropped = NULL;
	vec3_t	forward, right, up;
	vec3_t	offset;

	if (!(ent->client))
		return;

	if (deathmatch->value)
		return;

	//gi.dprintf("angles = %s\n", vtos(ent->s.angles));

	dropped = G_Spawn();
	
	if (!dropped)
		return;

	dropped->s.modelindex = gi.modelindex("models/weapons/sshell_md2/tris.md2");
		
	dropped->movetype = MOVETYPE_BOUNCE;
	
	dropped->mins[0] = -4;
	dropped->mins[1] = -4;
	dropped->mins[2] = -2;
	
	dropped->maxs[0] =  4;
	dropped->maxs[1] =  4;
	dropped->maxs[2] =  4;

	AngleVectors (ent->client->v_angle, forward, right, up);

	dropped->s.angles[YAW] = ent->client->v_angle[YAW];

	VectorSet(offset, 0, 16, ent->viewheight -2 );
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, dropped->s.origin);

	VectorScale (forward, 5 + ((rand()&15)-8), dropped->velocity);
    VectorMA (dropped->velocity, 50 + (rand()&31), right, dropped->velocity);
	dropped->velocity[2] = 150 + (rand()&31);

	// Ridah, changed this so they don't stick in the ground so much
	VectorSet(dropped->avelocity, 0/*((rand()&7)-4)*/, ((rand()&15)-8), ((rand()&7)-4));

	dropped->think = Think_Eject_Shell;
	dropped->misstime = 40;
	dropped->nextthink = level.time + 0.1;

	dropped->s.renderfx2 |= RF2_NOSHADOW;

	gi.linkentity (dropped);

	return;
}
// END JOSEPH




//======================================================================



// JOSEPH 12-OCT-98
void Blackjack_Hit (edict_t *ent, vec3_t vorigin, int damage)
{
	vec3_t		start;
	vec3_t		forward, right;
	int			kick = 2;
	vec3_t		offset;


	AngleVectors (ent->client->v_angle, forward, right, NULL);
	
	VectorSet(offset, 0, 0, ent->viewheight);
	
	VectorAdd (offset, vorigin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (fire_blackjack (ent, start, forward, damage, kick, MOD_BLACKJACK))
		ent->client->ps.weapon_usage++;

	if (ent->client->ps.weapon_usage > 20)
	{
		ent->client->ps.rdflags &= ~RDF_SKINPLUS;
		ent->client->ps.rdflags |= RDF_SKINPLUSPLUS;
	}
	else if (ent->client->ps.weapon_usage > 10)
		ent->client->ps.rdflags |= RDF_SKINPLUS;
	
	
//	PlayerNoise(ent, start, PNOISE_WEAPON);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLACKJACK_SWING);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}

void Weapon_Blackjack_Hit (edict_t *ent)
{
	int		damage=75;
	if(CustBlakjakDmg) damage = CustBlakjakDmg;

	Blackjack_Hit (ent, vec3_origin, damage); 
   	ent->client->ps.gunframe++;	
}

void Weapon_Blackjack (edict_t *ent)
{
	static int	pause_frames[]	= {0, 0};
	static int	fire_frames[]	= {8, 11, 0};
	
	
	if (ent->client->ps.gunframe < 5)
	{
		ent->client->ps.rdflags &= ~RDF_SKINPLUS;
		ent->client->ps.rdflags &= ~RDF_SKINPLUSPLUS;
	}
	
	Weapon_Generic (ent, 5, 13, 31, 36, pause_frames, fire_frames, Weapon_Blackjack_Hit);

	ent->client->ps.model_parts[PART_BODY].invisible_objects = 0;
	
}
// END JOSEPH

// JOSEPH 19-JAN-99
void Crowbar_Hit (edict_t *ent, vec3_t vorigin, int damage)
{
	vec3_t		start;
	vec3_t		forward, right;
	int			kick = 20;
	vec3_t		offset;


	AngleVectors (ent->client->v_angle, forward, right, NULL);
	
	VectorSet(offset, 0, 0, ent->viewheight);
	
	VectorAdd (offset, vorigin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (fire_crowbar (ent, start, forward, damage, kick, MOD_CROWBAR))
		ent->client->ps.weapon_usage++;

	if (ent->client->ps.weapon_usage > 20)
	{
		ent->client->ps.rdflags &= ~RDF_SKINPLUS;
		ent->client->ps.rdflags |= RDF_SKINPLUSPLUS;
	}
	else if (ent->client->ps.weapon_usage > 10)
		ent->client->ps.rdflags |= RDF_SKINPLUS;
	
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_CROWBAR_SWING);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}

void Weapon_Crowbar_Hit (edict_t *ent)
{
	int		damage=75;
	if(CustBlakjakDmg) damage = CustBlakjakDmg;

	Crowbar_Hit (ent, vec3_origin, damage); 
   	ent->client->ps.gunframe++;	
}

void Weapon_Crowbar (edict_t *ent)
{
	static int	pause_frames[]	= {0, 0};
	static int	fire_frames[]	= {8, 11, 0};
	
	
	if (ent->client->ps.gunframe < 5)
	{
		ent->client->ps.rdflags &= ~RDF_SKINPLUS;
		ent->client->ps.rdflags &= ~RDF_SKINPLUSPLUS;
	}
	
	Weapon_Generic (ent, 5, 13, 31, 36, pause_frames, fire_frames, Weapon_Crowbar_Hit);

	ent->client->ps.model_parts[PART_BODY].invisible_objects = 0;
	
}
// END JOSEPH 



// JOSEPH 7-JAN-99
void Eject_Pistol_Shell (edict_t *ent)
{
	edict_t	*dropped = NULL;
	vec3_t	forward, right, up;
	vec3_t	offset;

	if (!(ent->client))
		return;

	if (deathmatch->value)
		return;

	//gi.dprintf("angles = %s\n", vtos(ent->s.angles));

	dropped = G_Spawn();
	
	if (!dropped)
		return;

	dropped->s.modelindex = gi.modelindex("models/weapons/shell_md2/tris.md2");
		
	dropped->movetype = MOVETYPE_BOUNCE;

	dropped->mins[0] = -4;
	dropped->mins[1] = -4;
	dropped->mins[2] = -2;
	
	dropped->maxs[0] =  4;
	dropped->maxs[1] =  4;
	dropped->maxs[2] =  4;

	AngleVectors (ent->client->v_angle, forward, right, up);

	dropped->s.angles[YAW] = ent->client->v_angle[YAW];

	VectorSet(offset, 0, 16, ent->viewheight -2 );
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, dropped->s.origin);

	VectorScale (forward, 5 + ((rand()&15)-8), dropped->velocity);
    VectorMA (dropped->velocity, 50 + (rand()&31), right, dropped->velocity);
	dropped->velocity[2] = 150 + (rand()&31);

	// Ridah, changed this so they don't stick in the ground so much
	VectorSet(dropped->avelocity, 0/*((rand()&7)-4)*/, ((rand()&15)-8), ((rand()&7)-4));
	
	dropped->think = Think_Eject_Shell;
	dropped->misstime = 40;
	dropped->nextthink = level.time + 0.1;

	dropped->s.renderfx2 |= RF2_NOSHADOW;

	gi.linkentity (dropped);

	return;
}

// JOSEPH 7-JAN-99
void Eject_Tommy_Shell (edict_t *ent)
{
	edict_t	*dropped = NULL;
	vec3_t	forward, right, up;
	vec3_t	offset;

	if (!(ent->client))
		return;

	if (deathmatch->value)
		return;

	//gi.dprintf("angles = %s\n", vtos(ent->s.angles));

	dropped = G_Spawn();

	if (!dropped)
		return;
	
	dropped->s.modelindex = gi.modelindex("models/weapons/shell_md2/tris.md2");
		
	dropped->movetype = MOVETYPE_BOUNCE;
	
	dropped->mins[0] = -4;
	dropped->mins[1] = -4;
	dropped->mins[2] = -2;
	
	dropped->maxs[0] =  4;
	dropped->maxs[1] =  4;
	dropped->maxs[2] =  4;

	AngleVectors (ent->client->v_angle, forward, right, up);

	dropped->s.angles[YAW] = ent->client->v_angle[YAW];

	VectorSet(offset, 0, 16, ent->viewheight -2 );
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, dropped->s.origin);

	VectorScale (forward, 5 + ((rand()&15)-8), dropped->velocity);
    VectorMA (dropped->velocity, 50 + (rand()&31), right, dropped->velocity);
	dropped->velocity[2] = 150 + (rand()&31);

	// Ridah, changed this so they don't stick in the ground so much
	VectorSet(dropped->avelocity, 0/*((rand()&7)-4)*/, ((rand()&15)-8), ((rand()&7)-4));

	dropped->think = Think_Eject_Shell;
	dropped->misstime = 40;
	dropped->nextthink = level.time + 0.1;
	
	dropped->s.renderfx2 |= RF2_NOSHADOW;

	gi.linkentity (dropped);

	return;
}
// END JOSEPH

// WEAPON PISTOL
void Pistol_Fire (edict_t *ent, vec3_t vorigin, int damage)
{
	vec3_t		start;
	vec3_t		forward, right;
	int			kick = 80; // Frank changed this from 2;
	vec3_t		offset;

	// Frank underwater damage is halfed.
	if (ent->waterlevel >= 2)
		damage /= 2;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	
	VectorSet(offset, 0, 0, ent->viewheight);
	
	VectorAdd (offset, vorigin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);

	ent->client->kick_angles[0] = -4;
    
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_PISTOL);
	
	if (!(ent->waterlevel >= 2)) // Frank ADDED THIS FOR UNDERWATER BULLETS.
	{
		if (ent->client->pers.pistol_mods & WEAPON_MOD_DAMAGE)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_PISTOL_DAMAGE_MOD | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
		else
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_PISTOL | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
	
		PlayerNoise(ent, start, PNOISE_WEAPON);
	}

	else//must be underwater
	gi.sound(ent, CHAN_VOICE, gi.soundindex("actors/player/male/watr_un.wav"), 1, ATTN_NORM, 0);
	//END FRANK

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
	{
		ent->client->pers.weapon_clip[ent->client->clip_index]--;
	}

	ent->client->gun_noise = true;

}

void Weapon_Pistol_Fire (edict_t *ent)
{
	int		damage = 42; // Frank changed this from 15;
	int		rounds;

	if(CustPistolDmg) damage = CustPistolDmg;

	if (ent->client->ps.gunframe == 6)
	{
		if (ent->client->pers.pistol_mods & WEAPON_MOD_DAMAGE)		damage *= 2;

		if (ent->client->pers.pistol_mods & WEAPON_MOD_ROF)			ent->client->ps.gunframe = 8;	

		else														ent->client->ps.gunframe++;	

		Pistol_Fire (ent, vec3_origin, damage); 
	}

	else if (ent->client->ps.gunframe == 10)
	{
		if (!ent->client->pers.weapon_clip[ent->client->clip_index])
		{
			
			if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_PISTOL_ROUNDS)
			{
				rounds = ent->client->pers.inventory[ent->client->ammo_index];
			}
			else 
				rounds = MAX_PISTOL_ROUNDS;

			ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
			ent->client->pers.inventory[ent->client->ammo_index] -= rounds;

			ent->client->ps.gunframe++;	
			
			if (ent->client->pers.pistol_mods & WEAPON_MOD_RELOAD) ////gi.dprintf ("need fast reload sound\n");//OK Frank
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/pistol/clip_in.wav"), 1, ATTN_NORM, 0);
			else
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/pistol/clip_out.wav"), 1, ATTN_NORM, 0);

			if (! ent->client->pers.weapon_clip[ent->client->clip_index])
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
			ent->client->ps.gunframe = 29;
	}

}

void Weapon_Pistol (edict_t *ent)
{
	int		rounds;
	//static int	pause_frames[]	= {46, 0, 0};
	static int	pause_frames[]	= {0, 0, 0};
	static int	fire_frames[]	= {6, 10, 0};

	if (ent->client->reload_weapon)
	{
		if (ent->client->ps.gunframe >= 28 && ent->client->ps.gunframe <= 44)
		{
			if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_PISTOL_ROUNDS)
			{
				ent->client->ps.gunframe = 10;
				ent->client->weaponstate = WEAPON_RELOADING;
				
				ent->client->pers.inventory[ent->client->ammo_index] += ent->client->pers.weapon_clip[ent->client->clip_index];	

				if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_PISTOL_ROUNDS)
				{
					rounds = ent->client->pers.inventory[ent->client->ammo_index];
				}
				else 
					rounds = MAX_PISTOL_ROUNDS;

				ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
				ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
			}

			ent->client->reload_weapon = false;
		}
		else
			ent->client->reload_weapon = false;
	}
	else if (!ent->client->pers.weapon_clip[ent->client->clip_index] && ent->client->pers.inventory[ent->client->ammo_index])
	{
		if (ent->client->ps.gunframe >= 28 && ent->client->ps.gunframe <= 44)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 10;
			ent->client->weaponstate = WEAPON_RELOADING;

			if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_PISTOL_ROUNDS)
			{
				rounds = ent->client->pers.inventory[ent->client->ammo_index];
			}
			else 
				rounds = MAX_PISTOL_ROUNDS;

			ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
			ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
		}
	}
	
	if (ent->client->pers.pistol_mods & WEAPON_MOD_DAMAGE)
		ent->client->ps.gunindex = gi.modelindex("models/weapons/v_colt/magnum.mdx");

	Weapon_Generic (ent, 5, 28, 44, 49, pause_frames, fire_frames, Weapon_Pistol_Fire);

	if (ent->client->pers.pistol_mods & WEAPON_MOD_RELOAD)
	{
		if (ent->client->ps.gunframe == 21)
			ent->client->ps.gunframe = 26;
	}
	
	if (ent->client->ps.gunframe >= 10 && ent->client->ps.gunframe <= 28) 
	{
		ent->client->ps.model_parts[PART_BODY].invisible_objects = 0;
	}
	else
		ent->client->ps.model_parts[PART_BODY].invisible_objects = (1<<0 | 1<<1);

	if (ent->client->ps.gunframe == 1)
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/pistol/holster.wav"), 1, ATTN_NORM, 0);

	if (ent->client->ps.gunframe == 11)
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/pistol/clip_out.wav"), 1, ATTN_NORM, 0);

	if (ent->client->ps.gunframe == 18)
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/pistol/clip_in.wav"), 1, ATTN_NORM, 0);

	if (ent->client->ps.gunframe >= 10 && ent->client->ps.gunframe <= 24) 
	{
		ent->client->ps.model_parts[PART_GUN].invisible_objects = 0;
	}
	else
		ent->client->ps.model_parts[PART_GUN].invisible_objects = (1<<0 | 1<<1);

}



void SPistol_Fire (edict_t *ent, vec3_t vorigin, int damage)
{
	vec3_t		start;
	vec3_t		forward, right;
	int			kick = 40; // Frank changed from 2;
	vec3_t		offset;

	//FRANK underwater damage is halfed.
	if (ent->waterlevel >= 2)
		damage /= 2;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	
	VectorSet(offset, 0, 0, ent->viewheight);
	
	VectorAdd (offset, vorigin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	
	ent->client->kick_angles[0] = -4;
    
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_SILENCER);

	if (!(ent->waterlevel >= 2))//ADDED BY FRANK 
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_SPISTOL | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}

	else //must be underwater
		gi.sound(ent, CHAN_VOICE, gi.soundindex("actors/player/male/watr_un.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
		ent->client->pers.weapon_clip[ent->client->clip_index]--;

	if (!ent->client->pers.weapon_clip[ent->client->clip_index] && !ent->client->pers.inventory[ ent->client->ammo_index])
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}
    
	// Eject_Pistol_Shell(ent); // Frank whacked this out to save some bw
	ent->client->gun_noise = false;
}


void Weapon_SPistol_Fire (edict_t *ent)
{
	int		damage = 30; // Frank changed this from 15
	int		rounds;

	if(CustPistolDmg) damage = CustPistolDmg;

	if (ent->client->ps.gunframe == 21)
	{
		if (ent->client->pers.pistol_mods & WEAPON_MOD_ROF)
		{
			ent->client->ps.gunframe = 23;	
		}
		else
		{
			ent->client->ps.gunframe++;	
		}

		if (ent->client->pers.pistol_mods & WEAPON_MOD_DAMAGE)
		{
			damage *= 2;
		}

		SPistol_Fire (ent, vec3_origin, damage); 
		
		ent->client->pers.silencer_shots--;

		if (!ent->client->pers.silencer_shots)
			ent->client->ps.gunframe = 43;

	}
	else if (ent->client->ps.gunframe == 25)
	{
		if (!ent->client->pers.weapon_clip[ent->client->clip_index])
		{
			
			if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_PISTOL_ROUNDS)
			{
				rounds = ent->client->pers.inventory[ent->client->ammo_index];
			}
			else 
				rounds = MAX_PISTOL_ROUNDS;

			ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
			ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
			ent->client->ps.gunframe++;	
			if (ent->client->pers.pistol_mods & WEAPON_MOD_RELOAD)
				;//gi.dprintf ("need fast reload sound\n");
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/pistol/clip_out.wav"), 1, ATTN_NORM, 0);

			if (! ent->client->pers.weapon_clip[ent->client->clip_index])
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
			ent->client->ps.gunframe = 50;
	}
	
	else if (ent->client->ps.gunframe == 44) 
	{
		if (!(ent->client->pers.silencer_shots))
			ent->client->ps.gunframe++;
		else
			ent->client->ps.gunframe = 50;
	}
	

}


void Weapon_SPistol (edict_t *ent)
{
	static int	pause_frames[]	= {0, 0, 0};
	static int	fire_frames[]	= {21, 25, 44, 0};
	int		rounds;

// gi.dprintf ("silencer shots left: %d\n", ent->client->pers.silencer_shots);

	if (ent->client->ps.gunframe < 22)
	{
		ent->client->ps.model_parts[PART_BODY].invisible_objects = 0;
	}
	else if (ent->client->weaponstate == WEAPON_RELOADING_SPISTOL && ent->client->ps.gunframe < 40)
	{
		if (ent->client->ps.gunframe < 39)
		ent->client->ps.model_parts[PART_GUN].invisible_objects = 0;
		else
			ent->client->ps.model_parts[PART_GUN].invisible_objects = (1<<0 | 1<<1);

		ent->client->ps.model_parts[PART_BODY].invisible_objects = 0;
	}
	else if (ent->client->pers.holsteredweapon)
	{
		ent->client->ps.model_parts[PART_GUN].invisible_objects = (1<<0 | 1<<1);
		ent->client->ps.model_parts[PART_BODY].invisible_objects = (1<<0 | 1<<1);
	}
	else
	{
		ent->client->ps.model_parts[PART_GUN].invisible_objects = (1<<0 | 1<<1);
		ent->client->ps.model_parts[PART_BODY].invisible_objects = (1<<0 | 1<<1);
	}

	if (ent->client->ps.gunframe == 49)
	{
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("pistol"))] )
		{
			ent->client->newweapon = FindItem ("pistol");
			
		}
		else
			NoAmmoWeaponChange (ent);

		ent->client->ps.gunframe = 71;
		ent->client->weaponstate = WEAPON_DROPPING;
		{
			gitem_t *it;
			int		index;
			it = FindItem ("SPistol");
			index = ITEM_INDEX (it);
			ent->client->pers.inventory[index] = 0;
		}
	}
	else if (ent->client->reload_weapon)
	{
		if (ent->client->ps.gunframe >= 50 && ent->client->ps.gunframe <= 65)
		{
		
			if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_PISTOL_ROUNDS)
			{
				ent->client->ps.gunframe = 25;
				ent->client->weaponstate = WEAPON_RELOADING_SPISTOL;
				
				ent->client->pers.inventory[ent->client->ammo_index] += ent->client->pers.weapon_clip[ent->client->clip_index];	

				if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_PISTOL_ROUNDS)
				{
					rounds = ent->client->pers.inventory[ent->client->ammo_index];
				}
				else 
					rounds = MAX_PISTOL_ROUNDS;

				ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
				ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
			}

			ent->client->reload_weapon = false;
		}
	}
	else if (!ent->client->pers.weapon_clip[ent->client->clip_index] && ent->client->pers.inventory[ent->client->ammo_index])
	{
		
		if (ent->client->ps.gunframe >= 50 && ent->client->ps.gunframe <= 65)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 25;
			ent->client->weaponstate = WEAPON_RELOADING_SPISTOL;

			if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_PISTOL_ROUNDS)
			{
				rounds = ent->client->pers.inventory[ent->client->ammo_index];
			}
			else 
				rounds = MAX_PISTOL_ROUNDS;

			ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
			ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
		}
	}

	if (ent->client->pers.pistol_mods & WEAPON_MOD_DAMAGE)
		ent->client->ps.gunindex = gi.modelindex("models/weapons/silencer_mdx/magnum.mdx");
			

	Weapon_Generic (ent, 20, 49, 65, 75, pause_frames, fire_frames, Weapon_SPistol_Fire);
		
	if (ent->client->ps.gunframe == 1)
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/pistol/silencerattatch.wav"), 1, ATTN_NORM, 0);
	
	
}


// JOSEPH 26-AUG-98 
/*
======================================================================

TOMMYGUN 

======================================================================
*/

void Tommygun_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	//vec3_t	fireorigin;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 22; // Frank changed this from 8 * 1.75 = 14;
	int			kick = 50; // Frank changed this from 2;
	vec3_t		offset;


	if(CustTommyDmg) damage = CustTommyDmg;

	if (ent->waterlevel >= 2)
	{
		// gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);	
		if (ent->client->weaponstate == WEAPON_FIRING)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 30;
		}
		return;
	}

	// First shot
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		// ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe = 30;
		return;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] == 0 && ent->client->pers.weapon_clip[ent->client->clip_index] == 0) 
		return;

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

	// Tommy is too powerful in real-mode
	if (dm_realmode->value)
		damage /= 2;

	// Bounce
	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.22;
		ent->client->kick_angles[i] = crandom() * 0.6;
	}
	ent->client->kick_origin[0] = crandom() * 0.22;
	// ent->client->kick_angles[0] = ent->client->machinegun_shots * -.5;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	
	VectorSet(offset, 0, 0, ent->viewheight-1);
	
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	// Disable for unlimited ammo
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
	{
		ent->client->pers.weapon_clip [ent->client->clip_index]--;

		if (!(ent->client->pers.inventory[ent->client->ammo_index]) && !ent->client->pers.weapon_clip [ent->client->clip_index])
		{
			NoAmmoWeaponChange (ent);
			ent->client->ps.gunframe = 30;		
		}
		else if (!ent->client->pers.weapon_clip [ent->client->clip_index])
		{
			ent->client->reload_weapon = true;
			ent->client->ps.gunframe = 30;		
			// gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/machinegun/machgcock.wav"), 1, ATTN_NORM, 0);
		}
	}

	// if (!deathmatch->value)	// uses up bandwidth //Frank wacked it out, save the if statement.
    // Eject_Tommy_Shell(ent);

	ent->client->gun_noise = true;
}

void Weapon_Tommygun (edict_t *ent)
{
	int		rounds;
	static int	pause_frames[]	= {44, 0, 0};
	static int	fire_frames[]	= {4, 5, 0};
	
	if (ent->client->pers.inventory[ent->client->ammo_index] == 0 && ent->client->pers.weapon_clip[ent->client->clip_index] == 0
		&& ent->client->weaponstate != WEAPON_DROPPING) 
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
	}
	else if (ent->client->reload_weapon)
	{
		if (ent->client->ps.gunframe >= 30 && ent->client->ps.gunframe <= 40)
		{
			if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_TOMMYGUN_ROUNDS)
			{
				ent->client->ps.gunframe = 6;
				ent->client->weaponstate = WEAPON_RELOADING;
				
				ent->client->pers.inventory[ent->client->ammo_index] += ent->client->pers.weapon_clip[ent->client->clip_index];	

				if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_TOMMYGUN_ROUNDS)
				{
					rounds = ent->client->pers.inventory[ent->client->ammo_index];
				}
				else 
					rounds = MAX_TOMMYGUN_ROUNDS;

				ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
				ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/machinegun/machgcock.wav"), 1, ATTN_NORM, 0);
			}

			ent->client->reload_weapon = false;
		}
	}
	else if (!ent->client->pers.weapon_clip[ent->client->clip_index] && ent->client->pers.inventory[ent->client->ammo_index])
	{
		if (ent->client->ps.gunframe >= 30 && ent->client->ps.gunframe <= 40)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 6;
			ent->client->weaponstate = WEAPON_RELOADING;

			if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_TOMMYGUN_ROUNDS)
			{
				rounds = ent->client->pers.inventory[ent->client->ammo_index];
			}
			else 
				rounds = MAX_TOMMYGUN_ROUNDS;

			ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
			ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/machinegun/machgcock.wav"), 1, ATTN_NORM, 0);
		}
	}

	Weapon_Generic (ent, 3, 29, 40, 46, pause_frames, fire_frames, Tommygun_Fire);


}
// END JOSEPH



/*
======================================================================

  FLAMETHROWER

======================================================================
*/

void FlameThrower_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 2;
	int			kick = 0;
	static int flamesnd = 0;


	if(CustFlameDmg) damage = CustFlameDmg;

	if (ent->waterlevel >= 2)
	{
		// gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);	
		if (ent->client->weaponstate == WEAPON_FIRING)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 14;
			ent->s.renderfx2 &= ~RF2_FLAMETHROWER;
		}
		return;
	}

//	if (deathmatch->value && rand()%2)
//		damage = 1;

	if (ent->health <= 0)
		return;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		// ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe = 14;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/flame_thrower/flameend.wav"), 1, ATTN_NORM, 0);
		flamesnd = 0;
		ent->s.renderfx2 &= ~RF2_FLAMETHROWER;
		return;
	}

	ent->client->ps.gunframe ++ ;

	if (ent->client->ps.gunframe == 14)
		ent->client->ps.gunframe = 6;
	
	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6; 
		
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		ent->s.renderfx2 &= ~RF2_FLAMETHROWER;
		return;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	
	VectorCopy( ent->s.origin, start );
	
	fire_flamethrower (ent, start, forward, damage, kick, MOD_FLAMETHROWER);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->s.renderfx2 |= RF2_FLAMETHROWER;

	flamesnd++;
	
	if (flamesnd > 2)
		flamesnd = 0;

	if (flamesnd == 1)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/flame_thrower/flame2.wav"), 1, ATTN_NORM, 0);
	else if (flamesnd == 2)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/flame_thrower/flame3.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/flame_thrower/flame1.wav"), 1, ATTN_NORM, 0);


	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
	{
		if ((ent->client->pers.inventory[ent->client->ammo_index] -= 2) < 0)
			ent->client->pers.inventory[ent->client->ammo_index] = 0;
	}

	// RAFAEL	28-dec-98
	ent->client->gun_noise = true;


}

void Weapon_FlameThrower (edict_t *ent)
{
	static int	pause_frames[]	= {18, 24, 0};

	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 12, 13};

		Weapon_Generic (ent, 5, 13, 25, 31, pause_frames, fire_frames, FlameThrower_Fire);
	
}

/*
	SPAZ // assault2 virgin16
*/

void spaz_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 28; // frank changed this from 8;
	int			kick = 80; // frank changed this from 50;

	if(CustShotgunDmg) damage = CustShotgunDmg;

	//underwater damage is halved.
    if (ent->waterlevel >= 2)
		damage /= 2;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorSet(offset, 0, 0,  ent->viewheight-2);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if(!strcmp(ent->client->pers.weapon->pickup_name, "Spaz"))
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SPAZ);
	else
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_PUMP);

	VectorScale (forward, -2, ent->client->kick_origin);
	
	if (!(ent->waterlevel >= 2)) // Frank added this to save the bw
		{
			// send muzzle flash
//			gi.WriteByte (svc_muzzleflash);
//			gi.WriteShort (ent-g_edicts);
//			gi.WriteByte (MZ_SHOTGUN | is_silenced);
//			gi.multicast (ent->s.origin, MULTICAST_PVS);
//			PlayerNoise(ent, start, PNOISE_WEAPON);
			if(!strcmp(ent->client->pers.weapon->pickup_name, "Spaz"))
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/spaz/spaz.wav"), 1, ATTN_NORM, 0);
			else if(!strcmp(ent->client->pers.weapon->pickup_name, "Pump")) // virgin17
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/pump/pump.wav"), 1, ATTN_NORM, 0);

		}
	else // must be underwater Frank so sound like it.
		gi.sound(ent, CHAN_VOICE, gi.soundindex("actors/player/male/watr_un.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
		ent->client->pers.weapon_clip[ent->client->clip_index]--;
	
	// Eject_Shotgun_Shell(ent); // save the bandwidth. Frank
	ent->client->gun_noise = true;

}

void Weapon_Spaz_Fire (edict_t *ent)
{
	if (ent->client->ps.gunframe == 6 || ent->client->ps.gunframe == 8)
	{
		if (ent->client->pers.weapon_clip[ent->client->clip_index])
			spaz_fire (ent); 

		else
		{
			ent->client->ps.gunframe = 17;
			ent->client->reload_weapon = true;
			ent->client->weaponstate = WEAPON_RELOAD_CYCLE;
		}
	}
	else if (ent->client->ps.gunframe == 16)
	{
		if (!ent->client->pers.weapon_clip[ent->client->clip_index])
		{
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe ++;
				ent->client->reload_weapon = true;
				ent->client->weaponstate = WEAPON_RELOAD_CYCLE;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			ent->client->ps.gunframe = 33;
		}
	}
}

void Weapon_Spaz (edict_t *ent)
{
	static int	pause_frames[]	= {0, 0};
	static int	fire_frames[]	= {6, 8, 16, 0};
	static int  kick_frames[]   = {-12, -9, -6, -4, -1};

	if ((ent->client->ps.gunframe >= 6) & (ent->client->ps.gunframe <= 9))
	{
		ent->client->kick_angles[0] = kick_frames[ent->client->ps.gunframe-6];
	}
	
	if (ent->client->ps.gunframe == 7)
		ent->client->ps.rdflags |= RDF_NOLERP;
	else
		ent->client->ps.rdflags &= ~RDF_NOLERP;

	if (ent->client->reload_weapon)
	{
		if (ent->client->buttons & BUTTON_ATTACK && ent->client->pers.weapon_clip[ent->client->clip_index] >= 1 && ent->client->weaponstate != WEAPON_FIRING)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 33;
			ent->client->weaponstate = WEAPON_READY;
		}
		else if (ent->client->newweapon)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 33;
			ent->client->weaponstate = WEAPON_DROPPING;
		}
		else
		{
			if (ent->client->ps.gunframe == 29)
			{
				if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_SHOTGUN_ROUNDS)
				{
					if (ent->client->pers.inventory[ent->client->ammo_index])
					{
                       //HOGIE'S FIX LOOKS LIKE THIS FRANK

                       //do we have at least 4 shells in our pocket?     
                       if(ent->client->pers.inventory[ent->client->ammo_index]<4)
					   {    //less than 4? Then add what you got to your gun you lamer.
						   if(ent->client->pers.weapon_clip[ent->client->clip_index]+ent->client->pers.inventory[ent->client->ammo_index]>8)
						   {
                           ent->client->pers.inventory[ent->client->ammo_index]--;    //take one shell out of my pocket.
                           ent->client->pers.weapon_clip[ent->client->clip_index]++;  //put that one shell inside my gun.
						   } 
                           else 
						   {   
                           ent->client->pers.weapon_clip[ent->client->clip_index]+=ent->client->pers.inventory[ent->client->ammo_index]; 
                           //now be sure your pocket reads EMPTY.
                           ent->client->pers.inventory[ent->client->ammo_index] = 0;
						   }
					   }

                       //well since you got more than 4 in your pocket, take 4 out and put 4 in your gun.
                       else 
					   { 
	                             //check to see if you got more than 4 in your gun already.
                                 if(ent->client->pers.weapon_clip[ent->client->clip_index]>4)
								 {
                                    ent->client->pers.inventory[ent->client->ammo_index]--;    //take one shell out of my pocket.
                                    ent->client->pers.weapon_clip[ent->client->clip_index]++;  //put that one shell inside my gun.
								 }  

                           //since you dont got 5 or more shells in your gun but you do have 4 or more in your pocket, its all good.
                                 else
								 {
                                    ent->client->pers.inventory[ent->client->ammo_index] -= 4; //take four shells out of your pocket.     
                                    ent->client->pers.weapon_clip[ent->client->clip_index]+=4; //add four shells to your gun.
								 }
					   }  

						if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_SHOTGUN_ROUNDS)
							ent->client->ps.gunframe = 21; 
						else
						{
							ent->client->reload_weapon = false;
							ent->client->ps.gunframe = 29; 
							ent->client->weaponstate = WEAPON_RELOADING;
						}
					}
					else
					{
						ent->client->reload_weapon = false;
						ent->client->weaponstate = WEAPON_RELOADING;
					}
				}
				else
				{
					ent->client->reload_weapon = false;
					ent->client->weaponstate = WEAPON_RELOADING;
				}
			}///////////////////////////////////////////FRANK LOOK HERE
			else if (ent->client->ps.gunframe >= 33 && ent->client->ps.gunframe <= 40)
			{
				if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_SHOTGUN_ROUNDS &&
					ent->client->pers.inventory[ent->client->ammo_index])
				{
					ent->client->weaponstate = WEAPON_RELOAD_CYCLE;
					ent->client->ps.gunframe = 17;
				}
			}
		}
	}
	else if (!ent->client->pers.weapon_clip[ent->client->clip_index] && ent->client->weaponstate != WEAPON_DROPPING
		&& !ent->client->buttons & BUTTON_ATTACK)
	{
		ent->client->reload_weapon = true;
		ent->client->weaponstate = WEAPON_RELOAD_CYCLE;
		ent->client->ps.gunframe = 17;
		
		if (!ent->client->pers.inventory[ent->client->ammo_index])
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 33;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
	
	if (ent->client->ps.gunframe == 21)
	{
		if(!strcmp(ent->client->pers.weapon->pickup_name, "Spaz"))
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/spaz/spazload.wav"), 1, ATTN_NORM, 0);
		else if(!strcmp(ent->client->pers.weapon->pickup_name, "Pump")) // virgin17
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/pump/pumpload.wav"), 1, ATTN_NORM, 0);

	} // assault2 virgin16

	Weapon_Generic (ent, 5, 32, 40, 45, pause_frames, fire_frames, Weapon_Spaz_Fire);
}

/*
	SHOTGUN
*/

void shotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 28; // frank changed this from 8;
	int			kick = 80; // frank changed this from 50;

	if(CustShotgunDmg) damage = CustShotgunDmg;

	//underwater damage is halved.
    if (ent->waterlevel >= 2)
		damage /= 2;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorSet(offset, 0, 0,  ent->viewheight-2);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SAWED_SHOTGUN_COUNT, MOD_SHOTGUN);

	VectorScale (forward, -2, ent->client->kick_origin);
	
	if (!(ent->waterlevel >= 2)) // Frank added this to save the bw
		{
			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_SHOTGUN | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
			PlayerNoise(ent, start, PNOISE_WEAPON);
		}
	else // must be underwater Frank so sound like it.
		gi.sound(ent, CHAN_VOICE, gi.soundindex("actors/player/male/watr_un.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
		ent->client->pers.weapon_clip[ent->client->clip_index]--;
	
	// Eject_Shotgun_Shell(ent); // save the bandwidth. Frank
	ent->client->gun_noise = true;

}

void Weapon_Shotgun_Fire (edict_t *ent)
{
	if (ent->client->ps.gunframe == 6)
	{
		if (ent->client->pers.weapon_clip[ent->client->clip_index])
			shotgun_fire (ent);
		else
		{
			ent->client->ps.gunframe = 17;
			ent->client->reload_weapon = true;
			ent->client->weaponstate = WEAPON_RELOAD_CYCLE;
		}
	}
	else if (ent->client->ps.gunframe == 16)
	{
		if (!ent->client->pers.weapon_clip[ent->client->clip_index])
		{
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe ++;
				ent->client->reload_weapon = true;
				ent->client->weaponstate = WEAPON_RELOAD_CYCLE;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			ent->client->ps.gunframe = 33;
		}
	}
}

void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {0, 0};
	static int	fire_frames[]	= {6, 16, 0};
	static int  kick_frames[]   = {-12, -9, -6, -4, -1};

	if ((ent->client->ps.gunframe >= 6) & (ent->client->ps.gunframe <= 9))
	{
		ent->client->kick_angles[0] = kick_frames[ent->client->ps.gunframe-6];
	}
	
	if (ent->client->ps.gunframe == 7)
		ent->client->ps.rdflags |= RDF_NOLERP;
	else
		ent->client->ps.rdflags &= ~RDF_NOLERP;

	if (ent->client->reload_weapon)
	{
		if (ent->client->buttons & BUTTON_ATTACK && ent->client->pers.weapon_clip[ent->client->clip_index] >= 1 && ent->client->weaponstate != WEAPON_FIRING)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 33;
			ent->client->weaponstate = WEAPON_READY;
		}
		else if (ent->client->newweapon)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 33;
			ent->client->weaponstate = WEAPON_DROPPING;
		}
		else
		{
			if (ent->client->ps.gunframe == 29)
			{
				if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_SHOTGUN_ROUNDS)
				{
					if (ent->client->pers.inventory[ent->client->ammo_index])
					{
                       //HOGIE'S FIX LOOKS LIKE THIS FRANK

                       //do we have at least 4 shells in our pocket?     
                       if(ent->client->pers.inventory[ent->client->ammo_index]<4)
					   {    //less than 4? Then add what you got to your gun you lamer.
						   if(ent->client->pers.weapon_clip[ent->client->clip_index]+ent->client->pers.inventory[ent->client->ammo_index]>8)
						   {
                           ent->client->pers.inventory[ent->client->ammo_index]--;    //take one shell out of my pocket.
                           ent->client->pers.weapon_clip[ent->client->clip_index]++;  //put that one shell inside my gun.
						   } 
                           else 
						   {   
                           ent->client->pers.weapon_clip[ent->client->clip_index]+=ent->client->pers.inventory[ent->client->ammo_index]; 
                           //now be sure your pocket reads EMPTY.
                           ent->client->pers.inventory[ent->client->ammo_index] = 0;
						   }
					   }

                       //well since you got more than 4 in your pocket, take 4 out and put 4 in your gun.
                       else 
					   { 
	                             //check to see if you got more than 4 in your gun already.
                                 if(ent->client->pers.weapon_clip[ent->client->clip_index]>4)
								 {
                                    ent->client->pers.inventory[ent->client->ammo_index]--;    //take one shell out of my pocket.
                                    ent->client->pers.weapon_clip[ent->client->clip_index]++;  //put that one shell inside my gun.
								 }  

                           //since you dont got 5 or more shells in your gun but you do have 4 or more in your pocket, its all good.
                                 else
								 {
                                    ent->client->pers.inventory[ent->client->ammo_index] -= 4; //take four shells out of your pocket.     
                                    ent->client->pers.weapon_clip[ent->client->clip_index]+=4; //add four shells to your gun.
								 }
					   }  

						if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_SHOTGUN_ROUNDS)
							ent->client->ps.gunframe = 21; 
						else
						{
							ent->client->reload_weapon = false;
							ent->client->ps.gunframe = 29; 
							ent->client->weaponstate = WEAPON_RELOADING;
						}
					}
					else
					{
						ent->client->reload_weapon = false;
						ent->client->weaponstate = WEAPON_RELOADING;
					}
				}
				else
				{
					ent->client->reload_weapon = false;
					ent->client->weaponstate = WEAPON_RELOADING;
				}
			}///////////////////////////////////////////FRANK LOOK HERE
			else if (ent->client->ps.gunframe >= 33 && ent->client->ps.gunframe <= 40)
			{
				if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_SHOTGUN_ROUNDS &&
					ent->client->pers.inventory[ent->client->ammo_index])
				{
					ent->client->weaponstate = WEAPON_RELOAD_CYCLE;
					ent->client->ps.gunframe = 17;
				}
			}
		}
	}
	else if (!ent->client->pers.weapon_clip[ent->client->clip_index] && ent->client->weaponstate != WEAPON_DROPPING
		&& !ent->client->buttons & BUTTON_ATTACK)
	{
		ent->client->reload_weapon = true;
		ent->client->weaponstate = WEAPON_RELOAD_CYCLE;
		ent->client->ps.gunframe = 17;
		
		if (!ent->client->pers.inventory[ent->client->ammo_index])
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 33;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
	
	if (ent->client->ps.gunframe == 21)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/shotgun/shotgload.wav"), 1, ATTN_NORM, 0);
	}

	Weapon_Generic (ent, 5, 32, 40, 45, pause_frames, fire_frames, Weapon_Shotgun_Fire);
}

/*
BAR .308
*/
void weapon_barmachinegun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage =  100;
	int			kick = 150; // Frank from = 50;

	if(CustHMGDmg) damage = CustHMGDmg;

	if (ent->waterlevel >= 2)
	{
		if (ent->client->weaponstate == WEAPON_FIRING)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 43;
		}
		return;
	}


	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;
	VectorSet(offset, 0, 0,  ent->viewheight-1);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_bullet (ent, start, forward, damage, kick, 0, 0, MOD_BARMACHINEGUN);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hmg/single.wav"), 1, ATTN_NORM, 0); // HOGIE for his single shot

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
	{
		ent->client->pers.weapon_clip [ent->client->clip_index]--;

		if (!(ent->client->pers.inventory[ent->client->ammo_index]) && !ent->client->pers.weapon_clip [ent->client->clip_index])
		{
			NoAmmoWeaponChange (ent);
			ent->client->ps.gunframe = 43;		
		}
		else if (ent->client->pers.inventory[ent->client->ammo_index] && !(ent->client->pers.weapon_clip [ent->client->clip_index]))
		{
			ent->client->reload_weapon = true;
			ent->client->ps.gunframe = 43;		
		}
		
	}
  
	ent->client->gun_noise = true;

}

void barmachinegun_fire (edict_t *ent)
{

	if (ent->client->ps.gunframe == 19) //original code, if your normal hmg, go slow , wait for frame 19.
	{
		ent->client->ps.gunframe = 43;
		return;
	}

	else if (ent->client->ps.gunframe == 19) // HOGIE
	{
		if (ent->client->pers.pistol_mods & WEAPON_MOD_COOLING_JACKET)
		{
			ent->client->ps.gunframe = 43;
		}
	}

	else if (ent->client->ps.gunframe == 5 ) // HOGIE
	{
		weapon_barmachinegun_fire (ent);
	
		if (ent->client->pers.pistol_mods & WEAPON_MOD_COOLING_JACKET)
		{
			ent->client->pers.hmg_shots --;

			if (!ent->client->pers.hmg_shots)
			{
				ent->client->pers.pistol_mods &= ~WEAPON_MOD_COOLING_JACKET;
				
				ent->client->newweapon = ent->client->pers.weapon;
				
				if (ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.weapon_clip[ent->client->clip_index])
					ChangeWeapon (ent);
				else
					NoAmmoWeaponChange (ent);
			}
		}
	}
	
	else if ( (ent->client->ps.gunframe == 7) && (ent->client->pers.pistol_mods & WEAPON_MOD_COOLING_JACKET) ) // HOGIE double shot
	{
		weapon_barmachinegun_fire (ent);
		ent->client->pers.hmg_shots --;
		if (!ent->client->pers.hmg_shots)
		{
			ent->client->pers.pistol_mods &= ~WEAPON_MOD_COOLING_JACKET;
			
			ent->client->newweapon = ent->client->pers.weapon;
			
			if (ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.weapon_clip[ent->client->clip_index])
				ChangeWeapon (ent);
			else
				NoAmmoWeaponChange (ent);
		}
	}

	else if (ent->client->ps.gunframe == 7) ent->client->ps.gunframe = 10; // HOGIE for single shot HMG


ent->client->ps.gunframe++;
	
}

#define MAX_BARGUN_ROUNDS	30

void Weapon_Barmachinegun (edict_t *ent)
{
	static int	pause_frames[]	= {0, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 12, 19, 0};
	int			rounds;
	
	if (ent->client->pers.inventory[ent->client->ammo_index] == 0 && ent->client->pers.weapon_clip[ent->client->clip_index] == 0
		&& ent->client->weaponstate != WEAPON_DROPPING) 
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
	}
	else if (ent->client->reload_weapon)
	{
		if (ent->client->ps.gunframe >= 43 && ent->client->ps.gunframe <= 53)
		{
			if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_BARGUN_ROUNDS)
			{
				ent->client->ps.gunframe = 20;
				ent->client->weaponstate = WEAPON_RELOADING;
				
				ent->client->pers.inventory[ent->client->ammo_index] += ent->client->pers.weapon_clip[ent->client->clip_index];	

				if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_BARGUN_ROUNDS)
				{
					rounds = ent->client->pers.inventory[ent->client->ammo_index];
				}
				else 
					rounds = MAX_BARGUN_ROUNDS;

				ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
				ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hmg/hmgcock.wav"), 1, ATTN_NORM, 0);
			}

			ent->client->reload_weapon = false;
		}
	}
	else if (!ent->client->pers.weapon_clip[ent->client->clip_index] && ent->client->pers.inventory[ent->client->ammo_index])
	{
		if (ent->client->ps.gunframe >= 43 && ent->client->ps.gunframe <= 53)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 20;
			ent->client->weaponstate = WEAPON_RELOADING;

			if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_BARGUN_ROUNDS)
			{
				rounds = ent->client->pers.inventory[ent->client->ammo_index];
			}
			else 
				rounds = MAX_BARGUN_ROUNDS;

			ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
			ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hmg/hmgcock.wav"), 1, ATTN_NORM, 0);
		}
	}

	
	Weapon_Generic (ent, 4, 42, 53, 59, pause_frames, fire_frames, barmachinegun_fire);

 	//gi.dprintf ("Frame: %d\n", ent->client->ps.gunframe); // test code
}


/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void Eject_Grenade_Shell (edict_t *ent)
{
	edict_t	*dropped = NULL;
	vec3_t	forward, right, up;
	vec3_t	offset;

	if (!(ent->client))
		return;

	if (deathmatch->value)
		return;

	//gi.dprintf("angles = %s\n", vtos(ent->s.angles));

	dropped = G_Spawn();
	
	if (!dropped)
		return;

	dropped->s.modelindex = gi.modelindex("models/weapons/grenade_shell/tris.md2");
		
	dropped->movetype = MOVETYPE_BOUNCE;
	
	dropped->mins[0] = -4;
	dropped->mins[1] = -4;
	dropped->mins[2] = -2;
	
	dropped->maxs[0] =  4;
	dropped->maxs[1] =  4;
	dropped->maxs[2] =  4;

	AngleVectors (ent->client->v_angle, forward, right, up);

	dropped->s.angles[YAW] = ent->client->v_angle[YAW];

	VectorSet(offset, 0, 16, ent->viewheight -2 );
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, dropped->s.origin);

	VectorScale (forward, 5 + ((rand()&15)-8), dropped->velocity);
    VectorMA (dropped->velocity, 50 + (rand()&31), right, dropped->velocity);
	dropped->velocity[2] = 150 + (rand()&31);

	// Ridah, changed this so they don't stick in the ground so much
	VectorSet(dropped->avelocity, 0/*((rand()&7)-4)*/, ((rand()&15)-8), ((rand()&7)-4));

	dropped->think = Think_Eject_Shell;
	dropped->misstime = 40;
	dropped->nextthink = level.time + 0.1;

	dropped->s.renderfx2 |= RF2_NOSHADOW;

	gi.linkentity (dropped);

	return;
}


void weapon_grenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 180;
	float	radius = 360.00;

	if(CustGrenadeDmg) {
		damage = CustGrenadeDmg;
		radius = (1.5 * CustGrenadeDmg);
	}

	if (ent->waterlevel >= 2)
	{
		// gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);	
		if (ent->client->weaponstate == WEAPON_FIRING)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 33;
		}
		return;
	}

	VectorSet(offset, 16, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_grenade (ent, start, forward, damage, 750, 2.0, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
	{
		ent->client->pers.weapon_clip[ent->client->clip_index]--;
	
		if (!(ent->client->pers.inventory[ent->client->ammo_index]) && !ent->client->pers.weapon_clip [ent->client->clip_index])
		{
			NoAmmoWeaponChange (ent);
			ent->client->ps.gunframe = 33;		
		}
	}
		
	ent->client->gun_noise = true;

}

void Weapon_GrenadeLauncher_Fire (edict_t *ent)
{
	if (ent->client->ps.gunframe == 5)
			weapon_grenadelauncher_fire (ent);
	else if (ent->client->ps.gunframe == 16)
	{
		if ((ent->client->pers.inventory[ent->client->ammo_index]) && !ent->client->pers.weapon_clip [ent->client->clip_index])
		{
			ent->client->reload_weapon = true;
		}
		
		ent->client->ps.gunframe = 33;

	}


}

#define	MAX_GRENADE_ROUNDS	3

void Weapon_GrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {0, 0};
	static int	fire_frames[]	= {5, 16, 0};
	int			rounds;

	if (ent->client->pers.inventory[ent->client->ammo_index] == 0 && ent->client->pers.weapon_clip[ent->client->clip_index] == 0
		&& ent->client->weaponstate != WEAPON_DROPPING) 
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
	else if (ent->client->reload_weapon)
	{
		if (ent->client->ps.gunframe >= 33 && ent->client->ps.gunframe <= 40)
		{
			if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_GRENADE_ROUNDS)
			{
				ent->client->ps.gunframe = 17;
				ent->client->weaponstate = WEAPON_RELOADING;
				
				ent->client->pers.inventory[ent->client->ammo_index] += ent->client->pers.weapon_clip[ent->client->clip_index];	

				if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_GRENADE_ROUNDS)
				{
					rounds = ent->client->pers.inventory[ent->client->ammo_index];
				}
				else
					rounds = MAX_GRENADE_ROUNDS;

				ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
				ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/grenade_launcher/reload.wav"), 1, ATTN_NORM, 0);
			}

			ent->client->reload_weapon = false;
		}
	}
	else if (!ent->client->pers.weapon_clip[ent->client->clip_index] && ent->client->pers.inventory[ent->client->ammo_index])
	{
		if (ent->client->ps.gunframe >= 33 && ent->client->ps.gunframe <= 40)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 17;
			ent->client->weaponstate = WEAPON_RELOADING;

			if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_GRENADE_ROUNDS)
			{
				rounds = ent->client->pers.inventory[ent->client->ammo_index];
			}
			else 
				rounds = MAX_GRENADE_ROUNDS;

			ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
			ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/grenade_launcher/reload.wav"), 1, ATTN_NORM, 0);
		}
	}

	Weapon_Generic (ent, 4, 32, 40, 45, pause_frames, fire_frames, Weapon_GrenadeLauncher_Fire);

	if (ent->client->ps.gunframe >= 17 && ent->client->ps.gunframe <= 32)
	{
		ent->client->ps.model_parts[PART_GUN].invisible_objects = 0;
	}
	else
		ent->client->ps.model_parts[PART_GUN].invisible_objects = (1<<0 | 1<<1);

	
	if (ent->client->ps.gunframe >= 9 && ent->client->ps.gunframe <= 16)
		ent->client->ps.rdflags |= RDF_NOLERP;
	else
		ent->client->ps.rdflags &= ~RDF_NOLERP;


}

// rocket launcher

void weapon_rocketlauncher_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius=200;
	int		radius_damage=125;
	
	if (ent->waterlevel >= 2)
	{
		// gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);	
		if (ent->client->weaponstate == WEAPON_FIRING)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 30;
		}
		return;
	}

	damage = 100 + (int)(random() * 25.0);

	if(CustRocketDmg) damage = CustRocketDmg;

	damage_radius = (float)(damage*1.6);
	radius_damage = damage;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rocket (ent, start, forward, damage, 900, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
	{
		ent->client->pers.weapon_clip[ent->client->clip_index]--;
		
		if (!(ent->client->pers.inventory[ent->client->ammo_index]) && !ent->client->pers.weapon_clip [ent->client->clip_index])
		{
			NoAmmoWeaponChange (ent);
			ent->client->ps.gunframe = 30;		
		}
	}

	ent->client->gun_noise = true;

}

void Weapon_RocketLauncher_Fire (edict_t *ent)
{
	if (ent->client->ps.gunframe == 6)
		weapon_rocketlauncher_fire (ent);
	else if (ent->client->ps.gunframe == 13)
	{
		if ((ent->client->pers.inventory[ent->client->ammo_index]) && !ent->client->pers.weapon_clip [ent->client->clip_index])
		{
			ent->client->reload_weapon = true;
		}
	
		ent->client->ps.gunframe = 30;
	}

}

#define	MAX_ROCKET_ROUNDS	5

void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {0, 0};
	static int	fire_frames[]	= {6, 13, 0};
	int			rounds;

	if (ent->client->pers.inventory[ent->client->ammo_index] == 0 && ent->client->pers.weapon_clip[ent->client->clip_index] == 0
		&& ent->client->weaponstate != WEAPON_DROPPING) 
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
	}
	else if (ent->client->reload_weapon)
	{
		if (ent->client->ps.gunframe >= 30 && ent->client->ps.gunframe <= 41)
		{
			if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_ROCKET_ROUNDS)
			{
				ent->client->ps.gunframe = 14;
				ent->client->weaponstate = WEAPON_RELOADING;
				
				ent->client->pers.inventory[ent->client->ammo_index] += ent->client->pers.weapon_clip[ent->client->clip_index];	

				if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_ROCKET_ROUNDS)
				{
					rounds = ent->client->pers.inventory[ent->client->ammo_index];
				}
				else 
					rounds = MAX_ROCKET_ROUNDS;

				ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
				ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/rocket_launcher/reload.wav"), 1, ATTN_NORM, 0);
			}

			ent->client->reload_weapon = false;
		}
	}
	else if (!ent->client->pers.weapon_clip[ent->client->clip_index] && ent->client->pers.inventory[ent->client->ammo_index])
	{
		if (ent->client->ps.gunframe >= 30 && ent->client->ps.gunframe <= 41)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 14;
			ent->client->weaponstate = WEAPON_RELOADING;

			if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_ROCKET_ROUNDS)
			{
				rounds = ent->client->pers.inventory[ent->client->ammo_index];
			}
			else 
				rounds = MAX_ROCKET_ROUNDS;

			ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
			ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/rocket_launcher/reload.wav"), 1, ATTN_NORM, 0);
		}
	}
	
	Weapon_Generic (ent, 5, 29, 41, 47, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}


/* // Begin Assault
================
Weapon Assault

A generic function to handle the basics of weapon thinking
================
*/

void Weapon_Assault (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_FRST, int FRAME_FIRE_LAST, int FRAME_IDLE_FRST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_FRST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;
	
	if (ent->client->weaponstate ==	WEAPON_RELOADING)
	{
		if (ent->client->ps.gunframe == FRAME_FIRE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FRST;
			return;
		}
		if(ent->client->ps.gunframe > 59)
			{
				gi.dprintf ("WEAPON_RELOADING Frame: %d\n", ent->client->ps.gunframe); // test code
				ent->client->ps.gunframe = FRAME_IDLE_FRST;
				//return;
			}
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_RELOAD_CYCLE)
	{
		if(ent->client->ps.gunframe > 59)
			{
				gi.dprintf ("WEAPON_RELOAD_CYCLE Frame: %d\n", ent->client->ps.gunframe); // test code
				ent->client->ps.gunframe = FRAME_IDLE_FRST;
				//return;
			}
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{

		if (ent->client->pers.weapon == FindItem("SPistol"))
		{

			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
			{
				ChangeWeapon (ent);
				return;
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
			{
				ChangeWeapon (ent);
				return;
			}
			else if ((ent->client->weaponstate != WEAPON_FIRING) && (ent->client->ps.gunframe < FRAME_DEACTIVATE_FIRST))
			{
				ent->client->ps.gunframe = FRAME_DEACTIVATE_FRST;
				return;
			}
		}

		if(ent->client->ps.gunframe > 59)
			{
				gi.dprintf ("WEAPON_DROPPING Frame: %d\n", ent->client->ps.gunframe); // test code
				ent->client->ps.gunframe = FRAME_IDLE_FRST;
				//return;
			}
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FRST;
			return;
		}

		ent->client->ps.gunframe++;

		// Ridah, faster weapon changing in deathmatch
		if (deathmatch->value && ent->client->ps.gunframe < FRAME_ACTIVATE_LAST)
			ent->client->ps.gunframe++;

		if(ent->client->ps.gunframe > 59)
			{
				gi.dprintf ("WEAPON_ACTIVATING Frame: %d\n", ent->client->ps.gunframe); // test code
				ent->client->ps.gunframe = FRAME_IDLE_FRST;
				//return;
			}
		return;
	}
	
	if ((ent->client->newweapon) /*&& (ent->client->weaponstate != WEAPON_FIRING)*/)
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FRST;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			
			if (ent->client->clip_index == CLIP_SLUGS && ent->client->pers.weapon_clip[ent->client->clip_index] >=1)
			{
				ent->client->ps.gunframe = FRAME_FIRE_FRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}

			else if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRE_FRST;
				ent->client->weaponstate = WEAPON_FIRING;
			}
			else 
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
	//			gi.dprintf ("Frame: %d\n", ent->client->ps.gunframe); // test code
				ent->client->ps.gunframe = FRAME_IDLE_FRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand()&15)
							return;
					}
				}
			}
			
			if(ent->client->ps.gunframe > 59)
			{
				gi.dprintf ("WEAPON_READY Frame: %d\n", ent->client->ps.gunframe); // test code
				ent->client->ps.gunframe = FRAME_IDLE_FRST;
				return;
			}

			ent->client->ps.gunframe++;

			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;
/*
        if ((ent->client->ps.gunframe == 9) &&
			(!((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)) &&
			(!strcmp(ent->client->pers.weapon->pickup_name, "Crowbar")))
		{
			ent->client->ps.gunframe = FRAME_IDLE_FRST+1;
			ent->client->weaponstate = WEAPON_READY;
		}
*/
		if (ent->client->ps.gunframe == FRAME_FIRE_LAST) //ready to fire another shot
			ent->client->weaponstate = WEAPON_READY;
	}


} 
void weapon_msg90_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage =  50;
	int			kick = 150; // Frank from = 50;

	if (ent->waterlevel >= 2)
	{
		if (ent->client->weaponstate == WEAPON_FIRING)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 43;//assault2 43
		}
		return;
	}


	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;
	VectorSet(offset, 0, 0,  ent->viewheight-1); //assault2 -1
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_bullet (ent, start, forward, damage, kick, 0, 0, MOD_BARMACHINEGUN);
//	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hmg/single.wav"), 1, ATTN_NORM, 0); // HOGIE for his single shot
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/msg90/msg90_fire.wav"), 1, ATTN_NORM, 0);//assault2
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
	{
		ent->client->pers.weapon_clip [ent->client->clip_index]--;

		if (!(ent->client->pers.inventory[ent->client->ammo_index]) && !ent->client->pers.weapon_clip [ent->client->clip_index])
		{
			NoAmmoWeaponChange (ent);
			ent->client->ps.gunframe = 43;		
		}
		else if (ent->client->pers.inventory[ent->client->ammo_index] && !(ent->client->pers.weapon_clip [ent->client->clip_index]))
		{
			ent->client->reload_weapon = true;
			ent->client->ps.gunframe = 43;		
		}
		
	}
  
	ent->client->gun_noise = true;

}

void msg90_fire (edict_t *ent)
{
	if (ent->client->ps.gunframe == 21 ) // HOGIE assault2
	{ 
		weapon_msg90_fire (ent);
	
		if (ent->client->pers.pistol_mods & WEAPON_MOD_COOLING_JACKET)
		{
			ent->client->pers.hmg_shots --;

			if (!ent->client->pers.hmg_shots)
			{
				ent->client->pers.pistol_mods &= ~WEAPON_MOD_COOLING_JACKET;
				
				ent->client->newweapon = ent->client->pers.weapon;
				
				if (ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.weapon_clip[ent->client->clip_index])
					ChangeWeapon (ent);
				else
					NoAmmoWeaponChange (ent);
			}
		}
	}
	
	else if ( ((ent->client->ps.gunframe == 22)
		     ||(ent->client->ps.gunframe == 23)) 
			 && (ent->client->pers.pistol_mods & WEAPON_MOD_COOLING_JACKET) )
	{
		weapon_msg90_fire (ent);
		ent->client->pers.hmg_shots --;
		if (!ent->client->pers.hmg_shots)
		{
			ent->client->pers.pistol_mods &= ~WEAPON_MOD_COOLING_JACKET;
			
			ent->client->newweapon = ent->client->pers.weapon;
			
			if (ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.weapon_clip[ent->client->clip_index])
				ChangeWeapon (ent);
			else
				NoAmmoWeaponChange (ent);
		}
	}
ent->client->ps.gunframe++;
	
}

#define MAX_MSG90_ROUNDS 30

void Weapon_Msg90 (edict_t *ent)
{
	static int	pause_frames[]	= {43, 44, 45};
	static int	fire_frames[]	= {21, 22, 23};//assault2
	int			rounds;
	
	if (ent->client->pers.inventory[ent->client->ammo_index] == 0 && ent->client->pers.weapon_clip[ent->client->clip_index] == 0
		&& ent->client->weaponstate != WEAPON_DROPPING) 
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
	}
	else if (ent->client->reload_weapon)
	{
		if (ent->client->ps.gunframe >= 43 && ent->client->ps.gunframe <= 53)
		{
			if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_MSG90_ROUNDS)
			{
				ent->client->ps.gunframe = 20;
				ent->client->weaponstate = WEAPON_RELOADING;
				
				ent->client->pers.inventory[ent->client->ammo_index] += ent->client->pers.weapon_clip[ent->client->clip_index];	

				if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_MSG90_ROUNDS)
				{
					rounds = ent->client->pers.inventory[ent->client->ammo_index];
				}
				else 
					rounds = MAX_MSG90_ROUNDS;

				ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
				ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
	//			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hmg/hmgcock.wav"), 1, ATTN_NORM, 0);
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/msg90/msg90_reload4.wav"), 1, ATTN_NORM, 0);
			}

			ent->client->reload_weapon = false;
		}
	}
	else if (!ent->client->pers.weapon_clip[ent->client->clip_index] && ent->client->pers.inventory[ent->client->ammo_index])
	{
		if (ent->client->ps.gunframe >= 43 && ent->client->ps.gunframe <= 53)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 20;
			ent->client->weaponstate = WEAPON_RELOADING;

			if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_MSG90_ROUNDS)
			{
				rounds = ent->client->pers.inventory[ent->client->ammo_index];
			}
			else 
				rounds = MAX_MSG90_ROUNDS;

			ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
			ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
		//	gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hmg/hmgcock.wav"), 1, ATTN_NORM, 0);
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/msg90/msg90_reload4.wav"), 1, ATTN_NORM, 0);
		}
	}

	Weapon_Assault (ent, 20, 21, 35, 40, 45, 51, 56, pause_frames, fire_frames, msg90_fire);//assault2
 //	gi.dprintf ("Frame: %d\n", ent->client->ps.gunframe); // test code
}
//void Weapon_Assault (ent,FRAME_ACTIVATE_LAST, FRAME_FIRE_FRST, FRAME_FIRE_LAST,
//                                              FRAME_IDLE_FRST, FRAME_IDLE_LAST,
//                                              FRAME_DEACTIVATE_FRST, FRAME_DEACTIVATE_LAST,
//                                              *pause_frames, *fire_frames , *fire)

/*QUAKED weapon_mortar (.3 .3 1) (-32 -32 -48) (32 32 32)
model="models/weapons/g_mortar/mortar.mdx"
Mortar to frag your punk ass. Assault 2
"style" [ 1 or 2 ] Disables the opposing team's usage.
*/
void weapon_mortar_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage =  50;
	int			kick = 150; // Frank from = 50;

	if (ent->waterlevel >= 2)
	{
		if (ent->client->weaponstate == WEAPON_FIRING)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 43;//assault2 43
		}
		return;
	}


	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;
	VectorSet(offset, 0, 0,  ent->viewheight-1); //assault2 -1
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_bullet (ent, start, forward, damage, kick, 0, 0, MOD_BARMACHINEGUN);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hmg/single.wav"), 1, ATTN_NORM, 0); // HOGIE for his single shot

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
	{
		ent->client->pers.weapon_clip [ent->client->clip_index]--;

		if (!(ent->client->pers.inventory[ent->client->ammo_index]) && !ent->client->pers.weapon_clip [ent->client->clip_index])
		{
			NoAmmoWeaponChange (ent);
			ent->client->ps.gunframe = 43;		
		}
		else if (ent->client->pers.inventory[ent->client->ammo_index] && !(ent->client->pers.weapon_clip [ent->client->clip_index]))
		{
			ent->client->reload_weapon = true;
			ent->client->ps.gunframe = 43;		
		}
		
	}
  
	ent->client->gun_noise = true;

}

void mortar_fire (edict_t *ent)
{
	if (ent->client->ps.gunframe == 13 ) // HOGIE assault2
	{ 
		weapon_mortar_fire (ent);
	
	}
	
ent->client->ps.gunframe++;
	
}

#define MAX_MORTAR_ROUNDS 30/////////////////

void Weapon_Mortar (edict_t *ent)
{
	static int	pause_frames[]	= {0, 0};
	static int	fire_frames[]	= {11, 12, 13};//assault2
	int			rounds;
	
	if (ent->client->pers.inventory[ent->client->ammo_index] == 0 && ent->client->pers.weapon_clip[ent->client->clip_index] == 0
		&& ent->client->weaponstate != WEAPON_DROPPING) 
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
	}
	else if (ent->client->reload_weapon)
	{
		if (ent->client->ps.gunframe >= 43 && ent->client->ps.gunframe <= 53)
		{
			if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_MORTAR_ROUNDS)
			{
				ent->client->ps.gunframe = 20;
				ent->client->weaponstate = WEAPON_RELOADING;
				
				ent->client->pers.inventory[ent->client->ammo_index] += ent->client->pers.weapon_clip[ent->client->clip_index];	

				if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_MORTAR_ROUNDS)
				{
					rounds = ent->client->pers.inventory[ent->client->ammo_index];
				}
				else 
					rounds = MAX_MORTAR_ROUNDS;

				ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
				ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hmg/hmgcock.wav"), 1, ATTN_NORM, 0);
			}

			ent->client->reload_weapon = false;
		}
	}
	else if (!ent->client->pers.weapon_clip[ent->client->clip_index] && ent->client->pers.inventory[ent->client->ammo_index])
	{
		if (ent->client->ps.gunframe >= 43 && ent->client->ps.gunframe <= 53)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 20;
			ent->client->weaponstate = WEAPON_RELOADING;

			if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_MORTAR_ROUNDS)
			{
				rounds = ent->client->pers.inventory[ent->client->ammo_index];
			}
			else 
				rounds = MAX_MORTAR_ROUNDS;

			ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
			ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hmg/hmgcock.wav"), 1, ATTN_NORM, 0);
		}
	}

	Weapon_Assault (ent, 9, 10, 18, 19, 43, 44, 46, pause_frames, fire_frames, mortar_fire);//assault2
 //	gi.dprintf ("Frame: %d\n", ent->client->ps.gunframe); // test code
}

/*
======================================================================

AK47 virgin23

======================================================================
*/

void Ak47_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	//vec3_t	fireorigin;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 25; // Frank changed this from 8 * 1.75 = 14;
	int			kick = 50; // Frank changed this from 2;
	vec3_t		offset;


	if(CustTommyDmg) damage = CustTommyDmg+3;

	if (ent->waterlevel >= 2)
	{
		// gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);	
		if (ent->client->weaponstate == WEAPON_FIRING)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 30;
		}
		return;
	}

	// First shot
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		// ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe = 30;
		return;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] == 0 && ent->client->pers.weapon_clip[ent->client->clip_index] == 0) 
		return;

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

	// Tommy is too powerful in real-mode
	if (dm_realmode->value)
		damage /= 2;

	// Bounce
	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.22;
		ent->client->kick_angles[i] = crandom() * 0.6;
	}
	ent->client->kick_origin[0] = crandom() * 0.22;
	// ent->client->kick_angles[0] = ent->client->machinegun_shots * -.5;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	
	VectorSet(offset, 0, 0, ent->viewheight-1);
	
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD-50, DEFAULT_BULLET_VSPREAD-50, MOD_AK47);

//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);
//	PlayerNoise(ent, start, PNOISE_WEAPON);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/ak47/machgf1b.wav"), 1, ATTN_NORM, 0);

	// Disable for unlimited ammo
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO && deathmatch->value ) )
	{
		ent->client->pers.weapon_clip [ent->client->clip_index]--;

		if (!(ent->client->pers.inventory[ent->client->ammo_index]) && !ent->client->pers.weapon_clip [ent->client->clip_index])
		{
			NoAmmoWeaponChange (ent);
			ent->client->ps.gunframe = 30;		
		}
		else if (!ent->client->pers.weapon_clip [ent->client->clip_index])
		{
			ent->client->reload_weapon = true;
			ent->client->ps.gunframe = 30;		
			// gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/machinegun/machgcock.wav"), 1, ATTN_NORM, 0);
		}
	}

	// if (!deathmatch->value)	// uses up bandwidth //Frank wacked it out, save the if statement.
    // Eject_Tommy_Shell(ent);

	ent->client->gun_noise = true;
}

void Weapon_Ak47 (edict_t *ent)
{
	int		rounds;
	static int	pause_frames[]	= {44, 0, 0};
	static int	fire_frames[]	= {4, 5, 0};
	
	if (ent->client->pers.inventory[ent->client->ammo_index] == 0 && ent->client->pers.weapon_clip[ent->client->clip_index] == 0
		&& ent->client->weaponstate != WEAPON_DROPPING) 
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
	}
	else if (ent->client->reload_weapon)
	{
		if (ent->client->ps.gunframe >= 30 && ent->client->ps.gunframe <= 40)
		{
			if (ent->client->pers.weapon_clip[ent->client->clip_index] < MAX_TOMMYGUN_ROUNDS)
			{
				ent->client->ps.gunframe = 6;
				ent->client->weaponstate = WEAPON_RELOADING;
				
				ent->client->pers.inventory[ent->client->ammo_index] += ent->client->pers.weapon_clip[ent->client->clip_index];	

				if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_TOMMYGUN_ROUNDS)
				{
					rounds = ent->client->pers.inventory[ent->client->ammo_index];
				}
				else 
					rounds = MAX_TOMMYGUN_ROUNDS;

				ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
				ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/machinegun/machgcock.wav"), 1, ATTN_NORM, 0);
			}

			ent->client->reload_weapon = false;
		}
	}
	else if (!ent->client->pers.weapon_clip[ent->client->clip_index] && ent->client->pers.inventory[ent->client->ammo_index])
	{
		if (ent->client->ps.gunframe >= 30 && ent->client->ps.gunframe <= 40)
		{
			ent->client->reload_weapon = false;
			ent->client->ps.gunframe = 6;
			ent->client->weaponstate = WEAPON_RELOADING;

			if (ent->client->pers.inventory[ent->client->ammo_index] < MAX_TOMMYGUN_ROUNDS)
			{
				rounds = ent->client->pers.inventory[ent->client->ammo_index];
			}
			else 
				rounds = MAX_TOMMYGUN_ROUNDS;

			ent->client->pers.weapon_clip[ent->client->clip_index] = rounds;
			ent->client->pers.inventory[ent->client->ammo_index] -= rounds;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/machinegun/machgcock.wav"), 1, ATTN_NORM, 0);
		}
	}

	Weapon_Generic (ent, 3, 29, 40, 46, pause_frames, fire_frames, Ak47_Fire);


}

// End Assault2