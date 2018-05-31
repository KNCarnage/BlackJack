#include "oxygine-framework.h"
#include "../../oxygine-flow/src/oxygine-flow.h"
#include <functional>

using namespace oxygine;

class MessageBoxPrintf : public flow::Scene
{
public:
	MessageBoxPrintf(int Type, timeMS duration, std::string sCaption, std::string sFormat, ...);

	void PreShowMessageBox(Event* event);

	void PostShowMessageBox(Event* event);
	void RemoveMessageBox(Event* event);
	spActor _view;

};

typedef oxygine::intrusive_ptr<MessageBoxPrintf> spMessageBoxPrintf;