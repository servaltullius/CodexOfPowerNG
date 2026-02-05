Scriptname SVCollectionManager extends Quest

; IMPORTANT (CK 안정성):
; CategoryLists는 "FormList 안에 FormList들을 넣는" 방식(리스트-오브-리스트)으로 사용합니다.
; 예) SV_Col_Categories_FLST 안에 SV_Col_Armors_FLST, SV_Col_Weapons_FLST ... 를 넣고
;     이 프로퍼티에 SV_Col_Categories_FLST를 지정합니다.
FormList Property CategoryLists Auto

; 카테고리 완성 보상(옵션): CategoryLists와 같은 순서로 Spell을 담은 FormList
FormList Property CategoryRewardAbilities Auto

Message Property ConfirmRegisterMsg Auto

String Property ModName = "SVCollection" Auto

Int Property PageSize = 60 Auto

; true: "발견형" - 전체 목록 없이 등록한 것만 도감에 쌓임
Bool Property DiscoveryMode = true Auto

; When true, attempts to normalize "variant" items to their base forms:
; - Weapons use template (if available)
; - Other items can optionally use variant_map.json mappings
; Default OFF: variants (e.g., premade enchanted gear) count as separate collection entries.
Bool Property NormalizeRegistration = false Auto

; Rewards (Lineage-style): every N registrations grant a tiny permanent bonus
Bool Property EnableRewards = true Auto
Int Property RewardEvery = 10 Auto
Bool Property AllowSkillRewards = false Auto

; Spell power rewards (author-created PERK forms; optional)
; These are designed to affect player-cast spells only (via perk entry point conditions).
Perk Property PerkSpellMagDestruction Auto
Perk Property PerkSpellMagRestoration Auto
Perk Property PerkSpellDurAlteration Auto
Perk Property PerkSpellDurConjuration Auto
Perk Property PerkSpellDurIllusion Auto

; ----------------------------
; Localization (Auto + MCM override)
; ----------------------------
int _l10n = 0
string _l10nLang = ""

String Function GetLanguageOverride()
	; MCM menu: auto / en / ko
	String s = MCM.GetModSettingString(ModName, "sLanguageOverride:Main")
	if s == "en" || s == "ko"
		return s
	endif
	return "auto"
EndFunction

String Function GetActiveLangCode()
	String override = GetLanguageOverride()
	if override == "en" || override == "ko"
		return override
	endif

	String iniLang = Utility.GetINIString("sLanguage:General")
	if iniLang != ""
		if StringUtil.Find(iniLang, "KOREAN") != -1
			return "ko"
		endif
	endif

	return "en"
EndFunction

String Function GetL10nFilePath(String langCode)
	return "Data/SKSE/Plugins/SVCollection/lang/" + langCode + ".json"
EndFunction

Function EnsureL10nLoaded()
	String lang = GetActiveLangCode()
	if lang == ""
		lang = "en"
	endif

	if lang == _l10nLang && _l10n != 0 && JValue.isExists(_l10n)
		return
	endif

	_l10nLang = lang
	_l10n = JValue.readFromFile(GetL10nFilePath(lang))
	if _l10n == 0 && lang != "en"
		_l10nLang = "en"
		_l10n = JValue.readFromFile(GetL10nFilePath("en"))
	endif

	if _l10n != 0
		_l10n = JValue.retain(_l10n, ModName + "_l10n_" + _l10nLang)
	endif
EndFunction

String Function T(String path, String fallback)
	if path == ""
		return fallback
	endif

	EnsureL10nLoaded()
	if _l10n == 0
		return fallback
	endif
	return JValue.solveStr(_l10n, "." + path, fallback)
EndFunction

	int _registeredMap = 0
	int _notifiedMap = 0
	int _blockedMap = 0 ; items that cannot be destroyed (quest items etc.) are auto-excluded
	int _collectibleMap = 0

	int _variantBaseMap = 0 ; Form(variant) -> int(baseFormId runtime)
	bool _variantMapLoaded = false
	int _excludeMap = 0 ; always-excluded quest/unique items (config)
		bool _excludeMapLoaded = false
		Form[] _questItemCache = None
		bool _questItemCacheReady = false
		bool _po3Checked = false
		bool _po3Available = false

	bool _indexBuilt = false
	bool _menuOpen = false

; ----------------------------
; Input (MCM Helper keybind)
; ----------------------------
Event OnControlDown(string control)
	if control == "OpenCollection"
		OpenQuickRegisterMenu()
	endif
EndEvent

; ----------------------------
; Public API (Player alias)
; ----------------------------
Function MaybeNotifyCollectible(Form akBaseItem)
	if akBaseItem == None
		return
	endif

	if !EnsureReady()
		return
	endif

	if !IsLootNotifyEnabled()
		return
	endif

	Form regKey = GetRegisterKey(akBaseItem)
	if regKey == None
		return
	endif

	if DiscoveryMode
		if !IsDiscoverableItem(regKey)
			return
		endif
	else
		if !JFormMap.hasKey(_collectibleMap, regKey)
			return
		endif
	endif

	if IsRegistered(akBaseItem)
		return
	endif

	if _notifiedMap == 0
		return
	endif

	if JFormMap.hasKey(_notifiedMap, regKey)
		return
	endif
	if akBaseItem != regKey && JFormMap.hasKey(_notifiedMap, akBaseItem)
		return
	endif

	JFormMap.setInt(_notifiedMap, regKey, 1)
	Debug.Notification(T("msg.lootUnregisteredPrefix", "Unregistered: ") + regKey.GetName() + T("msg.lootUnregisteredSuffix", " (press hotkey to register)"))
EndFunction

	; ----------------------------
	; Storage / Index
	; ----------------------------
		Bool Function EnsureReady()
			if !JContainers.isInstalled()
				Debug.Notification(T("msg.requiresJContainers", "Codex of Power: JContainers is required."))
				return false
			endif

		; JContainers 객체 핸들은 세이브/로드 또는 GC 타이밍에 무효가 될 수 있어
		; 존재 여부를 항상 확인하고, 필요하면 새로 생성합니다.
		if _registeredMap != 0 && !JValue.isExists(_registeredMap)
			_registeredMap = 0
		endif
			if _registeredMap == 0
				_registeredMap = JFormMap.object()
				if _registeredMap != 0
					_registeredMap = JValue.retain(_registeredMap, ModName + "_registeredMap")
				endif
			endif
	
		if _notifiedMap != 0 && !JValue.isExists(_notifiedMap)
			_notifiedMap = 0
		endif
				if _notifiedMap == 0
					_notifiedMap = JFormMap.object()
					if _notifiedMap != 0
						_notifiedMap = JValue.retain(_notifiedMap, ModName + "_notifiedMap")
					endif
				endif

			if _blockedMap != 0 && !JValue.isExists(_blockedMap)
				_blockedMap = 0
			endif
				if _blockedMap == 0
					_blockedMap = JFormMap.object()
					if _blockedMap != 0
						_blockedMap = JValue.retain(_blockedMap, ModName + "_blockedMap")
					endif
				endif

		if !DiscoveryMode && !_indexBuilt
			BuildCollectibleIndex()
		endif

		if NormalizeRegistration
			EnsureVariantMapLoaded()
		endif
		EnsureExcludeMapLoaded()

		if DiscoveryMode
			return (_registeredMap != 0)
		endif
	return (_collectibleMap != 0)
	EndFunction

	; ----------------------------
	; Storage repair (JContainers)
	; ----------------------------
	Bool Function RepairRegisteredMap()
		int oldMap = _registeredMap
		int newMap = JFormMap.object()
		if newMap == 0
			return false
		endif
		newMap = JValue.retain(newMap, ModName + "_registeredMap")
		if newMap == 0
			return false
		endif

		; Best-effort migration (keep existing progress if possible)
		if oldMap != 0 && JValue.isExists(oldMap)
			Form[] keys = JFormMap.allKeysPArray(oldMap)
			if keys != None
				int i = 0
				int size = keys.Length
				while i < size
					Form k = keys[i]
					if k != None
						int v = JFormMap.getInt(oldMap, k, 0)
						JFormMap.setInt(newMap, k, v)
					endif
					i += 1
				endwhile
			endif
		endif

		_registeredMap = newMap
		return true
	EndFunction

	; ----------------------------
	; Variant -> Base mapping (optional)
	; ----------------------------
	String Function GetVariantMapFilePath()
		return "Data/SKSE/Plugins/" + ModName + "/variant_map.json"
	EndFunction

	; ----------------------------
	; Exclude map (optional, recommended)
	; ----------------------------
	String Function GetExcludeMapFilePath()
		return "Data/SKSE/Plugins/" + ModName + "/exclude_map.json"
	EndFunction

	Function EnsureExcludeMapLoaded()
		if _excludeMapLoaded && _excludeMap != 0 && JValue.isExists(_excludeMap)
			return
		endif
		_excludeMapLoaded = true

		_excludeMap = JFormMap.object()
		if _excludeMap != 0
			_excludeMap = JValue.retain(_excludeMap, ModName + "_excludeMap")
		endif

		int root = JValue.readFromFile(GetExcludeMapFilePath())
		if root == 0
			return
		endif

		int arr = JValue.solveObj(root, ".forms", 0)
		if arr == 0
			return
		endif

		int count = JArray.count(arr)
		if count <= 0
			return
		endif

		int loaded = 0
		int i = 0
		while i < count
			String file = JValue.solveStr(root, ".forms[" + i + "].file", "")
			String id = JValue.solveStr(root, ".forms[" + i + "].id", "")
			int localId = ToLocalFormId(ParseDecimalOrHex(id))

			if file != "" && localId > 0
				Form f = Game.GetFormFromFile(localId, file)
				if f != None && _excludeMap != 0
					JFormMap.setInt(_excludeMap, f, 1)
					loaded += 1
				endif
			endif

			i += 1
		endwhile

		if loaded > 0
			Debug.Trace("SVCollection: Exclude map loaded: " + loaded)
		endif
	EndFunction

	; ----------------------------
	; Quest item detection (optional): powerofthree's Papyrus Extender
	; If installed, we can exclude ALL quest items in the player's inventory automatically.
	; ----------------------------
	Bool Function HasPO3PapyrusExtender()
		if _po3Checked
			return _po3Available
		endif
		_po3Checked = true

		int[] ver = PO3_SKSEFunctions.GetPapyrusExtenderVersion()
		if ver != None && ver.Length > 0
			_po3Available = true
		else
			_po3Available = false
		endif
		return _po3Available
	EndFunction

	Function RefreshQuestItemCache()
		_questItemCacheReady = false
		if !HasPO3PapyrusExtender()
			_questItemCache = None
			return
		endif

		Actor player = Game.GetPlayer()
		if player == None
			_questItemCache = None
			return
		endif

		_questItemCache = PO3_SKSEFunctions.GetQuestItems(player, false, false)
		_questItemCacheReady = (_questItemCache != None)
	EndFunction

	Bool Function IsQuestItemCached(Form item)
		if item == None
			return false
		endif
		if !_questItemCacheReady
			return false
		endif
		if _questItemCache == None
			return false
		endif

		int i = 0
		int size = _questItemCache.Length
		while i < size
			if _questItemCache[i] == item
				return true
			endif
			i += 1
		endwhile
		return false
	EndFunction

