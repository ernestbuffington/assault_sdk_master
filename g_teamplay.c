
// g_teamplay.c - teamplay oriented code

#include "g_local.h"
//#include "stdlog.h"	// Standard Logging
//#include "gslog.h"	// Standard Logging


// current teamplay mode (set by "level.style")
teamplay_mode_t	teamplay_mode;

#define	CASH_ROLL			10
#define	CASH_BAG			25

#define	MAX_CASH_ITEMS		10		// never spawn more than this many cash items at once
int		num_cash_items;

char *team_names[] = {
	"(spectator)",
	"Thugs",
	"Runts", //Assault2
	NULL
};

char *dragonsafe = "Dragons' Safe";
char *nikkisafe = "Nikki's Boyz's Safe";

int	team_cash[3];	// cash per team, 0 is neutral so just ignore

float	last_safe_withdrawal[3];
float	last_safe_deposit[3];

//=====================================================================
// Entity spawn functions

// ....................................................................
// Cash Spawning during GRAB DA LOOT

void cash_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float speed;

	if (surf && plane && plane->normal[2] > 0.5)
	{	// let it rest here

		if ((speed = VectorLength( self->velocity )) > 10)
		{
			self->s.angles[ROLL]	= 0;
			self->s.angles[PITCH]	= 0;
			self->avelocity[PITCH]	= 0;
			self->avelocity[ROLL]	= 0;
			self->avelocity[YAW]	*= 0.5;

			// randomize bounce
			VectorAdd( self->velocity, tv( crandom()*speed*0.3, crandom()*speed*0.3, random()*speed*0.15 ), self->velocity );
		}
		else
		{
			VectorClear( self->velocity );
			VectorClear( self->avelocity );
			self->s.angles[PITCH] = 0;
			self->s.angles[ROLL] = 0;

			self->movetype = MOVETYPE_NONE;
		}

		return;
	}

	if (other->client)
	{
		if (other->client->pers.currentcash < MAX_CASH_PLAYER)
		{	// they can hold the cash

			if ((self->currentcash == CASH_BAG) || (self->movetype != MOVETYPE_NONE) || (other->client->ps.pmove.pm_flags & PMF_DUCKED))
			{	// they can pick it up

				Touch_Item( self, other, plane, surf );

				num_cash_items--;

				G_FreeEdict( self );
				return;
			}

		}
	}
}

void cash_kill( edict_t *self )
{
	num_cash_items--;
	G_FreeEdict( self );
}

void cashroll_animate( edict_t *self )
{
	// reduce XY velocity (air friction)
	self->velocity[0] *= 0.9;
	self->velocity[1] *= 0.9;

	if (level.time > (self->timestamp))
	{
		cash_kill( self );
		return;
	}

	if (self->movetype != MOVETYPE_NONE)
	{

		if (VectorDistance( self->s.origin, self->pos1 ) < 1)
			self->count++;
		else
			self->count = 0;

		VectorCopy( self->s.origin, self->pos1 );

		if (self->count > 2)	// rested for 2 frames
		{
			VectorClear( self->velocity );
			VectorClear( self->avelocity );
			self->s.angles[PITCH] = 0;
			self->s.angles[ROLL] = 0;

			self->movetype = MOVETYPE_NONE;
		}
	}

	self->nextthink = level.time + 0.1;
}

