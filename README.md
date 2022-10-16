# jems
jems: a stream-based JSON serialized for embedded systems

## About `jems`

`jems` is a compact, stream-based JSON serializer written in pure C for embedded
systems.

`jems` makes it easy generate complex JSON structures, writing the results into
a buffer, string or stream.  Specifically designed for embedded systems, `jems`
is:
* **compact**: one source file and one header file
* **portable**: written in pure C
* **deterministic**: `jems` uses user-provided data structures and never calls
`malloc()`.
* **yours to use**: `jems` is covered under the permissive MIT License.

`jems` derives much of its efficiency and small footprint by a philosophy of
trust: Rather than provide rigorous error checking of input parameters,
`jems` instead assumes that you provide valid parameters to function calls.

## A Short Example

`jems` has a "emit characters as you go" philosophy, which allows you to
generate huge JSON structures with minimal memory usage.  Here is a short
example:

```
#include "jems.h"
#include <stdio.h>

#defne MAX_LEVEL 10  // how deeply nested the JSON structures can get
static jems_level_t jems_levels[MAX_LEVEL];
static jems_t jems;

int main(void) {
    // initalize the jems object, using putc() as the method for writing chars.
    jems_init(&jems, jems_levels, MAX_LEVEL, putc);

    jems_object_open(&jems);       // start an object.
    jems_string(&jems, "colors");  // first object key is "colors"
    jems_array_open(&jems);        // first object value is an array
    jems_integer(&jems, 1);        // ... with three numbers
    jems_integer(&jems, 2);
    jems_integer(&jems, 3);
    jems_array_close(&jems);      // end of the array
    jems_string(&jems, "valid");  // second object key is "valid"
    jems_true(&jems);             // second object value is true
    jems_object_close(&jems);     // end of the object
}
```
This program will print
```
    {"colors":[1,2,3],"valid":true}
```
on the standard output.
