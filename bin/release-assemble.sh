#!/usr/bin/env bash
#
# release-assemble.sh - collect the CI build artifacts of a release and
#                       publish them as GitHub release assets.
#
# Every binaries-*.yml workflow publishes its archives as one artifact per
# matrix entry ("Release-...", plus "appimage-..." from binaries-linux.yml).
# This script finds the successful run of each workflow for the release
# commit, downloads those artifacts, verifies the SHA256 files the build
# wrote next to the archives, and uploads everything - archives, checksums,
# AppImages and the installer scripts - to the release of that tag.
#
# Synopsis:
#     bin/release-assemble.sh <tag> [options]
#
#     bin/release-assemble.sh v09.12.26 --prerelease
#     bin/release-assemble.sh v09.12.26 --dry-run
#     bin/release-assemble.sh v09.12.26 --dispatch --wait
#
# Options:
#     --prerelease        create the release as a prerelease
#     --notes-file FILE   release notes to use when the release is created
#     --dry-run           only report what would be uploaded
#     --workdir DIR       download directory (default: a fresh temp dir)
#     --repo OWNER/NAME   repository (default: the current git remote)
#     --dispatch [REF]    dispatch all binaries-* workflows on REF (master)
#     --wait              wait until every workflow for the commit finished
#     --timeout MINUTES   give up waiting after this many minutes (default 90)
#
# Exit status is non-zero when a workflow run is missing or failed, a
# checksum does not verify, an archive is empty, or an upload fails - a
# release assembled with this script is complete or it does not happen.

set -u

TAG=""
PRERELEASE=0
NOTES_FILE=""
DRY_RUN=0
WORKDIR=""
REPO=""
DISPATCH=0
DISPATCH_REF="master"
WAIT=0
TIMEOUT_MIN=90

while [ $# -gt 0 ]; do
    case "$1" in
        --prerelease)   PRERELEASE=1 ; shift ;;
        --notes-file)   NOTES_FILE="$2" ; shift 2 ;;
        --dry-run)      DRY_RUN=1 ; shift ;;
        --workdir)      WORKDIR="$2" ; shift 2 ;;
        --repo)         REPO="$2" ; shift 2 ;;
        --dispatch)
            DISPATCH=1
            case "${2:-}" in
                ""|-*) ;;
                *) DISPATCH_REF="$2" ; shift ;;
            esac
            shift ;;
        --wait)         WAIT=1 ; shift ;;
        --timeout)      TIMEOUT_MIN="$2" ; shift 2 ;;
        -h|--help)      sed -n '2,40p' "$0" | sed 's/^# \{0,1\}//' ; exit 0 ;;
        -*)             echo "Error: unknown option '$1'" >&2 ; exit 2 ;;
        *)
            if [ -z "$TAG" ]; then TAG="$1"; shift
            else echo "Error: unexpected argument '$1'" >&2 ; exit 2
            fi ;;
    esac
done

if [ -z "$TAG" ]; then
    echo "Error: no tag given" >&2
    echo "Usage: $0 <tag> [--prerelease] [--dry-run] [--dispatch [ref]] [--wait]" >&2
    exit 2
fi

## ---------------------------------------------------------------- helpers

function die {
    echo "Error: $*" >&2
    exit 1
}

function note {
    echo "== $*"
}

## The workflows that contribute release assets. Any other workflow
## (performance, testing, binaries-winlibs) produces nothing that belongs in
## a release: binaries-winlibs only uploads unpacked MicroEmacs09-*-Windows
## directories without archives or checksums, the Windows release archives
## come from binaries-linuxmingwgcc and binaries-msys2.
WORKFLOWS="binaries-linux
binaries-almalinux
binaries-alpine
binaries-arch
binaries-debian-i686
binaries-debian10
binaries-macosgcc
binaries-msys2
binaries-linuxmingwgcc
binaries-cygwin
binaries-freebsd"

## Artifacts worth downloading. The per-variant artifacts (individual
## binaries, brew/scoop archives) are packaging by-products, not release
## assets; only the "Release-..." bundles and the AppImages are published.
ARTIFACT_PATTERN='^(Release-|appimage-)'

## Workflows that must build successfully but whose archives are not
## published. binaries-linuxmingwgcc cross-compiles the same native Windows
## targets that binaries-msys2 produces (windows-mingw-*), only under the
## names windows-mingw32/windows-mingw64, which no installer refers to - so
## it stays a build check and its artifacts stay out of the release.
BUILD_ONLY="binaries-linuxmingwgcc"

if [ -z "$REPO" ]; then
    REPO=$(gh repo view --json nameWithOwner -q .nameWithOwner 2>/dev/null)
    [ -n "$REPO" ] || die "cannot determine the repository, use --repo OWNER/NAME"
fi

## Resolve the commit of the tag. Prefer the remote so a stale or missing
## local tag cannot point at the wrong build.
SHA=$(gh api "repos/${REPO}/commits/${TAG}" --jq .sha 2>/dev/null | grep -E '^[0-9a-f]{40}$')
[ -n "$SHA" ] || die "cannot resolve tag '${TAG}' in ${REPO} (has it been pushed?)"
note "tag ${TAG} -> ${SHA}"

