//%attributes = {}
$NFC:=Unicode normalize("Diplôme"; Normalization Form NFC)
ASSERT:C1129(Length:C16($NFC)=7)
$NFD:=Unicode normalize("Diplôme"; Normalization Form NFD)
ASSERT:C1129(Length:C16($NFD)=8)