void cashspawn_think( edict_t *self )
{
	edict_t	*cash;

	if (strstr(level.mapname, "assault")) return; // no cash spawning in assault 2

	if ((num_cash_items > MAX_CASH_ITEMS) || (level.modeset == MATCHSETUP) || (level.modeset == FINALCOUNT) || (level.modeset == FREEFORALL))
	{
		self->nextthink = level.time + self->delay;
		return;
	}

	// spawn some money
	cash = G_Spawn();

	VectorCopy( self->s.origin, cash->s.origin );
	cash->movetype = MOVETYPE_BOUNCE;
	cash->solid = SOLID_TRIGGER;

	AngleVectors( self->s.angles, cash->velocity, NULL, NULL );
	VectorScale( cash->velocity, self->speed, cash->velocity );

	// randomize the velocity a bit
	VectorAdd( cash->velocity, tv( crandom()*self->speed*0.3, crandom()*self->speed*0.3, crandom()*self->speed*0.15 ), cash->velocity );

	cash->s.renderfx2 |= RF2_NOSHADOW;

	// FIXME: this doesn't work, need to spawn actual item's, so the HUD is updated automatically when picking up

	if (!strcmp(self->type, "cashroll"))
	{	// small dollar notes
		cash->s.modelindex = gi.modelindex( "models/pu_icon/cash/tris.md2" );
		cash->gravity = 0.1 + random()*0.5;

		cash->think = cashroll_animate;
		cash->nextthink = level.time + 0.1;
		cash->s.angles[PITCH] = 10;
		VectorSet( cash->avelocity, 0, 10000 * cash->gravity, 0 );

		VectorSet( cash->mins, -4, -4, -15 );
		VectorSet( cash->maxs,  4,  4, -13 );

		cash->item = FindItem("Cash");

		cash->currentcash = CASH_ROLL;
		cash->touch = cash_touch;

		cash->timestamp = level.time + 60;

		cash->think = cashroll_animate;
		cash->nextthink = level.time + 0.1;
	}
	else
	{
		cash->s.modelindex = gi.modelindex( "models/pu_icon/money/money_sm.md2" );
		cash->gravity = 1.0;

		VectorSet( cash->mins, -12, -12, -15 );
		VectorSet( cash->maxs,  12,  12,  10 );

		cash->item = FindItem("Small Cash Bag");

		cash->currentcash = CASH_BAG;
		cash->touch = cash_touch;

		cash->think = cash_kill;
		cash->nextthink = level.time + 60;
	}

	num_cash_items++;

	self->nextthink = level.time + self->delay;

}

/*QUAKED dm_cashspawn (0.5 0 1) (-16 -16 -16) (16 16 16)
Spawn location for cash during "Grab da Loot" games

  angle - direction to project cash upon spawning
  speed - speed of projection
  type	- "cashroll" or "cashbag" (more money, longer delay)
*/
void SP_dm_cashspawn( edict_t *self )
{
	if (!teamplay->value || ((int)teamplay->value != TM_AUTO && (int)teamplay->value != TM_GRABDALOOT))
		return;

	// set the game to "Grab da Loot"
	teamplay_mode = TM_GRABDALOOT;
	num_cash_items = 0;

	if (!strcmp(self->type, "cashroll"))
	{
		self->delay = (float)g_cashspawndelay->value;
	}
	else	// bag, so longer delay
	{
		self->delay = (float)g_cashspawndelay->value * (CASH_BAG / CASH_ROLL);
	}

	if (!self->speed)
		self->speed = 10;

	self->think = cashspawn_think;
	self->nextthink = level.time + self->delay;
}

// ....................................................................
// Safe Bag, used for Grab Da Loot and teamplay Cash-Match 
extern qboolean Teamplay_ValidateJoinTeam( edict_t *self, int teamindex ); //assault2

