#include "g_local.h"

#define ENABLE_INDEX_NAMES		1

#define HEADERMESSAGE\
	"Running "GAMEVERSION,\
	"http://planetkingpin.com/kpl",

#define GAMEMODEMESSAGE\
	if ((int)teamplay->value == 0) {\
		if (dm_realmode->value)\
			Com_sprintf (temp, sizeof(temp), "Running Realmode Standard Deathmatch");\
		else\
			Com_sprintf (temp, sizeof(temp), "Running Standard Deathmatch");\
	} else if ((int)teamplay->value == 1) {\
		if (dm_realmode->value)\
			Com_sprintf (temp, sizeof(temp), "Running Realmode Bagman");\
		else\
			Com_sprintf (temp, sizeof(temp), "Running Team Assault v.024");\
	} else if ((int)teamplay->value == 4) {\
		if (dm_realmode->value)\
			Com_sprintf (temp, sizeof(temp), "Running Realmode Team Deathmatch");\
		else\
			Com_sprintf (temp, sizeof(temp), "Running Team Deathmatch");\
	}

#define SHOWCHASENAME\
	if (ent->client->chase_target && ent->client->chase_target->client)	{\
		ent->client->update_chase = false;\
		Com_sprintf(entry, sizeof(entry), "xm -80 yb -68 dmstr 773 \"Chasing %s\" xm -230 yb -40 dmstr 552 \".. use [ and ] to cycle, ACTIVATE to disable ..\" ",\
			ent->client->chase_target->client->pers.netname);\
		j = strlen(entry);\
		strcpy (string + stringlength, entry);\
		stringlength += j;\
	}

#define CHASEMESSAGE\
	if (!ent->client->chase_target) {\
		char	*chasecammsg = "[hit ACTIVATE to chase]";\
		Com_sprintf (entry, sizeof(entry),\
			"xm %i yb -50 dmstr 773 \"%s\" ",\
			-5*strlen(chasecammsg), chasecammsg );\
		j = strlen(entry);\
		strcpy (string + stringlength, entry);\
		stringlength += j;\
	}

#define ADDTHESTRING j = strlen(entry);	if (stringlength + j < 1024) { strcpy (string + stringlength, entry); stringlength += j; }
#define MYX 110 // assault2
#define motdx -5*strlen(MOTD[i].textline)+MYX // assault2

qboolean HasKey(edict_t *guy)
{
	if (guy->client->pers.inventory[ITEM_INDEX(FindItem("Key_One"))]) return true;
	if (guy->client->pers.inventory[ITEM_INDEX(FindItem("Key_Two"))]) return true;
	if (guy->client->pers.inventory[ITEM_INDEX(FindItem("Key_Three"))]) return true;

	return false;
}



/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->value || coop->value)
		ent->client->showscores = SCOREBOARD;
	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	// RAFAEL
	ent->client->quadfire_framenum = 0;
	
	// RAFAEL
	ent->client->trap_blew_up = false;
	ent->client->trap_time = 0;
	
	ent->viewheight = 0;
	ent->s.modelindex = 0;
//	ent->s.modelindex2 = 0;
//	ent->s.modelindex3 = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

//////////////////////////////////////////////////////// assault2
		//ent->movetype = MOVETYPE_NOCLIP;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->pers.spectator = SPECTATING;
//////////////////////////////////////////////////////// assault2

	// add the layout

	if (deathmatch->value || coop->value)
	{
		DeathmatchScoreboard (ent);
//		gi.unicast (ent, true);
	}

}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		if (client->health <= 0)
			respawn(client);
	}

	level.intermissiontime = level.time;
	
	level.changemap = targ->map;

	{
		edict_t *self;
		int		i;
		for_each_player (self,i)
		{
			if (self->client->pers.spectator) UPDATERATE(self)
		}
	}

	if (strstr(level.changemap, "*"))
	{
		//if (coop->value)
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_FLASHLIGHT)
						continue;

					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}

				client->episode_flags = client->client->pers.episode_flags = 0;
				client->client->pers.friends = 0;
				level.helpchange = 0;
			}
		}

		// Dan: this should eliminate the need to press a key to exit the end of episode
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

	level.exitintermission = 0;

	// find an intermission spot
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
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}
}
//===================================================================
//
// Papa - The following are the various scoreboards that I use 
//
//		phear id's confusing format - check qdevels on planetquake if
//		you don't understand this crap
//
//		one hint - dmstr xxx is the rgb color of the text :)
//
//===================================================================
//===================================================================

void SpectatorScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		yofs=0;
	edict_t	*player;
	char	name[32];
	char	*header = "NAME             ping"; 
	char	*selectheader[] =
		{
			"Spectators",
			NULL
		};

	string[0] = 0;
	stringlength = 0;

	for (i=0; selectheader[i]; i++)
	{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 999 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
	}
	yofs += 20;
	Com_sprintf (entry, sizeof(entry), "xv 27 yv %i dmstr 663 \"%s\" ", yofs + (int)(-60.0+-3.5*14), header );
	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	yofs += 20;

	for (i=0 ; i<maxclients->value ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse)
			continue;
		if (player->client->pers.spectator == SPECTATING)
		{
			if ((teamplay->value) && (player->client->pers.team != 0))
				continue;
			k =0;
			strcpy(name,"                ");
			while (player->client->pers.netname[k] != '\0')
			{
				name[k] = player->client->pers.netname[k];
				k++;
			}

			Com_sprintf (entry, sizeof(entry), "xv 27 yv %i dmstr 999 \"%s %4i\" ",yofs + (int)(-60.0+-3.5*14),
				name, player->client->ping);
			j = strlen(entry);
			if (stringlength + j < 1024)
			{
				strcpy (string + stringlength, entry);
				stringlength += j;
			}
			yofs += 20;
		}
	}

	if ((level.modeset != MATCHSETUP) && (level.modeset != FINALCOUNT) && (level.modeset != FREEFORALL)
		&& ent->client->pers.spectator == SPECTATING) {
		SHOWCHASENAME
		CHASEMESSAGE
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);

}

//===================================================================
//===================================================================
//===================================================================

void VoteMapScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	temp[32];
	char	string[1400];
	int		stringlength;
	int		i, j;
#if CHIEFGFX
	char    *tag;
	int     xpos, ypos;
#endif
	int		yofs;
	int		count[9];
	int		num_vote_set;
	edict_t *player;
	char	*choice = "Your choices are :";
	char	*endmes = "[F1] for scoreboard";
	char	*selectheader[] =
		{
			"End Level. Vote for next map.",
			"[ and ] or [weapon key] to Vote.",
			" ",
			" ",
			NULL
		};
	char	*basechoice[] =
		{
			"1 (Pipe)     ",
			"2 (Pistol)   ",
			"3 (Shotgun)  ",
			"4 (Tommygun) ",
			"5 (H.M.G.)   ",
			"6 (Grenade)  ",
			"7 (Bazooka)  ",
			"8 (Flamer)   ",
			NULL
		};
	int		hitter[MAX_CLIENTS];
	int		sortedhits[MAX_CLIENTS];
	int		k,hits, totalhits;
	edict_t *cl_entglad;
	string[0] = 0; stringlength = 0; totalhits = 0;

#if CHIEFGFX
	tag = PIC_KPLLOGO;
	xpos = -160;
	ypos = -130;
	Com_sprintf (entry, sizeof(entry),"xv %i yv %i picn %s ", xpos, ypos, tag);
	ADDTHESTRING
