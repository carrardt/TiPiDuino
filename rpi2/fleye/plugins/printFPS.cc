#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"
#include "TextService.h"

#include <sys/time.h>
#include <sstream>

struct printFPS : public FleyePlugin
{
	printFPS() : time_start(0), lastCount(0), fpsText(0) {}

	void setup(FleyeContext* ctx)
	{
		TextService* txtsvc = TextService_instance();
		fpsText = txtsvc->addPositionnedText(0.7,0.9);
		//std::cout<<"printFPS: fpsText @"<<fpsText<<"\n";
		fpsText->setText("Please wait...");
	}

	void run(FleyeContext* ctx)
	{
	   long long time_now;
	   struct timeval te;

	   gettimeofday(&te, NULL);
	   time_now = te.tv_sec * 1000LL + te.tv_usec / 1000;

	   if (time_start == 0)
	   {
		  time_start = time_now;
	   }
	   else if (time_now - time_start > 1000)
	   {
		  uint32_t frame_count = ctx->frameCounter - lastCount;
		  float fps = (float) frame_count / ((time_now - time_start) / 1000.0f);
		  lastCount = ctx->frameCounter;
		  time_start = time_now;
		  std::ostringstream oss;
		  int F = fps*100.0f;
		  oss<<F*0.01f<<" FPS\nframe "<<ctx->frameCounter;
		  fpsText->setText( oss.str().c_str() );
	   }		
	}

	long long time_start;
	uint32_t lastCount;
	PositionnedText* fpsText;
};

FLEYE_REGISTER_PLUGIN(printFPS);