void safebag_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
	else if (last_touch_time > (level.time - 0.1))
	{
		return;
	}
	else
	{
		last_touch_count++;
		last_touch_time = level.time;

		if (last_touch_count > (int)(50.0 * (1.0 + (0.5*(other->client->pers.team == self->style)))))
		{
			// let them go away on their own terms
			T_Damage( other, other, other, vec3_origin, other->s.origin, vec3_origin, 9999, 0, 0, MOD_SAFECAMPER );
			last_touch_count = 0;
		}
	}

	last_touch_ent = other;

	if (self->timestamp > (level.time - 1.0))
		return;

	self->timestamp = level.time;

	// depositing, or withdrawing?

	if (other->client->pers.team == self->style) //deposit
	{	// deposit
		////////////////////////////////////////////////////////////////////////////////////// Frank 4-15-00 PUREASSAULT

		//  Assault2 Do you have any keys?

		if (other->client->pers.inventory[ITEM_INDEX(FindItem("Key_One"))])
		{ // yes i have key1
				
				edict_t *teammates;
				int		j;

				if(other->deathtarget && assault_keytargets) // client->deathtarget gets set to key->target when the client picks up key in Pickup_Key();
				{
					self->target = G_CopyString( other->deathtarget ); // trigger Jaxon's func_walls.
					G_UseTargets (self, other);
					other->deathtarget = NULL;
				}

				if(other->option)  { AcceptableThugSpawnerStyle = other->option;    other->option = 0; }
				if(other->count)   { AcceptableRuntSpawnerStyle = other->count;     other->count = 0;  }

				other->client->pers.inventory[ITEM_INDEX(FindItem("Key_One"))] = 0; // take key1 away from inventory when you deposit it.
				other->client->pers.bagcash = 0; // remove the bag from his ass.
				team_cash[self->style] += 1; // increase the team score by one
				gi.sound(other, CHAN_AUTO, gi.soundindex("actors/male/hiredguy/fingers2.wav"), 1, ATTN_NORM, 0);
				other->client->resp.deposited += 1; // give credit to the player what did the deed.	

				for (j = 1; j <= game.maxclients; j++)
					{
						teammates = &g_edicts[j];
						if (!(teammates->inuse))
							continue;
						if (!(teammates->client))
							continue;
						gi.cprintf (teammates, PRINT_CHAT, ">> %s deposited Key_One!\n>> Two Keys Remain!\n", other->client->pers.netname ); // tell EVERYONE
						if(strcmp( other->client->pers.netname , teammates->client->pers.netname  ) ) // tell everyone except yourself.
						gi.sound(teammates, CHAN_ITEM, gi.soundindex("world/pickups/keys.wav"), 1, 3, 0);
						//if (teammates->client->pers.team != self->style)
						//	continue;
					}	
				return;
		}

		else if ( (other->client->pers.inventory[ITEM_INDEX(FindItem("Key_Two"))]) )
		{ // yes i have key2

				edict_t *teammates;
				int		j;

				if(other->deathtarget && assault_keytargets) // client->deathtarget gets set to key->target when the client picks up key in Pickup_Key();
				{
					self->target = G_CopyString( other->deathtarget ); // trigger Jaxon's func_walls.
					G_UseTargets (self, other);
					other->deathtarget = NULL;
				}

				if(other->option)  { AcceptableThugSpawnerStyle = other->option;    other->option = 0; }
				if(other->count)   { AcceptableRuntSpawnerStyle = other->count;     other->count = 0;  }

				other->client->pers.inventory[ITEM_INDEX(FindItem("Key_Two"))] = 0; // take key2 away from inventory when you deposit it.
				other->client->pers.bagcash = 0; // remove the bag from his ass.
				gi.sound(other, CHAN_AUTO, gi.soundindex("actors/male/hiredguy/fingers2.wav"), 1, ATTN_NORM, 0);
				team_cash[self->style] += 1; // increase the team score by one
				other->client->resp.deposited += 1; // give credit to the player what did the deed.

				for (j = 1; j <= game.maxclients; j++)
					{
						teammates = &g_edicts[j];
						if (!(teammates->inuse))
							continue;
						if (!(teammates->client))
							continue;
						gi.cprintf (teammates, PRINT_CHAT, ">> %s deposited Key_Two!\n>> One Key Remains!\n", other->client->pers.netname ); // tell EVERYONE
						if(strcmp( other->client->pers.netname , teammates->client->pers.netname  ) ) // tell everyone except yourself.
						gi.sound(teammates, CHAN_ITEM, gi.soundindex("world/pickups/keys.wav"), 1, 3, 0);
					}
				return;
		}

		else if ( (other->client->pers.inventory[ITEM_INDEX(FindItem("Key_Three"))]) )
		{

				int		j;
				int		loser=0;

				if(self->style == 1) loser = 2;
				else if(self->style == 2) loser = 1;
				else loser = 2;

				if(other->deathtarget && assault_keytargets) // client->deathtarget gets set to key->target when the client picks up key in Pickup_Key();
				{
					self->target = G_CopyString( other->deathtarget ); // trigger Jaxon's func_walls.
					G_UseTargets (self, other);
					other->deathtarget = NULL;
				}

				else 
				{

				self->target = G_CopyString( "end_game" ); // trigger Hogie's cut scene at the end of the level.
				G_UseTargets (self, other); // using other allows the depositor of key 3 to get the frags
				}


				level.fadeendtime = level.time; //timestamp the level so it will end after the end_game
				level.total_secrets = 69; // redundent check for a key three time stamp

				other->client->pers.inventory[ITEM_INDEX(FindItem("Key_Three"))] = 0; // take key3 away from inventory when you deposit it.
				other->client->pers.bagcash = 0; // remove the bag from his ass.
				gi.sound(other, CHAN_AUTO, gi.soundindex("actors/male/hiredguy/fingers2.wav"), 1, ATTN_NORM, 0);
				other->client->resp.deposited += 1; // give credit to the player what did the deed.
				team_cash[self->style] += 1; // increase the team score by one

				for (j = 1; j <= game.maxclients; j++)  //loop through all players
					{
						edict_t *teammates;
						int i;

						teammates = &g_edicts[j];

						if( (teammates->client->pers.health < 0) || (teammates->deadflag)  ) goto SKIPFILTERS;
						//spawn spectators to the loosing team
						if (teammates->client->pers.spectator == SPECTATING) {
							teammates->client->pers.spectator = PLAYING;
							Teamplay_ValidateJoinTeam( teammates, loser ); }

						if (!(teammates->inuse)) continue;

						if (!(teammates->client)) continue;

						//teammates->client->pers.weapon = NULL;
SKIPFILTERS:

						gi.cprintf(teammates, PRINT_CHAT, ">> %s deposited Key_Three!\n", other->client->pers.netname);
						if(strcmp( other->client->pers.netname , teammates->client->pers.netname  ) ) // tell everyone except yourself.
							gi.sound(teammates, CHAN_ITEM, gi.soundindex("world/pickups/keys.wav"), 1, 3, 0);

						if (teammates->client->pers.team != self->style
							&& teammates->client->pers.spectator == PLAYING // virgin21
							) // These guys are the losers.
						{
							teammates->client->pers.game_helpchanged = LOSER; // Set this value so we can spawn them to a style 3 spawner.
							RESPAWN_ASSAULT(teammates)
						}
						else //These guys are on the winning team.
						{	
							edict_t *ent;
							teammates->client->pers.game_helpchanged = WINNER; // Set this value so we can freeze the winning players in spec
							ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
							if (!ent)
								{	// the map creator forgot to put in an intermission point...
									ent = G_Find (NULL, FOFS(classname), "info_player_start");
									if (!ent)
										ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
								}
							else
								{	// chose one of four spots
									i = rand() & 3;
									while (i--)
									{
										ent = G_Find (ent, FOFS(classname),	"info_player_intermission");
										if (!ent)	// wrap around the list
											ent = G_Find (ent, FOFS(classname), "info_player_intermission");
									}
								}
							VectorCopy (ent->s.origin, level.intermission_origin);
							VectorCopy (ent->s.angles, level.intermission_angle);
							MoveClientToIntermission (teammates);
						} // CLOSING THESE GUYS ARE ON THE WINNING TEAM

						teammates->client->pers.weapon = NULL;
						for (i = 0; i < game.num_items; i++) // remove all weapons from both sides
							teammates->client->pers.inventory[i] = 0;

						teammates->client->showscores = false;
						teammates->client->showhelp = false;
						teammates->client->showinventory = false;
					}// ENDING for (j = 1; j <= game.maxclients; j++)
			return;
		} // END if ( (other->client->pers.inventory[ITEM_INDEX(FindItem("Key_Three"))]) )

//////////////////////////////////////////////////////////////////END FRANK PUREASSAULT /////////////////////////////////////		
		if (other->client->pers.currentcash > 0 || other->client->pers.bagcash > 0)
		{
			int	precash, amount;

			precash = team_cash[self->style];

			team_cash[self->style] += other->client->pers.currentcash;
			team_cash[self->style] += other->client->pers.bagcash;
			UPDATESCORE

			other->client->resp.deposited += other->client->pers.currentcash;
			other->client->resp.deposited += other->client->pers.bagcash;

			other->client->pers.currentcash = 0;
			other->client->pers.bagcash = 0;

			gi.sound(other, CHAN_ITEM, gi.soundindex("world/pickups/cash.wav"), 1, 3, 0);

			// make a droping sound
			gi.positioned_sound(self->s.origin, self, CHAN_ITEM, gi.soundindex("world/doors/dr1_end.wav"), 1, 1, 0);

			// let everyone know how much was deposited
			amount = team_cash[self->style] - precash;
			gi.bprintf( PRINT_MEDIUM, "%s deposited $%i\n", other->client->pers.netname, amount );

			last_safe_deposit[self->style] = level.time;
		}

	}