#endif
	if( (ent->vote > num_custom_maps) || (ent->vote < 0) ) ent->vote = 0;

	yofs = 20;
	for (i=0; selectheader[i]; i++)
	{ //752
		Com_sprintf (entry, sizeof(entry),"xm %i yv %i dmstr 373 \"%s\" ",
			-5*strlen(selectheader[i])+MYX, yofs + (int)(-60.0+-3.5*14), selectheader[i] );
		ADDTHESTRING
		yofs += 20;
	}

	if (num_custom_maps < 8 )
		num_vote_set = num_custom_maps;
	else
		num_vote_set = 8;

	memset (&count, 0, sizeof(count));
	for_each_player(player,i)
	{
		count[player->vote]++;
	}

	if (ent->vote == 0)
	{
		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 711 \" > \" xm %i yv %i dmstr 159 \"> >\" xm %i yv %i dmstr 991 \"    You gonna vote or what?\" xm %i yv %i dmstr 159 \"    You gonna vote or what?\" ",
				-5*40+MYX, yofs + (int)(-60.0+-3.5*14) ,
				-5*40+MYX, yofs + (int)(-60.0+-3.5*14) ,
				-5*40+MYX, yofs + (int)(-60.0+-3.5*14) ,
				-5*40+MYX+1, yofs + (int)(-60.0+-3.5*14)+1 				);

	}
	else
		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 555 \"    %d players didn't vote.\" ",
				-5*40+MYX, yofs + (int)(-60.0+-3.5*14), count[0]);

	ADDTHESTRING
	yofs += 30;
	//999
	Com_sprintf (entry, sizeof(entry),"xm %i yv %i dmstr 373 \"%s\" ",
		-5*32+MYX, yofs + (int)(-60.0+-3.5*14), choice );

	ADDTHESTRING

	yofs += 30;

	for (i=1; i < (num_vote_set+1); i++)
	{
		if (count[i] == 1)
			Com_sprintf (temp, sizeof(temp), "1 Vote  -");
		else
			Com_sprintf (temp, sizeof(temp), "%d Votes -",count[i]);


		if (ent->vote == i)
			Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 991 \"    %s %s %s\" xm %i yv %i dmstr 159 \"    %s %s %s\" ",
			-5*40+MYX, yofs + (int)(-60.0+-3.5*14), basechoice[i-1],temp,custom_list[vote_set[i]].custom_map, 
			(-5*40)+MYX+1, yofs + (int)(-60.0+-3.5*14)+1, basechoice[i-1],temp,custom_list[vote_set[i]].custom_map);
		else
			Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 555 \"    %s %s %s\" ",
					-5*40+MYX, yofs + (int)(-60.0+-3.5*14), basechoice[i-1],temp,custom_list[vote_set[i]].custom_map);
		ADDTHESTRING
		yofs += 20;
	}

	yofs += 30;
	Com_sprintf (entry, sizeof(entry),"xm %i yv %i dmstr 772 \"%s\" ",
		-5*strlen(endmes)+MYX, yofs + (int)(-60.0+-3.5*14), endmes );

		ADDTHESTRING

	// Send the current leader
	for (i=0 ; i<game.maxclients ; i++) //sort by hits
	{   cl_entglad = g_edicts + 1 + i; if (!cl_entglad->inuse) continue; //hits = game.clients[i].resp.score;
		hits = game.clients[i].resp.deposited + game.clients[i].resp.score;	// so it works in games without cash
		for (j=0 ; j<totalhits ; j++) { if (hits > sortedhits[j]) break; }
		for (k=totalhits ; k>j ; k--) {	hitter[k] = hitter[k-1]; sortedhits[k] = sortedhits[k-1]; }
		hitter[j] = i; sortedhits[j] = hits; totalhits++; }
	if (totalhits > 0) {	Com_sprintf (entry, sizeof(entry), "leader %i ", hitter[0] ); ADDTHESTRING }
	
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

//===================================================================
//===================================================================
//===================================================================

void MOTDScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	char	temp[128];
	int		stringlength;
	int		i, j;
#if CHIEFGFX
	int		xpos, ypos;
#endif
	int		yofs;
	int			found;
	edict_t		*player;
	char		*tag;
	char		*seperator = "==========================";
	char	*selectheader[] =
		{
			HEADERMESSAGE
			NULL
		};

	int		hitter[MAX_CLIENTS];
	int		sortedhits[MAX_CLIENTS];
	int		k,hits, totalhits;
	edict_t *cl_entglad;
	string[0] = 0; stringlength = 0; totalhits = 0;

#if CHIEFGFX
	tag = PIC_KPLLOGO;
	xpos = -160;
	ypos = -130;
	Com_sprintf (entry, sizeof(entry),"xv %i yv %i picn %s ", xpos, ypos, tag);
	ADDTHESTRING
#endif

	if((ent->client->chase_target && ent->client->chase_target->client)
	|| (ent->client->showscores == NO_SCOREBOARD)
	|| (!ent->client->showscores) ) goto skipscores;

	yofs = 80 - game.num_MOTD_lines * 10;
	if (yofs < 0 ) yofs = 0;

	for (i=0; selectheader[i]; i++)
	{
		if(i==1) tag = "999";
								
		else	 tag = "991";
		//752
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr %s \"%s\" ",
			-5*strlen(selectheader[i])+MYX, yofs + (int)(-60.0+-3.5*14),tag ,selectheader[i] );
		ADDTHESTRING
		yofs += 20;
	}
	yofs += 10;
	//772
	Com_sprintf (entry, sizeof(entry),"xm %i yv %i dmstr 444 \"%s\" ",-5*strlen(seperator)+MYX, yofs + (int)(-60.0+-3.5*14), seperator );
	ADDTHESTRING
	yofs += 30;

	for (i=0; i<game.num_MOTD_lines; i++)
	{
		//842
		if( i < 2 )
		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 999 \"%s\" ", motdx, yofs + (int)(-60.0+-3.5*14), MOTD[i].textline );

		else if( i >= 2 && i < 4 )
		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 696 \"%s\" ", motdx, yofs + (int)(-60.0+-3.5*14), MOTD[i].textline );

		else if( i >= 4 && i < 6 )
		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 393 \"%s\" ", motdx, yofs + (int)(-60.0+-3.5*14), MOTD[i].textline );

		else if( i >= 6 && i < 8 )
		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 696 \"%s\" ", motdx, yofs + (int)(-60.0+-3.5*14), MOTD[i].textline );

		else
		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 999 \"%s\" ", motdx, yofs + (int)(-60.0+-3.5*14), MOTD[i].textline );

		ADDTHESTRING

		yofs += 20;
	}
	yofs += 10;
	//772
	Com_sprintf (entry, sizeof(entry),"xm %i yv %i dmstr 444 \"%s\" ",-5*strlen(seperator)+MYX, yofs + (int)(-60.0+-3.5*14), seperator );
		ADDTHESTRING
	yofs += 30;

		GAMEMODEMESSAGE
//874
		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 991 \"%s\" ",
			-5*strlen(temp)+MYX, yofs + (int)(-60.0+-3.5*14), temp );

		yofs += 20;
		ADDTHESTRING

		if (level.modeset == FREEFORALL)
			if (teamplay->value)
				Com_sprintf (temp, sizeof(temp), "Please even the Teams!");
			else
				Com_sprintf (temp, sizeof(temp), "<< press [ or ] for Help >>");
		else if (level.modeset == MATCH)
			Com_sprintf (temp, sizeof(temp), "in Match Mode. (Please don't join).");
		else if (level.modeset == MATCHSETUP)
			Com_sprintf (temp, sizeof(temp), "in Match Setup Mode.");
		else if (level.modeset == FINALCOUNT)
			Com_sprintf (temp, sizeof(temp), "and is in the Final Countdown before a Match.");
		else if (level.modeset == TEAMPLAY)
			Com_sprintf (temp, sizeof(temp), "<< press [ or ] for Help >>");

//874
		Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 991 \"%s\" ",
			-5*strlen(temp)+MYX, yofs + (int)(-60.0+-3.5*14), temp );
		yofs += 20;
		ADDTHESTRING

		if (admincode[0] || !disable_admin_voting) {
			i=0;
			found = FALSE;
			while ((!found) && (i<maxclients->value)) 
			{
				player = g_edicts + 1 + i;
				i++;
				if (!player->inuse)
					continue;
				if (player->client->pers.admin > NOT_ADMIN)
					found = TRUE;
			}

			if (found)
				Com_sprintf (temp, sizeof(temp), "Your admin is %s",player->client->pers.netname);
			else
				Com_sprintf (temp, sizeof(temp), "No one currently has admin");
//874
			Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 373 \"%s\" ",
				-5*strlen(temp)+MYX, yofs + (int)(-60.0+-3.5*14), temp );
			ADDTHESTRING
		}



	// Send the current leader
	for (i=0 ; i<game.maxclients ; i++) //sort by hits
	{   cl_entglad = g_edicts + 1 + i; if (!cl_entglad->inuse) continue; //hits = game.clients[i].resp.score;
		hits = game.clients[i].resp.deposited + game.clients[i].resp.score;	// so it works in games without cash
		for (j=0 ; j<totalhits ; j++) { if (hits > sortedhits[j]) break; }
		for (k=totalhits ; k>j ; k--) {	hitter[k] = hitter[k-1]; sortedhits[k] = sortedhits[k-1]; }
		hitter[j] = i; sortedhits[j] = hits; totalhits++; }
	if (totalhits > 0) {	Com_sprintf (entry, sizeof(entry), "leader %i ", hitter[0] ); ADDTHESTRING }

skipscores:
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


//===================================================================
//===================================================================
//===================================================================


void RejoinScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j;
	int		yofs;
	char	*seperator = "==================================";
	char	*selectheader[] =
		{
/*			"Welcome to Reactive's Kingpin Competition Mod1.35!",
			"Found at : www.reactivesoftware.com",
			"Send comments to : papa@reactivesoftware.com",*/
			HEADERMESSAGE
			NULL
		};

	char	*rejoinheader[] =
		{
			"You were just playing on this server.",
			"Would you like to continue where you left off?",
			"Select the corresponding number.",
			NULL
		};
	char	*choices[] =
		{
			"1 - Yes",
			"2 - No",
			NULL
		};


	string[0] = 0;
	stringlength = 0;
	
	yofs = 0;

	for (i=0; selectheader[i]; i++)
	{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 752 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
	}
	yofs += 10;
	Com_sprintf (entry, sizeof(entry),
		"xm %i yv %i dmstr 772 \"%s\" ",
		-5*strlen(seperator), yofs + (int)(-60.0+-3.5*14), seperator );

	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;
	yofs += 30;

	for (i=0; rejoinheader[i]; i++)
	{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 842 \"%s\" ",
			-5*strlen(rejoinheader[i]), yofs + (int)(-60.0+-3.5*14), rejoinheader[i] );

		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
	}
	yofs += 30;
	for (i=0; choices[i]; i++)
	{
		if (ent->vote == i)
			Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 999 \"--> %s\" ",
					-5*40, yofs + (int)(-60.0+-3.5*14), choices[i]);
		else
			Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 777 \"    %s\" ",
					-5*40, yofs + (int)(-60.0+-3.5*14), choices[i]);

		j = strlen(entry);
		if (stringlength + j < 1024)
		{
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		yofs += 20;

	}


	gi.WriteByte (svc_layout);
	gi.WriteString (string);

}



/*
==================
GrabDaLootScoreboardMessage

==================
*/



void GrabDaLootScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total, real_total;
//	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;
	int		team;
	char *header;
	char	*headerb = "NAME         ping hits depst";// score";
	char	*headerd = "NAME         ping hits deaths";// score";
	char	nfill[64];
	int		yofs=0;
	int		teamcount[3];

	char	*selectheader[] =
		{
			"You are a Spectator!",
			"Press the corresponding number to join a team.",
			"Valid teams are:",
			NULL
		};


	header=teamplay->value==4?headerd:headerb;

//			"%12s - 1 (%i players)"\n%12s - 2\n\n--------------------------------------------------------\n", team_names[1], team_names[2]


	x = (-1*strlen(header) - 2) * 10;	// 10 pixels per char

	string[0] = 0;
	stringlength = 0;

	
	if (level.modeset == ENDMATCHVOTING)
	{
		Com_sprintf(entry, sizeof(entry), "xm -230 yb -40 dmstr 552 \"[Hit your scoreboard key (f1) for the vote menu]\" ");
			
		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;
	}
/*
	if (ent->client->chase_target && ent->client->chase_target->client)
	{
		ent->client->update_chase = false;
		Com_sprintf(entry, sizeof(entry), "xm -80 yb -68 dmstr 773 \"Chasing %s\" xm -230 yb -40 dmstr 552 \".. use [ and ] to cycle, ACTIVATE to disable ..\" ",
			ent->client->chase_target->client->pers.netname);
			
		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;
	}
*/
	SHOWCHASENAME

	if (!ent->client->showscores)
		goto skipscores;
		
	if (!ent->client->pers.team)
	{
		// count players per team
		memset( teamcount, 0, sizeof(int) * 3 );

		for (team=1; team<=2; team++)
		{
			for (i=0 ; i<game.maxclients ; i++)
			{
				cl_ent = g_edicts + 1 + i;
				if (!cl_ent->inuse)
					continue;

				if (game.clients[i].pers.team != team)
					continue;

				teamcount[team]++;
			}
		}

		// print the team selection header
		for (i=0; selectheader[i]; i++)
		{
			Com_sprintf (entry, sizeof(entry),
				"xm %i yv %i dmstr 999 \"%s\" ",
				-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

			j = strlen(entry);
			strcpy (string + stringlength, entry);
			stringlength += j;

			yofs += 30;
		}

		// show team counts
		for (team=1; team<=2; team++)
		{
			strcpy( nfill, team_names[team] );
			if (strlen(nfill) > 14)
				nfill[14] = '\0';

			if (strlen(team_names[team]) < 14)
			{
				for (k=0; k<14-strlen(team_names[team]); k++)
					strcat( nfill, " " );
			}

			Com_sprintf (entry, sizeof(entry),
				"xm %i yv %i dmstr 999 \"%i - %s (%i plyrs)\" ",
				-15*10, yofs + (int)(-60.0+-3.5*14), team, nfill, teamcount[team] );

			j = strlen(entry);
			strcpy (string + stringlength, entry);
			stringlength += j;

			yofs += 20;
		}

		yofs += 15;

		CHASEMESSAGE

	}

	for (team=1; team<=2; team++)
	{

		// Team header
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i tscore %i xm %i dmstr 677 \"%s\" teampic %i ",
			x+14*10, yofs + (int)(-60.0+-3.5*14), team, x, team_names[team], team );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		// sort the clients by score
		total = 0;
		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;

			if (game.clients[i].pers.team != team)
				continue;

//			score = (int)(1.0*game.clients[i].resp.score + (0.3*game.clients[i].resp.deposited));
//			score = game.clients[i].resp.deposited + game.clients[i].resp.score;	// so it works in games without cash
			if (teamplay->value==4) 
				score = (game.clients[i].resp.score<<8)-game.clients[i].resp.deposited;
			else
				score = (game.clients[i].resp.deposited<<4)+game.clients[i].resp.score;

			for (j=0 ; j<total ; j++)
			{
				if (score > sortedscores[j])
					break;
			}
			for (k=total ; k>j ; k--)
			{
				sorted[k] = sorted[k-1];
				sortedscores[k] = sortedscores[k-1];
			}
			sorted[j] = i;
			sortedscores[j] = score;
			total++;
		}

		real_total = total;

		// header
//#if ENABLE_INDEX_NAMES
		if (ent->client->pers.version >= 114)
		{

			// add the clients in sorted order
			if (ent->client->pers.team)
			{
				if (total > 10)
					total = 10;
			}
			else	// don't show as much, less space remaining
			{
				if (total > 7)
					total = 7;
			}

		}
//#else
		else
		{

			if (ent->client->pers.team)
			{
				if (total > 5)
					total = 5;
			}
			else	// don't show as much, less space remaining
			{
				if (total > 3)
					total = 3;
			}

		}
