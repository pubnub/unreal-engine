---
name: ue-pr-review
description: >-
  Review a pull request as a merge gate: blockers, other issues, and whether
  changes that require tests have test coverage. Use when the user asks for a
  PR review, merge review, or invokes this skill by name (for example "Use
  ue-pr-review") and specifies a PR link or the current branch. Applies the
  shared review rules plus the PubNub Unreal plugin rules in this repo.
---

# PR review

You are responsible for maintaining the repository to be safe. If such PR was added by your developer, would you merge it? If not what are the blockers?

Read-only. Do not edit files, commit, push, or post review comments unless the user asks.

## Parameter

The invocation must specify one target:

- a PR link (GitHub pull request URL, or `owner/repo#number`)
- the current branch

If neither is specified, stop and ask which one. Do not guess.

## Get the diff

Review the changes that would merge. Read enough surrounding code, call sites, and tests to judge each change. Do not review from commit subjects alone.

When the diff touches `Plugins/Pubnub` or `Plugins/PubnubChat`, read `AGENTS.md` and the matching file under `.ai/knowledge/` before judging those changes. `Source/SDKsTest` is the host project. Do not treat host-only edits as plugin work.

**PR link**

- `gh pr view <url> --json title,body,baseRefName,headRefName,files,additions,deletions`
- `gh pr diff <url>`
- Read changed files when the diff is not enough to judge correctness or tests.

**Current branch**

- Base is the repo default branch (`origin/HEAD`, usually `main` or `master`) unless the user named another base.
- `git status`, `git log <base>..HEAD`, `git diff <base>...HEAD` (three-dot / merge-base).
- Include uncommitted changes and say that they are included. Ignore untracked agent or editor files unless the user asked to include them.
- If the branch has no unique commits and no relevant uncommitted changes, say so and stop.

## What to check

Judge safety of the repository: correctness, security, data loss, API breakage, races, error handling, and whether the change does what it claims.

List blockers and other issues.

Verify if changes that require tests have the test coverage. A change requires tests when it changes behavior, fixes a bug, or adds a path that can fail. Missing coverage for that change is a blocker when the new or changed behavior is unverified. If existing tests already cover it, say which ones. Do not require new tests for comments, formatting, renames, or generated files.

Ignore lack of changing version or not updated changelog - these things are updated by the CI. Do not list them as blockers or other issues. That includes `VersionName`, `PubnubLibraryVersion.h`, `PubnubChatVersion.h`, and `.pubnub.yml`.

Flag an issue only when it was introduced by this change, is concrete, and the author would fix it. Skip speculative concerns, pre-existing problems, and style nits that do not obscure the code.

### Unreal plugins

Apply these only to changes in `Plugins/Pubnub` and `Plugins/PubnubChat`. Same bar as above: a blocker is a concrete defect this diff introduces.

- Public headers are the customer API. A new or edited public comment that names the C library, its types, or its functions is a blocker. Chat may name `UPubnubClient`. Do not flag comment length.
- A new customer-facing operation must follow the nearest peer: blocking name with no suffix, `Async` overloads, result struct, and the same init and empty-field macros. Core network work stays in `*_priv`. Chat has no `_priv`; its async path queues the sync method. A missing native-delegate overload, when the peer has one, is a blocker.
- Chat must keep calling the public core API. Including C headers from Chat, or calling `UPubnubClient::*Async` from the chat function thread, is a blocker.
- A `FUTF8StringHolder` (or other buffer the C call reads) that can die before the future is released is a blocker. So is `NewObject` from a background thread instead of `UPubnubInternalUtilities::SafeNewObject`.
- Core REST `_priv` holds `PubnubOperationMutex` until `pubnub_future_release`. Releasing it earlier, or leaving `InFlightFuture` set, is a blocker.
- Reordering existing `UENUM` values is a blocker. Adding a `SecretKey` setter, or changing crypto on a live client, is a blocker.
- A chat repository getter must copy under the lock. Storing a pointer or reference into a repository record is a blocker.
- `Subscribe` returning success means the subscription was registered, not that the handshake finished. A new receive test that publishes before `PSS_Connected` or `PSS_SubscriptionChanged` lists the target is a blocker. Chat tests must watch that on `UPubnubClient`, because chat drops `PSS_SubscriptionChanged`.
- `CppStandard`, `/std:c++20`, or a designated initializer is a blocker. The engine default standard must compile the change.
- Core behavior tests belong in `PubnubLibraryTests` (`FPubnubAutomationTestBase`, `PN_*` keys, `UE_SDK_` ids). Chat behavior tests belong in `PubnubChatSDKTests` (`FPubnubChatAutomationTestBase`, `PUBNUB_PUBLISH_KEY` / `PUBNUB_SUBSCRIBE_KEY`, `UE_ChatSDK_` ids). A test that uses the other plugin's keys or base class is a blocker.

## Output

Answer the merge question first.

```markdown
## Merge

Yes. / No.

<one or two sentences: why it is safe to merge, or what blocks it>

## Blockers

- `path/to/file:line` — what is wrong, in which scenario, and what must change before merge.

## Other issues

- `path/to/file:line` — non-blocking problem worth fixing.

## Tests

- What behavior changed and which tests cover it.
- Gaps that are blockers are repeated here with the test that is missing.
```

If a section has nothing, write `None.`

Order blockers by severity. Cite the smallest range that overlaps the diff.
