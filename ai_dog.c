// dog
#include "g_local.h"
#include "ai_dog.h"

#define GOHOMEFAST { self->enemy = NULL; \
			if(self->goal_ent != self->start_ent) self->last_goal = self->goalentity = self->goal_ent = self->start_ent; \
			self->cast_info.aiflags |= AI_FASTWALK; \
			self->cast_info.currentmove = self->cast_info.move_avoid_walk; }

#define GOHOMESLOW { self->enemy = NULL; \
			if(self->goal_ent != self->start_ent) self->last_goal = self->goalentity = self->goal_ent = self->start_ent; \
			self->cast_info.aiflags &= ~AI_FASTWALK; \
			self->cast_info.currentmove = self->cast_info.move_avoid_walk; }

#define DOGMAKEENEMY { self->goal_ent = self->enemy = trav;	\
				AI_MakeEnemy ( self, trav, 0 );	newenemy = true; }
//	trav->count = self->count; } // virgin11 crashes server

void dog_sound (edict_t *self);
void dog_end_stand (edict_t *self);
void dog_sound_think (edict_t *self);
void dog_melee (edict_t *self);
void dog_evade_amb (edict_t *self);
void dog_melee_bail (edict_t *self);
void dog_evade_checkadjust (edict_t *self);
void dog_evade_adjust (edict_t *self);
void dog_lick2( edict_t *self );
void dog_pounce (edict_t *self);
void dog_talk_think ( edict_t *self );
void dog_bite (edict_t *self);
void dog_bark (edict_t *self);
void dog_growl (edict_t *self);

#include "ai_dog_tables.h"

char *wuf[7] =
{
"actors/dog/dg_bark1.wav",
"actors/dog/dg_bark2.wav",
"actors/dog/dg_die.wav",
"actors/dog/dg_grwl1.wav",
"actors/dog/dg_grwl2.wav",
"actors/dog/dg_lunge.wav",
"actors/dog/dg_yelp.wav"
};

void dog_yelp (edict_t *self)
{
	self->last_talk_time = level.time;
	gi.sound( self, CHAN_VOICE, gi.soundindex( wuf[6] ), 1, ATTN_NORM, 0 );
}

void dog_bark (edict_t *self)
{
	self->last_talk_time = level.time;
	gi.sound( self, CHAN_VOICE, gi.soundindex( wuf[rand()%2] ), 1, ATTN_NORM, 0 );
}

void dog_growl (edict_t *self)
{
	self->last_talk_time = level.time + 0.5;	// growl lasts a bit longer
	gi.sound( self, CHAN_BODY, gi.soundindex( wuf[3+rand()%3] ), 1, ATTN_NORM, 0 );
}