## --------------------------------------------------------------- dispatch

if [ $DISPATCH -eq 1 ]; then
    note "dispatching ${WORKFLOWS//$'\n'/ } on ${DISPATCH_REF}"
    for wf in $WORKFLOWS; do
        if gh workflow run "${wf}.yml" --repo "$REPO" --ref "$DISPATCH_REF" >/dev/null 2>&1; then
            echo "   ${wf}"
        else
            die "could not dispatch ${wf}.yml on ${DISPATCH_REF}"
        fi
        sleep 2
    done
    WAIT=1
fi

## ------------------------------------------------------------------- wait

## The workflow runs are listed per commit, so this also picks up runs that
## were started by hand before the script was called.
function runs_for {
    gh run list --repo "$REPO" --workflow "$1.yml" --commit "$SHA" --limit 30 \
        --json databaseId,status,conclusion,createdAt \
        --jq '[.[] | select(.conclusion == "success")] | sort_by(.createdAt) | reverse | .[0].databaseId // ""'
}

if [ $WAIT -eq 1 ]; then
    note "waiting up to ${TIMEOUT_MIN} minutes for the workflows of ${SHA:0:7}"
    deadline=$(( $(date +%s) + TIMEOUT_MIN * 60 ))
    while : ; do
        pending=""
        for wf in $WORKFLOWS; do
            state=$(gh run list --repo "$REPO" --workflow "${wf}.yml" --commit "$SHA" --limit 30 \
                --json status,conclusion \
                --jq '[.[] | select(.status != "completed")] | length')
            if [ "${state:-0}" != "0" ]; then
                pending="${pending} ${wf}"
            fi
        done
        if [ -z "$pending" ]; then
            note "all workflows completed"
            break
        fi
        if [ "$(date +%s)" -ge "$deadline" ]; then
            die "timeout after ${TIMEOUT_MIN} minutes, still running:${pending}"
        fi
        echo "   waiting for:${pending}"
        sleep 30
    done
fi

## -------------------------------------------------------------- download

if [ -z "$WORKDIR" ]; then
    WORKDIR=$(mktemp -d "${TMPDIR:-/tmp}/me-release-${TAG}.XXXXXX")
fi
## Start from an empty download area: "gh run download" refuses to extract
## over existing files, so a reused workdir would fail on the second run.
rm -rf "$WORKDIR/raw" || die "cannot clean $WORKDIR/raw"
mkdir -p "$WORKDIR/raw" || die "cannot create $WORKDIR/raw"
note "download directory ${WORKDIR}"

missing=""
for wf in $WORKFLOWS; do
    run=$(runs_for "$wf")
    if [ -z "$run" ]; then
        missing="${missing} ${wf}"
        echo "   ${wf}: no successful run for ${SHA:0:7}"
        continue
    fi
    echo "   ${wf}: run ${run}"
    case " $BUILD_ONLY " in
        *" $wf "*)
            echo "   ${wf}: build check only, no release assets"
            continue ;;
    esac
    ## Download only the release bundles and AppImages of that run.
    names=$(gh api "repos/${REPO}/actions/runs/${run}/artifacts?per_page=100" \
        --jq '.artifacts[] | select(.expired == false) | .name' 2>/dev/null |
        grep -E "$ARTIFACT_PATTERN")
    if [ -z "$names" ]; then
        missing="${missing} ${wf}(no artifacts)"
        echo "   ${wf}: no release artifacts in run ${run}"
        continue
    fi
    for name in $names; do
        gh run download "$run" --repo "$REPO" -n "$name" -D "$WORKDIR/raw" >/dev/null 2>&1 ||
            die "cannot download artifact '${name}' of ${wf} (run ${run})"
    done
done
[ -z "$missing" ] || die "incomplete build, missing:${missing}"

## --------------------------------------------------------------- collect

## Flatten to the files that go into the release. Directory names are kept
## in the manifest only; the release itself is a flat asset list.
STAGE="$WORKDIR/assets"
rm -rf "$STAGE" || die "cannot clean $STAGE"
mkdir -p "$STAGE" || die "cannot create $STAGE"

find "$WORKDIR/raw" -type f \( -name '*.zip' -o -name '*.sha256' -o -name '*.AppImage' \) |
while read -r f; do
    base=$(basename "$f")
    if [ -e "$STAGE/$base" ]; then
        ## The same archive must never be shipped twice with different
        ## content - that would make the checksum file ambiguous.
        if ! cmp -s "$f" "$STAGE/$base"; then
            echo "Error: conflicting copies of ${base}" >&2
            echo "       $f" >&2
            exit 1
        fi
        continue
    fi
    cp "$f" "$STAGE/$base" || exit 1
done || exit 1

## --------------------------------------------------------------- installers

## Take the installer scripts from the tagged commit, not from the working
## tree, so the published scripts belong to the published binaries.
for f in install.sh install-windows.ps1 install-fonts.sh install-dict.sh; do
    if git cat-file -e "${TAG}:bin/${f}" 2>/dev/null; then
        git show "${TAG}:bin/${f}" > "$STAGE/$f" || die "cannot read bin/${f} from ${TAG}"
    elif [ -f "bin/$f" ]; then
        cp "bin/$f" "$STAGE/$f"
    else
        die "cannot find bin/${f} in ${TAG} or the working tree"
    fi
