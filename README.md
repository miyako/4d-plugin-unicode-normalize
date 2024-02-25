NFC, NFD

# Basics

* The file system on macOS uses NFD normalisation
* The file system on Windows/Linux uses NFC normalisation
* It is important to convert NFD to NFC before sending string from macOS to other systems
* `<input type="file"/>` on Google Chrome does **not** normalise decomposed Unicode on macOS
* The composed/decomposed string on NFC will look identical but considered diffferent