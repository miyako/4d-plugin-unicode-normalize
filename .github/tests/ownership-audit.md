# Unicode Normalize native ownership audit

## Provenance and complete tracked-source inventory

Baseline: `80661611ff133c52a1422edf3bd6627b323d21a6` (`main`).
Canonical guidance: `miyako/skills@e6b492fcb81e5e1a7f7db723594b420e63bf6eb9`,
`4d-skills/skills/4dplugin/SKILL.md`, read through the immutable GitHub API.
Issue: #2.

Both source and committed bundle manifests are blob
`5e349a83e92af9049ffaab92e42978908b15cf8a`: the single thread-safe command
`Unicode normalize(&T;&L):T` has scalar text/longint inputs and a text result.
Neither declares an array. This audit includes internal/dormant native code,
not just the command signature.

| Tracked native surface | Membership and finding |
|---|---|
| `4DPlugin-Unicode-Normalize.cpp/.h` | Xcode and all four MSBuild configurations. Selector 1 copies input into C_TEXT and writes the result through the content setter; other selectors do nothing. No owned PA descriptor is shared with the NSString/Windows normalization buffers. Existing normalization and exception behavior is untouched. |
| `4D Plugin API/Classes/C_TEXT.cpp/.h` | The only wrapper compiled by either project. Independent C++ string storage; already noncopyable. Parameter/result setters call PA_SetUnistring, not the descriptor-transfer array setter. UTF-8 conversion uses explicit byte and UTF-16 unit counts. Objective-C path methods are intercepted in the isolated host. |
| `Classes/ARRAY_TEXT.cpp/.h` | Tracked but absent from both projects. Copies borrowed elements into C++ strings before mutation. Accepted insertions already transfer without post-transfer disposal. Its raw owned vector exposed implicit shallow copy/assignment; now noncopyable, matching C_TEXT. |
| `Classes/C_BLOB.cpp/.h` | Tracked but absent from both projects. CBytes holds bytes by value; C_BLOB owns a raw CBytes pointer. Implicit wrapper copy/assignment is now disabled. Parameter replacement now allocates/fills before disposing the old owner, preserving it on allocation failure. |
| `Classes/C_LONGINT.cpp/.h`, `C_TYPES.h` | Dormant value-only integer wrapper; shared byte/UTF-16 string aliases. All four actual wrappers compile and execute in the host. |
| `support/4DPlugin-JSON.cpp/.h` | Tracked but absent from both projects and not called by the dispatcher. Four string setters and one picture setter lost owning empties. Four property getters leaked successful owning results/mismatches or inspected failed unspecified values. Stringify needed separate command error/return ownership handling. Matching collection handoff and object/collection setter transfers are preserved. No object getter exists in this target. |
| Complete `4DPluginAPI.c`, API/ABI headers, `.def` | Compiled whole in both projects and the host, never replaced with fragments. Source says v14, header v13; comments are not independently verified SDK release identifiers. The export definition exposes the SDK entry point, not another application wrapper. SDK-internal synchronous command aliases remain unchanged. |
| `include/json/*.h`, `a/libjsoncpp.a`, tracked Windows libraries | Vendor JSON headers/opaque libraries, not alternate PA wrappers; no current native consumer includes JsonCpp. Binaries are untouched and not behavior-certified. |
| Committed plugin bundles, ZIP/DMG, `build/`, `test/Project` methods/resources | Prebuilt artifacts and 4D fixture project are not executed or modified. Native test inputs here are public literal Unicode only. |

No CPicture/picture wrapper, picture setBytes/createCopy converter,
pointer-reference wrapper constructor, additional application callback helper,
worker, or native implementation is present in the complete tracked native
source/build inventory. `ob_set_p` is the only application picture helper.
CBytes::setBytes copies bytes into a vector and does not create pictures.
No production copy call is claimed for the two dormant wrappers; their defects
are latent, demonstrated with explicit named probes rather than inferred from
search absence.

SDK blobs, enforced before every host build:

| File in `4D Plugin API/` | Git blob |
|---|---|
| `4DPluginAPI.c` | `62f745e91d81ef2c3c901c656ea8e175d3b173e8` |
| `4DPluginAPI.h` | `528bf1d2552df08d6a8b1bc95011a9b72934ad5c` |
| `4DPluginAPI.def` | `5bbdb82fb724b6f8e307feb01d9f158b802de89d` |
| `EntryPoints.h` | `da428d5f6ca23b0a30371c7288a01d29aa3c466a` |
| `Flags.h` | `7a5a35539f27430a3fd9cb711d6b789051316471` |
| `PrivateTypes.h` | `a3098cb662cdbc43ba574fdef72a059c4c6d1cd4` |
| `PublicTypes.h` | `9ae20b001bff9128f690fff3e6c3f873b6238333` |