void dog_talk_think ( edict_t *self )
{
	edict_t	*talk_ent;
	cast_memory_t	*mem;

	if(rand()%2 && self->health < self->max_health) self->health++;

	if( self->timestamp < (level.time - 2) ) 
	{
		self->timestamp = level.time;
		DogCheckForEnemies(self);
		return;
	}

	if (!(talk_ent = self->cast_info.talk_ent))
		return;

	if (VectorDistance( talk_ent->s.origin, self->s.origin ) > 600)
	{
		self->cast_info.talk_ent = NULL;
		return;
	}

	if (	(self->cast_info.talk_ent == &g_edicts[1])
		&&	(self->cast_info.talk_ent->last_talk_time < (level.time - TALK_OTHER_DELAY*2))
		&&	(self->last_talk_time > (level.time - TALK_SELF_DELAY)))
	{
		return;
	}

//	if (last_client_talk && last_client_talk > (level.time - TALK_OTHER_DELAY))
//		return;	// don't talk too much around the client

	if ((talk_ent->health <= 0) || !visible(self, talk_ent) || !infront(talk_ent, self))
	{
		self->cast_info.talk_ent = NULL;
		return;
	}

	mem = level.global_cast_memory[self->character_index][talk_ent->character_index];
	if (!mem || (mem->flags & MEMORY_NO_TALK))
		return;

	// say something!
	if (	(	(self->last_talk_time < (level.time - TALK_SELF_DELAY))			// we haven't spoken for a while
			 ||	(	(talk_ent->client || talk_ent->last_talk_time)		// if they haven't spoken yet, don't bother
				 &&	(talk_ent->last_talk_time > (level.time - TALK_OTHER_DELAY*1.5))	// or they've just said something, and we've allowed some time for them to finish saying it
				 &&	(talk_ent->cast_info.talk_ent == self)
				 &&	(self->last_talk_time < talk_ent->last_talk_time)
				 &&	(self->last_talk_time < (level.time - TALK_OTHER_DELAY))))
		&&	(talk_ent->last_talk_time < (level.time - TALK_OTHER_DELAY)))
	{
		if (talk_ent->client)
		{
			// should we be aggressive? or friendly?

			if ((self->moral < MORAL_AGGRESSIVE) || (!self->profanity_level && talk_ent->profanity_level && (self->moral < rand()%MORAL_MAX)))
			{		// friendly
				self->cast_info.currentmove = &dog_move_sniff;
				dog_bark( self );
			}
			else if (self->profanity_level)
			{	// attack!
				AI_MakeEnemy( self, talk_ent, 0 );
				dog_bark( self );
			}
			else	// ready to attack at any second
			{
				self->cast_info.currentmove = &dog_move_sniff;
				dog_growl (self);
				self->profanity_level = 3;
			}

			self->last_talk_time = level.time;

			if (!infront( self, talk_ent ))
			{
				self->cast_info.avoid( self, talk_ent, true );
			}
		}
	}
}

void dog_end_stand( edict_t *self )
{
	if (self->cast_info.move_stand_evade && (self->last_stand_evade > (level.time - 3)))
		return;

	switch(rand()%20)
	{
	case 0:		self->cast_info.currentmove = &dog_move_pant;					break;
	case 1:		self->cast_info.currentmove = &dog_move_sniff;					break;
	case 2:		self->cast_info.currentmove = &dog_move_bark; dog_bark (self);	break;
	case 3:		self->cast_info.currentmove = &dog_move_pant;					break;
	case 4:		self->cast_info.currentmove = &dog_move_growl;					break;
	case 5:		self->cast_info.currentmove = &enemy_dog_move_pee;				break;
	default:	self->cast_info.currentmove = self->cast_info.move_stand;		break;
	}
}

void dog_backoff( edict_t *self, edict_t *other )
{

}

void dog_talk( edict_t *self )
{
	// executes at the end of every  AI_EndAttack()
}

void dog_pain (edict_t *self, edict_t *other, float kick, int damage, int mdx_part, int mdx_subobject)
{
	int rnd;
	int baseskin;//, currentskin;

	baseskin = self->s.model_parts[mdx_part].baseskin;
						
	if (self->health < (self->max_health * 0.5))
	{
		self->s.model_parts[PART_HEAD].skinnum[0] =  baseskin + 2;
	}
	else if (self->health < (self->max_health * 0.75))
	{
		self->s.model_parts[PART_HEAD].skinnum[0] =  baseskin + 1;
	}

	AI_CheckMakeEnemy( self, other );

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3 + random();

	if (skill->value >= 3)
		return;		// no pain anims in nightmare


	if (rand()%2)
		dog_yelp (self);


	// Ridah, randomly don't play an animation, since it' leaves them WAY open to be killed
	if (skill->value > 0 && rand()%2)
		return;

	rnd = rand()%3;

	switch (rnd)
	{
	case 0:
		self->cast_info.currentmove = &dog_move_pain1;
		break;
	case 1:
		self->cast_info.currentmove = &dog_move_pain2;
		break;
	case 2:
		self->cast_info.currentmove = &dog_move_pain3;
		break;
	}

}

