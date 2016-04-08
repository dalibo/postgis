3d stuff
========

- what operators are handled by postgis ?
  "exact": intersect ?
  "non exact": new functions gserialized_gidx_(gidx|geom)

Q: Do we keep these new functions even if they're not useful, or is it the duty
of a 3rd part lib like cfcgal to create them? And do we keep SQL export for
them? If we remove them, documentation needs to be updated

Julien: I think they should be kept, since they rely on postgis (GIDX) internal

- Check that ALTER OPERATOR FAMILY is handled by upgrade process

DONE:
- add 4d opclass
- check the 3d opclasses

NOT NEEDED
- add a _nd opclass, and and _nd brin add value function

General
=======

- Verify that the documentation doesn't mention stuff we removed or renamed.
  Ex:
  - C function brin add values (should not be documented?)
  - _2d, _3d and _4d functions

- improve regression tests