done

## -------------------------------------------------------------- verify

note "verifying checksums"
fail=0
for sha in "$STAGE"/*.sha256; do
    [ -e "$sha" ] || continue
    if ! ( cd "$STAGE" && sha256sum -c "$(basename "$sha")" >/dev/null 2>&1 ); then
        echo "   FAILED: $(basename "$sha")" >&2
        ( cd "$STAGE" && sha256sum -c "$(basename "$sha")" 2>&1 | grep -v ': OK$' >&2 )
        fail=1
    fi
done
[ $fail -eq 0 ] || die "checksum verification failed, nothing was uploaded"

empty=$(find "$STAGE" -name '*.zip' -size 0)
[ -z "$empty" ] || die "empty archives found: ${empty}"

## Every archive should have a checksum next to it. Count them so a missing
## .sha256 (a workflow that forgot to write it) is noticed here and not by
## a user downloading the release.
zips=$(find "$STAGE" -name '*.zip' | wc -l)
sums=$(find "$STAGE" -name '*.sha256' | wc -l)
covered=$(cat "$STAGE"/*.sha256 2>/dev/null | grep -c '\.zip$')
note "${zips} archives, ${sums} checksum files covering ${covered} archives"
[ "$covered" -ge "$zips" ] || die "only ${covered} of ${zips} archives have a checksum"

## The installer derives the version from the tag (v09.12.26 -> 091226) and
## looks for archives carrying exactly that string. A release whose binaries
## still say 091226b7 would therefore be invisible to it - catch that here.
TAGV=$(echo "$TAG" | sed -E 's/^.*v//; s/\.//g; s/beta/b/; s/[^0-9b]+//g')
note "tag version ${TAGV}"
for f in $(find "$STAGE" -maxdepth 1 -type f \( -name '*.zip' -o -name '*.AppImage' \)); do
    case "$(basename "$f")" in
        *"${TAGV}"*) ;;
        *) die "$(basename "$f") does not carry the tag version ${TAGV} - is src/evers.h bumped?" ;;
    esac
done

## Every supported platform family must be represented, otherwise a workflow
## silently produced nothing (or its artifacts were skipped).
for family in ubuntu almalinux alpine arch archarm i686-debian debian-10 macos windows cygwin freebsd; do
    if ! find "$STAGE" -maxdepth 1 -name "*.zip" | grep -q -- "$family"; then
        die "no archives for '${family}' - did that workflow run for ${SHA:0:7}?"
    fi
done
note "all platform families present"

## ---------------------------------------------------------------- upload

files=$(find "$STAGE" -maxdepth 1 -type f | sort)
count=$(echo "$files" | grep -c .)
total=$(du -sh "$STAGE" | cut -f1)
note "${count} assets, ${total}"

if [ $DRY_RUN -eq 1 ]; then
    echo "$files" | sed 's#^.*/#   #'
    note "dry run, nothing uploaded"
    exit 0
fi

if gh release view "$TAG" --repo "$REPO" >/dev/null 2>&1; then
    note "release ${TAG} exists, uploading with --clobber"
else
    args=(--repo "$REPO" --title "$TAG")
    [ $PRERELEASE -eq 1 ] && args+=(--prerelease)
    ## Only let gh create the tag when it is not there yet; pointing --target
    ## at an existing tag is an error.
    if ! gh api "repos/${REPO}/git/ref/tags/${TAG}" >/dev/null 2>&1; then
        args+=(--target "$SHA")
    fi
    if [ -n "$NOTES_FILE" ]; then
        args+=(--notes-file "$NOTES_FILE")
    else
        args+=(--notes "Automated build of ${TAG}.")
    fi
    note "creating release ${TAG}"
    gh release create "$TAG" "${args[@]}" >/dev/null || die "cannot create release ${TAG}"
fi

## Upload in batches: a few hundred file arguments in one call is fragile and
## gives no progress feedback on a slow connection.
batch=""
n=0
uploaded=0
for f in $files; do
    batch="$batch $f"
    n=$((n + 1))
    if [ $n -ge 20 ]; then
        gh release upload "$TAG" $batch --repo "$REPO" --clobber >/dev/null ||
            die "upload failed for:${batch}"
        uploaded=$((uploaded + n))
        echo "   uploaded ${uploaded}/${count}"
        batch=""
        n=0
    fi
done
if [ -n "$batch" ]; then
    gh release upload "$TAG" $batch --repo "$REPO" --clobber >/dev/null ||
        die "upload failed for:${batch}"
    uploaded=$((uploaded + n))
fi

## --------------------------------------------------------------- confirm

published=$(gh release view "$TAG" --repo "$REPO" --json assets --jq '.assets | length')
note "${published} assets published on ${TAG} (uploaded ${uploaded})"
[ "$published" -ge "$count" ] || die "release lists ${published} assets, expected at least ${count}"
note "done: https://github.com/${REPO}/releases/tag/${TAG}"
