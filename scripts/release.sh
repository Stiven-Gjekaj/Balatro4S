#!/bin/sh
# Make a release of the version in resources/Info.plist.
# Usage: scripts/release.sh
# It runs the tests on this Mac and on the phone, builds the .deb, pushes
# the commits and the tag, publishes the GitHub release with the notes of
# CHANGELOG.md, adds the .deb to the Cydia repository in ../cydia, and
# closes the milestone of the version with its issues.
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
CYDIA=${CYDIA_REPO:-$ROOT/../cydia}
REPO=Stiven-Gjekaj/Balatro4S
VERSION=$(plutil -extract CFBundleShortVersionString raw "$ROOT/resources/Info.plist")
TAG="v$VERSION"
DEB="io.github.stivengjekaj.balatro4s_${VERSION}_iphoneos-arm.deb"
cd "$ROOT"

[ -z "$(git status --porcelain)" ] || { echo "release: commit the changes first" >&2; exit 1; }
grep -q "^## $VERSION " CHANGELOG.md || { echo "release: CHANGELOG.md has no entry for $VERSION" >&2; exit 1; }
if git rev-parse -q --verify "refs/tags/$TAG" >/dev/null; then
    echo "release: the tag $TAG is already there" >&2
    exit 1
fi

scripts/test.sh >build/release-tests.log || { cat build/release-tests.log >&2; exit 1; }
tail -1 build/release-tests.log
scripts/test.sh --phone >build/release-tests.log || { cat build/release-tests.log >&2; exit 1; }
tail -1 build/release-tests.log
scripts/deb.sh >/dev/null

git push -q
git tag -a "$TAG" -m "Balatro4S $VERSION"
git push -q origin "$TAG"
awk -v v="## $VERSION " 'index($0, v) == 1 {f = 1; next} /^## / {f = 0} f' CHANGELOG.md >build/notes.md
cat >>build/notes.md <<EOF

Install: copy the .deb to the phone and run \`dpkg -i $DEB\` as root. Copy
\`Balatro.love\` from your own copy of Balatro to
\`/var/mobile/Media/Balatro4S/\`. The .deb holds no file of Balatro.
EOF
gh release create "$TAG" -R "$REPO" -t "Balatro4S $VERSION" -F build/notes.md "build/$DEB"

# The Cydia repository: add the package, and show the new version.
(
    cd "$CYDIA"
    scripts/update.sh "$ROOT/build/$DEB"
    git add debs Packages Packages.bz2 Packages.gz Release
    git commit -qm "feat: add Balatro4S $VERSION"
    sed -i '' "s/^| Balatro4S (\`io.github.stivengjekaj.balatro4s\`) | [0-9.]* |/| Balatro4S (\`io.github.stivengjekaj.balatro4s\`) | $VERSION |/" README.md
    git add README.md
    git commit -qm "docs: list Balatro4S $VERSION"
    git push -q
)

# Close the issues of the milestone, then the milestone.
MILESTONE=$(gh api "repos/$REPO/milestones?state=open" -q ".[] | select(.title | startswith(\"$TAG\")) | .number")
if [ -n "$MILESTONE" ]; then
    TITLE=$(gh api "repos/$REPO/milestones/$MILESTONE" -q .title)
    for issue in $(gh issue list -R "$REPO" -m "$TITLE" --json number -q '.[].number'); do
        gh issue close "$issue" -R "$REPO" -c "Done in $TAG." >/dev/null
    done
    gh api -X PATCH "repos/$REPO/milestones/$MILESTONE" -f state=closed -q '.title + ": " + .state'
fi
echo "released $TAG"
