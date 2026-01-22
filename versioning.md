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
├── i2c/idf_component.yml         → version: "1.0.0"
└── mcp23017/idf_component.yml    → version: "1.0.0"
```

## Release Workflow

```bash
# 1. Update version in all idf_component.yml files to new version (e.g., 1.1.0)

# 2. Commit the version bump
git commit -am "Release 1.1.0"

# 3. Create the tag (with 'v' prefix)
git tag v1.1.0

# 4. Push commit and tag
git push origin main
git push origin v1.1.0       # pushes specific tag
# or: git push origin --tags  # pushes all tags
```

## Using Components in Projects

Reference components in your project's `idf_component.yml`:

```yaml
dependencies:
  i2c:
    git: https://github.com/hvogeler/esp-components.git
    path: i2c
    version: "^1.0.0"
  mcp23017:
    git: https://github.com/hvogeler/esp-components.git
    path: mcp23017
    version: "^1.0.0"
```

## Version Specifiers

| Specifier | Meaning |
|-----------|---------|
| `1.0.0` | Exact version |
| `^1.0.0` | Compatible updates (>=1.0.0 <2.0.0) |
| `~1.0.0` | Patch updates only (>=1.0.0 <1.1.0) |
| `>=1.0.0` | At least this version |
| `*` | Any version (not recommended for production) |

## Tag Naming

Git tags should match the version with an optional `v` prefix:

| Version in manifest | Git tag |
|--------------------|---------|
| `1.0.0` | `v1.0.0` |
| `2.1.3` | `v2.1.3` |

The IDF Component Manager strips the `v` prefix when matching versions.

## Notes

- The shared version strategy means all components get a version bump even if only one changes
- This keeps dependency management simple and ensures consistent component sets
- This approach is fully compatible with the Espressif Component Registry for future publishing
