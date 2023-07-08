# Argument Policy

Up to [Documentation](README.md).


## Principles
1. Keep the library code simple, yet manageable and robust.
2. Enable callers to chose how to manage instance lifetimes.
3. Give callers a simple rule how to pass in arguments.


## Rule
Instances of non-primitive classes are passed in as __move references to pointer types__.


## Rationalization
### Pointers over references
References do not keep instances alive, i.e. they require the caller to manage their lifetime.
There is no other option.

Pointers allow the caller to choose between:
- Assume responsibility for an instance's lifetime management by passing in a __raw pointer__.
- Delegating lifetime management by passing in a `std::unique_ptr`.
- Share lifetime management by passing in a `std::shared_ptr`.

### Move over copy
If the caller wants to pass in a copy of the pointer to an instance, it can create a temporary copy, and pass in a move reference to that temporary copy.
There are several easy ways to create such temporary copies:
- `abc::copy(ptr)`
- `decltype(ptr)(ptr)`
- Using the specific constructor, e.g. `std::shared_ptr(ptr)`