/*  // no withdrawls allowed in Assault2
	else if (team_cash[self->style] > 0)
	{	// withdrawal

		if (other->client->pers.bagcash < MAX_BAGCASH_PLAYER)
		{
			int	precash, amount;

			precash = team_cash[self->style];

			team_cash[self->style] -= (MAX_BAGCASH_PLAYER - other->client->pers.bagcash);
			other->client->pers.bagcash += (MAX_BAGCASH_PLAYER - other->client->pers.bagcash);

			if (team_cash[self->style] < 0)
			{	// don't take more than they have
				other->client->pers.bagcash += team_cash[self->style];
				team_cash[self->style] = 0;
			}
			UPDATESCORE

			gi.sound(other, CHAN_ITEM, gi.soundindex("world/pickups/cash.wav"), 1, 3, 0);

			// alarm!
			gi.positioned_sound(self->s.origin, self, CHAN_ITEM, gi.soundindex("misc/cashmatch_alarm.wav"), 1, 1, 0);

			// let everyone know how much was stolen
			amount = precash - team_cash[self->style];
			gi.bprintf( PRINT_MEDIUM, "%s stole $%i from %s's safe!\n", other->client->pers.netname, amount, team_names[self->style] );

			last_safe_withdrawal[self->style] = level.time;
		}

	}
*/
}