Application/wrapper baseline blobs:

| File | Git blob |
|---|---|
| `4DPlugin-Unicode-Normalize.cpp` | `fcccfc8923af4fb8c69f5a56f9a0b64d238cf89d` |
| `4DPlugin-Unicode-Normalize.h` | `4dcecdb9feab41f2dab220a716bccfe9b55143a5` |
| `support/4DPlugin-JSON.cpp` | `74c880fb70dccb2d707fc8372880ff5c3492de41` |
| `support/4DPlugin-JSON.h` | `3e1c00dfc1e62766e9b235576ffd4621da057022` |
| `Classes/ARRAY_TEXT.cpp` | `534855e0959020ce9a66cef08f8434799c174495` |
| `Classes/ARRAY_TEXT.h` | `cf8a458fc8519b824c726a59d346c44d4c26e845` |
| `Classes/C_BLOB.cpp` | `27bea3172ce90e4da339991c9b318a71ea8609b2` |
| `Classes/C_BLOB.h` | `d181534e5a2e9bd46de492a040c1538ed81bf45a` |
| `Classes/C_LONGINT.cpp` | `005b83a4aec2c1466b8b219cb27ab845eb58de91` |
| `Classes/C_LONGINT.h` | `71e24df633aed7155cba1ffddc07d6a1dfc12fa9` |
| `Classes/C_TEXT.cpp` | `45ea6432dff52b681a5c6a7cea58ad5fc15df66b` |
| `Classes/C_TEXT.h` | `6967390757db569ffcd3cfea8696bf14045cc748` |
| `Classes/C_TYPES.h` | `972cf3f597a8c21881a8c9aa69bc1699455be373` |

## Exact ownership contracts and surgical changes

SDK CreateVariable (3920-4066) creates an owned empty string/picture; scalar
setters (4254-4259, 4280-4285) shallow-assign without releasing it. The five
helper destinations now start Undefined. Final ClearVariable still disposes
the transferred input; this is not blanket removal of scalar cleanup.
Object/collection initial null references do not have this empty-owner leak.

GetObjectProperty (636-651) and GetCollectionElement (586-600) explicitly
return owning variables to clear. This target has four property getters and
no application collection-element wrapper. Each now captures error immediately
before kind inspection/adoption, clears every successful non-handed-off result,
and preserves the matching owning collection handoff. Failed returns can be
unspecified/poisoned or raw fType 0; UnifyVarKind (1713-1735) maps raw 0 to
Unistring, so even a kind check before error capture is unsafe.

Stringify invokes the same OB Copy command directly, preserving the returned
descriptor until its error and kind are checked. Successful mismatches are
cleared; failed unspecified values are not adopted. The successful copied
object is an independent synchronous argument, cleared after stringify while
its error is captured first. Successful stringify returns are separately cleared
after use (including mismatches); failed unspecified returns are not read/cleared.
The original borrowed object argument to OB Copy is not cleared.

Existing failure outputs remain: text unchanged, boolean/number/reference
getters false/zero/null; ob_get_a still reports property existence. There is no
new exception, worker throw, error channel or success-shaped fallback. These
dormant helpers have no error-reporting interface and cleanup can reset SDK last
error: this remains an explicit observability gap, not claimed error reporting.

SetStringInArray (4933-4947) accepts Unicode kind, non-null data, index
0..fNbElements inclusive and non-null descriptor pointer; it disposes the old
element then shallow-transfers the input. Rejection retains caller ownership.
GetStringInArray (4738-4755) is borrowed. Accepted index zero, rejected
kind/index/data/descriptor, independent copies, repeated replacement, churn,
shrink and final cleanup are controls; the already-correct success transfer is
unchanged. PA_SetUnistring (289-299) forwards content to the engine instead.

## Isolated reproduction and measured results

