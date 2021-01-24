#include "g_local.h"
#include "file.h"
#include <stdio.h>


void fgetline (FILE* infile, char* buffer)
{
	int		i=0;
	char	ch;

	ch = fgetc(infile);

	while ( (ch != EOF) && (ch != '\n') && (i < (MAX_STRING_LENGTH-1)) )
	{
		buffer[i++] = ch;
		ch = fgetc(infile);
	}

	if ( (ch != '\n') && (ch != EOF) )	// Keep going to the eof
		ch = fgetc(infile);

	buffer[i] = '\0';
	
	return;

}


int proccess_line(char*	buffer)
{
	// Make sure the line's not empty
	if (strlen(buffer) == 0 || buffer[0] == '\n') return COMMENT_LINE;

	// Check to see if this is a comment line
	if (buffer[0] == '/' && buffer[1] == '/')	return COMMENT_LINE;

	// Check if this is the custom map keyword
	if (strstr(buffer, ADMIN_CODE_STRING)) return ADMIN_CODE_KEYWORD;

	// Check if this is the custom map keyword
	if (strstr(buffer, CUSTOM_MAP_STRING))
		return CUSTOM_MAP_KEYWORD;

	// Check if this is the map rotation keyword
	if (strstr(buffer, MAP_ROTATION_STRING))
		return MAP_ROTATION_KEYWORD;

	// Check if this is the default rotation keyword
	if (strstr(buffer, DEFAULT_MAP_STRING))
		return DEFAULT_MAP_KEYWORD;

	if (strstr(buffer, DEFAULT_TEAMPLAY_STRING))
		return DEFAULT_TEAMPLAY_KEYWORD;

	if (strstr(buffer, DEFAULT_DMFLAGS_STRING))
		return DEFAULT_DMFLAGS_KEYWORD;

	if (strstr(buffer, DEFAULT_PASSWORD_STRING))
		return DEFAULT_PASSWORD_KEYWORD;

	if (strstr(buffer, DEFAULT_MOTD_STRING))
		return DEFAULT_MOTD_KEYWORD;

	if (strstr(buffer, DEFAULT_TIME_STRING))
		return DEFAULT_TIME_KEYWORD;

	if (strstr(buffer, DEFAULT_FRAG_STRING))
		return DEFAULT_FRAG_KEYWORD;

	if (strstr(buffer, DEFAULT_CASH_STRING))
		return DEFAULT_CASH_KEYWORD;

	if (strstr(buffer, ALLOW_MAP_VOTING_STRING))
		return MAP_VOTING_KEYWORD;

	if (strstr(buffer, BAN_NAME_STRING))
		return BAN_NAME_KEYWORD;

	if (strstr(buffer, BAN_IP_STRING))
		return BAN_IP_KEYWORD;

	if (strstr(buffer, SCOREBOARD_FIRST_STRING))
		return SCOREBOARD_FIRST_KEYWORD;

	if (strstr(buffer, FPH_SCOREBOARD_STRING))
		return FPH_SCOREBOARD_KEYWORD;

	if (strstr(buffer, DISABLE_ADMIN_STRING))
		return DISABLE_ADMIN_KEYWORD;

	if (strstr(buffer, DEFAULT_REAL_STRING))
		return DEFAULT_REAL_KEYWORD;

	if (strstr(buffer, FIXED_GAMETYPE_STRING))
		return FIXED_GAMETYPE_KEYWORD;

	if (strstr(buffer, ENABLE_PASSWORD_STRING))
		return ENABLE_PASSWORD_KEYWORD;

	if (strstr(buffer, RCONX_FILE_STRING))
		return RCONX_FILE_KEYWORD;

	if (strstr(buffer, KEEP_ADMIN_STRING))
		return KEEP_ADMIN_KEYWORD;

	if (strstr(buffer, DEFAULT_RANDOM_MAP_STRING))
		return DEFAULT_RANDOM_MAP_KEYWORD;

	if (strstr(buffer, DISABLE_ANON_TEXT_STRING))
		return DISABLE_ANON_TEXT_KEYWORD;

	if (strstr(buffer, SPEC_MAXRATE_STRING))
		return SPEC_MAXRATE_KEYWORD;

	if (strstr(buffer, ASSAULT_MAPREDO_STRING))
		return ASSAULT_MAPREDO_KEYWORD;

	if (strstr(buffer, ASSAULT_AUTOSWITCH_STRING))
		return ASSAULT_AUTOSWITCH_KEYWORD;


	if(strstr(buffer,"custdmg_")) // assault2
	{
		char number[16];
		int value, i, d, q;
		value = i = d = q = 0;

		while(buffer[i] != '\0' && i < 100) i++;
		i++; // count the string length, plus one cuz it dont count that last one.
		while(buffer[q] != ' ' && q < 100 ) q++;
		q++; // count the length of the string up to the space, plus 1 cuz it dont count that last 1.
		for(d=0;q<=i;q++,d++) number[d] = buffer[q]; // copy the part after the space over to number[]

		value = atoi(number); // convert number[16] into and (int)value.

		//gi.bprintf(PRINT_HIGH,"%s===%i===\n",buffer,value);

		if(value > 0)
		{
			if(strstr(buffer,"_pistol"))       CustPistolDmg = value;
			else if(strstr(buffer,"_blakjak")) CustBlakjakDmg = value;
			else if(strstr(buffer,"_shotgun")) CustShotgunDmg = value;
			else if(strstr(buffer,"_tommy"))   CustTommyDmg = value;
			else if(strstr(buffer,"_hmg"))     CustHMGDmg = value;
			else if(strstr(buffer,"_grenade")) CustGrenadeDmg = value;
			else if(strstr(buffer,"_rocket"))  CustRocketDmg = value;
			else if(strstr(buffer,"_flame"))   CustFlameDmg = value;
//			else if(strstr(buffer,"_bomber"))  CustBomberDmg = value;

		}
		return COMMENT_LINE;
	}

	if(strstr(buffer,"assault_")) // assault2
	{
		char number[16];
		int value, i, d, q;
		value = i = d = q = 0;

		while(buffer[i] != '\0' && i < 100) i++;
		i++; // count the string length, plus one cuz it dont count that last one.
		while(buffer[q] != ' ' && q < 100 ) q++;
		q++; // count the length of the string up to the space, plus 1 cuz it dont count that last 1.
		for(d=0;q<=i;q++,d++) number[d] = buffer[q]; // copy the part after the space over to number[]

		value = atoi(number); // convert number[16] into and (int)value.

		//gi.bprintf(PRINT_HIGH,"%s===%i===\n",buffer,value);

		if(value > 0 && strstr(buffer,"_self_rocket_percent_dmg") && value <= 200)
		{
			assault_self_rocket_percent_dmg = value;
		}
/*
		if(value > 0 && strstr(buffer,"_customweaponfrags") && value >= 1 && value <= 16)
		{
			assault_customweaponfrags = value;
		} // virgin18
*/
//		if(strstr(buffer,"_bomberfire")) assault_bomberfire = true;
//		else if(strstr(buffer,"_firebomber")) assault_firebomber = true;
//		else if(strstr(buffer,"_medic")) assault_medic = true;
//		else 
		if(strstr(buffer,"_keytargets")) assault_keytargets = true; 
//		else if(strstr(buffer,"_classgunuse")) assault_classgunuse = true;
		else if(strstr(buffer,"_status")) assault_status = true;
		else if(strstr(buffer,"_replaymaps")) assault_replaymaps = true;
		
		else if(strstr(buffer,"_auto_switch_teams")) assault_auto_switch_teams = true;
		return COMMENT_LINE;
	}

	return FOUND_STRING;

}

