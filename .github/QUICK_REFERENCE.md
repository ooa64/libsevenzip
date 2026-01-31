# CI/CD Quick Reference

## Workflow Files

| File | Purpose | Trigger | Duration |
|------|---------|---------|----------|
| `ci.yml` | Main CI pipeline | Push/PR to main branches | ~10-15 min |
| `quick-check.yml` | Fast PR validation | Pull requests | ~5-7 min |
| `nightly.yml` | Extended testing | Daily at 2 AM UTC | ~20-30 min |

## Build Matrix Summary

### ci.yml - Main CI (26 jobs)
- **Linux:** Make (GCC/Clang) + CMake (Debug/Release) × 2 deps = 8 jobs
- **macOS:** Make + CMake (Debug/Release) × 2 deps = 6 jobs  
- **Windows:** NMAKE (x64/Win32) + CMake (4 configs) × 2 deps = 12 jobs

### quick-check.yml - Quick Check (3 jobs)
- Linux, macOS, Windows × 1 dep (ip7z) = 3 jobs

### nightly.yml - Nightly Build (10 jobs)
- Each OS × (Make + CMake) × 2 deps = ~10 jobs
- Includes Valgrind on Linux

## 7-Zip Dependencies

| Repository | Description | Codecs |
|------------|-------------|---------|
| `ip7z/7zip` | Official 7-Zip source | Standard (LZMA, LZMA2, PPMd, etc.) |
| `mcmilk/7-Zip-zstd` | Extended with extra codecs | + Zstandard, Brotli, LZ4, LZ5, Lizard, Fast-LZMA2 |

## Platform Build Commands

### Linux
```bash
# Make
make SEVENZIPSRC=../7zip
make tests SEVENZIPSRC=../7zip

# CMake
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSEVENZIPSRC=../7zip
cmake --build build
cd build && ctest --output-on-failure
```

### macOS
```bash
# Same as Linux
make SEVENZIPSRC=../7zip
make tests SEVENZIPSRC=../7zip
```

### Windows
```cmd
REM NMAKE
call vcvarsall.bat x64
nmake -f makefile.vc SEVENZIPSRC=..\7zip
nmake -f makefile.vc tests SEVENZIPSRC=..\7zip

REM CMake
cmake -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DSEVENZIPSRC=../7zip
cmake --build build --config Release
cd build && ctest --output-on-failure -C Release
```

## Artifacts Uploaded

Each build job uploads:
- Static library (`libsevenzip.a` / `sevenzip.lib`)
- Test executable (`tests` / `tests.exe`)
- Example programs (`example*`)

Retention:
- PR builds: 7 days
- Nightly builds: 30 days

## Status Badge URLs

Replace `YOUR_USERNAME` with your GitHub username:

```markdown
![CI](https://github.com/YOUR_USERNAME/libsevenzip/workflows/CI/badge.svg)
![Quick Check](https://github.com/YOUR_USERNAME/libsevenzip/workflows/Quick%20Check/badge.svg)
![Nightly Build](https://github.com/YOUR_USERNAME/libsevenzip/workflows/Nightly%20Build/badge.svg)
```

## Common CI Commands

### Trigger Manual Run
Go to: `Actions` tab → Select workflow → `Run workflow` button

### View Logs
`Actions` tab → Click on workflow run → Click on job name

### Download Artifacts
`Actions` tab → Workflow run → Scroll to `Artifacts` section

### Re-run Failed Jobs
Workflow run → `Re-run failed jobs` button

## Workflow Modifications

### Change Trigger Branches
Edit `on.push.branches` and `on.pull_request.branches` in workflow files

### Adjust Nightly Schedule
Edit `on.schedule.cron` in `nightly.yml`
- Format: `'minute hour day month weekday'`
- Current: `'0 2 * * *'` (2 AM UTC daily)

### Reduce Matrix Size
Comment out matrix entries in workflow files:
```yaml
matrix:
  compiler: [gcc]  # Remove clang to reduce jobs
  7zip_repo: 
    - { url: 'ip7z/7zip', name: 'ip7z' }  # Only test with one dependency
```

### Add Compiler Flags
Modify build commands:
```yaml
make SEVENZIPSRC=../7zip CXXFLAGS="-Wall -Wextra -Werror"
```

## Debugging CI Failures

### Local Reproduction
```bash
# Clone dependencies exactly as CI does
git clone https://github.com/ip7z/7zip.git ../7zip

# Run exact build commands from workflow
cd libsevenzip
make SEVENZIPSRC=../7zip
./tests/tests
```

### Enable Debug Output
Add to workflow step:
```yaml
- name: Build
  run: |
    set -x  # Enable bash debug output
    make SEVENZIPSRC=../7zip
```

### Check Specific Platform
Filter workflows in Actions tab or use labels in PR

## Performance Tips

1. **Use Quick Check for PRs** - faster feedback
2. **Main CI on merge only** - reduce runner usage
3. **Cache dependencies** - speed up checkout (future enhancement)
4. **Parallel jobs** - already enabled via matrix
5. **Conditional execution** - skip unchanged platforms

## Cost Considerations (Private Repos)

GitHub Free tier:
- 2,000 CI/CD minutes/month
- Linux: 1× multiplier
- macOS: 10× multiplier  
- Windows: 2× multiplier

Example per CI run:
- Linux: 8 jobs × 15 min = 120 min → 120 counted minutes
- macOS: 6 jobs × 15 min = 90 min → 900 counted minutes
- Windows: 12 jobs × 15 min = 180 min → 360 counted minutes
- **Total:** ~1,380 counted minutes per full CI run

**Public repositories have unlimited CI/CD minutes!**

## Advanced Features (Future)

- [ ] Code coverage reports (gcov, lcov)
- [ ] Static analysis (clang-tidy, cppcheck)
- [ ] Sanitizers (ASAN, TSAN, UBSAN)
- [ ] Performance benchmarks
- [ ] Release automation
- [ ] Documentation generation
- [ ] Docker images for testing
- [ ] Cross-compilation tests

## Getting Help

- Check workflow logs in Actions tab
- Review [CI_SETUP_SUMMARY.md](.github/CI_SETUP_SUMMARY.md)
- Read [CI.md](.github/CI.md)
- Run `.github/check-ci-config.sh` validator

## Useful GitHub Actions Documentation

- [Workflow syntax](https://docs.github.com/en/actions/reference/workflow-syntax-for-github-actions)
- [Matrix builds](https://docs.github.com/en/actions/using-jobs/using-a-matrix-for-your-jobs)
- [Upload artifacts](https://github.com/actions/upload-artifact)
- [Checkout action](https://github.com/actions/checkout)
