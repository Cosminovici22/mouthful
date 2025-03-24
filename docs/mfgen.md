# Mfgen

Mfgen is a command line utility which generates the lexer for a given list of
tokens and dumps it to disk.

## Usage

```
mfgen <token_list> <lexer_dump>
```

Parameter `lexer_dump` is the destination where the generated lexer will be
dumped.

Parameter `token_list` is a file containing a list of (`id`, `string`) pairings
which must abide by the following rules:
- the format of an entry is `<id>: <string>`
- `0 <= id < 1 000 000 000`
- `string` must exclusively contain printable ASCII characters
- mutiple `string`s may share the same `id`
- a `string` ought to be identified only by its last `id` pair in the token list
- the following escape sequences are recognized inside `string`s:
	| Escape sequence | Character       | ASCII hex value |
	| --------------- | --------------- | --------------- |
	| `\f`            | Formfeed        | 0C              |
	| `\n`            | Linefeed        | 0A              |
	| `\r`            | Carriage Return | 0D              |
	| `\s`            | Space           | 20              |
	| `\t`            | Horizontal Tab  | 09              |
	| `\v`            | Vertical Tab    | 0B              |
	| `\\`            | Backslash       | 5C              |

Example token lists are available inside subdirectories of `tests/`.
