# Development Workflow

How to maintain and extend components while working on other projects.

## Prerequisites

- esp-components cloned locally at `/Users/hvo/Esp32/esp-components`
- Repository pushed to GitHub

## Workflow

### 1. Reference components via local path

In your project's `main/idf_component.yml`:

```yaml
dependencies:
  i2c:
    path: /Users/hvo/Esp32/esp-components/i2c
  mcp23017:
    path: /Users/hvo/Esp32/esp-components/mcp23017
```

Local paths mean edits are picked up immediately on rebuild — no version bumping or copy/paste needed.

### 2. Develop and test

Build and flash your project as normal:

```bash
idf.py build flash monitor
```

When you need to modify a component (add feature, fix bug), edit files directly in the esp-components directory:

```
/Users/hvo/Esp32/esp-components/
├── i2c/
│   ├── i2c.cpp          # ← edit here
│   └── include/i2c.hpp
└── mcp23017/
    ├── mcp23017.cpp     # ← edit here
    └── include/mcp23017.hpp
```

Rebuild your project to test changes.

### 3. Commit component changes

Once changes are working:

```bash
cd /Users/hvo/Esp32/esp-components
git status
git add -A
git commit -m "Add interrupt support to mcp23017"
git push
```

### 4. (Optional) Switch project to git reference

For portable or shareable projects, switch from local path to git:

```yaml
dependencies:
  mcp23017:
    git: https://github.com/YOUR_USERNAME/esp-components.git
    path: mcp23017
    version: "*"
```

Then clean and rebuild:

```bash
rm -rf managed_components/
idf.py build
```

## Quick Reference

| Task | Location | Command |
|------|----------|---------|
| Edit component | `esp-components/<component>/` | — |
| Test changes | Your project | `idf.py build flash monitor` |
| Commit | `esp-components/` | `git add -A && git commit` |
| Push | `esp-components/` | `git push` |

## Notes

- Local path references are convenient for solo development
- Git references are better for sharing projects or CI/CD
- Component changes require hardware to test meaningfully
