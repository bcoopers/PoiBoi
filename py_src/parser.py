'''
Copyright 2018 Brian Coopersmith

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
'''

import grammar
import scanner

def expand(grammarpiece, token):
    if grammarpiece.__class__ == token.__class__:
        return [token]
    elif scanner.IsToken(grammarpiece):
        return None
    expansions = []
    allow_empty_expansion = False
    for descendents in grammarpiece.descendents():
        if len(descendents) == 0:
            allow_empty_expansion = True
            continue
        first_element = descendents[0]
        if expand(first_element, token) != None:
            expansions.append(descendents)
    assert len(expansions) < 2  # Ambiguous grammar.
    if len(expansions) == 1:
        return expansions[0]
    elif len(expansions) == 0 and allow_empty_expansion:
        return []
    else:
        return None


def ParseTokens(tokens, original_code):
    assert len(tokens) > 0
    root = grammar.Module()
    current_program = [root]
    token_index = 0
    while len(current_program) > 0:
        assert token_index < len(tokens)
        expansion = expand(current_program[0], tokens[token_index])
        if expansion == None:
            print "Mismatch. Got token:", tokens[token_index].content
            print "Expected:", current_program[0].content
            print "Line number: ", tokens[token_index].line_num
            print original_code.split("\n")[tokens[token_index].line_num - 1]
            return None
        if scanner.IsToken(current_program[0]):
            assert len(expansion) == 1
            assert current_program[0].__class__ == expansion[0].__class__
            current_program[0].content = expansion[0].content
            token_index += 1
            current_program.pop(0)
        else:
            current_program[0].children += expansion
            current_program.pop(0)
            current_program = expansion + current_program
    assert token_index == len(tokens)
    return root

def ParseCode(code):
    print "scanning tokens."
    tokens = scanner.Scan(code)
    if tokens == None:
        return None
    print "parsing tokens."
    return ParseTokens(tokens, code)


if __name__ == "__main__":
    main()
