NFC, NFD

```4d
$NFC:=Unicode normalize("Diplôme"; Normalization Form NFC)
ASSERT(Length($NFC)=7)
$NFD:=Unicode normalize("Diplôme"; Normalization Form NFD)
ASSERT(Length($NFD)=8)
```

# Basics

* The file system on macOS uses NFD normalisation
* The file system on Windows/Linux uses NFC normalisation
* It is important to convert NFD to NFC before sending string from macOS to other systems
* `<input type="file"/>` on Google Chrome does **not** normalise decomposed Unicode on macOS
* The composed/decomposed string on NFC will look identical but considered diffferent

# References

* [NormalizeString](https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-normalizestring)
* [precomposedStringWithCanonicalMapping](https://developer.apple.com/documentation/foundation/nsstring/1412645-precomposedstringwithcanonicalma)
