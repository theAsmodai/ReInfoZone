#include "precompiled.h"

CLang g_lang;

lang_t::lang_t(): code(UNKNOWN_LANG)
{
}

lang_t::lang_t(const char* c)
{
	operator=(c);
}

lang_t::lang_t(size_t i) : code(i)
{

}

bool lang_t::operator==(lang_t& lang) const
{
	return code == lang.code;
}

bool lang_t::operator==(size_t lang) const
{
	return code == lang;
}

bool lang_t::operator!=(size_t lang) const
{
	return code != lang;
}

NOINLINE void lang_t::operator=(const char* c)
{
	code = UNKNOWN_LANG;
	if (c[0]) code |= uint8(c[0]) << 0;  else return;
	if (c[1]) code |= uint8(c[1]) << 8;  else return;
	if (c[2]) code |= uint8(c[2]) << 16; else return;
	if (c[3]) code |= uint8(c[3]) << 24;
}

NOINLINE phrase_t::phrase_t(translation_t* t, size_t c) : translations_count(c)
{
	translations = reinterpret_cast<translation_t *>(dupMemory(t, c * sizeof(translation_t)));

	for (size_t i = 0; i < c; i++)
		translations[i].text = dupString(translations[i].text);
}

CLang::CLang() : m_defaultLang(UNKNOWN_LANG)
{

}

void CLang::clear()
{
	m_defaultLang = UNKNOWN_LANG;
	m_langs.clear();
	m_generic_phrases.clear();
}

lang_t CLang::addLang(const char* code)
{
	lang_t newLang(code);

	for (size_t i = 0; i < m_langs.size(); i++) {
		if (newLang == m_langs[i]) {
			return newLang;
		}
	}

	m_langs.push_back(newLang);
	return newLang;
}

NOINLINE void CLang::setDefault(lang_t lang)
{
	for (auto& itr : m_langs) {
		if (lang == itr) {
			m_defaultLang = lang;
			break;
		}
	}
}

void CLang::addPhrase(const char* phrase, translation_t* translations, size_t translations_count)
{
	m_generic_phrases.emplace_back(phrase, translations, translations_count);
}

NOINLINE const char* CLang::localize(const phrase_t& phrase, lang_t lang, const char* fail) const
{
	if (lang != UNKNOWN_LANG) {
		for (size_t i = 0; i < phrase.translations_count; i++) {
			if (lang == phrase.translations[i].lang)
				return phrase.translations[i].text;
		}
	}

	for (size_t i = 0; i < phrase.translations_count; i++) {
		if (m_defaultLang == phrase.translations[i].lang)
			return phrase.translations[i].text;
	}

	return fail;
}

NOINLINE const char* CLang::localize(const char* phrase, lang_t lang, const char* fail) const
{
	auto hash = getHash(phrase);

	for (auto& gen : m_generic_phrases) {
		if (hash == gen.hash) {
			return localize(gen.phrase, lang, fail);
		}
	}

	return fail;
}
