# CI/CD Documentation Index

Welcome to the libsevenzip CI/CD documentation!

## Quick Start

1. **Setup**: Follow [SETUP_CHECKLIST.md](SETUP_CHECKLIST.md) step-by-step
2. **Validate**: Run `./check-ci-config.sh` to verify configuration
3. **Push**: Commit and push to trigger your first CI run
4. **Monitor**: Check GitHub Actions tab for build status

## Documentation Files

### 📋 Getting Started
- **[SETUP_CHECKLIST.md](SETUP_CHECKLIST.md)** - Step-by-step setup guide with checklist
  - Pre-setup requirements
  - Configuration updates needed
  - First run verification
  - Troubleshooting tips

### 📖 Main Documentation
- **[CI_SETUP_SUMMARY.md](CI_SETUP_SUMMARY.md)** - Complete overview of CI/CD setup
  - Workflow descriptions
  - Build matrix details
  - Dependency management
  - Resource usage estimates
  - Future enhancements

- **[CI.md](CI.md)** - Detailed CI/CD information
  - Build matrix breakdown
  - Platform configurations
  - Artifacts information
  - Local testing instructions
  - Troubleshooting guide

### ⚡ Quick Reference
- **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - Command reference and cheat sheet
  - Build commands for each platform
  - Workflow modification examples
  - Debugging tips
  - Performance optimization
  - Common operations

### 🔧 Tools
- **[check-ci-config.sh](check-ci-config.sh)** - Configuration validator script
  - Checks workflow files exist
  - Validates YAML syntax
  - Verifies dependencies
  - Tests platform coverage

### 🐛 Troubleshooting
- **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** - Comprehensive troubleshooting guide
  - Configuration issues
  - Build failures
  - Test problems
  - Artifact issues
  - Performance problems
  - Common error messages and solutions

## Workflow Files

### `.github/workflows/`

1. **[ci.yml](workflows/ci.yml)** - Main CI Pipeline
   - **Purpose**: Comprehensive testing on every push/PR
   - **Jobs**: 26 parallel jobs
   - **Platforms**: Linux, macOS, Windows
   - **Build Systems**: Make/NMAKE and CMake
   - **Dependencies**: Tests both ip7z/7zip and mcmilk/7-Zip-zstd
   - **Duration**: ~10-15 minutes

2. **[quick-check.yml](workflows/quick-check.yml)** - Fast PR Validation
   - **Purpose**: Quick smoke test for pull requests
   - **Jobs**: 3 parallel jobs (one per platform)
   - **Platforms**: Linux, macOS, Windows
   - **Build System**: Make/NMAKE only
   - **Dependencies**: ip7z/7zip only
   - **Duration**: ~5-7 minutes

3. **[nightly.yml](workflows/nightly.yml)** - Extended Testing
   - **Purpose**: Comprehensive nightly testing with memory checks
   - **Jobs**: 10 parallel jobs
   - **Platforms**: Linux, macOS, Windows
   - **Build Systems**: Make/NMAKE and CMake
   - **Dependencies**: Both repositories
   - **Special**: Includes Valgrind memory leak detection on Linux
   - **Duration**: ~20-30 minutes
   - **Schedule**: Daily at 2 AM UTC

## Build Matrix Overview

```
CI Workflow (Main):
├── Linux (8 jobs)
│   ├── Make: GCC + Clang × 2 deps = 4 jobs
│   └── CMake: Debug + Release × 2 deps = 4 jobs
├── macOS (6 jobs)
│   ├── Make: 1 × 2 deps = 2 jobs
│   └── CMake: Debug + Release × 2 deps = 4 jobs
└── Windows (12 jobs)
    ├── NMAKE: x64 + Win32 × 2 deps = 4 jobs
    └── CMake: (Debug + Release) × (x64 + Win32) × 2 deps = 8 jobs

Quick Check:
├── Linux: Make × 1 dep = 1 job
├── macOS: Make × 1 dep = 1 job
└── Windows: NMAKE × 1 dep = 1 job

Nightly Build (10 jobs):
├── Linux: Make + CMake × 2 deps = 4 jobs
├── macOS: Make + CMake × 2 deps = 4 jobs
└── Windows: CMake × 2 deps = 2 jobs
```

## 7-Zip Dependencies Tested

### ip7z/7zip
- **Source**: https://github.com/ip7z/7zip
- **Description**: Official 7-Zip source code
- **Features**: Standard codecs (LZMA, LZMA2, PPMd, BZip2, Deflate, etc.)
- **Status**: Stable, minimal