Run only the narrow unsigned checks:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 .github/tests/test_ownership.py
PYTHONDONTWRITEBYTECODE=1 python3 .github/tests/test_repository_contract.py
actionlint .github/workflows/ownership.yml
```

The host compiles the complete unchanged SDK, all four actual wrapper sources,
the entire actual JSON helper and actual normalization dispatcher. ASan/UBSan
are enabled, excluding alignment for packed SDK ABI structs; macOS LSan is
disabled and explicit string/picture/handle/object/collection/platform owner
balances must all reach zero.

Forced-include stubs replace NSURL, CFURL path functions and SDK graphics
transforms before compilation. Only in-memory Foundation text normalization
is used. Unexpected engine/URL selectors fail closed. Before dispatch, nm
rejects real path/graphics/UI/system/filesystem/network imports and Foundation
classes outside NSObject/NSString/NSMutableString. No private documents,
settings, user files, discovery, platform file operation, UI, install, or real
4D host is accessed.

Every actual application consumer also receives forced-Windows-branch
syntax-only compilation, including _WIN32 UTF conversion branches, with
trap-only NormalizeString/conversion declarations. This is not a Windows SDK,
Windows wchar_t/ABI validation, Windows execution, or a reproduced 4D crash.

The same final harness was run against immutable sources extracted with
`git archive 80661611ff133c52a1422edf3bd6627b323d21a6` (no additional checkout),
selecting that directory with OWNERSHIP_SOURCE_ROOT. Baseline-only unsafe
copy/assignment probes additionally define OWNERSHIP_BASELINE_COPY=1.

| Group | Common cases | Baseline failures | Fixed failures |
|---|---:|---:|---:|
| Four string setters and picture setter | 33 | 17 | 0 |
| Four property getters | 64 | 39 | 0 |
| Stringify/copy success, mismatch, failure | 23 | 10 | 0 |
| Actual dispatcher, wrappers, arrays, failure predicates, controls, traits | 36 | 2 | 0 |
| Total common | 156 | 68 | 0 |
| Baseline-only unsafe copy/assignment | 4 | 4 ASan UAF | Not compiled on noncopyable fixed types |

Baseline: 160 runs, 88 pass, 72 fail. Fixed: 156 pass, zero failures.
Five preservation tests also pass. ASCII, BMP, supplementary surrogate pairs
and empty text cover both allocated and null engine empties, exact UTF-16 units,
UTF-8 bytes, capacity, termination and independent copied content. Fourteen
actual dispatcher cases execute 70 normalization calls: all four documented
forms plus representable unknown form 3 (existing macOS NFKC default), composed
and decomposed e-acute, compatibility ligature fi, and unchanged text.

Named failures include unreleased string/picture/reference owners,
`caller handle released after failed allocation`, invalid-kind UBSan,
`reading failed/non-string variable`, `stringify dead argument`, and four
ASan heap-use-after-free copy probes. Matching owning collection handoff,
null-empty setters, synchronous argument independence and normalization are
passing baseline controls, not evidence that allocated empties are safe.
Immutable logs/results are retained in session artifacts, not tracked binaries.

## Deferred contracts and non-release boundary

SDK PA_ResizeArray (4566-4603) updates fNbElements even when SetHandleSize
fails; shrink may dispose elements before resize succeeds. ARRAY_TEXT trusts
that metadata. Wrapper array OOM/capacity preservation requires separate
SDK/host contract work and is explicitly not certified. Tests prove low-level
allocation/resize failure predicates, not array-wrapper OOM safety. Failed
string creation/unspecified host metadata, arbitrary C++ allocation exceptions,
and opaque vendor binaries remain outside the verified contract.

An additional unchanged-source probe with normalization form 999 hit enum-range
UBSan before normalization; that pre-existing out-of-range enum problem is
outside this ownership patch. Its logs are retained separately. The final
preservation fixture uses representable non-enumerator 3; arbitrary out-of-range
form safety is not claimed or hidden by disabling enum sanitization.

Existing workflows/scripts and project phases were read before validation.
Build and Bump version are manual-only. Xcode Deployment has signing,
packaging and notarization/upload effects; its launch action targets real 4D.
No project build, launch, tag, release, signing, notarization, binary packaging,
publication, manual dispatch, rerun or dependency installation is performed.
Existing legacy Pages is configured main:/; triggering its deployment requires
separate coordinator/user permission.

The new Ownership workflow is unsigned, read-only, path-filtered, push-only
on the exact audit branch and main. Its full-history checkout supports the
existing immutable-history preservation tests. Only the four explicitly fixed
native files were added to that test's existing allowlist. SDK/ABI headers,
manifests, projects, release workflows, version metadata and binaries are
unchanged.

Harness structure was adapted from immutable
`miyako/4d-plugin-trash@2518783ce8ec6b8923e35d5cf93b4fcbc363860d`.
This target's four-getter inventory, all-consumer Windows syntax gate,
normalization fixtures and case counts were obtained here, not copied results.