int proccess_ini_file()
{
	FILE*	infile;		// Config file to be opened
	int	status = OK;
	int	mode = -1;
	char	buffer[MAX_STRING_LENGTH];
	char	map[32], dummy[32];
	char	filename[32],dir[32];
	cvar_t	*game_dir;


	default_map[0]=0;
	default_teamplay[0]=0;
	default_dmflags[0]=0;
	default_password[0]=0;
	default_dm_realmode[0]=0;

	admincode[0]=0;

	custom_map_filename[0]=0;
	ban_name_filename[0]=0;
	ban_ip_filename[0]=0;
	rconx_file[0]=0;

	// Set number of custom/rotation maps to 0
//	game.num_rmaps = 0;
	game.num_cmaps = 0;
	allow_map_voting = false;
	disable_admin_voting = false;
	scoreboard_first = false;
	num_custom_maps = 0;
	num_netnames = 0;
	num_ips = 0;
	fixed_gametype = false;
	enable_password = false;
	keep_admin_status = false;
	default_random_map = false;
	disable_anon_text = false;
	specrate = 0;
	assault_replaymaps = false;
	assault_status = false;
	assault_auto_switch_teams = false;
//	assault_customweaponfrags = 1; // virgin18
	assault_keytargets = false;
	assault_self_rocket_percent_dmg = 0;

	// Open config file
	game_dir = gi.cvar("game", "", 0);
	if (game_dir->string[0]==0)
		strcpy(dir, "main");
	else
		strcpy(dir, game_dir->string);

	Com_sprintf (filename, sizeof(filename), "%s"DIR_SLASH"assault.ini",dir);
	infile = fopen(filename, "r");
	if (infile == NULL)	return FILE_OPEN_ERROR;
	
	// Read first line of the file
	fgetline(infile, buffer);

	while (!feof(infile))	// while there's still stuff
	{
		switch (proccess_line(buffer))	// Determine what to do with the line, based offa what's in it
		{
		case ADMIN_CODE_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(admincode, map, 16);
			break;
		case COMMENT_LINE:	// do nothing
			break;
		case MAP_ROTATION_KEYWORD:	// add maps to map rotation
			mode = ADD_ROTATION;
			break;
		case DEFAULT_MOTD_KEYWORD:	// add MOTD line
			mode = ADD_MOTD_LINE;
			break;
		case FOUND_STRING:	// Found a string
			if (mode == ADD_CUSTOM)
				add_to_custom_maps(buffer);
			if (mode == ADD_MOTD_LINE)
				add_to_MOTD(buffer);
			break;
		case DEFAULT_MAP_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(default_map, map, 32);
			break;
		case DEFAULT_TEAMPLAY_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(default_teamplay, map, 16);
			break;
		case DEFAULT_DMFLAGS_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(default_dmflags, map, 16);
			break;
		case DEFAULT_PASSWORD_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(default_password, map, 16);
			break;
		case DEFAULT_TIME_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(default_timelimit, map, 16);
			break;
		case DEFAULT_CASH_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(default_cashlimit, map, 16);
			break;
		case DEFAULT_FRAG_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(default_fraglimit, map, 16);
			break;
		case CUSTOM_MAP_KEYWORD: // add maps to custom map list
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(custom_map_filename, map, 32);
			break;
		case MAP_VOTING_KEYWORD: 
			allow_map_voting = true;
			break;
		case BAN_NAME_KEYWORD: 
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(ban_name_filename, map, 32);
			break;
		case BAN_IP_KEYWORD: 
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(ban_ip_filename, map, 32);
			break;
		case SCOREBOARD_FIRST_KEYWORD: 
			scoreboard_first = true;
			break;
		case FPH_SCOREBOARD_KEYWORD: 
			fph_scoreboard = true;
			break;
		case DISABLE_ADMIN_KEYWORD: 
			disable_admin_voting = true;
			break;
		case DEFAULT_REAL_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(default_dm_realmode, map, 16);
			break;
		case FIXED_GAMETYPE_KEYWORD:
			fixed_gametype = true;
			break;
		case ENABLE_PASSWORD_KEYWORD:
			enable_password = true;
			break;
		case RCONX_FILE_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			strncpy(rconx_file, map, 32);
			break;
		case KEEP_ADMIN_KEYWORD:
			keep_admin_status = true;
			break;
		case DEFAULT_RANDOM_MAP_KEYWORD:
			default_random_map = true;
			break;
		case DISABLE_ANON_TEXT_KEYWORD:
			disable_anon_text = true;
			break;
		case SPEC_MAXRATE_KEYWORD:
			sscanf(buffer, "%s %s", dummy, map);	// Quick ugly hack :)
			specrate=atoi(map);
			break;

		case ASSAULT_MAPREDO_KEYWORD:
			assault_replaymaps = true;
			break;
		
		case ASSAULT_AUTOSWITCH_KEYWORD:
			assault_auto_switch_teams = true;
			break;

		default:	// wtf is this?
			gi.dprintf("Unhandled line!\n");
		}
		fgetline(infile, buffer);		// Retrieve next line from the input file
	}

	// close the ini file
	fclose(infile);

	if (ban_name_filename[0]) {
	Com_sprintf (filename, sizeof(filename), "%s"DIR_SLASH"%s",dir, ban_name_filename);
	infile = fopen(filename, "r");
	if (infile != NULL)	
	{
	// Read first line of the file
		fgetline(infile, buffer);
		num_netnames = 0;
		while (!feof(infile))	// while there's still stuff
		{
			if (strlen(buffer) == 0 || buffer[0] == '\n')
			{
				fgetline(infile, buffer);
				continue;
			}
		// Check to see if this is a comment line
			if (buffer[0] == '/' && buffer[1] == '/')
			{
				fgetline(infile, buffer);
				continue;
			}
			strncpy(netname[num_netnames].value,buffer,16);
			fgetline(infile, buffer);
			num_netnames++;
			if (num_netnames==100) break;
		}
		fclose(infile);
	}
	}

	if (ban_ip_filename[0]) {
	Com_sprintf (filename, sizeof(filename), "%s"DIR_SLASH"%s",dir, ban_ip_filename);
	infile = fopen(filename, "r");
	if (infile != NULL)	
	{
	// Read first line of the file
		fgetline(infile, buffer);
		num_ips = 0;
		while (!feof(infile))	// while there's still stuff
		{
			if (strlen(buffer) == 0 || buffer[0] == '\n')
			{
				fgetline(infile, buffer);
				continue;
			}
		// Check to see if this is a comment line
			if (buffer[0] == '/' && buffer[1] == '/')
			{
				fgetline(infile, buffer);
				continue;
			}
			strncpy(ip[num_ips].value,buffer,16);
			fgetline(infile, buffer);
			num_ips++;
			if (num_ips==100) break;
		}
		fclose(infile);
	}
	}

	if (rconx_file[0]) {
	Com_sprintf (filename, sizeof(filename), "%s"DIR_SLASH"%s",dir, rconx_file);
	infile = fopen(filename, "r");
	if (infile != NULL)	
	{
	// Read first line of the file
		fgetline(infile, buffer);
		num_rconx_pass = 0;
		while (!feof(infile))	// while there's still stuff
		{
			if (strlen(buffer) == 0 || buffer[0] == '\n')
			{
				fgetline(infile, buffer);
				continue;
			}
		// Check to see if this is a comment line
			if (buffer[0] == '/' && buffer[1] == '/')
			{
				fgetline(infile, buffer);
				continue;
			}
			strncpy(rconx_pass[num_rconx_pass].value,buffer,32);
			fgetline(infile, buffer);
			num_rconx_pass++;
			if (num_rconx_pass==100) break;
		}
		fclose(infile);
	}
	}

	return OK;
}

