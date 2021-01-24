
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

//............................................................................
// Called whenever a character reaches a path_corner_cast that has a "scriptname" set
void DoLine(edict_t *self, char *buffer)
{
	// Make sure the line's not empty
	if (strlen(buffer) == 0 || buffer[0] == '\n') return;
	// Check to see if this is a comment line
	else if (buffer[0] == '/' && buffer[1] == '/') return;
	else if (buffer[0] == '#' ) return;

	if(self->count < 16)
		strcpy(self->gesture[self->count],buffer);
// working perfectly so far
//	gi.bprintf(PRINT_HIGH,"script command: %s\n", self->gesture[self->count]);
	self->count++;
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