### mcmilk/7-Zip-zstd
- **Source**: https://github.com/mcmilk/7-Zip-zstd
- **Description**: 7-Zip with additional compression codecs
- **Features**: All standard codecs plus:
  - Zstandard (zstd) - Fast compression
  - Brotli - Google's compression algorithm
  - LZ4 - Extremely fast compression
  - LZ5 - Modified LZ4
  - Lizard - Efficient compressor
  - Fast LZMA2 - Faster LZMA2 variant
  - Additional hash algorithms (BLAKE2sp, BLAKE3, SHA3, XXH32, XXH64)
- **Status**: Actively maintained, feature-rich

## File Organization

```
.github/
├── workflows/
│   ├── ci.yml              # Main CI workflow
│   ├── quick-check.yml     # Fast PR checks
│   └── nightly.yml         # Nightly extended tests
├── check-ci-config.sh      # Configuration validator
├── CI.md                   # Detailed CI documentation
├── CI_SETUP_SUMMARY.md     # Complete setup overview
├── INDEX.md                # This file
├── QUICK_REFERENCE.md      # Commands and tips
└── SETUP_CHECKLIST.md      # Step-by-step setup guide
```

## Common Tasks

### View CI Status
```bash
# Visit your repository on GitHub
https://github.com/YOUR_USERNAME/libsevenzip/actions

# Or check badges in README
```

### Run Validation
```bash
.github/check-ci-config.sh
```

### Trigger Manual Build
1. Go to Actions tab on GitHub
2. Select workflow (CI, Quick Check, or Nightly Build)
3. Click "Run workflow" button
4. Select branch and click "Run workflow"

### Download Build Artifacts
1. Go to Actions tab
2. Click on a completed workflow run
3. Scroll to "Artifacts" section at the bottom
4. Click to download

### Test Locally
```bash
# Clone dependency
git clone https://github.com/ip7z/7zip.git ../7zip

# Build and test
cd libsevenzip
make SEVENZIPSRC=../7zip
make tests SEVENZIPSRC=../7zip
./tests/tests
```

## Getting Help

### Configuration Issues
1. Run `./check-ci-config.sh` for diagnostics
2. Check [SETUP_CHECKLIST.md](SETUP_CHECKLIST.md) troubleshooting section
3. Review workflow logs in GitHub Actions tab

### Build Failures
1. Check specific job logs in Actions tab
2. Review [QUICK_REFERENCE.md](QUICK_REFERENCE.md) debugging section
3. Test locally with same commands
4. Verify 7-Zip dependencies are accessible

### Workflow Modifications
1. See [QUICK_REFERENCE.md](QUICK_REFERENCE.md) workflow modification section
2. Refer to [GitHub Actions documentation](https://docs.github.com/en/actions)

## Status Badges

Add to README.md (replace YOUR_USERNAME):

```markdown
[![CI](https://github.com/YOUR_USERNAME/libsevenzip/workflows/CI/badge.svg)](https://github.com/YOUR_USERNAME/libsevenzip/actions/workflows/ci.yml)
[![Quick Check](https://github.com/YOUR_USERNAME/libsevenzip/workflows/Quick%20Check/badge.svg)](https://github.com/YOUR_USERNAME/libsevenzip/actions/workflows/quick-check.yml)
[![Nightly Build](https://github.com/YOUR_USERNAME/libsevenzip/workflows/Nightly%20Build/badge.svg)](https://github.com/YOUR_USERNAME/libsevenzip/actions/workflows/nightly.yml)
```

## Additional Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Workflow Syntax Reference](https://docs.github.com/en/actions/reference/workflow-syntax-for-github-actions)
- [Matrix Strategy](https://docs.github.com/en/actions/using-jobs/using-a-matrix-for-your-jobs)
- [7-Zip Official](https://www.7-zip.org/)
- [ip7z/7zip Repository](https://github.com/ip7z/7zip)
- [mcmilk/7-Zip-zstd Repository](https://github.com/mcmilk/7-Zip-zstd)

## Feedback and Improvements

This CI/CD setup is designed to be comprehensive yet flexible. Feel free to:
- Adjust matrix size based on your needs
- Modify schedules for nightly builds
- Add custom build steps
- Integrate additional tools (coverage, static analysis, etc.)

For questions specific to libsevenzip, refer to the main project documentation.

---

**Ready to get started?** Follow [SETUP_CHECKLIST.md](SETUP_CHECKLIST.md)!
