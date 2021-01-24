
#include "g_local.h"
#include "m_player.h"
//#include "stdlog.h"	//	Standard Logging
//#include "gslog.h"	//	Standard Logging

#include "voice_bitch.h"
#include "voice_punk.h"

#define NAME_CLASH_STR    "<Name Clash>"
#define NAME_BLANK_STR    "<No Name>"

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

void SP_misc_teleporter_dest (edict_t *ent);

//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

static void SP_FixCoopSpots (edict_t *self)
{
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384)
		{
			if ((!self->targetname) || Q_stricmp(self->targetname, spot->targetname) != 0)
			{
//				gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
				self->targetname = spot->targetname;
			}
			return;
		}
	}
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
	edict_t	*spot;

	if(Q_stricmp(level.mapname, "security") == 0)
	{
		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 128;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		return;
	}
}


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 48)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{

	extern void Show_Help (void);
	
	if (!(deathmatch->value))
	{
		if (	!strcmp(level.mapname, "sr1") 
			||	!strcmp(level.mapname, "pv_h")
			||	!strcmp(level.mapname, "sy_h")
			||	!strcmp(level.mapname, "steel1")
			||	!strcmp(level.mapname, "ty1")
			||	!strcmp(level.mapname, "rc1") )
		Show_Help ();
	}
	
	if (!coop->value)
		return;
	if(Q_stricmp(level.mapname, "security") == 0)
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_CreateCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 48)
potential spawning position for deathmatch games

  style - team # for Teamplay (1 or 2)
*/
void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
//	SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 48)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
/*
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}

  if((Q_stricmp(level.mapname, "jail2") == 0)   ||
	   (Q_stricmp(level.mapname, "jail4") == 0)   ||
	   (Q_stricmp(level.mapname, "mine1") == 0)   ||
	   (Q_stricmp(level.mapname, "mine2") == 0)   ||
	   (Q_stricmp(level.mapname, "mine3") == 0)   ||
	   (Q_stricmp(level.mapname, "mine4") == 0)   ||
	   (Q_stricmp(level.mapname, "lab") == 0)     ||
	   (Q_stricmp(level.mapname, "boss1") == 0)   ||
	   (Q_stricmp(level.mapname, "fact3") == 0)   ||
	   (Q_stricmp(level.mapname, "biggun") == 0)  ||
	   (Q_stricmp(level.mapname, "space") == 0)   ||
	   (Q_stricmp(level.mapname, "command") == 0) ||
	   (Q_stricmp(level.mapname, "power2") == 0) ||
	   (Q_stricmp(level.mapname, "strike") == 0))
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_FixCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
*/
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage, int mdx_part, int mdx_subobject)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
//	char		*info;

	if (!ent->client)
		return false;

	if (ent->gender == GENDER_FEMALE)
		return true;
/*
	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
*/
	return false;
}

qboolean IsNeutral (edict_t *ent)
{
//	char		*info;

	if (!ent->client)
		return false;

	if (ent->gender == GENDER_NONE)
		return true;
/*
	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
		return true;
*/
	return false;
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		// in realmode, track deaths
/*		if (dm_realmode->value && !teamplay->value)
			self->client->resp.deposited++;
		if (teamplay->value==4)
			self->client->resp.deposited++;*/
		if (mod!=MOD_RESTART && mod!=MOD_TELEFRAG && (teamplay->value==4
			|| (dm_realmode->value && !teamplay->value)))
			self->client->resp.deposited++;

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			message = "cratered";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			{
				static int q; // assault2
				q = (q+1) % 3;
				switch(q)
				{
				case 0:
					message = "sank like a rock"; break;
				case 1:
					message = "sleeps with the fishes"; break;
				case 2:
				default:
					message = "inhaled a lungfull of water"; 
				}
			}
			break;
		case MOD_EXPLODINGKEY:
			{
				static int w; // assault2
				w = (w+1) % 3;
				switch(w)
				{
				case 0:
					message = "was turned to sludge by that exploding key"; break;
				case 1:
					message = "isnt so close to that key anymore"; break;
				case 2:
				default:
					message = "became a nasty stain on the wall"; 
				}
			}
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
    // RAFAEL
		case MOD_GEKK:
		case MOD_BRAINTENTACLE:
			message = "that's gotta hurt";
			break;
    }
		if (attacker == self)
		{
      switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsNeutral(self))
					message = "tripped on its own grenade";
				else if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsNeutral(self))
					message = "blew itself up";
				else if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			// RAFAEL 03-MAY-98
			case MOD_TRAP:
			 	message = "sucked into his own trap";
				break;
			case MOD_SAFECAMPER: // assault2
			 	message = "shouldn't loiter near the radio.";
				break;
			case MOD_CANNON: // assault 2
				if (IsNeutral(self))
					message = "swallowed it's own cannon";
				else if (IsFemale(self))
					message = "did herself with a cannon!  Who needs a man";
				else
					message = "swallowed his own cannon";
			 	break;
			case MOD_NUKE: // assault2
				message = "went out in a nuclear cloud";
				break;
			case MOD_FLAMETHROWER:
				if (IsNeutral(self))
					message = "roasted itself";
				else if (IsFemale(self))
					message = "roasted herself";
				else
					message = "roasted himself";
				break;
			}
		}
		if (message && (mod != MOD_RESTART)) //assault2
		{
			gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
			{
				self->client->resp.score--;

				if ((int)teamplay->value == TM_GANGBANG) {
					team_cash[self->client->pers.team]--;
					UPDATESCORE
				}

			}
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_BLACKJACK:
				message = "was mashed by";
				break;
			case MOD_CROWBAR:
				message = "was severely dented by";
				break;
			case MOD_PISTOL:
				message = "was busted by";
				message2 = "'s cap";
				break;
			case MOD_SILENCER: //begin assault2
				{ // Frank
					static int d;
					d = (d+1)%3;
					switch(d)
					{
					case 2:
							message = "was capped by";
							message2 = "'s silencer like a sheep to the slaughter";
							break;
					case 0:
							message = "silently took";
							message2 = "'s hot lead to heart";
							break;
					default:
					case 1:
							message = "was shot up by";
							message2 = "'s silencer like a two bit hoe";
					}
				} // End Frank
				break;

			case MOD_SPAZ:
				{ // virgin16
					static int p;
					p = (p+1)%4;
					switch(p)
					{
					case 1:
							message = "was shut the hell up by";
							message2 = "'s Spaz gun";
							break;
					case 2:
							message = "'s head was blown off by";
							message2 = "'s Spaz gun";
							break;
					case 3:
							message = "'s face was rearranged by";
							message2 = "'s Spaz gun";
							break;
					default:
					case 0:
							message = "was ripped a new one by";
							message2 = "'s Spaz gun";
					}
				} // End Frank
				break;

			case MOD_PUMP:
				{ // virgin17
					static int q;
					q = (q+1)%4;
					switch(q)
					{
					case 1:
							message = "was pimp slapped by";
							message2 = "'s Pump gun";
							break;
					case 2:
							message = "face was turned to buckshot by";
							message2 = "'s Pump gun";
							break;
					case 3:
							message = "put on a few extra pounds of buckshot from";
							message2 = "'s Pump gun";
							break;
					default:
					case 0:
							message = "is making sucking noises on";
							message2 = "'s Pump gun";
					}
				} // End Frank
				break;

			case MOD_AK47:
				{ // virgin23
					static int r;
					r = (r+1)%4;
					switch(r)
					{
					case 1:
							message = "'s ass was shot full of lead by";
							message2 = "'s Ak-47";
							break;
					case 2:
							message = "got a little to personal with";
							message2 = "'s AK-47";
							break;
					case 3:
							message = "accepted no substitute for";
							message2 = "'s AK-47";
							break;
					default:
					case 0:
							message = "was shot up like a one night hoe by";
							message2 = "'s AK-47";
					}
				} // End Frank
				break;

			case MOD_SHOTGUN:
				{ // Frank
					static int e;
					e = (e+1)%3;
					switch(e)
					{
					case 1:
							message = "accepted";
							message2 = "'s hot shotty load";
							break;
					case 2:
							message = "went down on";
							message2 = "'s shotty";
							break;
					default:
					case 0:
							message = "was bitch slapped by";
							message2 = "'s shotgun";
					}
/*
					attacker->client->pers.shotgunfrags++; // assault2 virgin18
					if(attacker->client->pers.shotgunfrags>=assault_customweaponfrags)
					{
						gi.bprintf (PRINT_HIGH,"%s's shotgun upgrades to level %i.\n", attacker->client->pers.netname, attacker->client->pers.shotgunfrags);
					}
*/
				} // End Frank
				break;

			case MOD_MACHINEGUN:
				{ // Frank
					static int f;
					f = (f+1)%3;
					switch(f)
					{
					case 0:
							message = "was cut to ribbons by";
							message2 = "'s well hung Tommygun";
							break;
					case 1:
							message = "fell all to pieces over";
							message2 = "'s Tommygun";
							break;
					default:
					case 2:
							message = "was riddled with";
							message2 = "'s streaming Tommy slugs";
					}
				} // End Frank
				break;

			case MOD_FLAMETHROWER:
				{ // Frank
					static int g;
					g = (g+1)%3;
					switch(g)
					{
					case 0:
							message = "was flame broiled by";
							message2 = "'s hot rod";
							break;
					case 1:
							message = "was lit up like the 4th of July by";
							message2 = "'s pyrotechnics";
							break;
					default:
					case 2:
							message = "sucked on";
							message2 = "'s white hot gas bong";
					}
				} // End Frank
				break;

			case MOD_GRENADE:
				{ // Frank
					static int m;
					m = (m+1)%3;
					switch(m)
					{
					case 0:
							message = "fumbled one of";
							message2 = "'s grenades";
							break;
					case 1:
							message = "was choked by";
							message2 = "'s grenade";
							break;
					default:
					case 2:
							message = "was blown to bits by";
							message2 = "'s grenade";
					}
				} // End Frank
				break;

			case MOD_G_SPLASH:
				{ // Frank
					static int n;
					n = (n+1)%3;
					switch(n)
					{
					case 0:
							message = "was blown to Fuckin Hell by";
							message2 = "'s shrapnel";
							break;
					case 1:
							message = "made a grease spot with";
							message2 = "'s exploding balls";
							break;
					default:
					case 2:
							message = "fumbled";
							message2 = "'s grenade nut";
					}
				} // End Frank
				break;

			case MOD_ROCKET:
				{ // Frank
					static int k;
					k = (k+1)%3;
					switch(k)
					{
					case 0:
							message = "sucked down";
							message2 = "'s volcanic rocket load";
							break;
					case 1:
							message = "blocked";
							message2 = "'s rocket with his ugly mug";
							break;
					default:
					case 2:
							message = "dropped the soap and took";
							message2 = "'s rocket from behind";
					}
				} // End Frank
				break;

			case MOD_R_SPLASH:
				{ // Frank
					static int l;
					l = (l+1)%3;
					switch(l)
					{
					case 0:
							message = "couldn't escape";
							message2 = "'s rocket blast";
							break;
					case 1:
							message = "caught too much of";
							message2 = "'s rocket splash";
							break;
					default:
					case 2:
							message = "almost evaded";
							message2 = "'s rocket blast";
					}
				} // End Frank
				break;

			case MOD_TELEFRAG:
				message = "couldn't co-exist with";
				message2 = "";
				break;

			case MOD_BARMACHINEGUN:
				{ // Frank
					static int h;
					h = (h+1)%4;
					switch(h)
					{
					case 0:
							message = "'s clock was punched by";
							message2 = "'s H.M.G";
							break;
					case 1:
							message = "'s chain was pulled by";
							message2 = "'s H.M.G";
							break;
					case 2:
							message = "got a mouthful of Lead Chicklets from";
							message2 = "'s H.M.G";
							break;
					default:
					case 3:
							message = "was drilled by";
							message2 = "'s H.M.G";
					}
				} // End Frank
				break;

		    case MOD_CANNON: // FRANK
				{
					static int i;
					i = (i+1)%3;
					switch(i)
					{
					case 1:
							message = "took";
							message2 = "'s wet cannon wad right in the face like a cheap porn star";
							break;
					case 0:
							message = "was ripped a new asshole by";
							message2 = "'s fat cannon balls";
							break;
					default:
					case 2:
							message = "choked on";
							message2 = "'s thick cannon wad";
					}
				}
     		   break;

		    case MOD_NUKE: // FRANK
				{
					static int j;
					j = (j+1)%3;
					switch(j)
					{
					case 0:
							message = "swallowed";
							message2 = "'s nuclear blast";
							break;
					case 1:
							message = "inhaled";
							message2 = "'s hot nuclear anal gasses";
							break;
					default:
					case 2:
							message = "was atomized by";
							message2 = "'s nuclear explosion";
					}
				}
    		   break;
			// END assault2
			}
			if (message)
			{
				gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if ((deathmatch->value) && (mod != MOD_RESTART) && (mod != MOD_TELEFRAG))
				{
					if (ff)
					{
						attacker->client->resp.score--;

						if ((int)teamplay->value == TM_GANGBANG) {
							team_cash[attacker->client->pers.team]--;
							UPDATESCORE
						}
					}
					else
					{
						attacker->client->resp.score++;

						if ((int)teamplay->value == TM_GANGBANG) {
							team_cash[attacker->client->pers.team]++;
							UPDATESCORE
						}
					}
				}
				return;
			}
		}
	}

	// Frank if it got this far, we dont know what hit us which is where hogie's airstrikes leave us.
			{
				static int p;
				p = (p+1) % 3;
				switch(p)
				{
				case 0:
					message = "laid down for the fly boys"; break;
				case 1:
					message = "went out in blaze of glory from above"; break;
				case 2:
				default:
					message = "should have seen that bird drop those eggs"; 
				}
			}