void dog_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point, int mdx_part, int mdx_subobject)
{
	int rnd;

//	self->s.modelindex2 = 0;

	// regular death
	self->takedamage = DAMAGE_YES;

	if (DeathByGib(self, inflictor, attacker, damage))
	{	// gib
		self->deadflag = DEAD_DEAD;
		GibEntity( self, inflictor, damage );
		self->s.model_parts[PART_GUN].invisible_objects = (1<<0 | 1<<1);
		dog_respawn(self); // snatch replace dead dog.
		AI_UnloadCastMemory( self );
		G_FreeEdict (self);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	self->deadflag = DEAD_DEAD;

//	if (!self->onfiretime)
		dog_yelp (self);

	rnd = rand()%2;

	switch (rnd)
	{
	case 0:
		self->cast_info.currentmove = &dog_move_death1;
		break;
	case 1:
		self->cast_info.currentmove = &dog_move_death2;
		break;
	}

}

qboolean dog_attack (edict_t *self)
{
	vec3_t	vec;
	float dist;

	self->cast_info.aiflags |= AI_RUN_LIKE_HELL;	// dog runs really fast

	VectorSubtract( self->enemy->s.origin, self->s.origin, vec );
	dist = VectorNormalize( vec );

	// start panting now when we stop
	self->cast_info.move_stand = &dog_move_pant;

	// yell at them?
	if (self->last_talk_time < (level.time - 0.5*TALK_FIGHTING_DELAY))
	{
		if (rand()%2)
			dog_bark(self);
		else
			dog_growl(self);
	}

	{
		qboolean attack=false;

		if (dist < 48 && VectorLength( self->enemy->velocity ) < 250)
		{
			attack = true;
		}
		else if (dist < 64)
		{
			attack = false;	// so we do a high attack
		}
		else	// are they running towards us?
		{
			VectorSubtract( self->enemy->s.origin, self->s.origin, vec );
			VectorMA( vec, 0.5, self->enemy->velocity, vec );
			dist = VectorNormalize( vec );

			if (dist < 96)
			{
				attack = true;
			}
			
		}

		if (attack)
		{
			int rnd;

			self->ideal_yaw = vectoyaw(vec);

			M_ChangeYaw( self );

			if (vec[2] < -0.5)
			{
				self->cast_info.currentmove = &dog_move_low_atk;
			}
			else if (vec[2] < -0.1)
			{
				self->cast_info.currentmove = &dog_move_med_atk;
			}
			else if (vec[2] > 0.5)
			{
				self->cast_info.currentmove = &dog_move_upr_atk;
			}
			else if (self->enemy->maxs[2] < self->enemy->cast_info.standing_max_z)
			{
				self->cast_info.currentmove = &dog_move_med_atk;
			}
/*
			if (self->enemy->maxs[2] < 30)
			{
				if (rand()%10 < 8)
					self->cast_info.currentmove = &dog_move_med_atk;
				else
					self->cast_info.currentmove = &dog_move_low_atk;
			}
			else if (self->maxs[2] > self->enemy->maxs[2] || self->s.origin[2] > self->enemy->s.origin[2])
			{
				self->cast_info.currentmove = &dog_move_low_atk;
			}
			else if (self->s.origin[2] < self->enemy->s.origin[2])
			{
				self->cast_info.currentmove = &dog_move_upr_atk;
			}
*/
			else
			{
				rnd = rand()%10;

				if (dist < 48 && rnd < 4)
					self->cast_info.currentmove = &dog_move_low_atk;
				else if (rnd < 6)
					self->cast_info.currentmove = &dog_move_med_atk;
				else
					self->cast_info.currentmove = &dog_move_upr_atk;
			}	

			self->cast_info.aiflags &=~ AI_RUSH_THE_PLAYER;
			return true;
		}
		else if (dist < 180)	// Ridah, increased this to help SR4 dogs jump onto cars
		{
			self->ideal_yaw = vectoyaw(vec);
			M_ChangeYaw( self );

			self->cast_info.currentmove = &dog_move_upr_atk;
			return true;
		}
		else if (dist < 400)
			self->cast_info.aiflags |= AI_RUSH_THE_PLAYER;

	}

	return false;
}


void dog_bite (edict_t *self)
{
	vec3_t	start;//, offset;
	vec3_t	forward, right;

	float damage = 20;

	if(self->dmg) damage = self->dmg;

	if (self->cast_info.currentmove == &dog_move_upr_atk)
		damage *= 2;		// double handed attack

	// yell at them?
	if (self->last_talk_time < (level.time - 1.0))
	{
		if (rand()%2)
			dog_bark(self);
		else
			dog_growl(self);
	}

	// VectorSet(offset, 0, 8,  self->viewheight - 4);
	//VectorSet (offset, 0, 8,  16);

	AngleVectors (self->s.angles, forward, right, NULL);

	//G_ProjectSource (self->s.origin, offset, forward, right, start);
	
	/*
	if (self->cast_info.currentmove == &dog_move_low_atk)
		start[2] -= 20;
	else if (self->cast_info.currentmove == &dog_move_med_atk)
		start[2] -= 8;
	else if (self->cast_info.currentmove == &dog_move_upr_atk)
		start[2] += 8;
	*/
	
//	damage *= 0.5;	

	if(self->style && self->enemy && self->enemy->client 
		&& self->enemy->client->pers.team
		&& self->enemy->client->pers.team == self->style 
		&& self->cast_group != 0) GOHOMEFAST

	// fire_dogbite (self, start, forward, damage, 10, MOD_DOGBITE);
	if (self->enemy)
	{
//		trace_t		tr;
		vec3_t		aimdir, dang, end;

		VectorSubtract (self->enemy->s.origin, self->s.origin, aimdir);
		vectoangles (aimdir, dang);
		AngleVectors (dang, forward, NULL, NULL);
		VectorMA (self->s.origin, 16, forward, start);
		VectorMA (start, 32, forward, end);
		
		// ok it seems to line up with the head ok now
		// NAV_DrawLine (start, end);
		fire_blackjack( self, start, forward, damage, 0, MOD_DOGBITE );
		/*
		
		if (VectorLength (aimdir) < 96)
		{
			tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT );

			if (tr.ent->takedamage)
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, 0, DAMAGE_BULLET, MOD_DOGBITE); 
		}
		// ok lets see why this isnt working
		else if (self->enemy->groundentity == self)
			T_Damage (self->enemy, self, self, aimdir, self->enemy->s.origin, vec3_origin, damage, 0, DAMAGE_BULLET, MOD_DOGBITE); 
		*/
	}
}

void dog_long_attack (edict_t *self)
{
}


void dog_pounce (edict_t *self)
{
	if (!self->enemy)
		return;

	self->ideal_yaw = entyaw( self, self->enemy );
	M_ChangeYaw( self );

	// leap if on ground
	if (self->groundentity)
	{
		AngleVectors( self->s.angles, self->velocity, NULL, NULL );
		VectorScale( self->velocity, 500, self->velocity );
		self->velocity[2] = 200;
		self->groundentity = NULL;
	}

}


void dog_avoid ( edict_t *self, edict_t *other, qboolean face )
{
	vec3_t	vec;

	if (self->health <= 0)
		return;

	if (!self->groundentity)
		return;

	self->cast_info.last_avoid = level.time;

	if (face)
	{	// turn to face them
		VectorSubtract( other->s.origin, self->s.origin, vec );
		self->cast_info.avoid_ent = other;
	}
	else
	{	// turn to face away from them
		VectorSubtract( self->s.origin, other->s.origin, vec );
		self->cast_info.avoid_ent = NULL;
	}
	VectorNormalize( vec );

	self->ideal_yaw = vectoyaw( vec );

	if (!AI_SideTrace( self, -64, 0, 1))
		self->cast_info.currentmove = &dog_move_avoid_reverse_walk;
	else
		self->cast_info.currentmove = &dog_move_avoid_walk;
}

void dog_catch_fire( edict_t *self, edict_t *other )
{
	self->enemy = NULL;		// stop attacking
	self->cast_info.currentmove = &dog_move_run_on_fire;
}

/*QUAKED cast_dog (1 .5 0) (-16 -16 -16) (16 16 22) x TRIGGERED_START x x x 
Dog 
cast_group defines which group the character is a member of
default cast_group is 0, which is neutral (won't help others out)
player's cast_group is 1 (friendly characters)
skin = 1 or 2
model="models\actors\dog\"
*/
void SP_cast_dog (edict_t *self)
{
//	static int count; // assault virgin11 // virgin 11 crashes server
	int i;
	int	skin;

//	return;

//	self->count = (((count++)%32)+1); // assault virgin11 multiplayer

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	
	skin = self->s.skinnum = (self->skin-1) * 3;
	
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs,  16,  16,  DUCKING_MAX_Z+2);

	memset(&(self->s.model_parts[0]), 0, sizeof(model_part_t) * MAX_MODEL_PARTS);

	self->art_skins = NULL;

	self->s.num_parts++;
	self->s.model_parts[PART_HEAD].modelindex = gi.modelindex("models/actors/enemy_dog/enemy_dog.mdx");
	skin = self->s.skinnum;
	for (i=0; i<MAX_MODELPART_OBJECTS; i++)
		self->s.model_parts[PART_HEAD].baseskin = self->s.model_parts[PART_HEAD].skinnum[i] = skin;
	gi.GetObjectBounds( "models/actors/enemy_dog/enemy_dog.mdx", &self->s.model_parts[PART_HEAD] );

	if (!self->health) self->health = 400;
	if (!self->dmg)    self->dmg = 20;

	self->max_health = 400;
	self->gib_health = -200;
	self->mass = 100;

	self->gender = GENDER_NONE;

	self->yaw_speed = 10;

	self->pain = dog_pain;
	self->die = dog_die;

	self->cast_info.checkattack = AI_CheckAttack;
	self->cast_info.attack = dog_attack;
	self->cast_info.long_attack = dog_long_attack;
	self->cast_info.talk = dog_talk;
	self->cast_info.avoid = dog_avoid;
	self->cast_info.backoff = dog_backoff;
	self->cast_info.catch_fire = dog_catch_fire;
	self->cast_info.max_attack_distance = 128;

	self->cast_info.move_stand = &dog_move_amb;
	self->cast_info.move_run = &dog_move_run;
	self->cast_info.move_runwalk = &dog_move_trot;
//	self->cast_info.move_jump = &dog_move_jump;		// Jumping animation screws up jump attacking, so don't do one

	self->cast_info.move_avoid_walk = &dog_move_avoid_walk;
	self->cast_info.move_avoid_run = &dog_move_avoid_run;
	self->cast_info.move_avoid_reverse_walk = &dog_move_avoid_reverse_walk;
	self->cast_info.move_avoid_reverse_run = &dog_move_avoid_reverse_run;

	self->cast_info.move_evade = &dog_move_growl;

	self->cast_info.currentmove = self->cast_info.move_stand;

	self->cast_info.aiflags |= AI_MELEE;
	self->cast_info.aiflags |= AI_NO_TALK;
	self->cast_info.aiflags |= AI_ADJUSTPITCH;
	
//	self->gravity = 0.7;

// Temp fix for Dog in SR2 that follows lamont
//if (self->leader_target)
//	self->target = NULL;
	
	// we're a dog, therefore we don't hide
	self->cast_info.aiflags |= AI_NO_TAKE_COVER;

	gi.linkentity (self);

	walking_cast_start (self);

	self->count = -1;	// assault virgin13
}