// Safebag think, if a friendly guy has been standing near us for more than a few seconds, make them vulnerable to friendly fire
void safebag_think(edict_t *self)
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

		if (!trav->inuse || !trav->client)
			continue;

		if (trav->health <= 0)
			continue;

		if (!trav->client->pers.team || (trav->client->pers.team == self->style))
			continue;

		if (VectorDistance( self->s.origin, trav->s.origin ) > 512)
			continue;

		if (!gi.inPVS( self->s.origin, trav->s.origin ))
			continue;

		noenemies = false;
	}

	for (i=0; i<maxclients->value; i++)
	{
		trav = &g_edicts[i+1];

		if (!trav->inuse || !trav->client)
			continue;

		if (trav->health <= 0)
			continue;

		if (!trav->client->pers.team || (trav->client->pers.team != self->style))
			continue;

		if (noenemies)
		{

			if (VectorDistance( self->s.origin, trav->s.origin ) > SAFE_CLOSE_DIST)
			{
				trav->client->pers.timeatsafe -= 0.2;

				if (trav->client->pers.timeatsafe < 0)
					trav->client->pers.timeatsafe = 0;
			}
			else
			{
				trav->client->pers.timeatsafe += 0.2;

				if (trav->client->pers.timeatsafe > MAX_TIMEATSAFE)
					trav->client->pers.timeatsafe = MAX_TIMEATSAFE;
			}

			if (trav->client->pers.timeatsafe >= MAX_TIMEATSAFE)
				trav->client->pers.friendly_vulnerable = true;
			else
				trav->client->pers.friendly_vulnerable = false;

		}
		else	// turn off vulnerability, there is an enemy in range
		{
			trav->client->pers.friendly_vulnerable = false;
		}
	}

	self->nextthink = level.time + 0.2;
}

