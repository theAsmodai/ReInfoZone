#include "precompiled.h"

CZoneManager g_zoneManager;

bool CZoneManager::isInZone(Vector& origin, size_t zone)
{
	if (zone >= m_zones.size())
		return false;
	return m_zones[zone].contain(origin);
}

size_t CZoneManager::getZoneByOrigin(Vector& origin)
{
	for (zone_t& zone : m_zones) {
		if (zone.contain(origin)) {
			return zone.index;
		}
	}

	return UNKNOWN_ZONE;
}

size_t CZoneManager::getNearestZoneByOrigin(Vector& origin, float maxRange)
{
	auto vec = _mm_loadu_ps(origin);
	float minDist = maxRange;
	int index = UNKNOWN_ZONE;

	for (zone_t& zone : m_zones) {
		float dist = vec3DistToBox(vec, zone.lc(), zone.hc());

		if (dist >= minDist)
			continue;

		index = zone.index;

		if (dist == 0.0)
			break;

		minDist = dist;
	}

	return index;
}

size_t CZoneManager::getZonesCount() const
{
	return m_zones.size();
}

const char* CZoneManager::getZoneName(size_t index, lang_t lang)
{
	if (index >= m_zones.size())
		return "";
	return g_lang.localize(m_zoneNames[index], lang);
}

void CZoneManager::addZone(translation_t* translations, size_t translations_count, Vector lcorner, Vector hcorner)
{
	m_zones.emplace_back(m_zones.size(), lcorner, hcorner);
	m_zoneNames.emplace_back(translations, translations_count);
}

void CZoneManager::addZone(translation_t* translations, size_t translations_count, Vector origin, Vector mins, Vector maxs)
{
	addZone(translations, translations_count, origin + mins, origin + maxs);
}

void CZoneManager::clearZones()
{
	m_zones.clear();
	m_zoneNames.clear();
}