//	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if ((deathmatch->value) && (mod != MOD_RESTART))
	{
		self->client->resp.score--;

		if ((int)teamplay->value == TM_GANGBANG) {
			team_cash[self->client->pers.team]--;
			UPDATESCORE
		}
	}
}


void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	
	item = self->client->pers.weapon;

	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;

    // assault2 dont drop silencer or pistol when a player dies.
	if (item && ((strcmp (item->pickup_name, "Pistol") == 0) || (strcmp (item->pickup_name, "SPistol") == 0))) 
		item = NULL;

	if (item)
	{
		drop = Drop_Item (self, item);
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
}


/*
void DropKeys(edict_t *self) //created for hogie and ASSAULT2
 to drop keys if you quit or die
 //gi.bprintf (PRINT_HIGH, "%s %s will respawn to original position in %i seconds.\n",team_names[ent->style],ent->classname,ent->misstime);
*/


void DropKeys(edict_t *self)//cash = SpawnTheWeapon( self, "item_cashbagsmall" );
{
	edict_t		*key;
	qboolean    trigger;
	trigger = false;

	self->client->pers.bagcash = 0; // do not drop any bag of cash with 1 dollar in it.

	if ( (self->client->pers.inventory[ITEM_INDEX(FindItem("Key_One"))]) ) {
		key = SpawnTheWeapon( self, "key_key1" );
		strcpy( key->classname, "key_key1" );
		trigger = true; }

	else if ( (self->client->pers.inventory[ITEM_INDEX(FindItem("Key_Two"))]) ) {
		key = SpawnTheWeapon( self, "key_key2" );
		strcpy( key->classname, "key_key2" );		
		trigger = true; }

	else if ( (self->client->pers.inventory[ITEM_INDEX(FindItem("Key_Three"))]) ) {
		key = SpawnTheWeapon( self, "key_key3" );
		strcpy( key->classname, "key_key3" );		
		trigger = true; }


	if(trigger)
	{ 
		key->style = self->client->pers.team; // give the key its style.

		if(self->deathtarget) // dont give the key it's target back cuz it triggers premoturely.
		{
//			key->target = G_CopyString( self->deathtarget );
			self->deathtarget = NULL;
		}


		if(!skill->value) skill->value = 1;

		if(!(int)self->wait) self->wait = 15.0;
		key->misstime = (int)(self->wait*skill->value);
		self->wait = 0;

		if(self->option) { key->option = self->option;  self->option = 0; }

		if(self->count) { key->order = self->count;  self->count = 0; }

		key->nextthink = level.time + 1;
		key->think = AssaultKeyThink;

		{
			edict_t *guy; 
			int q;
			for_everyone(guy,q)
			gi.cprintf( guy, PRINT_CHAT, ">> %s %s will respawn in %i seconds.\n",team_names[key->style],key->classname,key->misstime);
			G_FreeEdict (guy);
		}
	}
G_FreeEdict (self);
} /// END ASSAULT2



/*
==================
player_die
==================
*/
extern void VelocityForDamage (int damage, vec3_t v);

void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point, int mdx_part, int mdx_subobject)
{
	int		n;

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

//	self->s.modelindex2 = 0;	// remove linked weapon model
	self->s.model_parts[PART_GUN].modelindex = 0;

	self->s.renderfx2 &= ~RF2_FLAMETHROWER;
	self->s.renderfx2 &= ~RF2_MONEYBAG;

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag && (self->health + damage > 0))
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);

		self->client->ps.pmove.pm_type = PM_DEAD;
		if(meansOfDeath!=MOD_RESTART)
		ClientObituary (self, inflictor, attacker);

//		sl_WriteStdLogDeath( &gi, level, self, inflictor, attacker);	// Standard Logging

		if (meansOfDeath!=MOD_RESTART && SixKeyMap()) // assault2	
			TossClientWeapon (self);

		if (deathmatch->value && meansOfDeath!=MOD_RESTART)	Cmd_Help_f (self, 0);		// show scores

		DropKeys(self); // ASSAULT2

		// clear inventory
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < game.num_items; n++)
		{
			if (coop->value && itemlist[n].flags & IT_KEY)
				self->client->resp.coop_respawn.inventory[n] = self->client->pers.inventory[n];
			self->client->pers.inventory[n] = 0;
		}

		// yell at us? // assault 2 // only if your above water. originally %6
		if (rand()%4 == 0 
			&& attacker->last_talk_time < (level.time - TALK_FIGHTING_DELAY) 
			&& (attacker->waterlevel <= 2) )
		{
			#define F_NUM_FIGHTING	8
			#define NUM_FIGHTING	10
			extern voice_table_t f_fightsounds[];
			extern voice_table_t fightsounds[];

			if (attacker->gender == GENDER_MALE)
				Voice_Random(attacker, self, fightsounds, NUM_FIGHTING);
			else if (attacker->gender == GENDER_FEMALE)
				Voice_Random(attacker, self, f_fightsounds, F_NUM_FIGHTING);
		}

		self->client->pers.bagcash = 0; //assault2
		self->client->pers.currentcash = 0; //assault2
// crashes server // if(self->count) self->count = 0; // assault virgin11
/* //assault2
		// drop cash if we have some
		// Papa - fixed bug where you didn't drop cash if you killed yourself
		if (deathmatch->value  && (!self->client->pers.friendly_vulnerable))
		{
			edict_t *cash;

			// always drop at least 10 bucks, to reward the killer
//			if (teamplay->value && (teamplay_mode != TM_GANGBANG) && (attacker->client) && (attacker->client->pers.team != self->client->pers.team))

			if (teamplay->value && (teamplay->value != TM_GANGBANG) && (attacker->client) && (attacker->client->pers.team != self->client->pers.team))
			{
				if (self->client->pers.currentcash < MAX_CASH_PLAYER)
				{
					self->client->pers.currentcash += 10;
				}

				if (self->client->pers.bagcash < MAX_BAGCASH_PLAYER)
				{
					// if they were killed in the enemy base, reward them with some extra cash
					cash = NULL;
					while (cash = G_Find( cash, FOFS(classname), "dm_safebag" ))
					{
						if (cash->style != self->client->pers.team)
						{
							if (	gi.inPHS( cash->s.origin, self->s.origin )
								||	(VectorDistance( cash->s.origin, self->s.origin ) < 512))
								self->client->pers.bagcash += 30;

							if (self->client->pers.bagcash > MAX_BAGCASH_PLAYER)
								self->client->pers.bagcash = MAX_BAGCASH_PLAYER;

							break;
						}
					}
				}
			}

			if (self->client->pers.currentcash)
			{
				cash = SpawnTheWeapon( self, "item_cashroll" );
				cash->currentcash = self->client->pers.currentcash;
				self->client->pers.currentcash = 0;

				cash->velocity[0] = crandom() * 100;
				cash->velocity[1] = crandom() * 100;
				cash->velocity[2] = 0;

				VectorNormalize( cash->velocity );
				VectorScale( cash->velocity, 100, cash->velocity );
				cash->velocity[2] = 300;
			}

			if (self->client->pers.bagcash)
			{
				if (self->client->pers.bagcash > 100)
					cash = SpawnTheWeapon( self, "item_cashbaglarge" );
				else
					cash = SpawnTheWeapon( self, "item_cashbagsmall" );

//				cash->nextthink = level.time + 120;

				cash->currentcash = -self->client->pers.bagcash;
				self->client->pers.bagcash = 0;

				cash->velocity[0] = crandom() * 100;
				cash->velocity[1] = crandom() * 100;
				cash->velocity[2] = 0;

				VectorNormalize( cash->velocity );
				VectorScale( cash->velocity, 100, cash->velocity );
				cash->velocity[2] = 300;
			}
		}
*/ //assault2
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;

	// RAFAEL
	self->client->quadfire_framenum = 0;