void DogCheckForEnemies( edict_t *self ) // snatch
{
	int i;
	edict_t *trav = NULL;
	qboolean newenemy;
	newenemy = false;

	if(self->moral > 1)
	{
		// first look for any other dogs you might want to attack.
		while ((trav = G_Find (trav, FOFS(classname), "cast_dog")) != NULL)
		{
			if(    trav->style 
				&& trav->style != self->style 
				&& AI_ClearSight( self, trav, false )
				&& trav->health > 0
				&& trav->deadflag != DEAD_DEAD
//				&& trav->moral > 1
			  ) DOGMAKEENEMY
		}
	}
	// even if we see another dog, a client is a more prefered target for attack.
	for (i=0; i<maxclients->value; i++) // check for any clients close by.
	{
		trav = &g_edicts[i+1];

		if (!trav->inuse || !trav->client)											continue;
		if (!trav->client->pers.team || (trav->client->pers.team == self->style)
			&& self->cast_group != 0
			&& self->style 
			)   continue;
		if (trav->health <= 0)														continue;
		if (VectorDistance( self->s.origin, trav->s.origin ) > 2048)			    continue;
		if (!gi.inPVS( self->s.origin, trav->s.origin ))							continue;
		if (trav->svflags & SVF_NOCLIENT)											continue;
		if (trav->client->pers.game_helpchanged == WINNER)							continue;
		if (trav->client->pers.spectator == SPECTATING)								continue;
		if (trav->deadflag == DEAD_DEAD) continue;
//		if (trav->count && trav->count != self->count ) continue; // assault virgin11

		if (    VectorDistance( self->s.origin, trav->s.origin ) < 128 ) DOGMAKEENEMY

		else if (VectorDistance( self->s.origin, trav->s.origin ) < 256) DOGMAKEENEMY

		else if (VectorDistance( self->s.origin, trav->s.origin ) < 512) DOGMAKEENEMY

		else if ( VectorDistance( self->s.origin, trav->s.origin ) < 1024 
			      && AI_ClearSight( self, trav, false ) // long distance must be in sight
			    ) DOGMAKEENEMY

		else if ( VectorDistance( self->s.origin, trav->s.origin ) < 2048 
			      && AI_ClearSight( self, trav, false ) // long distance must be in sight			
			    ) DOGMAKEENEMY
	}

    // look for our start_ent
	{
		vec3_t dest; 
		float goal_dist = 1.00;
		VectorSubtract(self->start_ent->s.origin, self->s.origin, dest);
		goal_dist = VectorNormalize(dest);

		if(!newenemy && !self->enemy )
		if( goal_dist > 150)		    GOHOMEFAST
		else if( goal_dist > 75)		GOHOMESLOW
	}
}