/*QUAKED dm_safebag (0.5 0 1) (-12 -12 -16) (12 12 12)
Bag that holds the money in the safe.

  style - team that this bag belongs to (1 or 2)
*/
void SP_dm_safebag( edict_t *self )
{
	if (!teamplay->value)
	{
		G_FreeEdict( self );
		return;
	}

/*	if (self->style == 1)
		strcpy (self->classname, dragonsafe);
	else if (self->style == 2)
		strcpy (self->classname, nikkisafe);
	else*/
	if (self->style < 1 || self->style > 2)
	{
		gi.dprintf( "dm_safebag has invalid \"style\" at %s, should be 1 or 2.\n", vtos(self->s.origin));
		G_FreeEdict( self );
		return;
	}

//	self->s.modelindex = gi.modelindex("models/pu_icon/money/money_lg.md2");//assault2
	if(self->type)
	self->s.modelindex = gi.modelindex(self->type);//assault2

	VectorSet( self->mins, -12, -12, -16 );
	VectorSet( self->maxs,  12,  12,  12 );

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_TRIGGER;

	gi.linkentity( self );

	self->touch = safebag_touch;
	self->currentcash = 0;	// start with no cash

	self->think = safebag_think;
	self->nextthink = level.time + 2;
}


/*QUAKED dm_props_banner (.5 0 1) (-4 -4 -4) (4 4 4)
Temp banner for teamplay

 style = team (1 / 2)
 scale = scale the size up/down (2 = double size)

model="models\props\temp\triangle\small.md2"
*/
void SP_dm_props_banner (edict_t *self)
{
//	vec3_t	end, bestnorm, bestend;
//	float bestdist;
//	int	x,y;
//	trace_t tr;

	if (!deathmatch->value || !teamplay->value)
	{	// remove
		G_FreeEdict (self);
		return;
	}
	
	if (!self->style)
	{
		gi.dprintf( "%s has invalid style (should be 1 or 2) at %s\n", self->classname, vtos(self->s.origin) );
		G_FreeEdict (self);
		return;
	}
/*
	// trace a line back, to get the wall, then go out
	{
		bestdist = 9999;

		for (x=-256; x<300; x+= 256)
		{
			VectorCopy( self->s.origin, end );
			end[0] = self->s.origin[0] + x;
			tr = gi.trace( self->s.origin, NULL, NULL, end, NULL, MASK_SOLID );
			if (tr.fraction < bestdist)
			{
				VectorCopy( tr.plane.normal, bestnorm );
				VectorCopy( tr.endpos, bestend );
				bestdist = tr.fraction;
			}
		}
		for (y=-256; y<300; y+= 256)
		{
			VectorCopy( self->s.origin, end );
			end[1] = self->s.origin[1] + y;
			tr = gi.trace( self->s.origin, NULL, NULL, end, NULL, MASK_SOLID );
			if (tr.fraction < bestdist)
			{
				VectorCopy( tr.plane.normal, bestnorm );
				VectorCopy( tr.endpos, bestend );
				bestdist = tr.fraction;
			}
		}

		vectoangles( bestnorm, self->s.angles );

		VectorMA( bestend, 40 * self->cast_info.scale, bestnorm, self->s.origin );
	}

*/
// Ridah, 1-jun-99, use flag models for now
#if 1
	{
		void think_flag (edict_t *self);

//		self->solid = SOLID_BBOX;
		self->movetype = MOVETYPE_NONE;

		if (self->style == 2)
		{
			self->model = "models/props/flag/flag1.md2";
		}
		else
		{
			self->model = "models/props/flag/flag3.md2";
		}

		self->s.modelindex = gi.modelindex (self->model);

		self->s.renderfx2 |= RF2_NOSHADOW;
		self->s.renderfx |= RF_MINLIGHT;

		if (!self->cast_info.scale)
			self->cast_info.scale = 1;

		self->s.scale = (self->cast_info.scale - 1);

//		VectorMA( bestend, 40 * self->cast_info.scale, bestnorm, self->s.origin );

		self->cast_info.scale *= 0.3;

		gi.linkentity (self);

		self->s.effects |= EF_ANIM_ALLFAST_NEW;
		self->s.renderfx2 |= RF2_MODULUS_FRAME;
		self->s.renderfx2 |= RDF_NOLERP;

// Disabled, doesn't animate much, and uses bandwidth
//		self->nextthink = level.time + FRAMETIME *2;
//		self->think = think_flag;
	}

#else // TRIANGULAR ROTATING ICONS

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;

	self->s.skinnum = self->style - 1;

	self->s.renderfx2 |= RF2_NOSHADOW;	
	self->s.renderfx |= RF_MINLIGHT;

	if (!self->cast_info.scale)
		self->cast_info.scale = 1;

	self->s.scale = self->cast_info.scale - 1;

	self->s.modelindex = gi.modelindex ("models/props/temp/triangle/small.md2");

	gi.linkentity (self);

	{
		edict_t *arm;

		arm = G_Spawn();
		arm->solid = self->solid;
		arm->movetype = self->movetype;
		arm->s.renderfx2 |= RF2_NOSHADOW;
		arm->s.scale = self->s.scale;

		VectorCopy( self->s.origin, arm->s.origin );
		VectorCopy( self->s.angles, arm->s.angles );

		arm->s.modelindex = gi.modelindex ("models/props/temp/triangle/arm.md2");
		gi.linkentity (arm);
	}

	VectorCopy( self->s.angles, self->last_step_pos );
	VectorClear( self->move_angles );
#endif
}

