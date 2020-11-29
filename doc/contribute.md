# Contribute

Up to [Documentation](README.md).

## Possibilities
The following are just a few possible ways to contribute.
Feel free to do your own thing.

- Read the documentation, and check for correctness and completeness.
- If you have a device or an OS that is not listed as tested, try the library on it.
- Read the code, if something looks suspicious, exploit it.
- If you have an idea for a program that may benefit from `abc`, launch it, and ask questions along the way.
- Spread the word about `abc`.

## Issues
The most common way to contribute is to file issues.
Whether you have cloned and built the repo, or whether you have just read the code online, you may find something that doesn't seem right.
[File it](../issues).

### Filing
When you file an issue, keep in mind that the issue must contain enough information so that it can be reproduced, categorized, and ultimately - triaged.

Each issue should fall in one of three categories - _bugs_, _enhancements_, or _questions_.
A bug is a discrepancy between the intent and the implementation of a released feature.
An enhancement is a missing or suboptimal functionality.
And a question is a request for information (which may eventually become a documentation enhancement).

To report a bug, consider the following template:
```
CONTEXT:
- Hardware
- OS distribution and bitness
- Compiler and version

REPRO:
1. Step 1
2. Step 2
3. ...

EXPECTED:
Describe the expected behavior.

ACTUAL:
Describe what you observe.
```

If you can make an enhancement fit the above template - great!
If not, use your best judgement.

For questions, feel encouraged to provide as much context as you are willing to share.

### Triage
The first step of the triage process is reproducing the issue.
Be prepared to make your case.

If the issue is reproducable, it will be labeled with one of the three labels referenced above - bug, enhancement, or question.
Otherwise, it will be closed.

If the issue is accepted, but there are more important issues to be addressed in the near future, the issue will be also be labeled with "help needed".
Such issued are up for grabs.

## Pull Requests
### Proposal
Make sure there is an issue that is labeled with "help needed".

Briefly propose a solution in a comment, and wait until it is approved.
This may take a few comments back and forth.

Once the issue is approved, assign it to yourself.

### Pull request
Clone the repo under your own GitHub account.

#### Branch
Issues are typically merged into the `master` branch.
If that will not be the case for a particular issue, it will be noted in the comments.
Develop on top of that branch.

#### Style
Figure out the style by looking at existing code.
Keep the code consistent.
If still in doubt, ask for guidance in a comment to the issue.

#### Comments
In samples, be very generous with comments.
In the product, if something needs commenting, think how you can make the code more explicit, so that it doesn't need commenting.

#### Tests
Add sufficient tests, and make sure all tests (not just the new ones) pass consistently.
If you changed the severity of the test `log_filter`, revert it back to `critical`.

#### Pull Request
Follow GitHub's instructions to [Create a pull request from a fork](https://docs.github.com/en/free-pro-team@latest/github/collaborating-with-issues-and-pull-requests/creating-a-pull-request-from-a-fork).

Address comments promptly.