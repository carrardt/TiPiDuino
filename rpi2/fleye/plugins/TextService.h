#ifndef __fleye_plugin_TextService_h
#define __fleye_plugin_TextService_h

#include <vector>
#include <string>
#include <iostream>
#include "fleye/service.h"

struct PositionnedText
{
	static constexpr int MaxSize = 1024;
	
	float x, y;
	std::string text;
	
	inline PositionnedText() : x(0.0f), y(0.0f) { text[0]='\0'; }
	
	inline void setText(std::string s)
	{
		text = s;
	}
	inline std::string getText() const { return text; }
};

class TextService : public FleyeService
{
  public:
	inline PositionnedText* addPositionnedText(float x, float y)
	{
		PositionnedText* t = new PositionnedText;
		t->x=x;
		t->y=y;
		m_posTexts.push_back(t);
		return t;
	}

	const std::vector<PositionnedText*>& getPositionnedTexts() const { return m_posTexts; };
	
  private:
	std::vector<PositionnedText*> m_posTexts;
};

FLEYE_DECLARE_SERVICE(TextService)

#endif