// Ridah
	self->moveout_ent = NULL;
// done.

	self->s.renderfx2 = 0;

	if (damage >= 50 && self->health < -30 && !inflictor->client)
	{	// gib
		GibEntity( self, inflictor, damage );
		self->s.renderfx2 |= RF2_ONLY_PARENTAL_LOCKED;
	}

	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i+1)%4;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crouch_death_01-1;
				self->client->anim_end = FRAME_crouch_death_12;
			}
			else switch (i)
			{
			case 0:
				self->s.frame = FRAME_death1_01-1;
				self->client->anim_end = FRAME_death1_19;
				break;
			case 1:
				self->s.frame = FRAME_death2_01-1;
				self->client->anim_end = FRAME_death2_16;
				break;
			case 2:
				self->s.frame = FRAME_death3_01-1;
				self->client->anim_end = FRAME_death3_28;
				break;
			default:
				self->s.frame = FRAME_death4_01-1;
				self->client->anim_end = FRAME_death4_13;
				break;
			}
			if (meansOfDeath!=MOD_RESTART)
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;

	gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
extern void AutoLoadWeapon( gclient_t *client, gitem_t *weapon, gitem_t *ammo );

void InitClientPersistant (gclient_t *client)
{
	gitem_t		*item, *ammo;
	int		team;	// save team
	int		admin;
	int		spec;
	char	rconx[32],ip[32];
	char	textbuf[TEXTBUFSIZE];


// Papa store player states 
	if (deathmatch->value)
	{
		team = client->pers.team;
		admin = client->pers.admin;
		spec = client->pers.spectator;
		strcpy(rconx,client->pers.rconx);
		strcpy(ip,client->pers.ip);
		strcpy(textbuf,client->pers.textbuf);
	}
	memset(&client->pers, 0, sizeof(client->pers));
	if (deathmatch->value)
	{
		client->pers.team = team;
		client->pers.admin = admin;
		client->pers.spectator = spec;
		strcpy(client->pers.rconx,rconx);
		strcpy(client->pers.ip,ip);
		strcpy(client->pers.textbuf,textbuf);
	}
//begin assault2
	item = FindItem("Crowbar");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	item = FindItem("Pipe");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;




	item = FindItem("Tommygun");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;  // Frank gives you a tommygun
	client->ammo_index = ITEM_INDEX(FindItem(item->ammo));
	client->pers.inventory[client->ammo_index] = 50;
	ammo = FindItem (item->ammo);
	AutoLoadWeapon( client, item, ammo );


	item = FindItem("Shotgun");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;  // Frank gives you a shotgun
	client->ammo_index = ITEM_INDEX(FindItem(item->ammo));
	client->pers.inventory[client->ammo_index] = 8;
	ammo = FindItem (item->ammo);
	AutoLoadWeapon( client, item, ammo );

	//virgin22
	item = FindItem("Pump"); // assault2 virgin17
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;  // Frank gives you a pumpgun
	client->ammo_index = ITEM_INDEX(FindItem(item->ammo));
	client->pers.inventory[client->ammo_index] = 8;
	ammo = FindItem (item->ammo);
	AutoLoadWeapon( client, item, ammo );

	item = FindItem("Spaz"); // assault2 virgin16
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;  // Frank gives you a spazgun
	client->ammo_index = ITEM_INDEX(FindItem(item->ammo));
	client->pers.inventory[client->ammo_index] = 16;
	ammo = FindItem (item->ammo);
	AutoLoadWeapon( client, item, ammo ); // end virgin22


	item = FindItem("Ak47"); // assault2 virgin23
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;
	client->ammo_index = ITEM_INDEX(FindItem(item->ammo));
	client->pers.inventory[client->ammo_index] = 50;
	ammo = FindItem (item->ammo);
	AutoLoadWeapon( client, item, ammo );


	// Ridah, start with Pistol in deathmatch
	switch (team) // HOGIE wants each team to have these weapons. PUREASSAULT
		{
		case 2:


				item = FindItem("FlameThrower");
				client->pers.selected_item = ITEM_INDEX(item);
				client->pers.inventory[client->pers.selected_item] = 1;  // Frank gives you a Flamer
				client->ammo_index = ITEM_INDEX(FindItem(item->ammo));
				client->pers.inventory[client->ammo_index] = 100;

				item = FindItem("Bazooka");
				client->pers.selected_item = ITEM_INDEX(item);
				client->pers.inventory[client->pers.selected_item] = 1;  // Frank gives you a Rocket Launcher
				client->ammo_index = ITEM_INDEX(FindItem(item->ammo));
				client->pers.inventory[client->ammo_index] = 10;
				ammo = FindItem (item->ammo);
				AutoLoadWeapon( client, item, ammo );
				break;
				
		case 1:


				item = FindItem("Heavy machinegun");
				client->pers.selected_item = ITEM_INDEX(item);
				client->pers.inventory[client->pers.selected_item] = 1;  // Frank gives you a hmg
				client->ammo_index = ITEM_INDEX(FindItem(item->ammo));
				client->pers.inventory[client->ammo_index] = 60;
				ammo = FindItem (item->ammo);
				AutoLoadWeapon( client, item, ammo );

				item = FindItem("Grenade Launcher");
				client->pers.selected_item = ITEM_INDEX(item);
				client->pers.inventory[client->pers.selected_item] = 1;  // Frank gives you a Grenade Launcher
				client->ammo_index = ITEM_INDEX(FindItem(item->ammo));
				client->pers.inventory[client->ammo_index] = 3;
				ammo = FindItem (item->ammo);
				AutoLoadWeapon( client, item, ammo );
				break;
		}

		item = FindItem("spistol"); //then gives you the silencer too. Frank start with silencer in DM.
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
		client->ammo_index = ITEM_INDEX(FindItem(item->ammo));
		client->pers.inventory[client->ammo_index] = 50;

		item = FindItem("pistol"); //then gives you the silencer too. Frank start with silencer in DM. // END PUREASSAULT
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
		client->ammo_index = ITEM_INDEX(FindItem(item->ammo));
		client->pers.inventory[client->ammo_index] = 50;
        client->pers.pistol_mods |= WEAPON_MOD_DAMAGE;


//end assault2
		client->pers.weapon = item;

		// Ridah, start with the pistol loaded
		ammo = FindItem (item->ammo);

		AutoLoadWeapon( client, item, ammo );

	if(!deathmatch->value)	// start holstered in single player
	{
		client->pers.holsteredweapon = item;
		client->pers.weapon = NULL;
	}


	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 300;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 25;
	client->pers.max_grenades	= 12;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 90;

	// RAFAEL
	client->pers.max_magslug	= 50;
	client->pers.max_trap		= 5;
	// assault2 virgin18
//	client->pers.shotgunfrags   = 0; // virgin19

	client->pers.connected = true;
}


void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;

	client->resp.checkdelta=level.framenum+17;
	client->resp.checkpvs=level.framenum+23;
	client->resp.checktime=level.framenum+11;
#ifdef DOUBLECHECK
	client->resp.checked=0;
#endif
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE|FL_NOTARGET|FL_POWER_ARMOR));
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->flags |= ent->client->pers.savedFlags;
	if (coop->value)
		ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint // edited by frank for hogie and ASSAULT2

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (edict_t *ent, qboolean team_spawnbase)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;
	qboolean ignoreteams = false;

spotagain:

	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		// For Hogie's style 3 spawners assault2
		if (teamplay->value && ent->client->pers.team && ent->client->pers.game_helpchanged == LOSER)
		{
			if(spot->style == 3)
			{
				ent->client->pers.game_helpchanged = 0; // reset this value when your done with it 
				spot->style = 0; // reset this value so it wont be used twice during a cut scene
				return spot;
			}
			else continue;
		}

		if( (teamplay->value) && (ent->client->pers.team == 1) && (AcceptableThugSpawnerStyle != 1) )
		{
			if( spot->style && (spot->style != AcceptableThugSpawnerStyle) ) continue;

		}
		else if( (teamplay->value) && (ent->client->pers.team == 2) && (AcceptableRuntSpawnerStyle != 2) )
		{
			if( spot->style && (spot->style != AcceptableRuntSpawnerStyle) ) continue;
		}

		else
		// Teamplay, don't go here if it's not in our base
		if (   teamplay->value && ent->client->pers.team
			&& spot->style 
			&& spot->style != ent->client->pers.team
		   )	// Never spawn in the enemy base
		{
			continue;
		}

		if (!ignoreteams && teamplay->value && ent->client->pers.team
			&&	((ent->client->resp.enterframe == level.framenum) || team_spawnbase))
		{
			if (spot->style != ent->client->pers.team)
				continue;
		}
//		else if (spot->style)
//		{
//			continue;	// ignore team spawns if not in a team // Frank wacked this out for DM running team maps
//		}
		// teamplay, done.

		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}

	}

	if (bestspot)
	{
		return bestspot;
	}
	else if (teamplay->value && ent->client->pers.team && !ignoreteams)
	{
		ignoreteams = true;
		goto spotagain;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}


edict_t *SelectDeathmatchSpawnPoint (edict_t *ent)
{
	// Ridah, in teamplay, spawn at base
	if (teamplay->value && ent->client->pers.team)
		return SelectFarthestDeathmatchSpawnPoint (ent, true);//assault2
	else if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint (ent, false);
	else
		return SelectRandomDeathmatchSpawnPoint (ent);
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

	if (deathmatch->value)
		spot = SelectDeathmatchSpawnPoint (ent);
	else if (coop->value)
		spot = SelectCoopSpawnPoint (ent);

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
				gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}

//======================================================================


