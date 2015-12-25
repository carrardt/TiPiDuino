#ifndef __fleye_service_H_
#define __fleye_service_H_

#include <map>
#include <string>

struct FleyeService
{
	static FleyeService* getService(std::string name);
	static void addService(std::string name,FleyeService* svc);
	
	static std::map<std::string,FleyeService*> s_services;
};

#define FLEYE_DECLARE_SERVICE(name) \
static inline name* name##_instance() \
{ \
	name* svc = (name*) FleyeService::getService(#name); \
	if( svc == 0 ) { svc = new name; FleyeService::addService(#name,svc); } \
	return svc; \
}

#endif
