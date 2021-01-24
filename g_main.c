
#include "g_local.h"
//#include "stdlog.h"	//	Standard Logging
//#include "gslog.h"	//	Standard Logging

#include "voice_bitch.h"
#include "voice_punk.h"

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;

int	sm_meat_index;
int	snd_fry;

int meansOfDeath;

edict_t		*g_edicts;

cast_memory_t	*g_cast_memory;
cast_group_t	*g_cast_groups;

int		num_object_bounds=0;
object_bounds_t	*g_objbnds[MAX_OBJECT_BOUNDS];

cvar_t	*deathmatch;

// RAFAEL
// cvar_t	*marines;

cvar_t	*coop;
cvar_t	*dmflags;
cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
cvar_t	*cashlimit;
cvar_t	*password;
cvar_t	*maxclients;
cvar_t	*maxentities;
cvar_t	*g_select_empty;
cvar_t	*dedicated;

cvar_t	*maxrate;

cvar_t	*filterban;

cvar_t	*sv_maxvelocity;
cvar_t	*sv_gravity;

cvar_t	*sv_rollspeed;
cvar_t	*sv_rollangle;
cvar_t	*gun_x;
cvar_t	*gun_y;
cvar_t	*gun_z;

cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;

cvar_t	*sv_cheats;

cvar_t	*flood_msgs;
cvar_t	*flood_persecond;
cvar_t	*flood_waitdelay;

// Ridah, new cvar's
cvar_t	*developer;
cvar_t	*ai_debug_memory;

cvar_t	*g_vehicle_test;

cvar_t	*dm_locational_damage;

cvar_t	*showlights;

cvar_t	*r_directional_lighting;

cvar_t	*cl_captions;

cvar_t	*sv_runscale;	// 2.0 = double speed, 0.0 = zero movement

cvar_t	*burn_enabled;
cvar_t	*burn_size;
cvar_t	*burn_intensity;
cvar_t	*burn_r;
cvar_t	*burn_g;
cvar_t	*burn_b;

cvar_t	*timescale;

cvar_t	*teamplay;
cvar_t	*g_cashspawndelay;

cvar_t	*cl_parental_lock;
cvar_t	*cl_parental_override;

cvar_t	*dm_realmode;

cvar_t	*g_mapcycle_file;
// Ridah, done.

void SpawnEntities (char *mapname, char *entities, char *spawnpoint);
void ClientThink (edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
void ClientBegin (edict_t *ent);
void ClientCommand (edict_t *ent);
void RunEntity (edict_t *ent);
void WriteGame (char *filename, qboolean autosave);
void ReadGame (char *filename);
void WriteLevel (char *filename);
void ReadLevel (char *filename);
void InitGame (void);
void G_RunFrame (void);

unsigned char		*lpbuf[0xFFFF];
int					num_lpbuf;

// JOSEPH 23-OCT-98
void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point, int mdx_part, int mdx_subobject);

//===================================================================

int gameinc=0;

void ShutdownGame (void)
{
	int			i;
	edict_t		*ent;
	char buf[1024]="";

	buf[0]=0;
	for (i=0 ; i<maxclients->value ; i++) {
		ent = g_edicts + 1 + i;
		if (!ent->inuse) continue;
		if (ent->client->pers.rconx[0])
			Info_SetValueForKey(buf,ent->client->pers.ip,ent->client->pers.rconx);
	}
	gi.cvar_set("rconx",buf);

	if (keep_admin_status) {
		for (i=0 ; i<maxclients->value ; i++) {
			ent = g_edicts + 1 + i;
			if (!ent->inuse) continue;
			if (ent->client->pers.admin>NOT_ADMIN) {
				gi.cvar_set("modadmin",ent->client->pers.ip);
				break;
			}
		}
		if (i==maxclients->value)
			gi.cvar_set("modadmin","");
	}

	gi.dprintf ("==== ShutdownGame ====\n");

//	sl_GameEnd( &gi, level );	// Standard Logging

// BEGIN:	Xatrix/Ridah/Navigator/21-mar-1998
	NAV_PurgeActiveNodes (level.node_data);	
// END:		Xatrix/Ridah/Navigator/21-mar-1998

	// Ridah, clear the lightpaint buffers
	if (num_lpbuf > 0)
	{
		int i;

		for (i=0; i<num_lpbuf; i++)
			free( lpbuf[i] );
	}

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);

	gi.ClearObjectBoundsCached();	// make sure we wipe the cached list
}

