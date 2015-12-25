#include <GLES2/gl2.h>
#include <dlfcn.h>

#include "fleye/service.h"

std::map<std::string,FleyeService*> FleyeService::s_services;

FleyeService* FleyeService::getService(std::string name)
{
	if( s_services.find(name)==s_services.end() ) return 0;
	return s_services[name];
}

void FleyeService::addService(std::string name,FleyeService* svc)
{
	s_services[name] = svc;
}
