Scriptname SVCollectionPlayerAlias extends ReferenceAlias

Float Property NotifyDelaySeconds = 3.0 Auto

bool _ready = false

Event OnInit()
	_ready = false
	RegisterForSingleUpdate(NotifyDelaySeconds)
EndEvent

Event OnPlayerLoadGame()
	_ready = false
	RegisterForSingleUpdate(NotifyDelaySeconds)
EndEvent

Event OnUpdate()
	_ready = true
EndEvent

Event OnItemAdded(Form akBaseItem, int aiItemCount, ObjectReference akItemReference, ObjectReference akSourceContainer)
	if !_ready
		return
	endif

	SVCollectionManager mgr = GetOwningQuest() as SVCollectionManager
	if mgr != None
		mgr.MaybeNotifyCollectible(akBaseItem)
	endif
EndEvent

