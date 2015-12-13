#include <string.h>
#include "fleye/texture.h"
#include "fleye/imageprocessing.h"

RASPITEX_Texture* get_named_texture(struct ImageProcessingState* ip, const char * name)
{
	int i;
	for(i=0;i<ip->nTextures;i++)
	{
		if( strcasecmp(name,ip->processing_texture[i].name)==0 )
		{
			return & ip->processing_texture[i];
		}
	}
	return 0;
}