void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point, int mdx_part, int mdx_subobject)
{
//	int	n;

	if (damage > 50)
	{
		// send the client-side gib message
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GIBS);
		gi.WritePosition (self->s.origin);
		gi.WriteDir (vec3_origin);
		gi.WriteByte ( 20 );	// number of gibs
		gi.WriteByte ( 0 );	// scale of direction to add to velocity
		gi.WriteByte ( 16 );	// random offset scale
		gi.WriteByte ( 200 );	// random velocity scale
		gi.multicast (self->s.origin, MULTICAST_PVS);

		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
	}
}

void HideBody( edict_t *ent )
{
//	ent->svflags |= SVF_NOCLIENT;
	ent->s.effects &= ~EF_FLAMETHROWER;
}

void Body_Animate( edict_t *ent )
{
	ent->s.frame++;

	if (ent->s.frame >= ent->cal)
	{
		ent->s.frame = ent->cal;
	}

	// sink into ground
	if ((ent->timestamp < (level.time - 5)) && ((int)(10.0*level.time) & 1))
	{
		ent->s.origin[2] -= 0.5;

		if (ent->s.origin[2] + 24 < ent->take_cover_time)
		{
			// done with this body
			ent->svflags |= SVF_NOCLIENT;
			return;
		}
	}

	ent->nextthink = level.time + 0.1;
}

void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);

	gi.unlinkentity (body);
	body->s = ent->s;
	body->s.number = body - g_edicts;

	body->cal = ent->client->anim_end;

	body->svflags = ent->svflags;
//	VectorCopy (ent->mins, body->mins);
//	VectorCopy (ent->maxs, body->maxs);

	VectorSet (body->mins, -64, -64, -24);
	VectorSet (body->maxs,  64,  64, -4);

	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);
	body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;

	body->svflags &= ~SVF_NOCLIENT;

	// Ridah so we can shoot the body
	body->svflags |= (SVF_MONSTER | SVF_DEADMONSTER);

	body->cast_info.scale = 1.0;

	body->s.renderfx = 0;
	body->s.renderfx2 = (ent->s.renderfx2 & RF2_ONLY_PARENTAL_LOCKED);
	body->s.renderfx2 |= RF2_NOSHADOW;
	body->s.effects = 0;
	body->s.angles[PITCH] = 0;

	body->gender = ent->gender;
	body->deadflag = ent->deadflag;

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	body->take_cover_time = body->s.origin[2];
	body->timestamp = level.time;

//	body->think = HideBody;
//	body->nextthink = level.time + 30;
	body->think = Body_Animate;
	body->nextthink = level.time + 0.1;

	gi.linkentity (body);
}


void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
		// make sure on the last death frame
//		self->s.frame = self->client->anim_end;

		CopyToBodyQue (self);
		PutClientInServer (self);

		// add a teleportation effect
		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		self->client->respawn_time = level.time;

		return;
	}

	// restart the entire server
	gi.AddCommandString ("menu_loadgame\n");
}

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 48};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ent->move_frame=0;
		ent->client->resp.lastrate=-ent->client->resp.lastrate;
		ClientUserinfoChanged (ent, userinfo);
	}

	else
	{
		if (ent != level.characters[0])
		{
			AddCharacterToGame(ent);
		}

		memset (&resp, 0, sizeof(resp));
	}

	ent->name_index = -1;

	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	if (((teamplay->value) && ((level.modeset == MATCHSETUP) || (level.modeset == FINALCOUNT)))
		|| (level.modeset == FREEFORALL) || (ent->client->pers.spectator == SPECTATING))
	{
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->pers.weapon = NULL;
		ent->client->pers.spectator = SPECTATING;
	}
	else
	{
		ent->movetype = MOVETYPE_WALK;
		ent->solid = SOLID_BBOX;
		ent->svflags &= ~(SVF_DEADMONSTER|SVF_NOCLIENT);
	}
	// RAFAEL
	ent->viewheight = 40;

	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
//	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	
	ent->s.renderfx2 = 0;
	ent->onfiretime = 0;

	ent->onarampage = 0; //assault 2

	ent->cast_info.aiflags |= AI_GOAL_RUN;	// make AI run towards us if in pursuit

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	ent->cast_info.standing_max_z = ent->maxs[2];

	ent->cast_info.scale = MODEL_SCALE;
	ent->s.scale = ent->cast_info.scale - 1.0;

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	// RAFAEL
	// weapon mdx
	{
		int i;
	
		memset(&(client->ps.model_parts[0]), 0, sizeof(model_part_t) * MAX_MODEL_PARTS);

		client->ps.num_parts++;
	// JOSEPH 22-JAN-99
		if (client->pers.weapon)
			client->ps.model_parts[PART_HEAD].modelindex = gi.modelindex(client->pers.weapon->view_model);
		
		for (i=0; i<MAX_MODELPART_OBJECTS; i++)
			client->ps.model_parts[PART_HEAD].skinnum[i] = 0; // will we have more than one skin???
	}

	if (client->pers.weapon)
		client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);
	// END JOSEPH

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
//	ent->s.modelindex2 = 255;		// custom gun model
	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

// bikestuff
ent->biketime = 0;
ent->bikestate = 0;

// Ridah, Hovercars
	if (g_vehicle_test->value)
	{
		if (g_vehicle_test->value == 3)
			ent->s.modelindex = gi.modelindex ("models/props/moto/moto.mdx");
		else
			ent->s.modelindex = gi.modelindex ("models/vehicles/cars/viper/tris_test.md2");

//		ent->s.modelindex2 = 0;
		ent->s.skinnum = 0;
		ent->s.frame = 0;

		if ((int)g_vehicle_test->value == 1)
			ent->flags |= FL_HOVERCAR_GROUND;
		else if ((int)g_vehicle_test->value == 2)
			ent->flags |= FL_HOVERCAR;
		else if ((int)g_vehicle_test->value == 3)
			ent->flags |= FL_BIKE;
		else if ((int)g_vehicle_test->value == 4)
			ent->flags |= FL_CAR;
	}
// done.

	else if (dm_locational_damage->value)	// deathmatch, note models must exist on server for client's to use them, but if the server has a model a client doesn't that client will see the default male model
	{
		char	*s;
		char	modeldir[MAX_QPATH];//, *skins;
		int		len;
		int		did_slash;
		char	modelname[MAX_QPATH];
//		int		skin;

		// NOTE: this is just here for collision detection, modelindex's aren't actually set

		ent->s.num_parts = 0;		// so the client's setup the model for viewing

		s = Info_ValueForKey (client->pers.userinfo, "skin");

//		skins = strstr( s, "/" ) + 1;

		// converts some characters to NULL's
		len = strlen( s );
		did_slash = 0;
		for (i=0; i<len; i++)
		{
			if (s[i] == '/')
			{
				s[i] = '\0';
				did_slash = true;
			}
			else if (s[i] == ' ' && did_slash)
			{
				s[i] = '\0';
			}
		}

		if (strlen(s) > MAX_QPATH-1)
			s[MAX_QPATH-1] = '\0';

		strcpy(modeldir, s);
		
		if (strlen(modeldir) < 1)
			strcpy( modeldir, "male_thug" );
		
		memset(&(ent->s.model_parts[0]), 0, sizeof(model_part_t) * MAX_MODEL_PARTS);
		
		ent->s.num_parts++;
		strcpy( modelname, "players/" );
		strcat( modelname, modeldir );
		strcat( modelname, "/head.mdx" );
		ent->s.model_parts[ent->s.num_parts-1].modelindex = 255;
		gi.GetObjectBounds( modelname, &ent->s.model_parts[ent->s.num_parts-1] );
		if (!ent->s.model_parts[ent->s.num_parts-1].object_bounds[0])
			gi.GetObjectBounds( "players/male_thug/head.mdx", &ent->s.model_parts[ent->s.num_parts-1] );

		ent->s.num_parts++;
		strcpy( modelname, "players/" );
		strcat( modelname, modeldir );
		strcat( modelname, "/legs.mdx" );
		ent->s.model_parts[ent->s.num_parts-1].modelindex = 255;
		gi.GetObjectBounds( modelname, &ent->s.model_parts[ent->s.num_parts-1] );
		if (!ent->s.model_parts[ent->s.num_parts-1].object_bounds[0])
			gi.GetObjectBounds( "players/male_thug/legs.mdx", &ent->s.model_parts[ent->s.num_parts-1] );

		ent->s.num_parts++;
		strcpy( modelname, "players/" );
		strcat( modelname, modeldir );
		strcat( modelname, "/body.mdx" );
		ent->s.model_parts[ent->s.num_parts-1].modelindex = 255;
		gi.GetObjectBounds( modelname, &ent->s.model_parts[ent->s.num_parts-1] );
		if (!ent->s.model_parts[ent->s.num_parts-1].object_bounds[0])
			gi.GetObjectBounds( "players/male_thug/body.mdx", &ent->s.model_parts[ent->s.num_parts-1] );

		ent->s.num_parts++;
		ent->s.model_parts[PART_GUN].modelindex = 255;
	}
	else	// make sure we can see their weapon
	{
		memset(&(ent->s.model_parts[0]), 0, sizeof(model_part_t) * MAX_MODEL_PARTS);
		ent->s.model_parts[PART_GUN].modelindex = 255;
		ent->s.num_parts = PART_GUN+1;	// make sure old clients recieve the view weapon index
	}

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.

  NOTE: called every level load/change in deathmatch
=====================
*/
extern void Teamplay_AutoJoinTeam( edict_t *self );


// Papa - Here is where I control how a player enters the game 

