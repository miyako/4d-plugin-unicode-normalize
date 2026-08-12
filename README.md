![version](https://img.shields.io/badge/version-17%2B-3E8B93)
![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm%20|%20win-64&color=blue)
[![license](https://img.shields.io/github/license/miyako/4d-plugin-unicode-normalize)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/4d-plugin-unicode-normalize/total)

# 4d-plugin-unicode-normalize

A 4D plugin that normalizes Unicode text using each platform's native normalization engine — `NormalizeString` on Windows, the `NSString` Unicode normalization category on macOS — and returns the result as `Text`. It exposes a single command, `Unicode normalize`, that converts a string to one of the four standard Unicode normalization forms (NFC, NFD, NFKC, NFKD).

| Command | Returns | Purpose |
|---|---|---|
| [`Unicode normalize`](#unicode-normalize) | Text | Normalize a string to a given Unicode normalization form |

**Platforms:** Windows, macOS

---

## Requirements & platform notes

- The command is declared `threadSafe` in the plugin manifest — it can be called concurrently from multiple 4D processes without additional synchronization on your side.
- The plugin never raises a 4D error. On any failure — an unrecognized normalization form, or an unexpected failure from the underlying OS call — it falls back to returning the **original, unmodified input text** rather than an empty string or an error. Always check the result if you need to confirm normalization actually happened (e.g. by comparing `Length` against the input, as the plugin's own test method does).
- **On Windows and macOS, an out-of-range/unrecognized form value is handled differently** — see the Description section below. Stick to the documented form constants to avoid depending on either fallback.
- The exact normalization-form constant names are confirmed from the plugin's own test method for NFC and NFD only (`Normalization Form NFC`, `Normalization Form NFD`); the NFKC/NFKD constants are inferred by naming symmetry with those two and haven't been directly verified against the plugin's shipped constant list — check your project's Explorer/constants list if `Normalization Form NFKC` or `Normalization Form NFKD` doesn't resolve.

---

## Unicode normalize

### Syntax

```4d
Unicode normalize ( text ; form ) → Text
```

| Parameter | Type | Description |
|---|---|---|
| `text` | Text | The string to normalize. |
| `form` | Longint | The target normalization form. Pass one of the plugin's normalization-form constants (see Description). |
| Result | Text | `text` converted to the requested normalization form, or the unmodified `text` if normalization couldn't be performed (see Requirements & platform notes). |

Both parameters are mandatory — the plugin reads them unconditionally, with no optional/overload form.

### Description

Unicode defines four standard normalization forms, and this command supports all four via its `form` parameter:

- **NFC** — canonical decomposition followed by canonical composition. Combines base characters and combining marks into precomposed characters where possible (e.g. `e` + combining acute → `é`).
- **NFD** — canonical decomposition only. Splits precomposed characters into base character + combining marks (e.g. `é` → `e` + combining acute), which is why a decomposed string is typically longer (in UTF-16 code units) than its composed equivalent.
- **NFKC** — compatibility decomposition followed by canonical composition. Like NFC, but also folds compatibility variants (e.g. certain typographic ligatures or width variants) into their standard forms.
- **NFKD** — compatibility decomposition only. Like NFD, but with the same compatibility folding as NFKC.

**On Windows**, the command calls `NormalizeString` directly with the requested form. If the form value doesn't correspond to a form `NormalizeString` recognizes, the call fails and the command returns the original `text` unchanged.

**On macOS**, the command maps the requested form to the matching `NSString` category method (`precomposedStringWithCanonicalMapping` for NFC, `decomposedStringWithCanonicalMapping` for NFD, `precomposedStringWithCompatibilityMapping` for NFKC, `decomposedStringWithCompatibilityMapping` for NFKD). An unrecognized form value is **not** passed through to a failing OS call the way it is on Windows — it silently falls back to the NFKC mapping instead of returning the text unchanged. In practice this only matters if you pass something other than the plugin's own constants for `form`; with the documented constants, both platforms behave identically.

### Example

From the plugin's own test method (`test.4dm`):

```4d
//%attributes = {}
$NFC:=Unicode normalize("Diplôme"; Normalization Form NFC)
ASSERT:C1129(Length:C16($NFC)=7)
$NFD:=Unicode normalize("Diplôme"; Normalization Form NFD)
ASSERT:C1129(Length:C16($NFD)=8)
```

This confirms, concretely, that composing `"Diplôme"` to NFC yields a 7-character string (the `ô` stays a single precomposed character), while decomposing to NFD yields 8 characters (`ô` splits into `o` + a combining circumflex).

Normalizing a batch of strings for consistent comparison or storage:

```4d
ARRAY TEXT($names; 0)
APPEND TO ARRAY($names; "Café")
APPEND TO ARRAY($names; "Café")  // one of these may use a precomposed é, the other e + combining acute

For ($i; 1; Size of array($names))
	$names{$i} := Unicode normalize($names{$i}; Normalization Form NFC)
End for

// $names now contains matching representations, safe to compare with "="
```

Round-tripping through NFD and back to NFC (useful when you need to strip combining marks, e.g. for accent-insensitive search, by discarding non-base characters after decomposing — the character-filtering step itself is ordinary 4D text handling, not shown here):

```4d
$decomposed:=Unicode normalize("Château"; Normalization Form NFD)
// ... filter out combining marks using your own logic ...
$recomposed:=Unicode normalize($decomposed; Normalization Form NFC)
```

---

## Error handling & troubleshooting

- **No 4D error is ever raised by this command.** Invalid input, an unrecognized `form` value, or an unexpected failure in the underlying OS call all fall back to returning the original `text` unchanged rather than raising an error or returning an empty string. If your code depends on normalization having actually happened, verify it explicitly (e.g. compare `Length` before/after, as the plugin's own test does), rather than relying on the absence of a 4D error as confirmation.
- **An unrecognized `form` value behaves differently per platform.** Windows returns the original text unchanged; macOS instead applies NFKC-style normalization by default. This only affects you if `form` comes from something other than the plugin's documented constants (e.g. a raw computed Longint) — with the documented constants there's no divergence to worry about.
- **`Normalization Form NFKC` and `Normalization Form NFKD` are inferred, not directly confirmed**, from the plugin's constant-naming pattern (only NFC/NFD appear in the shipped test method). If either name doesn't resolve in your project, check the plugin's constants list in the Explorer.

---

## Quick reference

```4d
$s1:=Unicode normalize("Diplôme"; Normalization Form NFC)   // precomposed, shorter
$s2:=Unicode normalize("Diplôme"; Normalization Form NFD)   // decomposed, longer

If ($s1=$s2)
	// same underlying text, different representation - shouldn't happen after NFC vs NFD
End if
```
