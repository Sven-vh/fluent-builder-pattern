# Branch Protection Setup

To ensure that all pull requests must pass tests before being merged into `main`, follow these steps:

## Setting up Branch Protection Rules

1. Go to your repository on GitHub
2. Navigate to **Settings** > **Branches**
3. Click **Add rule** or **Edit** if a rule already exists for `main`
4. Configure the following settings:

### Required Settings:
- **Branch name pattern**: `main`
- ✅ **Restrict pushes that create files larger than 100MB**
- ✅ **Require a pull request before merging**
  - ✅ **Require approvals**: 1 (or more as desired)
  - ✅ **Dismiss stale PR approvals when new commits are pushed**
- ✅ **Require status checks to pass before merging**
  - ✅ **Require branches to be up to date before merging**
  - **Status checks**: Select `test` (this is the job name from the CI workflow)

### Optional but Recommended:
- ✅ **Require conversation resolution before merging**
- ✅ **Include administrators** (applies rules to repository admins too)

## What This Achieves

With these settings enabled:
- No direct pushes to `main` are allowed
- All changes must go through pull requests
- The CI tests must pass before a PR can be merged
- PRs must be up-to-date with the latest `main` branch
- At least one approval is required for each PR

## Testing the Setup

1. Create a test branch
2. Make a small change and create a PR
3. Verify that the CI workflow runs automatically
4. Confirm that the merge button is disabled until tests pass

The CI workflow will run on every push to a PR targeting `main`, ensuring code quality is maintained.