//#endif

		Com_sprintf (entry, sizeof(entry),
			"yv %i dmstr 663 \"%s\" ",
			yofs + (int)(-60.0+-1.5*14), header );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		for (i=0 ; i<total ; i++)
		{
			cl = &game.clients[sorted[i]];
			cl_ent = g_edicts + 1 + sorted[i];

	//		picnum = gi.imageindex ("i_fixme");
			y = 32 + 32 * i;

			if (i < 2) y -= 16;

			if ( HasKey(cl_ent) )		tag = "939";
			else if (cl_ent == ent)		tag = "990";
			else if (cl_ent == killer)	tag = "960";
			else
				tag = "999";	// fullbright

//#if ENABLE_INDEX_NAMES
			if (ent->client->pers.version >= 114)
			{
				Com_sprintf (entry, sizeof(entry),
					"yv %i ds %s %i %i %i %i ",// %5i\" ",					// ~28 chars
					yofs + -60+i*16, tag, sorted[i]/*nfill*/, cl->ping, cl->resp.score, cl->resp.deposited/*, sortedscores[i]*/ );

			}
//#else
			else
			{

				strcpy( nfill, cl->pers.netname );
				if (strlen(nfill) > 12)
					nfill[12] = '\0';

				if (strlen(cl->pers.netname) < 12)
				{
					for (k=0; k<12-strlen(cl->pers.netname); k++)
						strcat( nfill, " " );
				}

				Com_sprintf (entry, sizeof(entry),
					"yv %i dmstr %s \"%s %4i %4i %5i \" ",// %5i\" ",		// 48 chars
					yofs + -60+i*16, tag, nfill, cl->ping, cl->resp.score, cl->resp.deposited/*, sortedscores[i]*/ );

			}
//#endif

			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		if (real_total > total)
		{	// show the nuber of undisplayed players

			Com_sprintf (entry, sizeof(entry),
				"yv %i dmstr 777 \"(%i players)\" ",
				yofs + -60+i*16 + 6, real_total );

			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		x = -8;
	}

skipscores:
	
	// TODO: add the spectators at the bottom

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

//===================================================================
//===================================================================
//===================================================================



void MatchSetupScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	char	temp[64];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total, real_total;
//	int		picnum;
	int		x, y;
	int		found;
	gclient_t	*cl;
	edict_t		*cl_ent, *player;
	char	*tag;
	int		team;
	char *header;
	char	*headerb = "NAME         ping hits depst";// score";
	char	*headerd = "NAME         ping hits deaths";// score";
	char	nfill[64];
	int		yofs=10;
	int		teamcount[3];
	char	*selectheader[] =
		{
			"Valid teams are:",
			NULL
		};

	header=teamplay->value==4?headerd:headerb;

//			"%12s - 1 (%i players)"\n%12s - 2\n\n--------------------------------------------------------\n", team_names[1], team_names[2]


	x = (-1*strlen(header) - 2) * 10;	// 10 pixels per char

	string[0] = 0;
	stringlength = 0;
	
	if (!ent->client->showscores)
		goto skipscores;
		

	GAMEMODEMESSAGE

	Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 874 \"%s\" ",
		-5*strlen(temp), yofs + (int)(-60.0+-3.5*14), temp );
	yofs += 20;
	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;
		
	if (level.modeset == MATCHSETUP)
		Com_sprintf (temp, sizeof(temp), "in Match Setup Mode.");
	else if (level.modeset == FINALCOUNT)
		Com_sprintf (temp, sizeof(temp), "and is in the Final Countdown before a Match.");
	else if (level.modeset == FREEFORALL)
		if (teamplay->value)
			Com_sprintf (temp, sizeof(temp), "The Game will start soon. Please even the Teams!");
		else
			Com_sprintf (temp, sizeof(temp), "The Game will start soon.");

	Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 874 \"%s\" ",
		-5*strlen(temp), yofs + (int)(-60.0+-3.5*14), temp );
	yofs += 20;
	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;


	i=0;
	found = FALSE;
	while ((!found) && (i<maxclients->value)) 
	{
		player = g_edicts + 1 + i;
		i++;
		if (!player->inuse)
			continue;
		if (player->client->pers.admin > NOT_ADMIN)
			found = TRUE;
	}

	if (found)
		Com_sprintf (temp, sizeof(temp), "Your admin is %s",player->client->pers.netname);
	else
		Com_sprintf (temp, sizeof(temp), "No one currently has admin");

	Com_sprintf (entry, sizeof(entry), "xm %i yv %i dmstr 874 \"%s\" ",
		-5*strlen(temp), yofs + (int)(-60.0+-3.5*14), temp );
	yofs += 30;
	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	// count players per team
	memset( teamcount, 0, sizeof(int) * 3 );

	for (team=1; team<=2; team++)
	{
		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;

			if (game.clients[i].pers.team != team)
				continue;

			teamcount[team]++;
		}
	}

	// print the team selection header
	for (i=0; selectheader[i]; i++)
	{
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 999 \"%s\" ",
			-5*strlen(selectheader[i]), yofs + (int)(-60.0+-3.5*14), selectheader[i] );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 30;
	}

	// show team counts
	for (team=1; team<=2; team++)
	{
		strcpy( nfill, team_names[team] );
		if (strlen(nfill) > 14)
			nfill[14] = '\0';

		if (strlen(team_names[team]) < 14)
		{
			for (k=0; k<14-strlen(team_names[team]); k++)
				strcat( nfill, " " );
		}

		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i dmstr 999 \"%i - %s (%i plyrs)\" ",
			-15*10, yofs + (int)(-60.0+-3.5*14), team, nfill, teamcount[team] );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		yofs += 20;
	}

	yofs += 15;

	for (team=1; team<=2; team++)
	{

		// Team header
		Com_sprintf (entry, sizeof(entry),
			"xm %i yv %i tscore %i xm %i dmstr 677 \"%s\" teampic %i ",
			x+14*10, yofs + (int)(-60.0+-3.5*14), team, x, team_names[team], team );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		// sort the clients by score
		total = 0;
		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;

			if (game.clients[i].pers.team != team)
				continue;

//			score = (int)(1.0*game.clients[i].resp.score + (0.3*game.clients[i].resp.deposited));
//			score = game.clients[i].resp.deposited + game.clients[i].resp.score;	// so it works in games without cash
			if (teamplay->value==4) 
				score = (game.clients[i].resp.score<<8)-game.clients[i].resp.deposited;
			else
				score = (game.clients[i].resp.deposited<<4)+game.clients[i].resp.score;

			for (j=0 ; j<total ; j++)
			{
				if (score > sortedscores[j])
					break;
			}
			for (k=total ; k>j ; k--)
			{
				sorted[k] = sorted[k-1];
				sortedscores[k] = sortedscores[k-1];
			}
			sorted[j] = i;
			sortedscores[j] = score;
			total++;
		}

		real_total = total;

		// header
//#if ENABLE_INDEX_NAMES
		if (ent->client->pers.version >= 114)
		{

			// add the clients in sorted order
			if (ent->client->pers.team)
			{
				if (total > 10)
					total = 10;
			}
			else	// don't show as much, less space remaining
			{
				if (total > 7)
					total = 7;
			}

		}
//#else
		else
		{

			if (ent->client->pers.team)
			{
				if (total > 5)
					total = 5;
			}
			else	// don't show as much, less space remaining
			{
				if (total > 3)
					total = 3;
			}

		}
