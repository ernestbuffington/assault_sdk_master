
#include "g_local.h"

/************************************************
void Die_Body_Animate( edict_t *ent )
is called from
g_ai.c: void AI_EndDeath(edict_t *self)

This routine causes dead ai to sink into the 
ground soonest ultimately terminating in a
			ent->svflags |= SVF_NOCLIENT;
			AI_UnloadCastMemory( ent );
			G_FreeEdict (ent);
*************************************************/


void Die_Body_Animate( edict_t *ent )           // sink into ground
{
	if (ent->timestamp > (level.time - 10) && (ent->timestamp <= level.time))	
	{
		ent->s.origin[2] -= 0.5;

		if( ent->s.origin[2] < (ent->count - 20) )
		{
			// done with this body
			ent->svflags |= SVF_NOCLIENT;
//			if (!Q_stricmp(ent->classname, "cast_dog")) dog_respawn(ent);

			AI_UnloadCastMemory( ent );
			G_FreeEdict (ent);
			return;
		}
	}
	ent->think = Die_Body_Animate;
	ent->nextthink = level.time + 0.1;
}

/************************************************
void DoLine(edict_t *self, char *buffer) AND
void getline(edict_t *self, FILE *infile, char *buffer)

are called from
ep_all.c: void EP_EventScript ( edict_t *self, char *scriptname ) // assault virgin11beta


Allows scripts to run for cast_thug etc.
*************************************************/

//............................................................................
// Called whenever a character reaches a path_corner_cast that has a "scriptname" set
void DoLine(edict_t *self, char *buffer)
{
	// Make sure the line's not empty
	if (strlen(buffer) == 0 || buffer[0] == '\n') return;
	// Check to see if this is a comment line
	else if (buffer[0] == '/' && buffer[1] == '/') return;
	else if (buffer[0] == '#' ) return;

	if(    !strstr(buffer,"wait")   // non cast specific script commands. 
		&& !strstr(buffer,"target")
		&& !strstr(buffer,"play")
			// only record stuff with our cast name on it.
		&& buffer[0] == self->classname[5]
		&& buffer[1] == self->classname[6]
		&& buffer[2] == self->classname[7]
		&& self->count < 16
	  ) 
	{
		strcpy(self->gesture[self->count],buffer);
		self->count++;
//		gi.bprintf(PRINT_HIGH,"command %s recorded to %s\n",buffer,self->classname);
		return;
	}

	else if(	self->count < 16 &&
		( strstr(buffer,"wait") ||	strstr(buffer,"target") || strstr(buffer,"play") )
	  )
	{
		strcpy(self->gesture[self->count],buffer);
		self->count++;
		return;
	}

//	else gi.bprintf(PRINT_HIGH,"Command %s Disregarded for %s\n",buffer,self->classname);
// working perfectly so far
//	gi.bprintf(PRINT_HIGH,"script command: %s\n", self->gesture[self->count]);

}

void getline(edict_t *self, FILE *infile, char *buffer)
{
	int		i=0;
	char	ch;

	ch = fgetc(infile);

	while ( (ch != EOF) && (ch != '\n') && (i < (1500)) )
	{
		buffer[i++] = ch;
		ch = fgetc(infile);
	}

	if ( (ch != '\n') && (ch != EOF) )	ch = fgetc(infile);

	buffer[i] = '\0'; // i is now the number of characters in the string.

	DoLine(self, buffer);
}

/************************************************
void GetValue(char *buffer, char *gesture)

are called from ai_thug.c CheckScripts

*************************************************/
void GetValue(char *buffer, char *gesture)
{
	int c = 0;
	int w = 0;
	while(gesture[c] && c<64)
	{
		if(gesture[c] == ' ') { c++; break; }
		c++;
	}

	while(gesture[c] && c<64 && w<8)
	{
		buffer[w] = gesture[c];
		c++;
		w++;
	}
	buffer[w]='\0';
}

/************************************************
void AssaultAICheckForEnemies( edict_t *self )

are called from CheckScripts in ai_thug.c

*************************************************/
#define SETENT self->enemy = NULL; 
//if(self->last_goal) self->goalentity = self->goal_ent = self->last_goal;

#define GOHOMEFAST { SETENT \
			self->cast_info.aiflags |= AI_FASTWALK; \
			self->cast_info.currentmove = self->cast_info.move_run; }

#define GOHOMESLOW { SETENT \
			self->cast_info.aiflags &= ~AI_FASTWALK; \
			self->cast_info.currentmove = self->cast_info.move_runwalk; }

#define MAKEENEMY { /*if(self->goal_ent) self->last_goal = self->goal_ent;*/ \
						self->enemy = trav;	\
						AI_MakeEnemy ( self, trav, 0 );	newenemy = true; }

#define CHECKNAME if( AI_ClearSight( self, trav, false ) \
						&& trav->name \
						&& trav->health > 0 \
						&& trav->deadflag != DEAD_DEAD \
						&& strstr (self->skins, trav->name) \
					) { self->enemy = trav;	\
						AI_MakeEnemy ( self, trav, 0 ); return; }

void AssaultAICheckForEnemies( edict_t *self )
{
	int i;
	edict_t *trav = NULL;
	qboolean newenemy;
	newenemy = false;
/*
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
			  ) MAKEENEMY
		}
	}
*/

	if(strlen(self->skins)>1) // ive been told to target somebody specific.
	{
		// first look for any other cast you might want to attack.
		while ((trav = G_Find (trav, FOFS(classname), "cast_bitch")) != NULL)
		{ CHECKNAME }
		while ((trav = G_Find (trav, FOFS(classname), "cast_dog")) != NULL)
		{ CHECKNAME }
		while ((trav = G_Find (trav, FOFS(classname), "cast_punk")) != NULL)
		{ CHECKNAME }
		while ((trav = G_Find (trav, FOFS(classname), "cast_runt")) != NULL)
		{ CHECKNAME }
		while ((trav = G_Find (trav, FOFS(classname), "cast_shorty")) != NULL)
		{ CHECKNAME }
		while ((trav = G_Find (trav, FOFS(classname), "cast_thug")) != NULL)
		{ CHECKNAME }
		while ((trav = G_Find (trav, FOFS(classname), "cast_thug2")) != NULL)
		{ CHECKNAME }
		while ((trav = G_Find (trav, FOFS(classname), "cast_whore")) != NULL)
		{ CHECKNAME }

//		gi.bprintf(PRINT_HIGH,"%s: Killtarget %s Not In Sight!\n",self->classname,self->skins);
		strcpy(self->skins," ");
//		return;
	}
	
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

		if (    VectorDistance( self->s.origin, trav->s.origin ) < 128 ) MAKEENEMY

		else if (VectorDistance( self->s.origin, trav->s.origin ) < 256) MAKEENEMY

		else if (VectorDistance( self->s.origin, trav->s.origin ) < 512) MAKEENEMY

		else if ( VectorDistance( self->s.origin, trav->s.origin ) < 1024 
			      && AI_ClearSight( self, trav, false ) // long distance must be in sight
			    ) MAKEENEMY

		else if ( VectorDistance( self->s.origin, trav->s.origin ) < 2048 
			      && AI_ClearSight( self, trav, false ) // long distance must be in sight			
			    ) MAKEENEMY
	}

	/*
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
	*/
}