// ....................................................................

void Teamplay_ValidateSkin( edict_t *self )
{
	// TODO: we need color coded skins, for now, just use any skin
}

extern void ClientUserinfoChanged (edict_t *ent, char *userinfo);

// Papa - made some changes to this function

qboolean Teamplay_ValidateJoinTeam( edict_t *self, int teamindex )
{
	// NOTE: this is called by each player on level change, as well as when a player issues a "join XXX" command

	// TODO: player limit per team? cvar?


	// setup client stuff
	Teamplay_ValidateSkin( self );

//	InitClientPersistant (self->client);

	self->client->pers.team = teamindex;
	self->client->pers.spectator = PLAYING;
	if ((level.modeset != STARTINGMATCH) && (level.modeset != STARTINGPUB))
	{
	gi.bprintf( PRINT_HIGH, "%s joined %s\n", self->client->pers.netname, team_names[teamindex] );
	// Frank
//	gi.cprintf (self, PRINT_CHAT, ">> %s joined the %s.\n>> %s = male%s.\n", self->client->pers.netname, team_names[teamindex], self->client->pers.netname, strrchr( self->client->pers.userinfo, '_' ));
	}

	if ((level.modeset == TEAMPLAY) || (level.modeset == MATCH) || (level.modeset == STARTINGMATCH) || (level.modeset == STARTINGPUB))
	{
		self->movetype = MOVETYPE_WALK;
		self->solid = SOLID_BBOX;
		self->svflags &= ~SVF_NOCLIENT;


		self->health = 0;	// so we're not counted in spawn point checking
		self->client->resp.enterframe = level.framenum;
	//	InitClientResp( self->client );
		self->client->resp.score = 0;
		self->client->resp.deposited = 0;
		PutClientInServer( self );	// find a new spawn point
	}
	return true;
}

