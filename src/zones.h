#pragma once

enum : size_t
{
	UNKNOWN_ZONE = size_t(-1)
};

class CZoneManager
{
public:
	bool isInZone(Vector& origin, size_t zone);
	size_t getZoneByOrigin(Vector& origin);
	size_t getNearestZoneByOrigin(Vector& origin, float maxRange);
	size_t getZonesCount() const;
	const char* getZoneName(size_t index, lang_t lang);
	void addZone(translation_t* translations, size_t translations_count, Vector lcorner, Vector hcorner);
	void addZone(translation_t* translations, size_t translations_count, Vector origin, Vector mins, Vector maxs);
	void clearZones();

private:
	typedef ALIGN16 vec4_t avec4_t;

	struct zone_t
	{
		vec3_t lcorner;
		size_t index;
		vec3_t hcorner;
		size_t padding;

		zone_t(size_t i, Vector l, Vector h) : lcorner(l), index(i), hcorner(h), padding(0) {};
		__m128 lc() const { return _mm_load_ps(lcorner); };
		__m128 hc() const { return _mm_load_ps(hcorner); };
		bool contain(Vector& origin) const { return vec3InBox(_mm_loadu_ps(origin), lc(), hc()); };
	};

public:
	std::vector<zone_t, aligned_allocator<zone_t, 64>> m_zones;
	std::vector<phrase_t> m_zoneNames;
};

extern CZoneManager g_zoneManager;
