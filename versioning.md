# Component Versioning

This repository uses **Semantic Versioning (SemVer)** with a shared version strategy, compatible with the [Espressif Component Registry](https://components.espressif.com/).

## Semantic Versioning Format

```
MAJOR.MINOR.PATCH
```

- **MAJOR**: Breaking API changes (incompatible changes)
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes (backward compatible)

## Shared Version Strategy

All components in this repository share the same version number. When releasing:

1. Update the `version` field in **all** component `idf_component.yml` files to the same version
2. Commit the changes
3. Create a git tag matching that version

Example structure:
```
esp-components/
├── hvo__i2c/idf_component.yml         → version: "1.0.0"
└── hvo__mcp23017/idf_component.yml    → version: "1.0.0"
```

## Release Workflow

```bash
# 1. Update version in all idf_component.yml files to new version (e.g., 1.1.0)

# 2. Commit the version bump
git commit -am "Release 1.1.0"

# 3. Create the tag (must match version exactly, no 'v' prefix)
git tag 1.1.0

# 4. Push commit and tag
git push origin main
git push origin 1.1.0        # pushes specific tag
# or: git push origin --tags  # pushes all tags
```

## Using Components in Projects

Reference components in your project's `idf_component.yml`:

```yaml
dependencies:
  hvo__i2c:
    git: https://github.com/hvogeler/esp-components.git
    path: hvo__i2c
    version: "1.0.3"
  hvo__mcp23017:
    git: https://github.com/hvogeler/esp-components.git
    path: hvo__mcp23017
    version: "1.0.3"
```

## Version Specifiers

**Important:** For git-based dependencies, the `version` field must be an **exact git reference** (tag, branch, or commit hash). Semver specifiers only work for components published to the ESP Component Registry.

| Git dependency | Registry dependency |
|----------------|---------------------|
| `"1.0.3"` (exact tag) | `"^1.0.0"` (compatible updates) |
| `main` (branch) | `"~1.0.0"` (patch updates only) |
| `abc123` (commit hash) | `">=1.0.0"` (at least version) |

## Tag Naming

Git tags must match the version exactly as specified in `idf_component.yml` (without a `v` prefix):

| Version in manifest | Git tag |
|--------------------|---------|
| `1.0.0` | `1.0.0` |
| `2.1.3` | `2.1.3` |

The IDF Component Manager requires the tag to match the version string exactly.

## Notes

- The shared version strategy means all components get a version bump even if only one changes
- This keeps dependency management simple and ensures consistent component sets
- **Git dependencies require exact version tags** - semver ranges (`^`, `~`, `>=`) only work after publishing to the ESP Component Registry
- To get automatic semver resolution, publish components to https://components.espressif.com/
