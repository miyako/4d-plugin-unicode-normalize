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

## Manual versioning and publication

The Actions workflows are independent:

- **Bump version**: select a branch with GitHub's built-in ref selector and choose
  `patch`, `minor`, or `major`. It checks out the immutable dispatch SHA, requires
  that exact remote branch tip and a clean checkout, then makes one nonempty,
  source-only, single-direct-parent commit with the Copilot co-author trailer.
  An executable pre-push guard checks the server-advertised old SHA/ref, literal
  new SHA, HEAD, parent and exactly one update (including rejecting an
  already-up-to-date race). The guard is required initially and immediately before
  a non-force push with both `push.followTags=false` and `--no-follow-tags`.
  It never tags, releases, builds, or dispatches another workflow.
- **Build**: select a branch or tag with the built-in ref selector. There are
  **no custom inputs**. Every job checks out `github.sha`; validated source
  metadata supplies the version and `vX.Y.Z` release name. A constant concurrency
  group serializes publication across refs without cancellation. Bump is not a
  prerequisite: Build can use any selected commit whose metadata validates and
  whose version is still unpublished.

### Source authority and intake repair

The baseline is `3a4fb27331dd1fa58e7e7444e29978e29160c1d0` (`main`).
Rename-follow Bump history starts at `7269b05`, whose header describes
marketing-first/current-fallback behavior. The later `5bb7d68` header explicitly
coordinates **MARKETING_VERSION, CURRENT_PROJECT_VERSION and Info.plist's
CFBundleShortVersionString/CFBundleVersion**. That newer explicit contract
governs, despite the old body's marketing-only bug. Independent build numbers
from the earlier fallback contract are not the current policy.

Intake does **not** increment `1.0.4`. All six marketing settings and the source
plist already agree on `1.0.4`; only the six stale current settings (`1.0.2`) and
source English display strings (`1.0.0`) are repaired to that existing authority.
`Info.plist` itself remains unchanged. `CFBundleShortVersionString` is required;
`CFBundleVersion` is optional and its absence is preserved. Supported
`$(MARKETING_VERSION)`, `${MARKETING_VERSION}`, `$(CURRENT_PROJECT_VERSION)` and
`${CURRENT_PROJECT_VERSION}` references are resolved and preserved; current
settings may reference marketing, but cycles/unknown references fail.

`.github/scripts/source_version.py` validates strict canonical `X.Y.Z`, drift,
missing/duplicate settings and localized display metadata before any writes.
Lexical project validation rejects quoted/unquoted inline duplicates, ignoring
comments and string noise. Only literal version tokens are edited; UTF-8/UTF-16
BOMs and endianness, individual mixed line endings, modes and unrelated bytes
are preserved. Prebuilt bundle metadata/localization, dependencies, manifest and
4D/schema counters are not version sources and are not changed by Bump.

### Native build and create-only release

The existing Windows Release/x64 build retains v143 and Windows SDK 10.0.
Its exact `.vcxproj` OutDir is
`test/plugins/Unicode Normalize.bundle/contents/windows64/Unicode Normalize.4DX`;
only that nonempty output is normalized/uploaded, never a recursive first match.
The macOS Default target retains `arm64 x86_64`, the workflow's 11.0 deployment
override, frameworks, resource phases and signing secret names. Output is
explicitly directed to `test/Plugins`, matching the actual `test/Project`
layout (not a presumed `.4dbase`). Cleanup invalidates generated executable,
plist, copied English strings, signatures and named intermediates, including
their Default sibling locations, without deleting bundles, Windows binaries,
resources or dependency directories.

The rebuilt binary must contain the exact `arm64` and `x86_64` tokens, not
`arm64e`/`x86_64h` substitutes. There is no new exact-slice-count or no-extra-slice
restriction. Built plist and localized versions are checked against selected
source, including XML/binary plist and UTF-16 strings. Signing retains deep
strict verification. Both ZIP and DMG submissions must exit successfully and
return JSON `Accepted`; stapling must then succeed.

Before native work and again **after all build/sign/notary/staple gates**, an
authenticated, fully paginated releases query checks every release, including
drafts, and an exact remote-tag query requires absence. HTTP 404, authorization,
network, malformed data and later-page failures all stop publication.
Publication then uses a separate create-only `POST git/refs` at the selected SHA,
verifies that exact remote SHA, and `POST`s a **new public, non-prerelease** release
with explicit tag/name/target/body/flags. The original empty-body/no-generated-notes
policy is retained. The exact ZIP and DMG are uploaded separately to the newly
returned release ID. Nothing is reused, updated, clobbered, retried or rolled back;
there is no combined create-with-assets operation.

### Partial-publication recovery

On failure, retain the Actions log, selected SHA, tag, returned release ID,
notary submission IDs and any uploaded asset names. A failed HTTP response can
also mean the server accepted a mutation before the response was lost: inspect
remote state before taking any action. The workflow deliberately leaves a
created tag, release and earlier assets intact as evidence, and a rerun of an
occupied version fails closed.

A maintainer must investigate and decide any manual recovery; this automation
never deletes/repairs partial state or adopts an existing release. To publish
another version, use Bump on the intended branch, then manually run Build on
the resulting commit. Do not retry publication blindly or move a production tag.

### Offline verification

Run `actionlint` for **all** workflows and
`PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s .github/tests -v`.
PowerShell (`pwsh`) is required, not skipped. Tests use source-derived fixtures,
disposable local Git/bare origins, mocked API faults/races and literal extracted
workflow Bash/PowerShell blocks with a closed PATH of native stubs. No real
native build, signing, notarization, 4D, GitHub dispatch or production
tag/release mutation is needed. The same full suite supports a genuinely
guarded source-only bump in a disposable clone, without changing this branch's
intake version.

Reference adaptations were reviewed at the exact pinned commits:
`miyako/4d-plugin-native-encoding-detection@c42ca565857f674bde5ea6ed3435c87375eda4a7`,
`miyako/4d-plugin-file-information@19848973601acfd3a2b5070230fa40cdc7e4edb2`,
`miyako/4d-plugin-common-string-algorithm@2606a68201866ec9fcbb38b51462d79215f504ef`,
`miyako/4d-plugin-window-button@3f14facb9aaaaff0e22eeb8e07531163680c3a8a`,
`miyako/4d-plugin-raw-picture-data@8144537fdc8dcc3d0c053dfe97aff0a8c0c3a6f9`,
and `miyako/4d-plugin-PRNG@522f62e1e9f388be0f3f226c1d4c39362dad2491`.