int read_map_file()
{

	FILE*	infile;		// Config file to be opened
	char	buffer[MAX_STRING_LENGTH];
	char	map[32], rank[16];
	char	filename[32], dir[32];
	cvar_t	*game_dir;

	game_dir = gi.cvar("game", "", 0);
	if (game_dir->string[0]==0)
		strcpy(dir, "main");
	else
		strcpy(dir, game_dir->string);

	if (custom_map_filename[0]) 
	{
		Com_sprintf (filename, sizeof(filename), "%s"DIR_SLASH"%s",dir, custom_map_filename);
		infile = fopen(filename, "r");
		if (infile == NULL)	return FILE_OPEN_ERROR;

		num_custom_maps = 0;

		// Read first line of the file
		fgetline(infile, buffer);
		while (!feof(infile))	// while there's still stuff
		{
			if (strlen(buffer) == 0 || buffer[0] == '\n')
			{
				fgetline(infile, buffer);
				continue;
			}
		// Check to see if this is a comment line
			if (buffer[0] == '/' && buffer[1] == '/')
			{
				fgetline(infile, buffer);
				continue;
			}

			sscanf(buffer, "%s %s", rank, map);	
			strncpy(custom_list[num_custom_maps].custom_map, map, 32);
			custom_list[num_custom_maps].rank = atoi(rank);
			total_rank += custom_list[num_custom_maps].rank;
			num_custom_maps++;
			fgetline(infile, buffer);
			//if (strstr(level.mapname, "assault_jungle")) //assault2
			//gi.dprintf ("map is: %s\n", map);

		}
	}

	return OK;
}

void add_to_custom_maps(char*	buffer)
{
	strncpy(custom_list[game.num_cmaps].custom_map, buffer, 16);
	game.num_cmaps++;
}

void add_to_MOTD(char*	buffer)
{
	strncpy(MOTD[game.num_MOTD_lines].textline, buffer, 99);
	game.num_MOTD_lines++;
}


// Test functions


int write_map_file()
{

	FILE*	outfile;
	char	temp[32],dir[32];
	int		i;

	cvar_t	*game_dir;
	game_dir = gi.cvar("game", "", 0);
	if (game_dir->string[0]==0)
		strcpy(dir, "main");
	else
		strcpy(dir, game_dir->string);

	if (custom_map_filename[0]) {
	Com_sprintf (temp, sizeof(temp), "%s"DIR_SLASH"%s",dir,custom_map_filename);
	outfile = fopen(temp, "w+");
	if (outfile == NULL)	return FILE_OPEN_ERROR;

	for (i=0; i< num_custom_maps; i++)	
		fprintf(outfile, "%d %s\n",custom_list[i].rank, custom_list[i].custom_map);

	fprintf(outfile, "\n");

	fclose(outfile);
	}

	return OK;
}
