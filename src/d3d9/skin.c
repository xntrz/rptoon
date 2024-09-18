#include "skin.h"

#include "rpdbgerr.h"
#include "rpskin.h"

#include "d3dx9.h"

/*****************************************************************************
 Defines
 */

/* NOTE: using MACRO_START & MACRO_STOP from rwcore.h */
#define _memio(addr, bytes, size, write) 									\
MACRO_START																	\
{																			\
	DWORD dwPrevProtect = 0;												\
	if (VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &dwPrevProtect))	\
	{																		\
		memcpy(																\
			(write ? addr : bytes),											\
			(write ? bytes : addr),											\
			size															\
		);																	\
		VirtualProtect(addr, size, dwPrevProtect, &dwPrevProtect);			\
	};																		\
}																			\
MACRO_STOP


/*****************************************************************************
 Typedef Enums
 */

typedef void (*RxD3D9AllInOneRenderCallBack)(RwResEntry* repEntry,
	void* object,
	RwUInt8 type,
	RwUInt32 flags);


/*****************************************************************************
 Typedef Structs
 */

struct _rxD3D9SkinNodeData
{
	RxD3D9AllInOneRenderCallBack renderCallback;
	void* lightingCallback;
	void* shaderBeginCallback;
	void* shaderLightingCallback;
	void* shaderGetMaterialShaderCallback;
	void* shaderMeshRenderCallback;
	void* shaderEndCallback;
};


typedef struct _rxD3D9SkinNodeData _rxD3D9SkinNodeData;


/*****************************************************************************
 vars & data
 */

extern void _rpToonD3D9RenderCallback(RwResEntry* repEntry, void* object, RwUInt8 type, RwUInt32 flags);
extern void _rpToonAtomicChainSkinnedAtomicRenderCallback(RpAtomic* atomic);
extern RwBool _rwD3D9SkinUseVertexShader(RpAtomic* atomic);
extern RwUInt32 _rpSkinGlobals;

static RxPipeline* ToonSkinPipeline = NULL;
static char ToonSkinHookBuffer[64];
static void* ToonSkinHookPtr = NULL;
static size_t ToonSkinHookBytes = 0;


/*****************************************************************************
 private functions
 */

static RwBool
_rwD3D9SkinUseVertexShaderHook(RpAtomic* atomic)
{	
	RxPipeline* pipe = NULL;
	RpSkin* skin = NULL;
	RwBool result = FALSE;
	uintptr_t* ptr = NULL;

	RWFUNCTION(RWSTRING("_rpToonSkinSetupGlobals"));

	RpAtomicGetPipelineMacro(atomic, &pipe);

	if ((pipe == NULL) || (pipe->pluginData != rpSKINTYPETOON))
	{
		// TODO
		ptr = &_rpSkinGlobals;
		ptr += 16;

		if (*(ptr + 0) && *(ptr + 1))
		{
			skin = RpSkinGeometryGetSkin(RpAtomicGetGeometryMacro(atomic));
			if (skin && (*((char*)skin + 28) <= *(ptr + 2)))
				result = TRUE;
		};
	};

	RWRETURN(result);
};


static RwBool
_rpToonSkinSetupGlobals(void)
{
	char* ptr = NULL;
	RxPipeline** pipes = NULL;

	RWFUNCTION(RWSTRING("_rpToonSkinSetupGlobals"));

	/* write toon pipeline into skin globals pipelines array for api such as RpSkinGetD3D9Pipeline and more */
	ptr = (char*)&_rpSkinGlobals;
	ptr += sizeof(uintptr_t) * 8;

	/* get pointer to pipeline array */
	pipes = (RxPipeline**)ptr;
	if (pipes[rpSKINTYPEMATFX] || pipes[rpSKINTYPETOON])
	{
		/* rpskinmatfx.lib or rpskintoon.lib or rpskinmatfxtoon.lib is linked */
		RWRETURN(FALSE);
	};

	/* set our pipeline */
	pipes[rpSKINTYPETOON] = ToonSkinPipeline;

	/* checkout */
	RWASSERT((RpSkinGetD3D9Pipeline(rpSKIND3D9PIPELINETOON) != NULL) && "NULL pipeline after set");
	RWASSERT((RpSkinGetD3D9Pipeline(rpSKIND3D9PIPELINETOON) != ToonSkinPipeline) && "not our pipeline");

	RWRETURN(TRUE);
};