//#endif

		Com_sprintf (entry, sizeof(entry),
			"yv %i dmstr 663 \"%s\" ",
			yofs + (int)(-60.0+-1.5*14), header );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		for (i=0 ; i<total ; i++)
		{
			cl = &game.clients[sorted[i]];
			cl_ent = g_edicts + 1 + sorted[i];

	//		picnum = gi.imageindex ("i_fixme");
			y = 32 + 32 * i;

			if (i < 2)
				y -= 16;

			if (cl_ent == ent)
				tag = "990";
			else if (cl_ent == killer)
				tag = "960";
			else
				tag = "999";	// fullbright

//#if ENABLE_INDEX_NAMES
			if (ent->client->pers.version >= 114)
			{
				Com_sprintf (entry, sizeof(entry),
					"yv %i ds %s %i %i %i %i ",// %5i\" ",					// ~28 chars
					yofs + -60+i*16, tag, sorted[i]/*nfill*/, cl->ping, cl->resp.score, cl->resp.deposited/*, sortedscores[i]*/ );

			}
//#else
			else
			{

				strcpy( nfill, cl->pers.netname );
				if (strlen(nfill) > 12)
					nfill[12] = '\0';

				if (strlen(cl->pers.netname) < 12)
				{
					for (k=0; k<12-strlen(cl->pers.netname); k++)
						strcat( nfill, " " );
				}

				Com_sprintf (entry, sizeof(entry),
					"yv %i dmstr %s \"%s %4i %4i %5i \" ",// %5i\" ",		// 48 chars
					yofs + -60+i*16, tag, nfill, cl->ping, cl->resp.score, cl->resp.deposited/*, sortedscores[i]*/ );

			}
//#endif

			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		if (real_total > total)
		{	// show the nuber of undisplayed players

			Com_sprintf (entry, sizeof(entry),
				"yv %i dmstr 777 \"(%i players)\" ",
				yofs + -60+i*16 + 6, real_total );

			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		x = -8;
	}


skipscores:

	// TODO: add the spectators at the bottom

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	temp[64];
	char	entry[1024];
	char	string[1400]="";
	int		stringlength=0;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total, realtotal;
//	int		picnum;
	int		/*x,*/ y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;


	SHOWCHASENAME

	if (!ent->client->showscores)
		goto skipscores;
		
	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

		if ((cl_ent->client->pers.spectator == SPECTATING) && (level.modeset != ENDMATCHVOTING))
			continue;

		if (fph_scoreboard)
			score = game.clients[i].resp.score * 36000 / (level.framenum - game.clients[i].resp.enterframe);
		else
		{
			score = game.clients[i].resp.score;

			if (dm_realmode->value && !teamplay->value)
				score -= game.clients[i].resp.deposited;
		}
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
//	string[0] = 0;

//	stringlength = strlen(string);

	realtotal = total;

	Com_sprintf (temp, sizeof(temp), "current map : %s",level.mapname);
	Com_sprintf (entry, sizeof(entry), "xm %i yt 5 dmstr 752 \"%s\" ", -5*strlen(temp), temp);

	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

//#if ENABLE_INDEX_NAMES && false
	// add the clients in sorted order
	if (ent->client->pers.version >= 114)
	{
		if (total > 14)
			total = 14;
	}
//#else
	else
	{
		if (total > 7)
			total = 7;
	}
//#endif

	// Send the current leader
	if (total > 0)
	{
		Com_sprintf (entry, sizeof(entry),
			"leader %i ",
			sorted[0] );

		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	// header
//#if ENABLE_INDEX_NAMES && false
	if (ent->client->pers.version >= 114)
	{
		if (dm_realmode->value && !teamplay->value)
		{		// show deaths
			if (fph_scoreboard)
				Com_sprintf (entry, sizeof(entry),
					"xr %i yv %i dmstr 442 \"NAME         ping hits  fph\" ",
					-36*10 - 10, -60+-2*14 );
			else
				Com_sprintf (entry, sizeof(entry),
					"xr %i yv %i dmstr 442 \"NAME         ping hits score\" ",
					-36*10 - 10, -60+-2*14 );
		}
		else	// normal
		{
			if (fph_scoreboard)
				Com_sprintf (entry, sizeof(entry),
					"xr %i yv %i dmstr 442 \"NAME         ping hits   fph\" ",
					-36*10 - 10, -60+-2*14 );
			else
				Com_sprintf (entry, sizeof(entry),
					"xr %i yv %i dmstr 442 \"NAME         ping time  hits\" ",
					-36*10 - 10, -60+-2*14 );
		}
	}
//#else
	else
	{
		if (dm_realmode->value && !teamplay->value)
		{		// show deaths
			Com_sprintf (entry, sizeof(entry),
				"xr %i yv %i dmstr 442 \"            NAME   ping  hits  score\" ",
				-36*10 - 10, -60+-2*14 );
		}
		else	// normal
		{
			Com_sprintf (entry, sizeof(entry),
				"xr %i yv %i dmstr 442 \"            NAME   ping  time   hits\" ",
				-36*10 - 10, -60+-2*14 );
		}
	}
//#endif

	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

//		if ((cl->pers.spectator == SPECTATING) && (level.modeset != ENDMATCHVOTING))
//			continue;
//		picnum = gi.imageindex ("i_fixme");
		y = 32 + 32 * i;

		if (i < 2)
			y -= 16;

		if (cl_ent == ent)
			tag = "990";
		else if (cl_ent == killer)
			tag = "960";
		else
			tag = "999";	// fullbright

//#if ENABLE_INDEX_NAMES && false
		if (ent->client->pers.version >= 114)
		{
			if (dm_realmode->value && !teamplay->value)
			{		// show deaths
				Com_sprintf (entry, sizeof(entry),
					"yv %i ds %s %i %i %i %i ",
					-60+i*16, tag, sorted[i]/*cl->pers.netname*/, cl->ping, cl->resp.score, sortedscores[i] );
			}
			else
			{
				if (fph_scoreboard)
					Com_sprintf (entry, sizeof(entry),
						"yv %i ds %s %i %i %i %i ",
						-60+i*16, tag, sorted[i]/*cl->pers.netname*/, cl->ping, cl->resp.score, cl->resp.score * 36000 / (level.framenum - cl->resp.enterframe));
				else
					Com_sprintf (entry, sizeof(entry),
						"yv %i ds %s %i %i %i %i ",
						-60+i*16, tag, sorted[i]/*cl->pers.netname*/, cl->ping, (level.framenum - cl->resp.enterframe)/600, cl->resp.score );
			}
		}
//#else
		else
		{
			if (dm_realmode->value && !teamplay->value)
			{		// show deaths
				Com_sprintf (entry, sizeof(entry),
					"yv %i dmstr %s \"%16s   %4i   %3i   %4i\" ",
					-60+i*16, tag, cl->pers.netname, cl->ping, cl->resp.score, sortedscores[i] );
			}
			else
			{
				Com_sprintf (entry, sizeof(entry),
					"yv %i dmstr %s \"%16s   %4i   %3i   %4i\" ",
					-60+i*16, tag, cl->pers.netname, cl->ping, (level.framenum - cl->resp.enterframe)/600, cl->resp.score * 36000 / (level.framenum - cl->resp.enterframe) );
			}
		}
//#endif

		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	if ((stringlength + 30 < 1024) && realtotal > total)
	{
		Com_sprintf (entry, sizeof(entry),
			"yv %i dmstr 999 \" ( %i total players )\" ",
			-60+total*16, realtotal );

		j = strlen(entry);
		if (stringlength + j < 1024)
		{
			strcpy (string + stringlength, entry);
			stringlength += j;
		}
	}

	if (ent->client->pers.spectator == SPECTATING)
		CHASEMESSAGE

skipscores:
	
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/

// Papa - Here is where i determine what scoreboard to display

void DeathmatchScoreboard (edict_t *ent)
{
	if (ent->client->showscores == SCORE_MOTD)
		MOTDScoreboardMessage (ent);
	else if (ent->client->showscores == SCORE_REJOIN)
		RejoinScoreboardMessage (ent);
/*	else if (ent->client->showscores == SCOREBOARD)
		if (teamplay->value)
			if ((level.modeset == MATCHSETUP) || (level.modeset == FINALCOUNT) || (level.modeset == FREEFORALL))
				MatchSetupScoreboardMessage (ent, ent->enemy);
			else
				GrabDaLootScoreboardMessage (ent, ent->enemy);
		else	
			DeathmatchScoreboardMessage (ent, ent->enemy);	
*/	else if (ent->client->showscores == SPECTATORS)
		SpectatorScoreboardMessage (ent);
	else if (ent->client->showscores == SCORE_MAP_VOTE)
		VoteMapScoreboardMessage(ent);
	else {
		if (teamplay->value)
			if ((level.modeset == MATCHSETUP) || (level.modeset == FINALCOUNT) || (level.modeset == FREEFORALL))
				MatchSetupScoreboardMessage (ent, ent->enemy);
			else
				GrabDaLootScoreboardMessage (ent, ent->enemy);
		else	
			DeathmatchScoreboardMessage (ent, ent->enemy);	
	}


	if (level.intermissiontime)
		gi.unicast (ent, true);
	else
		gi.unicast (ent, false);//true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/

// Papa - This is the start of the scoreboard command, this sets the showscores value
  
void Cmd_Score_f (edict_t *ent)
{
	int		i,found;
	edict_t	*dood;

	ent->client->showinventory = false;
	ent->client->showhelp = false;

	if (!deathmatch->value && !coop->value)
		return;


	if (level.modeset == ENDMATCHVOTING)
		if (ent->client->showscores == SCORE_MAP_VOTE)
			ent->client->showscores = SCOREBOARD;
		else
			ent->client->showscores = SCORE_MAP_VOTE;

	else if (ent->client->showscores == SCORE_MOTD)
		ent->client->showscores = SCOREBOARD;
	else if (ent->client->showscores == SCOREBOARD)
	{
		found = false;
		for_each_player(dood,i)
		{
			if (dood->client->pers.spectator == SPECTATING)
				found = true;
		}
		if (found)
			ent->client->showscores = SPECTATORS;
		else
			ent->client->showscores = NO_SCOREBOARD;
	}
	else if (ent->client->showscores == SPECTATORS)
	{
		if (level.intermissiontime)
			ent->client->showscores = SCOREBOARD;
		else
			ent->client->showscores = NO_SCOREBOARD;
	}
	else
		ent->client->showscores = SCOREBOARD;
		
	DeathmatchScoreboard (ent);
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent, int page)
{
	char	string[1024];
	/*char	*sk;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 33 yv 7 picnote " // background		
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters, 
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);*/

	EP_PlayerLog (ent, page);

	level.helpchange = 0;
	
	Com_sprintf (string, sizeof(string),
		"picnote \"%s\" "
		" \"%s\" ",		
		game.helpmessage1,
		game.helpmessage2);

//	Com_sprintf (string, sizeof(string),
//		"xv 32 yv 8 string2 \"Help screen not yet implemented\" "	);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}

#if 0
/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";
/*
	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters, 
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);
*/
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 string2 \"Help screen not yet implemented\" "	);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
#endif

/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent, int page)
{
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = NO_SCOREBOARD;

	if (ent->client->showhelp && (ent->client->pers.game_helpchanged == game.helpchanged) && !(page))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->pers.helpchanged = 0;

	HelpComputer (ent, page);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

	// if chasecam, show stats of player we are following
	if (ent->client->chase_target && ent->client->chase_target->client)
	{
		memcpy( ent->client->ps.stats, ent->client->chase_target->client->ps.stats, sizeof( ent->client->ps.stats ) );
		ent->client->ps.stats[STAT_LAYOUTS] = true;
//	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (level.intermissiontime || ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	}
		
		// make sure they don't gain world ranking stats
		ent->client->ps.stats[STAT_FRAGS] = 0;
		ent->client->ps.stats[STAT_DEPOSITED] = 0;
		
		return;
	}
	
	//
	// health
	//
	// JOSEPH 23-MAR-99
	//ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	{	
		int     index1, index2, index3;
		
		item = FindItem ("Cash");
		index1 = ITEM_INDEX (item);
		item = FindItem ("Large Cash Bag");
		index2 = ITEM_INDEX (item);
		item = FindItem ("Small Cash Bag");
		index3 = ITEM_INDEX (item);
		
		if (!((ent->client->ps.stats[STAT_PICKUP_STRING] == CS_ITEMS+index1) ||
			  (ent->client->ps.stats[STAT_PICKUP_STRING] == CS_ITEMS+index2) ||
			  (ent->client->ps.stats[STAT_PICKUP_STRING] == CS_ITEMS+index3)))
			ent->client->ps.stats[STAT_CASH_PICKUP] = 0;

		// JOSEPH 1-APR-99-B
		item = FindItem ("Helmet Armor");
		ent->client->ps.stats[STAT_ARMOR1] = ent->client->pers.inventory[ITEM_INDEX(item)];
		item = FindItem ("Jacket Armor");
		ent->client->ps.stats[STAT_ARMOR2] = ent->client->pers.inventory[ITEM_INDEX(item)];
		item = FindItem ("Legs Armor");
		ent->client->ps.stats[STAT_ARMOR3] = ent->client->pers.inventory[ITEM_INDEX(item)];
		item = FindItem ("Helmet Armor Heavy");
		if (ent->client->pers.inventory[ITEM_INDEX(item)])
			ent->client->ps.stats[STAT_ARMOR1] = ent->client->pers.inventory[ITEM_INDEX(item)] + 1024;
		item = FindItem ("Jacket Armor heavy");
		if (ent->client->pers.inventory[ITEM_INDEX(item)])
			ent->client->ps.stats[STAT_ARMOR2] = ent->client->pers.inventory[ITEM_INDEX(item)] + 1024;
		item = FindItem ("Legs Armor Heavy");
		if (ent->client->pers.inventory[ITEM_INDEX(item)])
			ent->client->ps.stats[STAT_ARMOR3] = ent->client->pers.inventory[ITEM_INDEX(item)] + 1024;
		// END JOSEPH		
	}
	// END JOSEPH

	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//
	// ammo
	//
	
	// JOSEPH 28-APR-99
	if (!level.bar_lvl)
	{
	if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}

	// RAFAEL 01-11-99
	// JOSEPH 9-MAR-99
	if (ent->client->ammo_index)
	{
		item = &itemlist[ent->client->ammo_index];
	}
	else
	{
		item = NULL;
	}

	if ((item) && (item->pickup_name) && (/*(!strcmp(item->pickup_name, "Rockets")) ||*/ ((!strcmp(item->pickup_name, "Gas")))))
	{
		ent->client->ps.stats[STAT_CLIP] = -1;
	}
	else
	{
		// ent->client->ps.stats[STAT_CLIP_ICON] = gi.imageindex (clipname);
		ent->client->ps.stats[STAT_CLIP] = ent->client->pers.weapon_clip[ent->client->clip_index];
	}
	}
	else
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
		ent->client->ps.stats[STAT_CLIP] = -1;
	}
	// END JOSEPH
	
	//
	// money
	//

	// Ridah, 26-may-99, show frag count
	if (deathmatch->value && teamplay->value!=1)
		ent->client->ps.stats[STAT_CASH] = ent->client->resp.score;
	else	// show cash
		ent->client->ps.stats[STAT_CASH] = ent->client->pers.currentcash;

	if (level.pawn_time)
		ent->client->ps.stats[STAT_FORCE_HUD] = 1;
	else
		ent->client->ps.stats[STAT_FORCE_HUD] = 0;
	// END JOSEPH

	// JOSEPH 4-MAR-99
	if ((level.cut_scene_time) || (level.fadeendtime > level.time))
		ent->client->ps.stats[STAT_HIDE_HUD] = 1;
	else
		ent->client->ps.stats[STAT_HIDE_HUD] = 0;
	// END JOSEPH    

	if ((level.cut_scene_time || level.cut_scene_end_count) && level.cut_scene_camera_switch)
		ent->client->ps.stats[STAT_SWITCH_CAMERA] = 1;
	else
		ent->client->ps.stats[STAT_SWITCH_CAMERA] = 0;
	// END JOSEPH    
	
	// JOSEPH 2-FEB-99
	if (level.time > ent->client->hud_enemy_talk_time)
	{
		ent->client->ps.stats[STAT_HUD_ENEMY_TALK] = 0;
	}
	else if ((ent->client->hud_enemy_talk_time - level.time) > 1.0)
	{
		ent->client->ps.stats[STAT_HUD_ENEMY_TALK_TIME] = 255;		
	}
	else
	{
		ent->client->ps.stats[STAT_HUD_ENEMY_TALK_TIME] =
			(short)((255.0/1.0)*(ent->client->hud_enemy_talk_time - level.time));
		if (ent->client->ps.stats[STAT_HUD_ENEMY_TALK_TIME] < 0)
			ent->client->ps.stats[STAT_HUD_ENEMY_TALK_TIME] = 0;
	}
	
	if (level.time > ent->client->hud_self_talk_time)
	{
		ent->client->ps.stats[STAT_HUD_SELF_TALK] = 0;
	}
	else if ((ent->client->hud_self_talk_time - level.time) > 1.0)
	{
		ent->client->ps.stats[STAT_HUD_SELF_TALK_TIME] = 255;		
	}
	else
	{
		ent->client->ps.stats[STAT_HUD_SELF_TALK_TIME] =
			(short)((255.0/1.0)*(ent->client->hud_self_talk_time - level.time));
		if (ent->client->ps.stats[STAT_HUD_SELF_TALK_TIME] < 0)
			ent->client->ps.stats[STAT_HUD_SELF_TALK_TIME] = 0;
	}
	// END JOSEPH	

	// JOSEPH 16-FEB-99
	{
		cast_memory_t *mem;
		int firsthire = 0;
		int i;
		edict_t *theent;

		ent->client->ps.stats[STAT_HUD_HIRE1] = 0;
		ent->client->ps.stats[STAT_HUD_HIRE2] = 0;

		theent = &g_edicts[0];
		for (i=0 ; i<globals.num_edicts ; i++, theent++)
		{
			if (!theent->inuse)
				continue;

			if ((theent->leader) && (theent->leader == ent))
			{
				mem = level.global_cast_memory[theent->character_index][ent->character_index];

				if (!mem)
					continue;

				if (mem->flags & MEMORY_HIRED)
				{   
					if (!firsthire)
					{
						if (theent->health > 0)
						{
							// JOSEPH 28-APR-99
							ent->client->ps.stats[STAT_HUD_HIRE1] = (int)((100.0/(float)theent->max_health)*((float)theent->health));
							// END JOSEPH
							
							if ((theent->enemy) && (theent->enemy->health > 0))
							{
								ent->client->ps.stats[STAT_HUD_HIRE1_CMD] = 3;
							}
							else if (theent->cast_info.aiflags & AI_MOVEOUT || theent->cast_info.aiflags & AI_DOKEY)
							{
								ent->client->ps.stats[STAT_HUD_HIRE1_CMD] = 1;
							}
							else if (theent->cast_info.aiflags & AI_HOLD_POSITION)
							{
								ent->client->ps.stats[STAT_HUD_HIRE1_CMD] = 2;
							}
							else
							{
								ent->client->ps.stats[STAT_HUD_HIRE1_CMD] = 0;
							}
								
							firsthire = 1;
						}
					}
					else
					{
						if (theent->health > 0)
						{
							// JOSEPH 28-APR-99
							ent->client->ps.stats[STAT_HUD_HIRE2] = (int)((100.0/(float)theent->max_health)*((float)theent->health));
							// END JOSEPH
						
							if ((theent->enemy) && (theent->enemy->health > 0))
							{
								ent->client->ps.stats[STAT_HUD_HIRE1_CMD] = 3;
							}
							else if (theent->cast_info.aiflags & AI_MOVEOUT || theent->cast_info.aiflags & AI_DOKEY)
							{
								ent->client->ps.stats[STAT_HUD_HIRE2_CMD] = 1;
							}
							else if (theent->cast_info.aiflags & AI_HOLD_POSITION)
							{
								ent->client->ps.stats[STAT_HUD_HIRE2_CMD] = 2;
							}
							else
							{
								ent->client->ps.stats[STAT_HUD_HIRE2_CMD] = 0;
							}						
						}
						break;
					}
				}
			}
		}
	}
	// END JOSEPH

	// JOSEPH 4-FEB-99-C
	ent->client->ps.stats[STAT_HUD_INV] = 0;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Battery"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 1;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Coil"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 2;

	// JOSEPH 17-MAR-99
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Watch"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 4;
	// END JOSEPH

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Safe docs"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 8;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Fuse"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 16;
	
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Valve"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 32;
	// END JOSEPH

	// JOSEPH 10-JUN-99
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Lizzy Head"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 64;
	
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Whiskey"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 128;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Oil Can"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 256;	

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Ticket"))])
		ent->client->ps.stats[STAT_HUD_INV] |= 512;		
	// END JOSEPH

	//
	// armor
	//
	power_armor_type = PowerArmorType (ent);
	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("Gas"))];
		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}

	index = ArmorIndex (ent);
	
	// JOSEPH 1-APR-99
	/*if (power_armor_type && (!index || (level.framenum & 8) ) )
	{	// flash between power armor and other armor icon
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
		ent->client->ps.stats[STAT_ARMOR] = cells;
	}
	else if (index)
	{
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		// ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}*/
	// END JOSEPH

	//
	// pickup message
	//
	// JOSEPH 25-JAN-99
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
		ent->client->ps.stats[STAT_PICKUP_COUNT] = 0;	
	}
	else if ((ent->client->pickup_msg_time - level.time) > 1.5)
	{
		ent->client->ps.stats[STAT_PICKUP_COUNT] = 255;		
	}
	else
	{
		ent->client->ps.stats[STAT_PICKUP_COUNT] =
			(short)((255.0/1.5)*(ent->client->pickup_msg_time - level.time));
		if (ent->client->ps.stats[STAT_PICKUP_COUNT] < 0)
			ent->client->ps.stats[STAT_PICKUP_COUNT] = 0;
	}
	// END JOSEPH

	//
	// timers
	//
	// JOSEPH 12-MAY-99-B
	/*
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
	}
	// RAFAEL
	else if (ent->client->quadfire_framenum > level.framenum)
	{
		// note to self
		// need to change imageindex
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quadfire");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quadfire_framenum - level.framenum)/10;
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}	
	*/