void ClientBeginDeathmatch (edict_t *ent)
{
	int		save;

//gi.dprintf("IN: ClientBeginDeathmatch(%d)\n",((int)ent-(int)g_edicts)/sizeof(edict_t));

	save = ent->client->showscores;
	G_InitEdict (ent);

	if ((level.modeset == FREEFORALL) || (level.modeset == TEAMPLAY))
	{
		InitClientResp (ent->client);
	}
	// locate ent at a spawn point
	PutClientInServer (ent);

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// Teamplay: if they aren't assigned to a team, make them a spectator
	if (teamplay->value)
	{
		if ((level.modeset == MATCHSETUP) || (level.modeset == FINALCOUNT) || (level.modeset == FREEFORALL) || (level.modeset == ENDMATCHVOTING))
		{
			ent->movetype = MOVETYPE_NOCLIP;
			ent->solid = SOLID_NOT;
			ent->svflags |= SVF_NOCLIENT;
			ent->client->pers.weapon = NULL;
			ent->client->pers.spectator = SPECTATING;
		}
		else if (ent->client->pers.team) 
		{
			// so we don't KillBox() ourselves
			ent->solid = SOLID_NOT;
			gi.linkentity( ent );

			if (!Teamplay_ValidateJoinTeam( ent, ent->client->pers.team ))
			{
				ent->client->pers.team = 0;
				ent->client->pers.spectator = SPECTATING;
			}

		}

		if (!ent->client->pers.team)
		{
			if (((int)dmflags->value) & DF_AUTO_JOIN_TEAM)
			{
				Teamplay_AutoJoinTeam( ent );
			}
			else
			{
				ent->movetype = MOVETYPE_NOCLIP;
				ent->solid = SOLID_NOT;
				ent->svflags |= SVF_NOCLIENT;
				ent->client->pers.spectator = SPECTATING;
				ent->client->pers.weapon = NULL;
			}
		}
	}
	else
	{
		if ((ent->client->pers.spectator == SPECTATING) || (ent->client->showscores == SCORE_REJOIN) || (level.modeset == ENDMATCHVOTING))
		{
			ent->movetype = MOVETYPE_NOCLIP;
			ent->solid = SOLID_NOT;
			ent->svflags |= SVF_NOCLIENT;
			ent->client->pers.weapon = NULL;
			ent->client->pers.spectator = SPECTATING;
		}
		else if (level.modeset != STARTINGPUB)
		{
			gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
//			sl_WriteStdLogPlayerEntered( &gi, level, ent );	// Standard Logging
		}
	}
	ent->client->showscores = save;
 

	if (level.modeset == FINALCOUNT)
		ent->client->showscores = NO_SCOREBOARD;
	else if (level.modeset == MATCHSETUP)
		ent->client->showscores = SCOREBOARD;
	else if ((level.modeset == FREEFORALL) && (ent->client->showscores == NO_SCOREBOARD))
		ent->client->showscores = SCOREBOARD;
	else if (ent->solid != SOLID_NOT)
		ent->client->showscores = NO_SCOREBOARD;

	if (ent->client->pers.spectator) UPDATERATE(ent)

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);


	// If they're using an old version, make sure they're aware of it
	if (ent->client->pers.version < 121)
	{
		gi.centerprintf( ent, "You are using an old version\nof Kingpin.\n\nGet the upgrade at:\n\nhttp://www.interplay.com/kingpin" );
	}

//gi.dprintf("OUT: ClientBeginDeathmatch(%d)\n",((int)ent-(int)g_edicts)/sizeof(edict_t));
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
extern void MoveClientToPawnoMatic (edict_t *ent);	// note to Rafael, was causing an undefined warning
extern void ED_CallSpawn (edict_t *ent);

int	num_followers = 0;
follower_t	followers[MAX_FOLLOWERS];

extern int client_connected;

extern qboolean	changing_levels;
extern void Cmd_HolsterBar_f (edict_t *ent);

void ClientBegin (edict_t *ent)
{
	int		i;
	char *a;

	client_connected = 1;

	ent->client = game.clients + (ent - g_edicts - 1);

//Chief reset some things
	ent->client->pers.game_helpchanged = 0;
//	ent->count = 0; // assault virgin11 // crashes server

// Papa - either show rejoin or MOTF scoreboards	
	if (ent->client->showscores != SCORE_REJOIN)
		ent->client->showscores = SCORE_MOTD;

	ent->client->pers.admin=NOT_ADMIN;
	if (keep_admin_status) {
		a=gi.cvar("modadmin","",0)->string;
		if (a[0]) {
			edict_t *player;
			for (i=0 ; i<maxclients->value ; i++) {
				player = g_edicts + 1 + i;
				if (!player->inuse) continue;
				if (player->client->pers.admin>NOT_ADMIN) {
					gi.cvar_set("modadmin","");
					break;
				}
			}
			if (i==maxclients->value) {
				if (!strcmp(a,ent->client->pers.ip)) {
					gi.cvar_set("modadmin","");
					ent->client->pers.admin = ELECTED;
					gi.bprintf(PRINT_HIGH,"%s is now admin.\n",ent->client->pers.netname);
				}
			}
		}
	}
	a=gi.cvar("rconx","",0)->string;
	if (a[0]) {
		char *s,buf[256];
		strcpy(buf,a);
		if (s=Info_ValueForKey(buf,ent->client->pers.ip)) {
			strcpy(ent->client->pers.rconx,s);
			Info_RemoveKey(buf,ent->client->pers.ip);
			gi.cvar_set("rconx",buf);
		}
	}

	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		return;
	}

	ent->cast_group = 1;

	// Ridah, copy the episode_flags over
	ent->episode_flags = ent->client->pers.episode_flags;

	level.speaktime = 0;

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else if (level.cut_scene_time)
	{
		MoveClientToCutScene (ent);
	}
	else if (level.pawn_time || strstr (level.mapname, "pawn_"))
	{
		level.pawn_time = 1.0;
		MoveClientToPawnoMatic (ent);
	}
	else if (strstr (level.mapname, "bar_"))
	{
		level.bar_lvl = true;
		Cmd_HolsterBar_f (ent);
		
		if (level.episode == 1)
		{
		//ent->episode_flags |= EP_BAR_FIRST_TIME;
		//ent->client->pers.episode_flags |= EP_BAR_FIRST_TIME;
		EP_Skidrow_Register_EPFLAG (ent, EP_BAR_FIRST_TIME);
		}
		else if (level.episode == 2)
		{
			EP_Skidrow_Register_EPFLAG (ent, EP_PV_BAR_FIRST_TIME);
		}
		
	}
	else if (strstr(level.mapname, "office_"))
	{
		level.bar_lvl = true;
		Cmd_HolsterBar_f (ent);
		
		if (level.episode == 2)
		{
			EP_Skidrow_Register_EPFLAG (ent, EP_PV_OFFICE_FIRST_TIME);
		}
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGIN);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
		}
	}

	// Ridah, restore any following characters
	if (num_followers > 0)
	{
		int j, k;
		follower_t	*fol;
		edict_t		*newent, *spawnspot;
		qboolean	killed, fakespawn;

		if (num_followers > MAX_FOLLOWERS)
			num_followers = MAX_FOLLOWERS;

		for (i=0; i<num_followers; i++)
		{
			fol = &followers[i];
			killed = false;

			// spawn a similar entity

			newent = G_Spawn();

			// copy the necessary data for spawning
			newent->classname = gi.TagMalloc( strlen( fol->classname ) + 1, TAG_LEVEL );
			strcpy( newent->classname, fol->classname );

			if (fol->name)
			{
				newent->name = gi.TagMalloc( strlen( fol->name ) + 1, TAG_LEVEL );
				strcpy( newent->name, fol->name );
			}

			if (fol->art_skins)
			{
				newent->art_skins = gi.TagMalloc( strlen( fol->art_skins ) + 1, TAG_LEVEL );
				strcpy( newent->art_skins, fol->art_skins );
			}

			newent->cast_info.scale = fol->scale;
			newent->head = fol->head;
			newent->cast_group = 1;
			newent->spawnflags = fol->spawnflags;
			newent->count = fol->count;

			// find a spawn spot
			spawnspot = NULL;
			while (1)
			{
				if (!(spawnspot = G_Find( spawnspot, FOFS(classname), "info_player_coop" )))
					break;

				if (VectorDistance( spawnspot->s.origin, ent->s.origin ) > 384)
					continue;

				if (!ValidBoxAtLoc( spawnspot->s.origin, tv(-16, -16, -24), tv(16, 16, 48), NULL, MASK_PLAYERSOLID|CONTENTS_MONSTERCLIP ))
				{
					if(developer->value)
						gi.dprintf( "WARNING: coop spawn in solid at: %i, %i, %i\n", (int)spawnspot->s.origin[0], (int)spawnspot->s.origin[1], (int)spawnspot->s.origin[2] );
					continue;
				}

				if (!(spawnspot->spawnflags & 0x10000000))
				{
					spawnspot->spawnflags |= 0x10000000;
					break;
				}
			}

			fakespawn = false;

			if (!spawnspot)
			{
				vec3_t	 vec;

				gi.dprintf( "** WARNING: Unable to find a coop spawn for %s. Hacking a spawn spot.\n", fol->classname );

				spawnspot = G_Spawn();
				AngleVectors( ent->s.angles, vec, NULL, NULL );

				VectorMA( ent->s.origin, -48*(i+1), vec, spawnspot->s.origin );
				VectorCopy( ent->s.angles, spawnspot->s.angles );

				fakespawn = true;
			}

			VectorCopy( spawnspot->s.origin, newent->s.origin );
			newent->s.origin[2] += 1;
			
			VectorCopy( spawnspot->s.angles, newent->s.angles );

			if (fakespawn)
				G_FreeEdict( spawnspot );

			// add it to the characters listing
			if (killed)
			{
				level.characters[j] = newent;
			}
			else
			{
				AddCharacterToGame( newent );
			}


			// spawn it!
			ED_CallSpawn( newent );		// will get added to the game in here


			// make them aware of and hired by us
			AI_RecordSighting( newent, ent, 64 );		// dist = 64 will do, will get updated next sight anyway
			level.global_cast_memory[newent->character_index][ent->character_index]->flags |= (MEMORY_HIRED | MEMORY_HIRE_FIRST_TIME | MEMORY_HIRE_ASK);

			// make them follow us
			newent->leader = ent;

			// restore pain skins
			for (j=0; j<newent->s.num_parts; j++)
			{
				for (k=0; k<MAX_MODELPART_OBJECTS; k++)
				{
					newent->s.model_parts[j].skinnum[k] += (byte)fol->skinofs[j][k];
				}
			}

			// restore health
			newent->health = fol->health;
			newent->max_health = fol->max_health;

		}

		num_followers = 0;

		// clear coop spawnflags
		spawnspot = NULL;
		while (1)
		{
			if (!(spawnspot = G_Find( spawnspot, FOFS(classname), "info_player_coop" )))
				break;

			spawnspot->spawnflags &= ~0x10000000;
		}
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);

	// Ridah, if we've come from another level, save the current game (fixes hired guy's disappearing after restarting a level after dying)
	if (changing_levels)
	{
		gi.SaveCurrentGame();
		changing_levels = false;
	}
}

