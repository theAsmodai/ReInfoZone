#pragma once

enum : size_t
{
	UNKNOWN_LANG = 0
};

struct lang_t
{
	lang_t();
	lang_t(const char* c);
	lang_t(size_t i);
	bool operator==(lang_t& lang) const;
	bool operator==(size_t lang) const;
	bool operator!=(size_t lang) const;
	void operator=(const char* c);
	size_t code;
};

struct translation_t
{
	lang_t lang;
	const char* text;
};

struct phrase_t
{
	phrase_t() : translations_count(0), translations(nullptr) {}
	phrase_t(translation_t* t, size_t c);
	size_t translations_count;
	translation_t* translations;
};

class CLang
{
public:
	CLang();
	void clear();
	lang_t addLang(const char* code);
	void setDefault(lang_t lang);
	void addPhrase(const char* phrase, translation_t* translations, size_t translations_count);
	const char* localize(const phrase_t& phrase, lang_t lang, const char* fail = "<unknown>") const;
	const char* localize(const char* phrase, lang_t lang, const char* fail = "<unknown>") const;

private:
	struct genphrase_t
	{
		genphrase_t(const char* p, translation_t* t, size_t c) : hash(getHash(p)), phrase(t, c) {}
		size_t hash;
		phrase_t phrase;
	};

	lang_t m_defaultLang;
	std::vector<lang_t> m_langs;
	std::vector<genphrase_t> m_generic_phrases;
};

extern CLang g_lang;
