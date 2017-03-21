#pragma once

#include <ppapi/cpp/instance.h>

namespace pp { namespace hue {
	class Instance : public pp::Instance {
	public:
		explicit Instance(PP_Instance instance);
		~Instance();

		void HandleMessage(const pp::Var& message) override;
	};
} }