Int Function ParseDecimalOrHex(String raw)
	if raw == ""
		return 0
	endif

	int len = StringUtil.GetLength(raw)
	if len <= 0
		return 0
	endif

	bool hex = false
	int start = 0

	; Optional 0x prefix
	if len >= 2
		String c0 = StringUtil.GetNthChar(raw, 0)
		String c1 = StringUtil.GetNthChar(raw, 1)
		if c0 == "0" && (c1 == "x" || c1 == "X")
			hex = true
			start = 2
		endif
	endif

	; Auto-detect hex if any A-F present
	if !hex
		int i = 0
		while i < len && !hex
			String c = StringUtil.GetNthChar(raw, i)
			int o = StringUtil.AsOrd(c)
			if o >= 65 && o <= 70 ; A-F (case-insensitive)
				hex = true
			endif
			i += 1
		endwhile
	endif

	int value = 0
	int idx = start
	while idx < len
		String c = StringUtil.GetNthChar(raw, idx)
		int o = StringUtil.AsOrd(c)

		int digit = -1
		if o >= 48 && o <= 57
			digit = o - 48
		elseif hex && o >= 65 && o <= 70
			digit = o - 55
		endif

		if digit == -1
			; skip separators/spaces
		else
			if hex
				value = (value * 16) + digit
			else
				value = (value * 10) + digit
			endif
		endif

		idx += 1
	endwhile

	return value
EndFunction

Int Function ToLocalFormId(Int anyId)
	; Accepts:
	; - Local formID (e.g. 0x00012EB7)  -> unchanged (masked)
	; - Full runtime formID (e.g. 0x2A012345, 0xFE123ABC) -> converted to local
	if anyId == 0
		return 0
	endif

	int high = Math.LogicalAnd(anyId, 0xFF000000)
	if high == 0xFE000000
		; ESL/Light: lowest 12 bits are the record ID in the file
		return Math.LogicalAnd(anyId, 0x00000FFF)
	endif
	return Math.LogicalAnd(anyId, 0x00FFFFFF)
EndFunction

Function EnsureVariantMapLoaded()
	if _variantMapLoaded
		return
	endif
	_variantMapLoaded = true

		_variantBaseMap = JFormMap.object()
		if _variantBaseMap != 0
			_variantBaseMap = JValue.retain(_variantBaseMap, ModName + "_variantBaseMap")
		endif

	int root = JValue.readFromFile(GetVariantMapFilePath())
	if root == 0
		return
	endif

	int arr = JValue.solveObj(root, ".mappings", 0)
	if arr == 0
		return
	endif

	int count = JArray.count(arr)
	if count <= 0
		return
	endif

	int loaded = 0
	int i = 0
	while i < count
		String vFile = JValue.solveStr(root, ".mappings[" + i + "].variant.file", "")
		String vId = JValue.solveStr(root, ".mappings[" + i + "].variant.id", "")
		String bFile = JValue.solveStr(root, ".mappings[" + i + "].base.file", "")
		String bId = JValue.solveStr(root, ".mappings[" + i + "].base.id", "")

		int vLocal = ToLocalFormId(ParseDecimalOrHex(vId))
		int bLocal = ToLocalFormId(ParseDecimalOrHex(bId))

		if vFile != "" && bFile != "" && vLocal > 0 && bLocal > 0
			Form vForm = Game.GetFormFromFile(vLocal, vFile)
			Form bForm = Game.GetFormFromFile(bLocal, bFile)
			if vForm != None && bForm != None && vForm != bForm
				; Store runtime base formID (stable within current load order)
				JFormMap.setInt(_variantBaseMap, vForm, bForm.GetFormID())
				loaded += 1
			endif
		endif

		i += 1
	endwhile

	if loaded > 0
		Debug.Trace("SVCollection: Variant map loaded: " + loaded)
	endif
EndFunction

Int Function GetCategoryCount()
	if CategoryLists == None
		return 0
	endif
	return CategoryLists.GetSize()
EndFunction

FormList Function GetCategoryList(Int catIdx)
	if CategoryLists == None
		return None
	endif

	int count = CategoryLists.GetSize()
	if catIdx < 0 || catIdx >= count
		return None
	endif

	return CategoryLists.GetAt(catIdx) as FormList
EndFunction

String Function GetCategoryName(Int catIdx)
	return T("ui.category", "Category") + " " + (catIdx + 1)
EndFunction

; ----------------------------
; Discovery helpers
; ----------------------------
	Bool Function IsExcludedItem(Form item)
		if item == None
			return true
		endif

		; Always-excluded list from exclude_map.json (quest-critical etc.)
		if _excludeMap != 0
			if JFormMap.hasKey(_excludeMap, item)
				return true
			endif
			Form k = GetRegisterKey(item)
			if k != None && k != item && JFormMap.hasKey(_excludeMap, k)
				return true
			endif
		endif

		; Quest items (PO3 Papyrus Extender, optional): exclude any quest item currently in player's inventory
		if IsQuestItemCached(item)
			return true
		endif
		Form qiKey = GetRegisterKey(item)
		if qiKey != None && qiKey != item && IsQuestItemCached(qiKey)
			return true
		endif

		; Dragon Claws: always excluded (quest/trophy items). Works for both English and Korean names.
		; (We intentionally use name matching so it also covers non-vanilla claws without CK edits.)
		if (item as MiscObject) != None
			String n = item.GetName()
			if n != ""
				if StringUtil.Find(n, "Dragon Claw") != -1
					return true
				endif
				; Some vanilla claws (e.g., The Golden Claw) don't include "Dragon" in the name.
				if StringUtil.Find(n, "Claw") != -1 || StringUtil.Find(n, "claw") != -1
					return true
				endif
				if StringUtil.Find(n, "발톱") != -1
					return true
				endif
			endif
		endif

		; Auto-excluded: cannot be destroyed (quest items, etc.)
		if _blockedMap != 0
			if JFormMap.hasKey(_blockedMap, item)
				return true
			endif
			Form regKey = GetRegisterKey(item)
			if regKey != None && regKey != item && JFormMap.hasKey(_blockedMap, regKey)
				return true
			endif
		endif

		; Keys
		if (item as Key) != None
			return true
	endif

	; Lockpick / Gold (base game formIDs)
	if item == Game.GetForm(0x0000000A) ; Lockpick
		return true
	endif
	if item == Game.GetForm(0x0000000F) ; Gold001
		return true
	endif

	return false
EndFunction

Int Function GetDiscoveryGroup(Form item)
	if IsExcludedItem(item)
		return -1
	endif

	if (item as Weapon) != None
		return 0
	endif
	if (item as Armor) != None
		return 1
	endif
	; Potions/Food/Poison are ALCH and castable to Potion in Papyrus
	if (item as Potion) != None
		return 2
	endif
	if (item as Ingredient) != None
		return 3
	endif
	if (item as Book) != None
		return 4
	endif

	; Misc (Ammo/Scroll/SoulGem/MiscObject 등)
	if (item as Ammo) != None
		return 5
	endif
	if (item as Scroll) != None
		return 5
	endif
	if (item as SoulGem) != None
		return 5
	endif
	if (item as MiscObject) != None
		return 5
	endif

	return -1
EndFunction

Bool Function IsDiscoverableItem(Form item)
	return (GetDiscoveryGroup(item) >= 0)
EndFunction

String Function GetDiscoveryGroupName(Int group)
	if group == 0
		return T("group.weapons", "Weapons")
	elseif group == 1
		return T("group.armors", "Armors")
	elseif group == 2
		return T("group.consumables", "Consumables")
	elseif group == 3
		return T("group.ingredients", "Ingredients")
	elseif group == 4
		return T("group.books", "Books")
	elseif group == 5
		return T("group.misc", "Misc")
	endif
	return T("group.misc", "Misc")
EndFunction

String Function GetDiscoveryGroupTag(Int group)
	return "[" + GetDiscoveryGroupName(group) + "] "
EndFunction

String Function GetBestItemName(Form primary, Form fallback = None)
	if primary != None
		String n = primary.GetName()
		if n != ""
			return n
		endif
	endif
	if fallback != None
		String n2 = fallback.GetName()
		if n2 != ""
			return n2
		endif
	endif
	return ""
EndFunction

String Function GetItemNameOrUnnamed(Form primary, Form fallback = None)
	String n = GetBestItemName(primary, fallback)
	if n != ""
		return n
	endif
	return T("ui.unnamed", "(unnamed)")
EndFunction