static void
_rpToonSkinShutdownGlobals(void)
{
	char* ptr = NULL;
	RxPipeline** pipes = NULL;

	RWFUNCTION(RWSTRING("_rpToonSkinShutdownGlobals"));

	/* cleanup our globals */
	ptr = (char*)&_rpSkinGlobals;
	ptr += sizeof(uintptr_t) * 8;

	/* get pointer to pipeline array */
	pipes = (RxPipeline**)ptr;
	if (pipes[rpSKINTYPETOON] == ToonSkinPipeline)
	{
		pipes[rpSKINTYPETOON] = NULL;
	};

	RWRETURNVOID();
};


static RwBool
_rpToonSkinInstallHook(void)
{
	RwBool result = FALSE;
	char* ptr = NULL;
	DWORD dwPrevProtect = 0;
	uintptr_t offset = 0;
	SYSTEM_INFO systemInfo;

	RWFUNCTION(RWSTRING("_rpToonSkinInstallHook"));

	/* obtaining system info for page size */
	GetSystemInfo(&systemInfo);

	/* check for incremental linking */
	ptr = (char*)&_rwD3D9SkinUseVertexShader;
	while (ptr != NULL)
	{
		switch (*ptr)
		{
		case '\xE9': // relative jump
			{
				++ptr;

				offset = *((uintptr_t*)ptr);
				ptr += sizeof(uintptr_t);
				ptr += offset;
			}
			break;

		default:
			{
				/* dirty but im still in searching for more elegant way ¯\_(ツ)_/¯ */

				_memio(ptr, ToonSkinHookBuffer, 5, FALSE);
				ToonSkinHookBytes = 5;
				ToonSkinHookPtr = ptr;
				
				static char jmp_payload[5];
				jmp_payload[0] = '\xE9';
				*((uintptr_t*)&jmp_payload[1]) = (uintptr_t)_rwD3D9SkinUseVertexShaderHook - (uintptr_t)ptr - sizeof(uintptr_t) - 1;

				_memio(ptr, jmp_payload, 5, TRUE);

				ptr = NULL;
				result = TRUE;
			}
			break;
		};
	};

	RWRETURN(result);
};


static void
_rpToonSkinUninstallHook(void)
{
	RWFUNCTION(RWSTRING("_rpToolSkinUninstallHook"));

	_memio(ToonSkinHookPtr, ToonSkinHookBuffer, ToonSkinHookBytes, TRUE);

	ToonSkinHookBytes = 0;
	ToonSkinHookPtr = NULL;

	RWRETURNVOID();
};


static RwBool
_rpToonSkinD3D9PipelineNodeInit(RxPipelineNode* self)
{
	_rxD3D9SkinNodeData* data = (_rxD3D9SkinNodeData*)self->privateData;

	RWFUNCTION(RWSTRING("_rpToonSkinD3D9PipelineNodeInit"));

	data->renderCallback 					= &_rpToonD3D9RenderCallback;
	data->shaderBeginCallback 				= NULL;
	data->shaderLightingCallback 			= NULL;
	data->shaderGetMaterialShaderCallback 	= NULL;
	data->shaderMeshRenderCallback 			= NULL;
	data->shaderEndCallback 				= NULL;

	RWRETURN(TRUE);
};


static RxNodeDefinition*
RxNodeDefinitionGetD3D9ToonSkin(void)
{
	static RwChar _rpToonSkin_csl[] = "nodeD3D9SkinAtomicAllInOne.csl";
	static RxNodeDefinition rpToonSkinNodeD3D9;
	RxNodeDefinition* skinAtomicNode = NULL;

	RWFUNCTION(RWSTRING("RxNodeDefinitionGetD3D9ToonSkin"));

	skinAtomicNode = RxNodeDefinitionGetD3D9SkinAtomicAllInOne();
	memcpy(&rpToonSkinNodeD3D9, skinAtomicNode, sizeof(rpToonSkinNodeD3D9));

	rpToonSkinNodeD3D9.nodeMethods.pipelineNodeInit = &_rpToonSkinD3D9PipelineNodeInit;

	RWRETURN(&rpToonSkinNodeD3D9);
};


