#pragma once

#define HUD_CHECK_INTERVAL			0.2

extern cvar_t* iz_zone_leave_time;
extern cvar_t* iz_use_interval;
extern cvar_t* iz_smooth_positioning;
extern cvar_t* iz_max_aim_angle;
extern cvar_t* iz_item_max_radius;

struct field_data_t
{
	int		type;
	size_t	int_number;
	float	float_number;
	Vector	vec;
	char	string[512];
};

enum data_type_e;

class CFields
{
public:
	CFields();
	bool parseFormat(char* string);
	size_t getCount() const;
	field_data_t* getField(data_type_e type);
	field_data_t* getField(size_t index);
	size_t getTranslations(translation_t* translations, size_t max_count) const;

private:
	field_data_t	m_fields[16];
	size_t			m_count;
};

enum reinfozone_log_mode
{
	rl_none = 0,
	rl_console = 1,
	rl_logfile = 2,
};

struct config_t
{
	size_t logMode;
	bool botsFix;
	size_t defaultOptions;
	char defaultLang[4];

	bool hasLogMode(reinfozone_log_mode m) const
	{
		return (logMode & m) == m;
	}
};

extern config_t g_config;

void loadConfigs();