int *GetNumObjectBounds (void)
{
	return &num_object_bounds;
}

void *GetObjectBoundsPointer (void)
{
	return (void *)(&g_objbnds);
}

int GetNumJuniors (void)
{
	return level.num_light_sources;
}

/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
game_export_t *GetGameAPI (game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	globals.GetNumObjectBounds = GetNumObjectBounds;
	globals.GetObjectBoundsPointer = GetObjectBoundsPointer;

	globals.GetNumJuniors = GetNumJuniors;

	return &globals;
}

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	gi.error (ERR_FATAL, "%s", text);
}

void Com_Printf (char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	gi.dprintf ("%s", text);
}

#endif

//======================================================================


/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames (void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		ClientEndServerFrame (ent);
	}

}

/*
=================
MapCycleNext

  Uses maps.lst and teammaps.lst to cycle the maps during deathmatch
=================
*/
char *MapCycleNext( char *map )
{
	char	*basevars[] = {"basedir", "cddir", NULL};	// consol variables that point to possible file locations
	cvar_t	*game_dir, *base_dir;
	char	filename[MAX_QPATH], dir[MAX_QPATH];
	FILE	*f;
	static char	nextmap[MAX_QPATH];
	char	firstmap[MAX_QPATH];
	char	travmap[MAX_QPATH];
	qboolean matched = false;
	int		i;
	char	ch;
	qboolean eof = false;

	game_dir = gi.cvar("game", "", 0);

	// dir, eg: .\gamedir\routes
	for (i=0; basevars[i]; i++)
	{
		base_dir = gi.cvar(basevars[i], ".", 0);

		strcpy( dir, base_dir->string);

		if (dir[strlen(dir)-1] != DIR_SLASH[0])
			strcat( dir, DIR_SLASH);

		if (strlen(game_dir->string) == 0)
			strcat( dir, "main");
		else
			strcat( dir, game_dir->string);

		// filename, eg: .\gamedir\maps.lst
		strcpy( filename, dir);
		strcat( filename, DIR_SLASH);
		if (g_mapcycle_file->string && strlen(g_mapcycle_file->string) > 0)
			strcat( filename, g_mapcycle_file->string);
		else if (!teamplay->value)
			strcat( filename, "maps.lst");
		else
			strcat( filename, "teammaps.lst");

		// try and open the file for reading
		f = fopen ( filename, "rb");
		if (f)
			break;	// we have a valid file
	}

	if (!f)	// no valid file found
		return NULL;

	// read in the first map
	fscanf( f, "%s", firstmap );
	strcpy( travmap, firstmap );
	ch = 0;
	while (ch!='\n' && !feof(f))
		fscanf(f, "%c", &ch);

	do
	{
		eof = feof(f);

		if (!Q_stricmp( travmap, level.mapname ))
		{
			matched = true;
		}

		if (!eof)
		{
			fscanf( f, "%s", travmap );
			ch = 0;
			while (ch!='\n' && !feof(f))
				fscanf(f, "%c", &ch);
		}

		if (matched)
		{
			if (strcmp(travmap, level.mapname) != 0 && strlen(travmap) > 1)
			{	// take this map
				strcpy( nextmap, travmap );
			}
			else
			{	// use the firstmap
				strcpy( nextmap, firstmap );
			}

			fclose(f);
			return nextmap;
		}
	}
	while (!eof);

	fclose(f);

	// no match, so return nothing
	return NULL;
}

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel (void)
{
	edict_t		*ent;
	char		*nextmap;
	static int  replaythismap;

	//	gi.cvar_set("password",default_password);
	if ((assault_replaymaps) && (MapReplay) && (level.unit <= 3) ) //level.unit is the total number of keys in the map
	{
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = level.mapname; //play the same map again
		goto done;
	}
 // assault2


	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = level.mapname;

		goto done;
	}

	if ( (level.unit > 3) && (allow_map_voting) && (MapReplay) ) // 6 key maps with voting turned on
	{
		SetupMapVote();
		return;
	}

	if (nextmap = MapCycleNext( level.mapname ))
	{
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = nextmap;

		goto done;
	}

	if (level.nextmap[0])
	{	// go to a specific map
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = level.nextmap;
	}
	else
	{	// search for a changelevel
		ent = G_Find (NULL, FOFS(classname), "target_changelevel");
		if (!ent)
		{	// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			ent = G_Spawn ();
			ent->classname = "target_changelevel";
			ent->map = level.mapname;
		}
	}

