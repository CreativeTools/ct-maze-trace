/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2011 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////


#include "c4d.h"
#include <string.h>

Bool RegisterPointGenerator(void);
Bool RegisterTSP(void);


Bool PluginStart(void)
{
	if (!RegisterPointGenerator()) return FALSE;
	if (!RegisterTSP()) return FALSE;

	return TRUE;
}

void PluginEnd(void)
{
}

Bool PluginMessage(LONG id, void *data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
			if (!resource.Init()) return FALSE; // don't start plugin without resource
			return TRUE;

		case C4DMSG_PRIORITY:
			return TRUE;

		case C4DPL_EDITIMAGE:
			return FALSE;
	}

	return FALSE;
}