; ----------------------------
; Rewards (Discovery mode)
; ----------------------------
Function MaybeGrantRegistrationReward(Int group, Int totalRegistered)
	if !EnableRewards
		return
	endif

	int every = RewardEvery
	if every <= 0
		return
	endif

	if totalRegistered <= 0
		return
	endif

	if (totalRegistered % every) != 0
		return
	endif

	GrantWeightedRandomReward(group)
EndFunction

Function GrantWeightedRandomReward(Int group)
	Actor player = Game.GetPlayer()
	if player == None
		return
	endif

	int roll = Utility.RandomInt(1, 100)

			; group: 0=무기, 1=방어구, 2=소모품, 3=재료, 4=책, 5=기타
			if group == 0
				; Weapons: damage-feel without touching skill levels (perk mod friendly)
				; Note: we intentionally avoid "MeleeDamage" to prevent melee-only bias.
				if roll <= 35
					GrantReward(player, "AttackDamageMult", 0.015, T("av.attackDamageMult", "Attack damage (physical)"))
				elseif roll <= 60
					GrantReward(player, "CritChance", 1.0, T("av.critChance", "Critical chance"))
				elseif roll <= 70
					GrantReward(player, "UnarmedDamage", 0.20, T("av.unarmedDamage", "Unarmed damage"))
				elseif roll <= 80
					GrantReward(player, "StaminaRate", 0.10, T("av.staminaRate", "Stamina regen"))
				elseif roll <= 90
					GrantReward(player, "Stamina", 2.0, T("av.stamina", "Stamina"))
				elseif roll <= 97
					GrantReward(player, "SpeedMult", 0.50, T("av.speedMult", "Move speed"))
				else
					GrantReward(player, "CarryWeight", 5.0, T("av.carryWeight", "Carry weight"))
				endif
				; Optional: allow tiny skill bumps (OFF by default)
				if AllowSkillRewards
					int extra = Utility.RandomInt(1, 3)
					if extra == 1
					GrantReward(player, "OneHanded", 0.10, T("skill.oneHanded", "Skill (One-Handed)"))
				elseif extra == 2
					GrantReward(player, "TwoHanded", 0.10, T("skill.twoHanded", "Skill (Two-Handed)"))
				else
					GrantReward(player, "Marksman", 0.10, T("skill.marksman", "Skill (Archery)"))
				endif
			endif
			elseif group == 1
				; Armors: defenses + resist
				if roll <= 15
					GrantReward(player, "DamageResist", 5.0, T("av.damageResist", "Armor rating"))
				elseif roll <= 28
					GrantReward(player, "Health", 2.0, T("av.health", "Health"))
				elseif roll <= 36
					GrantReward(player, "MagicResist", 0.75, T("av.magicResist", "Magic resist"))
				elseif roll <= 44
					GrantReward(player, "FireResist", 1.0, T("av.fireResist", "Fire resist"))
				elseif roll <= 52
					GrantReward(player, "FrostResist", 1.0, T("av.frostResist", "Frost resist"))
				elseif roll <= 60
					GrantReward(player, "ElectricResist", 1.0, T("av.electricResist", "Shock resist"))
				elseif roll <= 68
					GrantReward(player, "HealRate", 0.02, T("av.healRate", "Health regen"))
				elseif roll <= 74
					GrantReward(player, "ReflectDamage", 0.30, T("av.reflectDamage", "Reflect damage"))
				elseif roll <= 80
					if AllowSkillRewards
						GrantReward(player, "HeavyArmor", 0.10, T("skill.heavyArmor", "Skill (Heavy Armor)"))
					else
					GrantReward(player, "SmithingMod", 0.50, T("av.smithingMod", "Smithing effectiveness"))
				endif
				elseif roll <= 88
					if AllowSkillRewards
						GrantReward(player, "LightArmor", 0.10, T("skill.lightArmor", "Skill (Light Armor)"))
					else
						GrantReward(player, "CarryWeight", 5.0, T("av.carryWeight", "Carry weight"))
					endif
				elseif roll <= 94
					if AllowSkillRewards
						GrantReward(player, "Block", 0.10, T("skill.block", "Skill (Block)"))
					else
						GrantReward(player, "SpeedMult", 0.50, T("av.speedMult", "Move speed"))
					endif
				else
					GrantReward(player, "Health", 2.0, T("av.health", "Health"))
				endif
			elseif group == 2
				; Consumables: attributes + small resist
				if roll <= 20
					GrantReward(player, "Magicka", 2.0, T("av.magicka", "Magicka"))
				elseif roll <= 40
					GrantReward(player, "Health", 2.0, T("av.health", "Health"))
				elseif roll <= 60
					GrantReward(player, "Stamina", 2.0, T("av.stamina", "Stamina"))
				elseif roll <= 68
					GrantReward(player, "MagickaRate", 0.10, T("av.magickaRate", "Magicka regen"))
				elseif roll <= 76
					GrantReward(player, "StaminaRate", 0.10, T("av.staminaRate", "Stamina regen"))
				elseif roll <= 82
					GrantReward(player, "HealRate", 0.02, T("av.healRate", "Health regen"))
				elseif roll <= 87
					GrantReward(player, "MagicResist", 0.75, T("av.magicResist", "Magic resist"))
				elseif roll <= 91
					GrantReward(player, "FireResist", 1.0, T("av.fireResist", "Fire resist"))
				elseif roll <= 95
					GrantReward(player, "FrostResist", 1.0, T("av.frostResist", "Frost resist"))
				elseif roll <= 98
					GrantReward(player, "ElectricResist", 1.0, T("av.electricResist", "Shock resist"))
				else
					GrantReward(player, "SpeedMult", 0.50, T("av.speedMult", "Move speed"))
				endif
			elseif group == 3
				; Ingredients: alchemy + poison/disease resist
				if roll <= 18
					GrantReward(player, "PoisonResist", 1.0, T("av.poisonResist", "Poison resist"))
				elseif roll <= 28
					GrantReward(player, "DiseaseResist", 1.0, T("av.diseaseResist", "Disease resist"))
				elseif roll <= 40
					GrantReward(player, "HealRate", 0.02, T("av.healRate", "Health regen"))
				elseif roll <= 52
					GrantReward(player, "MagickaRate", 0.10, T("av.magickaRate", "Magicka regen"))
				elseif roll <= 66
					GrantReward(player, "AlchemyMod", 0.50, T("av.alchemyMod", "Alchemy effectiveness"))
				elseif roll <= 76
					if AllowSkillRewards
					GrantReward(player, "Alchemy", 0.10, T("skill.alchemy", "Skill (Alchemy)"))
					else
						GrantReward(player, "Magicka", 2.0, T("av.magicka", "Magicka"))
					endif
				elseif roll <= 88
					GrantReward(player, "Health", 2.0, T("av.health", "Health"))
				elseif roll <= 96
					GrantReward(player, "StaminaRate", 0.10, T("av.staminaRate", "Stamina regen"))
				else
					GrantReward(player, "Stamina", 2.0, T("av.stamina", "Stamina"))
				endif
			elseif group == 4
				; Books: magic utility + (rare) spell power boosts.
				; Spell power is perk-based, so it can be scoped to player-cast spells only.
				if roll <= 4
					if !GrantPerkRewardScaled(PerkSpellMagDestruction, 0.01, "SpellMagDestruction", T("bonus.spellMagDestruction", "Destruction spell power"))
						GrantReward(player, "DestructionMod", 0.50, T("av.destructionMod", "Destruction cost reduction"))
					endif
				elseif roll <= 8
					if !GrantPerkRewardScaled(PerkSpellMagRestoration, 0.01, "SpellMagRestoration", T("bonus.spellMagRestoration", "Restoration spell power"))
						GrantReward(player, "RestorationMod", 0.50, T("av.restorationMod", "Restoration cost reduction"))
					endif
				elseif roll <= 12
					if !GrantPerkRewardScaled(PerkSpellDurAlteration, 0.01, "SpellDurAlteration", T("bonus.spellDurAlteration", "Alteration spell duration"))
						GrantReward(player, "AlterationMod", 0.50, T("av.alterationMod", "Alteration cost reduction"))
					endif
				elseif roll <= 16
					if !GrantPerkRewardScaled(PerkSpellDurConjuration, 0.01, "SpellDurConjuration", T("bonus.spellDurConjuration", "Conjuration spell duration"))
						GrantReward(player, "ConjurationMod", 0.50, T("av.conjurationMod", "Conjuration cost reduction"))
					endif
				elseif roll <= 20
					if !GrantPerkRewardScaled(PerkSpellDurIllusion, 0.01, "SpellDurIllusion", T("bonus.spellDurIllusion", "Illusion spell duration"))
						GrantReward(player, "IllusionMod", 0.50, T("av.illusionMod", "Illusion cost reduction"))
					endif
				elseif roll <= 34
					GrantReward(player, "EnchantingMod", 0.50, T("av.enchantingMod", "Enchanting effectiveness"))
				elseif roll <= 45
					GrantReward(player, "AlterationMod", 0.50, T("av.alterationMod", "Alteration cost reduction"))
				elseif roll <= 56
					GrantReward(player, "DestructionMod", 0.50, T("av.destructionMod", "Destruction cost reduction"))
				elseif roll <= 66
					GrantReward(player, "ConjurationMod", 0.50, T("av.conjurationMod", "Conjuration cost reduction"))
				elseif roll <= 76
					GrantReward(player, "IllusionMod", 0.50, T("av.illusionMod", "Illusion cost reduction"))
				elseif roll <= 86
					GrantReward(player, "RestorationMod", 0.50, T("av.restorationMod", "Restoration cost reduction"))
				elseif roll <= 92
					GrantReward(player, "AbsorbChance", 0.40, T("av.absorbChance", "Spell absorption chance"))
				elseif roll <= 96
					GrantReward(player, "ShoutRecoveryMult", -0.02, T("av.shoutRecoveryMult", "Shout cooldown"))
				else
					GrantReward(player, "MagickaRate", 0.10, T("av.magickaRate", "Magicka regen"))
				endif
			else
				; Misc: crafting/economy/stealth utility
				if roll <= 15
					GrantReward(player, "CarryWeight", 5.0, T("av.carryWeight", "Carry weight"))
				elseif roll <= 30
					GrantReward(player, "SpeechcraftMod", 0.50, T("av.speechcraftMod", "Barter effectiveness"))
				elseif roll <= 42
					GrantReward(player, "SmithingMod", 0.50, T("av.smithingMod", "Smithing effectiveness"))
			elseif roll <= 54
				GrantReward(player, "LockpickingMod", 0.50, T("av.lockpickingMod", "Lockpicking effectiveness"))
			elseif roll <= 66
				GrantReward(player, "PickPocketMod", 0.50, T("av.pickpocketMod", "Pickpocket effectiveness"))
				elseif roll <= 78
					GrantReward(player, "SneakMod", 0.50, T("av.sneakMod", "Sneak effectiveness"))
				elseif roll <= 86
					GrantReward(player, "SpeedMult", 0.50, T("av.speedMult", "Move speed"))
				elseif roll <= 92
					GrantReward(player, "CritChance", 1.0, T("av.critChance", "Critical chance"))
				elseif roll <= 96
					GrantReward(player, "HealRate", 0.02, T("av.healRate", "Health regen"))
				else
					GrantReward(player, "StaminaRate", 0.10, T("av.staminaRate", "Stamina regen"))
				endif
				; keep optional skill rewards minimal in misc
				if AllowSkillRewards
					; previously had lockpicking/pickpocket ranges; keep rare extra tick
					int extraMisc = Utility.RandomInt(1, 6)
				if extraMisc == 1
					GrantReward(player, "Lockpicking", 0.10, T("skill.lockpicking", "Skill (Lockpicking)"))
				elseif extraMisc == 2
					GrantReward(player, "Pickpocket", 0.10, T("skill.pickpocket", "Skill (Pickpocket)"))
				endif
			endif
		endif
