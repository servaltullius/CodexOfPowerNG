Scriptname JValue Hidden

; Lifetime management / validation (JContainers)
Bool Function isExists(Int object) Global Native

Int Function retain(Int object, String tag = "") Global Native

; JSON serialization/deserialization
Int Function readFromFile(String filePath) Global Native

; Path resolving
String Function solveStr(Int object, String path, String default = "") Global Native

Int Function solveInt(Int object, String path, Int default = 0) Global Native

Int Function solveObj(Int object, String path, Int default = 0) Global Native
