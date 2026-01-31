# CI/CD Troubleshooting Guide

Common issues and solutions for GitHub Actions CI/CD setup.

## Configuration Issues

### Workflows Don't Appear in Actions Tab

**Symptoms:**
- Pushed workflows but don't see them in Actions tab
- "No workflows found" message

**Solutions:**
1. Check if Actions are enabled:
   - Go to Settings → Actions → General
   - Ensure "Allow all actions" is selected
   
2. Verify workflow files location:
   ```bash
   ls -la .github/workflows/
   # Should show: ci.yml, quick-check.yml, nightly.yml
   ```

3. Check YAML syntax:
   ```bash
   # Install yamllint
   pip install yamllint
   
   # Validate workflows
   yamllint .github/workflows/*.yml
   ```

4. Ensure workflows are committed and pushed:
   ```bash
   git status
   git add .github/
   git commit -m "Add CI workflows"
   git push
   ```

### "Unable to resolve action" Error

**Symptoms:**
- Error like: `Unable to resolve action 'actions/checkout@v4'`

**Cause:** Network issues or GitHub Actions service problem

**Solutions:**
1. Wait a few minutes and re-run
2. Check [GitHub Status](https://www.githubstatus.com/)
3. Try older version: `actions/checkout@v3`

### Branch Name Mismatch

**Symptoms:**
- Workflows don't trigger on push

**Cause:** Workflow configured for different branch names

**Solution:**
Edit workflow files to match your branches:
```yaml
on:
  push:
    branches: [ main ]  # Change to match your default branch
```

## Build Failures

### 7-Zip Dependency Not Found

**Symptoms:**
```
Error: C/7zTypes.h: No such file or directory
Error: CPP/Common/MyString.h: No such file or directory
```

**Cause:** 7-Zip repository structure or checkout issue

**Solutions:**
1. Verify repository URLs are correct in workflow
2. Check if 7-Zip repos are accessible:
   - https://github.com/ip7z/7zip
   - https://github.com/mcmilk/7-Zip-zstd
   
3. Test locally:
   ```bash
   git clone https://github.com/ip7z/7zip.git ../7zip
   ls ../7zip/C/  # Should show 7zTypes.h etc.
   ls ../7zip/CPP/  # Should show Common/, Windows/, etc.
   ```

### Compiler Errors on Specific Platform

**Symptoms:**
- Builds pass on some platforms but fail on others
- Compiler-specific errors

**Solutions:**
1. Check error logs for specific issues
2. Test locally on failing platform
3. Add compiler flags if needed:
   ```yaml
   make SEVENZIPSRC=../7zip CXXFLAGS="-Wno-error"
   ```

4. Check for platform-specific code issues

### NMAKE Not Found (Windows)

**Symptoms:**
```
'nmake' is not recognized as an internal or external command
```

**Cause:** vcvarsall.bat not sourced correctly

**Solution:**
Verify vcvarsall.bat path in workflow:
```yaml
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
```

Or use different Visual Studio path:
```yaml
# VS 2019
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
```

### CMake Configuration Fails

**Symptoms:**
```
CMake Error: Could not find CMAKE_C_COMPILER
```

**Solutions:**
1. Ensure build-essential installed (Linux):
   ```yaml
   sudo apt-get install -y build-essential cmake
   ```

2. Verify MSVC setup (Windows):
   ```yaml
   - name: Setup MSVC
     uses: microsoft/setup-msbuild@v2
   ```

3. Check CMake version:
   ```bash
   cmake --version
   # Should be >= 3.20
   ```

## Test Failures

### Tests Fail But Build Succeeds

**Symptoms:**
- Compilation completes successfully
- `tests` executable fails during run

**Solutions:**
1. Check test logs for specific failures
2. Run tests locally:
   ```bash
   make tests SEVENZIPSRC=../7zip
   ./tests/tests
   ```

3. Check if 7z.dll/7z.so is needed:
   - Some tests may require 7-Zip runtime library
   - Check test code dependencies

4. Verify test data files exist:
   ```bash
   ls test*.7z test*.dmg testDIRS*
   ```

### Valgrind Reports Memory Leaks

**Symptoms:**
- Nightly build fails on Linux
- Valgrind detects memory leaks

**Solutions:**
1. Review Valgrind output in logs
2. Run locally with Valgrind:
   ```bash
   valgrind --leak-check=full ./tests/tests
   ```

3. Fix memory leaks in code
4. Or suppress known false positives:
   ```bash
   valgrind --suppressions=valgrind.supp ./tests/tests
   ```

## Artifact Issues

### Artifacts Not Uploaded

**Symptoms:**
- Build succeeds but no artifacts appear

**Solutions:**
1. Check artifact paths are correct:
   ```yaml
   path: |
     libsevenzip/build/libsevenzip.a  # Verify this path
     libsevenzip/build/tests
   ```

2. Verify files exist before upload:
   ```yaml
   - name: Verify artifacts
     run: ls -la build/
   ```

3. Check workflow permissions:
   - Settings → Actions → Workflow permissions
   - Enable "Read and write permissions"

### Cannot Download Artifacts

**Symptoms:**
- Artifacts shown but download fails

**Solutions:**
1. Check artifact retention hasn't expired
2. Ensure you're logged into GitHub
3. Try different browser or clear cache

## Performance Issues

### Builds Take Too Long

**Symptoms:**
- CI runs exceed 30 minutes
- Jobs timeout

**Solutions:**
1. Reduce matrix size:
   ```yaml
   matrix:
     compiler: [gcc]  # Remove clang
     7zip_repo: 
       - { url: 'ip7z/7zip', name: 'ip7z' }  # Test only one
   ```

2. Use parallel builds:
   ```bash
   make -j$(nproc) SEVENZIPSRC=../7zip
   ```

3. Cache dependencies (add to workflow):
   ```yaml
   - name: Cache 7-Zip
     uses: actions/cache@v3
     with:
       path: ../7zip
       key: 7zip-${{ matrix.7zip_repo.name }}
   ```

### Running Out of CI Minutes (Private Repos)

**Symptoms:**
- Warning about CI minutes used
- Workflows disabled due to limits

**Solutions:**
1. Use Quick Check only for PRs
2. Reduce nightly build frequency
3. Decrease matrix combinations
4. Consider self-hosted runners
5. Make repository public (unlimited minutes)

## Matrix Strategy Issues

### Too Many Jobs Running

**Symptoms:**
- 26 jobs seems excessive
- Want to reduce CI time/cost

**Solutions:**
1. Focus on critical combinations:
   ```yaml
   matrix:
     include:
       - os: ubuntu-latest
         compiler: gcc
         7zip_repo: { url: 'ip7z/7zip', name: 'ip7z' }
       - os: macos-latest
         compiler: clang
         7zip_repo: { url: 'ip7z/7zip', name: 'ip7z' }
       - os: windows-latest
         platform: x64
         7zip_repo: { url: 'ip7z/7zip', name: 'ip7z' }
   ```

2. Test one dependency in main CI, other in nightly

### Matrix Combinations Failing

**Symptoms:**
- Some specific matrix combinations fail
- Others succeed

**Solutions:**
1. Add `exclude` to matrix:
   ```yaml
   matrix:
     os: [ubuntu-latest, windows-latest]
     compiler: [gcc, clang]
     exclude:
       - os: windows-latest
         compiler: gcc  # Not available on Windows
   ```

2. Use `fail-fast: false` to see all failures:
   ```yaml
   strategy:
     fail-fast: false
     matrix:
       # ...
   ```

## Notification Issues

### Not Receiving Build Notifications

**Solutions:**
1. Check GitHub notification settings:
   - Settings → Notifications
   - Enable "Actions" notifications

2. Check email filters/spam

3. Add custom notifications to workflows:
   ```yaml
   - name: Send notification
     if: failure()
     run: |
       curl -X POST webhook-url -d '{"text":"Build failed"}'
   ```

## Status Badge Issues

### Badge Shows "Unknown" Status

**Symptoms:**
- Badge displays "unknown" instead of passing/failing

**Solutions:**
1. Verify workflow has run at least once
2. Check badge URL:
   ```markdown
   ![CI](https://github.com/USERNAME/REPO/workflows/CI/badge.svg)
   ```
   
3. Use branch-specific badge:
   ```markdown
   ![CI](https://github.com/USERNAME/REPO/workflows/CI/badge.svg?branch=main)
   ```

4. Wait for workflow to complete first run

### Badge Not Updating

**Solutions:**
1. Clear browser cache
2. Force refresh (Ctrl+F5 / Cmd+Shift+R)
3. Check if workflow name matches exactly:
   - Badge: `workflows/CI/badge.svg`
   - Workflow: `name: CI` in yml file

## Debugging Steps

### General Debugging Process

1. **Check workflow logs:**
   - Actions tab → Click on run → Click on job → Expand steps

2. **Add debug output:**
   ```yaml
   - name: Debug info
     run: |
       echo "PWD: $PWD"
       echo "SEVENZIPSRC: $SEVENZIPSRC"
       ls -la ../7zip/
   ```

3. **Enable debug logging:**
   - Settings → Secrets → New secret
   - Name: `ACTIONS_STEP_DEBUG`
   - Value: `true`

4. **Test locally:**
   ```bash
   # Replicate exact CI commands
   git clone https://github.com/ip7z/7zip.git ../7zip
   cd libsevenzip
   make SEVENZIPSRC=../7zip
   ```

5. **Run in container (advanced):**
   ```bash
   docker run -it ubuntu:latest
   # Install dependencies and test
   ```

### Getting Help

1. **Check documentation:**
   - [INDEX.md](.github/INDEX.md)
   - [CI_SETUP_SUMMARY.md](.github/CI_SETUP_SUMMARY.md)
   - [QUICK_REFERENCE.md](.github/QUICK_REFERENCE.md)

2. **Run validator:**
   ```bash
   .github/check-ci-config.sh
   ```

3. **Review logs systematically:**
   - Start with failed job
   - Check "Set up job" section
   - Review each failing step
   - Look for error messages

4. **Search GitHub Actions docs:**
   - [GitHub Actions Documentation](https://docs.github.com/en/actions)
   - [Workflow syntax](https://docs.github.com/en/actions/reference/workflow-syntax-for-github-actions)

5. **Community resources:**
   - GitHub Community Forum
   - Stack Overflow (tag: github-actions)
   - Issue trackers for specific actions

## Common Error Messages

### "Resource not accessible by integration"
**Solution:** Enable write permissions in Settings → Actions → Workflow permissions

### "The workflow is not valid"
**Solution:** Check YAML syntax, verify indentation

### "Process completed with exit code 2"
**Solution:** Check step output for actual error, usually compilation/test failure

### "Unable to locate executable file: cmake"
**Solution:** Add cmake installation step or use pre-installed runner

### "Permission denied"
**Solution:** 
- Make scripts executable: `chmod +x script.sh`
- Or run with shell: `bash script.sh`

## Prevention Tips

1. **Test locally first:** Always test builds locally before pushing
2. **Use branches:** Test workflow changes in feature branches
3. **Start simple:** Begin with minimal matrix, expand gradually
4. **Monitor regularly:** Check Actions tab after each push
5. **Keep updated:** Update action versions (`@v4`, `@v3`, etc.)
6. **Document changes:** Update docs when modifying workflows

---

**Still having issues?**

1. Review complete error logs in Actions tab
2. Check [QUICK_REFERENCE.md](QUICK_REFERENCE.md) for commands
3. Verify setup with `.github/check-ci-config.sh`
4. Test exact CI commands locally
5. Check [GitHub Status](https://www.githubstatus.com/)
