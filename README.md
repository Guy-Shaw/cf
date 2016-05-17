# libcf -- C filter

### Filter C source code

#### Extract or suppress parts of C source code

Extract / suppress any combination of:

  1. comments,
  2. string literals (in double quotes),
  3. character literals (in single quotes),
  4. code (everything else).


`libcf` is a library that provides the data types and functions
makes it relatively easy to filter comments, strings, and character
literals.  It does this by maintaining a state machine.

Its state machine is not unlike many that are used
in commonly available utilities.  Usually the programs that use
them are specialized.  They are designed for some specific purpose,
at hand, like stripping comments.  But, libcf lets you use the
state machine for whatever.

They simplifying idea is to run the state machine and deliver to
the caller a series of super-characters.  These super-characters
are represented by `{ character class, character }` pairs.
These results can be considered to be an extended character set.
That is, we can think of the super-characters as character in some
sort of super-Unicode 64-bit character set.  We can pretend that
the characters inside comments are all distinct from any characters
outside of comments, and similarly for the super-unicode code blocks
for characters inside double quotes, and for those inside single quotes,
and all of those would be distinct from the "C code" character set.

So, for example, if I searched for an identifier in code, it would
not be confused with some similar looking string inside a comment
or inside quotes.  The logical extension of this would be to make
C keywords separate super-unicode code points, after the fashion
of Algol, but we do not do that here.

When operating on the stream of super-characters returned by libcf,
specifying which parts are to be kept and which parts are to be
filtered out is not much more difficult than using "ctype" functions
like `isalpha()` or `isdigit()`.

The library, `libcf` just translates a stream of C source code
characters into a stream of super-characters.  It says nothing
about what to do with them.

### The `cf` command

One "client" of `libcf` is the command, `cf`.

`cf` has a small vocabulary of C source character classes.
Using the `--show` option, `cf` can be instructed what is
to be done with each type of super-character.

Run `cf --help` to see al the options and some examples.

### Example
```
  --show=all,-comment
```
strips comments.


### Example
```
  --show=comments
```
filters out everything but comments

### Example
```
  --show=code
```
strips comments, inner-string, and inner-char
but leaves the single quote marks and double-quote marks



## Tip

If you extract some parts that do not have embedded newlines,
then the output could get all run together into one big line.

There is no option to print a separator string between occurrences
of parts.  I might add one, some day.

Meanwhile, a good way to deal with it is to use the option,
`--preserve-lines`.  That keeps things from all running together.
But, if you are not really interested in the line numbers, then
the result can be annoying, because there can be _huge_ runs
of blanks lines.  You can fix that by sending to `cat -s`.


```
   cf --show=inner-comment --preserve-lines cf.c | cat -s
```

## License

See the file `LICENSE.md`

-- Guy Shaw

   gshaw@acm.org

