#ifndef SkOSMenu_DEFINED
#define SkOSMenu_DEFINED

#include "SkEvent.h"
#include "SkTDArray.h"

class SkOSMenu {
public:
	explicit SkOSMenu(const char title[]);
	~SkOSMenu();

	const char*	getTitle() const { return fTitle; }

	void	appendItem(const char title[], const char eventType[], S32 eventData);

	// called by SkOSWindow when it receives an OS menu event
	int		countItems() const;
	const char*	getItem(int index, U32* cmdID) const;

	SkEvent* createEvent(U32 os_cmd);

private:
	const char*	fTitle;

	struct Item {
		const char*	fTitle;
		const char*	fEventType;
		U32			fEventData;
		U32			fOSCmd;	// internal
	};
	SkTDArray<Item>	fItems;

	// illegal
	SkOSMenu(const SkOSMenu&);
	SkOSMenu& operator=(const SkOSMenu&);
};

#endif

