#include "instance.h"
#include <ppapi/cpp/var.h>

namespace pp { namespace hue {

	Instance::Instance(PP_Instance instance)
	: pp::Instance(instance)
	{
	}

	Instance::~Instance()
	{
	}

	void Instance::HandleMessage(const pp::Var& message)
	{
		LogToConsoleWithSource(PP_LOGLEVEL_LOG, "Instance message", message);
	}
} }