done:

	if (ent && ent->map)
	{ // assault2
		if (strstr(ent->map, "assault"))
			gi.dprintf("Assault changelevel: %s (time: %i secs)\n", ent->map, (int)level.time );
		else
			gi.dprintf("DM changelevel: %s (time: %i secs)\n", ent->map, (int)level.time );
	}


	// Ridah, play a random music clip
	gi.WriteByte( svc_stufftext );
	gi.WriteString( va("play world/cypress%i.wav", 2+(rand()%4)) );
	gi.multicast (vec3_origin, MULTICAST_ALL);

	BeginIntermission (ent);
}


///////////////////////////////////////////////////////////////////////////////////////////
///// CheckForKeysGoneMissing /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///// level.unit stores the total numbers of keys for this map, both teams included. //////
///////////////////////////////////////////////////////////////////////////////////////////
///// level.total_secrets  = thugkeyone;    ///// level.found_secrets   = runtkeyone; /////
///// level.total_goals    = thugkeytwo;    ///// level.found_goals     = runtkeytwo; /////
///// level.total_monsters = thugkeythree;  ///// level.killed_monsters = runtkeythree; ///
///////////////////////////////////////////////////////////////////////////////////////////

void CheckForKeysGoneMissing(void) // ASSAULT2 bug fix for when the game is ruined cuz a key has gone missing
{
	edict_t	*key = NULL;
	edict_t *keyholder;
	int thugkeyone, thugkeytwo, thugkeythree, runtkeyone, runtkeytwo, runtkeythree, teamnumber;
	int possessedruntkeys, possessedthugkeys, existingruntkeys, existingthugkeys;
	qboolean keygonemissing; //to tell me if there is a problem or not

	keygonemissing = false;
	teamnumber = 2; //default to runts team
	thugkeyone=thugkeytwo=thugkeythree=runtkeyone=runtkeytwo=runtkeythree=0;
	possessedruntkeys=possessedthugkeys=existingruntkeys=existingthugkeys=0;

	if(level.power_cubes == 86) // assault2 , a key has just exploded and this func got called by main so only do it once.
		level.power_cubes = 0;

	//loop through all the entities in the map and only do this code if you find a key_key1
	while((key = G_Find (key, FOFS(classname), "key_key1")) != NULL) 
	{
		if(key->style == 1) thugkeyone++;
		else if(key->style == 2) runtkeyone++;
	}

	//loop through all the entities in the map and only do this code if you find a key_key2
	while((key = G_Find (key, FOFS(classname), "key_key2")) != NULL)
	{
		if(key->style == 1) thugkeytwo++;
		else if(key->style == 2) runtkeytwo++;
	}

	//loop through all the entities in the map and only do this code if you find a key_key3
	while((key = G_Find (key, FOFS(classname), "key_key3")) != NULL)
	{
		if(key->style == 1) thugkeythree++;
		else if(key->style == 2) runtkeythree++;
	}

	existingruntkeys = runtkeyone + runtkeytwo + runtkeythree;
	existingthugkeys = thugkeyone + thugkeytwo + thugkeythree;

//	if( ( existingruntkeys + existingthugkeys ) == level.unit) goto finished; // all keys still exist in the map.

//	else //if( ( existingruntkeys + existingthugkeys ) != level.unit) // not all keys exist so check for deposits and clients carrying.
	{

		int k,holdersteam;

		for_everyone(keyholder,k) //look for keys in clients possession.
		{
			holdersteam = keyholder->client->pers.team;
			switch(holdersteam)
			{
			case 1:		if (keyholder->client->pers.inventory[ITEM_INDEX(FindItem("Key_One"))])   possessedthugkeys++;
						if (keyholder->client->pers.inventory[ITEM_INDEX(FindItem("Key_Two"))])   possessedthugkeys++;
						if (keyholder->client->pers.inventory[ITEM_INDEX(FindItem("Key_Three"))]) possessedthugkeys++;
						break;
			default:
			case 2:		if (keyholder->client->pers.inventory[ITEM_INDEX(FindItem("Key_One"))])   possessedruntkeys++;
						if (keyholder->client->pers.inventory[ITEM_INDEX(FindItem("Key_Two"))])   possessedruntkeys++;
						if (keyholder->client->pers.inventory[ITEM_INDEX(FindItem("Key_Three"))]) possessedruntkeys++;
						break;
			}
		}
	}

	if( (existingthugkeys + existingruntkeys + team_cash[1] + team_cash[2] + possessedthugkeys + possessedruntkeys) == level.unit )
		goto finished;

	else
	{
		if( (existingthugkeys + existingruntkeys + team_cash[1] + team_cash[2] + possessedthugkeys + possessedruntkeys) > level.unit )
		{ gi.bprintf (PRINT_HIGH,"Number of registered keys exceeds the map total!\n" ); goto finished; }

		keygonemissing = true;

		// find out whose team is missing a key
		if( level.unit == 3 ) teamnumber = 2;

		else
		{
			if( (existingthugkeys+possessedthugkeys+team_cash[1]) < (existingruntkeys+possessedruntkeys+team_cash[2]) ) teamnumber = 1;
			else teamnumber = 2;
		}

	}

finished:

	if(keygonemissing)
	{ 
		char *msg;

		msg = "key_key?";
		//check the Score to tell which key has gone missing.
		if     (team_cash[teamnumber] == 0) { if(teamnumber == 1) strcpy (msg , "thugs_key1"); else strcpy (msg , "runts_key1"); }
		else if(team_cash[teamnumber] == 1) { if(teamnumber == 1) strcpy (msg , "thugs_key2"); else strcpy (msg , "runts_key2"); }
		else if(team_cash[teamnumber] == 2) { if(teamnumber == 1) strcpy (msg , "thugs_key3"); else strcpy (msg , "runts_key3"); }

		keyholder->target = G_CopyString( msg );
		G_UseTargets (keyholder,keyholder);
		return;
	}
/*
	else
	gi.bprintf (PRINT_HIGH,
		"\n ThugKeyOne  : %i\n ThugKeyTwo  : %i\n ThugKeyThree: %i\n RuntKeyOne  : %i\n RuntKeyTwo  : %i\n RuntKeyThree: %i\nHeld  Keys: %i\nTotal Dpst: %i\n\nTotal Keys: %i\n\n",
		thugkeyone, thugkeytwo, thugkeythree,runtkeyone,runtkeytwo,runtkeythree,(possessedruntkeys+possessedthugkeys),(team_cash[1]+team_cash[2]),
		thugkeyone+thugkeytwo+thugkeythree+runtkeyone+runtkeytwo+runtkeythree+possessedruntkeys+possessedthugkeys+team_cash[1]+team_cash[2]);
*/
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules (void)
{
	int		count=0;

	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;

//////////////////////////////// ASSAULT2

	if(    (level.total_secrets != 69) 
		&& (level.time >= (timelimit->value*60)+34) 
		&& (timelimit->value)
	  ) // time ran out
		{
			edict_t *teammates; 
			int		j,i;
			qboolean trigger;
			trigger = false;

			for (j = 1; j <= game.maxclients; j++) //to handle end_game triggering
				{
					teammates = &g_edicts[j];

					if (!(teammates->inuse))
						continue;
					if (!(teammates->client))
						continue;

					if(!trigger) //first client on either team triggers end game
					{
						if(level.thugs_key3_target)			teammates->target = G_CopyString( level.thugs_key3_target ); // trigger Hogie's cut scene
						else if(level.runts_key3_target)	teammates->target = G_CopyString( level.runts_key3_target );
						else								teammates->target = G_CopyString( "end_game" );
						G_UseTargets (teammates, teammates);
						trigger = true;
					}


					if (teammates->client->pers.team != 1
						&& teammates->client->pers.spectator == PLAYING // virgin21
						) // These guys are the losers.
					{
						teammates->client->pers.game_helpchanged = LOSER; // Set this value so we can spawn them to a style 3 spawner.
						RESPAWN_ASSAULT(teammates)
					}

					else //These guys are on the winning team.
					{	
						edict_t *ent;
					
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
			} //closing for (j = 1; j <= game.maxclients; j++)


			level.fadeendtime = level.time; //timestamp the level so it will end after the end_game
			level.total_secrets = 69; // redundent check for a key three time stamp
			return;
		}

	// check if key3 + 15 seconds has been deposited + ellapsed.
	if((level.total_secrets == 69) && (level.fadeendtime != 0) && (level.fadeendtime <= level.time - cutscenetime) )
		{
			level.total_secrets = 0;
			level.fadeendtime = 0;

			if( (assault_auto_switch_teams) && (level.unit<=3) ) //only switch teams for 3 key maps.
			{
				edict_t *teammates; 
				int		j;
				for (j = 1; j <= game.maxclients; j++) // to switch everyone's team around
					{
						teammates = &g_edicts[j];
						if (!(teammates->inuse))
							continue;
						if (!(teammates->client))
							continue;

						if (teammates->client->pers.team == 1) 
							teammates->client->pers.team = 2; // AUTO TEAM SWITCH assault2
						else teammates->client->pers.team = 1;
					}
				gi.bprintf (PRINT_HIGH, "Automagically Switching Teams!\n"); //HOGIE

			}
			if ((!allow_map_voting) || (assault_replaymaps && MapReplay) ) {
				EndDMLevel ();
				if((MapReplay) && (assault_replaymaps) && (level.unit<=3) )	MapReplay = false; // assault2
			}
			else {
				SetupMapVote();
				if( (assault_replaymaps) && (!MapReplay) ) MapReplay = true;
			}

			AcceptableThugSpawnerStyle = 1;
			AcceptableRuntSpawnerStyle = 2;
			return;
		}

	if(level.power_cubes == 86) // assault2 , a key has just exploded.
		CheckForKeysGoneMissing();

	if ((level.framenum % 300) == 0) 	//every 30 seconds
		CheckForKeysGoneMissing();

/*
	if (fraglimit->value)
	{
		for (i=0 ; i<maxclients->value ; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;

			if (cl->resp.score >= fraglimit->value)
			{
				gi.bprintf (PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel ();
				return;
			}
		}
	}

	if (cashlimit->value)
	{
		if ((team_cash[1] >= cashlimit->value) || (team_cash[2] >= cashlimit->value))
		{
			gi.bprintf (PRINT_HIGH, "Cashlimit hit.\n");
			EndDMLevel ();
			return;
		}
	}
*/
	/////////////////// END ASSAULT2
}



/*
=============
ExitLevel
=============
*/

void ExitLevel (void)
{
	int		i;
	edict_t	*ent;
	char	command [256];
	char buf[1024]="";


	level.player_num=0;

	buf[0]=0;
	for (i=0 ; i<maxclients->value ; i++) {
		ent = g_edicts + 1 + i;
		if (!ent->inuse) continue;
		if (ent->client->pers.rconx[0])
			Info_SetValueForKey(buf,ent->client->pers.ip,ent->client->pers.rconx);
	}
	gi.cvar_set("rconx",buf);

	if (keep_admin_status) {
		for (i=0 ; i<maxclients->value ; i++) {
			ent = g_edicts + 1 + i;
			if (!ent->inuse)
				continue;
			if (ent->client->pers.admin>NOT_ADMIN) {
				gi.cvar_set("modadmin",ent->client->pers.ip);
				break;
			}
		}
		if (i==maxclients->value)
			gi.cvar_set("modadmin","");
	}

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();

// RAFAEL
	level.cut_scene_time = 0;
	level.speaktime = 0;
	// JOSEPH 25-FEB-99
	level.cut_scene_end_count = 0;
    level.fadeendtime = 0;
	level.totalfade = 0;
	// END JOSEPH

	// JOSEPH 13-JUN-99
    level.helpchange = 0;
	// END JOSEPH

	if (level.bar_lvl)
	{
		extern void Cmd_HolsterBar_f (edict_t *ent);
		edict_t *player;
		
		player = &g_edicts[1];
		level.bar_lvl = false;
		
		Cmd_HolsterBar_f (player);
	}
		
	// level.pawn_time = 0;
	// level.pawn_exit = false;

	// clear some things before going to next level
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;

	}

// BEGIN:	Xatrix/Ridah/19-apr-1998
	// make sure Nav data isn't carried over to next level
	NAV_PurgeActiveNodes(level.node_data);
	level.node_data = NULL;
// BEGIN:	Xatrix/Ridah/19-apr-1998

}

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
extern int bbox_cnt;
extern edict_t	*mdx_bbox[];

void AI_ProcessCombat (void);

void G_RunFrame (void)
{
	int		i;
	edict_t	*ent;

	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	// exit intermissions

	if (level.exitintermission)
	{
		ExitLevel ();
		return;
	}


	// do character sighting/memory stuff
	if ((maxclients->value > 1) && !(deathmatch->value))
	{	// coop server, do more checking here

		if (dedicated->value)
			AI_UpdateCharacterMemories( 256 );

	}


	// Process Generic Combat AI layer

	AI_ProcessCombat ();


	//
	// treat each object in turn
	// even the world gets a chance to think
	//
	ent = &g_edicts[0];
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		level.current_entity = ent;

		VectorCopy (ent->s.origin, ent->s.old_origin);


		if (ent->svflags & SVF_MONSTER || ent->client)
		{
			if (ent->waterlevel > 1)
			{
				ent->onfiretime = 0;
			}

			// On fire
			if (ent->onfiretime < 0)
			{
				ent->onfiretime++;
			}
			else if (ent->onfiretime > 0)
			{
				vec3_t	point, org, dir;
				int		i,j;
				float	dist;

				// Deathmatch flames done on client-side
				if ((!deathmatch->value /*|| ent->onfiretime == 1*/) && (deathmatch->value || !ent->client))
				{
					VectorSubtract( g_edicts[1].s.origin, ent->s.origin, dir );
					dist = VectorNormalize( dir );

					// Ridah, spawn flames at each body part
					MDX_HitCheck( ent, world, world, vec3_origin, vec3_origin, vec3_origin, 0, 0, 0, 0, vec3_origin );

					for (i = 0; i < bbox_cnt; i++)
					{

						// don't draw so many if the client is up close
						if (dist < 256)
						{
							if (random() > dist/256)
								continue;
						}

						VectorAdd( mdx_bbox[i]->s.origin, dir, org );

						if (!deathmatch->value)
						{
							for (j=0; j<2; j++)
							{
								point[2] = (org[2] + ((rand()%18) - 6) + 6);
								point[1] = (org[1] + ((rand()%10) - 5));
								point[0] = (org[0] + ((rand()%10) - 5));

								gi.WriteByte (svc_temp_entity);
								gi.WriteByte (TE_SFXFIREGO);
								gi.WritePosition (point);

								if (ent->onfiretime == 1)
									gi.WriteByte (1.2 * 10.0);
								else
									gi.WriteByte (0.6 * 10.0);

								gi.multicast (point, MULTICAST_PVS);		
							}
						}

						// just do one smoke cloud
						if ((ent->onfiretime == 1) && (rand()%2))
						{
							point[2] = (org[2] + 20);// + ((rand()&31) - 16) + 20);
							point[1] = (org[1]);// + ((rand()%14) - 7));
							point[0] = (org[0]);// + ((rand()%14) - 7));

							gi.WriteByte (svc_temp_entity);
							gi.WriteByte (TE_SFXSMOKE);
							gi.WritePosition (point);
							// gi.WriteDir (ent->s.angles);
							gi.WriteByte (16 + (rand()%24));
							gi.WriteByte (0);
							gi.multicast (point, MULTICAST_PVS);
						}
					}
				}

			    if (!ent->deadflag)
				{
					edict_t *trav=NULL;
					float	damage=1;

					if (!deathmatch->value)
						damage;// *= 3;
					else
						damage;// *= 2;

					T_Damage( ent, ent->onfireent, ent->onfireent, vec3_origin, ent->s.origin, vec3_origin, damage, 0, DAMAGE_NO_KNOCKBACK, MOD_FLAMETHROWER );

					// make sure they are in the "catch_fire" motion
					if (!deathmatch->value && (ent->health > 0) && ent->cast_info.catch_fire)
					{
						ent->cast_info.catch_fire( ent, ent->onfireent );
					}

				}

				ent->onfiretime--;	

				if (ent->onfiretime <= 0)
				{
					ent->onfireent = NULL;
					ent->onfiretime = 0;
				}

				// JOSEPH 3-JUN-99
				if (ent->health > 0 && ent->onfiretime == 0)
				{
					ent->s.model_parts[PART_GUN].invisible_objects = 0;
					ent->s.model_parts[PART_GUN2].invisible_objects = 0;
				}
				else
				{
					ent->s.model_parts[PART_GUN].invisible_objects = (1<<0 | 1<<1);
					ent->s.model_parts[PART_GUN2].invisible_objects = (1<<0 | 1<<1);
				}
				// END JOSEPH

				if (ent->health > 0)
				{
					// use voice tables for this?

					// gi.dprintf( "SOUND TODO: ARRRGGGHH!!! (on fire)\n" );
				}
				else	// dead
				{
					if (ent->onfiretime > 20)
						ent->onfiretime = 20;

					if (ent->onfiretime == 1)
					{
						gi.WriteByte (svc_temp_entity);
						gi.WriteByte (TE_BURN_TO_A_CRISP);
						gi.WritePosition (ent->s.origin);
						gi.multicast (ent->s.origin, MULTICAST_PVS);
					}

				}
			}
		}

		if (ent->svflags & SVF_MONSTER)
		{
//			float	alpha;

			
			// Blood trail
			// JOSEPH 24-JAN-99
			if ((!ent->deadflag) && (ent->health < ent->max_health) && (!(ent->svflags & SVF_PROP))/*&& ((ent->healtimer&3) == 2)*/)
			// END JOSEPH
			{
				vec3_t	stop;
				vec3_t	start;
				trace_t	trace;
					
				VectorCopy(ent->s.origin, start);
				start[0] += ((rand()&15)-8); 
				start[1] += ((rand()&15)-8);	
				VectorCopy(start, stop);
				stop[2] -= 16*16;

				trace = gi.trace (start, NULL, NULL, stop, ent, MASK_DEADSOLID);
				if (trace.fraction < 1)
				{
					float	rnd;

					rnd = (0.2 + 1.5*random());
					SurfaceSpriteEffect(SFX_SPRITE_SURF_BLOOD1, (byte)(rnd * SFX_BLOOD_WIDTH), (byte)(rnd * SFX_BLOOD_HEIGHT),
										trace.ent, trace.endpos, trace.plane.normal);
				}
			}
			
			// Heal thy self (healspeed -1 = no heal, healspeed 0 = 1, healspeed >1 = value)
			if ((!ent->deadflag) && (ent->health < ent->max_health) && (ent->healspeed >= 0)
					&&	(!ent->leader || ent->cast_group != 1 || (((int)timescale->value) == 1))	// Ridah, added this or they can set timescale = 100 and followers will restore full health in second
					&&	(ent->healtimer++ > 30))
			// END JOSEPH
			{
				int i, i2, baseskin;
				
				ent->healtimer = 0;
				
				if (!ent->healspeed)
				  	ent->health += 1;
				else
					ent->health += ent->healspeed;
				
				if (ent->health > ent->max_health)
					ent->health = ent->max_health;
				
				for (i = 0; i < MAX_MODEL_PARTS; i++)
				{
					for (i2 = 0; i2 < MAX_MODELPART_OBJECTS; i2++)
					{
						baseskin = ent->s.model_parts[i].baseskin;
													
						if (ent->s.model_parts[i].skinnum[i2] > baseskin)
						{	
							if (ent->health > (ent->max_health * 0.75))
							{
								ent->s.model_parts[i].skinnum[i2] = baseskin;
							}
							else if (ent->health > (ent->max_health * 0.5))
							{
								if (cl_parental_lock->value && !cl_parental_override->value)
									ent->s.model_parts[i].skinnum[i2] = baseskin;
								else
									ent->s.model_parts[i].skinnum[i2] = baseskin + 1;
							}
						}
					}
				}
			}
		}
		// END JOSEPH

		// if the ground entity moved, make sure we are still on it
		if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;
			if ( !(ent->flags & (FL_SWIM|FL_FLY)) && (ent->svflags & SVF_MONSTER) )
			{
				M_CheckGround (ent);
			}
		}

		if (i > 0 && i <= maxclients->value)
		{
			ClientBeginServerFrame (ent);
		}
		else
		{
			G_RunEntity (ent);

			// Ridah, fast walking speed
			if (	(ent->cast_info.aiflags & AI_FASTWALK)
				&&	(ent->svflags & SVF_MONSTER)
				&&	(ent->cast_info.currentmove)
//				&&	(ent->cast_info.currentmove->frame->aifunc == ai_run)
				&&	(ent->cast_info.currentmove->frame->dist < 20)
				&&	(!ent->enemy))
			{
				G_RunEntity (ent);
				// JOSEPH 12-MAR-99
				if (ent->think) ent->think(ent);
				// END JOSEPH
			}
		}

		// Ridah, update lights if using directional lighting
		if (!(r_directional_lighting->value) && !deathmatch->value)
		{
			if (ent->s.renderfx2 & RF2_DIR_LIGHTS)
			{
				VectorSet( ent->s.last_lighting_update_pos, -9999, -9999, -9999 );
			}
		}
		else if (((ent->s.renderfx2 & RF2_DIR_LIGHTS) || (ent->client) || deathmatch->value))
		{
			if (!level.num_light_sources)	// no lights to source from, so default back to no dir lighting
			{
				ent->s.renderfx2 &= ~RF2_DIR_LIGHTS;
			}
			else
			{

				if (ent->client)
					ent->s.renderfx2 |= RF2_DIR_LIGHTS;

				// if single player, only calculate if it's visible to our player
				if (	(!VectorCompare(ent->s.last_lighting_update_pos, ent->s.origin))
					 &&	(	(ent->client && !deathmatch->value)
						||	(	(VectorDistance( ent->s.origin, ent->s.last_lighting_update_pos ) > (deathmatch->value ? 128 : 64))
							 &&	(	(deathmatch->value)
								 ||	(level.cut_scene_time)
								 ||	(	(gi.inPVS( g_edicts[1].s.origin, ent->s.origin))
									 &&	(infront( &g_edicts[1], ent ) ))))))
				{
					UpdateDirLights( ent );

					VectorCopy( ent->s.origin, ent->s.last_lighting_update_pos );
				}
				else if (showlights->value && gi.inPVS( g_edicts[1].s.origin, ent->s.origin))
				{
					UpdateDirLights( ent );
				}

			}
		}

	}