void Cmd_Dog_f (edict_t *self, char *secondword)
{
	if(!Q_stricmp (secondword, "2"))
	{
		self->target = G_CopyString( "nikki_dog" );
		G_UseTargets (self, self);
		return;
	}
	else if(!Q_stricmp (secondword, "1"))
	{
		self->target = G_CopyString( "dragon_dog" );
		G_UseTargets (self, self);		
		return;
	}
}

void dog_respawn(edict_t *self)
{
	if(self->style == 1)
	{
//		gi.bprintf(PRINT_HIGH,"ReSpawned Dragon\'s Dog!\n");
		self->target = G_CopyString( "dragon_dog" );
		G_UseTargets (self, self);
	}
	else if(self->style == 2)
	{
//		gi.bprintf(PRINT_HIGH,"ReSpawned Nikki\'s Dog!\n");
		self->target = G_CopyString( "nikki_dog" );
		G_UseTargets (self, self);
	}
}

void Dog_Body_Animate( edict_t *ent )           // sink into ground
{
	if (ent->timestamp > (level.time - 10) && (ent->timestamp <= level.time))	
	{
		ent->s.origin[2] -= 0.5;

		if( ent->s.origin[2] < (ent->count - 20) )
		{
			// done with this body
			ent->svflags |= SVF_NOCLIENT;
			dog_respawn(ent); // snatch replace dead dog.
			AI_UnloadCastMemory( ent );
			G_FreeEdict (ent);
			return;
		}
	}
	ent->think = Dog_Body_Animate;
	ent->nextthink = level.time + 0.1;
}