// Papa - Here is the Timer for the hud

	if ((int)timelimit->value)
	{
		if (level.modeset == FREEFORALL)
			ent->client->ps.stats[STAT_TIMER] = ((350 -  level.framenum ) / 10);

//	else if ((level.modeset == FREEFORALL) && (timelimit->value))
//		if (level.framenum > ((timelimit->value * 600) - 605))  
//			ent->client->ps.stats[STAT_TIMER] = (((timelimit->value * 600) - level.framenum ) / 10);
//		else
//			ent->client->ps.stats[STAT_TIMER] = (((timelimit->value * 600) - level.framenum ) / 600);
//	else if ((level.modeset == TEAMPLAY) && (timelimit->value))
//		if (level.framenum > ((timelimit->value * 600) - 605))  
//			ent->client->ps.stats[STAT_TIMER] = (((timelimit->value * 600) - level.framenum ) / 10);
//		else
//			ent->client->ps.stats[STAT_TIMER] = (((timelimit->value * 600) - level.framenum ) / 600);

		else if (level.modeset == FINALCOUNT)
			ent->client->ps.stats[STAT_TIMER] =	((150 - (level.framenum - level.startframe)) / 10);

		else if (level.modeset == ENDMATCHVOTING)
			ent->client->ps.stats[STAT_TIMER] =	((300 - (level.framenum - level.startframe)) / 10);

		else if ((level.modeset == MATCH) || (level.modeset == TEAMPLAY))
			if (level.framenum > (level.startframe + (((int)timelimit->value  * 600) - 605)))  
				ent->client->ps.stats[STAT_TIMER] = ((((int)timelimit->value * 600) + level.startframe - level.framenum ) / 10);
			else
				ent->client->ps.stats[STAT_TIMER] = ((((int)timelimit->value * 600) + level.startframe - level.framenum ) / 600);
		else 
			ent->client->ps.stats[STAT_TIMER] = 0;

		if (ent->client->ps.stats[STAT_TIMER] < 0 )
			ent->client->ps.stats[STAT_TIMER] = 0;
	}
	else
		ent->client->ps.stats[STAT_TIMER] = 0;
	
	// END JOSEPH

	//
	// selected item
	//
	// JOSEPH 1-APR-99
	/*if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);
	*/
	// END JOSEPH
	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;
	ent->client->ps.stats[STAT_DEPOSITED] = ent->client->resp.deposited;

	//
	// help icon / current weapon if not shown
	//
	if (ent->client->resp.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	// JOSEPH 14-JUN-99
	ent->client->ps.stats[STAT_HELPICON] = level.helpchange;
	// END JOSEPH

	// Teamplay
	if (deathmatch->value && teamplay->value)
	{
		int i;

		// show team scores
		for (i=0; i<2; i++)
		{
			ent->client->ps.stats[STAT_TEAM1_SCORE + i] = team_cash[1+i];	// set score

			ent->client->ps.stats[STAT_TEAM1_FLASH + i] = 0;	// set normal

			if (last_safe_withdrawal[i+1] > last_safe_deposit[i+1])
			{
				if (last_safe_withdrawal[i+1] > (level.time - 3.0))
					ent->client->ps.stats[STAT_TEAM1_FLASH + i] = 2;	// flash red
			}
			else
			{
				if (last_safe_deposit[i+1] > (level.time - 3.0))
					ent->client->ps.stats[STAT_TEAM1_FLASH + i] = 1;	// flash green
			}
		}

		// show bagged cash
		ent->client->ps.stats[STAT_BAGCASH] = ent->client->pers.bagcash;

	}
}