void maxrate_think(edict_t *self)
{
	edict_t *ent=self->owner;
	gi.cprintf(ent, PRINT_HIGH, "Server restricting rate to %s\n",Info_ValueForKey(ent->client->pers.userinfo,"rate"));
	ent->client->resp.ratetext=1;
	G_FreeEdict(self);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s,*s2;
//	char	*fog;
	int		playernum;
	char	*extras;
    int     fIgnoreName = 0,a;

//gi.dprintf("in CUC: %d %d\n",ent->client->resp.enterframe,ent->client->resp.lastrate);

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		// strcpy (userinfo, "\\name\\badinfo\\skin\\male_thug/018 016 010\\extras\\0");
		strcpy (userinfo, "\\name\\badinfo\\skin\\male_thug/009 019 017\\extras\\0");
	}

	// set name
    /* Standard Logging - don't change name */
/*    if( '\0' != ent->client->pers.netname[0] )
    {
        Info_SetValueForKey (userinfo, "name", ent->client->pers.netname);
        fIgnoreName = 1;
    }
*/    
	// set name
	s = Info_ValueForKey (userinfo, "name");
	if (s2=strchr(s,'%')) {
		while (s2=strchr(s,'%')) *s2=' ';
		Info_SetValueForKey (userinfo, "name", s);
	}
	for (a=strlen(s)-1;a>=0 && s[a]<33;a--) ;
	if (a==-1) {
//		if( '\0' != ent->client->pers.netname[0] )
//			fIgnoreName = 1;
//		else
			s=NAME_BLANK_STR;
	} else {
	s[a+1]=0;

    // Standard Logging - stop name clashes
    if( !fIgnoreName )
    {
        edict_t		*cl_ent;
        unsigned int i;

        for (i=0 ; i<game.maxclients ; i++)
        {
            cl_ent = g_edicts + 1 + i;
            if( cl_ent->inuse &&
                (cl_ent != ent) &&
                !strcmp(cl_ent->client->pers.netname, s) )
            {
//                if( '\0' != ent->client->pers.netname[0] )
  //                  fIgnoreName = 1;
    //            else
                    s = NAME_CLASH_STR;

                Info_SetValueForKey (userinfo, "name", s);
            }
        }
    }
	}
    // Standard Logging

    if( !fIgnoreName )
    {
        // start - Standard Logging
        // Has the player got a name
/*        if( strlen(ent->client->pers.netname) )
        {
            // has the name changed
            if( strcmp( ent->client->pers.netname, s ) )
            {
                // Standard Logging -  log player rename
                sl_LogPlayerRename( &gi,
                                    ent->client->pers.netname,
                                    s,
                                    level.time );
            }
        }
*/        // end - Standard Logging

        strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);
    }

	for (a=0;a<num_netnames;a++) {
		if (Q_strcasecmp (netname[a].value, s) == 0) {
			KICKENT(ent,"%s is being kicked because they're banned!\n");
		}
	}

	// check maxrate
	if (ent->client->resp.lastrate>=0) {
		s = Info_ValueForKey (userinfo, "rate");
		if (strlen(s) && (a=atoi(s))) {
			if (specrate && ent->client->pers.spectator && ent->client->resp.enterframe
				&& !level.intermissiontime && (level.modeset==TEAMPLAY || level.modeset==MATCH
				|| level.modeset==STARTINGPUB || level.modeset==STARTINGMATCH)) {
				if (a>specrate) {
					if (ent->client->resp.ratetext!=2 && ent->client->resp.lastrate!=a) {
						gi.cprintf(ent, PRINT_HIGH, "Server restricting spectator rate to %d\n",specrate);
						ent->client->resp.ratetext=2;
					}
					Info_SetValueForKey(userinfo,"rate",va("%i",specrate));
				}
			} else if ((int)maxrate->value && a>(int)maxrate->value) {
				if (!ent->client->resp.enterframe) {
					edict_t *thinker;
					thinker = G_Spawn();
					thinker->think = maxrate_think;
					thinker->nextthink = level.time + 2 + random()*2;
					thinker->owner = ent;
				} else if (ent->client->resp.lastrate!=a && ent->client->resp.ratetext!=1) {
					gi.cprintf(ent, PRINT_HIGH, "Server restricting rate to %d\n",(int)maxrate->value);
					ent->client->resp.ratetext=1;
				}
				Info_SetValueForKey( userinfo, "rate", va("%i", (int)maxrate->value) );
				ent->client->resp.ratetext=0;
			} else if (ent->client->resp.enterframe && ent->client->resp.ratetext==2 && ent->client->resp.lastrate!=a) {
				gi.cprintf(ent, PRINT_HIGH, "Server setting rate to %d\n",a);
				ent->client->resp.ratetext=0;
			}
			ent->client->resp.lastrate=a;
		}
	} else
		ent->client->resp.lastrate=-ent->client->resp.lastrate;

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	if (ent->skins[0] && level.framenum<(ent->move_frame+10)) {
		Info_SetValueForKey( userinfo, "skin", ent->skins);
//		gi.cprintf( ent, PRINT_HIGH, "change skin %d\n",ent->move_frame);
	} else {

	// Ridah, HACK for teamplay demo, set skins manually
	if (deathmatch->value && teamplay->value && ent->client->pers.team)
	{
		// NOTE: skin order is "HEAD BODY LEGS"
		char *skin, *body, *legs;
		char tempstr[MAX_QPATH];
		int i, valid, model_index;

		// Hard-coded skin sets for each model

		static char *valid_models[] = { "female_chick", "male_thug", "male_runt", NULL };
		static char *valid_skinsets[][2][2][2] =

			// ordering here is {"LEGS", "BODY"}
			{
				{	// Bitch
					{{"056","057"}, {"056","058"}},		// Team 1
					{{"033","032"}, {"031","031"}}		// Team 2
				},
				{	// Thug
					{{"057","056"}, {"058","091"}},
					{{"031","031"}, {"032","035"}}
				},
				{	// Runt
					{{"058","056"}, {"057","056"}},
					{{"031","030"}, {"032","031"}}
				}
			};

		// make sure they are using one of the standard models
		valid = false;
		i = 0;

//      Frank wacked this out to allow for forcing team thug/runt models ASSAULT2
//		strcpy( tempstr, s ); //tempstr now equals s          example: tempstr = "male_thug/001 001 001";
			if(ent->client->pers.team==1) //Thugs
				strcpy( tempstr, "male_thug/001 001 001" );
			else if(ent->client->pers.team==2) //Runts
				strcpy( tempstr, "male_runt/001 001 001" );

			else // ghost bug fix
			{
				strcpy( tempstr, "male_runt/001 001 001" );
				ent->client->pers.team=2;
			}
//      End Frank ASSAULT2

		skin = strrchr( tempstr, '/' );

		if (!skin)
		{	// invalid model, so assign a default
			model_index = 0;
			strcpy( tempstr, valid_models[model_index] );

			// also recreate a new skin for "s"
			strcpy( s, tempstr );
			strcat( s, "/001 001 001" );

			valid = true;
		}
		else
		{
			skin[0] = '\0';

			while (valid_models[i])
			{
				if (!Q_stricmp( tempstr, valid_models[i] ))
				{
					valid = true;
					model_index = i;
					break;
				}

				i++;
			}
		}

		if (!valid)
		{	// assign a random model
			model_index = -1;

			// look for a gender match
			while (valid_models[i])
			{
				if (!strncmp( tempstr, valid_models[i], 4 ))
				{
					model_index = i;
					strcpy( tempstr, valid_models[model_index] );
					break;
				}

				i++;
			}

			if (model_index < 0)
			{
				model_index = rand()%i;
				strcpy( tempstr, valid_models[model_index] );
			}
		}

		// At this point, tempstr = model only (eg. "male_thug")

		// check that skin is valid
		skin = strrchr( s, '/' ) + 1;
		skin[3] = skin[7] = '\0';

		body = &skin[4];
		legs = &skin[8];

		valid = false;

		for (i=0; i<2; i++)
		{
			if (	!Q_stricmp( body, valid_skinsets[model_index][ent->client->pers.team-1][i][1] )
				&&	!Q_stricmp( legs, valid_skinsets[model_index][ent->client->pers.team-1][i][0] ))
			{
				valid = true;
				break;
			}
		}

		if (!valid)
		{	// Assign a random skin for this model
			i = rand()%2;

			strcpy( body, valid_skinsets[model_index][ent->client->pers.team-1][i][1] );
			strcpy( legs, valid_skinsets[model_index][ent->client->pers.team-1][i][0] );
		}

		skin[3] = skin[7] = ' ';

		// paste the skin into the tempstr
		strcat( tempstr, "/" );
		strcat( tempstr, skin );

		Info_SetValueForKey( userinfo, "skin", tempstr );
		strcpy(ent->skins,tempstr);
	}
	else if (!deathmatch->value)	// enforce thug with single player skin set
	{
		static char *singleplayerskin = "male_thug/018 016 010";

		Info_SetValueForKey( userinfo, "skin", singleplayerskin );
	} else
		strcpy(ent->skins,s);
	}

	// now check it again after the filtering, and set the Gender accordingly
	s = Info_ValueForKey (userinfo, "skin");

	if ((strstr(s, "female") == s))
		ent->gender = GENDER_FEMALE;
	else if ((strstr(s, "male") == s) || (strstr(s, "thug")))
		ent->gender = GENDER_MALE;
	else
		ent->gender = GENDER_NONE;

	extras = Info_ValueForKey (userinfo, "extras");

	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s %s", ent->client->pers.netname, s, extras) );

	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	/*
	{
		vec3_t vars1, vars2;

		fog = Info_ValueForKey (userinfo, "fogcolor");

		if (strlen (fog) == 17)
		{
			int i, cnt;
			char *varR, *varG, *varB;
			
			for (i=0; i<17; i++)
			{
				if (i < 5)
				{
					varR[i] = fog[i];
				}
				else if (i == 5)
					continue;
				else if (i < 11)
				{
					varG[i-5] = fog[i];
				}
				else if (i == 11)
					continue;
				else
				{
					varB[i-11] = fog[i];
				}
			}

			vars1[0] = atof (varR);
			vars1[1] = atof (varG);
			vars1[2] = atof (varB);

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_FOG_BRUSH);

			gi.WritePosition (vars1);
			gi.WritePosition (vars2);

			gi.multicast (ent->s.origin, MULTICAST_PVS);

			gi.dprintf ("fog color <%s %s %s>\n", varR, varG, varB); 
		}
		else
			gi.dprintf ("must be in <0.000 0.000 0.000> format\n");
	}
	*/

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

	// client exe version
	s = Info_ValueForKey (userinfo, "ver");
	if (s && strlen(s))
	{
		ent->client->pers.version = atoi(s);
	}
	else	// assume client is old version
	{
		ent->client->pers.version = 100;
	}

	s = Info_ValueForKey (userinfo, "ip");
	if (strlen(s))
	{
		strcpy(ent->client->pers.ip,s);
	}

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);