EndFunction

	Function GrantReward(Actor player, String avName, Float amount, String label)
		if player == None
			return
		endif

		Float mult = GetRewardMult()
		if mult <= 0.0
			return
		endif
		Float applied = amount * mult
		; ShoutRecoveryMult: lower is better (default 1.0, 0 removes cooldown)
		if avName == "ShoutRecoveryMult"
			Float cur = player.GetActorValue(avName)
			Float minValue = 0.30
		if cur <= minValue
			applied = 0.0
		elseif (cur + applied) < minValue
			applied = minValue - cur
		endif
	endif

	if applied == 0.0
		return
	endif

	player.ModActorValue(avName, applied)
	RecordRewardDelta(player, avName, applied)
	Debug.Notification(T("msg.rewardPrefix", "Collection reward: ") + label)
EndFunction

Bool Function GrantPerkRewardScaled(Perk akPerk, Float deltaPerRank, String rewardKey, String label)
	if akPerk == None
		return false
	endif

	Actor player = Game.GetPlayer()
	if player == None
		return false
	endif

	Float mult = GetRewardMult()
	if mult <= 0.0
		return false
	endif

	; Scale perk "rank adds" by the same reward multiplier used for AV rewards.
	; Example:
	; - mult=0.5 => 50% chance to add 1 rank
	; - mult=2.2 => add 2 ranks + 20% chance for 1 extra rank
	Int guaranteed = mult as Int
	Float frac = mult - guaranteed
	Int appliedRanks = 0

	while appliedRanks < guaranteed
		player.AddPerk(akPerk)
		appliedRanks += 1
	endwhile

	if frac > 0.0
		Int chance = ((frac * 100.0) + 0.5) as Int
		if chance > 0 && Utility.RandomInt(1, 100) <= chance
			player.AddPerk(akPerk)
			appliedRanks += 1
		endif
	endif

	if appliedRanks <= 0
		return false
	endif

	RecordRewardDelta(player, rewardKey, (deltaPerRank * appliedRanks))
	Debug.Notification(T("msg.rewardPrefix", "Collection reward: ") + label)
	return true
EndFunction

Function RecordRewardDelta(Form formKey, String avName, Float delta)
	if formKey == None
		return
	endif
	if avName == ""
		return
	endif

	; Stored per-form (player) to avoid global key collisions.
	String path = ".SVCollectionRewards." + avName
	Float prev = JFormDB.solveFlt(formKey, path, 0.0)
	JFormDB.solveFltSetter(formKey, path, prev + delta, true)
EndFunction

Float Function GetRewardTotal(Form formKey, String avName)
	if formKey == None || avName == ""
		return 0.0
	endif
	return JFormDB.solveFlt(formKey, ".SVCollectionRewards." + avName, 0.0)
EndFunction

Int Function AddRewardLine(UIListMenu menu, Form formKey, String label, String avName, String suffix = "")
	if menu == None
		return 0
	endif

	Float total = GetRewardTotal(formKey, avName)
	if total == 0.0
		return 0
	endif

	String text = label + ": " + FormatSignedFloatSmart(total) + suffix
	menu.AddEntryItem(text)
	return 1
EndFunction

Int Function AddRewardLinePct(UIListMenu menu, Form formKey, String label, String avName)
	if menu == None
		return 0
	endif

	Float total = GetRewardTotal(formKey, avName)
	if total == 0.0
		return 0
	endif

	menu.AddEntryItem(label + ": " + FormatSignedFloatSmart(total) + "%")
	return 1
EndFunction

Int Function AddRewardLineMultPct(UIListMenu menu, Form formKey, String label, String avName)
	if menu == None
		return 0
	endif

	Float total = GetRewardTotal(formKey, avName)
	if total == 0.0
		return 0
	endif

	menu.AddEntryItem(label + ": " + FormatSignedFloatSmart(total * 100.0) + "%")
	return 1
EndFunction

String Function FormatFloatSmart(Float value)
	bool neg = (value < 0.0)
	Float absVal = value
	if neg
		absVal = 0.0 - value
	endif

	int scaled = ((absVal * 100.0) + 0.5) as int
	int whole = scaled / 100
	int frac = scaled - (whole * 100)

	if frac == 0
		if neg
			return "-" + whole
		endif
		return "" + whole
	endif

	String fracStr = "" + frac
	if frac < 10
		fracStr = "0" + frac
	endif

	if neg
		return "-" + whole + "." + fracStr
	endif
	return "" + whole + "." + fracStr
EndFunction

String Function FormatSignedFloatSmart(Float value)
	if value < 0.0
		return "-" + FormatFloatSmart(0.0 - value)
	endif
	return "+" + FormatFloatSmart(value)
EndFunction