// JOSEPH 16-DEC-98

// RAFAEL
/*void BeginCutScene (edict_t *ent)
{
	edict_t *player;

	player = g_edicts + 1;

	level.cut_scene_time = level.time + 0.1;
	
	VectorCopy (ent->s.angles, level.cut_scene_angle);
	VectorCopy (ent->s.origin, level.cut_scene_origin);
	
	VectorCopy (player->s.origin, level.player_oldpos);
	VectorCopy (player->s.angles, level.player_oldang);

	MoveClientToCutScene (player);

}*/

void FollowEnt (edict_t *ent)
{


    if (ent->target2_ent)
	{
//    	vec3_t	forward, forward2;
		vec3_t	avect;

// Ridah, not sure how this is supposed to work, doesn't make sense to do an AngleVectors() call on an origin
//		AngleVectors (ent->s.origin, forward, NULL, NULL);
//	    AngleVectors (ent->target2_ent->s.origin, forward2, NULL, NULL);
//		VectorSubtract(forward, forward2, avect);

		VectorSubtract(ent->target2_ent->s.origin, ent->s.origin, avect);

		VectorNormalize(avect);
		vectoangles(avect, ent->s.angles);	
	}
}

// JOSEPH 23-FEB-99
void BeginCutScene (edict_t *ent)
{
	edict_t *player;

	player = g_edicts + 1;

    level.cut_scene_camera_switch = 1;

	level.cut_scene_time = level.time + 0.1;
	
	VectorCopy(ent->s.angles, ent->save_avel);
	VectorCopy(ent->s.origin, ent->savecameraorigin);
	
	if (ent->cameraangle[0])
	{
		ent->s.angles[0] += ent->cameraangle[0];
	}

	if (ent->cameraangle[1])
	{
		ent->s.angles[1] += ent->cameraangle[1];
	}

	if (ent->cameraangle[2])
	{
		ent->s.angles[2] += ent->cameraangle[2];
	}

    FollowEnt (ent);

	VectorCopy (ent->s.angles, level.cut_scene_angle);
	VectorCopy (ent->s.origin, level.cut_scene_origin);
	
	VectorCopy (player->s.origin, level.player_oldpos);
	VectorCopy (player->s.angles, level.player_oldang);
	
	MoveClientToCutSceneCamera (player, ent->deadticks);
}

void NewCutSceneCamera (edict_t *ent)
{
	edict_t *player;

	player = g_edicts + 1;

	level.cut_scene_camera_switch = 1;

	VectorCopy(ent->s.angles, ent->save_avel);
	VectorCopy(ent->s.origin, ent->savecameraorigin);
	
	level.cut_scene_time = level.time + 0.1;

	// JOSEPH 19-MAR-99-B
	ent->alphalevel = level.time + ent->reactdelay;	
	// END JOSEPH

	if (ent->cameraangle[0])
	{
		ent->s.angles[0] += ent->cameraangle[0];
	}

	if (ent->cameraangle[1])
	{
		ent->s.angles[1] += ent->cameraangle[1];
	}

	if (ent->cameraangle[2])
	{
		ent->s.angles[2] += ent->cameraangle[2];
	}

    FollowEnt (ent);
	
	VectorCopy (ent->s.angles, level.cut_scene_angle);
	VectorCopy (ent->s.origin, level.cut_scene_origin);
	
	MoveClientToCutSceneCamera (player, ent->deadticks);
}

void AdjustCutSceneCamera(edict_t *ent)
{
	edict_t *player;

	player = g_edicts + 1;

	if (ent->accel)
	{
		if (ent->delay < (level.time - ent->timestamp))
		{
			if ((ent->delay + ent->reactdelay) < (level.time - ent->timestamp))
			{
				if (ent->speed > 0)
				{
					ent->speed -= ent->decel * FRAMETIME;

					if (ent->speed < 0)
						ent->speed = 0;
				}
			}
			else if (ent->speed < 1)
			{
				ent->speed += ent->accel * FRAMETIME;
				if (ent->speed > 1)
					ent->speed = 1;
			}
		}
	}

    level.cut_scene_camera_switch = 0;	

	level.cut_scene_time = level.time + 0.1;

	if (ent->rotate[0])
	{
		ent->s.angles[0] += ent->rotate[0]*FRAMETIME*ent->speed;
	}

	if (ent->rotate[1])
	{
		ent->s.angles[1] += ent->rotate[1]*FRAMETIME*ent->speed;
	}

	if (ent->rotate[2])
	{
		ent->s.angles[2] += ent->rotate[2]*FRAMETIME*ent->speed;
	}

	// Ridah, not sure what ->save_avel is, but we need a way of setting an absolute velocity
	VectorMA(ent->s.origin, FRAMETIME*ent->speed, ent->cameravel, ent->s.origin);
/*
	if (ent->cameravel[0])
	{
    	vec3_t	forward;

	    AngleVectors (ent->save_avel, forward, NULL, NULL);
		VectorMA(ent->s.origin, ent->cameravel[0]*FRAMETIME, forward, ent->s.origin);
	}
	
	if (ent->cameravel[1])
	{
    	vec3_t	right;

	    AngleVectors (ent->save_avel, NULL, right, NULL);
		VectorMA(ent->s.origin, ent->cameravel[1]*FRAMETIME, right, ent->s.origin);
	}
	
	if (ent->cameravel[2])
	{
    	vec3_t	up;

	    AngleVectors (ent->save_avel, NULL, NULL, up);
		VectorMA(ent->s.origin, ent->cameravel[2]*FRAMETIME, up, ent->s.origin);
	}	
*/
	if (ent->cameravelrel[0])
	{
    	vec3_t	forward;

	    AngleVectors (ent->s.angles, forward, NULL, NULL);
		VectorMA(ent->s.origin, ent->cameravelrel[0]*FRAMETIME*ent->speed, forward, ent->s.origin);
	}
	
	if (ent->cameravelrel[1])
	{
    	vec3_t	right;

	    AngleVectors (ent->s.angles, NULL, right, NULL);
		VectorMA(ent->s.origin, ent->cameravelrel[1]*FRAMETIME*ent->speed, right, ent->s.origin);
	}
	
	if (ent->cameravelrel[2])
	{
    	vec3_t	up;

	    AngleVectors (ent->s.angles, NULL, NULL, up);
		VectorMA(ent->s.origin, ent->cameravelrel[2]*FRAMETIME*ent->speed, up, ent->s.origin);
	}	

    FollowEnt (ent);
 	
	VectorCopy (ent->s.angles, level.cut_scene_angle);
	VectorCopy (ent->s.origin, level.cut_scene_origin);

    //VectorCopy (ent->s.origin, ent->origin);	
    //VectorCopy (ent->s.origin, ent->oldorigin);	
	//VectorCopy (player->s.origin, level.player_oldpos);
	//VectorCopy (player->s.angles, level.player_oldang);

	// JOSEPH 19-MAR-99-B
	// Fade out
	if (ent->duration)
	{
		if ((level.time + ent->duration) > ent->alphalevel) 	
		{
			level.totalfade = ent->duration;
			level.fadeendtime = level.time + level.totalfade;		
			level.inversefade = 1;
			ent->duration = 0;
		}
	}
    // END JOSEPH

    MoveClientToCutSceneCamera (player, ent->deadticks);
}

