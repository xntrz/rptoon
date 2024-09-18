/*****************************************************************************

	File: skin.h

	Purpose: make toon plugin work by changing some skin plugin internals without recompile it with toonfx feature

 */

#ifndef SKIN_H
#define SKIN_H

#include "rwcore.h"
#include "rpworld.h"


#ifdef     __cplusplus
extern "C"
{
#endif  /* __cplusplus */


extern void
RpToonSkinAtomicSetType(RpAtomic* atomic, RwInt32 type);


#ifdef    __cplusplus
}
#endif /* __cplusplus */
	
#endif /* SKIN_H */