// Papa 10.6.99
// these is where the server checks the state of the current mode
// all the called functions are found in tourney.c

	if (level.modeset == FREEFORALL)
		CheckStartPub ();

	if (level.modeset == MATCHSETUP)
		CheckIdleMatchSetup ();

	if (level.modeset == FINALCOUNT)
		CheckStartMatch ();

	if ((level.modeset == STARTINGMATCH) || (level.modeset == STARTINGPUB))
		CheckAllPlayersSpawned ();

	if (level.modeset == MATCH)
		CheckEndMatch ();

	if (level.modeset == TEAMPLAY) 
		CheckDMRules ();

	if (level.modeset == ENDMATCHVOTING) 
		CheckEndVoteTime ();

	if (level.voteset != NO_VOTES)
		CheckVote();

	if ((int)timelimit->value && !(level.framenum%10)) {
		char buf[16];
		int t;
		if (level.modeset == FINALCOUNT) {
			t =	((150 - (level.framenum - level.startframe)) / 10);
			sprintf(buf,"start in %d",t);
			gi.cvar_set(TIMENAME,buf);
		} else if (level.modeset == FREEFORALL) {
			t = ((350 -  level.framenum ) / 10);
			sprintf(buf,"start in %d",t);
			gi.cvar_set(TIMENAME,buf);
		} else if ((level.modeset == MATCH) || (level.modeset == TEAMPLAY)) {
			t = ((((int)timelimit->value * 600) + level.startframe - level.framenum ) / 10);
			if (t>0) {
				sprintf(buf,"%d:%02d",t/60,t%60);
				gi.cvar_set(TIMENAME,buf);
			} else
				gi.cvar_set(TIMENAME,"");
		} else
			gi.cvar_set(TIMENAME,"");
	}

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();
}