Function OpenRewardSummaryMenu()
	if !EnsureReady()
		return
	endif

	UIListMenu menu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
	if menu == None
		return
	endif

	Form playerKey = Game.GetPlayer()
	int totalReg = GetRegisteredCount()
	int rolls = 0
	if RewardEvery > 0
		rolls = totalReg / RewardEvery
	endif

		while true
			menu.ResetMenu()
			menu.AddEntryItem(T("menu.rewards.titlePrefix", "Collection rewards - rolls: ") + rolls + " (" + T("menu.rewards.titleRegistered", "registered: ") + totalReg + ")")
			int backIdx = 1
			menu.AddEntryItem(T("ui.back", "Back"))
			int resetIdx = 2
			menu.AddEntryItem(T("menu.rewards.reset", "Reset rewards (refund)"))
			menu.AddEntryItem("-----")

		int shown = 0

			shown += AddRewardLine(menu, playerKey, T("av.health", "Health"), "Health")
			shown += AddRewardLine(menu, playerKey, T("av.magicka", "Magicka"), "Magicka")
			shown += AddRewardLine(menu, playerKey, T("av.stamina", "Stamina"), "Stamina")
			menu.AddEntryItem("-----")

			shown += AddRewardLine(menu, playerKey, T("av.healRate", "Health regen"), "HealRate")
			shown += AddRewardLine(menu, playerKey, T("av.magickaRate", "Magicka regen"), "MagickaRate")
			shown += AddRewardLine(menu, playerKey, T("av.staminaRate", "Stamina regen"), "StaminaRate")
			menu.AddEntryItem("-----")

			shown += AddRewardLineMultPct(menu, playerKey, T("av.attackDamageMult", "Attack damage (physical)"), "AttackDamageMult")
			shown += AddRewardLinePct(menu, playerKey, T("av.critChance", "Critical chance"), "CritChance")
			shown += AddRewardLine(menu, playerKey, T("av.unarmedDamage", "Unarmed damage"), "UnarmedDamage")
			shown += AddRewardLinePct(menu, playerKey, T("av.reflectDamage", "Reflect damage"), "ReflectDamage")
			menu.AddEntryItem("-----")

			shown += AddRewardLinePct(menu, playerKey, T("av.magicResist", "Magic resist"), "MagicResist")
			shown += AddRewardLinePct(menu, playerKey, T("av.fireResist", "Fire resist"), "FireResist")
			shown += AddRewardLinePct(menu, playerKey, T("av.frostResist", "Frost resist"), "FrostResist")
			shown += AddRewardLinePct(menu, playerKey, T("av.electricResist", "Shock resist"), "ElectricResist")
			shown += AddRewardLinePct(menu, playerKey, T("av.poisonResist", "Poison resist"), "PoisonResist")
			shown += AddRewardLinePct(menu, playerKey, T("av.diseaseResist", "Disease resist"), "DiseaseResist")
			menu.AddEntryItem("-----")

				shown += AddRewardLine(menu, playerKey, T("av.damageResist", "Armor rating"), "DamageResist")
				shown += AddRewardLine(menu, playerKey, T("av.carryWeight", "Carry weight"), "CarryWeight")
				shown += AddRewardLinePct(menu, playerKey, T("av.speedMult", "Move speed"), "SpeedMult")
				menu.AddEntryItem("-----")

				; Crafting / Economy
				shown += AddRewardLine(menu, playerKey, T("av.smithingMod", "Smithing effectiveness"), "SmithingMod")
				shown += AddRewardLine(menu, playerKey, T("av.alchemyMod", "Alchemy effectiveness"), "AlchemyMod")
				shown += AddRewardLine(menu, playerKey, T("av.enchantingMod", "Enchanting effectiveness"), "EnchantingMod")
				shown += AddRewardLine(menu, playerKey, T("av.speechcraftMod", "Barter effectiveness"), "SpeechcraftMod")
				menu.AddEntryItem("-----")

				; Stealth / Utility Mods
				shown += AddRewardLine(menu, playerKey, T("av.sneakMod", "Sneak effectiveness"), "SneakMod")
				shown += AddRewardLine(menu, playerKey, T("av.lockpickingMod", "Lockpicking effectiveness"), "LockpickingMod")
				shown += AddRewardLine(menu, playerKey, T("av.pickpocketMod", "Pickpocket effectiveness"), "PickPocketMod")
				menu.AddEntryItem("-----")

				; Magic cost modifiers (higher = cheaper)
				shown += AddRewardLine(menu, playerKey, T("av.alterationMod", "Alteration cost reduction"), "AlterationMod")
				shown += AddRewardLine(menu, playerKey, T("av.conjurationMod", "Conjuration cost reduction"), "ConjurationMod")
				shown += AddRewardLine(menu, playerKey, T("av.destructionMod", "Destruction cost reduction"), "DestructionMod")
				shown += AddRewardLine(menu, playerKey, T("av.illusionMod", "Illusion cost reduction"), "IllusionMod")
				shown += AddRewardLine(menu, playerKey, T("av.restorationMod", "Restoration cost reduction"), "RestorationMod")
				menu.AddEntryItem("-----")

				; Spell power (player-cast spells only; perk-based)
				shown += AddRewardLinePct(menu, playerKey, T("bonus.spellMagDestruction", "Destruction spell power"), "SpellMagDestruction")
				shown += AddRewardLinePct(menu, playerKey, T("bonus.spellMagRestoration", "Restoration spell power"), "SpellMagRestoration")
				shown += AddRewardLinePct(menu, playerKey, T("bonus.spellDurAlteration", "Alteration spell duration"), "SpellDurAlteration")
				shown += AddRewardLinePct(menu, playerKey, T("bonus.spellDurConjuration", "Conjuration spell duration"), "SpellDurConjuration")
				shown += AddRewardLinePct(menu, playerKey, T("bonus.spellDurIllusion", "Illusion spell duration"), "SpellDurIllusion")
				menu.AddEntryItem("-----")

				; Special
				shown += AddRewardLinePct(menu, playerKey, T("av.absorbChance", "Spell absorption chance"), "AbsorbChance")
				shown += AddRewardLineMultPct(menu, playerKey, T("av.shoutRecoveryMult", "Shout cooldown"), "ShoutRecoveryMult")

				if shown == 0
					menu.AddEntryItem(T("menu.rewards.none", "(No rewards yet)"))
				endif

		menu.OpenMenu()
		int pick = menu.GetResultInt()
		if pick < 0 || pick == backIdx
			return
		endif
		if pick == resetIdx
			if ConfirmRewardReset()
				RefundRewards(playerKey)
			endif
		endif
	endwhile
EndFunction

Bool Function ConfirmRewardReset()
	UIListMenu menu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
	if menu == None
		return false
	endif

	menu.ResetMenu()
	menu.AddEntryItem(T("menu.rewards.confirmTitle", "Reset rewards (refund) - Warning"))
	menu.AddEntryItem(T("menu.rewards.confirmDo", "Proceed (cannot be undone)"))
	menu.AddEntryItem(T("ui.cancel", "Cancel"))
	menu.AddEntryItem("-----")
	menu.AddEntryItem(T("menu.rewards.confirmLine1", "Only recorded rewards will be refunded."))
	menu.AddEntryItem(T("menu.rewards.confirmLine2", "Registration records remain."))
	menu.AddEntryItem(T("menu.rewards.confirmLine3", "Destroyed items are not restored."))

	menu.OpenMenu()
	int pick = menu.GetResultInt()
	return (pick == 1)
EndFunction

Function RefundRewards(Form playerKey)
	if playerKey == None
		return
	endif

	Actor player = Game.GetPlayer()
	if player == None
		return
	endif

	int cleared = 0

	cleared += RefundOne(player, playerKey, "Health")
	cleared += RefundOne(player, playerKey, "Magicka")
	cleared += RefundOne(player, playerKey, "Stamina")
	cleared += RefundOne(player, playerKey, "HealRate")
	cleared += RefundOne(player, playerKey, "MagickaRate")
	cleared += RefundOne(player, playerKey, "StaminaRate")
	cleared += RefundOne(player, playerKey, "AttackDamageMult")
	cleared += RefundOne(player, playerKey, "CritChance")
	cleared += RefundOne(player, playerKey, "UnarmedDamage")
	cleared += RefundOne(player, playerKey, "ReflectDamage")
	cleared += RefundOne(player, playerKey, "MagicResist")
	cleared += RefundOne(player, playerKey, "FireResist")
	cleared += RefundOne(player, playerKey, "FrostResist")
	cleared += RefundOne(player, playerKey, "ElectricResist")
	cleared += RefundOne(player, playerKey, "PoisonResist")
	cleared += RefundOne(player, playerKey, "DiseaseResist")
		cleared += RefundOne(player, playerKey, "DamageResist")
		cleared += RefundOne(player, playerKey, "CarryWeight")
		cleared += RefundOne(player, playerKey, "SpeedMult")
		cleared += RefundOne(player, playerKey, "SmithingMod")
		cleared += RefundOne(player, playerKey, "AlchemyMod")
		cleared += RefundOne(player, playerKey, "EnchantingMod")
		cleared += RefundOne(player, playerKey, "SpeechcraftMod")
		cleared += RefundOne(player, playerKey, "SneakMod")
		cleared += RefundOne(player, playerKey, "LockpickingMod")
		cleared += RefundOne(player, playerKey, "PickPocketMod")
		cleared += RefundOne(player, playerKey, "AlterationMod")
		cleared += RefundOne(player, playerKey, "ConjurationMod")
		cleared += RefundOne(player, playerKey, "DestructionMod")
		cleared += RefundOne(player, playerKey, "IllusionMod")
		cleared += RefundOne(player, playerKey, "RestorationMod")
		cleared += RefundPerkReward(player, playerKey, PerkSpellMagDestruction, "SpellMagDestruction")
		cleared += RefundPerkReward(player, playerKey, PerkSpellMagRestoration, "SpellMagRestoration")
		cleared += RefundPerkReward(player, playerKey, PerkSpellDurAlteration, "SpellDurAlteration")
		cleared += RefundPerkReward(player, playerKey, PerkSpellDurConjuration, "SpellDurConjuration")
		cleared += RefundPerkReward(player, playerKey, PerkSpellDurIllusion, "SpellDurIllusion")
		cleared += RefundOne(player, playerKey, "AbsorbChance")
		cleared += RefundOne(player, playerKey, "ShoutRecoveryMult")

	if AllowSkillRewards
		cleared += RefundOne(player, playerKey, "OneHanded")
		cleared += RefundOne(player, playerKey, "TwoHanded")
		cleared += RefundOne(player, playerKey, "Marksman")
		cleared += RefundOne(player, playerKey, "HeavyArmor")
		cleared += RefundOne(player, playerKey, "LightArmor")
		cleared += RefundOne(player, playerKey, "Block")
		cleared += RefundOne(player, playerKey, "Alchemy")
		cleared += RefundOne(player, playerKey, "Enchanting")
		cleared += RefundOne(player, playerKey, "Alteration")
		cleared += RefundOne(player, playerKey, "Conjuration")
		cleared += RefundOne(player, playerKey, "Destruction")
		cleared += RefundOne(player, playerKey, "Illusion")
		cleared += RefundOne(player, playerKey, "Restoration")
		cleared += RefundOne(player, playerKey, "Speechcraft")
		cleared += RefundOne(player, playerKey, "Sneak")
		cleared += RefundOne(player, playerKey, "Lockpicking")
		cleared += RefundOne(player, playerKey, "Pickpocket")
	endif

Debug.Notification(T("msg.rewardResetPrefix", "Codex of Power: Rewards reset (") + cleared + T("msg.countSuffix", " items)"))
EndFunction

Int Function RefundOne(Actor player, Form playerKey, String avName)
	Float total = GetRewardTotal(playerKey, avName)
	if total == 0.0
		return 0
	endif

	player.ModActorValue(avName, 0.0 - total)
	JFormDB.solveFltSetter(playerKey, ".SVCollectionRewards." + avName, 0.0, true)
	return 1
EndFunction

Int Function RefundPerkReward(Actor player, Form playerKey, Perk akPerk, String rewardKey)
	if player == None || playerKey == None || rewardKey == ""
		return 0
	endif

	Float total = GetRewardTotal(playerKey, rewardKey)
	Bool hadPerk = false
	if akPerk != None && player.HasPerk(akPerk)
		hadPerk = true
		player.RemovePerk(akPerk)
	endif

	if total != 0.0 || hadPerk
		JFormDB.solveFltSetter(playerKey, ".SVCollectionRewards." + rewardKey, 0.0, true)
		return 1
	endif
	return 0
EndFunction

