# GitHub Actions Setup Checklist

Use this checklist when setting up CI/CD for your libsevenzip repository.

## Pre-Setup

- [ ] Repository is hosted on GitHub
- [ ] You have admin access to the repository
- [ ] Repository is public (or you have GitHub Actions enabled for private repos)

## File Verification

Run the configuration checker:
```bash
.github/check-ci-config.sh
```

- [ ] All workflow files exist (`ci.yml`, `quick-check.yml`, `nightly.yml`)
- [ ] All workflows have valid YAML syntax
- [ ] All workflows have name, triggers, and jobs defined

## Configuration Updates

- [ ] Edit [README.md](../README.md):
  - Replace `YOUR_USERNAME` with your GitHub username in badge URLs
  - Verify badges point to correct workflow files

- [ ] Review workflow triggers in each `.yml` file:
  - [ ] `ci.yml` - triggers on push/PR to your main branches
  - [ ] `quick-check.yml` - triggers on PR (optional, can be disabled)
  - [ ] `nightly.yml` - schedule time is acceptable (default: 2 AM UTC)

- [ ] Verify branch names match your repository:
  - Look for `branches: [ main, master, develop ]` in workflows
  - Adjust to match your branch naming convention

## GitHub Repository Settings

1. **Enable GitHub Actions**
   - [ ] Go to repository Settings → Actions → General
   - [ ] Ensure "Allow all actions and reusable workflows" is selected
   - [ ] Save if changed

2. **Set Workflow Permissions**
   - [ ] Go to Settings → Actions → General → Workflow permissions
   - [ ] Select "Read and write permissions" (for artifact uploads)
   - [ ] Check "Allow GitHub Actions to create and approve pull requests" if needed
   - [ ] Save if changed

3. **Configure Branch Protection (Optional)**
   - [ ] Go to Settings → Branches
   - [ ] Add rule for main branch
   - [ ] Check "Require status checks to pass before merging"
   - [ ] Select CI workflows to require
   - [ ] Save

## Initial Push

- [ ] Commit all CI configuration files:
  ```bash
  git add .github/
  git add README.md
  git commit -m "Add GitHub Actions CI/CD configuration"
  ```

- [ ] Push to trigger first CI run:
  ```bash
  git push origin main  # or your default branch
  ```

## First Run Verification

- [ ] Go to repository Actions tab on GitHub
- [ ] Verify workflows appear in the sidebar:
  - [ ] CI
  - [ ] Quick Check  
  - [ ] Nightly Build

- [ ] Click on first CI workflow run:
  - [ ] All jobs are queued or running
  - [ ] No immediate failures

- [ ] Wait for completion (~10-15 minutes)
  - [ ] Linux builds pass (Make + CMake)
  - [ ] macOS builds pass (Make + CMake)
  - [ ] Windows builds pass (NMAKE + CMake)
  - [ ] All tests pass
  - [ ] Artifacts are uploaded

## Status Badge Verification

- [ ] View your README.md on GitHub
- [ ] Status badges appear at the top
- [ ] Badges show "passing" status (green)
- [ ] Clicking badges opens Actions tab with correct workflow

## Troubleshooting First Run

If builds fail:

- [ ] Check workflow logs for specific errors
- [ ] Verify 7-Zip dependencies are accessible:
  - [ ] https://github.com/ip7z/7zip
  - [ ] https://github.com/mcmilk/7-Zip-zstd

- [ ] Common issues:
  - [ ] Branch name mismatch in workflow triggers
  - [ ] Missing build dependencies (should auto-install)
  - [ ] Test failures (check test code)
  - [ ] Artifact upload permissions

## Optional Optimizations

- [ ] **Reduce CI runs for cost/time:**
  - [ ] Disable `quick-check.yml` if `ci.yml` is sufficient
  - [ ] Reduce matrix size in `ci.yml` (e.g., test only one 7-Zip dependency)
  - [ ] Change nightly schedule or disable if not needed

- [ ] **Add repository secrets (if needed):**
  - [ ] Go to Settings → Secrets → Actions
  - [ ] Add any required secrets (e.g., for notifications)

- [ ] **Set up notifications:**
  - [ ] Configure email notifications in GitHub profile
  - [ ] Or add notification steps to workflows (Slack, Discord, etc.)

- [ ] **Enable Dependabot (Optional):**
  - [ ] Go to Settings → Security → Dependabot
  - [ ] Enable Dependabot alerts and security updates

## Documentation Review

- [ ] Read [CI_SETUP_SUMMARY.md](.github/CI_SETUP_SUMMARY.md) for overview
- [ ] Read [CI.md](.github/CI.md) for detailed information
- [ ] Read [QUICK_REFERENCE.md](.github/QUICK_REFERENCE.md) for commands
- [ ] Bookmark GitHub Actions tab for monitoring

## Maintenance

- [ ] Set calendar reminder to review CI logs monthly
- [ ] Check nightly build results regularly
- [ ] Update workflows when adding new features
- [ ] Keep workflow actions up to date (checkout@v4, etc.)

## Done!

✅ Your CI/CD is now configured and running!

### What Happens Now?

- **Every push/PR to main branches**: Full CI runs (26 jobs)
- **Every pull request**: Quick Check runs (3 jobs) *optional*
- **Daily at 2 AM UTC**: Nightly Build runs (10 jobs + memory checks)

### Monitoring

- Check the Actions tab regularly
- Review failed builds promptly
- Artifacts available for debugging
- Status badges show current state in README

### Next Steps

Consider adding:
- Code coverage reporting
- Static analysis integration
- Performance benchmarking
- Automated releases

---

**Questions or Issues?**

- Review workflow logs in Actions tab
- Check documentation in `.github/` folder
- Verify configuration with `.github/check-ci-config.sh`
- Check GitHub Actions documentation
