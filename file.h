#define		ADMIN_CODE_STRING		"admin_code"
#define		CUSTOM_MAP_STRING		"custom_map_file"
#define		MAP_ROTATION_STRING		"map_rotation"
#define		DEFAULT_MAP_STRING		"default_map"
#define		DEFAULT_TEAMPLAY_STRING	"default_teamplay"
#define		DEFAULT_DMFLAGS_STRING	"default_dmflags"
#define		DEFAULT_PASSWORD_STRING	"default_password"
#define		DEFAULT_TIME_STRING		"default_timelimit"
#define		DEFAULT_CASH_STRING		"default_cashlimit"
#define		DEFAULT_FRAG_STRING		"default_fraglimit"
#define		DEFAULT_MOTD_STRING		"MOTD"
#define		ALLOW_MAP_VOTING_STRING	"allow_map_voting"
#define		BAN_NAME_STRING			"ban_name_filename"
#define		BAN_IP_STRING			"ban_ip_filename"
#define		SCOREBOARD_FIRST_STRING	"scoreboard_first"
#define		FPH_SCOREBOARD_STRING	"frags_per_hour_scoreboard"
#define		DISABLE_ADMIN_STRING	"disable_admin_voting"
#define		DEFAULT_REAL_STRING		"default_dm_realmode"
#define		FIXED_GAMETYPE_STRING	"fixed_gametype"
#define		ENABLE_PASSWORD_STRING	"enable_password"
#define		RCONX_FILE_STRING		"rconx_file"
#define		KEEP_ADMIN_STRING		"keep_admin_status"
#define		DEFAULT_RANDOM_MAP_STRING	"default_random_map"
#define		DISABLE_ANON_TEXT_STRING	"disable_anon_text"
#define		SPEC_MAXRATE_STRING		"spec_maxrate"
#define		KICK_DIRTY_STRING		"kick_dirty"

#define		ASSAULT_MAPREDO_STRING	"assault_replaymaps"
#define		ASSAULT_AUTOSWITCH_STRING	"assault_auto_switch_teams"

#define		MAX_STRING_LENGTH	100
#define		FILE_OPEN_ERROR		0
#define		OK					1
#define		ADMIN_CODE_ERROR	2

// Mode Definitions

#define		COMMENT_LINE			0
#define		CUSTOM_MAP_KEYWORD		1
#define		MAP_ROTATION_KEYWORD	2
#define		ADMIN_CODE_KEYWORD		3
#define		FOUND_STRING			4
#define		DEFAULT_MAP_KEYWORD		5
#define		DEFAULT_TEAMPLAY_KEYWORD	6
#define		DEFAULT_DMFLAGS_KEYWORD		7
#define		DEFAULT_PASSWORD_KEYWORD	8
#define		DEFAULT_MOTD_KEYWORD		9
#define		DEFAULT_TIME_KEYWORD		10
#define		DEFAULT_CASH_KEYWORD		11
#define		DEFAULT_FRAG_KEYWORD		12
#define		MAP_VOTING_KEYWORD			13
#define		BAN_NAME_KEYWORD			14
#define		BAN_IP_KEYWORD				15
#define		SCOREBOARD_FIRST_KEYWORD	16
#define		FPH_SCOREBOARD_KEYWORD		17
#define		DISABLE_ADMIN_KEYWORD		18
#define		DEFAULT_REAL_KEYWORD		19
#define		FIXED_GAMETYPE_KEYWORD		20
#define		ENABLE_PASSWORD_KEYWORD		21
#define		RCONX_FILE_KEYWORD			22
#define		KEEP_ADMIN_KEYWORD			23
#define		DEFAULT_RANDOM_MAP_KEYWORD	24
#define		DISABLE_ANON_TEXT_KEYWORD	25
#define		SPEC_MAXRATE_KEYWORD		26
#define		KICK_DIRTY_KEYWORD			27
#define		ASSAULT_MAPREDO_KEYWORD		28
#define		ASSAULT_AUTOSWITCH_KEYWORD  29

#define		STANDBY					0
#define		ADD_CUSTOM				1
#define		ADD_ROTATION			2
#define		ADD_MOTD_LINE			3

int read_map_file();
int write_map_file();
int read_admin_code(char* buffer);
void fgetline (FILE* infile, char* buffer);
int proccess_line(char*	buffer);
int proccess_ini_file();
void test_write();

void add_to_map_rotation(char*	buffer);
void add_to_custom_maps(char*	buffer);
void add_to_MOTD(char*	buffer);
void print_rotation();