static void
_rpToonSkinAttachPipeline(RpAtomic* atomic, RpSkinType type)
{
	RxPipeline* pipe = NULL;

	RWFUNCTION(RWSTRING("_rpToonSkinAttachPipeline"));

	if (type == rpSKINTYPETOON)
	{
		_rpToonAtomicChainSkinnedAtomicRenderCallback(atomic);

		pipe = RpSkinGetD3D9Pipeline(rpSKIND3D9PIPELINETOON);
		RWASSERT((pipe != NULL) && "toon pipeline is NULL");

		RpAtomicSetPipelineMacro(atomic, pipe);
	}
	else
	{
		RWASSERT(type == rpSKINTYPEGENERIC);

		pipe = RpSkinGetD3D9Pipeline(rpSKIND3D9PIPELINEGENERIC);
		RWASSERT((pipe != NULL) && "generic pipeline is NULL");

		RpAtomicSetPipelineMacro(atomic, pipe);
	};

	RWRETURNVOID();
};


static RxPipeline*
_rpToonSkinCreatePipeline(void)
{
	RxPipeline* pipe = NULL;
	RxLockedPipe* lockedPipe = NULL;

	RWFUNCTION(RWSTRING("_rpToonSkinCreatePipeline"));

	pipe = RxPipelineCreate();
	if (pipe)
	{		
		pipe->pluginId = rwID_SKINPLUGIN;
		pipe->pluginData = rpSKINTYPETOON;

		lockedPipe = RxPipelineLock(pipe);
		if (lockedPipe)
		{
			lockedPipe = RxLockedPipeAddFragment(lockedPipe, NULL, RxNodeDefinitionGetD3D9ToonSkin(), NULL);
			RWASSERT(lockedPipe != NULL);

			pipe = RxLockedPipeUnlock(lockedPipe);
			RWASSERT(pipe != NULL);
			RWASSERT(pipe == lockedPipe);
		}
		else
		{
			RxPipelineDestroy(pipe);
			pipe = NULL;
		};
	};

	RWRETURN(pipe);
};


static void
_rpToonSkinDestroyPipeline(RxPipeline* pipeline)
{
	RWFUNCTION(RWSTRING("_rpToonSkinDestroyPipeline"));

	RxPipelineDestroy(pipeline);

	RWRETURNVOID();
};


/*****************************************************************************
 public functions
 */

RwBool
_rpToonSkinPipelineCreate(void)
{
	RWFUNCTION(RWSTRING("_rpToonSkinPipelineCreate"));

	ToonSkinPipeline = _rpToonSkinCreatePipeline();
	if (ToonSkinPipeline != NULL)
	{		
		RwBool result = FALSE;

		result = _rpToonSkinSetupGlobals();
		if (result != FALSE)
		{
			result = _rpToonSkinInstallHook();
			if (result != FALSE)
			{
				RWRETURN(TRUE);
			};
		};

		_rpToonSkinDestroyPipeline(ToonSkinPipeline);
		ToonSkinPipeline = NULL;
	};

	RWRETURN(FALSE);
};


RwBool
_rpToonSkinPipelineDestroy(void)
{
	RWFUNCTION(RWSTRING("_rpToonSkinPipelineDestroy"));

	if (ToonSkinPipeline)
	{
		_rpToonSkinUninstallHook();

		_rpToonSkinShutdownGlobals();

		_rpToonSkinDestroyPipeline(ToonSkinPipeline);
		ToonSkinPipeline = NULL;
	};

	RWRETURN(TRUE);
};


void
RpToonSkinAtomicSetType(RpAtomic* atomic, RwInt32 type)
{	
	RpSkinType pipelineType = (RpSkinType)type;

	RWFUNCTION(RWSTRING("RpToonSkinAtomicSetType"));

	if ((pipelineType == rpSKINTYPETOON) && (RwEngineGetPluginOffset(rwID_TOONPLUGIN) == -1))
		pipelineType = rpSKINTYPEGENERIC;

	if ((pipelineType == rpSKINTYPEMATFX) && (RwEngineGetPluginOffset(rwID_MATERIALEFFECTSPLUGIN) == -1))
		pipelineType = rpSKINTYPEGENERIC;

	_rpToonSkinAttachPipeline(atomic, pipelineType);

	RWRETURNVOID();
};