Function BuildCollectibleIndex()
		_collectibleMap = JFormMap.object()
		if _collectibleMap != 0
			_collectibleMap = JValue.retain(_collectibleMap, ModName + "_collectibleMap")
		endif

	int catIdx = 0
	int catCount = GetCategoryCount()
	while catIdx < catCount
		FormList fl = GetCategoryList(catIdx)
		if fl != None
			int size = fl.GetSize()

			int i = 0
			while i < size
				Form item = fl.GetAt(i)
				if item != None
					if JFormMap.hasKey(_collectibleMap, item)
						int prev = JFormMap.getInt(_collectibleMap, item)
						Debug.Trace("SVCollection: Duplicate collectible in categories: " + item + " (" + prev + " vs " + catIdx + ")")
					else
						JFormMap.setInt(_collectibleMap, item, catIdx)
					endif
				endif
				i += 1
			endwhile
		endif
		catIdx += 1
	endwhile

	_indexBuilt = true
EndFunction

Int Function GetPageSize()
	int size = PageSize
	if size < 10
		size = 10
	endif
	if size > 128
		size = 128
	endif
	return size
EndFunction

Int Function GetCategoryTotal(Int catIdx)
	FormList fl = GetCategoryList(catIdx)
	if fl == None
		return 0
	endif
	return fl.GetSize()
EndFunction

Int Function GetCategoryDone(Int catIdx)
	if _registeredMap == 0
		return 0
	endif

	FormList fl = GetCategoryList(catIdx)
	if fl == None
		return 0
	endif

	int done = 0
	int size = fl.GetSize()
	int i = 0
	while i < size
		Form item = fl.GetAt(i)
		if item != None && IsRegistered(item)
			done += 1
		endif
		i += 1
	endwhile
	return done
EndFunction

Bool Function IsRegistered(Form item)
	Form regKey = GetRegisterKey(item)
	if regKey == None || _registeredMap == 0
		return false
	endif
	if JFormMap.hasKey(_registeredMap, regKey)
		return true
	endif
	; Backward compat: older saves may have stored the non-template variant as the key.
	if item != None && item != regKey
		return JFormMap.hasKey(_registeredMap, item)
	endif
	return false
EndFunction

Int Function GetRegisteredCount()
	if _registeredMap == 0
		return 0
	endif
	return JFormMap.count(_registeredMap)
EndFunction

Int Function GetRegisteredCountInGroup(Int group)
	if _registeredMap == 0
		return 0
	endif

	if group < 0
		return GetRegisteredCount()
	endif

	Form[] keys = JFormMap.allKeysPArray(_registeredMap)
	if keys == None
		return 0
	endif

	int count = 0
	int size = keys.Length
	int i = 0
	while i < size
		Form item = keys[i]
		if item != None && GetDiscoveryGroup(item) == group
			count += 1
		endif
		i += 1
	endwhile
	return count
EndFunction

Int Function GetItemCategory(Form item)
	Form regKey = GetRegisterKey(item)
	if regKey == None || _collectibleMap == 0
		return -1
	endif
	if !JFormMap.hasKey(_collectibleMap, regKey)
		return -1
	endif
	return JFormMap.getInt(_collectibleMap, regKey)
EndFunction

; ----------------------------
; Settings (MCM ModSetting)
; ----------------------------
	Bool Function IsLootNotifyEnabled()
		return MCM.GetModSettingBool(ModName, "bEnableLootNotify:Main")
	EndFunction

	Float Function GetRewardMult()
		Float mult = MCM.GetModSettingFloat(ModName, "fRewardMult:Main")
		if mult <= 0.0
			return 0.0
		endif
		if mult < 0.10
			mult = 0.10
		elseif mult > 5.0
			mult = 5.0
		endif
		return mult
	EndFunction

; ----------------------------
; Register
; ----------------------------
Form Function GetRegisterKey(Form item)
	if item == None
		return None
	endif
	if !NormalizeRegistration
		return item
	endif

	; Enchanted weapons in Skyrim are often separate WEAP forms with a "template" base.
	; Normalizing to template prevents suffix-variants from becoming separate codex entries.
	Form regKey = item

	Weapon w = regKey as Weapon
	if w != None
		Weapon tpl = w.GetTemplate()
		if tpl != None
			regKey = tpl
		endif
	endif

	; Optional: user-provided variant->base map (for armor or other items without templates)
	int safety = 0
	bool done = false
	while !done && safety < 8 && _variantBaseMap != 0 && regKey != None && JFormMap.hasKey(_variantBaseMap, regKey)
		int baseId = JFormMap.getInt(_variantBaseMap, regKey, 0)
		if baseId == 0
			done = true
		else
			Form baseForm = Game.GetForm(baseId)
			if baseForm == None || baseForm == regKey
				done = true
			else
				regKey = baseForm
				safety += 1
			endif
		endif
	endwhile

	return regKey
EndFunction

	Bool Function TryRegisterItem(Form item)
		if item == None
			return false
		endif
		if !EnsureReady()
			return false
		endif
		RefreshQuestItemCache()

			Form consumedItem = item
			Form regKey = GetRegisterKey(consumedItem)
			if regKey == None
				return false
			endif
			String displayName = GetBestItemName(regKey, consumedItem)

		; Defense-in-depth: even if something slips into a menu, never consume excluded/quest-critical items.
		if IsExcludedItem(consumedItem) || IsExcludedItem(regKey)
			Debug.Notification(T("msg.registerQuestItem", "Codex of Power: Cannot register (quest item)"))
			return false
		endif

			; Never allow registering quest items (some quest items can still be removed via Papyrus RemoveItem).
			if IsQuestItemCached(consumedItem) || IsQuestItemCached(regKey)
				Debug.Notification(T("msg.registerQuestItem", "Codex of Power: Cannot register (quest item)"))
				return false
			endif

			; If an item has no display name, do not allow registering it (prevents confusing "[Armor]" blank entries).
			if displayName == ""
				; Mark as blocked so it won't show again in discovery/quick-register lists.
				if _blockedMap != 0
					JFormMap.setInt(_blockedMap, regKey, 1)
					if consumedItem != regKey
						JFormMap.setInt(_blockedMap, consumedItem, 1)
					endif
				endif
				Debug.Notification(T("msg.registerUnnamed", "Codex of Power: Cannot register (unnamed item)"))
				return false
			endif

		if IsRegistered(consumedItem)
			return false
		endif
		if DiscoveryMode && !IsDiscoverableItem(consumedItem)
			return false
		endif

		Actor player = Game.GetPlayer()
		if player == None
			return false
		endif

		; Safety: never allow consuming equipped/favorited items (prevents accidental loss).
		; With po3 Papyrus Extender we can reliably detect "safe" inventory entries.
		if HasPO3PapyrusExtender()
			Form[] safeInv = PO3_SKSEFunctions.AddAllItemsToArray(player, true, true, true)
			if safeInv != None
				bool safe = false
				int si = 0
				int ssize = safeInv.Length
				while si < ssize && !safe
					if safeInv[si] == consumedItem
						safe = true
					endif
					si += 1
				endwhile

				if !safe
					Debug.Notification(T("msg.registerProtected", "Codex of Power: Cannot register (equipped/favorited)."))
					return false
				endif
			else
				; Fallback: at least block equipped items.
				if player.IsEquipped(consumedItem)
					Debug.Notification(T("msg.registerProtected", "Codex of Power: Cannot register (equipped/favorited)."))
					return false
				endif
			endif
		else
			; Fallback: without po3 we can only block equipped items.
			if player.IsEquipped(consumedItem)
				Debug.Notification(T("msg.registerProtected", "Codex of Power: Cannot register (equipped/favorited)."))
				return false
			endif
		endif

		int have = player.GetItemCount(consumedItem)
		if have < 1
			return false
		endif

	int oldHave = have

	player.RemoveItem(consumedItem, 1, true)
	int newHave = player.GetItemCount(consumedItem)

		; 퀘스트 아이템 등으로 RemoveItem이 실패했는지 방어적으로 확인
		if newHave >= oldHave
			; Mark as blocked so it won't show again in discovery/quick-register lists.
			if _blockedMap != 0
				JFormMap.setInt(_blockedMap, regKey, 1)
				if consumedItem != regKey
					JFormMap.setInt(_blockedMap, consumedItem, 1)
				endif
			endif
			Debug.Notification(T("msg.registerCantDestroy", "Codex of Power: Cannot register (cannot destroy item)"))
			return false
		endif

	int storedValue = 1
	if DiscoveryMode
		storedValue = GetDiscoveryGroup(regKey)
	endif

		JFormMap.setInt(_registeredMap, regKey, storedValue)
		bool ok = (_registeredMap != 0) && JFormMap.hasKey(_registeredMap, regKey)
		if !ok
			Debug.Trace("SVCollection: Register FAILED. regMap=" + _registeredMap + ", exists=" + JValue.isExists(_registeredMap) + ", item=" + regKey)

			; Self-heal: sometimes the stored handle can become unusable (save upgrade / stale handle).
			; Rebuild the map once (best-effort migrate) and retry.
			if RepairRegisteredMap()
				JFormMap.setInt(_registeredMap, regKey, storedValue)
				ok = (_registeredMap != 0) && JFormMap.hasKey(_registeredMap, regKey)
				if ok
					Debug.Trace("SVCollection: Register recovered by rebuilding map. newRegMap=" + _registeredMap)
				endif
			endif

			if !ok
				; 인벤은 이미 줄었으니 되돌림
				player.AddItem(consumedItem, 1, true)
				Debug.Notification(T("msg.registerSaveFail", "Codex of Power: Register failed (save failed)"))
				return false
			endif
		endif

		if DiscoveryMode
			int group = storedValue
			int totalReg = GetRegisteredCount()
			Debug.Notification(T("msg.registerOkPrefix", "Registered: ") + displayName + " (" + GetDiscoveryGroupName(group) + ", " + T("msg.totalPrefix", "total ") + totalReg + T("msg.totalSuffix", " items") + ")")
			MaybeGrantRegistrationReward(group, totalReg)
			return true
		endif

	int catIdx = GetItemCategory(regKey)
	if catIdx >= 0
		TryGrantCategoryReward(catIdx)
	endif

		if catIdx >= 0
			int done = GetCategoryDone(catIdx)
			int total = GetCategoryTotal(catIdx)
			Debug.Notification(T("msg.registerOkPrefix", "Registered: ") + displayName + " (" + done + "/" + total + ")")
		else
			Debug.Notification(T("msg.registerOkPrefix", "Registered: ") + displayName)
		endif
		return true
