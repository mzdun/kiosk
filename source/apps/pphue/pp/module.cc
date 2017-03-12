#include <ppapi/cpp/module.h>
#include "instance.h"

namespace pp
{
	namespace hue
	{
		class Module : public pp::Module
		{
		public:
			pp::Instance* CreateInstance(PP_Instance instance) override
			{
				return new (std::nothrow) Instance(instance);
			}
		};
	}

	Module* CreateModule()
	{
		return new hue::Module();
	}
}