/************************************************
qboolean CheckDogScripts(edict_t *self)

is called from
g_ai_fight.c: qboolean AI_CheckAttack(edict_t *self) AND
g_ai.c:       void AI_TalkThink( edict_t *self, qboolean ismale )


Allows scripts to run for cast_dog etc.
*************************************************/


qboolean CheckDogScripts(edict_t *self)
{
	int n = self->option - self->count;

	if(    self->count > -1 
		&& !strcmp(self->classname,"cast_dog") 
		&& self->gesture[n] ) // assault virgin13
	{
		CHECKPLAYSOUND

		else if(!strcmp(self->gesture[n],"dog_attack"))	
		{
			AssaultAICheckForEnemies(self);
			if(self->count-- < 0) return false;
		}

		n = self->option - self->count;

			 if(!strcmp(self->gesture[n],"wait") || !strcmp(self->gesture[n],"dog_move_amb") )
		   self->cast_info.currentmove = &dog_move_amb;
		else if(!strcmp(self->gesture[n],"dog_move_pant"))
		   self->cast_info.currentmove = &dog_move_pant;
		else if(!strcmp(self->gesture[n],"dog_move_sniff"))
		   self->cast_info.currentmove = &dog_move_sniff;
		else if(!strcmp(self->gesture[n],"dog_move_bark"))
		   self->cast_info.currentmove = &dog_move_bark;
		else if(!strcmp(self->gesture[n],"dog_move_growl"))
		   self->cast_info.currentmove = &dog_move_growl;
		else if(!strcmp(self->gesture[n],"enemy_dog_move_pee"))
		   self->cast_info.currentmove = &enemy_dog_move_pee;
		else if(!strcmp(self->gesture[n],"dog_move_low_atk"))
		   self->cast_info.currentmove = &dog_move_low_atk;
		else if(!strcmp(self->gesture[n],"dog_move_med_atk"))
		   self->cast_info.currentmove = &dog_move_med_atk;
		else if(!strcmp(self->gesture[n],"dog_move_upr_atk"))
		   self->cast_info.currentmove = &dog_move_upr_atk;
		else if(!strcmp(self->gesture[n],"dog_move_pain1"))
		   self->cast_info.currentmove = &dog_move_pain1;
		else if(!strcmp(self->gesture[n],"dog_move_pain2"))
		   self->cast_info.currentmove = &dog_move_pain2;
		else if(!strcmp(self->gesture[n],"dog_move_pain3"))
		   self->cast_info.currentmove = &dog_move_pain3;
		else if(!strcmp(self->gesture[n],"dog_move_death1"))
		   self->cast_info.currentmove = &dog_move_death1;
		else if(!strcmp(self->gesture[n],"dog_move_death2"))
		   self->cast_info.currentmove = &dog_move_death2;
		else if(!strcmp(self->gesture[n],"dog_move_walk"))
		   self->cast_info.currentmove = &dog_move_walk;
		else if(!strcmp(self->gesture[n],"dog_move_trot"))
		   self->cast_info.currentmove = &dog_move_trot;
		else if(!strcmp(self->gesture[n],"dog_move_run"))
		   self->cast_info.currentmove = &dog_move_run;
		else if(!strcmp(self->gesture[n],"dog_move_run_on_fire"))
		   self->cast_info.currentmove = &dog_move_run_on_fire;
		else if(!strcmp(self->gesture[n],"dog_move_avoid_walk"))
		   self->cast_info.currentmove = &dog_move_avoid_walk;
		else if(!strcmp(self->gesture[n],"dog_move_avoid_reverse_walk"))
		   self->cast_info.currentmove = &dog_move_avoid_reverse_walk;
		else if(!strcmp(self->gesture[n],"dog_move_avoid_run"))
		   self->cast_info.currentmove = &dog_move_avoid_run;
		else if(!strcmp(self->gesture[n],"dog_move_avoid_reverse_run"))
		   self->cast_info.currentmove = &dog_move_avoid_reverse_run;
		else self->cast_info.currentmove = &dog_move_amb;


#if CHIEFCHAT
		gi.bprintf(PRINT_HIGH,"Script line: %i %s gesture: %s\n",n, self->classname, self->gesture[n]);
#endif
		self->count--;
		return true;
	}

	return false;
}