EndFunction

Function TryGrantCategoryReward(Int catIdx)
	if CategoryRewardAbilities == None
		return
	endif

	int rewardCount = CategoryRewardAbilities.GetSize()
	if catIdx < 0 || catIdx >= rewardCount
		return
	endif

	Spell reward = CategoryRewardAbilities.GetAt(catIdx) as Spell
	if reward == None
		return
	endif

	int total = GetCategoryTotal(catIdx)
	if total <= 0
		return
	endif

	int done = GetCategoryDone(catIdx)
	if done < total
		return
	endif

	Actor player = Game.GetPlayer()
	if player.HasSpell(reward)
		return
	endif

	player.AddSpell(reward, false)
	Debug.Notification(T("msg.categoryRewardPrefix", "Category completion reward: ") + GetCategoryName(catIdx))
EndFunction

; ----------------------------
; UI: Quick Register (default hotkey)
; ----------------------------
	Function OpenQuickRegisterMenu()
		if DiscoveryMode
			OpenQuickRegisterMenu_Discovery()
			return
	endif

	if _menuOpen
		return
	endif

			if !EnsureReady()
				return
			endif

		UIListMenu menu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
		if menu == None
			Debug.Notification(T("msg.requiresUIExtensions", "Codex of Power: UIExtensions is required."))
		return
	endif

	_menuOpen = true

	int page = 0
	bool keepOpen = true
	while keepOpen
		int pageLen = GetPageSize()

		Form[] pageItems = new Form[128]
		String[] pageLabels = new String[128]
		int pageCount = 0
		bool hasNext = false
		int totalAvailable = 0

		int start = page * pageLen
		int passed = 0

		Actor player = Game.GetPlayer()
		Form[] safeInv = None
		int safeInvMap = 0
		if HasPO3PapyrusExtender()
			safeInv = PO3_SKSEFunctions.AddAllItemsToArray(player, true, true, true)
			if safeInv != None
				safeInvMap = JFormMap.object()
				int si = 0
				int ssize = safeInv.Length
				while si < ssize
					Form sf = safeInv[si]
					if sf != None
						JFormMap.setInt(safeInvMap, sf, 1)
					endif
					si += 1
				endwhile
			endif
		endif
		int catCount = GetCategoryCount()
		int catIdx = 0
				while catIdx < catCount
					FormList fl = GetCategoryList(catIdx)
					if fl != None
					int size = fl.GetSize()
					int i = 0
					while i < size
						Form item = fl.GetAt(i)
						i += 1
	
							if item != None
								if IsExcludedItem(item)
									; skip quest items / keys / etc.
								elseif !IsRegistered(item)
									int have = player.GetItemCount(item)
									if have >= 1
										bool safe = true
										if safeInvMap != 0
											safe = JFormMap.hasKey(safeInvMap, item)
										else
											; Fallback: without po3 we can only safely hide equipped items.
											if player.IsEquipped(item)
												safe = false
											endif
										endif

										if safe
											totalAvailable += 1
		
											if passed < start
												passed += 1
											else
												if pageCount < pageLen
													pageItems[pageCount] = item
													pageLabels[pageCount] = GetItemNameOrUnnamed(item)
													pageCount += 1
												else
													hasNext = true
												endif
											endif
										endif
									endif
								endif
							endif
						endwhile
					endif
					catIdx += 1
			endwhile

		menu.ResetMenu()

		int cursor = 0
		menu.AddEntryItem(T("menu.quickRegister.titlePrefix", "Codex of Power - Available: ") + totalAvailable + " (" + T("ui.page", "Page") + " " + (page + 1) + ")")
		cursor += 1

		int dashboardIdx = cursor
		menu.AddEntryItem(T("menu.quickRegister.dashboard", "Dashboard / Codex"))
		cursor += 1

		int prevIdx = -1
		if page > 0
			prevIdx = cursor
			menu.AddEntryItem(T("ui.prevPage", "Previous page"))
			cursor += 1
		endif

		int nextIdx = -1
		if hasNext
			nextIdx = cursor
			menu.AddEntryItem(T("ui.nextPage", "Next page"))
			cursor += 1
		endif

		int closeIdx = cursor
		menu.AddEntryItem(T("ui.close", "Close"))
		cursor += 1

		menu.AddEntryItem("-----")
		cursor += 1

		int firstItemIdx = cursor
		int i = 0
		while i < pageCount
			menu.AddEntryItem(pageLabels[i])
			i += 1
		endwhile

			menu.OpenMenu()
			int pick = menu.GetResultInt()

			if pick < 0 || pick == closeIdx
				keepOpen = false
			elseif pick == dashboardIdx
				OpenDashboard()
			elseif prevIdx != -1 && pick == prevIdx
				page -= 1
				if page < 0
					page = 0
				endif
			elseif nextIdx != -1 && pick == nextIdx
				page += 1
			else
				int itemPick = pick - firstItemIdx
				if itemPick >= 0 && itemPick < pageCount
					Form selected = pageItems[itemPick]
					if selected != None
						bool doRegister = true
						if ConfirmRegisterMsg != None
							doRegister = (ConfirmRegisterMsg.Show() == 0)
						endif

						if doRegister
							TryRegisterItem(selected)
						endif
					endif
				endif
			endif
		endwhile

	_menuOpen = false
EndFunction

	Function OpenQuickRegisterMenu_Discovery()
	if _menuOpen
		return
	endif

			if !EnsureReady()
				return
			endif

		UIListMenu menu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
		if menu == None
			Debug.Notification(T("msg.requiresUIExtensions", "Codex of Power: UIExtensions is required."))
		return
	endif

	_menuOpen = true

	int page = 0
	bool keepOpen = true
	while keepOpen
		int pageLen = GetPageSize()

		Form[] pageItems = new Form[128]
		String[] pageLabels = new String[128]
		int pageCount = 0
		bool hasNext = false
		int totalAvailable = 0

		int start = page * pageLen
		int passed = 0

			Actor player = Game.GetPlayer()
			Form[] inv = None
			if HasPO3PapyrusExtender()
				; Exclude equipped + favorited + quest items for safety (prevents accidental destruction).
				inv = PO3_SKSEFunctions.AddAllItemsToArray(player, true, true, true)
			endif

			int seen = JFormMap.object()
			int numItems = 0
			if inv != None
				numItems = inv.Length
			else
				numItems = player.GetNumItems()
			endif
			int i = 0
			while i < numItems
				Form item = None
				if inv != None
					item = inv[i]
				else
					item = player.GetNthForm(i)
					; Fallback: without po3 we can only safely hide equipped items.
					if item != None && player.IsEquipped(item)
						item = None
					endif
				endif
				i += 1
	
				if item != None
					Form regKey = GetRegisterKey(item)
					if regKey != None
						; Skip already-registered (avoid calling GetRegisterKey twice via IsRegistered()).
						bool registered = false
						if _registeredMap != 0
							if JFormMap.hasKey(_registeredMap, regKey)
								registered = true
							elseif item != regKey && JFormMap.hasKey(_registeredMap, item)
								registered = true
							endif
						endif

						if !registered
							int group = GetDiscoveryGroup(regKey)
							if group >= 0
								; Prefer normalized name, but fall back to the actual inventory item name.
								; If both are blank, skip entirely (avoid confusing "[Armor]" empty entries).
								String labelName = GetBestItemName(regKey, item)
								if labelName != "" && !JFormMap.hasKey(seen, regKey)
									; Deduplicate templated variants (e.g., enchanted weapon forms)
									JFormMap.setInt(seen, regKey, 1)
									totalAvailable += 1

									if passed < start
										passed += 1
									else
										if pageCount < pageLen
											pageItems[pageCount] = item
											pageLabels[pageCount] = GetDiscoveryGroupTag(group) + labelName
											pageCount += 1
										else
											hasNext = true
										endif
									endif
								endif
							endif
						endif
					endif
				endif
			endwhile

		menu.ResetMenu()

		int cursor = 0
		menu.AddEntryItem(T("menu.quickRegister.titlePrefix", "Codex of Power - Available: ") + totalAvailable + " (" + T("ui.page", "Page") + " " + (page + 1) + ")")
		cursor += 1

		int dashboardIdx = cursor
		menu.AddEntryItem(T("menu.quickRegister.codex", "Codex (registered items)"))
		cursor += 1

		int prevIdx = -1
		if page > 0
			prevIdx = cursor
			menu.AddEntryItem(T("ui.prevPage", "Previous page"))
			cursor += 1
		endif

		int nextIdx = -1
		if hasNext
			nextIdx = cursor
			menu.AddEntryItem(T("ui.nextPage", "Next page"))
			cursor += 1
		endif

		int closeIdx = cursor
		menu.AddEntryItem(T("ui.close", "Close"))
		cursor += 1

		menu.AddEntryItem("-----")
		cursor += 1

		int firstItemIdx = cursor
		i = 0
		while i < pageCount
			menu.AddEntryItem(pageLabels[i])
			i += 1
		endwhile

		menu.OpenMenu()
		int pick = menu.GetResultInt()

		if pick < 0 || pick == closeIdx
			keepOpen = false
		elseif pick == dashboardIdx
			OpenDashboard()
		elseif prevIdx != -1 && pick == prevIdx
			page -= 1
			if page < 0
				page = 0
			endif
		elseif nextIdx != -1 && pick == nextIdx
			page += 1
		else
			int itemPick = pick - firstItemIdx
			if itemPick >= 0 && itemPick < pageCount
				Form selected = pageItems[itemPick]
				if selected != None
					bool doRegister = true
					if ConfirmRegisterMsg != None
						doRegister = (ConfirmRegisterMsg.Show() == 0)
					endif

					if doRegister
						TryRegisterItem(selected)
					endif
				endif
			endif
		endif
	endwhile

	_menuOpen = false
