#include "SkOSMenu.h"

static int gOSMenuCmd = 7000;

SkOSMenu::SkOSMenu(const char title[])
{
	fTitle = title;
}

SkOSMenu::~SkOSMenu()
{
}

int SkOSMenu::countItems() const
{
	return fItems.count();
}

void SkOSMenu::appendItem(const char title[], const char eventType[], int32_t eventData)
{
	Item* item = fItems.append();

	item->fTitle	 = title;
	item->fEventType = eventType;
	item->fEventData = eventData;
	item->fOSCmd	 = ++gOSMenuCmd;
}

SkEvent* SkOSMenu::createEvent(uint32_t os_cmd)
{
	const Item* iter = fItems.begin();
	const Item*	stop = fItems.end();

	while (iter < stop)
	{
		if (iter->fOSCmd == os_cmd)
		{
			SkEvent* evt = new SkEvent(iter->fEventType);
			evt->setFast32(iter->fEventData);
			return evt;
		}
		iter++;
	}
	return NULL;
}

const char* SkOSMenu::getItem(int index, uint32_t* cmdID) const
{
	if (cmdID)
		*cmdID = fItems[index].fOSCmd;
	return fItems[index].fTitle;
}