void EndCutScene (edict_t *ent)
{
	edict_t *player;

	player = g_edicts + 1;

    level.cut_scene_camera_switch = 0;

    player->client->ps.fov = 90;
	
	VectorCopy (level.player_oldpos, player->s.origin);
	VectorCopy (level.player_oldang, player->s.angles);
	
	// JOSEPH 24-FEB-99
    level.cut_scene_camera_switch = 1;
	level.cut_scene_end_count = 5;
	// END JOSEPH	

	level.cut_scene_time = 0;
	
	player->client->ps.pmove.origin[0] = player->s.origin[0]*8;
	player->client->ps.pmove.origin[1] = player->s.origin[1]*8;
	player->client->ps.pmove.origin[2] = player->s.origin[2]*8;
	
	VectorCopy (level.player_oldang, player->client->ps.viewangles);


	if (!player->client->pers.weapon)
		return;
	
	{
		int i;
	
		
		if (!strcmp (player->client->pers.weapon->pickup_name , "SPistol"))
		{
			if ((player->client->pers.pistol_mods) && (player->client->pers.pistol_mods & WEAPON_MOD_DAMAGE))
				player->client->ps.gunindex = gi.modelindex("models/weapons/silencer_mdx/magnum.mdx");
			else
				player->client->ps.gunindex = gi.modelindex(player->client->pers.weapon->view_model);
		}
		else if (!strcmp (player->client->pers.weapon->pickup_name , "Pistol"))
		{
			if ((player->client->pers.pistol_mods) && (player->client->pers.pistol_mods & WEAPON_MOD_DAMAGE))
				player->client->ps.gunindex = gi.modelindex("models/weapons/v_colt/magnum.mdx");
			else
				player->client->ps.gunindex = gi.modelindex(player->client->pers.weapon->view_model);
		}
		else if (!strcmp (player->client->pers.weapon->pickup_name, "Heavy machinegun"))
		{
			if (player->client->pers.pistol_mods & WEAPON_MOD_COOLING_JACKET)
				player->client->ps.gunindex = gi.modelindex("models/weapons/v_hmgcool/hmgcool.mdx");
//				player->client->ps.gunindex = gi.modelindex("models/weapons/v_msg90/msg90.mdx");//assault2
			else
				player->client->ps.gunindex = gi.modelindex(player->client->pers.weapon->view_model);
		}
		else
			player->client->ps.gunindex = gi.modelindex(player->client->pers.weapon->view_model);


		memset(&(player->client->ps.model_parts[0]), 0, sizeof(model_part_t) * MAX_MODEL_PARTS);

		player->client->ps.num_parts++;
		// player->client->ps.model_parts[PART_HEAD].modelindex = gi.modelindex(player->client->pers.weapon->view_model);
		player->client->ps.model_parts[PART_HEAD].modelindex = player->client->ps.gunindex;
		for (i=0; i<MAX_MODELPART_OBJECTS; i++)
			player->client->ps.model_parts[PART_HEAD].skinnum[i] = 0; // will we have more than one skin???
	
		// HACK, set arm model
		if (!strcmp(player->client->pers.weapon->pickup_name, "Shotgun"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/shotgun/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/shotgun/shell.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???
		}
		else if (!strcmp(player->client->pers.weapon->pickup_name, "Pipe"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/blakjak/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		}
		else if (!strcmp(player->client->pers.weapon->pickup_name, "Crowbar"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/crowbar/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		}
		else if (!strcmp(player->client->pers.weapon->pickup_name, "FlameThrower"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_flamegun/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		}
		// JOSEPH 4-MAR-99
		else if (!strcmp(player->client->pers.weapon->pickup_name, "Bazooka"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_rocket_launcher/hand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
		
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_rocket_launcher/shell_a.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???		

			
			/*
			// note to self: not using this anymore
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_GUN].modelindex = gi.modelindex("models/weapons/v_rocket_launcher/shell_b.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_GUN].skinnum[i] = 0; // will we have more than one skin???		
			*/

			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_GUN2].modelindex = gi.modelindex("models/weapons/v_rocket_launcher/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_GUN2].skinnum[i] = 0; // will we have more than one skin???		

			
		}
		// END JOSEPH
		else if (!strcmp (player->client->pers.weapon->pickup_name , "SPistol"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/silencer_mdx/handl.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
			
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/silencer_mdx/handr.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???

			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_GUN].modelindex = gi.modelindex("models/weapons/silencer_mdx/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_GUN].skinnum[i] = 0;

		}
		else if (!strcmp (player->client->pers.weapon->pickup_name , "Pistol"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_colt/handl.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
			
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_colt/handr.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???

			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_GUN].modelindex = gi.modelindex("models/weapons/v_colt/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_GUN].skinnum[i] = 0;
		}
		// JOSEPH 16-APR-99
		else if (!strcmp (player->client->pers.weapon->pickup_name , "Heavy machinegun"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_hmg/lhand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???

			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_hmg/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???
			
	}
		// END JOSEPH
		else if (!strcmp (player->client->pers.weapon->pickup_name , "Grenade Launcher"))
		{
			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_LEGS].modelindex = gi.modelindex("models/weapons/v_grenade_launcher/lhand.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_LEGS].skinnum[i] = 0; // will we have more than one skin???

			player->client->ps.num_parts++;
			player->client->ps.model_parts[PART_BODY].modelindex = gi.modelindex("models/weapons/v_grenade_launcher/clip.mdx");
			for (i=0; i<MAX_MODELPART_OBJECTS; i++)
				player->client->ps.model_parts[PART_BODY].skinnum[i] = 0; // will we have more than one skin???

		}

	}

// 	player->client->ps.gunindex = gi.modelindex(player->client->pers.weapon->view_model);
}
// END JOSEPH

void MoveClientToCutScene (edict_t *ent)
{
	VectorCopy (level.cut_scene_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.cut_scene_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.cut_scene_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.cut_scene_origin[2]*8;
	VectorCopy (level.cut_scene_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
		
	// note to self
	// this may cause a problem
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;
			
	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	// RAFAEL
	ent->client->quadfire_framenum = 0;

	// RAFAEL
	ent->client->trap_blew_up = false;
	ent->client->trap_time = 0;
	
	/*
	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;
	*/

}

void MoveClientToCutSceneCamera (edict_t *ent, int fov)
{
	ent->client->ps.fov = fov;
	
	VectorCopy (level.cut_scene_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.cut_scene_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.cut_scene_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.cut_scene_origin[2]*8;
	VectorCopy (level.cut_scene_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	
	// note to self
	// this may cause a problem
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	// RAFAEL
	ent->client->quadfire_framenum = 0;
	
	// RAFAEL
	ent->client->trap_blew_up = false;
	ent->client->trap_time = 0;
	
	/*
	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;
	*/

}
// END JOSEPH

void MoveClientToPawnoMatic (edict_t *ent)
{
// RAFAEL 03-14-99
// Drew will compensate by moveing the start pos	
//	ent->client->ps.fov = 96;
	
// 	ent->s.origin[2] = level.pawn_origin[2];
	ent->client->ps.pmove.origin[0] = level.cut_scene_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.cut_scene_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.cut_scene_origin[2]*8;
	
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	// RAFAEL
	ent->client->quadfire_framenum = 0;
	
	// RAFAEL
	ent->client->trap_blew_up = false;
	ent->client->trap_time = 0;
	

}