EndFunction

; ----------------------------
; UI: Dashboard + Category view
; ----------------------------
Function OpenDashboard()
	if DiscoveryMode
		OpenDashboard_Discovery()
		return
	endif

	if !EnsureReady()
		return
	endif

	UIListMenu menu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
	if menu == None
		return
	endif

	int catCount = GetCategoryCount()
	while true
		int overallDone = 0
		int overallTotal = 0

		int i = 0
		while i < catCount
			overallTotal += GetCategoryTotal(i)
			overallDone += GetCategoryDone(i)
			i += 1
		endwhile

			menu.ResetMenu()
			menu.AddEntryItem(T("menu.dashboard.overallPrefix", "Codex of Power - Overall: ") + overallDone + "/" + overallTotal)
			menu.AddEntryItem(T("ui.back", "Back"))

		i = 0
		while i < catCount
			int done = GetCategoryDone(i)
			int total = GetCategoryTotal(i)

			int pct = 0
			if total > 0
				pct = (done * 100) / total
			endif
			menu.AddEntryItem(GetCategoryName(i) + "  " + done + "/" + total + " (" + pct + "%)")
			i += 1
		endwhile

		menu.OpenMenu()
		int pick = menu.GetResultInt()
		if pick < 0 || pick == 1
			return
		endif

		int catIdx = pick - 2
		if catIdx >= 0 && catIdx < catCount
			OpenCategoryMenu(catIdx)
		endif
	endwhile
EndFunction

Function OpenDashboard_Discovery()
	if !EnsureReady()
		return
	endif

	UIListMenu menu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
	if menu == None
		return
	endif

	while true
		int totalReg = GetRegisteredCount()

			menu.ResetMenu()
			menu.AddEntryItem(T("menu.dashboard.registeredPrefix", "Codex of Power - Registered: ") + totalReg)

			int quickIdx = 1
			menu.AddEntryItem(T("menu.dashboard.quickRegister", "Quick Register (unregistered in inventory)"))

			int rewardsIdx = 2
			menu.AddEntryItem(T("menu.dashboard.rewards", "Rewards / Stats"))

			int backIdx = 3
			menu.AddEntryItem(T("ui.back", "Back"))

			int allIdx = 4
			menu.AddEntryItem(T("menu.dashboard.allItems", "All items"))

		int firstGroupIdx = 5
		int group = 0
		while group <= 5
			menu.AddEntryItem(GetDiscoveryGroupName(group) + "  " + GetRegisteredCountInGroup(group))
			group += 1
		endwhile

		menu.OpenMenu()
		int pick = menu.GetResultInt()
		if pick < 0 || pick == backIdx
			return
		endif

		if pick == quickIdx
			OpenQuickRegisterMenu()
		elseif pick == rewardsIdx
			OpenRewardSummaryMenu()
		elseif pick == allIdx
			OpenDiscoveryGroupMenu(-1)
		else
			int groupPick = pick - firstGroupIdx
			if groupPick >= 0 && groupPick <= 5
				OpenDiscoveryGroupMenu(groupPick)
			endif
		endif
	endwhile
EndFunction

Function OpenDiscoveryGroupMenu(Int group)
	if !EnsureReady()
		return
	endif

	UIListMenu menu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
	if menu == None
		return
	endif

	int page = 0
	while true
		int pageLen = GetPageSize()

		Form[] pageItems = new Form[128]
		String[] pageLabels = new String[128]
		int pageCount = 0
		bool hasNext = false

		int start = page * pageLen
		int passed = 0

		Actor player = Game.GetPlayer()
		Form[] keys = JFormMap.allKeysPArray(_registeredMap)
		int size = 0
		if keys != None
			size = keys.Length
		endif

		int i = 0
		while i < size
			Form item = keys[i]
			i += 1

			if item != None
				bool groupMatch = true
				if group >= 0 && GetDiscoveryGroup(item) != group
					groupMatch = false
				endif

				if groupMatch
					if passed < start
						passed += 1
					else
						if pageCount < pageLen
							pageItems[pageCount] = item

							int have = player.GetItemCount(item)
							String haveTxt = ""
							if have > 0
								haveTxt = " x" + have
							endif

							if group < 0
								int g = GetDiscoveryGroup(item)
								pageLabels[pageCount] = GetDiscoveryGroupTag(g) + GetItemNameOrUnnamed(item) + haveTxt
							else
								pageLabels[pageCount] = GetItemNameOrUnnamed(item) + haveTxt
							endif

							pageCount += 1
						else
							hasNext = true
						endif
					endif
				endif
			endif
		endwhile

		menu.ResetMenu()

			String title = T("menu.list.allTitle", "All items")
			if group >= 0
				title = GetDiscoveryGroupName(group)
			endif

			menu.AddEntryItem(title + " - " + T("menu.list.registered", "registered: ") + GetRegisteredCountInGroup(group) + " (" + T("ui.page", "Page") + " " + (page + 1) + ")")

			int backIdx = 1
			menu.AddEntryItem(T("ui.back", "Back"))

		int cursor = 2
		int prevIdx = -1
			if page > 0
				prevIdx = cursor
				menu.AddEntryItem(T("ui.prevPage", "Previous page"))
				cursor += 1
			endif

		int nextIdx = -1
			if hasNext
				nextIdx = cursor
				menu.AddEntryItem(T("ui.nextPage", "Next page"))
				cursor += 1
			endif

		menu.AddEntryItem("-----")
		cursor += 1

		int firstItemIdx = cursor
		i = 0
		while i < pageCount
			menu.AddEntryItem(pageLabels[i])
			i += 1
		endwhile

		menu.OpenMenu()
		int pick = menu.GetResultInt()
		if pick < 0 || pick == backIdx
			return
		endif

		if prevIdx != -1 && pick == prevIdx
			page -= 1
			if page < 0
				page = 0
			endif
		elseif nextIdx != -1 && pick == nextIdx
			page += 1
		endif
	endwhile
EndFunction

Function OpenCategoryMenu(Int catIdx)
	if !EnsureReady()
		return
	endif
	if catIdx < 0 || catIdx >= GetCategoryCount()
		return
	endif

	UIListMenu menu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
	if menu == None
		return
	endif

	int filter = 0 ; 0=전체, 1=미등록, 2=미등록+소지중
	int page = 0

	FormList fl = GetCategoryList(catIdx)
	if fl == None
		return
	endif

	while true
		int done = GetCategoryDone(catIdx)
		int total = GetCategoryTotal(catIdx)

		int pct = 0
		if total > 0
			pct = (done * 100) / total
		endif

		int pageLen = GetPageSize()
		Form[] pageItems = new Form[128]
		String[] pageLabels = new String[128]
		int pageCount = 0
		bool hasNext = false

		int start = page * pageLen
		int passed = 0

		Actor player = Game.GetPlayer()
		int size = fl.GetSize()
		int i = 0
		while i < size
			Form item = fl.GetAt(i)
			i += 1
			if item != None
				bool reg = IsRegistered(item)
				int have = player.GetItemCount(item)

				bool show = true
				if filter == 1 && reg
					show = false
				elseif filter == 2
					if reg || have < 1
						show = false
					endif
				endif

				if show
					if passed < start
						passed += 1
					else
						if pageCount < pageLen
							pageItems[pageCount] = item
							String mark = "[ ] "
							if reg
								mark = "[X] "
							endif

							String haveTxt = ""
							if !reg && have > 0
								haveTxt = " x" + have
							endif

							pageLabels[pageCount] = mark + GetItemNameOrUnnamed(item) + haveTxt
							pageCount += 1
						else
							hasNext = true
						endif
					endif
				endif
			endif
		endwhile

		menu.ResetMenu()
		menu.AddEntryItem(GetCategoryName(catIdx) + "  " + done + "/" + total + " (" + pct + "%)")
		int cursor = 1

		int backIdx = cursor
		menu.AddEntryItem(T("ui.back", "Back"))
		cursor += 1

		String filterLabel = T("ui.filterAll", "Filter: All")
		if filter == 1
			filterLabel = T("ui.filterUnregistered", "Filter: Unregistered")
		elseif filter == 2
			filterLabel = T("ui.filterUnregisteredOwned", "Filter: Unregistered + Owned")
		endif

		int filterIdx = cursor
		menu.AddEntryItem(filterLabel)
		cursor += 1

		int prevIdx = -1
		if page > 0
			prevIdx = cursor
			menu.AddEntryItem(T("ui.prevPage", "Previous page"))
			cursor += 1
		endif

		int nextIdx = -1
		if hasNext
			nextIdx = cursor
			menu.AddEntryItem(T("ui.nextPage", "Next page"))
			cursor += 1
		endif

		menu.AddEntryItem("-----")
		cursor += 1

		int firstItemIdx = cursor
		i = 0
		while i < pageCount
			menu.AddEntryItem(pageLabels[i])
			i += 1
		endwhile

		menu.OpenMenu()
		int pick = menu.GetResultInt()
		if pick < 0 || pick == backIdx
			return
		endif

		if pick == filterIdx
			filter = (filter + 1) % 3
			page = 0
		elseif prevIdx != -1 && pick == prevIdx
			page -= 1
			if page < 0
				page = 0
			endif
		elseif nextIdx != -1 && pick == nextIdx
			page += 1
		else
			int itemPick = pick - firstItemIdx
			if itemPick >= 0 && itemPick < pageCount
				Form selected = pageItems[itemPick]
				if selected != None
					if !IsRegistered(selected) && player.GetItemCount(selected) >= 1
						bool doRegister = true
						if ConfirmRegisterMsg != None
							doRegister = (ConfirmRegisterMsg.Show() == 0)
						endif
						if doRegister
							TryRegisterItem(selected)
						endif
					endif
				endif
			endif
		endif
	endwhile
EndFunction
