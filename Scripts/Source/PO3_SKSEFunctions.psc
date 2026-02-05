Scriptname PO3_SKSEFunctions Hidden

; Minimal stub for powerofthree's Papyrus Extender (po3 Papyrus Extender).
; We only use these functions to detect quest items in the player's inventory.

Form[] Function GetQuestItems(ObjectReference akRef, bool abNoEquipped = false, bool abNoFavorited = false) global native

; Adds all inventory items to array, optionally filtering out equipped/favorited/quest items.
Form[] Function AddAllItemsToArray(ObjectReference akRef, bool abNoEquipped = true, bool abNoFavorited = false, bool abNoQuestItem = false) global native

; (major, minor, patch) e.g. [6,0,0]
int[] Function GetPapyrusExtenderVersion() global native