//gi.dprintf("out CUC\n");
}



/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char	*value, *skinvalue;
	int		i;

	ent->client = NULL;
	ent->inuse = false;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	if (SV_FilterPacket(value))
		return false;

	// check for a password
	value = Info_ValueForKey (userinfo, "password");
	if (strcmp(password->string, value) != 0)
		return false;
	
	if (CheckPlayerBan (ent, userinfo))
		return false;

// Ridah, if this isn't a loadgame, try to add them to the character list
	if (!deathmatch->value && (ent->inuse == false))
	{
		if (!AddCharacterToGame(ent))
		{
			return false;
		}
	}
// Ridah, done.

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon) {
			memset(ent->client->pers.rconx,0,32);
			ent->client->pers.textbuf[0]=0;
			InitClientPersistant (ent->client);
		}

		// JOSEPH 14-MAR-99
		if (!strcmp(level.mapname, "sr1") || !strcmp(level.mapname, "kpcut1"))
		{
			if (!(game.maxclients > 1))
			{
				ent->client->pers.health = 68;
				ent->health = 68;
			}	
		}
		// END JOSEPH	
	}

	ent->client->resp.enterframe = 0;
	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s (%s) connected\n", ent->client->pers.netname,ent->client->pers.ip);

	// Ridah, make sure they have to join a team

	if (teamplay->value)
		ent->client->pers.team = 0;

// check to see if a player was disconnected
	i = 0;
	skinvalue = Info_ValueForKey (userinfo, "skin");
	while (i < level.player_num)
	{
		if (!teamplay->value)
			if ((Q_stricmp (ent->client->pers.netname,playerlist[i].netname) == 0) && 
				(Q_stricmp (skinvalue,playerlist[i].skin) == 0))
				ent->client->showscores = SCORE_REJOIN;
			else;
		else if (Q_stricmp (ent->client->pers.netname,playerlist[i].netname) == 0)
				ent->client->showscores = SCORE_REJOIN;

		i++;
	}

	if (!teamplay->value) ent->client->pers.spectator = PLAYING;

	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;
	int		i;
	char	*skinvalue;

	if (!ent->client)
		return;

//	sl_LogPlayerDisconnect( &gi, level, ent );	// Standard Logging

// Papa - store player info after they disconnect	
	if ((ent->client->pers.spectator == PLAYING) && (level.player_num < 63))
	{
		playerlist[level.player_num].frags = ent->client->resp.score;
		playerlist[level.player_num].deposits = ent->client->resp.deposited;
		playerlist[level.player_num].team = ent->client->pers.team;
		playerlist[level.player_num].time = level.framenum - ent->client->resp.enterframe;

		skinvalue = Info_ValueForKey (ent->client->pers.userinfo, "skin");
		strcpy (playerlist[level.player_num].skin, skinvalue);
		
		strcpy (playerlist[level.player_num].netname, ent->client->pers.netname);
		level.player_num++;
	}

	for (i=1; i<=maxclients->value; i++)
	{
		if (!g_edicts[i].inuse)
			continue;
		if (!g_edicts[i].client)
			continue;
		if (g_edicts[i].client->chase_target == ent) {
			g_edicts[i].client->chase_target = NULL;
			g_edicts[i].client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		}
	}

	{ //assault2
	static int i;
	DropKeys(ent);

	i = rand()%8;
	
	switch (i)
	{
	default:
	case 0:
		gi.bprintf (PRINT_HIGH, "%s fled the scene.\n", ent->client->pers.netname);
		break;
	case 1:
		gi.bprintf (PRINT_HIGH, "%s checked out.\n", ent->client->pers.netname);
		break;
	case 2:
		gi.bprintf (PRINT_HIGH, "%s will see you in hell.\n", ent->client->pers.netname); // MostWanted
		break;
	case 3:
		gi.bprintf (PRINT_HIGH, "%s bounced.\n", ent->client->pers.netname);
		break;
	case 4:
		gi.bprintf (PRINT_HIGH, "%s left town.\n", ent->client->pers.netname);
		break;
	case 5:
		gi.bprintf (PRINT_HIGH, "%s couldn't hang.\n", ent->client->pers.netname);
		break;
	case 6:
		gi.bprintf (PRINT_HIGH, "%s is on the run.\n", ent->client->pers.netname);
		break;
	case 7:
		gi.bprintf (PRINT_HIGH, "%s is outta here.\n", ent->client->pers.netname);
		break;
	}}//end assault2
		
	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->s.num_parts = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
}

//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
	{
		if (nav_dynamic->value)	// if dynamic on, get blocked by MONSTERCLIP brushes as the AI will be
			return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID | CONTENTS_MONSTERCLIP);
		else
			return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	}
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client; //client = ent->client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;

	if (ucmd->forwardmove|ucmd->sidemove|ucmd->upmove)
	{
		ent->move_frame=level.framenum;//(ent->move_frame>>3)|ucmd->buttons|ucmd->forwardmove|ucmd->sidemove|ucmd->upmove;
		ent->onarampage = 0; //assault2 for turret weapon holstering/unholstering
	}
//gi.bprintf(PRINT_HIGH,"%d\n",ent->move_frame);

	// BOT! (*nothing* uses impulse command so safe to assume)
	if (ucmd->impulse) {
		gi.dprintf("Received impulse: %s (%d)\n",ent->client->pers.netname,ucmd->impulse);
		KICKENT(ent,"%s is being kicked for using a bot!\n");
	}

	if( ent->groundentity && (sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]) > 500) ) 
	{
//		gi.bprintf(PRINT_HIGH, "%s speed: %.0f exceeds 500.  vtos: %s.\n", ent->client->pers.netname, sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]), vtos(ent->velocity) );

		ent->velocity[0] -= (ent->velocity[0]*0.33); // subtract 33% until your speed is normal.
		ent->velocity[1] -= (ent->velocity[1]*0.33); // this works for both (+) and (-) numbers.
		
//		gi.bprintf(PRINT_HIGH, "%s speed reduced to %.0f.  vtos: %s.\n", ent->client->pers.netname, sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]), vtos(ent->velocity) );
	}

	level.current_entity = ent;
	client = ent->client;

	if (client->pers.textbuf[0]) {
		j=strlen(client->pers.textbuf);
		if (j>80) {
			char b[TEXTBUFSIZE];
			strcpy(b,client->pers.textbuf);
			for (i=80;i<j;i++)
				if (b[i]=='\n') {
					i++;
					b[i]=0;
					break;
				}
			gi.cprintf(ent,PRINT_HIGH,"%s",b);
			if (i<j)
				memmove(client->pers.textbuf,client->pers.textbuf+i,j+1-i);
			else
				client->pers.textbuf[0]=0;
		} else {
			gi.cprintf(ent,PRINT_HIGH,"%s",client->pers.textbuf);
			client->pers.textbuf[0]=0;
		}
	}

	// JOSEPH 24-FEB-99
	if (level.cut_scene_end_count)
	{
		level.cut_scene_end_count--;

		if (!level.cut_scene_end_count)
			level.cut_scene_camera_switch = 0;
	}
	// END JOSEPH

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 11.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}
	// RAFAEL
	else if (level.cut_scene_time)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		
		// note to self
		// need to do any precanned player move stuff
		
		if (level.time > level.cut_scene_time + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.cut_scene_time = 0;

		return;
	}
	else if (level.pawn_time)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		return;
	}

	pm_passent = ent;

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (ent->client->chase_target)
	{
		if (ent->solid != SOLID_NOT)
		{	// stop chasing
			ent->client->chase_target = NULL;
			ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		}
		else
		{
			goto chasing;
		}
	}

	if (ent->flags & FL_CHASECAM)
	{
		client->ps.pmove.pm_flags |= PMF_CHASECAM;
	}
	else
	{
		client->ps.pmove.pm_flags &= ~PMF_CHASECAM;
	}

//assault2
		 if (ent->durationflag == 86)			client->ps.pmove.pm_type = PM_FREEZE; //PUREASSAULT this is triggered by Cmd_Freeze_f in G_cmds.c
	else if (ent->movetype == MOVETYPE_NOCLIP)	client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != 255)			client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)						client->ps.pmove.pm_type = PM_DEAD;
	else										client->ps.pmove.pm_type = PM_NORMAL; // PUREASSAULT This is simpler jet pack aint used anyway