void Teamplay_AutoJoinTeam( edict_t *self )
{
	int	team_count[2];
	int	i;

	// count number of players on each team, assign the team with least players

	team_count[0] = 0;
	team_count[1] = 0;

	for (i=1; i<maxclients->value; i++)
	{
		if (g_edicts[i].client && g_edicts[i].client->pers.team)
			team_count[g_edicts[i].client->pers.team - 1]++;
	}

	if (team_count[0] > team_count[1])
		self->client->pers.team = 2;
	else
		self->client->pers.team = 1;
	self->client->pers.spectator = PLAYING;

	Teamplay_ValidateJoinTeam( self, self->client->pers.team );
}


void Teamplay_InitTeamplay (void)
{
	num_cash_items = 0;
	memset( team_cash, 0, sizeof(int) * 3 );
	memset( last_safe_withdrawal, 0, sizeof(float) * 3 );
	memset( last_safe_deposit, 0, sizeof(float) * 3 );

	last_safe_deposit[0] = last_safe_deposit[1] = 0;
	last_safe_withdrawal[0] = last_safe_withdrawal[1] = 0;
	if (teamplay->value) UPDATESCORE
}
/*QUAKED props_radio (0 .5 .8) (-8 -12 -8) (8 12 8)

A radio that will take damage and activate triggers on destruction

"health" - hit points (25 default)

model="models\props\radio\tris.md2"
*/

void radio_explode (edict_t *self)
{
	vec3_t	org;
	float	spd;
	vec3_t	save;

	VectorCopy (self->s.origin, save);
	VectorMA (self->absmin, 0.5, self->size, self->s.origin);

	// a few big chunks
	spd = 1.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/props/metal/metal1.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/props/metal/metal1.md2", spd, org);

	// bottom corners
	spd = 1.75 * (float)self->dmg / 200.0;
	VectorCopy (self->absmin, org);
	ThrowDebris (self, "models/props/metal/metal2.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	ThrowDebris (self, "models/props/metal/metal3.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[1] += self->size[1];
	ThrowDebris (self, "models/props/metal/metal4.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	org[1] += self->size[1];
	ThrowDebris (self, "models/props/metal/metal5.md2", spd, org);

	// a bunch of little chunks
	spd = 2 * self->dmg / 200;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/props/metal/metal1.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/props/metal/metal1.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/props/metal/metal1.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/props/metal/metal1.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/props/metal/metal1.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/props/metal/metal1.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/props/metal/metal1.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/props/metal/metal1.md2", spd, org);

	VectorCopy (save, self->s.origin);

	G_FreeEdict (self);
}

void radio_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point, int mdx_part, int mdx_subobject)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + FRAMETIME;
	self->think = radio_explode;
	self->activator = attacker;

	G_UseTargets (self, inflictor);
	
	{
		vec3_t realorigin;

		VectorCopy(self->s.origin, realorigin);
		realorigin[2] += 8;

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SPLASH);
		gi.WriteByte (25);
		gi.WritePosition (realorigin);
		gi.WriteDir (self->movedir);
		gi.WriteByte (1);
		gi.multicast (realorigin, MULTICAST_PVS);
	}
}

void SP_props_radio (edict_t *self)
{

	self->solid = SOLID_TRIGGER; //assault 2
//	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_PROP;
	
	if(self->type)	self->model = self->type;
	else self->model = "models/props/radio/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
//	VectorSet (self->mins, -8, -12, -8);
//	VectorSet (self->maxs, 8, 12, 8);		
	VectorSet( self->mins, -12, -12, -16 );// assault 2
	VectorSet( self->maxs,  12,  12,  12 );

	if (!(self->health == 666))
	{
		self->die = radio_die;
		self->takedamage = DAMAGE_YES;

		if (!self->health)
			self->health = 25;
	}

	self->surfacetype = SURF_METAL_L;	
	gi.linkentity (self);
	self->touch = safebag_touch; //assault 2
	self->currentcash = 0;	// start with no cash

	self->think = safebag_think;
	self->nextthink = level.time + 2;
}