chasing:

	client->ps.pmove.gravity = sv_gravity->value;
	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
	{
		pm.snapinitial = true;
//		gi.dprintf ("pmove changed!\n");
	}

	pm.cmd = *ucmd;

	pm.trace = PM_trace;	// adds default parms

	pm.pointcontents = gi.pointcontents;

	// perform a pmove
	gi.Pmove (&pm);

	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	// JOSEPH 1-SEP-98
	ent->footsteptype = pm.footsteptype;
	
	for (i=0 ; i<3 ; i++)
	{
		ent->s.origin[i] = pm.s.origin[i]*0.125;
		ent->velocity[i] = pm.s.velocity[i]*0.125;
	}

	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	if (ent->groundentity 
		&& !pm.groundentity 
		&& (pm.cmd.upmove >= 10) 
		&& (pm.waterlevel == 0))
	{
		int rval;
		rval = rand()%100;
		if (rval > 66)	
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
		else if (rval > 33)
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump2.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump3.wav"), 1, ATTN_NORM, 0);
			
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}

	ent->viewheight = pm.viewheight;
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	if (pm.groundentity)
	{
		ent->groundentity_linkcount = pm.groundentity->linkcount;

		// if standing on an AI, get off
		if (pm.groundentity->svflags & SVF_MONSTER)
		{
			VectorSet( ent->velocity, rand()%400 - 200, rand()%400 - 200, 200 );

			if (pm.groundentity->maxs[2] == pm.groundentity->cast_info.standing_max_z)
			{	// duck
				if (pm.groundentity->cast_info.move_crouch_down)
					pm.groundentity->cast_info.currentmove = pm.groundentity->cast_info.move_crouch_down;
				pm.groundentity->maxs[2] = DUCKING_MAX_Z;
			}

			// avoid
			pm.groundentity->cast_info.avoid( pm.groundentity, ent, false );

		}
	}

	if (ent->deadflag)
	{
		client->ps.viewangles[ROLL] = 40;
		client->ps.viewangles[PITCH] = -15;
		client->ps.viewangles[YAW] = client->killer_yaw;
	}
	else
	{
		VectorCopy (pm.viewangles, client->v_angle);
		VectorCopy (pm.viewangles, client->ps.viewangles);
	}

	gi.linkentity (ent);

	if (ent->movetype != MOVETYPE_NOCLIP)
		G_TouchTriggers (ent);

	// touch other objects
	for (i=0 ; i<pm.numtouch ; i++)
	{
		other = pm.touchents[i];
		for (j=0 ; j<i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated
		if (!other->touch)
			continue;
		other->touch (other, ent, NULL, NULL);
	}

	// JOSEPH 22-JAN-99
	// Activate button is pressed
	if (((client->latched_buttons|client->buttons) & BUTTON_ACTIVATE))
	{
		edict_t		*trav, *best;
		float		best_dist=9999, this_dist;
	
		// find the nearest pull-enabled object 
		trav = best = NULL;
		while (trav = findradius( trav, ent->s.origin, 48 ))
		{
			if (!trav->pullable)
				continue;
			if (((this_dist = VectorDistance(ent->s.origin, trav->s.origin)) > best_dist) && (this_dist > 64))
				continue;
			
			best = trav;
			best_dist = this_dist;
		}

		// If we find something to drag
		if (best)
		{
			cplane_t plane;
			
			plane.type = 123;
			best->touch (best, ent, &plane, NULL);	
		
			// Slow down the player
			// JOSEPH 24-MAY-99
			ent->velocity[0] /= 8;
			ent->velocity[1] /= 8;
			// END JOSEPH
		}
	}
	// END JOSEPH


	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;
	
	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK)
	{
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

	Think_FlashLight (ent);

	// BEGIN:	Xatrix/Ridah/Navigator/18-mar-1998
	if (!deathmatch->value && nav_dynamic->value && !(ent->flags & (FL_HOVERCAR_GROUND | FL_HOVERCAR | FL_BIKE | FL_CAR)))
	{
		static float alpha;

		// check for nodes
		NAV_EvaluateMove( ent );

		// optimize routes (flash screen if lots of optimizations
		if (NAV_OptimizeRoutes( ent->active_node_data ) > 50)
		{
			alpha += 0.05;
			if (alpha > 1)
				alpha = 1;
		}
		else if (alpha > 0)
		{
			alpha -= 0.05;
		}

		if (nav_debug->value)
			ent->client->bonus_alpha = alpha;
	}
	// END:		Xatrix/Ridah/Navigator/18-mar-1998

	// Ridah, new AI
	if (maxclients->value == 1)
	{
		AI_UpdateCharacterMemories( 16 );
	}
	// done.

	// Ridah, special burn surface code for artists
	if ((maxclients->value == 1) && burn_enabled->value)
	{
		static vec3_t	last_endpos;

		if (!(client->buttons & BUTTON_ATTACK))
		{	// next press must draw, since they've just hit the attack button
			last_endpos[0] = -9999;
			last_endpos[1] = -9999;
			last_endpos[2] = -9999;
		}
		else if (num_lpbuf >= 0xFFFF)
		{
			gi.dprintf( "LightPaint buffers are full, you must save to continue painting.\n");
		}
		else
		{
			trace_t tr;
			vec3_t	start, end, fwd;

			VectorCopy( ent->s.origin, start );
			start[2] += ent->viewheight;

			AngleVectors( ent->client->v_angle, fwd, NULL, NULL );

			VectorMA( start, 4000, fwd, end );

			tr = gi.trace( start, NULL, NULL, end, ent, (MASK_OPAQUE & ~CONTENTS_MONSTER) );

			if (tr.fraction < 1 && (VectorDistance( last_endpos, tr.endpos ) > ((float)burn_size->value)*0.5))
			{
				VectorMA( tr.endpos, 1, tr.plane.normal, last_endpos );

				// spawn a burn ent at this location
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_ART_BURN);
				gi.WritePosition (last_endpos);

				gi.WriteShort( (int)burn_size->value );

				gi.WriteByte ( (int) (255.0 * (float)burn_r->value) );
				gi.WriteByte ( (int) (255.0 * (float)burn_g->value) );
				gi.WriteByte ( (int) (255.0 * (float)burn_b->value) );

				gi.WriteByte ( (int) (127.0 * (float)burn_intensity->value) + 127 );

				gi.multicast (ent->s.origin, MULTICAST_ALL);

				// record this, so we can save them to a file
				lpbuf[num_lpbuf] = malloc( LP_SIZE );

				memcpy( lpbuf[num_lpbuf], last_endpos, 12 );

				*((short *) (lpbuf[num_lpbuf]+12)) = (short) burn_size->value;
				*(lpbuf[num_lpbuf]+14) = (unsigned char) (255.0 * (float)burn_r->value);
				*(lpbuf[num_lpbuf]+15) = (unsigned char) (255.0 * (float)burn_g->value);
				*(lpbuf[num_lpbuf]+16) = (unsigned char) (255.0 * (float)burn_b->value);
				*(lpbuf[num_lpbuf]+17) = (unsigned char) ((127.0 * (float)burn_intensity->value) + 127.0);

				num_lpbuf++;
			}
		}
	}

	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;

	if (ent->kickdelay) {
		if (!--ent->kickdelay) {
			gi.bprintf(PRINT_HIGH,ent->kickmess,ent->client->pers.netname);
			gi.AddCommandString(va("kick %i\n", (int)(ent - g_edicts - 1)));
		}
		return;
	}

	if (ent->moveout_ent && ent->moveout_ent->health <= 0)
	{
		ent->moveout_ent = NULL;
	}

	if (level.intermissiontime)
		return;

	// RAFAEL
	if (level.cut_scene_time)
		return;

	if (level.pawn_time)
		return;

/*
	if (teamplay->value && !ent->client->pers.team && (ent->movetype == MOVETYPE_NOCLIP) && ((int)(level.time*10)%10 == 0))
	{
		gi.centerprintf( ent, "--------------------------------------------------------\n\nYou are a spectator!\n\nPress the corresponding number\nto join a team.\n\nValid teams are:\n\n%12s - 1\n%12s - 2\n\n--------------------------------------------------------\n", team_names[1], team_names[2] );
	}
*/

	client = ent->client;

	// Ridah, hack, make sure we duplicate the episode flags
	ent->episode_flags |= ent->client->pers.episode_flags;
	ent->client->pers.episode_flags |= ent->episode_flags;

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	Think_FlashLight (ent);

	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			// in deathmatch, only wait for attack button
			if (deathmatch->value)
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;

			if(  (client->latched_buttons & buttonMask) || (deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN))  ) 
			{
					if( level.fadeendtime <= level.time - (cutscenetime-1) ) //assault2 so dead losers stay dead until level change.
					{
						respawn(ent);
						client->latched_buttons = 0;
					}
			}
		}
		return;
	}

// BEGIN: Xatrix/Ridah/Navigator/16-apr-1998
	if (!deathmatch->value && !ent->nav_build_data && nav_dynamic->value)
	{
		// create the nav_build_data structure, so we can begin dropping nodes
		ent->nav_build_data = gi.TagMalloc(sizeof(nav_build_data_t), TAG_LEVEL);
		memset(ent->nav_build_data, 0, sizeof(ent->nav_build_data));

		ent->nav_build_data->jump_ent = G_Spawn();
		VectorCopy(ent->maxs, ent->nav_build_data->jump_ent->maxs );
		VectorCopy(ent->mins, ent->nav_build_data->jump_ent->mins );
	}
// END:		Xatrix/Ridah/Navigator/16-apr-1998

	// BEGIN:	Xatrix/Ridah/Navigator/23-mar-1998
	// show the debug path
	{
		extern int		showpath_on;
		extern edict_t	*showpath_ent;

		if (showpath_on)
		{
			NAV_Debug_DrawPath(ent, showpath_ent);
		}
	}
	// END:		Xatrix/Ridah/Navigator/23-mar-1998

	client->latched_buttons = 0;

	if (!(ent->flags & FL_JETPACK))
	{
		ent->client->jetpack_warned = false;

		if (ent->client->jetpack_power < 15.0)
			ent->client->jetpack_power += 0.05;
	}
	else
	{
		ent->client->jetpack_power -= 0.1;

		if (ent->client->jetpack_power <= 0.0)
		{	// disable the jetpack
			gitem_t	*jetpack;

			jetpack = FindItem("Jet Pack");
			jetpack->use( ent, jetpack );
		}
		else if (!ent->client->jetpack_warned && ent->client->jetpack_power < 5.0)
		{
			ent->client->jetpack_warned = true;
			cprintf( ent, PRINT_HIGH, "SOUND TODO: WARNING: Jet Pack power is LOW\n");
		}
	}

//gi.dprintf("%d: %d - %d %d %d\n",level.framenum,ent->client->resp.enterframe,ent->client->resp.checkdelta,ent->client->resp.checkpvs,ent->client->resp.checktime);

	if (level.framenum>ent->client->resp.checkdelta) {
		ent->client->resp.checkdelta=level.framenum+70;
		gi.WriteByte(13);
		gi.WriteString("cl_nodelta 0\n");
		gi.unicast(ent, true);
	} else if (level.framenum>ent->client->resp.checkpvs) {
		char buf[40];
		ent->client->resp.checkpvs=level.framenum+110;
		sprintf(buf,"%s $gl_lockpvs $r_drawworld\n",lockpvs);
		gi.WriteByte(13);
		gi.WriteString(buf);
		gi.unicast(ent, true);
	} else if (level.framenum>ent->client->resp.checktime) {
		char buf[32];
		ent->client->resp.checktime=level.framenum+130;
		sprintf(buf,"%s $timescale\n",scaletime);
		gi.WriteByte(13);
		gi.WriteString(buf);
		gi.unicast(ent, true);
	}
}

void cprintf(edict_t *ent, int printlevel, char *fmt, ...)
{
	char buf[TEXTBUFSIZE]="",*tb=ent->client->pers.textbuf;
	va_list vl;
	va_start(vl,fmt);
	vsprintf(buf,fmt,vl);
	va_end(vl);
	if ((strlen(tb)+strlen(buf))<TEXTBUFSIZE) strcat(tb,buf);
}
